/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2011 Ciaran McCreesh
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

#include <paludis/resolver/make_uninstall_blocker.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/dep_spec_annotations.hh>
#include <paludis/dep_spec.hh>

using namespace paludis;
using namespace paludis::resolver;

BlockDepSpec
paludis::resolver::make_uninstall_blocker(const PackageDepSpec & spec)
{
    BlockDepSpec result("!?" + stringify(spec), spec);
    auto annotations(std::make_shared<DepSpecAnnotations>());
    annotations->add(make_named_values<DepSpecAnnotation>(
                n::key() = "<resolution>",
                n::kind() = dsak_synthetic,
                n::role() = dsar_blocker_weak,
                n::value() = "<implicit-weak>"
                ));
    result.set_annotations(annotations);
    return result;
}

