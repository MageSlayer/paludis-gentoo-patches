/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010 Ciaran McCreesh
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

#include <paludis/util/config_file.hh>
#include <paludis/hooker.hh>
#include <paludis/hook.hh>
#include <paludis/set_file.hh>
#include <paludis/distribution.hh>
#include <paludis/dep_tag.hh>
#include <paludis/package_id.hh>
#include <paludis/mask.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/literal_metadata_key.hh>
#include <paludis/repository_factory.hh>

#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/log.hh>
#include <paludis/util/system.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/save.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/set.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/map.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/options.hh>
#include <paludis/util/tribool.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/standard_output_manager.hh>

#include <tr1/functional>
#include <functional>
#include <algorithm>
#include <list>
#include <map>

using namespace paludis;
using namespace paludis::paludis_environment;

namespace paludis
{
    template<>
    struct Implementation<PaludisEnvironment>
    {
        mutable Mutex hook_mutex;
        mutable bool done_hooks;
        mutable std::tr1::shared_ptr<Hooker> hooker;
        mutable std::list<std::pair<FSEntry, bool> > hook_dirs;

        std::tr1::shared_ptr<PaludisConfig> config;
        std::string paludis_command;

        std::tr1::shared_ptr<PackageDatabase> package_database;

        mutable Mutex sets_mutex;
        mutable std::map<SetName, std::tr1::shared_ptr<const SetSpecTree> > sets;

        std::tr1::shared_ptr<LiteralMetadataValueKey<std::string> > format_key;
        std::tr1::shared_ptr<LiteralMetadataValueKey<FSEntry> > config_location_key;
        std::tr1::shared_ptr<LiteralMetadataValueKey<FSEntry> > world_file_key;

        Implementation(PaludisEnvironment * const e, std::tr1::shared_ptr<PaludisConfig> c) :
            done_hooks(false),
            config(c),
            paludis_command("paludis"),
            package_database(new PackageDatabase(e)),
            format_key(new LiteralMetadataValueKey<std::string>("format", "Format", mkt_significant, "paludis")),
            config_location_key(new LiteralMetadataValueKey<FSEntry>("conf_dir", "Config dir", mkt_normal,
                        config->config_dir())),
            world_file_key(config->world()->location_if_set() ? make_shared_ptr(
                        new LiteralMetadataValueKey<FSEntry>("world_file", "World file", mkt_normal,
                            *config->world()->location_if_set()))
                    : std::tr1::shared_ptr<LiteralMetadataValueKey<FSEntry> >())
        {
        }

        void add_one_hook(const FSEntry & r, const bool v) const
        {
            try
            {
                if (r.is_directory())
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

        void need_hook_dirs(const FSEntry & c) const
        {
            if (! done_hooks)
            {
                add_one_hook(c / "hooks", true);
                if (getenv_with_default("PALUDIS_NO_GLOBAL_HOOKS", "").empty())
                {
                    add_one_hook(FSEntry(LIBEXECDIR) / "paludis" / "hooks", false);
                    add_one_hook(FSEntry(DATADIR) / "paludis" / "hooks", true);
                    add_one_hook(FSEntry(LIBDIR) / "paludis" / "hooks", true);
                }
                done_hooks = true;
            }
        }
    };
}

PaludisEnvironment::PaludisEnvironment(const std::string & s) :
    PrivateImplementationPattern<PaludisEnvironment>(new Implementation<PaludisEnvironment>(
                this, std::tr1::shared_ptr<PaludisConfig>(new PaludisConfig(this, s)))),
    _imp(PrivateImplementationPattern<PaludisEnvironment>::_imp)
{
    Context context("When loading paludis environment:");

    for (PaludisConfig::RepositoryConstIterator r(_imp->config->begin_repositories()),
            r_end(_imp->config->end_repositories()) ; r != r_end ; ++r)
        _imp->package_database->add_repository(
                RepositoryFactory::get_instance()->importance(this, *r),
                RepositoryFactory::get_instance()->create(this, *r));

    add_metadata_key(_imp->format_key);
    add_metadata_key(_imp->config_location_key);
    if (_imp->world_file_key)
        add_metadata_key(_imp->world_file_key);
}

PaludisEnvironment::~PaludisEnvironment()
{
}

bool
PaludisEnvironment::accept_keywords(const std::tr1::shared_ptr<const KeywordNameSet> & k,
        const PackageID & e) const
{
    return _imp->config->keywords_conf()->query(k, e);
}

bool
PaludisEnvironment::accept_license(const std::string & license, const PackageID & d) const
{
    if (license == "*")
        return true;
    if (license == "-*")
        return false;

    Context context("When checking license of '" + license + "' for '" + stringify(d) + "':");

    return _imp->config->licenses_conf()->query(license, d);
}

bool
PaludisEnvironment::unmasked_by_user(const PackageID & d) const
{
    return _imp->config->package_unmask_conf()->query(d);
}

std::tr1::shared_ptr<const FSEntrySequence>
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

HookResult
PaludisEnvironment::perform_hook(const Hook & hook) const
{
    Lock lock(_imp->hook_mutex);

    if (! _imp->hooker)
    {
        _imp->need_hook_dirs(_imp->config->config_dir());
        _imp->hooker.reset(new Hooker(this));
        for (std::list<std::pair<FSEntry, bool> >::const_iterator h(_imp->hook_dirs.begin()),
                h_end(_imp->hook_dirs.end()) ; h != h_end ; ++h)
            _imp->hooker->add_dir(h->first, h->second);
    }

    return _imp->hooker->perform_hook(hook);
}

std::tr1::shared_ptr<const FSEntrySequence>
PaludisEnvironment::hook_dirs() const
{
    Lock lock(_imp->hook_mutex);

    _imp->need_hook_dirs(_imp->config->config_dir());

    std::tr1::shared_ptr<FSEntrySequence> result(new FSEntrySequence);
    std::transform(_imp->hook_dirs.begin(), _imp->hook_dirs.end(), result->back_inserter(),
            std::tr1::mem_fn(&std::pair<FSEntry, bool>::first));

    return result;
}

std::tr1::shared_ptr<const FSEntrySequence>
PaludisEnvironment::fetchers_dirs() const
{
    std::tr1::shared_ptr<FSEntrySequence> result(new FSEntrySequence);

    result->push_back(FSEntry(_imp->config->config_dir()) / "fetchers");

    if (getenv_with_default("PALUDIS_NO_GLOBAL_FETCHERS", "").empty())
    {
        std::tr1::shared_ptr<const FSEntrySequence> r(EnvironmentImplementation::fetchers_dirs());
        std::copy(r->begin(), r->end(), result->back_inserter());
    }

    return result;
}

std::tr1::shared_ptr<const FSEntrySequence>
PaludisEnvironment::syncers_dirs() const
{
    std::tr1::shared_ptr<FSEntrySequence> result(new FSEntrySequence);

    result->push_back(FSEntry(_imp->config->config_dir()) / "syncers");

    if (getenv_with_default("PALUDIS_NO_GLOBAL_SYNCERS", "").empty())
    {
        std::tr1::shared_ptr<const FSEntrySequence> r(EnvironmentImplementation::syncers_dirs());
        std::copy(r->begin(), r->end(), result->back_inserter());
    }

    return result;
}

void
PaludisEnvironment::add_to_world(const QualifiedPackageName & q) const
{
    _imp->config->world()->add_to_world(q);
}

void
PaludisEnvironment::add_to_world(const SetName & s) const
{
    _imp->config->world()->add_to_world(s);
}

void
PaludisEnvironment::remove_from_world(const QualifiedPackageName & q) const
{
    _imp->config->world()->remove_from_world(q);
}

void
PaludisEnvironment::remove_from_world(const SetName & s) const
{
    _imp->config->world()->remove_from_world(s);
}

std::tr1::shared_ptr<const MirrorsSequence>
PaludisEnvironment::mirrors(const std::string & m) const
{
    return _imp->config->mirrors_conf()->query(m);
}

const FSEntry
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

std::string
PaludisEnvironment::config_dir() const
{
    return _imp->config->config_dir();
}

std::tr1::shared_ptr<PackageDatabase>
PaludisEnvironment::package_database()
{
    return _imp->package_database;
}

std::tr1::shared_ptr<const PackageDatabase>
PaludisEnvironment::package_database() const
{
    return _imp->package_database;
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

            char key() const
            {
                return 'B';
            }

            const std::string description() const
            {
                return "breaks Portage";
            }

            const std::string explanation() const
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

            char key() const
            {
                return _overridden ? 'u' : 'U';
            }

            const std::string description() const
            {
                return _overridden ? "user (overridden)" : "user";
            }
    };
}

const std::tr1::shared_ptr<const Mask>
PaludisEnvironment::mask_for_breakage(const PackageID & id) const
{
    if (! _imp->config->accept_all_breaks_portage())
    {
        std::tr1::shared_ptr<const Set<std::string> > breakages(id.breaks_portage());
        if (breakages)
        {
            std::list<std::string> bad_breakages;
            std::set_difference(breakages->begin(), breakages->end(),
                     _imp->config->accept_breaks_portage().begin(), _imp->config->accept_breaks_portage().end(),
                     std::back_inserter(bad_breakages));
            if (! bad_breakages.empty())
                return make_shared_ptr(new BreaksPortageMask(join(breakages->begin(), breakages->end(), " ")));
        }
    }

    return std::tr1::shared_ptr<const Mask>();
}

const std::tr1::shared_ptr<const Mask>
PaludisEnvironment::mask_for_user(const PackageID & d, const bool o) const
{
    if (_imp->config->package_mask_conf()->query(d))
        return make_shared_ptr(new UserConfigMask(o));

    return std::tr1::shared_ptr<const Mask>();
}

void
PaludisEnvironment::need_keys_added() const
{
}

const std::tr1::shared_ptr<const MetadataValueKey<std::string> >
PaludisEnvironment::format_key() const
{
    return _imp->format_key;
}

const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> >
PaludisEnvironment::config_location_key() const
{
    return _imp->config_location_key;
}

const Tribool
PaludisEnvironment::want_choice_enabled(
        const std::tr1::shared_ptr<const PackageID> & id,
        const std::tr1::shared_ptr<const Choice> & choice,
        const UnprefixedChoiceName & value
        ) const
{
    return _imp->config->use_conf()->want_choice_enabled(id, choice, value);
}

const std::string
PaludisEnvironment::value_for_choice_parameter(
        const std::tr1::shared_ptr<const PackageID> & id,
        const std::tr1::shared_ptr<const Choice> & choice,
        const UnprefixedChoiceName & value
        ) const
{
    return _imp->config->use_conf()->value_for_choice_parameter(id, choice, value);
}

std::tr1::shared_ptr<const Set<UnprefixedChoiceName> >
PaludisEnvironment::known_choice_value_names(
        const std::tr1::shared_ptr<const PackageID> & id,
        const std::tr1::shared_ptr<const Choice> & choice
        ) const
{
    return _imp->config->use_conf()->known_choice_value_names(id, choice);
}

const std::tr1::shared_ptr<OutputManager>
PaludisEnvironment::create_output_manager(const CreateOutputManagerInfo & i) const
{
    return _imp->config->output_conf()->create_output_manager(i);
}

namespace
{
    std::tr1::shared_ptr<const SetSpecTree> make_world_set(const std::tr1::shared_ptr<const World> & world)
    {
        return world->world_set();
    }

    std::tr1::shared_ptr<const SetSpecTree> make_set(
            const Environment * const env,
            const FSEntry & f,
            const SetName & n,
            SetFileSetOperatorMode mode,
            SetFileType type)
    {
        Context context("When making set '" + stringify(f) + "':");

        const std::tr1::shared_ptr<GeneralSetDepTag> tag(new GeneralSetDepTag(n, stringify(f.basename())));

        SetFile s(make_named_values<SetFileParams>(
                    n::environment() = env,
                    n::file_name() = f,
                    n::parser() = std::tr1::bind(&parse_user_package_dep_spec,
                            std::tr1::placeholders::_1, env, UserPackageDepSpecOptions() + updso_allow_wildcards,
                            filter::All()),
                    n::set_operator_mode() = mode,
                    n::tag() = tag,
                    n::type() = type
                    ));
        return s.contents();
    }
}

void
PaludisEnvironment::populate_sets() const
{
    Lock lock(_imp->sets_mutex);
    add_set(SetName("world"), SetName("world::environment"), std::tr1::bind(&make_world_set, _imp->config->world()), true);

    FSEntry sets_dir(FSEntry(_imp->config->config_dir()) / "sets");
    Context context("When looking in sets directory '" + stringify(sets_dir) + "':");

    if (! sets_dir.exists())
        return;

    for (DirIterator d(sets_dir, DirIteratorOptions() + dio_inode_sort), d_end ;
            d != d_end ; ++d)
    {
        if (is_file_with_extension(*d, ".bash", IsFileWithOptions()))
        {
            SetName n(strip_trailing_string(d->basename(), ".bash"));
            add_set(n, n, std::tr1::bind(&make_set, this, *d, n, sfsmo_natural, sft_paludis_bash), false);

            SetName n_s(stringify(n) + "*");
            add_set(n_s, n_s, std::tr1::bind(&make_set, this, *d, n_s, sfsmo_star, sft_paludis_bash), false);
        }
        else if (is_file_with_extension(*d, ".conf", IsFileWithOptions()))
        {
            SetName n(strip_trailing_string(d->basename(), ".conf"));
            add_set(n, n, std::tr1::bind(&make_set, this, *d, n, sfsmo_natural, sft_paludis_conf), false);

            SetName n_s(stringify(n) + "*");
            add_set(n_s, n_s, std::tr1::bind(&make_set, this, *d, n_s, sfsmo_star, sft_paludis_conf), false);
        }
    }
}

const std::tr1::shared_ptr<Repository>
PaludisEnvironment::repository_from_new_config_file(const FSEntry & f)
{
    const std::tr1::function<std::string (const std::string &)> repo_func(_imp->config->repo_func_from_file(f));
    if (! repo_func)
        throw PaludisConfigError("File '" + stringify(f) + "' does not describe a valid repository");
    return RepositoryFactory::get_instance()->create(this, repo_func);
}

void
PaludisEnvironment::update_config_files_for_package_move(const PackageDepSpec & s, const QualifiedPackageName & n) const
{
    _imp->config->world()->update_config_files_for_package_move(s, n);
}

