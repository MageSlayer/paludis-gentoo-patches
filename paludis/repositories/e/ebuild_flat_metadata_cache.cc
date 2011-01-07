/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
 * Copyright (c) 2008 David Leverton
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
#include <paludis/util/set.hh>
#include <paludis/util/destringify.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/safe_ofstream.hh>
#include <paludis/util/safe_ifstream.hh>
#include <paludis/util/timestamp.hh>
#include <paludis/util/fs_stat.hh>
#include <paludis/util/fs_error.hh>
#include <paludis/repositories/e/dep_parser.hh>
#include <paludis/repositories/e/dependencies_rewriter.hh>
#include <paludis/repositories/e/eapi.hh>
#include <paludis/repositories/e/spec_tree_pretty_printer.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/environment.hh>
#include <paludis/package_database.hh>
#include <paludis/unformatted_pretty_printer.hh>
#include <set>
#include <map>
#include <list>
#include <vector>
#include <functional>
#include <algorithm>
#include <cstring>
#include <ctime>
#include <errno.h>
#include <utime.h>

using namespace paludis;
using namespace paludis::erepository;

namespace paludis
{
    template <>
    struct Imp<EbuildFlatMetadataCache>
    {
        const Environment * const env;
        const FSPath filename;
        const FSStat filename_stat;
        const FSPath ebuild;
        const FSStat ebuild_stat;
        std::time_t master_mtime;
        std::shared_ptr<const EclassMtimes> eclass_mtimes;
        bool silent;

        Imp(const Environment * const e, const FSPath & f, const FSPath & eb,
                std::time_t m, const std::shared_ptr<const EclassMtimes> em, bool s) :
            env(e),
            filename(f),
            filename_stat(filename.stat()),
            ebuild(eb),
            ebuild_stat(ebuild.stat()),
            master_mtime(m),
            eclass_mtimes(em),
            silent(s)
        {
        }
    };
}

namespace
{
    bool load_flat_list(
        const std::shared_ptr<const EbuildID> & id, const std::vector<std::string> & lines, Imp<EbuildFlatMetadataCache> * _imp)
    {
        Context ctx("When loading flat_list format cache file:");

        if (lines.size() < 15)
        {
            Log::get_instance()->message("e.cache.flat_list.truncated", ll_warning, lc_context)
                << "cache file has " << lines.size() << " lines, but expected at least 15";
            return false;
        }

        id->set_eapi(lines[14]);

        if (id->eapi()->supported())
        {
            const EAPIEbuildMetadataVariables & m(*id->eapi()->supported()->ebuild_metadata_variables());
            if (-1 == m.minimum_flat_list_size())
            {
                Log::get_instance()->message("e.cache.flat_list.unsupported", ll_warning, lc_context)
                    << "flat_list cache is not supported for EAPI '" << id->eapi()->name() << "'";
                return false;
            }

            if (static_cast<int>(lines.size()) < m.minimum_flat_list_size())
            {
                Log::get_instance()->message("e.cache.flat_list.truncated", ll_warning, lc_context)
                    << "cache file has " << lines.size() << " lines, but expected at least " << m.minimum_flat_list_size();
                return false;
            }

            {
                std::time_t cache_time(std::max(_imp->master_mtime, _imp->filename.stat().mtim().seconds()));
                bool ok(_imp->ebuild_stat.mtim().seconds() <= cache_time);
                if (! ok)
                    Log::get_instance()->message("e.cache.flat_list.mtime", ll_debug, lc_context)
                        << "ebuild has mtime " << _imp->ebuild_stat.mtim().seconds() << ", but expected at most " << cache_time;

                if (ok && "0" != id->guessed_eapi_name())
                {
                    Log::get_instance()->message("e.cache.flat_list.guessed_eapi", ll_debug, lc_context)
                        << "ebuild has guessed EAPI '" << id->guessed_eapi_name() << "', but flat_list assumes '0'";
                    ok = false;
                }

                if (ok)
                {
                    std::set<std::string> tokens;
                    tokenise_whitespace(lines[m.inherited()->flat_list_index()], std::inserter(tokens, tokens.begin()));
                    auto repo(_imp->env->package_database()->fetch_repository(id->repository_name()));
                    FSPath eclassdir((repo->location_key()->value() / "eclass").realpath_if_exists());
                    for (std::set<std::string>::const_iterator it(tokens.begin()),
                             it_end(tokens.end()); it_end != it; ++it)
                    {
                        const std::pair<FSPath, FSStat> * eclass(_imp->eclass_mtimes->eclass(*it));
                        if (eclass)
                            Log::get_instance()->message("e.cache.flat_list.eclass.path", ll_debug, lc_context)
                                << "Cache-requested eclass '" << *it << "' maps to '" << eclass->first << "'";

                        if (! eclass)
                        {
                            Log::get_instance()->message("e.cache.flat_list.eclass.missing", ll_debug, lc_context)
                                << "Can't find cache-requested eclass '" << *it << "'";
                            ok = false;
                        }

                        else if (eclass->first.dirname() != eclassdir)
                        {
                            Log::get_instance()->message("e.cache.flat_list.eclass.wrong_location", ll_debug, lc_context)
                                << "Cache-requested eclass '" << *it << "' was found at '"
                                << eclass->first.dirname() << "', but expected '" << eclassdir << "'";
                            ok = false;
                        }

                        else if (eclass->second.mtim().seconds() > cache_time)
                        {
                            Log::get_instance()->message("e.cache.flat_list.eclass.wrong_mtime", ll_debug, lc_context)
                                << "Cache-requested eclass '" << *it << "' has mtime "
                                << eclass->second.mtim().seconds() << ", but expected at most " << cache_time;
                            ok = false;
                        }

                        if (! ok)
                            break;
                    }
                }

                if (! ok)
                {
                    Log::get_instance()->message("e.cache.stale", ll_warning, lc_no_context)
                        << "Stale cache file at '" << _imp->filename << "'";
                    return false;
                }
            }

            if (-1 != m.dependencies()->flat_list_index() && ! m.dependencies()->name().empty())
            {
                DependenciesRewriter rewriter;
                parse_depend(lines.at(m.dependencies()->flat_list_index()), _imp->env, *id->eapi())->top()->accept(rewriter);
                id->load_raw_depend(m.dependencies()->name(), m.dependencies()->description(),
                        lines.at(m.dependencies()->flat_list_index()));
                id->load_build_depend(m.dependencies()->name() + ".DEPEND", m.dependencies()->description() + " (build)", rewriter.depend(), true);
                id->load_run_depend(m.dependencies()->name() + ".RDEPEND", m.dependencies()->description() + " (run)", rewriter.rdepend(), true);
                id->load_post_depend(m.dependencies()->name() + ".PDEPEND", m.dependencies()->description() + " (post)", rewriter.pdepend(), true);
            }

            if (-1 != m.build_depend()->flat_list_index() && ! m.build_depend()->name().empty())
                id->load_build_depend(m.build_depend()->name(), m.build_depend()->description(), lines.at(m.build_depend()->flat_list_index()), false);

            if (-1 != m.run_depend()->flat_list_index() && ! m.run_depend()->name().empty())
                id->load_run_depend(m.run_depend()->name(), m.run_depend()->description(), lines.at(m.run_depend()->flat_list_index()), false);

            id->load_slot(m.slot(), lines.at(m.slot()->flat_list_index()));

            if (-1 != m.src_uri()->flat_list_index() && ! m.src_uri()->name().empty())
                id->load_src_uri(m.src_uri(), lines.at(m.src_uri()->flat_list_index()));

            if (-1 != m.restrictions()->flat_list_index() && ! m.restrictions()->name().empty())
                id->load_restrict(m.restrictions(), lines.at(m.restrictions()->flat_list_index()));

            if (-1 != m.properties()->flat_list_index() && ! m.properties()->name().empty())
                id->load_properties(m.properties(), lines.at(m.properties()->flat_list_index()));

            if (-1 != m.homepage()->flat_list_index() && ! m.homepage()->name().empty())
                id->load_homepage(m.homepage()->name(), m.homepage()->description(), lines.at(m.homepage()->flat_list_index()));

            if (-1 != m.license()->flat_list_index() && ! m.license()->name().empty())
                id->load_license(m.license(), lines.at(m.license()->flat_list_index()));

            if (-1 != m.short_description()->flat_list_index() && ! m.short_description()->name().empty())
                id->load_short_description(m.short_description()->name(),
                        m.short_description()->description(),
                        lines.at(m.short_description()->flat_list_index()));

            if (-1 != m.long_description()->flat_list_index() && ! m.long_description()->name().empty())
            {
                std::string value(lines.at(m.long_description()->flat_list_index()));
                if (! value.empty())
                    id->load_long_description(m.long_description()->name(),
                            m.long_description()->description(), value);
            }

            if (-1 != m.keywords()->flat_list_index() && ! m.keywords()->name().empty())
                id->load_keywords(m.keywords(), lines.at(m.keywords()->flat_list_index()));

            if (-1 != m.inherited()->flat_list_index() && ! m.inherited()->name().empty())
                id->load_inherited(m.inherited(), lines.at(m.inherited()->flat_list_index()));

            if (-1 != m.defined_phases()->flat_list_index() && ! m.defined_phases()->name().empty())
                if (! lines.at(m.defined_phases()->flat_list_index()).empty())
                    id->load_defined_phases(m.defined_phases(), lines.at(m.defined_phases()->flat_list_index()));

            if (-1 != m.iuse()->flat_list_index() && ! m.iuse()->name().empty())
                id->load_iuse(m.iuse(), lines.at(m.iuse()->flat_list_index()));

            if (-1 != m.myoptions()->flat_list_index() && ! m.myoptions()->name().empty())
                id->load_myoptions(m.myoptions()->name(), m.myoptions()->description(), lines.at(m.myoptions()->flat_list_index()));

            if (-1 != m.required_use()->flat_list_index() && ! m.required_use()->name().empty())
                id->load_required_use(m.required_use()->name(), m.required_use()->description(), lines.at(m.required_use()->flat_list_index()));

            if (-1 != m.pdepend()->flat_list_index() && ! m.pdepend()->name().empty())
                id->load_post_depend(m.pdepend()->name(), m.pdepend()->description(), lines.at(m.pdepend()->flat_list_index()), false);

            if (-1 != m.provide()->flat_list_index() && ! m.provide()->name().empty())
                id->load_provide(m.provide(), lines.at(m.provide()->flat_list_index()));

            if (-1 != m.use()->flat_list_index() && ! m.use()->name().empty())
                id->load_use(m.use(), lines.at(m.use()->flat_list_index()));

            if (-1 != m.generated_from()->flat_list_index() && ! m.generated_from()->name().empty())
                id->load_generated_from(m.generated_from(), lines.at(m.generated_from()->flat_list_index()));

            if (-1 != m.generated_time()->flat_list_index() && ! m.generated_time()->name().empty())
                id->load_generated_time(m.generated_time()->name(), m.generated_time()->description(), lines.at(m.generated_time()->flat_list_index()));

            if (-1 != m.generated_using()->flat_list_index() && ! m.generated_using()->name().empty())
                id->load_generated_using(m.generated_using()->name(), m.generated_using()->description(), lines.at(m.generated_using()->flat_list_index()));

            if (-1 != m.upstream_changelog()->flat_list_index() && ! m.upstream_changelog()->name().empty())
            {
                std::string value(lines.at(m.upstream_changelog()->flat_list_index()));
                if (! value.empty())
                    id->load_upstream_changelog(m.upstream_changelog()->name(),
                            m.upstream_changelog()->description(), value);
            }

            if (-1 != m.upstream_documentation()->flat_list_index() && ! m.upstream_documentation()->name().empty())
            {
                std::string value(lines.at(m.upstream_documentation()->flat_list_index()));
                if (! value.empty())
                    id->load_upstream_documentation(m.upstream_documentation()->name(),
                            m.upstream_documentation()->description(), value);
            }

            if (-1 != m.upstream_release_notes()->flat_list_index() && ! m.upstream_release_notes()->name().empty())
            {
                std::string value(lines.at(m.upstream_release_notes()->flat_list_index()));
                if (! value.empty())
                    id->load_upstream_release_notes(m.upstream_release_notes()->name(),
                            m.upstream_release_notes()->description(), value);
            }

            if (-1 != m.bugs_to()->flat_list_index() && ! m.bugs_to()->name().empty())
            {
                std::string value(lines.at(m.bugs_to()->flat_list_index()));
                if (! value.empty())
                    id->load_bugs_to(m.bugs_to(), value);
            }

            if (-1 != m.remote_ids()->flat_list_index() && ! m.remote_ids()->name().empty())
            {
                std::string value(lines.at(m.remote_ids()->flat_list_index()));
                if (! value.empty())
                    id->load_remote_ids(m.remote_ids(), value);
            }
        }

        Log::get_instance()->message("e.cache.success", ll_debug, lc_context) << "Successfully loaded cache file";
        return true;
    }
}

EbuildFlatMetadataCache::EbuildFlatMetadataCache(const Environment * const v, const FSPath & f,
        const FSPath & e, std::time_t t, const std::shared_ptr<const EclassMtimes> & m, bool s) :
    Pimp<EbuildFlatMetadataCache>(v, f, e, t, m, s)
{
}

EbuildFlatMetadataCache::~EbuildFlatMetadataCache()
{
}

bool
EbuildFlatMetadataCache::load(const std::shared_ptr<const EbuildID> & id, const bool silent_on_stale)
{
    using namespace std::placeholders;

    Context context("When loading version metadata from '" + stringify(_imp->filename) + "':");

    if (! _imp->filename_stat.exists())
    {
        Log::get_instance()->message("e.cache.failure", _imp->silent ? ll_debug : ll_warning, lc_no_context)
                << "Couldn't use the cache file at '" << _imp->filename << "': " << std::strerror(errno);
        return false;
    }

    SafeIFStream cache(_imp->filename);

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(cache, line))
        lines.push_back(line);

    try
    {
        std::map<std::string, std::string> keys;
        std::string duplicate;
        for (std::vector<std::string>::const_iterator it(lines.begin()),
                 it_end(lines.end()); it_end != it; ++it)
        {
            std::string::size_type equals(it->find('='));
            if (std::string::npos == equals)
            {
                Log::get_instance()->message("e.cache.flat_hash.not", ll_debug, lc_context)
                    << "cache file lacks = on line " << ((it - lines.begin()) + 1) << ", assuming flat_list";
                return load_flat_list(id, lines, _imp.get());
            }

            if (! keys.insert(std::make_pair(it->substr(0, equals), it->substr(equals + 1))).second)
                duplicate = it->substr(0, equals);
        }

        Context ctx("When loading flat_hash format cache file:");

        if (! duplicate.empty())
        {
            Log::get_instance()->message("e.cache.flat_hash.broken", ll_warning, lc_context)
                << "cache file contains duplicate key '" << duplicate << "'";
            return false;
        }

        std::map<std::string, std::string>::const_iterator eapi(keys.find("EAPI"));
        if (keys.end() == eapi)
        {
            Log::get_instance()->message("e.cache.flat_hash.broken", ll_warning, lc_context)
                << "cache file contains no 'EAPI' key";
            return false;
        }
        id->set_eapi(eapi->second);

        if (id->eapi()->supported())
        {
            const EAPIEbuildMetadataVariables & m(*id->eapi()->supported()->ebuild_metadata_variables());
            std::vector<std::string> inherited;

            {
                std::map<std::string, std::string>::const_iterator mtime_it(keys.find("_mtime_"));
                std::time_t cache_time(keys.end() == mtime_it ? _imp->filename_stat.mtim().seconds() : destringify<std::time_t>(mtime_it->second));
                bool ok(_imp->ebuild_stat.mtim().seconds() == cache_time);
                if (! ok)
                    Log::get_instance()->message("e.cache.flat_hash.mtime", ll_debug, lc_context)
                        << "ebuild has mtime " << _imp->ebuild_stat.mtim().seconds() << ", but expected " << cache_time;

                if (ok) {
                    std::string cache_guessed(keys["_guessed_eapi_"]);
                    if (cache_guessed.empty())
                        cache_guessed = "0";
                    if (id->guessed_eapi_name() != cache_guessed)
                    {
                        Log::get_instance()->message("e.cache.flat_hash.guessed_eapi", ll_debug, lc_context)
                            << "ebuild has guessed EAPI '" << id->guessed_eapi_name() << "', but cache has '" << cache_guessed << "'";
                        ok = false;
                    }
                }

                if (ok && id->eapi()->supported()->ebuild_options()->support_eclasses())
                {
                    std::vector<std::string> eclasses;
                    tokenise<delim_kind::AnyOfTag, delim_mode::DelimiterTag>(keys["_eclasses_"], "\t", "", std::back_inserter(eclasses));
                    auto repo(_imp->env->package_database()->fetch_repository(id->repository_name()));
                    FSPath eclassdir((repo->location_key()->value() / "eclass").realpath_if_exists());
                    for (std::vector<std::string>::const_iterator it(eclasses.begin()),
                             it_end(eclasses.end()); it_end != it; ++it)
                    {
                        std::string eclass_name(*it);
                        inherited.push_back(eclass_name);
                        if (eclasses.end() == ++it)
                        {
                            Log::get_instance()->message("e.cache.flat_hash.eclass.truncated", ll_warning, lc_context)
                                << "_eclasses_ entry is incomplete";
                            return false;
                        }
                        FSPath eclass_dir(std::string::npos != it->find('/') ? FSPath(*it++) : eclassdir);
                        if (eclasses.end() == it)
                        {
                            Log::get_instance()->message("e.cache.flat_hash.eclass.truncated", ll_warning, lc_context)
                                << "_eclasses_ entry is incomplete";
                            return false;
                        }
                        std::time_t eclass_mtime(destringify<std::time_t>(*it));

                        auto eclass(_imp->eclass_mtimes->eclass(eclass_name));
                        if (eclass)
                            Log::get_instance()->message("e.cache.flat_hash.eclass.path", ll_debug, lc_context)
                                << "Cache-requested eclass '" << eclass_name << "' maps to '" << eclass->first << "'";

                        if (! eclass)
                        {
                            Log::get_instance()->message("e.cache.flat_hash.eclass.missing", ll_debug, lc_context)
                                << "Can't find cache-requested eclass '" << eclass_name << "'";
                            ok = false;
                        }

                        else if (eclass->first.dirname() != eclass_dir)
                        {
                            Log::get_instance()->message("e.cache.flat_hash.eclass.wrong_location", ll_debug, lc_context)
                                << "Cache-requested eclass '" << eclass_name << "' was found at '"
                                << eclass->first.dirname() << "', but expected '" << eclass_dir << "'";
                            ok = false;
                        }

                        else if (eclass->second.mtim().seconds() != eclass_mtime)
                        {
                            Log::get_instance()->message("e.cache.flat_hash.eclass.wrong_mtime", ll_debug, lc_context)
                                << "Cache-requested eclass '" << eclass_name << "' has mtime "
                                << eclass->second.mtim().seconds() << ", but expected " << eclass_mtime;
                            ok = false;
                        }

                        if (! ok)
                            break;
                    }
                }

                else if (ok && id->eapi()->supported()->ebuild_options()->support_exlibs())
                {
                    std::vector<std::string> exlibs;
                    tokenise<delim_kind::AnyOfTag, delim_mode::DelimiterTag>(keys["_exlibs_"], "\t", "", std::back_inserter(exlibs));
                    for (std::vector<std::string>::const_iterator it(exlibs.begin()),
                             it_end(exlibs.end()); it_end != it; ++it)
                    {
                        std::string exlib_name(*it);
                        inherited.push_back(exlib_name);
                        if (exlibs.end() == ++it)
                        {
                            Log::get_instance()->message("e.cache.flat_hash.exlib.truncated", ll_warning, lc_context)
                                << "_exlibs_ entry is incomplete";
                            return false;
                        }
                        FSPath exlib_dir(*it);
                        if (exlibs.end() == ++it)
                        {
                            Log::get_instance()->message("e.cache.flat_hash.exlibs.truncated", ll_warning, lc_context)
                                << "_exlibs_ entry is incomplete";
                            return false;
                        }
                        std::time_t exlib_mtime(destringify<std::time_t>(*it));

                        auto exlib(_imp->eclass_mtimes->exlib(exlib_name, id->name()));
                        if (exlib)
                            Log::get_instance()->message("e.cache.flat_hash.exlib.path", ll_debug, lc_context)
                                << "Cache-requested exlib '" << exlib_name << "' maps to '" << exlib->first << "'";

                        if (! exlib)
                        {
                            Log::get_instance()->message("e.cache.flat_hash.exlib.missing", ll_debug, lc_context)
                                << "Can't find cache-requested exlib '" << exlib_name << "'";
                            ok = false;
                        }

                        else if (exlib->first.dirname() != exlib_dir)
                        {
                            Log::get_instance()->message("e.cache.flat_hash.exlib.wrong_location", ll_debug, lc_context)
                                << "Cache-requested exlib '" << exlib_name << "' was found at '"
                                << exlib->first.dirname() << "', but expected '" << exlib_dir << "'";
                            ok = false;
                        }

                        else if (exlib->second.mtim().seconds() != exlib_mtime)
                        {
                            Log::get_instance()->message("e.cache.flat_hash.exlib.wrong_mtime", ll_debug, lc_context)
                                << "Cache-requested exlib '" << exlib_name << "' has mtime "
                                << exlib->second.mtim().seconds() << ", but expected " << exlib_mtime;
                            ok = false;
                        }

                        if (! ok)
                            break;
                    }
                }

                if (! ok)
                {
                    if (! silent_on_stale)
                        Log::get_instance()->message("e.cache.stale", ll_warning, lc_no_context)
                            << "Stale cache file at '" << _imp->filename << "'";
                    return false;
                }
            }

            if (! m.dependencies()->name().empty())
            {
                DependenciesRewriter rewriter;
                parse_depend(keys[m.dependencies()->name()], _imp->env, *id->eapi())->top()->accept(rewriter);
                id->load_raw_depend(m.dependencies()->name(), m.dependencies()->description(),
                        keys[m.dependencies()->name()]);
                id->load_build_depend(m.dependencies()->name() + ".DEPEND", m.dependencies()->description() + " (build)", rewriter.depend(), true);
                id->load_run_depend(m.dependencies()->name() + ".RDEPEND", m.dependencies()->description() + " (run)", rewriter.rdepend(), true);
                id->load_post_depend(m.dependencies()->name() + ".PDEPEND", m.dependencies()->description() + " (post)", rewriter.pdepend(), true);
            }

            if (! m.build_depend()->name().empty())
                id->load_build_depend(m.build_depend()->name(), m.build_depend()->description(), keys[m.build_depend()->name()], false);

            if (! m.run_depend()->name().empty())
                id->load_run_depend(m.run_depend()->name(), m.run_depend()->description(), keys[m.run_depend()->name()], false);

            id->load_slot(m.slot(), keys[m.slot()->name()]);

            if (! m.src_uri()->name().empty())
                id->load_src_uri(m.src_uri(), keys[m.src_uri()->name()]);

            if (! m.restrictions()->name().empty())
                id->load_restrict(m.restrictions(), keys[m.restrictions()->name()]);

            if (! m.properties()->name().empty())
                id->load_properties(m.properties(), keys[m.properties()->name()]);

            if (! m.homepage()->name().empty())
                id->load_homepage(m.homepage()->name(), m.homepage()->description(), keys[m.homepage()->name()]);

            if (! m.license()->name().empty())
                id->load_license(m.license(), keys[m.license()->name()]);

            if (! m.short_description()->name().empty())
                    id->load_short_description(m.short_description()->name(),
                            m.short_description()->description(),
                            keys[m.short_description()->name()]);

            if (! m.long_description()->name().empty())
            {
                std::string value(keys[m.long_description()->name()]);
                if (! value.empty())
                    id->load_long_description(m.long_description()->name(),
                            m.long_description()->description(), value);
            }

            if (! m.keywords()->name().empty())
                id->load_keywords(m.keywords(), keys[m.keywords()->name()]);

            if (! m.inherited()->name().empty())
                id->load_inherited(m.inherited(), join(inherited.begin(), inherited.end(), " "));

            if (! m.defined_phases()->name().empty())
                if (! keys[m.defined_phases()->name()].empty())
                    id->load_defined_phases(m.defined_phases(), keys[m.defined_phases()->name()]);

            if (! m.iuse()->name().empty())
                id->load_iuse(m.iuse(), keys[m.iuse()->name()]);

            if (! m.myoptions()->name().empty())
                id->load_myoptions(m.myoptions()->name(), m.myoptions()->description(), keys[m.myoptions()->name()]);

            if (! m.required_use()->name().empty())
                id->load_required_use(m.required_use()->name(), m.required_use()->description(), keys[m.required_use()->name()]);

            if (! m.pdepend()->name().empty())
                id->load_post_depend(m.pdepend()->name(), m.pdepend()->description(), keys[m.pdepend()->name()], false);

            if (! m.provide()->name().empty())
                id->load_provide(m.provide(), keys[m.provide()->name()]);

            if (! m.use()->name().empty())
                id->load_use(m.use(), keys[m.use()->name()]);

            if (! m.generated_from()->name().empty())
                id->load_generated_from(m.generated_from(), keys[m.generated_from()->name()]);

            if (! m.generated_time()->name().empty())
                id->load_generated_time(m.generated_time()->name(), m.generated_time()->description(), keys[m.generated_time()->name()]);

            if (! m.generated_using()->name().empty())
                id->load_generated_using(m.generated_using()->name(), m.generated_using()->description(), keys[m.generated_using()->name()]);

            if (! m.upstream_changelog()->name().empty())
            {
                std::string value(keys[m.upstream_changelog()->name()]);
                if (! value.empty())
                    id->load_upstream_changelog(m.upstream_changelog()->name(),
                            m.upstream_changelog()->description(), value);
            }

            if (! m.upstream_documentation()->name().empty())
            {
                std::string value(keys[m.upstream_documentation()->name()]);
                if (! value.empty())
                    id->load_upstream_documentation(m.upstream_documentation()->name(),
                            m.upstream_documentation()->description(), value);
            }

            if (! m.upstream_release_notes()->name().empty())
            {
                std::string value(keys[m.upstream_release_notes()->name()]);
                if (! value.empty())
                    id->load_upstream_release_notes(m.upstream_release_notes()->name(),
                            m.upstream_release_notes()->description(), value);
            }

            if (! m.bugs_to()->name().empty())
            {
                std::string value(keys[m.bugs_to()->name()]);
                if (! value.empty())
                    id->load_bugs_to(m.bugs_to(), value);
            }

            if (! m.remote_ids()->name().empty())
            {
                std::string value(keys[m.remote_ids()->name()]);
                if (! value.empty())
                    id->load_remote_ids(m.remote_ids(), value);
            }
        }

        Log::get_instance()->message("e.cache.success", ll_debug, lc_context) << "Successfully loaded cache file";
        return true;
    }
    catch (const InternalError &)
    {
        throw;
    }
    catch (const Exception & e)
    {
        Log::get_instance()->message("e.cache.failure", ll_warning, lc_no_context) << "Not using cache file at '"
            << _imp->filename << "' due to exception '" << e.message() << "' (" << e.what() << ")";
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
        UnformattedPrettyPrinter f;
        SpecTreePrettyPrinter p(f, { });
        d->top()->accept(p);
        return stringify(p);
    }

    template <typename T_>
    void write_kv(std::ostream & stream, const std::string & key, const T_ & value)
    {
        std::string str_value(stringify(value));
        if (! str_value.empty())
            stream << key << "=" << str_value << std::endl;
    }
}

void
EbuildFlatMetadataCache::save(const std::shared_ptr<const EbuildID> & id)
{
    Context context("When saving version metadata to '" + stringify(_imp->filename) + "':");

    try
    {
        FSPath cat_dir(_imp->filename.dirname());
        FSPath repo_dir(cat_dir.dirname());
        FSPath main_dir(repo_dir.dirname());
        FSStat main_dir_stat(main_dir);

        if (! main_dir_stat.exists())
        {
            Log::get_instance()->message("e.cache.save.no_dir", ll_warning, lc_no_context) << "Directory '"
                << main_dir << "' does not exist, so cannot save cache file '" << _imp->filename << "' "
                << "(see the faq for why this directory will not be created automatically)";
            return;
        }

        if (repo_dir.mkdir(main_dir_stat.permissions(), { fspmkdo_ok_if_exists }))
            repo_dir.chmod(main_dir_stat.permissions());

        if (cat_dir.mkdir(main_dir_stat.permissions(), { fspmkdo_ok_if_exists }))
            cat_dir.chmod(main_dir_stat.permissions());
    }
    catch (const FSError & e)
    {
        Log::get_instance()->message("e.cache.save.failure", ll_warning, lc_no_context) << "Couldn't create cache directory: " << e.message();
        return;
    }

    if (! id->eapi()->supported())
    {
        Log::get_instance()->message("e.cache.save.eapi_unsupoprted", ll_warning, lc_no_context) << "Not writing cache file to '"
            << _imp->filename << "' because EAPI '" << id->eapi()->name() << "' is not supported";
        return;
    }

    std::ostringstream cache;
    write_kv(cache, "_mtime_", _imp->ebuild_stat.mtim().seconds());
    write_kv(cache, "_guessed_eapi_", id->guessed_eapi_name());

    if (id->eapi()->supported()->ebuild_options()->support_eclasses() && id->inherited_key())
    {
        std::vector<std::string> eclasses;
        for (Set<std::string>::ConstIterator it(id->inherited_key()->value()->begin()),
                 it_end(id->inherited_key()->value()->end()); it_end != it; ++it)
        {
            auto eclass(_imp->eclass_mtimes->eclass(*it));
            if (! eclass)
                throw InternalError(PALUDIS_HERE, "eclass '" + *it + "' disappeared?");
            eclasses.push_back(*it);
            eclasses.push_back(stringify(eclass->first.dirname()));
            eclasses.push_back(stringify(eclass->second.mtim().seconds()));
        }
        write_kv(cache, "_eclasses_", join(eclasses.begin(), eclasses.end(), "\t"));
    }

    else if (id->eapi()->supported()->ebuild_options()->support_exlibs() && id->inherited_key())
    {
        std::vector<std::string> exlibs;
        for (Set<std::string>::ConstIterator it(id->inherited_key()->value()->begin()),
                 it_end(id->inherited_key()->value()->end()); it_end != it; ++it)
        {
            auto exlib(_imp->eclass_mtimes->exlib(*it, id->name()));
            if (! exlib)
                throw InternalError(PALUDIS_HERE, "exlib '" + *it + "' for '" + stringify(id->name()) + "' disappeared?");
            exlibs.push_back(*it);
            exlibs.push_back(stringify(exlib->first.dirname()));
            exlibs.push_back(stringify(exlib->second.mtim().seconds()));
        }
        write_kv(cache, "_exlibs_", join(exlibs.begin(), exlibs.end(), "\t"));
    }

    try
    {
        const EAPIEbuildMetadataVariables & m(*id->eapi()->supported()->ebuild_metadata_variables());

        if (! m.dependencies()->name().empty())
        {
            std::string s;

            if (id->dependencies_key())
                s.append(flatten(id->dependencies_key()->value()));
            else
            {
                if (id->build_dependencies_key())
                    s.append(flatten(id->build_dependencies_key()->value()) + " ");
                if (id->run_dependencies_key())
                    s.append(flatten(id->run_dependencies_key()->value()) + " ");
                if (id->post_dependencies_key())
                    s.append(flatten(id->post_dependencies_key()->value()) + " ");
            }

            write_kv(cache, m.dependencies()->name(), s);
        }

        if (! m.use()->name().empty() && id->raw_use_key())
            write_kv(cache, m.use()->name(), join(id->raw_use_key()->value()->begin(), id->raw_use_key()->value()->end(), " "));

        if (! m.build_depend()->name().empty() && id->build_dependencies_key())
            write_kv(cache, m.build_depend()->name(), flatten(id->build_dependencies_key()->value()));

        if (! m.run_depend()->name().empty() && id->run_dependencies_key())
            write_kv(cache, m.run_depend()->name(), flatten(id->run_dependencies_key()->value()));

        if (! m.slot()->name().empty() && id->slot_key())
            write_kv(cache, m.slot()->name(), normalise(id->slot_key()->value()));

        if (! m.src_uri()->name().empty() && id->fetches_key())
            write_kv(cache, m.src_uri()->name(), flatten(id->fetches_key()->value()));

        if (! m.restrictions()->name().empty() && id->restrict_key())
            write_kv(cache, m.restrictions()->name(), flatten(id->restrict_key()->value()));

        if (! m.properties()->name().empty() && id->properties_key())
            write_kv(cache, m.properties()->name(), flatten(id->properties_key()->value()));

        if (! m.homepage()->name().empty() && id->homepage_key())
            write_kv(cache, m.homepage()->name(), flatten(id->homepage_key()->value()));

        if (! m.license()->name().empty() && id->license_key())
            write_kv(cache, m.license()->name(), flatten(id->license_key()->value()));

        if (! m.short_description()->name().empty() && id->short_description_key())
            write_kv(cache, m.short_description()->name(), normalise(id->short_description_key()->value()));

        if (! m.keywords()->name().empty() && id->keywords_key())
            write_kv(cache, m.keywords()->name(), join(id->keywords_key()->value()->begin(), id->keywords_key()->value()->end(), " "));

        if (! m.iuse()->name().empty() && id->raw_iuse_key())
            write_kv(cache, m.iuse()->name(), join(id->raw_iuse_key()->value()->begin(), id->raw_iuse_key()->value()->end(), " "));

        if (! m.myoptions()->name().empty() && id->raw_myoptions_key())
            write_kv(cache, m.myoptions()->name(), flatten(id->raw_myoptions_key()->value()));

        if (! m.required_use()->name().empty() && id->required_use_key())
            write_kv(cache, m.required_use()->name(), flatten(id->required_use_key()->value()));

        if (! m.pdepend()->name().empty() && id->post_dependencies_key())
            write_kv(cache, m.pdepend()->name(), flatten(id->post_dependencies_key()->value()));

        if (! m.provide()->name().empty() && id->provide_key())
            write_kv(cache, m.provide()->name(), flatten(id->provide_key()->value()));

        write_kv(cache, "EAPI", normalise(id->eapi()->name()));

        if (! m.long_description()->name().empty() && id->long_description_key())
            write_kv(cache, m.long_description()->name(), normalise(id->long_description_key()->value()));

        if (! m.bugs_to()->name().empty() && id->bugs_to_key())
            write_kv(cache, m.bugs_to()->name(), flatten(id->bugs_to_key()->value()));

        if (! m.remote_ids()->name().empty() && id->remote_ids_key())
            write_kv(cache, m.remote_ids()->name(), flatten(id->remote_ids_key()->value()));

        if (! m.generated_using()->name().empty() && id->generated_using_key())
            write_kv(cache, m.generated_using()->name(), id->generated_using_key()->value());

        if (! m.generated_time()->name().empty() && id->generated_time_key())
            write_kv(cache, m.generated_time()->name(), stringify(id->generated_time_key()->value().seconds()));

        if (! m.generated_from()->name().empty() && id->generated_from_key())
            write_kv(cache, m.generated_from()->name(), join(id->generated_from_key()->value()->begin(),
                        id->generated_from_key()->value()->end(), " "));

        if (! m.upstream_changelog()->name().empty() && id->upstream_changelog_key())
            write_kv(cache, m.upstream_changelog()->name(), flatten(id->upstream_changelog_key()->value()));

        if (! m.upstream_documentation()->name().empty() && id->upstream_documentation_key())
            write_kv(cache, m.upstream_documentation()->name(), flatten(id->upstream_documentation_key()->value()));

        if (! m.upstream_release_notes()->name().empty() && id->upstream_release_notes_key())
            write_kv(cache, m.upstream_release_notes()->name(), flatten(id->upstream_release_notes_key()->value()));

        if (! m.defined_phases()->name().empty() && id->defined_phases_key())
            write_kv(cache, m.defined_phases()->name(), join(id->defined_phases_key()->value()->begin(),
                        id->defined_phases_key()->value()->end(), " "));
    }
    catch (const InternalError &)
    {
        throw;
    }
    catch (const Exception & e)
    {
        Log::get_instance()->message("e.cache.save.failure", ll_warning, lc_no_context) << "Not writing cache file to '"
            << _imp->filename << "' due to exception '" << e.message() << "' (" << e.what() << ")";
        return;
    }

    try
    {
        {
            SafeOFStream cache_file(_imp->filename, -1, true);
            cache_file << cache.str();
        }
        _imp->filename.utime(Timestamp(_imp->ebuild_stat.mtim().seconds(), 0));
    }
    catch (const SafeOFStreamError & e)
    {
        Log::get_instance()->message("e.cache.save.failure", ll_warning, lc_no_context) << "Couldn't write cache file to '"
            << _imp->filename << "': " << e.message() + " (" + e.what() + ")";
    }
}

template class Pimp<EbuildFlatMetadataCache>;

