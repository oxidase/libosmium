#ifndef OSMIUM_HANDLER_HPP
#define OSMIUM_HANDLER_HPP

/*

This file is part of Osmium (http://osmcode.org/libosmium).

Copyright 2013,2014 Jochen Topf <jochen@topf.org> and others (see README).

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

namespace osmium {

    class Object;
    class Node;
    class Way;
    class Relation;
    class Area;
    class Changeset;
    class TagList;
    class WayNodeList;
    class RelationMemberList;
    class OuterRing;
    class InnerRing;

    /**
     * @brief Osmium handlers provide callbacks for OSM objects
     */
    namespace handler {

        class Handler {

        public:

            void osm_object(const osmium::Object&) const {
            }

            void node(const osmium::Node&) const {
            }

            void way(const osmium::Way&) const {
            }

            void relation(const osmium::Relation&) const {
            }

            void area(const osmium::Area&) const {
            }

            void changeset(const osmium::Changeset&) const {
            }

            void tag_list(const osmium::TagList&) const {
            }

            void way_node_list(const osmium::WayNodeList&) const {
            }

            void relation_member_list(const osmium::RelationMemberList&) const {
            }

            void outer_ring(const osmium::OuterRing&) const {
            }

            void inner_ring(const osmium::InnerRing&) const {
            }

            void flush() const {
            }

        }; // class Handler

    } // namspace handler

} // namespace osmium

#endif // OSMIUM_HANDLER_HPP
