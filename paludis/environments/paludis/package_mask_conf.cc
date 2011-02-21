/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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
#include <vector>
#include <list>
#include <set>
#include <algorithm>
#include <functional>

using namespace paludis;
using namespace paludis::paludis_environment;

typedef std::list<std::pair<SetName, std::pair<std::shared_ptr<const SetSpecTree>, std::set<std::string> > > > Sets;

namespace paludis
{
    template<>
    struct Imp<PackageMaskConf>
    {
        const PaludisEnvironment * const env;
        const bool allow_reasons;
        std::list<std::pair<std::shared_ptr<const PackageDepSpec>, std::set<std::string> > > masks;
        mutable Sets sets;
        mutable Mutex set_mutex;

        Imp(const PaludisEnvironment * const e, const bool a) :
            env(e),
            allow_reasons(a)
        {
        }
    };
}

PackageMaskConf::PackageMaskConf(const PaludisEnvironment * const e, const bool a) :
    _imp(e, a)
{
}

PackageMaskConf::~PackageMaskConf()
{
}

void
PackageMaskConf::add(const FSPath & filename)
{
    Context context("When adding source '" + stringify(filename) + "' as a package mask or unmask file:");

    std::shared_ptr<LineConfigFile> f(make_bashable_conf(filename, { }));
    if (! f)
        return;

    for (LineConfigFile::ConstIterator line(f->begin()), line_end(f->end()) ;
            line != line_end ; ++line)
    {
        std::vector<std::string> tokens;
        tokenise_whitespace(*line, std::back_inserter(tokens));

        if (tokens.empty())
            continue;

        std::string spec(tokens.at(0));
        tokens.erase(tokens.begin(), tokens.begin() + 1);

        std::set<std::string> reasons(tokens.begin(), tokens.end());
        if ((! reasons.empty()) && (! _imp->allow_reasons))
            throw ConfigurationError("Only one token per line is allowed in '" + stringify(filename) + "'");

        try
        {
            _imp->masks.push_back(std::make_pair(std::shared_ptr<PackageDepSpec>(std::make_shared<PackageDepSpec>(parse_user_package_dep_spec(
                                    spec, _imp->env,
                                    { updso_allow_wildcards, updso_no_disambiguation, updso_throw_if_set }))), reasons));
        }
        catch (const GotASetNotAPackageDepSpec &)
        {
            _imp->sets.push_back(std::make_pair(SetName(*line), std::make_pair(make_null_shared_ptr(), reasons)));
        }
    }
}

bool
PackageMaskConf::query(const std::shared_ptr<const PackageID> & e, const std::string & r) const
{
    using namespace std::placeholders;

    for (auto i(_imp->masks.begin()), i_end(_imp->masks.end()) ;
            i != i_end ; ++i)
        if (match_package(*_imp->env, *i->first, e, make_null_shared_ptr(), MatchPackageOptions()))
        {
            if (r.empty())
            {
                if (i->second.empty())
                    return true;
            }
            else
            {
                if (i->second.empty() || (i->second.end() != i->second.find(r)))
                    return true;
            }
        }

    {
        Lock lock(_imp->set_mutex);

        for (Sets::iterator it(_imp->sets.begin()),
                 it_end(_imp->sets.end()); it_end != it; ++it)
        {
            if (! it->second.first)
            {
                it->second.first = _imp->env->set(it->first);
                if (! it->second.first)
                {
                    Log::get_instance()->message("paludis_environment.package_mask.unknown_set", ll_warning, lc_no_context) << "Set name '"
                        << it->first << "' does not exist";
                    it->second.first = std::make_shared<SetSpecTree>(std::make_shared<AllDepSpec>());
                }
            }

            if (match_package_in_set(*_imp->env, *it->second.first, e, { }))
            {
                if (r.empty())
                {
                    if (it->second.second.empty())
                        return true;
                }
                else
                {
                    if (it->second.second.empty() || (it->second.second.end() != it->second.second.find(r)))
                        return true;
                }
            }
        }
    }

    return false;
}

