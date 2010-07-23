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

#include "package_mask_conf.hh"
#include <paludis/environment.hh>
#include <paludis/name.hh>
#include <paludis/dep_spec.hh>
#include <paludis/spec_tree.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/match_package.hh>
#include <paludis/util/config_file.hh>
#include <paludis/package_id.hh>
#include <paludis/environments/paludis/paludis_environment.hh>
#include <paludis/environments/paludis/bashable_conf.hh>
#include <paludis/util/log.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/options.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <list>
#include <algorithm>
#include <functional>

using namespace paludis;
using namespace paludis::paludis_environment;

typedef std::list<std::pair<SetName, std::shared_ptr<const SetSpecTree> > > Sets;

namespace paludis
{
    template<>
    struct Imp<PackageMaskConf>
    {
        const PaludisEnvironment * const env;
        std::list<std::shared_ptr<const PackageDepSpec> > masks;
        mutable Sets sets;
        mutable Mutex set_mutex;

        Imp(const PaludisEnvironment * const e) :
            env(e)
        {
        }
    };
}

PackageMaskConf::PackageMaskConf(const PaludisEnvironment * const e) :
    Pimp<PackageMaskConf>(e)
{
}

PackageMaskConf::~PackageMaskConf()
{
}

void
PackageMaskConf::add(const FSEntry & filename)
{
    Context context("When adding source '" + stringify(filename) + "' as a package mask or unmask file:");

    std::shared_ptr<LineConfigFile> f(make_bashable_conf(filename, LineConfigFileOptions()));
    if (! f)
        return;

    for (LineConfigFile::ConstIterator line(f->begin()), line_end(f->end()) ;
            line != line_end ; ++line)
    {
        try
        {
            _imp->masks.push_back(std::shared_ptr<PackageDepSpec>(std::make_shared<PackageDepSpec>(parse_user_package_dep_spec(
                                *line, _imp->env,
                                UserPackageDepSpecOptions() + updso_allow_wildcards + updso_no_disambiguation + updso_throw_if_set))));
        }
        catch (const GotASetNotAPackageDepSpec &)
        {
            _imp->sets.push_back(std::make_pair(SetName(*line), make_null_shared_ptr()));
        }
    }
}

bool
PackageMaskConf::query(const PackageID & e) const
{
    using namespace std::placeholders;
    if (indirect_iterator(_imp->masks.end()) != std::find_if(
            indirect_iterator(_imp->masks.begin()),
            indirect_iterator(_imp->masks.end()),
            std::bind(&match_package, std::ref(*_imp->env), _1, std::cref(e), MatchPackageOptions())))
        return true;

    {
        Lock lock(_imp->set_mutex);

        for (Sets::iterator it(_imp->sets.begin()),
                 it_end(_imp->sets.end()); it_end != it; ++it)
        {
            if (! it->second)
            {
                it->second = _imp->env->set(it->first);
                if (! it->second)
                {
                    Log::get_instance()->message("paludis_environment.package_mask.unknown_set", ll_warning, lc_no_context) << "Set name '"
                        << it->first << "' does not exist";
                    it->second = std::make_shared<SetSpecTree>(std::make_shared<AllDepSpec>());
                }
            }

            if (match_package_in_set(*_imp->env, *it->second, e, MatchPackageOptions()))
                return true;
        }
    }

    return false;
}

