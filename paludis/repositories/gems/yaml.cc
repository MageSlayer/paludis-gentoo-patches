/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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
#include <paludis/util/fs_entry.hh>
#include <paludis/util/stringify.hh>
#include <yaml.h>
#include <list>

using namespace paludis;

YamlNode::YamlNode()
{
}

YamlNode::~YamlNode()
{
}

YamlScalarNode::YamlScalarNode(const std::string & v, const std::string & t) :
    _value(v),
    _tag(t)
{
}

namespace paludis
{
    template<>
    struct Implementation<YamlMappingNode>
    {
        std::list<std::pair<tr1::shared_ptr<YamlScalarNode>, tr1::shared_ptr<YamlNode> > > nodes;
    };

    template<>
    struct Implementation<YamlSequenceNode>
    {
        std::list<tr1::shared_ptr<YamlNode> > nodes;
    };
}


YamlMappingNode::YamlMappingNode() :
    PrivateImplementationPattern<YamlMappingNode>(new Implementation<YamlMappingNode>)
{
}

YamlMappingNode::Iterator
YamlMappingNode::begin() const
{
    return Iterator(_imp->nodes.begin());
}

YamlMappingNode::Iterator
YamlMappingNode::end() const
{
    return Iterator(_imp->nodes.end());
}

std::pair<tr1::shared_ptr<YamlScalarNode>, tr1::shared_ptr<YamlNode> > &
YamlMappingNode::back()
{
    return _imp->nodes.back();
}

YamlSequenceNode::YamlSequenceNode() :
    PrivateImplementationPattern<YamlSequenceNode>(new Implementation<YamlSequenceNode>)
{
}

YamlSequenceNode::Iterator
YamlSequenceNode::begin() const
{
    return Iterator(_imp->nodes.begin());
}

YamlSequenceNode::Iterator
YamlSequenceNode::end() const
{
    return Iterator(_imp->nodes.end());
}

void
YamlSequenceNode::add(tr1::shared_ptr<YamlNode> node)
{
    _imp->nodes.push_back(node);
}

void
YamlMappingNode::add(tr1::shared_ptr<YamlScalarNode> a, tr1::shared_ptr<YamlNode> b)
{
    _imp->nodes.push_back(std::make_pair(a, b));
}

namespace paludis
{
    template<>
    struct Implementation<YamlDocument>
    {
        tr1::shared_ptr<YamlSequenceNode> top;

        void parse(yaml_parser_t * parser);
        tr1::shared_ptr<YamlScalarNode> parse_scalar(yaml_parser_t * parser);

        Implementation() :
            top(new YamlSequenceNode)
        {
        }
    };
}

namespace
{
    template <typename PtrType_, typename ReturnType_ = void>
    class PtrHolder
    {
        private:
            PtrType_ _ptr;
            ReturnType_ (* _free_func) (PtrType_);
            bool _new_used;

            PtrHolder(const PtrHolder &);
            void operator= (const PtrHolder &);

        public:
            PtrHolder(PtrType_ ptr, ReturnType_ (* free_func) (PtrType_), bool new_used = false) :
                _ptr(ptr),
                _free_func(free_func),
                _new_used(new_used)
            {
            }

            ~PtrHolder()
            {
                if (0 != _ptr)
                {
                    _free_func(_ptr);
                    if (_new_used)
                        delete _ptr;
                }
            }

            operator PtrType_ () const
            {
                return _ptr;
            }
    };

    struct ScalarAdder :
        YamlNodeVisitorTypes::Visitor
    {
        tr1::shared_ptr<YamlScalarNode> a;

        ScalarAdder(tr1::shared_ptr<YamlScalarNode> aa) :
            a(aa)
        {
        }

        void visit(YamlMappingNode * n)
        {
            if (n->empty() || n->back().second)
                n->add(a, tr1::shared_ptr<YamlNode>());
            else
                n->back().second = a;
        }

        void visit(YamlScalarNode *) PALUDIS_ATTRIBUTE((noreturn))
        {
            throw YamlError("Can't add a YamlScalarNode to a YamlScalarNode");
        }

        void visit(YamlSequenceNode * n)
        {
            n->add(a);
        }
    };

    struct NonScalarAdder :
        YamlNodeVisitorTypes::Visitor
    {
        tr1::shared_ptr<YamlNode> a;

        NonScalarAdder(tr1::shared_ptr<YamlNode> aa) :
            a(aa)
        {
        }

        void visit(YamlMappingNode * n)
        {
            if (n->empty() || n->back().second)
                throw YamlError("Can't add a non YamlScalarNode to a YamlMappingNode on the left");
            else
                n->back().second = a;
        }

        void visit(YamlScalarNode *) PALUDIS_ATTRIBUTE((noreturn))
        {
            throw YamlError("Can't add a YamlScalarNode to a YamlScalarNode");
        }

        void visit(YamlSequenceNode * n)
        {
            n->add(a);
        }
    };
}

YamlDocument::YamlDocument(const std::string & s) :
    PrivateImplementationPattern<YamlDocument>(new Implementation<YamlDocument>)
{
    Context context("When creating YamlDocument from string:");

    PtrHolder<yaml_parser_t *> parser(new yaml_parser_t, yaml_parser_delete, true);
    yaml_parser_initialize(parser);
    /* silly c api */
    yaml_parser_set_input_string(parser, const_cast<unsigned char *>(
                reinterpret_cast<const unsigned char *>(s.c_str())), s.length());

    _imp->parse(parser);
}

YamlDocument::YamlDocument(const FSEntry & loc) :
    PrivateImplementationPattern<YamlDocument>(new Implementation<YamlDocument>)
{
    Context context("When creating YamlDocument from location '" + stringify(loc) + "':");

    if (! loc.is_regular_file())
        throw YamlError("Document '" + stringify(loc) + "' is not a regular file");

    PtrHolder<FILE *, int> f(std::fopen(stringify(loc).c_str(), "rb"), &std::fclose);
    if (! f)
        throw YamlError("Document '" + stringify(loc) + "' is not readable");

    PtrHolder<yaml_parser_t *> parser(new yaml_parser_t, yaml_parser_delete, true);
    yaml_parser_initialize(parser);
    yaml_parser_set_input_file(parser, f);

    _imp->parse(parser);
}

void
Implementation<YamlDocument>::parse(yaml_parser_t * parser)
{
    std::list<tr1::shared_ptr<YamlNode> > stack;
    stack.push_back(top);

    bool done(false);

    while (! done)
    {
        PtrHolder<yaml_event_t *> event_holder(new yaml_event_t, yaml_event_delete, true);
        std::memset(event_holder, 0, sizeof(*event_holder));
        if (! yaml_parser_parse(parser, event_holder))
            throw YamlError("Error parsing document");

        yaml_event_t * event(event_holder);

        switch (event->type)
        {
            case YAML_STREAM_START_EVENT:
                break;

            case YAML_STREAM_END_EVENT:
                done = true;
                break;

            case YAML_MAPPING_START_EVENT:
                {
                    tr1::shared_ptr<YamlMappingNode> node(new YamlMappingNode);
                    if (stack.empty())
                        throw YamlError("Error building tree: stack empty on YAML_SEQUENCE_START_EVENT");

                    if (stack.back())
                    {
                        NonScalarAdder a(node);
                        stack.back()->accept(&a);
                    }
                    else
                        stack.back() = node;
                    stack.push_back(node);
                }
                break;

            case YAML_MAPPING_END_EVENT:
            case YAML_SEQUENCE_END_EVENT:
                if (stack.empty())
                    throw YamlError("Error building tree: stack empty on YAML_*_END_EVENT");
                stack.pop_back();
                break;

            case YAML_SEQUENCE_START_EVENT:
                {
                    tr1::shared_ptr<YamlSequenceNode> node(new YamlSequenceNode);
                    if (stack.empty())
                        throw YamlError("Error building tree: stack empty on YAML_SEQUENCE_START_EVENT");
                    if (stack.back())
                    {
                        NonScalarAdder a(node);
                        stack.back()->accept(&a);
                    }
                    else
                        stack.back() = node;
                    stack.push_back(node);
                }
                break;

            case YAML_SCALAR_EVENT:
                {
                    if (stack.empty())
                        throw YamlError("Error building tree: stack empty on YAML_SCALAR_EVENT");
                    tr1::shared_ptr<YamlScalarNode> node(new YamlScalarNode(
                                event->data.scalar.value ? reinterpret_cast<const char *>(event->data.scalar.value) : "",
                                event->data.scalar.tag ? reinterpret_cast<const char *>(event->data.scalar.tag) : ""));
                    if (stack.back())
                    {
                        ScalarAdder a(node);
                        stack.back()->accept(&a);
                    }
                    else
                        stack.back() = node;
                }
                break;

            case YAML_NO_EVENT:
            case YAML_DOCUMENT_START_EVENT:
            case YAML_DOCUMENT_END_EVENT:
            case YAML_ALIAS_EVENT:
                break;
        }
    };

    if (stack.empty())
        throw YamlError("Error building tree: stack empty at end");
    stack.pop_back();
    if (! stack.empty())
        throw YamlError("Error building tree: stack not empty at end");
}

YamlDocument::~YamlDocument()
{
}

tr1::shared_ptr<const YamlNode>
YamlDocument::top() const
{
    return _imp->top;
}

YamlError::YamlError(const std::string & msg) throw () :
    ConfigurationError(msg)
{
}

