/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010, 2011 Ciaran McCreesh
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
#include <paludis/choice.hh>

#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/map-impl.hh>

using namespace paludis;
using namespace paludis::erepository;

Profile::~Profile()
{
}

ProfileFactory::ProfileFactory()
{
}

const std::shared_ptr<Profile>
ProfileFactory::create(
        const std::string & format,
        const Environment * const env,
        const RepositoryName & name,
        const EAPIForFileFunction & eapi_for_file,
        const IsArchFlagFunction & is_arch_flag,
        const FSPathSequence & dirs,
        const std::string & arch_var_if_special,
        const bool profiles_explicitly_set,
        const bool has_master_repositories,
        const bool ignore_deprecated_profiles) const
{
    if (format == "traditional")
        return std::make_shared<TraditionalProfile>(env, name, eapi_for_file, is_arch_flag, dirs, arch_var_if_special, profiles_explicitly_set, has_master_repositories, ignore_deprecated_profiles);
    if (format == "exheres")
        return std::make_shared<ExheresProfile>(env, name, eapi_for_file, is_arch_flag, dirs, arch_var_if_special, profiles_explicitly_set, has_master_repositories, ignore_deprecated_profiles);

    throw ConfigurationError("Unrecognised profile '" + format + "'");
}

template class Map<QualifiedPackageName, PackageDepSpec>;
template class WrappedForwardIterator<Map<QualifiedPackageName, PackageDepSpec>::ConstIteratorTag,
         const std::pair<const QualifiedPackageName, PackageDepSpec> >;

