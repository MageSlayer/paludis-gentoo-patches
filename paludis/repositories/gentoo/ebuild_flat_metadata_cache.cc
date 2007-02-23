/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include "ebuild_flat_metadata_cache.hh"
#include <paludis/util/log.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/join.hh>
#include <fstream>
#include <set>
#include <list>

using namespace paludis;

EbuildFlatMetadataCache::EbuildFlatMetadataCache(const FSEntry & f,
        const FSEntry & e, time_t t, std::tr1::shared_ptr<const EclassMtimes> m, bool s) :
    _filename(f),
    _ebuild(e),
    _master_mtime(t),
    _eclass_mtimes(m),
    _silent(s)
{
}

bool
EbuildFlatMetadataCache::load(std::tr1::shared_ptr<EbuildVersionMetadata> result)
{
    Context context("When loading version metadata to '" + stringify(_filename) + "':");

    std::ifstream cache(stringify(_filename).c_str());
    std::string line;

    if (cache)
    {
        std::getline(cache, line); result->build_depend_string = line;
        std::getline(cache, line); result->run_depend_string = line;
        std::getline(cache, line); result->slot = SlotName(line);
        std::getline(cache, line); result->src_uri = line;
        std::getline(cache, line); result->restrict_string = line;
        std::getline(cache, line); result->homepage = line;
        std::getline(cache, line); result->license_string = line;
        std::getline(cache, line); result->description = line;
        std::getline(cache, line); result->keywords = line;
        std::getline(cache, line); result->inherited = line;
        std::getline(cache, line); result->iuse = line;
        std::getline(cache, line);
        std::getline(cache, line); result->post_depend_string = line;
        std::getline(cache, line); result->provide_string = line;
        std::getline(cache, line); result->eapi = line;

        // check mtimes
        time_t cache_time(std::max(_master_mtime, _filename.mtime()));
        bool ok = true;

        if (_ebuild.mtime() > cache_time)
            ok = false;
        else
        {
            std::set<std::string> inherits;
            WhitespaceTokeniser::get_instance()->tokenise(
                    stringify(result->inherited),
                    std::inserter(inherits, inherits.end()));

            for (std::set<std::string>::const_iterator i(inherits.begin()),
                    i_end(inherits.end()) ; i != i_end ; ++i)
                if (_eclass_mtimes->mtime(*i) > cache_time)
                {
                    ok = false;
                    break;
                }
        }

        if (! ok)
            Log::get_instance()->message(ll_warning, lc_no_context, "Stale cache file at '"
                    + stringify(_filename) + "'");

        return ok;
    }
    else
    {
        Log::get_instance()->message(_silent ? ll_debug : ll_warning, lc_no_context,
                "Couldn't use the cache file at '" + stringify(_filename) + "'");
        return false;
    }
}

namespace
{
    template <typename T_>
    std::string normalise(const T_ & s)
    {
        std::list<std::string> tokens;
        WhitespaceTokeniser::get_instance()->tokenise(stringify(s), std::back_inserter(tokens));
        return join(tokens.begin(), tokens.end(), " ");
    }
}

void
EbuildFlatMetadataCache::save(std::tr1::shared_ptr<const EbuildVersionMetadata> v)
{
    Context context("When saving version metadata to '" + stringify(_filename) + "':");

    try
    {
        _filename.dirname().dirname().mkdir();
        _filename.dirname().mkdir();
    }
    catch (const FSError &e)
    {
        // let the 'if (cache)' handle the error
    }
    std::ofstream cache(stringify(_filename).c_str());

    if (cache)
    {
        cache << normalise(v->build_depend_string) << std::endl;
        cache << normalise(v->run_depend_string) << std::endl;
        cache << normalise(v->slot) << std::endl;
        cache << normalise(v->src_uri) << std::endl;
        cache << normalise(v->restrict_string) << std::endl;
        cache << normalise(v->homepage) << std::endl;
        cache << normalise(v->license_string) << std::endl;
        cache << normalise(v->description) << std::endl;
        cache << normalise(v->keywords) << std::endl;
        cache << normalise(v->inherited) << std::endl;
        cache << normalise(v->iuse) << std::endl;
        cache << std::endl;
        cache << normalise(v->post_depend_string) << std::endl;
        cache << normalise(v->provide_string) << std::endl;
        cache << normalise(v->eapi) << std::endl;
    }
    else
    {
        Log::get_instance()->message(ll_warning, lc_no_context,
                "Couldn't write cache file to '" + stringify(_filename) + "'");
    }

}

