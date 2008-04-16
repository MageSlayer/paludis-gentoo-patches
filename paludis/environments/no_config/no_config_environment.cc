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

#include "no_config_environment.hh"
#include <paludis/util/tokeniser.hh>
#include <paludis/util/log.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/map.hh>
#include <paludis/util/set.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/create_iterator-impl.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/kc.hh>
#include <paludis/repository_maker.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/config_file.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/distribution.hh>
#include <paludis/package_database.hh>
#include <paludis/hook.hh>
#include <paludis/repositories/e/e_repository_params.hh>
#include <algorithm>
#include <set>
#include <list>

using namespace paludis;
using namespace paludis::no_config_environment;

#include <paludis/environments/no_config/no_config_environment-sr.cc>

namespace paludis
{
    template<>
    struct Implementation<NoConfigEnvironment>
    {
        const no_config_environment::Params params;

        const FSEntry top_level_dir;
        const FSEntry write_cache;
        bool accept_unstable;
        bool is_vdb;

        tr1::shared_ptr<Repository> main_repo;
        tr1::shared_ptr<Repository> master_repo;

        std::string paludis_command;

        tr1::shared_ptr<PackageDatabase> package_database;

        Implementation(NoConfigEnvironment * const env, const no_config_environment::Params & params);
        void initialise(NoConfigEnvironment * const env);
    };

    /* This goat is for Dave Wickham */
}

namespace
{
    bool is_vdb_repository(const FSEntry & location, no_config_environment::RepositoryType type)
    {
        switch (type)
        {
            case ncer_ebuild:
                return false;
            case ncer_vdb:
                return true;
            case ncer_auto:
                ;
        }

        Context context("When determining repository type at '" + stringify(location) + "':");

        if (! location.is_directory())
            throw ConfigurationError("Location is not a directory");

        if ((location / "profiles").is_directory())
        {
            Log::get_instance()->message("no_config_environment.ebuild_detected", ll_debug, lc_context)
                << "Found profiles/, looks like Ebuild format";
            return false;
        }

        int outer_count(0);
        for (DirIterator d(location), d_end ; d != d_end ; ++d)
        {
            if (! d->is_directory())
                continue;

            int inner_count(0);
            for (DirIterator e(*d), e_end ; e != e_end ; ++e)
            {
                if (! e->is_directory())
                    continue;

                if ((*e / "CONTENTS").exists())
                {
                    Log::get_instance()->message("no_config_environment.vdb_detected", ll_debug, lc_context)
                        << "Found '" << stringify(*e) << "/CONTENTS', looks like VDB format";
                    return true;
                }

                if (inner_count++ >= 5)
                    break;
            }

            if (outer_count++ >= 5)
                break;
        }

        throw ConfigurationError("Can't work out what kind of repository this is");
    }
}

Implementation<NoConfigEnvironment>::Implementation(
        NoConfigEnvironment * const env, const no_config_environment::Params & p) :
    params(p),
    top_level_dir(p.repository_dir),
    write_cache(p.write_cache),
    accept_unstable(p.accept_unstable),
    is_vdb(is_vdb_repository(p.repository_dir, p.repository_type)),
    paludis_command("false"),
    package_database(new PackageDatabase(env))
{
}

void
Implementation<NoConfigEnvironment>::initialise(NoConfigEnvironment * const env)
{
    Context context("When initialising NoConfigEnvironment at '" + stringify(params.repository_dir) + "':");

    if (! is_vdb)
    {
        if (FSEntry("/var/empty") != params.master_repository_dir)
        {
            if (params.repository_dir.realpath() == params.master_repository_dir.realpath())
                Log::get_instance()->message("no_config_environment.master_repository.ignoring", ll_warning, lc_context)
                    << "Ignoring master_repository_dir '" << params.master_repository_dir
                    << "' because it is the same as repository_dir";

            else
            {
                tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);

                if (params.extra_params)
                    std::copy(params.extra_params->begin(), params.extra_params->end(), keys->inserter());

                keys->insert("format", "ebuild");
                keys->insert("location", stringify(params.master_repository_dir));
                keys->insert("profiles", "/var/empty");
                keys->insert("ignore_deprecated_profiles", "true");
                keys->insert("write_cache", stringify(params.write_cache));
                keys->insert("names_cache", "/var/empty");
                if (params.disable_metadata_cache)
                    keys->insert("cache", "/var/empty");

                package_database->add_repository(1, ((master_repo =
                                RepositoryMaker::get_instance()->find_maker("ebuild")(env, keys))));
            }
        }

        tr1::shared_ptr<Map<std::string, std::string> > keys( new Map<std::string, std::string>);

        if (params.extra_params)
            std::copy(params.extra_params->begin(), params.extra_params->end(), keys->inserter());

        keys->insert("format", "ebuild");
        keys->insert("location", stringify(params.repository_dir));
        keys->insert("profiles", "/var/empty");
        keys->insert("ignore_deprecated_profiles", "true");
        keys->insert("write_cache", stringify(params.write_cache));
        keys->insert("names_cache", "/var/empty");

        if (params.disable_metadata_cache)
            keys->insert("cache", "/var/empty");

        if (master_repo)
            keys->insert("master_repository", stringify(master_repo->name()));

        if ((params.repository_dir / "metadata" / "profiles_desc.conf").exists())
            keys->insert("layout", "exheres");

        package_database->add_repository(2, ((main_repo =
                        RepositoryMaker::get_instance()->find_maker("ebuild")(env, keys))));

        if ((*DistributionData::get_instance()->distribution_from_string(env->default_distribution()))[k::support_old_style_virtuals()])
            package_database->add_repository(-2, RepositoryMaker::get_instance()->find_maker("virtuals")(env,
                        tr1::shared_ptr<Map<std::string, std::string> >()));
    }
    else
    {
        Log::get_instance()->message("no_config_environment.vdb_detected", ll_debug, lc_context) << "VDB, using vdb_db";

        tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
        if (params.extra_params)
            std::copy(params.extra_params->begin(), params.extra_params->end(), keys->inserter());

        keys->insert("format", "vdb");
        keys->insert("names_cache", "/var/empty");
        keys->insert("provides_cache", "/var/empty");
        keys->insert("location", stringify(top_level_dir));

        package_database->add_repository(1, RepositoryMaker::get_instance()->find_maker("vdb")(env, keys));

        tr1::shared_ptr<Map<std::string, std::string> > iv_keys(
                new Map<std::string, std::string>);
        iv_keys->insert("root", "/");

        if ((*DistributionData::get_instance()->distribution_from_string(env->default_distribution()))[k::support_old_style_virtuals()])
            package_database->add_repository(-2, RepositoryMaker::get_instance()->find_maker("installed_virtuals")(env,
                        iv_keys));
    }
}

NoConfigEnvironment::NoConfigEnvironment(const no_config_environment::Params & params) :
    PrivateImplementationPattern<NoConfigEnvironment>(
            new Implementation<NoConfigEnvironment>(this, params))
{
    _imp->initialise(this);

    if (_imp->main_repo)
        if ((*_imp->main_repo)[k::e_interface()]->end_profiles() != (*_imp->main_repo)[k::e_interface()]->begin_profiles())
            (*_imp->main_repo)[k::e_interface()]->set_profile((*_imp->main_repo)[k::e_interface()]->begin_profiles());

    if (_imp->master_repo)
        if ((*_imp->master_repo)[k::e_interface()]->end_profiles() !=
                (*_imp->master_repo)[k::e_interface()]->begin_profiles())
            (*_imp->master_repo)[k::e_interface()]->set_profile(
                    (*_imp->master_repo)[k::e_interface()]->begin_profiles());
}

NoConfigEnvironment::~NoConfigEnvironment()
{
}

FSEntry
NoConfigEnvironment::main_repository_dir() const
{
    return _imp->top_level_dir;
}

void
NoConfigEnvironment::set_accept_unstable(const bool value)
{
    _imp->accept_unstable = value;
    for (PackageDatabase::RepositoryConstIterator it(_imp->package_database->begin_repositories()),
             it_end(_imp->package_database->end_repositories());
         it_end != it; ++it)
        (*it)->invalidate_masks();
}

tr1::shared_ptr<Repository>
NoConfigEnvironment::main_repository()
{
    return _imp->main_repo;
}

tr1::shared_ptr<const Repository>
NoConfigEnvironment::main_repository() const
{
    return _imp->main_repo;
}

tr1::shared_ptr<Repository>
NoConfigEnvironment::master_repository()
{
    return _imp->master_repo;
}

tr1::shared_ptr<const Repository>
NoConfigEnvironment::master_repository() const
{
    return _imp->master_repo;
}

tr1::shared_ptr<PackageDatabase>
NoConfigEnvironment::package_database()
{
    return _imp->package_database;
}

tr1::shared_ptr<const PackageDatabase>
NoConfigEnvironment::package_database() const
{
    return _imp->package_database;
}

std::string
NoConfigEnvironment::paludis_command() const
{
    return _imp->paludis_command;
}

void
NoConfigEnvironment::set_paludis_command(const std::string & s)
{
    _imp->paludis_command = s;
}

bool
NoConfigEnvironment::accept_keywords(tr1::shared_ptr<const KeywordNameSet> keywords,
        const PackageID &) const
{
    if (_imp->is_vdb)
        return true;

    std::string accept_keywords_var((*_imp->main_repo)[k::e_interface()]->accept_keywords_variable());
    std::string ak;
    if (! accept_keywords_var.empty())
        ak = (*_imp->main_repo)[k::e_interface()]->profile_variable(accept_keywords_var);

    if (ak.empty())
    {
        std::string arch_var((*_imp->main_repo)[k::e_interface()]->arch_variable());

        if (arch_var.empty())
            throw ConfigurationError("Don't know how to work out whether keywords are acceptable");

        std::string arch((*_imp->main_repo)[k::e_interface()]->profile_variable(arch_var));

        if (keywords->end() != keywords->find(KeywordName(arch)))
            return true;

        if (_imp->accept_unstable && keywords->end() != keywords->find(KeywordName("~" + arch)))
            return true;
    }
    else
    {
        std::list<KeywordName> accepted;
        tokenise_whitespace(ak, create_inserter<KeywordName>(std::back_inserter(accepted)));

        for (KeywordNameSet::ConstIterator k(keywords->begin()), k_end(keywords->end()) ;
                k != k_end ; ++k)
        {
            if (accepted.end() != std::find(accepted.begin(), accepted.end(), *k))
                return true;

            if (_imp->accept_unstable && stringify(*k).at(0) == '~')
                if (accepted.end() != std::find(accepted.begin(), accepted.end(), KeywordName(stringify(*k).substr(1))))
                    return true;
        }
    }

    return false;
}

tr1::shared_ptr<SetSpecTree::ConstItem>
NoConfigEnvironment::local_set(const SetName &) const
{
    return tr1::shared_ptr<SetSpecTree::ConstItem>();
}

tr1::shared_ptr<SetSpecTree::ConstItem>
NoConfigEnvironment::world_set() const
{
    return tr1::shared_ptr<SetSpecTree::ConstItem>();
}

void
NoConfigEnvironment::add_to_world(const QualifiedPackageName &) const
{
}

void
NoConfigEnvironment::remove_from_world(const QualifiedPackageName &) const
{
}

void
NoConfigEnvironment::add_to_world(const SetName &) const
{
}

void
NoConfigEnvironment::remove_from_world(const SetName &) const
{
}

bool
NoConfigEnvironment::unmasked_by_user(const PackageID &) const
{
    return false;
}

const tr1::shared_ptr<const Mask>
NoConfigEnvironment::mask_for_breakage(const PackageID &) const
{
    return tr1::shared_ptr<const Mask>();
}

const tr1::shared_ptr<const Mask>
NoConfigEnvironment::mask_for_user(const PackageID &) const
{
    return tr1::shared_ptr<const Mask>();
}

uid_t
NoConfigEnvironment::reduced_uid() const
{
    return getuid();
}

gid_t
NoConfigEnvironment::reduced_gid() const
{
    return getgid();
}

tr1::shared_ptr<const MirrorsSequence>
NoConfigEnvironment::mirrors(const std::string &) const
{
    return make_shared_ptr(new MirrorsSequence);
}

bool
NoConfigEnvironment::accept_license(const std::string &, const PackageID &) const
{
    return true;
}

const FSEntry
NoConfigEnvironment::root() const
{
    return FSEntry("/");
}

HookResult
NoConfigEnvironment::perform_hook(const Hook &) const
{
    return HookResult(0, "");
}

tr1::shared_ptr<const FSEntrySequence>
NoConfigEnvironment::hook_dirs() const
{
    return make_shared_ptr(new FSEntrySequence);
}

tr1::shared_ptr<const UseFlagNameSet>
NoConfigEnvironment::known_use_expand_names(const UseFlagName &, const PackageID &) const
{
    return make_shared_ptr(new UseFlagNameSet);
}

