/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <list>
#include <paludis/config_file.hh>
#include <paludis/hashed_containers.hh>
#include <paludis/environments/paludis/paludis_config.hh>
#include <paludis/environments/paludis/paludis_environment.hh>
#include <paludis/match_package.hh>
#include <paludis/package_database.hh>
#include <paludis/query.hh>
#include <paludis/repository.hh>
#include <paludis/repositories/repository_maker.hh>
#include <paludis/util/collection_concrete.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/log.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/system.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/dir_iterator.hh>
#include <vector>

using namespace paludis;

typedef MakeHashedMap<std::string, bool>::Type HookPresentCache;

namespace paludis
{
    template<>
    struct Implementation<PaludisEnvironment>
    {
        mutable HookPresentCache hook_cache;
        std::tr1::shared_ptr<PaludisConfig> config;
        std::string paludis_command;
        std::list<UseConfigEntry> forced_use;

        Implementation(std::tr1::shared_ptr<PaludisConfig> c) :
            config(c),
            paludis_command("paludis")
        {
        }
    };
}

PaludisEnvironment::PaludisEnvironment(const std::string & s) :
    Environment(std::tr1::shared_ptr<PackageDatabase>(new PackageDatabase(this))),
    PrivateImplementationPattern<PaludisEnvironment>(new Implementation<PaludisEnvironment>(
                std::tr1::shared_ptr<PaludisConfig>(new PaludisConfig(this, s))))
{
    Context context("When loading paludis environment:");

    for (PaludisConfig::RepositoryIterator r(_imp->config->begin_repositories()),
            r_end(_imp->config->end_repositories()) ; r != r_end ; ++r)
    {
        std::string keys;
        if (Log::get_instance()->log_level() <= ll_debug)
        {
            if (r->keys)
                for (AssociativeCollection<std::string, std::string>::Iterator
                        i(r->keys->begin()), i_end(r->keys->end()) ; i != i_end ; ++i)
                {
                    if (! keys.empty())
                        keys.append(", ");
                    keys.append("'" + i->first + "'='" + i->second + "'");
                }
            else
                keys = "empty";

            Log::get_instance()->message(ll_debug, lc_context,
                    "Creating repository with format='" + r->format + "', importance='"
                    + stringify(r->importance) + "', keys " + keys);
        }

        package_database()->add_repository(
                RepositoryMaker::get_instance()->find_maker(r->format)(this, r->keys));
    }
}

PaludisEnvironment::~PaludisEnvironment()
{
}

bool
PaludisEnvironment::query_use(const UseFlagName & f, const PackageDatabaseEntry * e) const
{
    /* first check package database use masks... */
    const Repository * const repo((e ? package_database()->fetch_repository(e->repository).get() : 0));

    if (repo && repo->use_interface)
    {
        if (repo->use_interface->query_use_mask(f, e))
            return false;
        if (repo->use_interface->query_use_force(f, e))
            return true;
    }

    /* check use: forced use config */
    for (std::list<UseConfigEntry>::const_iterator
            u(_imp->forced_use.begin()), u_end(_imp->forced_use.end()) ; u != u_end ; ++u)
    {
        if (u->flag_name != f)
            continue;

        if (! match_package(*this, *u->dep_spec, *e))
            continue;

        Log::get_instance()->message(ll_debug, lc_no_context, "Forced use flag: "
                + stringify(u->flag_name) + ", state: "
                + ((u->flag_state == use_enabled) ? "enabled" : "disabled"));

        return u->flag_state == use_enabled;
    }

    /* check use: per package user config */
    if (e)
    {
        UseFlagState s(use_unspecified);

        for (PaludisConfig::UseConfigIterator
                u(_imp->config->begin_use_config(e->name)),
                u_end(_imp->config->end_use_config(e->name)) ;
                u != u_end ; ++u)
        {
            if (f != u->flag_name)
                continue;

            if (! match_package(*this, *u->dep_spec, *e))
                continue;

            switch (u->flag_state)
            {
                case use_enabled:
                    s = use_enabled;
                    continue;

                case use_disabled:
                    s = use_disabled;
                    continue;

                case use_unspecified:
                    continue;
            }

            throw InternalError(PALUDIS_HERE, "Bad state");
        }

        do
        {
            switch (s)
            {
                case use_enabled:
                    return true;

                case use_disabled:
                    return false;

                case use_unspecified:
                    continue;
            }
            throw InternalError(PALUDIS_HERE, "Bad state");
        } while (false);

        /* and the -* bit */
        for (PaludisConfig::PackageUseMinusStarIterator
                i(_imp->config->begin_package_use_prefixes_with_minus_star(e->name)),
                i_end(_imp->config->end_package_use_prefixes_with_minus_star(e->name)) ;
                i != i_end ; ++i)
        {
            if (! match_package(*this, *i->first, *e))
                continue;

            if (0 == i->second.compare(0, i->second.length(), stringify(f), 0, i->second.length()))
                return false;
        }
    }

    /* check use: set user config */
    if (e)
    {
        UseFlagState s(use_unspecified);

        for (PaludisConfig::SetUseConfigIterator
                u(_imp->config->begin_set_use_config()),
                u_end(_imp->config->end_set_use_config()) ;
                u != u_end ; ++u)
        {
            if (f != u->flag_name)
                continue;

            if (! match_package_in_heirarchy(*this, *u->dep_spec, *e))
                continue;

            switch (u->flag_state)
            {
                case use_enabled:
                    s = use_enabled;
                    continue;

                case use_disabled:
                    s = use_disabled;
                    continue;

                case use_unspecified:
                    continue;
            }

            throw InternalError(PALUDIS_HERE, "Bad state");
        }

        do
        {
            switch (s)
            {
                case use_enabled:
                    return true;

                case use_disabled:
                    return false;

                case use_unspecified:
                    continue;
            }
            throw InternalError(PALUDIS_HERE, "Bad state");
        } while (false);

        /* and the -* bit */
        for (PaludisConfig::SetUseMinusStarIterator
                i(_imp->config->begin_set_use_prefixes_with_minus_star()),
                i_end(_imp->config->end_set_use_prefixes_with_minus_star()) ;
                i != i_end ; ++i)
        {
            if (! match_package_in_heirarchy(*this, *i->dep_spec, *e))
                continue;

            if (0 == i->prefix.compare(0, i->prefix.length(), stringify(f), 0, i->prefix.length()))
                return false;
        }
    }

    /* check use: general user config */
    do
    {
        UseFlagState state(use_unspecified);

        for (PaludisConfig::DefaultUseIterator
                u(_imp->config->begin_default_use()),
                u_end(_imp->config->end_default_use()) ;
                u != u_end ; ++u)
            if (f == u->first)
                state = u->second;

        switch (state)
        {
            case use_enabled:
                return true;

            case use_disabled:
                return false;

            case use_unspecified:
                continue;
        }

        throw InternalError(PALUDIS_HERE, "bad state " + stringify(state));
    } while (false);

    /* and -* again. slight gotcha: "* -*" should not override use expand things. if it
     * does, USERLAND etc get emptied. */
    bool consider_minus_star(true);
    if (e && repo && repo->use_interface)
    {
        std::tr1::shared_ptr<const UseFlagNameCollection> prefixes(repo->use_interface->use_expand_prefixes());
        for (UseFlagNameCollection::Iterator i(prefixes->begin()), i_end(prefixes->end()) ;
                i != i_end ; ++i)
            if (0 == i->data().compare(0, i->data().length(), stringify(f), 0, i->data().length()))
            {
                consider_minus_star = false;
                break;
            }
    }

    for (PaludisConfig::UseMinusStarIterator
            i(_imp->config->begin_use_prefixes_with_minus_star()),
            i_end(_imp->config->end_use_prefixes_with_minus_star()) ;
            i != i_end ; ++i)
    {
        if ((! consider_minus_star) && i->empty())
            continue;
        if (0 == i->compare(0, i->length(), stringify(f), 0, i->length()))
            return false;
    }

    /* check use: package database config */
    if (repo && repo->use_interface)
    {
        switch (repo->use_interface->query_use(f, e))
        {
            case use_disabled:
            case use_unspecified:
                return false;

            case use_enabled:
                return true;
        }

        throw InternalError(PALUDIS_HERE, "bad state");
    }
    else
    {
        return false;
    }
}

bool
PaludisEnvironment::accept_keyword(const KeywordName & keyword, const PackageDatabaseEntry * const d,
        const bool override_tilde_keywords) const
{
    static KeywordName star_keyword("*");
    static KeywordName minus_star_keyword("-*");

    if (keyword == star_keyword)
        return true;

    Context context("When checking accept_keyword of '" + stringify(keyword) +
            (d ? "' for " + stringify(*d) : stringify("'")) + ":");

    bool result(false);

    if (keyword != minus_star_keyword)
    {
        result |= _imp->config->end_default_keywords() !=
            std::find(_imp->config->begin_default_keywords(),
                    _imp->config->end_default_keywords(),
                    keyword);
    }

    result |= _imp->config->end_default_keywords() !=
        std::find(_imp->config->begin_default_keywords(),
                _imp->config->end_default_keywords(),
                star_keyword);

    if (d)
    {
        for (PaludisConfig::SetKeywordsIterator
                k(_imp->config->begin_set_keywords()),
                k_end(_imp->config->end_set_keywords()) ;
                k != k_end ; ++k)
        {
            if (! match_package_in_heirarchy(*this, *k->dep_spec, *d))
                continue;

            if (k->keyword == minus_star_keyword)
                result = false;
            else
            {
                result |= k->keyword == keyword;
                result |= k->keyword == star_keyword;
            }
        }

        for (PaludisConfig::PackageKeywordsIterator
                k(_imp->config->begin_package_keywords(d->name)),
                k_end(_imp->config->end_package_keywords(d->name)) ;
                k != k_end ; ++k)
        {
            if (! match_package(*this, *k->first, *d))
                continue;

            if (k->second == minus_star_keyword)
                result = false;
            else
            {
                result |= k->second == keyword;
                result |= k->second == star_keyword;
            }
        }

    }

    if ((! result) && override_tilde_keywords && ('~' == stringify(keyword).at(0)))
        result = accept_keyword(KeywordName(stringify(keyword).substr(1)), d, false);

    return result;
}

bool
PaludisEnvironment::accept_license(const std::string & license, const PackageDatabaseEntry * const d) const
{
    if (license == "*")
        return true;
    if (license == "-*")
        return false;

    Context context("When checking license of '" + license +
            (d ? "' for " + stringify(*d) : stringify("'")) + ":");

    bool result(false);

    result |= _imp->config->end_default_licenses() !=
        std::find(_imp->config->begin_default_licenses(),
                _imp->config->end_default_licenses(),
                license);

    result |= _imp->config->end_default_licenses() !=
        std::find(_imp->config->begin_default_licenses(),
                _imp->config->end_default_licenses(),
                "*");

    if (d)
    {
        for (PaludisConfig::SetLicensesIterator
                k(_imp->config->begin_set_licenses()),
                k_end(_imp->config->end_set_licenses()) ;
                k != k_end ; ++k)
        {
            if (! match_package_in_heirarchy(*this, *k->dep_spec, *d))
                continue;

            if (k->license == "-*")
                result = false;
            else
            {
                result |= k->license == license;
                result |= k->license == "*";
            }
        }

        for (PaludisConfig::PackageLicensesIterator
                k(_imp->config->begin_package_licenses(d->name)),
                k_end(_imp->config->end_package_licenses(d->name)) ;
                k != k_end ; ++k)
        {
            if (! match_package(*this, *k->first, *d))
                continue;

            if (k->second == "-*")
                result = false;
            else
            {
                result |= k->second == license;
                result |= k->second == "*";
            }
        }
    }

    return result;
}

bool
PaludisEnvironment::query_user_masks(const PackageDatabaseEntry & d) const
{
    for (PaludisConfig::UserMasksIterator
            k(_imp->config->begin_user_masks(d.name)),
            k_end(_imp->config->end_user_masks(d.name)) ;
            k != k_end ; ++k)
    {
        if (! match_package(*this, *k, d))
            continue;

        return true;
    }

    for (PaludisConfig::UserMasksSetsIterator
            k(_imp->config->begin_user_masks_sets()),
            k_end(_imp->config->end_user_masks_sets()) ;
            k != k_end ; ++k)
    {
        if (! match_package_in_heirarchy(*this, *k->dep_spec, d))
            continue;

        return true;
    }

    return false;
}

bool
PaludisEnvironment::query_user_unmasks(const PackageDatabaseEntry & d) const
{
    for (PaludisConfig::UserMasksIterator
            k(_imp->config->begin_user_unmasks(d.name)),
            k_end(_imp->config->end_user_unmasks(d.name)) ;
            k != k_end ; ++k)
    {
        if (! match_package(*this, *k, d))
            continue;

        return true;
    }

    for (PaludisConfig::UserMasksSetsIterator
            k(_imp->config->begin_user_unmasks_sets()),
            k_end(_imp->config->end_user_unmasks_sets()) ;
            k != k_end ; ++k)
    {
        if (! match_package_in_heirarchy(*this, *k->dep_spec, d))
            continue;

        return true;
    }

    return false;
}

std::string
PaludisEnvironment::bashrc_files() const
{
    return _imp->config->bashrc_files();
}

std::string
PaludisEnvironment::paludis_command() const
{
    return _imp->paludis_command;
}

void
PaludisEnvironment::set_paludis_command(const std::string & s)
{
    _imp->paludis_command = s;
}

namespace
{
    void add_one_hook(const std::string & base, std::list<FSEntry> & result)
    {
        try
        {
            FSEntry r(base);
            if (r.is_directory())
            {
                Log::get_instance()->message(ll_debug, lc_no_context, "Adding hook directory '"
                        + base + "'");
                result.push_back(r);
            }
            else
                Log::get_instance()->message(ll_debug, lc_no_context, "Skipping hook directory candidate '"
                        + base + "'");
        }
        catch (const FSError & e)
        {
            Log::get_instance()->message(ll_warning, lc_no_context, "Caught exception '" +
                    e.message() + "' (" + e.what() + ") when checking hook "
                    "directory '" + base + "'");
        }
    }

    const std::list<FSEntry> & get_hook_dirs(const std::string & c)
    {
        static std::list<FSEntry> result;
        static bool done_hooks(false);
        if (! done_hooks)
        {
            add_one_hook(c + "/hooks", result);
            if (getenv_with_default("PALUDIS_NO_GLOBAL_HOOKS", "").empty())
            {
                add_one_hook(LIBEXECDIR "/paludis/hooks", result);
                add_one_hook(DATADIR "/paludis/hooks", result);
            }
            done_hooks = true;
        }
        return result;
    }

    struct Hooker
    {
        Hook hook;
        std::string paludis_command;
        std::string root;

        Hooker(const Hook & h, const std::string & p, const std::string & r) :
            hook(h),
            paludis_command(p),
            root(r)
        {
        }

        int operator() (const FSEntry & f) const;
    };

    int
    Hooker::operator() (const FSEntry & f) const
    {
        Context context("When running hook script '" + stringify(f) +
                "' for hook '" + hook.name() + "':");
        Log::get_instance()->message(ll_debug, lc_no_context, "Starting hook script '" +
                stringify(f) + "' for '" + hook.name() + "'");

        Command cmd(Command("bash '" + stringify(f) + "'")
                .with_setenv("ROOT", root)
                .with_setenv("HOOK", hook.name())
                .with_setenv("HOOK_LOG_LEVEL", stringify(Log::get_instance()->log_level()))
                .with_setenv("PALUDIS_EBUILD_DIR", getenv_with_default("PALUDIS_EBUILD_DIR", LIBEXECDIR "/paludis"))
                .with_setenv("PALUDIS_COMMAND", paludis_command));

        for (Hook::Iterator h(hook.begin()), h_end(hook.end()) ; h != h_end ; ++h)
            cmd.with_setenv(h->first, h->second);

        int exit_status(run_command(cmd));
        if (0 == exit_status)
            Log::get_instance()->message(ll_debug, lc_no_context, "Hook '" + stringify(f)
                    + "' returned success '" + stringify(exit_status) + "'");
        else
            Log::get_instance()->message(ll_warning, lc_no_context, "Hook '" + stringify(f)
                    + "' returned failure '" + stringify(exit_status) + "'");
        return exit_status;
    }
}

int
PaludisEnvironment::perform_hook(const Hook & hook) const
{
    HookPresentCache::iterator cache_entry(_imp->hook_cache.end());
    if (_imp->hook_cache.end() != ((cache_entry = _imp->hook_cache.find(hook.name()))))
        if (! cache_entry->second)
            return 0;

    Context context("When triggering hook '" + hook.name() + "'");
    Log::get_instance()->message(ll_debug, lc_no_context, "Starting hook '" + hook.name() + "'");

    const std::list<FSEntry> & hook_dirs_ref(get_hook_dirs(_imp->config->config_dir()));

    bool had_hook(false);
    int max_exit_status(0);
    for (std::list<FSEntry>::const_iterator h(hook_dirs_ref.begin()),
            h_end(hook_dirs_ref.end()) ; h != h_end ; ++h)
    {
        FSEntry hh(*h / hook.name());
        if (! hh.is_directory())
            continue;

        std::list<FSEntry> hooks;
        std::copy(DirIterator(hh), DirIterator(),
                filter_inserter(std::back_inserter(hooks), IsFileWithExtension(".bash")));

        if (! hooks.empty())
            had_hook = true;

        for (std::list<FSEntry>::const_iterator hk(hooks.begin()),
                hk_end(hooks.end()) ; hk != hk_end ; ++hk)
             max_exit_status = std::max(max_exit_status, Hooker(hook, paludis_command(),
                         stringify(root()))(*hk));
    }

    if (_imp->hook_cache.end() == cache_entry)
        _imp->hook_cache.insert(std::make_pair(hook.name(), had_hook));

    return max_exit_status;
}

std::string
PaludisEnvironment::hook_dirs() const
{
    const std::list<FSEntry> & hook_dirs_ref(get_hook_dirs(_imp->config->config_dir()));
    return join(hook_dirs_ref.begin(), hook_dirs_ref.end(), " ");
}

std::string
PaludisEnvironment::fetchers_dirs() const
{
    std::string dirs(stringify(FSEntry(_imp->config->config_dir()) / "fetchers"));
    if (getenv_with_default("PALUDIS_NO_GLOBAL_FETCHERS", "").empty())
        dirs += " " + Environment::fetchers_dirs();
    return dirs;
}

std::string
PaludisEnvironment::syncers_dirs() const
{
    std::string dirs(stringify(FSEntry(_imp->config->config_dir()) / "syncers"));
    if (getenv_with_default("PALUDIS_NO_GLOBAL_SYNCERS", "").empty())
        dirs += " " + Environment::syncers_dirs();
    return dirs;
}

std::tr1::shared_ptr<CompositeDepSpec>
PaludisEnvironment::local_package_set(const SetName & s) const
{
    Context context("When looking for package set '" + stringify(s) + "' in paludis environment:");

    FSEntry ff(FSEntry(_imp->config->config_dir()) / "sets" / (stringify(s) + ".conf"));
    if (ff.exists())
    {
        LineConfigFile f(ff);
        std::tr1::shared_ptr<AllDepSpec> result(new AllDepSpec);
        std::tr1::shared_ptr<GeneralSetDepTag> tag(new GeneralSetDepTag(s, stringify(s) + ".conf"));

        for (LineConfigFile::Iterator line(f.begin()), line_end(f.end()) ;
                line != line_end ; ++line)
        {
            std::vector<std::string> tokens;
            WhitespaceTokeniser::get_instance()->tokenise(*line, std::back_inserter(tokens));
            if (tokens.empty())
                continue;

            if (1 == tokens.size())
            {
                Log::get_instance()->message(ll_warning, lc_context, "Line '" + *line + "' in set file '"
                        + stringify(ff) + "' does not specify '*' or '?', assuming '*'");
                std::tr1::shared_ptr<PackageDepSpec> spec(new PackageDepSpec(tokens.at(0)));
                spec->set_tag(tag);
                result->add_child(spec);
            }
            else if ("*" == tokens.at(0))
            {
                std::tr1::shared_ptr<PackageDepSpec> spec(new PackageDepSpec(tokens.at(1)));
                spec->set_tag(tag);
                result->add_child(spec);
            }
            else if ("?" == tokens.at(0))
            {
                std::tr1::shared_ptr<PackageDepSpec> p(new PackageDepSpec(tokens.at(1)));
                p->set_tag(tag);

                if (p->package_ptr())
                {
                    if (! package_database()->query(
                                query::Package(*p->package_ptr()) & query::InstalledAtRoot(root()),
                                qo_whatever)->empty())
                        result->add_child(p);
                }
                else
                    Log::get_instance()->message(ll_warning, lc_context, "Line '" + *line + "' in set file '"
                            + stringify(ff) + "' uses ? operator but does not specify an unambiguous package");
            }
            else
                Log::get_instance()->message(ll_warning, lc_context, "Line '" + *line + "' in set file '"
                        + stringify(ff) + "' does not start with '*' or '?' token, skipping");

            if (tokens.size() > 2)
                Log::get_instance()->message(ll_warning, lc_context, "Line '" + *line + "' in set file '"
                        + stringify(ff) + "' has trailing garbage");
        }

        return result;
    }

    return std::tr1::shared_ptr<AllDepSpec>();
}

std::tr1::shared_ptr<const SetsCollection>
PaludisEnvironment::sets_list() const
{
    std::tr1::shared_ptr<SetsCollection> result(new SetsCollection::Concrete);

    if ((FSEntry(_imp->config->config_dir()) / "sets").exists())
        for (DirIterator d(FSEntry(_imp->config->config_dir()) / "sets"), d_end ;
                d != d_end ; ++d)
        {
            if (! IsFileWithExtension(".conf")(*d))
                continue;

            result->insert(SetName(strip_trailing_string(d->basename(), ".conf")));
        }

    return result;
}

PaludisEnvironment::MirrorIterator
PaludisEnvironment::begin_mirrors(const std::string & mirror) const
{
    return _imp->config->begin_mirrors(mirror);
}

PaludisEnvironment::MirrorIterator
PaludisEnvironment::end_mirrors(const std::string & mirror) const
{
    return _imp->config->end_mirrors(mirror);
}

std::tr1::shared_ptr<const UseFlagNameCollection>
PaludisEnvironment::known_use_expand_names(const UseFlagName & prefix, const PackageDatabaseEntry * pde) const
{
    std::tr1::shared_ptr<UseFlagNameCollection> result(new UseFlagNameCollection::Concrete);

    std::string prefix_lower;
    std::transform(prefix.data().begin(), prefix.data().end(), std::back_inserter(prefix_lower), &::tolower);
    for (PaludisConfig::DefaultUseIterator i(_imp->config->begin_default_use()),
            i_end(_imp->config->end_default_use()) ; i != i_end ; ++i)
        if (i->first.data().length() > prefix_lower.length() &&
                0 == i->first.data().compare(0, prefix_lower.length(), prefix_lower, 0, prefix_lower.length()))
            result->insert(i->first);

    if (pde)
    {
        for (std::list<UseConfigEntry>::const_iterator i(_imp->forced_use.begin()),
                i_end(_imp->forced_use.end()) ; i != i_end ; ++i)
        {
            if (! i->dep_spec)
                continue;

            if (! match_package(*this, *i->dep_spec, *pde))
                continue;

            if (i->flag_name.data().length() > prefix_lower.length() &&
                    0 == i->flag_name.data().compare(0, prefix_lower.length(), prefix_lower, 0, prefix_lower.length()))
              result->insert(i->flag_name);
        }

        for (PaludisConfig::UseConfigIterator i(_imp->config->begin_use_config(pde->name)),
                i_end(_imp->config->end_use_config(pde->name)) ; i != i_end ; ++i)
            if (i->flag_name.data().length() > prefix_lower.length() &&
                    0 == i->flag_name.data().compare(0, prefix_lower.length(), prefix_lower, 0, prefix_lower.length()))
                result->insert(i->flag_name);
    }

    Log::get_instance()->message(ll_debug, lc_no_context, "PaludisEnvironment::known_use_expand_names("
            + stringify(prefix) + ", " + (pde ? stringify(*pde) : stringify("0")) + ") -> ("
            + join(result->begin(), result->end(), ", ") + ")");
    return result;
}

FSEntry
PaludisEnvironment::root() const
{
    return _imp->config->root();
}

uid_t
PaludisEnvironment::reduced_uid() const
{
    uid_t u(getuid());
    if (0 == u)
        return _imp->config->reduced_uid();
    else
        return u;
}

gid_t
PaludisEnvironment::reduced_gid() const
{
    gid_t g(getgid());
    if (0 == g)
        return _imp->config->reduced_gid();
    else
        return g;
}

void
PaludisEnvironment::force_use(std::tr1::shared_ptr<const PackageDepSpec> a,
        const UseFlagName & f, const UseFlagState s)
{
    _imp->forced_use.push_back(UseConfigEntry(a, f, s));
}

void
PaludisEnvironment::clear_forced_use()
{
    _imp->forced_use.clear();
}

std::string
PaludisEnvironment::config_dir() const
{
    return _imp->config->config_dir();
}

