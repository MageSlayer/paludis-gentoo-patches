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
#include <paludis/util/set.hh>
#include <paludis/repositories/e/dep_spec_pretty_printer.hh>
#include <paludis/stringify_formatter.hh>
#include <paludis/repositories/e/eapi.hh>
#include <paludis/util/tr1_functional.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>
#include <fstream>
#include <set>
#include <list>
#include <vector>
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

    if (cache)
    {
        std::vector<std::string> lines;
        std::string line;
        while (std::getline(cache, line))
            lines.push_back(line);

        try
        {
            if (lines.size() >= 15)
            {
                id->set_eapi(lines[14]);
                bool ok(true);

                if (id->eapi()->supported)
                {
                    const EAPIEbuildMetadataVariables & m(*id->eapi()->supported->ebuild_metadata_variables);

                    {
                        time_t cache_time(std::max(_master_mtime, _filename.mtime()));
                        std::set<std::string> tokens;
                        WhitespaceTokeniser::tokenise(lines[9], std::inserter(tokens, tokens.begin()));
                        ok = _ebuild.mtime() <= cache_time;

                        if (ok && ! tokens.empty())
                            ok = tokens.end() == std::find_if(tokens.begin(), tokens.end(),
                                    tr1::bind(std::greater<time_t>(), tr1::bind(
                                            tr1::mem_fn(&EclassMtimes::mtime), _eclass_mtimes.get(), _1), cache_time));
                    }

                    if (ok)
                    {
                        if (! m.metadata_build_depend.empty())
                            id->load_build_depend(m.metadata_build_depend, m.description_build_depend, lines[0]);
                        if (! m.metadata_run_depend.empty())
                            id->load_run_depend(m.metadata_run_depend, m.description_run_depend, lines[1]);
                        id->set_slot(SlotName(lines[2]));
                        if (! m.metadata_src_uri.empty())
                            id->load_src_uri(m.metadata_src_uri, m.description_src_uri, lines[3]);
                        if (! m.metadata_restrict.empty())
                            id->load_restrict(m.metadata_restrict, m.description_restrict, lines[4]);
                        if (! m.metadata_homepage.empty())
                            id->load_homepage(m.metadata_homepage, m.description_homepage, lines[5]);
                        if (! m.metadata_license.empty())
                            id->load_license(m.metadata_license, m.description_license, lines[6]);
                        if (! m.metadata_description.empty())
                            id->load_short_description(m.metadata_description, m.description_description, lines[7]);
                        if (! m.metadata_keywords.empty())
                            id->load_keywords(m.metadata_keywords, m.description_keywords, lines[8]);
                        if (! m.metadata_inherited.empty())
                            id->load_inherited(m.metadata_inherited, m.description_inherited, lines[9]);
                        if (! m.metadata_iuse.empty())
                            id->load_iuse(m.metadata_iuse, m.description_iuse, lines[10]);
                        /* 11 no longer used */
                        if (! m.metadata_pdepend.empty())
                            id->load_post_depend(m.metadata_pdepend, m.description_pdepend, lines[12]);
                        if (! m.metadata_provide.empty())
                            id->load_provide(m.metadata_provide, m.description_provide, lines[13]);
                    }
                }
                else
                    id->set_slot(SlotName("UNKNOWN"));

                if (! ok)
                    Log::get_instance()->message(ll_warning, lc_no_context) << "Stale cache file at '"
                        << _filename << "'";
                return ok;
            }
            else
            {
                Log::get_instance()->message(ll_warning, lc_no_context) << "Incomplete cache file at '"
                    << _filename << "'";
                return false;
            }
        }
        catch (const Exception & e)
        {
            Log::get_instance()->message(ll_warning, lc_no_context) << "Not using cache file at '"
                << _filename << "' due to exception '" << e.message() << "' (" << e.what() << ")";
            return false;
        }
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
        WhitespaceTokeniser::tokenise(stringify(s), std::back_inserter(tokens));
        return join(tokens.begin(), tokens.end(), " ");
    }

    template <typename T_>
    std::string flatten(const T_ & d)
    {
        StringifyFormatter ff;
        DepSpecPrettyPrinter p(0, tr1::shared_ptr<const PackageID>(), ff, 0, false);
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
        // let the 'if (cache_file)' handle the error
    }

    std::ostringstream cache;

    try
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

        if (id->fetches_key())
            cache << flatten(id->fetches_key()->value()) << std::endl;
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
    catch (const Exception & e)
    {
        Log::get_instance()->message(ll_warning, lc_no_context) << "Not writing cache file to '"
            << _filename << "' due to exception '" << e.message() << "' (" << e.what() << ")";
        return;
    }

    std::ofstream cache_file(stringify(_filename).c_str());

    if (cache_file)
        cache_file << cache.str();
    else
    {
        Log::get_instance()->message(ll_warning, lc_no_context) << "Couldn't write cache file to '"
            << _filename << "'";
    }
}

