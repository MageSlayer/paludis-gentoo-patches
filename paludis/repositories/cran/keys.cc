/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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
#include <paludis/dep_spec.hh>

using namespace paludis;
using namespace paludis::cranrepository;

URIKey::URIKey(const std::string & r, const std::string & h, const std::string & v, const MetadataKeyType t) :
    MetadataSpecTreeKey<URISpecTree>(r, h, t),
    _v(v)
{
}

const tr1::shared_ptr<const URISpecTree::ConstItem>
URIKey::value() const
{
    return make_shared_ptr(new TreeLeaf<URISpecTree, URIDepSpec>(make_shared_ptr(new URIDepSpec(_v))));
}

std::string
URIKey::pretty_print() const
{
    return _v;
}

std::string
URIKey::pretty_print_flat() const
{
    return _v;
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

PackageIDSequenceKey::PackageIDSequenceKey(const std::string & r, const std::string & h, const MetadataKeyType t) :
    MetadataSetKey<PackageIDSequence>(r, h, t),
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

DepKey::DepKey(const std::string & r, const std::string & h, const std::string & v,
        const MetadataKeyType t) :
    MetadataSpecTreeKey<DependencySpecTree>(r, h, t),
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
DepKey::pretty_print() const
{
    DepSpecPrettyPrinter p(12, true);
    value()->accept(p);
    return stringify(p);
}

std::string
DepKey::pretty_print_flat() const
{
    DepSpecPrettyPrinter p(0, false);
    value()->accept(p);
    return stringify(p);
}

