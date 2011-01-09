/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010, 2011 Ciaran McCreesh
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

#include <paludis/repositories/e/spec_tree_pretty_printer.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/save.hh>
#include <paludis/util/accept_visitor.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/pretty_printer.hh>
#include <paludis/dep_spec_annotations.hh>
#include <algorithm>
#include <sstream>

using namespace paludis;
using namespace paludis::erepository;

namespace paludis
{
    template<>
    struct Imp<SpecTreePrettyPrinter>
    {
        std::stringstream s;
        const PrettyPrinter & pretty_printer;
        const PrettyPrintOptions options;

        unsigned indent;
        bool extra_label_indent;
        bool use_newlines;
        bool outer_block;
        bool all_needs_parens;
        bool need_space;

        Imp(const PrettyPrinter & p, const PrettyPrintOptions & o) :
            pretty_printer(p),
            options(o),
            indent(0),
            extra_label_indent(false),
            use_newlines(options[ppo_multiline_allowed]),
            outer_block(true),
            all_needs_parens(false),
            need_space(false)
        {
        }
    };
}

SpecTreePrettyPrinter::SpecTreePrettyPrinter(
        const PrettyPrinter & p,
        const PrettyPrintOptions & o) :
    _imp(p, o)
{
}

SpecTreePrettyPrinter::~SpecTreePrettyPrinter() = default;

std::ostream &
paludis::erepository::operator<< (std::ostream & s, const SpecTreePrettyPrinter & p)
{
    s << p._imp->s.str();
    return s;
}

namespace
{
    struct IsLabelVisitor
    {
        bool result;

        IsLabelVisitor() :
            result(false)
        {
        }

        void visit(const GenericSpecTree::NodeType<PlainTextDepSpec>::Type &)
        {
        }

        void visit(const GenericSpecTree::NodeType<SimpleURIDepSpec>::Type &)
        {
        }

        void visit(const GenericSpecTree::NodeType<FetchableURIDepSpec>::Type &)
        {
        }

        void visit(const GenericSpecTree::NodeType<LicenseDepSpec>::Type &)
        {
        }

        void visit(const GenericSpecTree::NodeType<PackageDepSpec>::Type &)
        {
        }

        void visit(const GenericSpecTree::NodeType<BlockDepSpec>::Type &)
        {
        }

        void visit(const GenericSpecTree::NodeType<PlainTextLabelDepSpec>::Type &)
        {
            result = true;
        }

        void visit(const GenericSpecTree::NodeType<URILabelsDepSpec>::Type &)
        {
            result = true;
        }

        void visit(const GenericSpecTree::NodeType<DependenciesLabelsDepSpec>::Type &)
        {
            result = true;
        }

        void visit(const GenericSpecTree::NodeType<NamedSetDepSpec>::Type &)
        {
        }

        void visit(const GenericSpecTree::NodeType<AllDepSpec>::Type &)
        {
        }

        void visit(const GenericSpecTree::NodeType<ExactlyOneDepSpec>::Type &)
        {
        }

        void visit(const GenericSpecTree::NodeType<AnyDepSpec>::Type &)
        {
        }

        void visit(const GenericSpecTree::NodeType<ConditionalDepSpec>::Type &)
        {
        }
    };

    bool is_label(const GenericSpecTree::BasicNode & i)
    {
        IsLabelVisitor v;
        i.accept(v);
        return v.result;
    }
}

void
SpecTreePrettyPrinter::visit(const GenericSpecTree::NodeType<AllDepSpec>::Type & node)
{
    bool need_parens(_imp->all_needs_parens || node.spec()->maybe_annotations() ||
            (! _imp->outer_block && indirect_iterator(node.end()) != std::find_if(indirect_iterator(node.begin()),
                                                                                  indirect_iterator(node.end()),
                                                                                  is_label)));
    Save<bool> old_outer(&_imp->outer_block, false);
    Save<bool> old_needs_parens(&_imp->all_needs_parens, false);

    if (need_parens)
    {
        if (_imp->use_newlines)
            _imp->s << _imp->pretty_printer.indentify(_imp->indent);
        else if (_imp->need_space)
            _imp->s << " ";
        _imp->s << "(";
        if (_imp->use_newlines)
            _imp->s << _imp->pretty_printer.newline();
        else
            _imp->need_space = true;
    }

    {
        Save<unsigned> old_indent(&_imp->indent, need_parens ? _imp->indent +1 : _imp->indent);
        Save<bool> extra_label_indent(&_imp->extra_label_indent, need_parens ? false : _imp->extra_label_indent);
        std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
    }

    if (need_parens)
    {
        if (_imp->use_newlines)
            _imp->s << _imp->pretty_printer.indentify(_imp->indent);
        else if (_imp->need_space)
            _imp->s << " ";
        _imp->s << ")";

        do_annotations(*node.spec());

        if (_imp->use_newlines)
            _imp->s << _imp->pretty_printer.newline();
        else
            _imp->need_space = true;
    }
}

void
SpecTreePrettyPrinter::visit(const GenericSpecTree::NodeType<AnyDepSpec>::Type & node)
{
    Save<bool> old_outer(&_imp->outer_block, false);
    Save<bool> old_needs_parens(&_imp->all_needs_parens, true);

    if (_imp->use_newlines)
        _imp->s << _imp->pretty_printer.indentify(_imp->indent);
    else if (_imp->need_space)
        _imp->s << " ";
    _imp->s << "|| (";
    if (_imp->use_newlines)
        _imp->s << _imp->pretty_printer.newline();
    else
        _imp->need_space = true;

    {
        Save<unsigned> old_indent(&_imp->indent, _imp->indent + 1);
        Save<bool> extra_label_indent(&_imp->extra_label_indent, false);
        std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
    }

    if (_imp->use_newlines)
        _imp->s << _imp->pretty_printer.indentify(_imp->indent);
    else if (_imp->need_space)
        _imp->s << " ";
    _imp->s << ")";

    do_annotations(*node.spec());

    if (_imp->use_newlines)
        _imp->s << _imp->pretty_printer.newline();
    else
        _imp->need_space = true;
}

void
SpecTreePrettyPrinter::visit(const GenericSpecTree::NodeType<ExactlyOneDepSpec>::Type & node)
{
    Save<bool> old_outer(&_imp->outer_block, false);
    Save<bool> old_needs_parens(&_imp->all_needs_parens, true);

    if (_imp->use_newlines)
        _imp->s << _imp->pretty_printer.indentify(_imp->indent);
    else if (_imp->need_space)
        _imp->s << " ";
    _imp->s << "^^ (";
    if (_imp->use_newlines)
        _imp->s << _imp->pretty_printer.newline();
    else
        _imp->need_space = true;

    {
        Save<unsigned> old_indent(&_imp->indent, _imp->indent + 1);
        Save<bool> extra_label_indent(&_imp->extra_label_indent, false);
        std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
    }

    if (_imp->use_newlines)
        _imp->s << _imp->pretty_printer.indentify(_imp->indent);
    else if (_imp->need_space)
        _imp->s << " ";
    _imp->s << ")";

    do_annotations(*node.spec());

    if (_imp->use_newlines)
        _imp->s << _imp->pretty_printer.newline();
    else
        _imp->need_space = true;
}

void
SpecTreePrettyPrinter::visit(const GenericSpecTree::NodeType<ConditionalDepSpec>::Type & node)
{
    Save<bool> old_outer(&_imp->outer_block, false);
    Save<bool> old_needs_parens(&_imp->all_needs_parens, false);

    if (_imp->use_newlines)
        _imp->s << _imp->pretty_printer.indentify(_imp->indent);
    else if (_imp->need_space)
        _imp->s << " ";

    _imp->s << _imp->pretty_printer.prettify(*node.spec()) << " (";

    if (_imp->use_newlines)
        _imp->s << _imp->pretty_printer.newline();
    else
        _imp->need_space = true;

    {
        Save<unsigned> old_indent(&_imp->indent, _imp->indent + 1);
        Save<bool> extra_label_indent(&_imp->extra_label_indent, false);
        std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
    }

    if (_imp->use_newlines)
        _imp->s << _imp->pretty_printer.indentify(_imp->indent);
    else if (_imp->need_space)
        _imp->s << " ";
    _imp->s << ")";

    do_annotations(*node.spec());

    if (_imp->use_newlines)
        _imp->s << _imp->pretty_printer.newline();
    else
        _imp->need_space = true;
}

void
SpecTreePrettyPrinter::visit(const GenericSpecTree::NodeType<PackageDepSpec>::Type & node)
{
    if (_imp->use_newlines)
        _imp->s << _imp->pretty_printer.indentify(_imp->indent);
    else if (_imp->need_space)
        _imp->s << " ";

    _imp->s << _imp->pretty_printer.prettify(*node.spec());

    do_annotations(*node.spec());

    if (_imp->use_newlines)
        _imp->s << _imp->pretty_printer.newline();
    else
        _imp->need_space = true;
}

void
SpecTreePrettyPrinter::visit(const GenericSpecTree::NodeType<PlainTextDepSpec>::Type & node)
{
    if (_imp->use_newlines)
        _imp->s << _imp->pretty_printer.indentify(_imp->indent);
    else if (_imp->need_space)
        _imp->s << " ";

    _imp->s << _imp->pretty_printer.prettify(*node.spec());

    do_annotations(*node.spec());

    if (_imp->use_newlines)
        _imp->s << _imp->pretty_printer.newline();
    else
        _imp->need_space = true;
}

void
SpecTreePrettyPrinter::visit(const GenericSpecTree::NodeType<NamedSetDepSpec>::Type & node)
{
    if (_imp->use_newlines)
        _imp->s << _imp->pretty_printer.indentify(_imp->indent);
    else if (_imp->need_space)
        _imp->s << " ";

    _imp->s << _imp->pretty_printer.prettify(*node.spec());

    do_annotations(*node.spec());

    if (_imp->use_newlines)
        _imp->s << _imp->pretty_printer.newline();
    else
        _imp->need_space = true;
}

void
SpecTreePrettyPrinter::visit(const GenericSpecTree::NodeType<LicenseDepSpec>::Type & node)
{
    if (_imp->use_newlines)
        _imp->s << _imp->pretty_printer.indentify(_imp->indent);
    else if (_imp->need_space)
        _imp->s << " ";

    _imp->s << _imp->pretty_printer.prettify(*node.spec());

    do_annotations(*node.spec());

    if (_imp->use_newlines)
        _imp->s << _imp->pretty_printer.newline();
    else
        _imp->need_space = true;
}

void
SpecTreePrettyPrinter::visit(const GenericSpecTree::NodeType<FetchableURIDepSpec>::Type & node)
{
    if (_imp->use_newlines)
        _imp->s << _imp->pretty_printer.indentify(_imp->indent);
    else if (_imp->need_space)
        _imp->s << " ";

    _imp->s << _imp->pretty_printer.prettify(*node.spec());

    do_annotations(*node.spec());

    if (_imp->use_newlines)
        _imp->s << _imp->pretty_printer.newline();
    else
        _imp->need_space = true;
}

void
SpecTreePrettyPrinter::visit(const GenericSpecTree::NodeType<SimpleURIDepSpec>::Type & node)
{
    if (_imp->use_newlines)
        _imp->s << _imp->pretty_printer.indentify(_imp->indent);
    else if (_imp->need_space)
        _imp->s << " ";

    _imp->s << _imp->pretty_printer.prettify(*node.spec());

    do_annotations(*node.spec());

    if (_imp->use_newlines)
        _imp->s << _imp->pretty_printer.newline();
    else
        _imp->need_space = true;
}

void
SpecTreePrettyPrinter::visit(const GenericSpecTree::NodeType<BlockDepSpec>::Type & node)
{
    if (_imp->use_newlines)
        _imp->s << _imp->pretty_printer.indentify(_imp->indent);
    else if (_imp->need_space)
        _imp->s << " ";

    _imp->s << _imp->pretty_printer.prettify(*node.spec());

    do_annotations(*node.spec());

    if (_imp->use_newlines)
        _imp->s << _imp->pretty_printer.newline();
    else
        _imp->need_space = true;
}

void
SpecTreePrettyPrinter::visit(const GenericSpecTree::NodeType<URILabelsDepSpec>::Type & node)
{
    if (_imp->extra_label_indent)
    {
        _imp->extra_label_indent = false;
        _imp->indent -= 1;
    }

    if (_imp->use_newlines)
        _imp->s << _imp->pretty_printer.indentify(_imp->indent);
    else if (_imp->need_space)
        _imp->s << " ";

    _imp->s << _imp->pretty_printer.prettify(*node.spec());

    do_annotations(*node.spec());

    if (_imp->use_newlines)
        _imp->s << _imp->pretty_printer.newline();
    else
        _imp->need_space = true;

    if (! _imp->extra_label_indent)
    {
        _imp->extra_label_indent = true;
        _imp->indent += 1;
    }
}

void
SpecTreePrettyPrinter::visit(const GenericSpecTree::NodeType<PlainTextLabelDepSpec>::Type & node)
{
    if (_imp->extra_label_indent)
    {
        _imp->extra_label_indent = false;
        _imp->indent -= 1;
    }

    if (_imp->use_newlines)
        _imp->s << _imp->pretty_printer.indentify(_imp->indent);
    else if (_imp->need_space)
        _imp->s << " ";

    _imp->s << _imp->pretty_printer.prettify(*node.spec());

    do_annotations(*node.spec());

    if (_imp->use_newlines)
        _imp->s << _imp->pretty_printer.newline();
    else
        _imp->need_space = true;

    if (! _imp->extra_label_indent)
    {
        _imp->extra_label_indent = true;
        _imp->indent += 1;
    }
}

void
SpecTreePrettyPrinter::visit(const GenericSpecTree::NodeType<DependenciesLabelsDepSpec>::Type & node)
{
    if (_imp->extra_label_indent)
    {
        _imp->extra_label_indent = false;
        _imp->indent -= 1;
    }

    if (_imp->use_newlines)
        _imp->s << _imp->pretty_printer.indentify(_imp->indent);
    else if (_imp->need_space)
        _imp->s << " ";

    _imp->s << _imp->pretty_printer.prettify(*node.spec());

    do_annotations(*node.spec());

    if (_imp->use_newlines)
        _imp->s << _imp->pretty_printer.newline();
    else
        _imp->need_space = true;

    if (!_imp->extra_label_indent)
    {
        _imp->extra_label_indent = true;
        _imp->indent += 1;
    }
}

void
SpecTreePrettyPrinter::do_annotations(const DepSpec & p)
{
    if (p.maybe_annotations() && (p.maybe_annotations()->begin() != p.maybe_annotations()->end()))
    {
        _imp->s << " [[ ";

        for (auto m(p.maybe_annotations()->begin()), m_end(p.maybe_annotations()->end()) ;
                m != m_end ; ++m)
        {
            _imp->s << m->key() << " = [" << (m->value().empty() ? " " : " " + m->value() + " ") << "] ";
        }
        _imp->s << "]]";
    }
}

