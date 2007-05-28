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

#include <algorithm>
#include <paludis/dep_spec.hh>
#include <paludis/dep_spec_pretty_printer.hh>
#include <paludis/util/save.hh>
#include <paludis/util/visitor-impl.hh>

/** \file
 * Implementation of dep_spec_pretty_printer.hh.
 *
 * \ingroup grpdepspecprettyprinter
 */

using namespace paludis;

namespace paludis
{
    template<>
    struct Implementation<DepSpecPrettyPrinter>
    {
        std::stringstream s;
        unsigned indent;
        bool use_newlines;
        bool outer_block;
        mutable bool need_space;

        Implementation(unsigned i, bool b) :
            indent(i),
            use_newlines(b),
            outer_block(true),
            need_space(false)
        {
        }
    };
}

DepSpecPrettyPrinter::DepSpecPrettyPrinter(unsigned i, bool b) :
    PrivateImplementationPattern<DepSpecPrettyPrinter>(new Implementation<DepSpecPrettyPrinter>(i, b))
{
}

DepSpecPrettyPrinter::~DepSpecPrettyPrinter()
{
}

std::ostream &
paludis::operator<< (std::ostream & s, const DepSpecPrettyPrinter & p)
{
    s << p._imp->s.str();
    return s;
}

void
DepSpecPrettyPrinter::visit_sequence(const AllDepSpec &,
        GenericSpecTree::ConstSequenceIterator cur,
        GenericSpecTree::ConstSequenceIterator end)
{
    if (! _imp->outer_block)
    {
        _imp->s << indent() << "(";
        _imp->s << newline();
    }

    {
        Save<unsigned> old_indent(&_imp->indent, _imp->outer_block ? _imp->indent : _imp->indent + 4);
        std::for_each(cur, end, accept_visitor(*this));
    }

    if (! _imp->outer_block)
    {
        _imp->s << indent() << ")";
        _imp->s << newline();
    }
}

void
DepSpecPrettyPrinter::visit_sequence(const AnyDepSpec &,
        GenericSpecTree::ConstSequenceIterator cur,
        GenericSpecTree::ConstSequenceIterator end)
{
    Save<bool> old_outer(&_imp->outer_block, false);

    _imp->s << indent() << "|| (";
    _imp->s << newline();
    {
        Save<unsigned> old_indent(&_imp->indent, _imp->indent + 4);
        std::for_each(cur, end, accept_visitor(*this));
    }
    _imp->s << indent() << ")";
    _imp->s << newline();
}

void
DepSpecPrettyPrinter::visit_sequence(const UseDepSpec & a,
        GenericSpecTree::ConstSequenceIterator cur,
        GenericSpecTree::ConstSequenceIterator end)
{
    Save<bool> old_outer(&_imp->outer_block, false);

    _imp->s << indent() << (a.inverse() ? "!" : "") << a.flag() << "? (";
    _imp->s << newline();
    {
        Save<unsigned> old_indent(&_imp->indent, _imp->indent + 4);
        std::for_each(cur, end, accept_visitor(*this));
    }
    _imp->s << indent() << ")";
    _imp->s << newline();
}

void
DepSpecPrettyPrinter::visit_leaf(const PackageDepSpec & p)
{
    _imp->s << indent() << p;
    _imp->s << newline();
}

void
DepSpecPrettyPrinter::visit_leaf(const PlainTextDepSpec & p)
{
    _imp->s << indent() << p.text();
    _imp->s << newline();
}

void
DepSpecPrettyPrinter::visit_leaf(const URIDepSpec & p)
{
    if (! p.renamed_url_suffix().empty())
    {
        _imp->s << indent() << p.original_url() << " -> " << p.renamed_url_suffix();
    }
    else
        _imp->s << indent() << p.original_url();
    _imp->s << newline();
}

void
DepSpecPrettyPrinter::visit_leaf(const BlockDepSpec & b)
{
    _imp->s << indent() << "!" << *b.blocked_spec();
    _imp->s << newline();
}

std::string
DepSpecPrettyPrinter::newline() const
{
    if (_imp->use_newlines)
        return "\n";
    else
    {
        _imp->need_space = true;
        return "";
    }
}

std::string
DepSpecPrettyPrinter::indent() const
{
    if (_imp->use_newlines)
        return std::string(_imp->indent, ' ');
    else if (_imp->need_space)
    {
        _imp->need_space = false;
        return " ";
    }
    else
        return "";
}

