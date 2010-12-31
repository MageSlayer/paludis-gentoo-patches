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

#include <paludis/repositories/cran/keys.hh>
#include <paludis/repositories/cran/cran_package_id.hh>
#include <paludis/repositories/cran/cran_dep_parser.hh>
#include <paludis/repositories/cran/spec_tree_pretty_printer.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/join.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/set.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/dep_spec.hh>
#include <paludis/action.hh>
#include <paludis/repository.hh>
#include <paludis/environment.hh>
#include <paludis/package_database.hh>
#include <paludis/call_pretty_printer.hh>
#include <functional>

using namespace paludis;
using namespace paludis::cranrepository;

PackageIDSequenceKey::PackageIDSequenceKey(const Environment * const e,
        const std::string & r, const std::string & h, const MetadataKeyType t) :
    _env(e),
    _v(std::make_shared<PackageIDSequence>()),
    _r(r),
    _h(h),
    _t(t)
{
}

const std::shared_ptr<const PackageIDSequence>
PackageIDSequenceKey::value() const
{
    return _v;
}

const std::string
PackageIDSequenceKey::raw_name() const
{
    return _r;
}

const std::string
PackageIDSequenceKey::human_name() const
{
    return _h;
}

MetadataKeyType
PackageIDSequenceKey::type() const
{
    return _t;
}

void
PackageIDSequenceKey::push_back(const std::shared_ptr<const PackageID> & i)
{
    _v->push_back(i);
}

const std::string
PackageIDSequenceKey::pretty_print_value(
        const PrettyPrinter & p, const PrettyPrintOptions &) const
{
    using namespace std::placeholders;
    return join(value()->begin(), value()->end(), " ", CallPrettyPrinter(p));
}

PackageIDKey::PackageIDKey(const Environment * const e, const std::string & r, const std::string & h,
        const CRANPackageID * const v, const MetadataKeyType t) :
    _env(e),
    _v(v),
    _r(r),
    _h(h),
    _t(t)
{
}

const std::shared_ptr<const PackageID>
PackageIDKey::value() const
{
    return _v->shared_from_this();
}

const std::string
PackageIDKey::pretty_print_value(const PrettyPrinter & p, const PrettyPrintOptions &) const
{
    return p.prettify(value());
}

const std::string
PackageIDKey::raw_name() const
{
    return _r;
}

const std::string
PackageIDKey::human_name() const
{
    return _h;
}

MetadataKeyType
PackageIDKey::type() const
{
    return _t;
}

namespace paludis
{
    template <>
    struct Imp<DepKey>
    {
        const Environment * const env;
        const std::string v;

        mutable Mutex mutex;
        mutable std::shared_ptr<const DependencySpecTree> c;
        const std::shared_ptr<const DependenciesLabelSequence> labels;

        const std::string raw_name;
        const std::string human_name;
        const MetadataKeyType type;

        Imp(const Environment * const e, const std::string & vv,
                const std::shared_ptr<const DependenciesLabelSequence> & s,
                const std::string & r, const std::string & h, const MetadataKeyType & t) :
            env(e),
            v(vv),
            labels(s),
            raw_name(r),
            human_name(h),
            type(t)
        {
        }
    };
}

DepKey::DepKey(const Environment * const e, const std::string & r, const std::string & h, const std::string & v,
        const std::shared_ptr<const DependenciesLabelSequence> & s, const MetadataKeyType t) :
    Pimp<DepKey>(e, v, s, r, h, t),
    _imp(Pimp<DepKey>::_imp)
{
}

DepKey::~DepKey()
{
}

const std::string
DepKey::raw_name() const
{
    return _imp->raw_name;
}

const std::string
DepKey::human_name() const
{
    return _imp->human_name;
}

MetadataKeyType
DepKey::type() const
{
    return _imp->type;
}

const std::shared_ptr<const DependencySpecTree>
DepKey::value() const
{
    Lock l(_imp->mutex);
    if (_imp->c)
        return _imp->c;

    Context context("When parsing CRAN dependency string:");
    _imp->c = parse_depends(_imp->v);
    return _imp->c;
}

const std::string
DepKey::pretty_print_value(
        const PrettyPrinter & printer, const PrettyPrintOptions & options) const
{
    SpecTreePrettyPrinter p(printer, options);
    value()->top()->accept(p);
    return stringify(p);
}

const std::shared_ptr<const DependenciesLabelSequence>
DepKey::initial_labels() const
{
    return _imp->labels;
}

