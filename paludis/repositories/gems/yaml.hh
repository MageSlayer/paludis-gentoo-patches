/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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
#include <paludis/util/visitor.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/exception.hh>

namespace paludis
{
    namespace yaml
    {
        class Node;
        class StringNode;
        class SequenceNode;
        class MapNode;

        struct NodeVisitorTypes :
            VisitorTypes<
                NodeVisitorTypes,
                Node,
                StringNode,
                SequenceNode,
                MapNode>
        {
        };

        class PALUDIS_VISIBLE Node :
            public virtual ConstAcceptInterface<NodeVisitorTypes>
        {
            public:
                virtual ~Node() = 0;
        };

        class PALUDIS_VISIBLE StringNode :
            public Node,
            public ConstAcceptInterfaceVisitsThis<NodeVisitorTypes, StringNode>,
            private PrivateImplementationPattern<StringNode>
        {
            public:
                StringNode(const std::string &);
                ~StringNode();

                std::string text() const PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class PALUDIS_VISIBLE SequenceNode :
            public Node,
            public ConstAcceptInterfaceVisitsThis<NodeVisitorTypes, SequenceNode>,
            private PrivateImplementationPattern<SequenceNode>
        {
            public:
                SequenceNode();
                ~SequenceNode();

                void push_back(const Node * const);

                typedef libwrapiter::ForwardIterator<SequenceNode, const Node * const> Iterator;
                Iterator begin() const PALUDIS_ATTRIBUTE((warn_unused_result));
                Iterator end() const PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class PALUDIS_VISIBLE MapNode :
            public Node,
            public ConstAcceptInterfaceVisitsThis<NodeVisitorTypes, MapNode>,
            private PrivateImplementationPattern<MapNode>
        {
            public:
                MapNode();
                ~MapNode();

                void push_back(const std::pair<const Node *, const Node *> &);

                typedef libwrapiter::ForwardIterator<MapNode, const std::pair<const Node *, const Node *> > Iterator;
                Iterator begin() const PALUDIS_ATTRIBUTE((warn_unused_result));
                Iterator end() const PALUDIS_ATTRIBUTE((warn_unused_result));
                Iterator find(const std::string &) const PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class PALUDIS_VISIBLE Document :
            private PrivateImplementationPattern<Document>
        {
            public:
                Document(const std::string &);
                ~Document();

                const Node * top() const PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class PALUDIS_VISIBLE NodeManager :
            private PrivateImplementationPattern<NodeManager>,
            public InstantiationPolicy<NodeManager, instantiation_method::SingletonTag>
        {
            friend class InstantiationPolicy<NodeManager, instantiation_method::SingletonTag>;

            private:
                NodeManager();
                ~NodeManager();

            public:
                void register_document(const void * const);
                void deregister_document(const void * const);

                void manage_node(const void * const, const Node * const);
        };

        class PALUDIS_VISIBLE ParseError :
            public Exception
        {
            public:
                ParseError(const std::string &) throw ();
        };
    }
}

#endif
