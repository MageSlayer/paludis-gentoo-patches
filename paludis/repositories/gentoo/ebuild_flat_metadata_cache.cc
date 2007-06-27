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
#include <paludis/util/visitor-impl.hh>
#include <paludis/dep_spec_pretty_printer.hh>
#include <paludis/eapi.hh>
#include <fstream>
#include <set>
#include <list>
#include <paludis/util/tr1_functional.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>
#include <functional>
#include <algorithm>

using namespace paludis;
using namespace paludis::erepository;

EbuildFlatMetadataCache::EbuildFlatMetadataCache(const FSEntry & f,
        const FSEntry & e, time_t t, tr1::shared_ptr<const EclassMtimes> m, bool s) :
    _filename(f),
    _ebuild(e),
    _master_mtime(t),
    _eclass_mtimes(m),
    _silent(s)
{
}

bool
EbuildFlatMetadataCache::load(const tr1::shared_ptr<const EbuildID> & id)
{
    using namespace tr1::placeholders;

    Context context("When loading version metadata from '" + stringify(_filename) + "':");

    std::ifstream cache(stringify(_filename).c_str());
    std::string line;

    if (cache)
    {
        std::getline(cache, line); id->load_build_depend("DEPEND", "Build depend", line);
        std::getline(cache, line); id->load_run_depend("RDEPEND", "Run depend", line);
        std::getline(cache, line); id->set_slot(SlotName(line));
        std::getline(cache, line); id->load_src_uri("SRC_URI", "Source URI", line);
        std::getline(cache, line); id->load_restrict("RESTRICT", "Restrictions", line);
        std::getline(cache, line); id->load_homepage("HOMEPAGE", "Homepage", line);
        std::getline(cache, line); id->load_license("LICENSE", "License", line);
        std::getline(cache, line); id->load_short_description("DESCRIPTION", "Description", line);
        std::getline(cache, line); id->load_keywords("KEYWORDS", "Keywords", line);
        std::getline(cache, line); id->load_inherited("INHERITED", "Inherited", line);
        std::getline(cache, line); id->load_iuse("IUSE", "Used USE flags", line);
        std::getline(cache, line);
        std::getline(cache, line); id->load_post_depend("PDEPEND", "Post depend", line);
        std::getline(cache, line); id->load_provide("PROVIDE", "Provides", line);
        std::getline(cache, line); id->set_eapi(line);

        // check mtimes
        time_t cache_time(std::max(_master_mtime, _filename.mtime()));
        bool ok = _ebuild.mtime() <= cache_time &&
            id->inherited_key()->value()->end() == std::find_if(id->inherited_key()->value()->begin(), id->inherited_key()->value()->end(),
                    tr1::bind(std::greater<time_t>(), tr1::bind(
                            tr1::mem_fn(&EclassMtimes::mtime), _eclass_mtimes.get(), _1), cache_time));

        if (! ok)
            Log::get_instance()->message(ll_warning, lc_no_context) << "Stale cache file at '"
                << _filename << "'";
        return ok;

        return true;
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

    template <typename T_>
    std::string flatten(const T_ & d)
    {
        DepSpecPrettyPrinter p(0, false);
        d->accept(p);
        return stringify(p);
    }
}

void
EbuildFlatMetadataCache::save(const tr1::shared_ptr<const EbuildID> & id)
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
        if (id->build_dependencies_key())
            cache << flatten(id->build_dependencies_key()->value()) << std::endl;
        else
            cache << std::endl;

        if (id->run_dependencies_key())
            cache << flatten(id->run_dependencies_key()->value()) << std::endl;
        else
            cache << std::endl;

        cache << normalise(id->slot()) << std::endl;

        if (id->src_uri_key())
            cache << flatten(id->src_uri_key()->value()) << std::endl;
        else
            cache << std::endl;

        if (id->restrict_key())
            cache << flatten(id->restrict_key()->value()) << std::endl;
        else
            cache << std::endl;

        if (id->homepage_key())
            cache << flatten(id->homepage_key()->value()) << std::endl;
        else
            cache << std::endl;

        if (id->license_key())
            cache << flatten(id->license_key()->value()) << std::endl;
        else
            cache << std::endl;

        if (id->short_description_key())
            cache << normalise(id->short_description_key()->value()) << std::endl;
        else
            cache << std::endl;

        if (id->keywords_key())
            cache << join(id->keywords_key()->value()->begin(), id->keywords_key()->value()->end(), " ") << std::endl;
        else
            cache << std::endl;

        if (id->inherited_key())
            cache << join(id->inherited_key()->value()->begin(), id->inherited_key()->value()->end(), " ") << std::endl;
        else
            cache << std::endl;

        if (id->iuse_key())
            cache << join(id->iuse_key()->value()->begin(), id->iuse_key()->value()->end(), " ") << std::endl;
        else
            cache << std::endl;

        cache << std::endl;

        if (id->post_dependencies_key())
            cache << flatten(id->post_dependencies_key()->value()) << std::endl;
        else
            cache << std::endl;

        if (id->provide_key())
            cache << flatten(id->provide_key()->value()) << std::endl;
        else
            cache << std::endl;

        cache << normalise(id->eapi()->name) << std::endl;
    }
    else
    {
        Log::get_instance()->message(ll_warning, lc_no_context) << "Couldn't write cache file to '"
            << _filename << "'";
    }
}

