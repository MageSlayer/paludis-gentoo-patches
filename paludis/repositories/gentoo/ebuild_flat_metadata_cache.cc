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
#include <paludis/dep_spec_pretty_printer.hh>
#include <fstream>
#include <set>
#include <list>
#include <tr1/functional>
#include <functional>
#include <algorithm>

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
    using namespace std::tr1::placeholders;

    Context context("When loading version metadata to '" + stringify(_filename) + "':");

    std::ifstream cache(stringify(_filename).c_str());
    std::string line;

    if (cache)
    {
        std::getline(cache, line); result->set_build_depend(line);
        std::getline(cache, line); result->set_run_depend(line);
        std::getline(cache, line); result->slot = SlotName(line);
        std::getline(cache, line); result->set_src_uri(line);
        std::getline(cache, line); result->set_restrictions(line);
        std::getline(cache, line); result->set_homepage(line);
        std::getline(cache, line); result->set_license(line);
        std::getline(cache, line); result->description = line;
        std::getline(cache, line); result->set_keywords(line);
        std::getline(cache, line); result->set_inherited(line);
        std::getline(cache, line); result->set_iuse(line);
        std::getline(cache, line);
        std::getline(cache, line); result->set_post_depend(line);
        std::getline(cache, line); result->set_provide(line);
        std::getline(cache, line); result->eapi = EAPIData::get_instance()->eapi_from_string(line);

        // check mtimes
        time_t cache_time(std::max(_master_mtime, _filename.mtime()));
        bool ok = _ebuild.mtime() <= cache_time &&
            result->inherited()->end() == std::find_if(result->inherited()->begin(), result->inherited()->end(),
                    std::tr1::bind(std::greater<time_t>(), std::tr1::bind(
                            std::tr1::mem_fn(&EclassMtimes::mtime), _eclass_mtimes.get(), _1), cache_time));

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

    std::string normalise(std::tr1::shared_ptr<const DepSpec> d)
    {
        DepSpecPrettyPrinter p(0, false);
        d->accept(&p);
        return stringify(p);
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
    catch (const FSError &)
    {
        // let the 'if (cache)' handle the error
    }
    std::ofstream cache(stringify(_filename).c_str());

    if (cache)
    {
        cache << normalise(v->build_depend()) << std::endl;
        cache << normalise(v->run_depend()) << std::endl;
        cache << normalise(v->slot) << std::endl;
        cache << normalise(v->src_uri()) << std::endl;
        cache << normalise(v->restrictions()) << std::endl;
        cache << normalise(v->homepage()) << std::endl;
        cache << normalise(v->license()) << std::endl;
        cache << normalise(v->description) << std::endl;
        cache << join(v->keywords()->begin(), v->keywords()->end(), " ") << std::endl;
        cache << join(v->inherited()->begin(), v->inherited()->end(), " ") << std::endl;
        cache << join(v->iuse()->begin(), v->iuse()->end(), " ") << std::endl;
        cache << std::endl;
        cache << normalise(v->post_depend()) << std::endl;
        cache << normalise(v->provide()) << std::endl;
        cache << normalise(v->eapi.name) << std::endl;
    }
    else
    {
        Log::get_instance()->message(ll_warning, lc_no_context,
                "Couldn't write cache file to '" + stringify(_filename) + "'");
    }

}

