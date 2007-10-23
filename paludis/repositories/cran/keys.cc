/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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
#include <paludis/util/iterator.hh>
#include <paludis/util/tr1_functional.hh>
#include <paludis/dep_spec.hh>
#include <paludis/stringify_formatter-impl.hh>
#include <paludis/formatter.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>

using namespace paludis;
using namespace paludis::cranrepository;

SimpleURIKey::SimpleURIKey(const std::string & r, const std::string & h, const std::string & v, const MetadataKeyType t) :
    MetadataSpecTreeKey<SimpleURISpecTree>(r, h, t),
    _v(v)
{
}

const tr1::shared_ptr<const SimpleURISpecTree::ConstItem>
SimpleURIKey::value() const
{
    return make_shared_ptr(new TreeLeaf<SimpleURISpecTree, SimpleURIDepSpec>(make_shared_ptr(new SimpleURIDepSpec(_v))));
}

std::string
SimpleURIKey::pretty_print(const SimpleURISpecTree::ItemFormatter & f) const
{
    return f.format(_v, format::Plain());
}

std::string
SimpleURIKey::pretty_print_flat(const SimpleURISpecTree::ItemFormatter & f) const
{
    return f.format(_v, format::Plain());
}

StringKey::StringKey(const std::string & r, const std::string & h, const std::string & v, const MetadataKeyType t) :
    MetadataStringKey(r, h, t),
    _v(v)
{
}

const std::string
StringKey::value() const
{
    return _v;
}

FSLocationKey::FSLocationKey(const std::string & r, const std::string & h,
        const FSEntry & v, const MetadataKeyType t) :
    MetadataFSEntryKey(r, h, t),
    _v(v)
{
}

const FSEntry
FSLocationKey::value() const
{
    return _v;
}

PackageIDSequenceKey::PackageIDSequenceKey(const Environment * const e,
        const std::string & r, const std::string & h, const MetadataKeyType t) :
    MetadataSetKey<PackageIDSequence>(r, h, t),
    _env(e),
    _v(new PackageIDSequence)
{
}

const tr1::shared_ptr<const PackageIDSequence>
PackageIDSequenceKey::value() const
{
    return _v;
}

void
PackageIDSequenceKey::push_back(const tr1::shared_ptr<const PackageID> & i)
{
    _v->push_back(i);
}

std::string
PackageIDSequenceKey::pretty_print_flat(const Formatter<tr1::shared_ptr<const PackageID> > & f) const
{
    using namespace tr1::placeholders;
    return join(value()->begin(), value()->end(), " ", tr1::bind(
                static_cast<std::string (Formatter<tr1::shared_ptr<const PackageID> >::*)(
                    const tr1::shared_ptr<const PackageID> &, const format::Plain &) const>(
                        &Formatter<tr1::shared_ptr<const PackageID> >::format),
                tr1::cref(f), _1, format::Plain()));
}

PackageIDKey::PackageIDKey(const std::string & r, const std::string & h,
        const CRANPackageID * const v, const MetadataKeyType t) :
    MetadataPackageIDKey(r, h, t),
    _v(v)
{
}

const tr1::shared_ptr<const PackageID>
PackageIDKey::value() const
{
    return _v->shared_from_this();
}

DepKey::DepKey(const Environment * const e, const std::string & r, const std::string & h, const std::string & v,
        const MetadataKeyType t) :
    MetadataSpecTreeKey<DependencySpecTree>(r, h, t),
    _env(e),
    _v(v)
{
}

const tr1::shared_ptr<const DependencySpecTree::ConstItem>
DepKey::value() const
{
    Lock l(_m);
    if (_c)
        return _c;

    Context context("When parsing CRAN dependency string:");
    _c = parse_depends(_v);
    return _c;
}

std::string
DepKey::pretty_print(const DependencySpecTree::ItemFormatter & f) const
{
    StringifyFormatter ff(f);
    DepSpecPrettyPrinter p(_env, ff, 12, true);
    value()->accept(p);
    return stringify(p);
}

std::string
DepKey::pretty_print_flat(const DependencySpecTree::ItemFormatter & f) const
{
    StringifyFormatter ff(f);
    DepSpecPrettyPrinter p(_env, ff, 0, false);
    value()->accept(p);
    return stringify(p);
}

