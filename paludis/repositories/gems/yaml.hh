/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010 Ciaran McCreesh
 *
 * This file is part of the Paludis package manager. Paludis is free software;
 * you can redistribute it and/or modify it under the terms of the GNU General
 * Public License version 2, as published by the Free Software Foundation.
 *
 * Paludis is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_GEMS_YAML_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_GEMS_YAML_HH 1

#include <paludis/repositories/gems/yaml-fwd.hh>
#include <paludis/util/simple_visitor.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/singleton.hh>
#include <paludis/util/pimp.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/type_list.hh>
#include <paludis/util/wrapped_forward_iterator.hh>

namespace paludis
{
    namespace yaml
    {
        class Node;
        class StringNode;
        class SequenceNode;
        class MapNode;

        /**
         * A node in a yaml document.
         *
         * \ingroup grpgemsrepository
         * \nosubgrouping
         */
        class PALUDIS_VISIBLE Node :
            public virtual DeclareAbstractAcceptMethods<Node, MakeTypeList<
                StringNode, SequenceNode, MapNode>::Type>
        {
            public:
                ///\name Basic operations
                ///\{

                virtual ~Node() = 0;

                ///\}
        };

        /**
         * A string node in a yaml document.
         *
         * \ingroup grpgemsrepository
         * \nosubgrouping
         */
        class PALUDIS_VISIBLE StringNode :
            public Node,
            public ImplementAcceptMethods<Node, StringNode>,
            private Pimp<StringNode>
        {
            public:
                ///\name Basic operations
                ///\{

                StringNode(const std::string &);
                ~StringNode();

                ///\}

                /**
                 * The node's raw text.
                 */
                std::string text() const PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        /**
         * A sequence node in a yaml document.
         *
         * \ingroup grpgemsrepository
         * \nosubgrouping
         */
        class PALUDIS_VISIBLE SequenceNode :
            public Node,
            public ImplementAcceptMethods<Node, SequenceNode>,
            private Pimp<SequenceNode>
        {
            public:
                ///\name Basic operations
                ///\{

                SequenceNode();
                ~SequenceNode();

                ///\}

                /**
                 * Add a child node.
                 */
                void push_back(const Node * const);

                ///\name Iterate over our child nodes.
                ///\{

                struct ConstIteratorTag;
                typedef WrappedForwardIterator<ConstIteratorTag, const Node * const> ConstIterator;
                ConstIterator begin() const PALUDIS_ATTRIBUTE((warn_unused_result));
                ConstIterator end() const PALUDIS_ATTRIBUTE((warn_unused_result));

                ///\}
        };

        /**
         * A mapping node in a yaml document.
         *
         * \ingroup grpgemsrepository
         * \nosubgrouping
         */
        class PALUDIS_VISIBLE MapNode :
            public Node,
            public ImplementAcceptMethods<Node, MapNode>,
            private Pimp<MapNode>
        {
            public:
                ///\name Basic operations
                ///\{

                MapNode();
                ~MapNode();

                ///\}

                /**
                 * Add a child node pair.
                 */
                void push_back(const std::pair<const Node *, const Node *> &);

                ///\name Iterate over and find our child nodes.
                ///\{

                struct ConstIteratorTag;
                typedef WrappedForwardIterator<ConstIteratorTag, const std::pair<const Node *, const Node *> > ConstIterator;
                ConstIterator begin() const PALUDIS_ATTRIBUTE((warn_unused_result));
                ConstIterator end() const PALUDIS_ATTRIBUTE((warn_unused_result));
                ConstIterator find(const std::string &) const PALUDIS_ATTRIBUTE((warn_unused_result));

                ///\}
        };

        /**
         * A yaml document.
         *
         * \ingroup grpgemsrepository
         * \nosubgrouping
         */
        class PALUDIS_VISIBLE Document :
            private Pimp<Document>
        {
            public:
                ///\name Basic operations
                ///\{

                Document(const std::string &);
                ~Document();

                ///\}

                /**
                 * The top node in our document.
                 */
                const Node * top() const PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        /**
         * Handles memory management for yaml nodes, since syck assumes garbage
         * collection.
         *
         * \ingroup grpgemsrepository
         * \nosubgrouping
         */
        class PALUDIS_VISIBLE NodeManager :
            private Pimp<NodeManager>,
            public Singleton<NodeManager>
        {
            friend class Singleton<NodeManager>;

            private:
                ///\name Basic operations
                ///\{

                NodeManager();
                ~NodeManager();

                ///\}

            public:
                ///\name Memory management operations
                ///\{

                void register_document(const void * const);
                void deregister_document(const void * const);

                void manage_node(const void * const, const Node * const);

                ///\}
        };

        /**
         * Thrown if a yaml document cannot be parsed.
         *
         * \ingroup grpgemsrepository
         * \ingroup grpexceptions
         * \nosubgrouping
         */
        class PALUDIS_VISIBLE ParseError :
            public Exception
        {
            public:
                ///\name Basic operations
                ///\{

                ParseError(const std::string &) throw ();

                ///\}
        };
    }

    extern template class WrappedForwardIterator<yaml::MapNode::ConstIteratorTag, const std::pair<const yaml::Node *, const yaml::Node *> >;
    extern template class WrappedForwardIterator<yaml::SequenceNode::ConstIteratorTag, const yaml::Node * const>;

}

#endif
