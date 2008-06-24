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

                if ((*id->eapi())[k::supported()])
                {
                    const EAPIEbuildMetadataVariables & m((*(*id->eapi())[k::supported()])[k::ebuild_metadata_variables()]);

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
                        ok = static_cast<int>(lines.size()) >= m.flat_cache_minimum_size;

                    if (ok)
                    {
                        if (-1 != m.flat_cache_dependencies)
                            if (! m.metadata_dependencies.empty())
                            {
                                DependenciesRewriter rewriter;
                                parse_depend(lines.at(m.flat_cache_dependencies), _env, id, *id->eapi())->accept(rewriter);
                                id->load_build_depend(m.metadata_dependencies + ".DEPEND", m.description_dependencies + " (build)", rewriter.depend());
                                id->load_run_depend(m.metadata_dependencies + ".RDEPEND", m.description_dependencies + " (run)", rewriter.rdepend());
                                id->load_post_depend(m.metadata_dependencies + ".PDEPEND", m.description_dependencies + " (post)", rewriter.pdepend());
                            }

                        if (-1 != m.flat_cache_build_depend)
                            if (! m.metadata_build_depend.empty())
                                id->load_build_depend(m.metadata_build_depend, m.description_build_depend, lines.at(m.flat_cache_build_depend));

                        if (-1 != m.flat_cache_run_depend)
                            if (! m.metadata_run_depend.empty())
                                id->load_run_depend(m.metadata_run_depend, m.description_run_depend, lines.at(m.flat_cache_run_depend));

                        id->set_slot(SlotName(lines.at(m.flat_cache_slot)));

                        if (-1 != m.flat_cache_src_uri)
                            if (! m.metadata_src_uri.empty())
                                id->load_src_uri(m.metadata_src_uri, m.description_src_uri, lines.at(m.flat_cache_src_uri));

                        if (-1 != m.flat_cache_restrict)
                            if (! m.metadata_restrict.empty())
                                id->load_restrict(m.metadata_restrict, m.description_restrict, lines.at(m.flat_cache_restrict));

                        if (-1 != m.flat_cache_homepage)
                            if (! m.metadata_homepage.empty())
                                id->load_homepage(m.metadata_homepage, m.description_homepage, lines.at(m.flat_cache_homepage));

                        if (-1 != m.flat_cache_license)
                            if (! m.metadata_license.empty())
                                id->load_license(m.metadata_license, m.description_license, lines.at(m.flat_cache_license));

                        if (-1 != m.flat_cache_description)
                            if (! m.metadata_description.empty())
                                id->load_short_description(m.metadata_description, m.description_description, lines.at(m.flat_cache_description));

                        if (-1 != m.flat_cache_keywords)
                            if (! m.metadata_keywords.empty())
                                id->load_keywords(m.metadata_keywords, m.description_keywords, lines.at(m.flat_cache_keywords));

                        if (-1 != m.flat_cache_inherited)
                            if (! m.metadata_inherited.empty())
                                id->load_inherited(m.metadata_inherited, m.description_inherited, lines.at(m.flat_cache_inherited));

                        if (-1 != m.flat_cache_iuse)
                            if (! m.metadata_iuse.empty())
                                id->load_iuse(m.metadata_iuse, m.description_iuse, lines.at(m.flat_cache_iuse));

                        if (-1 != m.flat_cache_pdepend)
                            if (! m.metadata_pdepend.empty())
                                id->load_post_depend(m.metadata_pdepend, m.description_pdepend, lines.at(m.flat_cache_pdepend));

                        if (-1 != m.flat_cache_provide)
                            if (! m.metadata_provide.empty())
                                id->load_provide(m.metadata_provide, m.description_provide, lines.at(m.flat_cache_provide));

                        if (-1 != m.flat_cache_use)
                            if (! m.metadata_use.empty())
                                id->load_use(m.metadata_use, m.description_use, lines.at(m.flat_cache_use));
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

    if (! (*id->eapi())[k::supported()])
    {
        Log::get_instance()->message("e.cache.save.eapi_unsupoprted", ll_warning, lc_no_context) << "Not writing cache file to '"
            << _filename << "' because EAPI '" << (*id->eapi())[k::name()] << "' is not supported";
        return;
    }

    std::ostringstream cache;

    try
    {
        const EAPIEbuildMetadataVariables & m((*(*id->eapi())[k::supported()])[k::ebuild_metadata_variables()]);
        for (int x(0), x_end(m.flat_cache_minimum_size) ; x != x_end ; ++x)
        {
            if (x == m.flat_cache_dependencies)
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
            else if (x == m.flat_cache_use)
            {
                if (id->use_key())
                    cache << join(id->use_key()->value()->begin(), id->use_key()->value()->end(), " ") << std::endl;
                else
                    cache << std::endl;
            }
            else if (x == m.flat_cache_build_depend)
            {
                if (id->build_dependencies_key())
                    cache << flatten(id->build_dependencies_key()->value()) << std::endl;
                else
                    cache << std::endl;
            }
            else if (x == m.flat_cache_run_depend)
            {
                if (id->run_dependencies_key())
                    cache << flatten(id->run_dependencies_key()->value()) << std::endl;
                else
                    cache << std::endl;
            }
            else if (x == m.flat_cache_slot)
            {
                cache << normalise(id->slot()) << std::endl;
            }
            else if (x == m.flat_cache_src_uri)
            {
                if (id->fetches_key())
                    cache << flatten(id->fetches_key()->value()) << std::endl;
                else
                    cache << std::endl;
            }
            else if (x == m.flat_cache_restrict)
            {
                if (id->restrict_key())
                    cache << flatten(id->restrict_key()->value()) << std::endl;
                else
                    cache << std::endl;
            }
            else if (x == m.flat_cache_homepage)
            {
                if (id->homepage_key())
                    cache << flatten(id->homepage_key()->value()) << std::endl;
                else
                    cache << std::endl;
            }
            else if (x == m.flat_cache_license)
            {
                if (id->license_key())
                    cache << flatten(id->license_key()->value()) << std::endl;
                else
                    cache << std::endl;
            }
            else if (x == m.flat_cache_description)
            {
                if (id->short_description_key())
                    cache << normalise(id->short_description_key()->value()) << std::endl;
                else
                    cache << std::endl;
            }
            else if (x == m.flat_cache_keywords)
            {
                if (id->keywords_key())
                    cache << join(id->keywords_key()->value()->begin(), id->keywords_key()->value()->end(), " ") << std::endl;
                else
                    cache << std::endl;
            }
            else if (x == m.flat_cache_inherited)
            {
                if (id->inherited_key())
                    cache << join(id->inherited_key()->value()->begin(), id->inherited_key()->value()->end(), " ") << std::endl;
                else
                    cache << std::endl;
            }
            else if (x == m.flat_cache_iuse)
            {
                if (id->iuse_key())
                    cache << join(id->iuse_key()->value()->begin(), id->iuse_key()->value()->end(), " ") << std::endl;
                else
                    cache << std::endl;
            }
            else if (x == m.flat_cache_pdepend)
            {
                if (id->post_dependencies_key())
                    cache << flatten(id->post_dependencies_key()->value()) << std::endl;
                else
                    cache << std::endl;
            }
            else if (x == m.flat_cache_provide)
            {
                if (id->provide_key())
                    cache << flatten(id->provide_key()->value()) << std::endl;
                else
                    cache << std::endl;
            }
            else if (x == m.flat_cache_eapi)
            {
                cache << normalise((*id->eapi())[k::name()]) << std::endl;
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

