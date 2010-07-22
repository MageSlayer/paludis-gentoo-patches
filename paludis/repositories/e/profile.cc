/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009 Ciaran McCreesh
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

#include <paludis/repositories/e/profile.hh>
#include <paludis/repositories/e/traditional_profile.hh>
#include <paludis/repositories/e/exheres_profile.hh>
#include <paludis/name.hh>
#include <paludis/dep_spec.hh>

#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/map-impl.hh>

using namespace paludis;
using namespace paludis::erepository;

Profile::~Profile()
{
}

namespace
{
    template <typename T_>
    std::shared_ptr<Profile>
    make_profile(
        const Environment * const env,
        const ERepository * const repo,
        const RepositoryName & name,
        const FSEntrySequence & locations,
        const std::string & a,
        const bool x)
    {
        return std::shared_ptr<Profile>(new T_(env, repo, name, locations, a, x));
    }
}

ProfileFactory::ProfileFactory()
{
}

const std::shared_ptr<Profile>
ProfileFactory::create(
        const std::string & s,
        const Environment * const env,
        const ERepository * const repo,
        const RepositoryName & name,
        const FSEntrySequence & locations,
        const std::string & a,
        const bool x) const
{
    if (s == "traditional")
        return make_profile<TraditionalProfile>(env, repo, name, locations, a, x);
    if (s == "exheres")
        return make_profile<ExheresProfile>(env, repo, name, locations, a, x);
    throw ConfigurationError("Unrecognised profile '" + s + "'");
}

template class Map<QualifiedPackageName, PackageDepSpec>;
template class WrappedForwardIterator<Map<QualifiedPackageName, PackageDepSpec>::ConstIteratorTag,
         const std::pair<const QualifiedPackageName, PackageDepSpec> >;

