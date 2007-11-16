/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh
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

#include <paludis/repositories/unpackaged/unpackaged_key.hh>
#include <paludis/repositories/unpackaged/dep_printer.hh>
#include <paludis/repositories/unpackaged/dep_parser.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/fs_entry.hh>

using namespace paludis;
using namespace paludis::unpackaged_repositories;

namespace paludis
{
    template <>
    struct Implementation<UnpackagedDependencyKey>
    {
        const Environment * const env;
        const tr1::shared_ptr<const DependencySpecTree::ConstItem> value;

        Implementation(const Environment * const e, const std::string & v) :
            env(e),
            value(DepParser::parse(v))
        {
        }
    };
}

UnpackagedDependencyKey::UnpackagedDependencyKey(const Environment * const env,
        const std::string & r, const std::string & h, const MetadataKeyType t,
        const std::string & v) :
    MetadataSpecTreeKey<DependencySpecTree>(r, h, t),
    PrivateImplementationPattern<UnpackagedDependencyKey>(new Implementation<UnpackagedDependencyKey>(env, v)),
    _imp(PrivateImplementationPattern<UnpackagedDependencyKey>::_imp)
{
}

UnpackagedDependencyKey::~UnpackagedDependencyKey()
{
}

const tr1::shared_ptr<const DependencySpecTree::ConstItem>
UnpackagedDependencyKey::value() const
{
    return _imp->value;
}

std::string
UnpackagedDependencyKey::pretty_print(const DependencySpecTree::ItemFormatter & f) const
{
    DepPrinter p(_imp->env, f, false);
    _imp->value->accept(p);
    return p.result();
}

std::string
UnpackagedDependencyKey::pretty_print_flat(const DependencySpecTree::ItemFormatter & f) const
{
    DepPrinter p(_imp->env, f, true);
    _imp->value->accept(p);
    return p.result();
}

