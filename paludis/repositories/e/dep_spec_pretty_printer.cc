/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008 Ciaran McCreesh
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

#include <paludis/repositories/e/dep_spec_pretty_printer.hh>
#include <paludis/dep_spec.hh>
#include <paludis/metadata_key.hh>
#include <paludis/formatter.hh>
#include <paludis/util/save.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/visitor_cast.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/set.hh>
#include <paludis/environment.hh>
#include <paludis/selection.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/action-fwd.hh>
#include <algorithm>
#include <sstream>

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
        const std::tr1::shared_ptr<const PackageID> id;
        GenericSpecTree::ItemFormatter formatter;
        unsigned indent;
        bool extra_label_indent;
        bool use_newlines;
        bool outer_block;
        bool all_needs_parens;
        bool need_space;
        bool check_conditions;

        Implementation(
                const Environment * const e,
                const std::tr1::shared_ptr<const PackageID> & i,
                const GenericSpecTree::ItemFormatter & f,
                unsigned in,
                bool b,
                bool c) :
            env(e),
            id(i),
            formatter(f),
            indent(in),
            extra_label_indent(false),
            use_newlines(b),
            outer_block(true),
            all_needs_parens(false),
            need_space(false),
            check_conditions(c)
        {
        }
    };
}

DepSpecPrettyPrinter::DepSpecPrettyPrinter(
        const Environment * const e,
        const std::tr1::shared_ptr<const PackageID> & id,
        const GenericSpecTree::ItemFormatter & f,
        unsigned i,
        bool b,
        bool c) :
    PrivateImplementationPattern<DepSpecPrettyPrinter>(new Implementation<DepSpecPrettyPrinter>(e, id, f, i, b, c))
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

namespace
{
    struct IsLabelVisitor :
        ConstVisitor<GenericSpecTree>
    {
        bool result;

        IsLabelVisitor() :
            result(false)
        {
        }

        void visit_leaf(const PlainTextDepSpec &)
        {
        }

        void visit_leaf(const SimpleURIDepSpec &)
        {
        }

        void visit_leaf(const FetchableURIDepSpec &)
        {
        }

        void visit_leaf(const LicenseDepSpec &)
        {
        }

        void visit_leaf(const PackageDepSpec &)
        {
        }

        void visit_leaf(const BlockDepSpec &)
        {
        }

        void visit_leaf(const PlainTextLabelDepSpec &)
        {
            result = true;
        }

        void visit_leaf(const URILabelsDepSpec &)
        {
            result = true;
        }

        void visit_leaf(const DependencyLabelsDepSpec &)
        {
            result = true;
        }

        void visit_leaf(const NamedSetDepSpec &)
        {
        }

        void visit_sequence(const AllDepSpec &, GenericSpecTree::ConstSequenceIterator, GenericSpecTree::ConstSequenceIterator)
        {
        }

        void visit_sequence(const AnyDepSpec &, GenericSpecTree::ConstSequenceIterator, GenericSpecTree::ConstSequenceIterator)
        {
        }

        void visit_sequence(const ConditionalDepSpec &, GenericSpecTree::ConstSequenceIterator, GenericSpecTree::ConstSequenceIterator)
        {
        }
    };

    bool is_label(const ConstAcceptInterface<GenericSpecTree> & i)
    {
        IsLabelVisitor v;
        i.accept(v);
        return v.result;
    }
}

void
DepSpecPrettyPrinter::visit_sequence(const AllDepSpec & a,
        GenericSpecTree::ConstSequenceIterator cur,
        GenericSpecTree::ConstSequenceIterator end)
{
    bool need_parens(_imp->all_needs_parens ||
            (! _imp->outer_block && end != std::find_if(cur, end, is_label)));
    Save<bool> old_outer(&_imp->outer_block, false);
    Save<bool> old_needs_parens(&_imp->all_needs_parens, false);

    if (need_parens)
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
        Save<unsigned> old_indent(&_imp->indent, need_parens ? _imp->indent +1 : _imp->indent);
        Save<bool> extra_label_indent(&_imp->extra_label_indent, need_parens ? false : _imp->extra_label_indent);
        std::for_each(cur, end, accept_visitor(*this));
    }

    if (need_parens)
    {
        if (_imp->use_newlines)
            _imp->s << _imp->formatter.indent(_imp->indent);
        else if (_imp->need_space)
            _imp->s << " ";
        _imp->s << ")";

        do_annotations(a);

        if (_imp->use_newlines)
            _imp->s << _imp->formatter.newline();
        else
            _imp->need_space = true;
    }
}

void
DepSpecPrettyPrinter::visit_sequence(const AnyDepSpec & a,
        GenericSpecTree::ConstSequenceIterator cur,
        GenericSpecTree::ConstSequenceIterator end)
{
    Save<bool> old_outer(&_imp->outer_block, false);
    Save<bool> old_needs_parens(&_imp->all_needs_parens, true);

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

    do_annotations(a);

    if (_imp->use_newlines)
        _imp->s << _imp->formatter.newline();
    else
        _imp->need_space = true;
}

void
DepSpecPrettyPrinter::visit_sequence(const ConditionalDepSpec & a,
        GenericSpecTree::ConstSequenceIterator cur,
        GenericSpecTree::ConstSequenceIterator end)
{
    Save<bool> old_outer(&_imp->outer_block, false);
    Save<bool> old_needs_parens(&_imp->all_needs_parens, false);

    if (_imp->use_newlines)
        _imp->s << _imp->formatter.indent(_imp->indent);
    else if (_imp->need_space)
        _imp->s << " ";

    if (! _imp->check_conditions)
        _imp->s << _imp->formatter.format(a, format::Plain()) << " (";
    else if (a.condition_met())
        _imp->s << _imp->formatter.format(a, format::Enabled()) << " (";
    else
        _imp->s << _imp->formatter.format(a, format::Disabled()) << " (";

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

    do_annotations(a);

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

    if (_imp->env && _imp->check_conditions)
    {
        if (! (*_imp->env)[selection::SomeArbitraryVersion(generator::Matches(p) |
                    filter::InstalledAtRoot(_imp->env->root()))]->empty())
            _imp->s << _imp->formatter.format(p, format::Installed());
        else if (! (*_imp->env)[selection::SomeArbitraryVersion(generator::Matches(p) |
                    filter::SupportsAction<InstallAction>() | filter::NotMasked())]->empty())
            _imp->s << _imp->formatter.format(p, format::Installable());
        else
            _imp->s << _imp->formatter.format(p, format::Plain());
    }
    else
        _imp->s << _imp->formatter.format(p, format::Plain());

    do_annotations(p);

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

    do_annotations(p);

    if (_imp->use_newlines)
        _imp->s << _imp->formatter.newline();
    else
        _imp->need_space = true;
}

void
DepSpecPrettyPrinter::visit_leaf(const NamedSetDepSpec & p)
{
    if (_imp->use_newlines)
        _imp->s << _imp->formatter.indent(_imp->indent);
    else if (_imp->need_space)
        _imp->s << " ";

    _imp->s << _imp->formatter.format(p, format::Plain());

    do_annotations(p);

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

    if (_imp->env && _imp->id && _imp->check_conditions)
    {
        if (_imp->env->accept_license(p.text(), *_imp->id))
            _imp->s << _imp->formatter.format(p, format::Accepted());
        else
            _imp->s << _imp->formatter.format(p, format::Unaccepted());
    }
    else
        _imp->s << _imp->formatter.format(p, format::Plain());

    do_annotations(p);

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

    do_annotations(p);

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

    do_annotations(p);

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

    do_annotations(b);

    if (_imp->use_newlines)
        _imp->s << _imp->formatter.newline();
    else
        _imp->need_space = true;
}

void
DepSpecPrettyPrinter::visit_leaf(const URILabelsDepSpec & l)
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

    do_annotations(l);

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
DepSpecPrettyPrinter::visit_leaf(const PlainTextLabelDepSpec & l)
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

    do_annotations(l);

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
DepSpecPrettyPrinter::visit_leaf(const DependencyLabelsDepSpec & l)
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

    do_annotations(l);

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

void
DepSpecPrettyPrinter::do_annotations(const DepSpec & p)
{
    if (p.annotations_key() && (p.annotations_key()->begin_metadata() != p.annotations_key()->end_metadata()))
    {
        _imp->s << " [[ ";
        for (MetadataSectionKey::MetadataConstIterator k(p.annotations_key()->begin_metadata()), k_end(p.annotations_key()->end_metadata()) ;
                k != k_end ; ++k)
        {
            const MetadataValueKey<std::string> * r(visitor_cast<const MetadataValueKey<std::string> >(**k));
            if (! r)
                throw InternalError(PALUDIS_HERE, "annotations must be string keys");
            _imp->s << (*k)->raw_name() << " = [" << (r->value().empty() ? " " : " " + r->value() + " ") << "] ";
        }
        _imp->s << "]]";
    }
}

