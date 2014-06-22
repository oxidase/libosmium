#ifndef OSMIUM_AREA_MULTIPOLYGON_COLLECTOR_HPP
#define OSMIUM_AREA_MULTIPOLYGON_COLLECTOR_HPP

/*

This file is part of Osmium (http://osmcode.org/libosmium).

Copyright 2014 Jochen Topf <jochen@topf.org> and others (see README).

Boost Software License - Version 1.0 - August 17th, 2003

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

*/

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <vector>

#include <osmium/memory/buffer.hpp>
#include <osmium/osm/item_type.hpp>
#include <osmium/osm/relation.hpp>
#include <osmium/osm/tag.hpp>
#include <osmium/osm/way.hpp>
#include <osmium/relations/collector.hpp>
#include <osmium/relations/detail/member_meta.hpp>
#include <osmium/relations/detail/relation_meta.hpp>

namespace osmium {

    /**
     * @brief Code related to the building of areas (multipolygons) from relations.
     */
    namespace area {

        /**
         * This class collects all data needed for creating areas from
         * relations tagged with type=multipolygon or type=boundary.
         * Most of its functionality is derived from the parent class
         * osmium::relations::Collector.
         *
         * The actual assembling of the areas is done by the assembler
         * class given as template argument.
         *
         * @tparam TAssembler Multipolygon Assembler class.
         */
        template <class TAssembler>
        class MultipolygonCollector : public osmium::relations::Collector<MultipolygonCollector<TAssembler>, false, true, false> {

            typedef typename osmium::relations::Collector<MultipolygonCollector<TAssembler>, false, true, false> collector_type;

            typedef typename TAssembler::config_type assembler_config_type;
            const assembler_config_type m_assembler_config;

            osmium::memory::Buffer m_output_buffer;

            static constexpr size_t initial_output_buffer_size = 1024 * 1024;
            static constexpr size_t max_buffer_size_for_flush = 100 * 1024;

            void flush_output_buffer() {
                if (this->callback()) {
                    this->callback()(m_output_buffer);
                    m_output_buffer.clear();
                }
            }

            void possibly_flush_output_buffer() {
                if (m_output_buffer.committed() > max_buffer_size_for_flush) {
                    flush_output_buffer();
                }
            }

        public:

            MultipolygonCollector(const assembler_config_type& assembler_config) :
                collector_type(),
                m_assembler_config(assembler_config),
                m_output_buffer(initial_output_buffer_size, osmium::memory::Buffer::auto_grow::yes) {
            }

            /**
             * We are interested in all relations tagged with type=multipolygon or
             * type=boundary.
             *
             * Overwritten from the base class.
             */
            bool keep_relation(const osmium::Relation& relation) const {
                const char* type = relation.tags().get_value_by_key("type");

                // ignore relations without "type" tag
                if (!type) {
                    return false;
                }

                if ((!strcmp(type, "multipolygon")) || (!strcmp(type, "boundary"))) {
                    return true;
                }

                return false;
            }

            /**
             * Overwritten from the base class.
             */
            bool keep_member(const osmium::relations::RelationMeta& /*relation_meta*/, const osmium::RelationMember& member) const {
                // We are only interested in members of type way.
                return member.type() == osmium::item_type::way;
            }

            /**
             * This is called when a way is not in any multipolygon
             * relation.
             *
             * Overwritten from the base class.
             */
            void way_not_in_any_relation(const osmium::Way& way) {
                if (way.ends_have_same_location() && way.nodes().size() > 3) {
                    // way is closed and has enough nodes, build simple multipolygon
                    TAssembler assembler(m_assembler_config);
                    assembler(way, m_output_buffer);
                    possibly_flush_output_buffer();
                }
            }

            void complete_relation(osmium::relations::RelationMeta& relation_meta) {
                const osmium::Relation& relation = this->get_relation(relation_meta);
                std::vector<size_t> offsets;
                for (const auto& member : relation.members()) {
                    if (member.ref() != 0) {
                        offsets.push_back(this->get_offset(member.type(), member.ref()));
                    }
                }
                TAssembler assembler(m_assembler_config);
                assembler(relation, offsets, this->members_buffer(), m_output_buffer);
                possibly_flush_output_buffer();

                // clear member metas
                for (const auto& member : relation.members()) {
                    if (member.ref() != 0) {
                        auto& mmv = this->member_meta(member.type());
                        auto range = std::equal_range(mmv.begin(), mmv.end(), osmium::relations::MemberMeta(member.ref()));
                        assert(range.first != range.second);

                        // if this is the last time this object was needed
                        // the mark it as deleted
                        if (range.first + 1 == range.second) {
                            this->get_member(range.first->buffer_offset()).deleted(true);
                        }

                        for (auto it = range.first; it != range.second; ++it) {
                            if (relation.id() == this->get_relation(it->relation_pos()).id()) {
                                mmv.erase(it);
                                break;
                            }
                        }
                    }
                }
            }

            void flush() {
                flush_output_buffer();
            }

            void report_missing() {
                collector_type::clean_assembled_relations();
                if (! collector_type::relations().empty()) {
                    std::cerr << "Warning! Some member ways missing for these multipolygon relations:";
                    for (const osmium::relations::RelationMeta& relation_meta : collector_type::relations()) {
                        std::cerr << " " << this->get_relation(relation_meta).id();
                    }
                    std::cerr << "\n";
                }
            }

            osmium::memory::Buffer read() {
                osmium::memory::Buffer buffer(initial_output_buffer_size, osmium::memory::Buffer::auto_grow::yes);
                std::swap(buffer, m_output_buffer);
                return buffer;
            }

        }; // class MultipolygonCollector

    } // namespace area

} // namespace osmium

#endif // OSMIUM_AREA_MULTIPOLYGON_COLLECTOR_HPP
