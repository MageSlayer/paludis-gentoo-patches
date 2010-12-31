/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010 Ciaran McCreesh
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

#include <paludis/repositories/gemcutter/gemcutter_uri_key.hh>
#include <paludis/util/accept_visitor.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/spec_tree.hh>
#include <paludis/dep_spec.hh>
#include <paludis/pretty_printer.hh>
#include <sstream>
#include <algorithm>

using namespace paludis;
using namespace paludis::gemcutter_repository;

namespace
{
    std::shared_ptr<SimpleURISpecTree> parse(const std::string & s)
    {
        auto result(std::make_shared<SimpleURISpecTree>(std::make_shared<AllDepSpec>()));
        result->top()->append(std::make_shared<SimpleURIDepSpec>(s));
        return result;
    }

    struct ValuePrinter
    {
        std::stringstream s;
        const PrettyPrinter & printer;
        const PrettyPrintOptions options;

        const unsigned indent;
        const bool flat;
        bool need_space;

        ValuePrinter(
                const PrettyPrinter & p,
                const PrettyPrintOptions & o) :
            printer(p),
            options(o),
            indent(0),
            flat(! options[ppo_multiline_allowed]),
            need_space(false)
        {
        }

        void visit(const SimpleURISpecTree::NodeType<SimpleURIDepSpec>::Type & node)
        {
            if (! flat)
                s << printer.indentify(indent + 1);
            else if (need_space)
                s << " ";
            else
                need_space = true;

            s << printer.prettify(*node.spec());

            if (! flat)
                s << printer.newline();
        }

        void visit(const SimpleURISpecTree::NodeType<AllDepSpec>::Type & node)
        {
            std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
        }

        void visit(const SimpleURISpecTree::NodeType<ConditionalDepSpec>::Type & node)
        {
            std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
        }
    };
}

namespace paludis
{
    template <>
    struct Imp<GemcutterURIKey>
    {
        const std::string * const raw_name;
        const std::string * const human_name;
        const MetadataKeyType type;
        const std::string value_string;
        const std::shared_ptr<SimpleURISpecTree> value;

        Imp(
                const std::string * const r,
                const std::string * const h,
                const MetadataKeyType t,
                const std::string & v) :
            raw_name(r),
            human_name(h),
            type(t),
            value_string(v),
            value(parse(v))
        {
        }
    };
}

GemcutterURIKey::GemcutterURIKey(const std::string * const r, const std::string * const h,
        const MetadataKeyType t, const std::string & v) :
    Pimp<GemcutterURIKey>(r, h, t, v),
    _imp(Pimp<GemcutterURIKey>::_imp)
{
}

GemcutterURIKey::~GemcutterURIKey() = default;

const std::shared_ptr<const SimpleURISpecTree>
GemcutterURIKey::value() const
{
    return _imp->value;
}

const std::string
GemcutterURIKey::raw_name() const
{
    return *_imp->raw_name;
}

const std::string
GemcutterURIKey::human_name() const
{
    return *_imp->human_name;
}

MetadataKeyType
GemcutterURIKey::type() const
{
    return _imp->type;
}

const std::string
GemcutterURIKey::pretty_print_value(
        const PrettyPrinter & printer, const PrettyPrintOptions & options) const
{
    ValuePrinter p{printer, options};
    _imp->value->top()->accept(p);
    return p.s.str();
}

template class Pimp<GemcutterURIKey>;

