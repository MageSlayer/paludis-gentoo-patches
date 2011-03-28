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

#include <paludis/repositories/e/traditional_mask_store.hh>
#include <paludis/repositories/e/profile_file.hh>
#include <paludis/repositories/e/traditional_mask_file.hh>
#include <paludis/repositories/e/eapi.hh>

#include <paludis/util/pimp-impl.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/log.hh>
#include <paludis/util/make_null_shared_ptr.hh>

#include <paludis/dep_spec.hh>
#include <paludis/package_id.hh>
#include <paludis/match_package.hh>
#include <paludis/package_dep_spec_constraint.hh>

#include <algorithm>
#include <unordered_map>
#include <list>

using namespace paludis;
using namespace paludis::erepository;

typedef std::unordered_map<QualifiedPackageName,
        std::list<std::pair<PackageDepSpec, std::shared_ptr<const MaskInfo> > >,
        Hash<QualifiedPackageName> > RepositoryMaskMap;

namespace paludis
{
    template <>
    struct Imp<TraditionalMaskStore>
    {
        const Environment * const env;
        RepositoryName repo_name;
        const std::shared_ptr<const FSPathSequence> files;
        EAPIForFileFunction eapi_for_file;

        RepositoryMaskMap repo_mask;

        Imp(const Environment * const e, const RepositoryName & r, const std::shared_ptr<const FSPathSequence> & f, const EAPIForFileFunction & n) :
            env(e),
            repo_name(r),
            files(f),
            eapi_for_file(n)
        {
        }
    };
}

TraditionalMaskStore::TraditionalMaskStore(
        const Environment * const e,
        const RepositoryName & r,
        const std::shared_ptr<const FSPathSequence> & f,
        const EAPIForFileFunction & n) :
    _imp(e, r, f, n)
{
    _populate();
}

TraditionalMaskStore::~TraditionalMaskStore()
{
}

void
TraditionalMaskStore::_populate()
{
    Context context("When loading repository masks for '" + stringify(_imp->repo_name) + "':");

    using namespace std::placeholders;

    ProfileFile<TraditionalMaskFile> repository_mask_file(_imp->eapi_for_file);
    std::for_each(_imp->files->begin(), _imp->files->end(),
            std::bind(&ProfileFile<TraditionalMaskFile>::add_file, std::ref(repository_mask_file), _1));

    for (ProfileFile<TraditionalMaskFile>::ConstIterator
            line(repository_mask_file.begin()), line_end(repository_mask_file.end()) ;
            line != line_end ; ++line)
    {
        try
        {
            auto a(parse_elike_package_dep_spec(
                        line->second.first, line->first->supported()->package_dep_spec_parse_options(),
                        line->first->supported()->version_spec_options()));
            if (a.package_name_constraint())
                _imp->repo_mask[a.package_name_constraint()->name()].push_back(std::make_pair(a, line->second.second));
            else
                Log::get_instance()->message("e.package_mask.bad_spec", ll_warning, lc_context)
                    << "Loading package mask spec '" << line->second.first << "' failed because specification does not restrict to a "
                    "unique package";
        }
        catch (const InternalError &)
        {
            throw;
        }
        catch (const Exception & e)
        {
            Log::get_instance()->message("e.package_mask.bad_spec", ll_warning, lc_context) << "Loading package mask spec '"
                << line->second.first << "' failed due to exception '" << e.message() << "' ("
                << e.what() << ")";
        }
    }
}

const std::shared_ptr<const MasksInfo>
TraditionalMaskStore::query(const std::shared_ptr<const PackageID> & id) const
{
    auto result(std::make_shared<MasksInfo>());
    auto r(_imp->repo_mask.find(id->name()));
    if (_imp->repo_mask.end() != r)
        for (auto k(r->second.begin()), k_end(r->second.end()) ; k != k_end ; ++k)
            if (match_package(*_imp->env, k->first, id, make_null_shared_ptr(), { }))
                result->push_back(*k->second);

    return result;
}

