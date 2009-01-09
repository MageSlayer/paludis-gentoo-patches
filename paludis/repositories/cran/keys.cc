/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009 Ciaran McCreesh
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
#include <paludis/repositories/cran/dep_spec_pretty_printer.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/join.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/set.hh>
#include <paludis/dep_spec.hh>
#include <paludis/stringify_formatter-impl.hh>
#include <paludis/formatter.hh>
#include <paludis/action.hh>
#include <tr1/functional>

using namespace paludis;
using namespace paludis::cranrepository;

PackageIDSequenceKey::PackageIDSequenceKey(const Environment * const e,
        const std::string & r, const std::string & h, const MetadataKeyType t) :
    MetadataCollectionKey<PackageIDSequence>(r, h, t),
    _env(e),
    _v(new PackageIDSequence)
{
}

const std::tr1::shared_ptr<const PackageIDSequence>
PackageIDSequenceKey::value() const
{
    return _v;
}

void
PackageIDSequenceKey::push_back(const std::tr1::shared_ptr<const PackageID> & i)
{
    _v->push_back(i);
}

std::string
PackageIDSequenceKey::pretty_print_flat(const Formatter<PackageID> & f) const
{
    using namespace std::tr1::placeholders;
    return join(indirect_iterator(value()->begin()), indirect_iterator(value()->end()), " ",
            std::tr1::bind(static_cast<std::string (Formatter<PackageID>::*)(const PackageID &, const format::Plain &) const>(
                    &Formatter<PackageID>::format),
                std::tr1::cref(f), _1, format::Plain()));
}

PackageIDKey::PackageIDKey(const std::string & r, const std::string & h,
        const CRANPackageID * const v, const MetadataKeyType t) :
    MetadataValueKey<std::tr1::shared_ptr<const PackageID> >(r, h, t),
    _v(v)
{
}

const std::tr1::shared_ptr<const PackageID>
PackageIDKey::value() const
{
    return _v->shared_from_this();
}

std::string
PackageIDKey::pretty_print(const Formatter<PackageID> & f) const
{
    if (_v->supports_action(SupportsActionTest<InstalledAction>()))
        return f.format(*_v, format::Installed());
    else if (_v->supports_action(SupportsActionTest<InstallAction>()))
    {
        if (_v->masked())
            return f.format(*_v, format::Plain());
        else
            return f.format(*_v, format::Installable());
    }
    else
        return f.format(*_v, format::Plain());
}

namespace paludis
{
    template <>
    struct Implementation<DepKey>
    {
        const Environment * const env;
        const std::string v;

        mutable Mutex mutex;
        mutable std::tr1::shared_ptr<const DependencySpecTree> c;
        const std::tr1::shared_ptr<const DependencyLabelSequence> labels;

        Implementation(const Environment * const e, const std::string & vv,
                const std::tr1::shared_ptr<const DependencyLabelSequence> & s) :
            env(e),
            v(vv),
            labels(s)
        {
        }
    };
}

DepKey::DepKey(const Environment * const e, const std::string & r, const std::string & h, const std::string & v,
        const std::tr1::shared_ptr<const DependencyLabelSequence> & s, const MetadataKeyType t) :
    MetadataSpecTreeKey<DependencySpecTree>(r, h, t),
    PrivateImplementationPattern<DepKey>(new Implementation<DepKey>(e, v, s)),
    _imp(PrivateImplementationPattern<DepKey>::_imp)
{
}

DepKey::~DepKey()
{
}

const std::tr1::shared_ptr<const DependencySpecTree>
DepKey::value() const
{
    Lock l(_imp->mutex);
    if (_imp->c)
        return _imp->c;

    Context context("When parsing CRAN dependency string:");
    _imp->c = parse_depends(_imp->v);
    return _imp->c;
}

std::string
DepKey::pretty_print(const DependencySpecTree::ItemFormatter & f) const
{
    StringifyFormatter ff(f);
    DepSpecPrettyPrinter p(_imp->env, ff, 12, true);
    value()->root()->accept(p);
    return stringify(p);
}

std::string
DepKey::pretty_print_flat(const DependencySpecTree::ItemFormatter & f) const
{
    StringifyFormatter ff(f);
    DepSpecPrettyPrinter p(_imp->env, ff, 0, false);
    value()->root()->accept(p);
    return stringify(p);
}

const std::tr1::shared_ptr<const DependencyLabelSequence>
DepKey::initial_labels() const
{
    return _imp->labels;
}

