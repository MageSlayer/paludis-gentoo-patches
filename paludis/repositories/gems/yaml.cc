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

#include "yaml.hh"
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/singleton-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <functional>
#include <syck.h>
#include <cstring>
#include <algorithm>
#include <map>
#include <list>

using namespace paludis;
using namespace paludis::yaml;

Node::~Node()
{
}

namespace paludis
{
    template <>
    struct Imp<StringNode>
    {
        const std::string text;

        Imp(const std::string & t) :
            text(t)
        {
        }
    };

    template <>
    struct WrappedForwardIteratorTraits<MapNode::ConstIteratorTag>
    {
        typedef std::list<std::pair<const Node *, const Node *> >::const_iterator UnderlyingIterator;
    };

    template <>
    struct WrappedForwardIteratorTraits<SequenceNode::ConstIteratorTag>
    {
        typedef std::list<const Node *>::const_iterator UnderlyingIterator;
    };
}

StringNode::StringNode(const std::string & t) :
    Pimp<StringNode>(t)
{
}

StringNode::~StringNode()
{
}

std::string
StringNode::text() const
{
    return _imp->text;
}

namespace paludis
{
    template <>
    struct Imp<SequenceNode>
    {
        std::list<const Node *> nodes;
    };
}

SequenceNode::SequenceNode() :
    Pimp<SequenceNode>()
{
}

SequenceNode::~SequenceNode()
{
}

void
SequenceNode::push_back(const Node * const n)
{
    _imp->nodes.push_back(n);
}

SequenceNode::ConstIterator
SequenceNode::begin() const
{
    return ConstIterator(_imp->nodes.begin());
}

SequenceNode::ConstIterator
SequenceNode::end() const
{
    return ConstIterator(_imp->nodes.end());
}

namespace paludis
{
    template <>
    struct Imp<MapNode>
    {
        std::list<std::pair<const Node *, const Node *> > nodes;
    };
}

MapNode::MapNode() :
    Pimp<MapNode>()
{
}

MapNode::~MapNode()
{
}

void
MapNode::push_back(const std::pair<const Node *, const Node *> & p)
{
    _imp->nodes.push_back(p);
}

MapNode::ConstIterator
MapNode::begin() const
{
    return ConstIterator(_imp->nodes.begin());
}

MapNode::ConstIterator
MapNode::end() const
{
    return ConstIterator(_imp->nodes.end());
}

namespace
{
    struct MatchStringVisitor
    {
        bool found;
        const std::string target;

        MatchStringVisitor(const std::string & s) :
            found(false),
            target(s)
        {
        }

        void visit(const StringNode & n)
        {
            found = n.text() == target;
        }

        void visit(const MapNode &)
        {
        }

        void visit(const SequenceNode &)
        {
        }
    };

    bool match_string_node(const std::string & s, const Node * const n)
    {
        MatchStringVisitor v(s);
        n->accept(v);
        return v.found;
    }
}

MapNode::ConstIterator
MapNode::find(const std::string & s) const
{
    using namespace std::placeholders;
    return std::find_if(begin(), end(),
            std::bind(match_string_node, s, std::bind<const Node *>(std::mem_fn(&std::pair<const Node *, const Node *>::first), _1)));
}

namespace
{
    static Mutex document_error_table_mutex;
    static std::map<void *, std::string> document_error_table;

    template <typename R_, typename T_>
    struct CallUnlessNull
    {
        R_ (* function) (T_ *);

        CallUnlessNull(R_ (*f) (T_ *)) :
            function(f)
        {
        }

        void operator() (T_ * const t) const
        {
            if (t)
                function(t);
        }
    };

    template <typename R_, typename T_>
    CallUnlessNull<R_, T_>
    call_unless_null(R_ (* f) (T_ *))
    {
        return CallUnlessNull<R_, T_>(f);
    }

    SYMID node_handler(SyckParser * p, SyckNode * n)
    {
        Node * node(0);

        switch (n->kind)
        {
            case syck_str_kind:
                {
                    node = new StringNode(std::string(n->data.str->ptr, n->data.str->len));
                    NodeManager::get_instance()->manage_node(p, node);
                }
                break;

            case syck_seq_kind:
                {
                    SequenceNode * s(new SequenceNode);
                    NodeManager::get_instance()->manage_node(p, s);
                    for (int i = 0 ; i < n->data.list->idx ; ++i)
                    {
                        SYMID v_id(syck_seq_read(n, i));
                        char * v(0);
                        syck_lookup_sym(p, v_id, &v);
                        s->push_back(reinterpret_cast<Node *>(v));
                    }
                    node = s;
                }
                break;

            case syck_map_kind:
                {
                    MapNode * m(new MapNode);
                    NodeManager::get_instance()->manage_node(p, m);
                    for (int i = 0 ; i < n->data.pairs->idx ; ++i)
                    {
                        SYMID k_id(syck_map_read(n, map_key, i)), v_id(syck_map_read(n, map_value, i));
                        char * k(0), * v(0);
                        syck_lookup_sym(p, k_id, &k);
                        syck_lookup_sym(p, v_id, &v);
                        m->push_back(std::make_pair(reinterpret_cast<Node *>(k), reinterpret_cast<Node *>(v)));
                    }
                    node = m;
                }
                break;
        }

        return syck_add_sym(p, reinterpret_cast<char *>(node));
    }

    void error_handler(SyckParser * p, char * s)
    {
        Lock l(document_error_table_mutex);
        document_error_table[p] = s;
    }
}

namespace paludis
{
    template <>
    struct Imp<Document>
    {
        struct Register
        {
            Imp<Document> * _imp;

            Register(Imp<Document> * imp) :
                _imp(imp)
            {
                NodeManager::get_instance()->register_document(_imp->parser.get());
            }

            ~Register()
            {
                NodeManager::get_instance()->deregister_document(_imp->parser.get());
            }
        };

        Node * top;
        std::shared_ptr<SyckParser> parser;
        std::shared_ptr<char> data;
        unsigned data_length;

        Register reg;

        Imp(const std::string & s) :
            top(0),
            parser(syck_new_parser(), call_unless_null(syck_free_parser)),
            data(strdup(s.c_str()), call_unless_null(std::free)),
            data_length(s.length()),
            reg(this)
        {
        }
    };
}

Document::Document(const std::string & s) :
    Pimp<Document>(s)
{
    Context c("When parsing yaml document:");

    syck_parser_str(_imp->parser.get(), _imp->data.get(), _imp->data_length, 0);
    syck_parser_handler(_imp->parser.get(), node_handler);
    syck_parser_error_handler(_imp->parser.get(), error_handler);

    SYMID root_id(syck_parse(_imp->parser.get()));

    {
        Lock l(document_error_table_mutex);
        if (document_error_table.end() != document_error_table.find(_imp->parser.get()))
        {
            std::string e(document_error_table.find(_imp->parser.get())->second);
            document_error_table.erase(_imp->parser.get());
            throw ParseError(e);
        }
    }

    char * root_uncasted(0);
    syck_lookup_sym(_imp->parser.get(), root_id, &root_uncasted);
    _imp->top = reinterpret_cast<Node *>(root_uncasted);
}

Document::~Document()
{
}

const Node *
Document::top() const
{
    return _imp->top;
}

namespace paludis
{
    template <>
    struct Imp<NodeManager>
    {
        std::map<const void *, std::list<std::shared_ptr<const Node> > > store;
    };
}

NodeManager::NodeManager() :
    Pimp<NodeManager>()
{
}

NodeManager::~NodeManager()
{
}

void
NodeManager::register_document(const void * const d)
{
    if (! _imp->store.insert(std::make_pair(d, std::list<std::shared_ptr<const Node> >())).second)
        throw InternalError(PALUDIS_HERE, "duplicate document");
}

void
NodeManager::deregister_document(const void * const d)
{
    if (0 == _imp->store.erase(d))
        throw InternalError(PALUDIS_HERE, "no such document");
}

void
NodeManager::manage_node(const void * const d, const Node * const n)
{
    std::map<const void *, std::list<std::shared_ptr<const Node> > >::iterator i(_imp->store.find(d));
    if (i == _imp->store.end())
        throw InternalError(PALUDIS_HERE, "no such document");
    i->second.push_back(std::shared_ptr<const Node>(n));
}

ParseError::ParseError(const std::string & s) throw () :
    Exception(s)
{
}

template class Singleton<NodeManager>;

template class WrappedForwardIterator<MapNode::ConstIteratorTag, const std::pair<const Node *, const Node *> >;
template class WrappedForwardIterator<SequenceNode::ConstIteratorTag, const Node * const>;


