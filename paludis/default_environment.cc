/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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
#include <paludis/default_config.hh>
#include <paludis/default_environment.hh>
#include <paludis/match_package.hh>
#include <paludis/package_database.hh>
#include <paludis/repository.hh>
#include <paludis/util/collection_concrete.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/log.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/system.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/dir_iterator.hh>
#include <vector>

using namespace paludis;

DefaultEnvironment::DefaultEnvironment() :
    Environment(PackageDatabase::Pointer(new PackageDatabase(this)))
{
    Context context("When loading default environment:");

    for (DefaultConfig::RepositoryIterator r(DefaultConfig::get_instance()->begin_repositories()),
            r_end(DefaultConfig::get_instance()->end_repositories()) ; r != r_end ; ++r)
        package_database()->add_repository(
                RepositoryMaker::get_instance()->find_maker(r->get<rce_format>())(
                    this, package_database().raw_pointer(), r->get<rce_keys>()));
}

DefaultEnvironment::~DefaultEnvironment()
{
}

bool
DefaultEnvironment::query_use(const UseFlagName & f, const PackageDatabaseEntry * e) const
{
    /* first check package database use masks... */
    const Repository * const repo((e ?
                package_database()->fetch_repository(e->get<pde_repository>()) :
                package_database()->fetch_repository(package_database()->favourite_repository())
                ).raw_pointer());

    if (repo->get_interface<repo_use>())
        if (repo->get_interface<repo_use>()->query_use_mask(f, e))
            return false;

    /* check use: per package user config */
    if (e)
    {
        UseFlagState s(use_unspecified);

        for (DefaultConfig::UseConfigIterator
                u(DefaultConfig::get_instance()->begin_use_config(e->get<pde_name>())),
                u_end(DefaultConfig::get_instance()->end_use_config(e->get<pde_name>())) ;
                u != u_end ; ++u)
        {
            if (f != u->get<uce_flag_name>())
                continue;

            if (! match_package(this, *u->get<uce_dep_atom>(), *e))
                continue;

            switch (u->get<uce_flag_state>())
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
    }

    /* check use: general user config */
    do
    {
        UseFlagState state(use_unspecified);

        for (DefaultConfig::DefaultUseIterator
                u(DefaultConfig::get_instance()->begin_default_use()),
                u_end(DefaultConfig::get_instance()->end_default_use()) ;
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

    /* check use: package database config */
    if (repo->get_interface<repo_use>())
    {
        switch (repo->get_interface<repo_use>()->query_use(f, e))
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
DefaultEnvironment::accept_keyword(const KeywordName & keyword, const PackageDatabaseEntry * const d) const
{
    if (keyword == KeywordName("*"))
        return true;

    Context context("When checking accept_keyword of '" + stringify(keyword) +
            (d ? "' for " + stringify(*d) : stringify("'")) + ":");

    bool result(false);

    if (d)
        for (DefaultConfig::PackageKeywordsIterator
                k(DefaultConfig::get_instance()->begin_package_keywords(d->get<pde_name>())),
                k_end(DefaultConfig::get_instance()->end_package_keywords(d->get<pde_name>())) ;
                k != k_end ; ++k)
        {
            if (! match_package(this, k->first, d))
                continue;

            result |= k->second == keyword;
        }

    result |= DefaultConfig::get_instance()->end_default_keywords() !=
        std::find(DefaultConfig::get_instance()->begin_default_keywords(),
                DefaultConfig::get_instance()->end_default_keywords(),
                keyword);

    return result;
}

bool
DefaultEnvironment::accept_license(const std::string & license, const PackageDatabaseEntry * const d) const
{
    if (license == "*")
        return true;

    Context context("When checking license of '" + license +
            (d ? "' for " + stringify(*d) : stringify("'")) + ":");

    bool result(false);

    if (d)
        for (DefaultConfig::PackageLicensesIterator
                k(DefaultConfig::get_instance()->begin_package_licenses(d->get<pde_name>())),
                k_end(DefaultConfig::get_instance()->end_package_licenses(d->get<pde_name>())) ;
                k != k_end ; ++k)
        {
            if (! match_package(this, k->first, d))
                continue;

            result |= k->second == license;
            result |= k->second == "*";
        }

    result |= DefaultConfig::get_instance()->end_default_licenses() !=
        std::find(DefaultConfig::get_instance()->begin_default_licenses(),
                DefaultConfig::get_instance()->end_default_licenses(),
                license);

    result |= DefaultConfig::get_instance()->end_default_licenses() !=
        std::find(DefaultConfig::get_instance()->begin_default_licenses(),
                DefaultConfig::get_instance()->end_default_licenses(),
                "*");

    return result;
}

bool
DefaultEnvironment::query_user_masks(const PackageDatabaseEntry & d) const
{
    for (DefaultConfig::UserMasksIterator
            k(DefaultConfig::get_instance()->begin_user_masks(d.get<pde_name>())),
            k_end(DefaultConfig::get_instance()->end_user_masks(d.get<pde_name>())) ;
            k != k_end ; ++k)
    {
        if (! match_package(this, *k, d))
            continue;

        return true;
    }

    return false;
}

bool
DefaultEnvironment::query_user_unmasks(const PackageDatabaseEntry & d) const
{
    for (DefaultConfig::UserMasksIterator
            k(DefaultConfig::get_instance()->begin_user_unmasks(d.get<pde_name>())),
            k_end(DefaultConfig::get_instance()->end_user_unmasks(d.get<pde_name>())) ;
            k != k_end ; ++k)
    {
        if (! match_package(this, *k, d))
            continue;

        return true;
    }

    return false;
}

std::string
DefaultEnvironment::bashrc_files() const
{
    return DefaultConfig::get_instance()->bashrc_files();
}

std::string
DefaultEnvironment::paludis_command() const
{
    return DefaultConfig::get_instance()->paludis_command();
}

UseFlagNameCollection::Pointer
DefaultEnvironment::query_enabled_use_matching(const std::string & prefix,
        const PackageDatabaseEntry * e) const
{
    UseFlagNameCollection::Pointer result(new UseFlagNameCollection::Concrete);

    for (DefaultConfig::DefaultUseIterator
            u(DefaultConfig::get_instance()->begin_default_use()),
            u_end(DefaultConfig::get_instance()->end_default_use()) ;
            u != u_end ; ++u)
    {
        if (0 != u->first.data().compare(0, prefix.length(), prefix))
            continue;

        switch (u->second)
        {
            case use_enabled:
                result->insert(u->first);
                break;

            case use_disabled:
                result->erase(u->first);
                break;

            case use_unspecified:
                break;
        }
    }

    if (e)
    {
        for (DefaultConfig::UseConfigIterator
                u(DefaultConfig::get_instance()->begin_use_config(e->get<pde_name>())),
                u_end(DefaultConfig::get_instance()->end_use_config(e->get<pde_name>())) ;
                u != u_end ; ++u)
        {
            if (0 != u->get<uce_flag_name>().data().compare(0, prefix.length(), prefix))
                continue;

            if (! match_package(this, *u->get<uce_dep_atom>(), *e))
                continue;

            switch (u->get<uce_flag_state>())
            {
                case use_enabled:
                    result->insert(u->get<uce_flag_name>());
                    break;

                case use_disabled:
                    result->erase(u->get<uce_flag_name>());
                    break;

                case use_unspecified:
                    break;
            }
        }
    }

    return result;
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

    const std::list<FSEntry> & get_hook_dirs()
    {
        static std::list<FSEntry> result;
        static bool done_hooks(false);
        if (! done_hooks)
        {
            add_one_hook(DefaultConfig::get_instance()->config_dir() + "/hooks", result);
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

        Hooker(const Hook & h, const std::string & p) :
            hook(h),
            paludis_command(p)
        {
        }

        void operator() (const FSEntry & f) const;
    };

    void
    Hooker::operator() (const FSEntry & f) const
    {
        Context context("When running hook script '" + stringify(f) +
                "' for hook '" + hook.name() + "':");
        Log::get_instance()->message(ll_debug, lc_no_context, "Starting hook script '" +
                stringify(f) + "' for '" + hook.name() + "'");

        MakeEnvCommand cmd(make_env_command("bash '" + stringify(f) + "'")
                ("ROOT", DefaultConfig::get_instance()->root())
                ("HOOK", hook.name())
                ("HOOK_LOG_LEVEL", Log::get_instance()->log_level_string())
                ("HOOK_CONFIG_SUFFIX", DefaultConfig::config_suffix())
                ("PALUDIS_EBUILD_DIR", getenv_with_default("PALUDIS_EBUILD_DIR", LIBEXECDIR "/paludis"))
                ("PALUDIS_COMMAND", paludis_command));

        for (Hook::Iterator h(hook.begin()), h_end(hook.end()) ; h != h_end ; ++h)
            cmd = cmd(h->first, h->second);

        int exit_status(run_command(cmd));
        if (0 == exit_status)
            Log::get_instance()->message(ll_debug, lc_no_context, "Hook '" + stringify(f)
                    + "' returned success '" + stringify(exit_status) + "'");
        else
            Log::get_instance()->message(ll_warning, lc_no_context, "Hook '" + stringify(f)
                    + "' returned failure '" + stringify(exit_status) + "'");
    }
}

void
DefaultEnvironment::perform_hook(const Hook & hook) const
{
    Context context("When triggering hook '" + hook.name() + "'");
    Log::get_instance()->message(ll_debug, lc_no_context, "Starting hook '" + hook.name() + "'");

    const std::list<FSEntry> & hook_dirs(get_hook_dirs());

    for (std::list<FSEntry>::const_iterator h(hook_dirs.begin()),
            h_end(hook_dirs.end()) ; h != h_end ; ++h)
    {
        FSEntry hh(*h / hook.name());
        if (! hh.is_directory())
            continue;

        std::list<FSEntry> hooks;
        std::copy(DirIterator(hh), DirIterator(),
                filter_inserter(std::back_inserter(hooks), IsFileWithExtension(".bash")));
        std::for_each(hooks.begin(), hooks.end(), Hooker(hook, paludis_command()));
    }
}

std::string
DefaultEnvironment::hook_dirs() const
{
    const std::list<FSEntry> & hook_dirs(get_hook_dirs());
    return join(hook_dirs.begin(), hook_dirs.end(), " ");
}

DepAtom::Pointer
DefaultEnvironment::local_package_set(const std::string & s,
        const PackageSetOptions &) const
{
    Context context("When looking for package set '" + s + "' in default environment:");

    FSEntry ff(FSEntry(DefaultConfig::get_instance()->config_dir()) / "sets" / (s + ".conf"));
    if (ff.exists())
    {
        LineConfigFile f(ff);
        AllDepAtom::Pointer result(new AllDepAtom);
        GeneralSetDepTag::Pointer tag(new GeneralSetDepTag(s));

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
                PackageDepAtom::Pointer atom(new PackageDepAtom(tokens.at(0)));
                atom->set_tag(tag);
                result->add_child(atom);
            }
            else if ("*" == tokens.at(0))
            {
                PackageDepAtom::Pointer atom(new PackageDepAtom(tokens.at(1)));
                atom->set_tag(tag);
                result->add_child(atom);
            }
            else if ("?" == tokens.at(0))
            {
                PackageDepAtom::Pointer p(new PackageDepAtom(tokens.at(1)));
                p->set_tag(tag);
                if (! package_database()->query(
                            PackageDepAtom::Pointer(new PackageDepAtom(p->package())),
                            is_installed_only)->empty())
                    result->add_child(p);
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

    return DepAtom::Pointer(0);
}

DefaultEnvironment::MirrorIterator
DefaultEnvironment::begin_mirrors(const std::string & mirror) const
{
    return DefaultConfig::get_instance()->begin_mirrors(mirror);
}

DefaultEnvironment::MirrorIterator
DefaultEnvironment::end_mirrors(const std::string & mirror) const
{
    return DefaultConfig::get_instance()->end_mirrors(mirror);
}

