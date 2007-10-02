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
#include <sstream>
#include <paludis/dep_spec.hh>
#include <paludis/metadata_key.hh>
#include <paludis/formatter.hh>
#include <paludis/repositories/e/dep_spec_pretty_printer.hh>
#include <paludis/util/save.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/environment.hh>
#include <paludis/query.hh>
#include <paludis/package_database.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>

/** \file
 * Implementation of dep_spec_pretty_printer.hh.
 *
 * \ingroup grpdepspecprettyprinter
 */

using namespace paludis;
using namespace paludis::erepository;

namespace paludis
{
    template<>
    struct Implementation<DepSpecPrettyPrinter>
    {
        std::stringstream s;
        const Environment * const env;
        const tr1::shared_ptr<const PackageID> id;
        GenericSpecTree::Formatter formatter;
        unsigned indent;
        bool extra_label_indent;
        bool use_newlines;
        bool outer_block;
        bool need_space;

        Implementation(
                const Environment * const e,
                const tr1::shared_ptr<const PackageID> & i,
                const GenericSpecTree::Formatter & f,
                unsigned in,
                bool b) :
            env(e),
            id(i),
            formatter(f),
            indent(in),
            extra_label_indent(false),
            use_newlines(b),
            outer_block(true),
            need_space(false)
        {
        }
    };
}

DepSpecPrettyPrinter::DepSpecPrettyPrinter(
        const Environment * const e,
        const tr1::shared_ptr<const PackageID> & id,
        const GenericSpecTree::Formatter & f,
        unsigned i,
        bool b) :
    PrivateImplementationPattern<DepSpecPrettyPrinter>(new Implementation<DepSpecPrettyPrinter>(e, id, f, i, b))
{
}

DepSpecPrettyPrinter::~DepSpecPrettyPrinter()
{
}

std::ostream &
paludis::erepository::operator<< (std::ostream & s, const DepSpecPrettyPrinter & p)
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
        if (_imp->use_newlines)
            _imp->s << _imp->formatter.indent(_imp->indent);
        else if (_imp->need_space)
            _imp->s << " ";
        _imp->s << "(";
        if (_imp->use_newlines)
            _imp->s << _imp->formatter.newline();
        else
            _imp->need_space = true;
    }

    {
        Save<unsigned> old_indent(&_imp->indent, _imp->outer_block ? _imp->indent : _imp->indent + 1);
        Save<bool> extra_label_indent(&_imp->extra_label_indent, _imp->outer_block ? _imp->extra_label_indent : false);
        std::for_each(cur, end, accept_visitor(*this));
    }

    if (! _imp->outer_block)
    {
        if (_imp->use_newlines)
            _imp->s << _imp->formatter.indent(_imp->indent);
        else if (_imp->need_space)
            _imp->s << " ";
        _imp->s << ")";
        if (_imp->use_newlines)
            _imp->s << _imp->formatter.newline();
        else
            _imp->need_space = true;
    }
}

void
DepSpecPrettyPrinter::visit_sequence(const AnyDepSpec &,
        GenericSpecTree::ConstSequenceIterator cur,
        GenericSpecTree::ConstSequenceIterator end)
{
    Save<bool> old_outer(&_imp->outer_block, false);

    if (_imp->use_newlines)
        _imp->s << _imp->formatter.indent(_imp->indent);
    else if (_imp->need_space)
        _imp->s << " ";
    _imp->s << "|| (";
    if (_imp->use_newlines)
        _imp->s << _imp->formatter.newline();
    else
        _imp->need_space = true;

    {
        Save<unsigned> old_indent(&_imp->indent, _imp->indent + 1);
        Save<bool> extra_label_indent(&_imp->extra_label_indent, false);
        std::for_each(cur, end, accept_visitor(*this));
    }

    if (_imp->use_newlines)
        _imp->s << _imp->formatter.indent(_imp->indent);
    else if (_imp->need_space)
        _imp->s << " ";
    _imp->s << ")";
    if (_imp->use_newlines)
        _imp->s << _imp->formatter.newline();
    else
        _imp->need_space = true;
}

void
DepSpecPrettyPrinter::visit_sequence(const UseDepSpec & a,
        GenericSpecTree::ConstSequenceIterator cur,
        GenericSpecTree::ConstSequenceIterator end)
{
    Save<bool> old_outer(&_imp->outer_block, false);

    if (_imp->use_newlines)
        _imp->s << _imp->formatter.indent(_imp->indent);
    else if (_imp->need_space)
        _imp->s << " ";

    if (_imp->env && _imp->id)
    {
        if (_imp->env->query_use(a.flag(), *_imp->id))
            _imp->s << _imp->formatter.format(a, format::Enabled()) << " (";
        else
            _imp->s << _imp->formatter.format(a, format::Disabled()) << " (";
    }
    else
        _imp->s << _imp->formatter.format(a, format::Plain()) << " (";

    if (_imp->use_newlines)
        _imp->s << _imp->formatter.newline();
    else
        _imp->need_space = true;

    {
        Save<unsigned> old_indent(&_imp->indent, _imp->indent + 1);
        Save<bool> extra_label_indent(&_imp->extra_label_indent, false);
        std::for_each(cur, end, accept_visitor(*this));
    }

    if (_imp->use_newlines)
        _imp->s << _imp->formatter.indent(_imp->indent);
    else if (_imp->need_space)
        _imp->s << " ";
    _imp->s << ")";
    if (_imp->use_newlines)
        _imp->s << _imp->formatter.newline();
    else
        _imp->need_space = true;
}

void
DepSpecPrettyPrinter::visit_leaf(const PackageDepSpec & p)
{
    if (_imp->use_newlines)
        _imp->s << _imp->formatter.indent(_imp->indent);
    else if (_imp->need_space)
        _imp->s << " ";

    if (_imp->env)
    {
        if (! _imp->env->package_database()->query(query::Matches(p) &
                    query::InstalledAtRoot(_imp->env->root()), qo_whatever)->empty())
            _imp->s << _imp->formatter.format(p, format::Installed());
        else if (! _imp->env->package_database()->query(query::Matches(p) &
                    query::SupportsAction<InstallAction>() & query::NotMasked(), qo_whatever)->empty())
            _imp->s << _imp->formatter.format(p, format::Installable());
        else
            _imp->s << _imp->formatter.format(p, format::Plain());
    }
    else
        _imp->s << _imp->formatter.format(p, format::Plain());

    if (_imp->use_newlines)
        _imp->s << _imp->formatter.newline();
    else
        _imp->need_space = true;
}

void
DepSpecPrettyPrinter::visit_leaf(const PlainTextDepSpec & p)
{
    if (_imp->use_newlines)
        _imp->s << _imp->formatter.indent(_imp->indent);
    else if (_imp->need_space)
        _imp->s << " ";

    _imp->s << _imp->formatter.format(p, format::Plain());

    if (_imp->use_newlines)
        _imp->s << _imp->formatter.newline();
    else
        _imp->need_space = true;
}

void
DepSpecPrettyPrinter::visit_leaf(const LicenseDepSpec & p)
{
    if (_imp->use_newlines)
        _imp->s << _imp->formatter.indent(_imp->indent);
    else if (_imp->need_space)
        _imp->s << " ";

    if (_imp->env && _imp->id)
    {
        if (_imp->env->accept_license(p.text(), *_imp->id))
            _imp->s << _imp->formatter.format(p, format::Accepted());
        else
            _imp->s << _imp->formatter.format(p, format::Unaccepted());
    }
    else
        _imp->s << _imp->formatter.format(p, format::Plain());

    if (_imp->use_newlines)
        _imp->s << _imp->formatter.newline();
    else
        _imp->need_space = true;
}

void
DepSpecPrettyPrinter::visit_leaf(const FetchableURIDepSpec & p)
{
    if (_imp->use_newlines)
        _imp->s << _imp->formatter.indent(_imp->indent);
    else if (_imp->need_space)
        _imp->s << " ";

    _imp->s << _imp->formatter.format(p, format::Plain());

    if (_imp->use_newlines)
        _imp->s << _imp->formatter.newline();
    else
        _imp->need_space = true;
}

void
DepSpecPrettyPrinter::visit_leaf(const SimpleURIDepSpec & p)
{
    if (_imp->use_newlines)
        _imp->s << _imp->formatter.indent(_imp->indent);
    else if (_imp->need_space)
        _imp->s << " ";

    _imp->s << _imp->formatter.format(p, format::Plain());

    if (_imp->use_newlines)
        _imp->s << _imp->formatter.newline();
    else
        _imp->need_space = true;
}

void
DepSpecPrettyPrinter::visit_leaf(const BlockDepSpec & b)
{
    if (_imp->use_newlines)
        _imp->s << _imp->formatter.indent(_imp->indent);
    else if (_imp->need_space)
        _imp->s << " ";

    _imp->s << _imp->formatter.format(b, format::Plain());

    if (_imp->use_newlines)
        _imp->s << _imp->formatter.newline();
    else
        _imp->need_space = true;
}

void
DepSpecPrettyPrinter::visit_leaf(const LabelsDepSpec<URILabelVisitorTypes> & l)
{
    if (_imp->extra_label_indent)
    {
        _imp->extra_label_indent = false;
        _imp->indent -= 1;
    }

    if (_imp->use_newlines)
        _imp->s << _imp->formatter.indent(_imp->indent);
    else if (_imp->need_space)
        _imp->s << " ";

    _imp->s << _imp->formatter.format(l, format::Plain());

    if (_imp->use_newlines)
        _imp->s << _imp->formatter.newline();
    else
        _imp->need_space = true;

    if (! _imp->extra_label_indent)
    {
        _imp->extra_label_indent = true;
        _imp->indent += 1;
    }
}

void
DepSpecPrettyPrinter::visit_leaf(const DependencyLabelDepSpec & l)
{
    if (_imp->extra_label_indent)
    {
        _imp->extra_label_indent = false;
        _imp->indent -= 1;
    }

    if (_imp->use_newlines)
        _imp->s << _imp->formatter.indent(_imp->indent);
    else if (_imp->need_space)
        _imp->s << " ";

    _imp->s << _imp->formatter.format(l, format::Plain());

    if (_imp->use_newlines)
        _imp->s << _imp->formatter.newline();
    else
        _imp->need_space = true;

    if (!_imp->extra_label_indent)
    {
        _imp->extra_label_indent = true;
        _imp->indent += 1;
    }
}

