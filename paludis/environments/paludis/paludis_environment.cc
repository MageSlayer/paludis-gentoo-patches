/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2013, 2014 Ciaran McCreesh
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

#include <paludis/environments/paludis/paludis_config.hh>
#include <paludis/environments/paludis/paludis_environment.hh>
#include <paludis/environments/paludis/keywords_conf.hh>
#include <paludis/environments/paludis/use_conf.hh>
#include <paludis/environments/paludis/package_mask_conf.hh>
#include <paludis/environments/paludis/licenses_conf.hh>
#include <paludis/environments/paludis/mirrors_conf.hh>
#include <paludis/environments/paludis/output_conf.hh>
#include <paludis/environments/paludis/world.hh>
#include <paludis/environments/paludis/suggestions_conf.hh>

#include <paludis/hooker.hh>
#include <paludis/hook.hh>
#include <paludis/set_file.hh>
#include <paludis/distribution.hh>
#include <paludis/package_id.hh>
#include <paludis/mask.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/literal_metadata_key.hh>
#include <paludis/repository_factory.hh>
#include <paludis/standard_output_manager.hh>

#include <paludis/util/config_file.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/log.hh>
#include <paludis/util/system.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/save.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/set.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/map.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/options.hh>
#include <paludis/util/tribool.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/fs_stat.hh>
#include <paludis/util/fs_iterator.hh>
#include <paludis/util/fs_error.hh>
#include <paludis/util/env_var_names.hh>
#include <paludis/util/join.hh>

#include <functional>
#include <algorithm>
#include <list>
#include <map>
#include <mutex>
#include <unistd.h>

using namespace paludis;
using namespace paludis::paludis_environment;

namespace paludis
{
    template<>
    struct Imp<PaludisEnvironment>
    {
        mutable std::mutex hook_mutex;
        mutable bool done_hooks;
        mutable std::shared_ptr<Hooker> hooker;
        mutable std::list<std::pair<FSPath, bool> > hook_dirs;

        std::shared_ptr<PaludisConfig> config;

        mutable std::mutex sets_mutex;
        mutable std::map<SetName, std::shared_ptr<const SetSpecTree> > sets;

        std::shared_ptr<LiteralMetadataValueKey<std::string> > format_key;
        std::shared_ptr<LiteralMetadataValueKey<FSPath> > config_location_key;
        std::shared_ptr<LiteralMetadataValueKey<FSPath> > world_file_key;
        std::shared_ptr<LiteralMetadataValueKey<FSPath> > preferred_root_key;
        std::shared_ptr<LiteralMetadataValueKey<FSPath> > system_root_key;

        Imp(std::shared_ptr<PaludisConfig> c) :
            done_hooks(false),
            config(c),
            format_key(std::make_shared<LiteralMetadataValueKey<std::string>>("format", "Format", mkt_significant, "paludis")),
            config_location_key(std::make_shared<LiteralMetadataValueKey<FSPath>>("conf_dir", "Config dir", mkt_normal,
                        FSPath(config->config_dir()))),
            world_file_key(config->world()->location_if_set() ? std::make_shared<LiteralMetadataValueKey<FSPath>>("world_file", "World file", mkt_normal,
                            *config->world()->location_if_set())
                    : std::shared_ptr<LiteralMetadataValueKey<FSPath> >()),
            preferred_root_key(std::make_shared<LiteralMetadataValueKey<FSPath>>("root", "Root", mkt_normal,
                        FSPath(config->root()))),
            system_root_key(std::make_shared<LiteralMetadataValueKey<FSPath>>("system_root", "System Root", mkt_normal,
                        FSPath(config->system_root())))
        {
        }

        void add_one_hook(const FSPath & r, const bool v) const
        {
            try
            {
                if (r.stat().is_directory())
                {
                    Log::get_instance()->message("paludis_environment.hooks.add_dir", ll_debug, lc_no_context)
                        << "Adding hook directory '" << r << "'";
                    hook_dirs.push_back(std::make_pair(r, v));
                }
                else
                    Log::get_instance()->message("paludis_environment.hook.skipping", ll_debug, lc_no_context)
                        << "Skipping hook directory candidate '" << r << "'";
            }
            catch (const FSError & e)
            {
                Log::get_instance()->message("paludis_environment.hook.failure", ll_warning, lc_no_context)
                    << "Caught exception '" << e.message() << "' (" << e.what() << ") when checking hook "
                    "directory '" << r << "'";
            }
        }

        void need_hook_dirs(const FSPath & c) const
        {
            if (! done_hooks)
            {
                add_one_hook(c / "hooks", true);
                if (getenv_with_default(env_vars::no_global_hooks, "").empty())
                {
                    add_one_hook(FSPath(LIBEXECDIR) / "paludis" / "hooks", false);
                    add_one_hook(FSPath(DATADIR) / "paludis" / "hooks", true);
                    add_one_hook(FSPath(LIBDIR) / "paludis" / "hooks", true);
                }
                done_hooks = true;
            }
        }
    };
}

PaludisEnvironment::PaludisEnvironment(const std::string & s) :
    _imp(std::make_shared<PaludisConfig>(this, s))
{
    Context context("When loading paludis environment:");

    for (PaludisConfig::RepositoryConstIterator r(_imp->config->begin_repositories()),
            r_end(_imp->config->end_repositories()) ; r != r_end ; ++r)
        add_repository(
                RepositoryFactory::get_instance()->importance(this, *r),
                RepositoryFactory::get_instance()->create(this, *r));

    add_metadata_key(_imp->format_key);
    add_metadata_key(_imp->config_location_key);
    if (_imp->world_file_key)
        add_metadata_key(_imp->world_file_key);
    add_metadata_key(_imp->preferred_root_key);
    add_metadata_key(_imp->system_root_key);
}

PaludisEnvironment::~PaludisEnvironment() = default;

bool
PaludisEnvironment::accept_keywords(const std::shared_ptr<const KeywordNameSet> & k,
        const std::shared_ptr<const PackageID> & e) const
{
    return _imp->config->keywords_conf()->query(k, e);
}

bool
PaludisEnvironment::accept_license(const std::string & license, const std::shared_ptr<const PackageID> & d) const
{
    if (license == "*")
        return true;
    if (license == "-*")
        return false;

    Context context("When checking license of '" + license + "' for '" + stringify(*d) + "':");

    return _imp->config->licenses_conf()->query(license, d);
}

bool
PaludisEnvironment::unmasked_by_user(const std::shared_ptr<const PackageID> & d, const std::string & reason) const
{
    return _imp->config->package_unmask_conf()->query(d, reason);
}

std::shared_ptr<const FSPathSequence>
PaludisEnvironment::bashrc_files() const
{
    return _imp->config->bashrc_files();
}

HookResult
PaludisEnvironment::perform_hook(
        const Hook & hook,
        const std::shared_ptr<OutputManager> & optional_output_manager) const
{
    std::unique_lock<std::mutex> lock(_imp->hook_mutex);

    if (! _imp->hooker)
    {
        _imp->need_hook_dirs(FSPath(_imp->config->config_dir()));
        _imp->hooker = std::make_shared<Hooker>(this);
        for (const auto & hook_dir : _imp->hook_dirs)
            _imp->hooker->add_dir(hook_dir.first, hook_dir.second);
    }

    return _imp->hooker->perform_hook(hook, optional_output_manager);
}

std::shared_ptr<const FSPathSequence>
PaludisEnvironment::hook_dirs() const
{
    std::unique_lock<std::mutex> lock(_imp->hook_mutex);

    _imp->need_hook_dirs(FSPath(_imp->config->config_dir()));

    std::shared_ptr<FSPathSequence> result(std::make_shared<FSPathSequence>());
    std::transform(_imp->hook_dirs.begin(), _imp->hook_dirs.end(), result->back_inserter(),
            std::mem_fn(&std::pair<FSPath, bool>::first));

    return result;
}

std::shared_ptr<const FSPathSequence>
PaludisEnvironment::fetchers_dirs() const
{
    std::shared_ptr<FSPathSequence> result(std::make_shared<FSPathSequence>());

    result->push_back(FSPath(_imp->config->config_dir()) / "fetchers");

    if (getenv_with_default(env_vars::no_global_fetchers, "").empty())
    {
        std::shared_ptr<const FSPathSequence> r(EnvironmentImplementation::fetchers_dirs());
        std::copy(r->begin(), r->end(), result->back_inserter());
    }

    return result;
}

std::shared_ptr<const FSPathSequence>
PaludisEnvironment::syncers_dirs() const
{
    std::shared_ptr<FSPathSequence> result(std::make_shared<FSPathSequence>());

    result->push_back(FSPath(_imp->config->config_dir()) / "syncers");

    if (getenv_with_default(env_vars::no_global_syncers, "").empty())
    {
        std::shared_ptr<const FSPathSequence> r(EnvironmentImplementation::syncers_dirs());
        std::copy(r->begin(), r->end(), result->back_inserter());
    }

    return result;
}

bool
PaludisEnvironment::add_to_world(const QualifiedPackageName & q) const
{
    return _imp->config->world()->add_to_world(q);
}

bool
PaludisEnvironment::add_to_world(const SetName & s) const
{
    return _imp->config->world()->add_to_world(s);
}

bool
PaludisEnvironment::remove_from_world(const QualifiedPackageName & q) const
{
    return _imp->config->world()->remove_from_world(q);
}

bool
PaludisEnvironment::remove_from_world(const SetName & s) const
{
    return _imp->config->world()->remove_from_world(s);
}

std::shared_ptr<const MirrorsSequence>
PaludisEnvironment::mirrors(const std::string & m) const
{
    return _imp->config->mirrors_conf()->query(m);
}

std::string
PaludisEnvironment::reduced_username() const
{
    uid_t u(getuid());
    if (0 == u)
        return _imp->config->reduced_username();
    else
        return get_user_name(u);
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

std::string
PaludisEnvironment::config_dir() const
{
    return _imp->config->config_dir();
}

std::string
PaludisEnvironment::distribution() const
{
    return _imp->config->distribution();
}

namespace
{
    class BreaksPortageMask :
        public UnsupportedMask
    {
        private:
            std::string breakages;

        public:
            BreaksPortageMask(const std::string & b) :
                breakages(b)
            {
            }

            char key() const override
            {
                return 'B';
            }

            const std::string description() const override
            {
                return "breaks Portage";
            }

            const std::string explanation() const override
            {
                return breakages;
            }
    };

    class UserConfigMask :
        public UserMask
    {
        private:
        bool _overridden;

        public:
            UserConfigMask(const bool o) :
                _overridden(o)
            {
            }

            char key() const override
            {
                return _overridden ? 'u' : 'U';
            }

            const std::string description() const override
            {
                return _overridden ? "user (overridden)" : "user";
            }
    };
}

const std::shared_ptr<const Mask>
PaludisEnvironment::mask_for_user(const std::shared_ptr<const PackageID> & d, const bool o) const
{
    if (_imp->config->package_mask_conf()->query(d, ""))
        return std::make_shared<UserConfigMask>(o);

    return nullptr;
}

void
PaludisEnvironment::need_keys_added() const
{
}

const std::shared_ptr<const MetadataValueKey<std::string> >
PaludisEnvironment::format_key() const
{
    return _imp->format_key;
}

const std::shared_ptr<const MetadataValueKey<FSPath> >
PaludisEnvironment::config_location_key() const
{
    return _imp->config_location_key;
}

const std::shared_ptr<const MetadataValueKey<FSPath> >
PaludisEnvironment::preferred_root_key() const
{
    return _imp->preferred_root_key;
}

const std::shared_ptr<const MetadataValueKey<FSPath> >
PaludisEnvironment::system_root_key() const
{
    return _imp->system_root_key;
}

const Tribool
PaludisEnvironment::want_choice_enabled(
        const std::shared_ptr<const PackageID> & id,
        const std::shared_ptr<const Choice> & choice,
        const UnprefixedChoiceName & value
        ) const
{
    return _imp->config->use_conf()->want_choice_enabled(id, choice, value);
}

const std::string
PaludisEnvironment::value_for_choice_parameter(
        const std::shared_ptr<const PackageID> & id,
        const std::shared_ptr<const Choice> & choice,
        const UnprefixedChoiceName & value
        ) const
{
    return _imp->config->use_conf()->value_for_choice_parameter(id, choice, value);
}

std::shared_ptr<const Set<UnprefixedChoiceName> >
PaludisEnvironment::known_choice_value_names(
        const std::shared_ptr<const PackageID> & id,
        const std::shared_ptr<const Choice> & choice
        ) const
{
    return _imp->config->use_conf()->known_choice_value_names(id, choice);
}

Tribool
PaludisEnvironment::interest_in_suggestion(
        const std::shared_ptr<const PackageID> & i,
        const PackageDepSpec & s) const
{
    return _imp->config->suggestions_conf()->interest_in_suggestion(i, s);
}

const std::shared_ptr<OutputManager>
PaludisEnvironment::create_output_manager(const CreateOutputManagerInfo & i) const
{
    return _imp->config->output_conf()->create_output_manager(i);
}

namespace
{
    std::shared_ptr<const SetSpecTree> make_world_set(const std::shared_ptr<const World> & world)
    {
        return world->world_set();
    }

    std::shared_ptr<const SetSpecTree> make_set(
            const Environment * const env,
            const FSPath & f,
            const SetName &,
            SetFileSetOperatorMode mode,
            SetFileType type)
    {
        Context context("When making set '" + stringify(f) + "':");

        SetFile s(make_named_values<SetFileParams>(
                    n::environment() = env,
                    n::file_name() = f,
                    n::parser() = std::bind(&parse_user_package_dep_spec, std::placeholders::_1, env,
                        UserPackageDepSpecOptions() + updso_allow_wildcards, filter::All()),
                    n::set_operator_mode() = mode,
                    n::type() = type
                    ));
        return s.contents();
    }
}

void
PaludisEnvironment::populate_sets() const
{
    std::unique_lock<std::mutex> lock(_imp->sets_mutex);
    add_set(SetName("world"), SetName("world::environment"), std::bind(&make_world_set, _imp->config->world()), true);

    std::list<FSPath> sets_dirs;

    sets_dirs.push_back(FSPath(_imp->config->config_dir()) / "sets");
    if (getenv_with_default(env_vars::no_global_sets, "").empty())
    {
        sets_dirs.push_back(FSPath(LIBEXECDIR) / "paludis" / "sets");
        sets_dirs.push_back(FSPath(DATADIR) / "paludis" / "sets");
        sets_dirs.push_back(FSPath(LIBDIR) / "paludis" / "sets");
    }

    for (const auto & sets_dir : sets_dirs)
    {
        Context context("When looking in sets directory '" + stringify(sets_dir) + "':");

        if (! sets_dir.stat().exists())
            continue;

        for (FSIterator d(sets_dir, { fsio_inode_sort }), d_end ; d != d_end ; ++d)
        {
            if (is_file_with_extension(*d, ".bash", { }))
            {
                SetName n(strip_trailing_string(d->basename(), ".bash"));
                add_set(n, n, std::bind(&make_set, this, *d, n, sfsmo_natural, sft_paludis_bash), false);

                SetName n_s(stringify(n) + "*");
                add_set(n_s, n_s, std::bind(&make_set, this, *d, n_s, sfsmo_star, sft_paludis_bash), false);
            }
            else if (is_file_with_extension(*d, ".conf", { }))
            {
                SetName n(strip_trailing_string(d->basename(), ".conf"));
                add_set(n, n, std::bind(&make_set, this, *d, n, sfsmo_natural, sft_paludis_conf), false);

                SetName n_s(stringify(n) + "*");
                add_set(n_s, n_s, std::bind(&make_set, this, *d, n_s, sfsmo_star, sft_paludis_conf), false);
            }
        }
    }
}

const std::shared_ptr<Repository>
PaludisEnvironment::repository_from_new_config_file(const FSPath & f)
{
    const std::function<std::string (const std::string &)> repo_func(_imp->config->repo_func_from_file(f));
    if (! repo_func)
        throw PaludisConfigError("File '" + stringify(f) + "' does not describe a valid repository");
    return RepositoryFactory::get_instance()->create(this, repo_func);
}

void
PaludisEnvironment::update_config_files_for_package_move(const PackageDepSpec & s, const QualifiedPackageName & n) const
{
    _imp->config->world()->update_config_files_for_package_move(s, n);
}

