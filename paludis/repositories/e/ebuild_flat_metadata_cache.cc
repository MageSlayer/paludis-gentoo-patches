/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008 Ciaran McCreesh
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
#include <paludis/repositories/e/dep_parser.hh>
#include <paludis/repositories/e/dependencies_rewriter.hh>
#include <paludis/stringify_formatter.hh>
#include <paludis/repositories/e/eapi.hh>
#include <tr1/functional>
#include <fstream>
#include <set>
#include <list>
#include <vector>
#include <functional>
#include <algorithm>
#include <cstring>
#include <errno.h>

using namespace paludis;
using namespace paludis::erepository;

EbuildFlatMetadataCache::EbuildFlatMetadataCache(const Environment * const v, const FSEntry & f,
        const FSEntry & e, time_t t, std::tr1::shared_ptr<const EclassMtimes> m, bool s) :
    _env(v),
    _filename(f),
    _ebuild(e),
    _master_mtime(t),
    _eclass_mtimes(m),
    _silent(s)
{
}

bool
EbuildFlatMetadataCache::load(const std::tr1::shared_ptr<const EbuildID> & id)
{
    using namespace std::tr1::placeholders;

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

                if (id->eapi()->supported())
                {
                    const EAPIEbuildMetadataVariables & m(*id->eapi()->supported()->ebuild_metadata_variables());

                    {
                        time_t cache_time(std::max(_master_mtime, _filename.mtime()));
                        std::set<std::string> tokens;
                        tokenise_whitespace(lines[9], std::inserter(tokens, tokens.begin()));
                        ok = _ebuild.mtime() <= cache_time;

                        if (ok && ! tokens.empty())
                            ok = tokens.end() == std::find_if(tokens.begin(), tokens.end(),
                                    std::tr1::bind(std::greater<time_t>(), std::tr1::bind(
                                            std::tr1::mem_fn(&EclassMtimes::mtime), _eclass_mtimes.get(), _1), cache_time));
                    }

                    if (ok)
                        ok = static_cast<int>(lines.size()) >= m.minimum_flat_cache_size();

                    if (ok)
                    {
                        if (-1 != m.dependencies().flat_cache_index())
                            if (! m.dependencies().name().empty())
                            {
                                DependenciesRewriter rewriter;
                                parse_depend(lines.at(m.dependencies().flat_cache_index()), _env, id, *id->eapi())->accept(rewriter);
                                id->load_build_depend(m.dependencies().name() + ".DEPEND", m.dependencies().description() + " (build)", rewriter.depend());
                                id->load_run_depend(m.dependencies().name() + ".RDEPEND", m.dependencies().description() + " (run)", rewriter.rdepend());
                                id->load_post_depend(m.dependencies().name() + ".PDEPEND", m.dependencies().description() + " (post)", rewriter.pdepend());
                            }

                        if (-1 != m.build_depend().flat_cache_index())
                            if (! m.build_depend().name().empty())
                                id->load_build_depend(m.build_depend().name(), m.build_depend().description(), lines.at(m.build_depend().flat_cache_index()));

                        if (-1 != m.run_depend().flat_cache_index())
                            if (! m.run_depend().name().empty())
                                id->load_run_depend(m.run_depend().name(), m.run_depend().description(), lines.at(m.run_depend().flat_cache_index()));

                        id->set_slot(SlotName(lines.at(m.slot().flat_cache_index())));

                        if (-1 != m.src_uri().flat_cache_index())
                            if (! m.src_uri().name().empty())
                                id->load_src_uri(m.src_uri().name(), m.src_uri().description(), lines.at(m.src_uri().flat_cache_index()));

                        if (-1 != m.restrictions().flat_cache_index())
                            if (! m.restrictions().name().empty())
                                id->load_restrict(m.restrictions().name(), m.restrictions().description(), lines.at(m.restrictions().flat_cache_index()));

                        if (-1 != m.homepage().flat_cache_index())
                            if (! m.homepage().name().empty())
                                id->load_homepage(m.homepage().name(), m.homepage().description(), lines.at(m.homepage().flat_cache_index()));

                        if (-1 != m.license().flat_cache_index())
                            if (! m.license().name().empty())
                                id->load_license(m.license().name(), m.license().description(), lines.at(m.license().flat_cache_index()));

                        if (-1 != m.description().flat_cache_index())
                            if (! m.description().name().empty())
                                id->load_short_description(m.description().name(), m.description().description(), lines.at(m.description().flat_cache_index()));

                        if (-1 != m.keywords().flat_cache_index())
                            if (! m.keywords().name().empty())
                                id->load_keywords(m.keywords().name(), m.keywords().description(), lines.at(m.keywords().flat_cache_index()));

                        if (-1 != m.inherited().flat_cache_index())
                            if (! m.inherited().name().empty())
                                id->load_inherited(m.inherited().name(), m.inherited().description(), lines.at(m.inherited().flat_cache_index()));

                        if (-1 != m.iuse().flat_cache_index())
                            if (! m.iuse().name().empty())
                                id->load_iuse(m.iuse().name(), m.iuse().description(), lines.at(m.iuse().flat_cache_index()));

                        if (-1 != m.pdepend().flat_cache_index())
                            if (! m.pdepend().name().empty())
                                id->load_post_depend(m.pdepend().name(), m.pdepend().description(), lines.at(m.pdepend().flat_cache_index()));

                        if (-1 != m.provide().flat_cache_index())
                            if (! m.provide().name().empty())
                                id->load_provide(m.provide().name(), m.provide().description(), lines.at(m.provide().flat_cache_index()));

                        if (-1 != m.use().flat_cache_index())
                            if (! m.use().name().empty())
                                id->load_use(m.use().name(), m.use().description(), lines.at(m.use().flat_cache_index()));
                    }
                }
                else
                    id->set_slot(SlotName("UNKNOWN"));

                if (! ok)
                    Log::get_instance()->message("e.cache.stale", ll_warning, lc_no_context) << "Stale cache file at '"
                        << _filename << "'";
                return ok;
            }
            else
            {
                Log::get_instance()->message("e.cache.incomplete", ll_warning, lc_no_context) << "Incomplete cache file at '"
                    << _filename << "'";
                return false;
            }
        }
        catch (const InternalError &)
        {
            throw;
        }
        catch (const Exception & e)
        {
            Log::get_instance()->message("e.cache.failure", ll_warning, lc_no_context) << "Not using cache file at '"
                << _filename << "' due to exception '" << e.message() << "' (" << e.what() << ")";
            return false;
        }
    }
    else
    {
        Log::get_instance()->message("e.cache.failure", _silent ? ll_debug : ll_warning, lc_no_context)
                << "Couldn't use the cache file at '" << _filename << "'";
        return false;
    }
}

namespace
{
    template <typename T_>
    std::string normalise(const T_ & s)
    {
        std::list<std::string> tokens;
        tokenise_whitespace(stringify(s), std::back_inserter(tokens));
        return join(tokens.begin(), tokens.end(), " ");
    }

    template <typename T_>
    std::string flatten(const T_ & d)
    {
        StringifyFormatter ff;
        DepSpecPrettyPrinter p(0, std::tr1::shared_ptr<const PackageID>(), ff, 0, false);
        d->accept(p);
        return stringify(p);
    }
}

void
EbuildFlatMetadataCache::save(const std::tr1::shared_ptr<const EbuildID> & id)
{
    Context context("When saving version metadata to '" + stringify(_filename) + "':");

    try
    {
        _filename.dirname().dirname().mkdir();
        _filename.dirname().mkdir();
    }
    catch (const FSError & e)
    {
        Log::get_instance()->message("e.cache.save.failure", ll_warning, lc_no_context) << "Couldn't create cache directory: " << e.message();
        return;
    }

    if (! id->eapi()->supported())
    {
        Log::get_instance()->message("e.cache.save.eapi_unsupoprted", ll_warning, lc_no_context) << "Not writing cache file to '"
            << _filename << "' because EAPI '" << id->eapi()->name() << "' is not supported";
        return;
    }

    std::ostringstream cache;

    try
    {
        const EAPIEbuildMetadataVariables & m(*id->eapi()->supported()->ebuild_metadata_variables());
        for (int x(0), x_end(m.minimum_flat_cache_size()) ; x != x_end ; ++x)
        {
            if (x == m.dependencies().flat_cache_index())
            {
                std::string s;

                if (id->build_dependencies_key())
                    s.append(flatten(id->build_dependencies_key()->value()) + " ");
                if (id->run_dependencies_key())
                    s.append(flatten(id->run_dependencies_key()->value()) + " ");
                if (id->post_dependencies_key())
                    s.append(flatten(id->post_dependencies_key()->value()) + " ");

                cache << s << std::endl;
            }
            else if (x == m.use().flat_cache_index())
            {
                if (id->use_key())
                    cache << join(id->use_key()->value()->begin(), id->use_key()->value()->end(), " ") << std::endl;
                else
                    cache << std::endl;
            }
            else if (x == m.build_depend().flat_cache_index())
            {
                if (id->build_dependencies_key())
                    cache << flatten(id->build_dependencies_key()->value()) << std::endl;
                else
                    cache << std::endl;
            }
            else if (x == m.run_depend().flat_cache_index())
            {
                if (id->run_dependencies_key())
                    cache << flatten(id->run_dependencies_key()->value()) << std::endl;
                else
                    cache << std::endl;
            }
            else if (x == m.slot().flat_cache_index())
            {
                cache << normalise(id->slot()) << std::endl;
            }
            else if (x == m.src_uri().flat_cache_index())
            {
                if (id->fetches_key())
                    cache << flatten(id->fetches_key()->value()) << std::endl;
                else
                    cache << std::endl;
            }
            else if (x == m.restrictions().flat_cache_index())
            {
                if (id->restrict_key())
                    cache << flatten(id->restrict_key()->value()) << std::endl;
                else
                    cache << std::endl;
            }
            else if (x == m.homepage().flat_cache_index())
            {
                if (id->homepage_key())
                    cache << flatten(id->homepage_key()->value()) << std::endl;
                else
                    cache << std::endl;
            }
            else if (x == m.license().flat_cache_index())
            {
                if (id->license_key())
                    cache << flatten(id->license_key()->value()) << std::endl;
                else
                    cache << std::endl;
            }
            else if (x == m.description().flat_cache_index())
            {
                if (id->short_description_key())
                    cache << normalise(id->short_description_key()->value()) << std::endl;
                else
                    cache << std::endl;
            }
            else if (x == m.keywords().flat_cache_index())
            {
                if (id->keywords_key())
                    cache << join(id->keywords_key()->value()->begin(), id->keywords_key()->value()->end(), " ") << std::endl;
                else
                    cache << std::endl;
            }
            else if (x == m.inherited().flat_cache_index())
            {
                if (id->inherited_key())
                    cache << join(id->inherited_key()->value()->begin(), id->inherited_key()->value()->end(), " ") << std::endl;
                else
                    cache << std::endl;
            }
            else if (x == m.iuse().flat_cache_index())
            {
                if (id->iuse_key())
                    cache << join(id->iuse_key()->value()->begin(), id->iuse_key()->value()->end(), " ") << std::endl;
                else
                    cache << std::endl;
            }
            else if (x == m.pdepend().flat_cache_index())
            {
                if (id->post_dependencies_key())
                    cache << flatten(id->post_dependencies_key()->value()) << std::endl;
                else
                    cache << std::endl;
            }
            else if (x == m.provide().flat_cache_index())
            {
                if (id->provide_key())
                    cache << flatten(id->provide_key()->value()) << std::endl;
                else
                    cache << std::endl;
            }
            else if (x == m.eapi().flat_cache_index())
            {
                cache << normalise(id->eapi()->name()) << std::endl;
            }
            else
                cache << std::endl;
        }
    }
    catch (const InternalError &)
    {
        throw;
    }
    catch (const Exception & e)
    {
        Log::get_instance()->message("e.cache.save.failure", ll_warning, lc_no_context) << "Not writing cache file to '"
            << _filename << "' due to exception '" << e.message() << "' (" << e.what() << ")";
        return;
    }

    std::ofstream cache_file(stringify(_filename).c_str());

    if (cache_file)
        cache_file << cache.str();
    else
    {
        Log::get_instance()->message("e.cache.save.failure", ll_warning, lc_no_context) << "Couldn't write cache file to '"
            << _filename << "': " << std::strerror(errno);
    }
}

