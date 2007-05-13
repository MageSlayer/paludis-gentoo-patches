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

#include <paludis/environments/paludis/paludis_config.hh>
#include <paludis/environments/paludis/paludis_environment.hh>
#include <paludis/environments/paludis/keywords_conf.hh>
#include <paludis/environments/paludis/use_conf.hh>
#include <paludis/environments/paludis/package_mask_conf.hh>
#include <paludis/environments/paludis/licenses_conf.hh>
#include <paludis/environments/paludis/mirrors_conf.hh>

#include <paludis/repositories/repository_maker.hh>

#include <paludis/config_file.hh>
#include <paludis/hooker.hh>
#include <paludis/set_file.hh>

#include <paludis/util/collection_concrete.hh>
#include <paludis/util/log.hh>
#include <paludis/util/system.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/strip.hh>
#include <tr1/functional>
#include <functional>
#include <algorithm>
#include <list>

using namespace paludis;

namespace paludis
{
    template<>
    struct Implementation<PaludisEnvironment>
    {
        mutable bool done_hooks;
        mutable std::tr1::shared_ptr<Hooker> hooker;
        mutable std::list<std::pair<FSEntry, bool> > hook_dirs;

        std::tr1::shared_ptr<PaludisConfig> config;
        std::string paludis_command;
        std::list<UseConfigEntry> forced_use;

        std::tr1::shared_ptr<PackageDatabase> package_database;

        Implementation(PaludisEnvironment * const e, std::tr1::shared_ptr<PaludisConfig> c) :
            done_hooks(false),
            config(c),
            paludis_command("paludis"),
            package_database(new PackageDatabase(e))
        {
        }

        void add_one_hook(const FSEntry & r, const bool v) const
        {
            try
            {
                if (r.is_directory())
                {
                    Log::get_instance()->message(ll_debug, lc_no_context, "Adding hook directory '"
                            + stringify(r) + "'");
                    hook_dirs.push_back(std::make_pair(r, v));
                }
                else
                    Log::get_instance()->message(ll_debug, lc_no_context, "Skipping hook directory candidate '"
                            + stringify(r) + "'");
            }
            catch (const FSError & e)
            {
                Log::get_instance()->message(ll_warning, lc_no_context, "Caught exception '" +
                        e.message() + "' (" + e.what() + ") when checking hook "
                        "directory '" + stringify(r) + "'");
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
                }
                done_hooks = true;
            }
        }
    };
}

PaludisEnvironment::PaludisEnvironment(const std::string & s) :
    PrivateImplementationPattern<PaludisEnvironment>(new Implementation<PaludisEnvironment>(
                this, std::tr1::shared_ptr<PaludisConfig>(new PaludisConfig(this, s))))
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

        _imp->package_database->add_repository(r->importance,
                RepositoryMaker::get_instance()->find_maker(r->format)(this, r->keys));
    }
}

PaludisEnvironment::~PaludisEnvironment()
{
}

bool
PaludisEnvironment::query_use(const UseFlagName & f, const PackageDatabaseEntry & e) const
{
    Context context("When querying use flag '" + stringify(f) + "' for '" + stringify(e) +
            "' in Paludis environment:");

    static bool recursive(false);
    if (recursive)
    {
        Log::get_instance()->message(ll_warning, lc_context) <<
            "use flag state is defined recursively, forcing it to disabled instead";
        return false;
    }
    Save<bool> save_recursive(&recursive, true);

    /* first check package database use masks... */
    std::tr1::shared_ptr<const Repository> repo(package_database()->fetch_repository(e.repository));

    if (repo->use_interface)
    {
        if (repo->use_interface->query_use_mask(f, e))
            return false;
        if (repo->use_interface->query_use_force(f, e))
            return true;
    }

    /* check configs */
    do
    {
        switch (_imp->config->use_conf()->query(f, e))
        {
            case use_disabled:
                return false;

            case use_enabled:
                return true;

            case use_unspecified:
                continue;

            case last_use:
                ;
        }
        throw InternalError(PALUDIS_HERE, "bad state");
    } while (false);

    /* check use: package database config */
    if (repo->use_interface)
    {
        switch (repo->use_interface->query_use(f, e))
        {
            case use_disabled:
            case use_unspecified:
                return false;

            case use_enabled:
                return true;

            case last_use:
                ;
        }

        throw InternalError(PALUDIS_HERE, "bad state");
    }

    return false;
}

bool
PaludisEnvironment::accept_breaks_portage(const PackageDatabaseEntry &) const
{
    return _imp->config->accept_breaks_portage();
}

bool
PaludisEnvironment::accept_keywords(std::tr1::shared_ptr<const KeywordNameCollection> k,
        const PackageDatabaseEntry & e) const
{
    return _imp->config->keywords_conf()->query(k, e);
}

bool
PaludisEnvironment::accept_license(const std::string & license, const PackageDatabaseEntry & d) const
{
    if (license == "*")
        return true;
    if (license == "-*")
        return false;

    Context context("When checking license of '" + license + "' for '" + stringify(d) + "':");

    return _imp->config->licenses_conf()->query(license, d);
}

bool
PaludisEnvironment::masked_by_user(const PackageDatabaseEntry & d) const
{
    return _imp->config->package_mask_conf()->query(d);
}

bool
PaludisEnvironment::unmasked_by_user(const PackageDatabaseEntry & d) const
{
    return _imp->config->package_unmask_conf()->query(d);
}

std::tr1::shared_ptr<const FSEntryCollection>
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

int
PaludisEnvironment::perform_hook(const Hook & hook) const
{
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

std::tr1::shared_ptr<const FSEntryCollection>
PaludisEnvironment::hook_dirs() const
{
    _imp->need_hook_dirs(_imp->config->config_dir());

    std::tr1::shared_ptr<FSEntryCollection> result(new FSEntryCollection::Concrete);
    std::copy(_imp->hook_dirs.begin(), _imp->hook_dirs.end(),
            transform_inserter(result->inserter(), SelectFirst<FSEntry, bool>()));

    return result;
}

std::tr1::shared_ptr<const FSEntryCollection>
PaludisEnvironment::fetchers_dirs() const
{
    std::tr1::shared_ptr<FSEntryCollection> result(new FSEntryCollection::Concrete);

    result->push_back(FSEntry(_imp->config->config_dir()) / "fetchers");

    if (getenv_with_default("PALUDIS_NO_GLOBAL_FETCHERS", "").empty())
    {
        std::tr1::shared_ptr<const FSEntryCollection> r(EnvironmentImplementation::fetchers_dirs());
        std::copy(r->begin(), r->end(), result->inserter());
    }

    return result;
}

std::tr1::shared_ptr<const FSEntryCollection>
PaludisEnvironment::syncers_dirs() const
{
    std::tr1::shared_ptr<FSEntryCollection> result(new FSEntryCollection::Concrete);

    result->push_back(FSEntry(_imp->config->config_dir()) / "syncers");

    if (getenv_with_default("PALUDIS_NO_GLOBAL_SYNCERS", "").empty())
    {
        std::tr1::shared_ptr<const FSEntryCollection> r(EnvironmentImplementation::syncers_dirs());
        std::copy(r->begin(), r->end(), result->inserter());
    }

    return result;
}

std::tr1::shared_ptr<CompositeDepSpec>
PaludisEnvironment::local_set(const SetName & s) const
{
    Context context("When looking for package set '" + stringify(s) + "' in paludis environment:");

    FSEntry dir(FSEntry(_imp->config->config_dir()) / "sets");
    std::tr1::shared_ptr<GeneralSetDepTag> tag(new GeneralSetDepTag(s, stringify(s) + ".conf"));

    if ((dir / (stringify(s) + ".bash")).exists())
    {
        SetFile f(SetFileParams::create()
                .file_name(dir / (stringify(s) + ".bash"))
                .type(sft_paludis_bash)
                .parse_mode(pds_pm_unspecific)
                .tag(tag)
                .environment(this));
        return f.contents();
    }
    else if ((dir / (stringify(s) + ".conf")).exists())
    {
        SetFile f(SetFileParams::create()
                .file_name(dir / (stringify(s) + ".conf"))
                .type(sft_paludis_conf)
                .parse_mode(pds_pm_unspecific)
                .tag(tag)
                .environment(this));
        return f.contents();
    }
    else
        return std::tr1::shared_ptr<AllDepSpec>();
}

std::tr1::shared_ptr<const SetNameCollection>
PaludisEnvironment::set_names() const
{
    std::tr1::shared_ptr<SetNameCollection> result(new SetNameCollection::Concrete);

    if ((FSEntry(_imp->config->config_dir()) / "sets").exists())
        for (DirIterator d(FSEntry(_imp->config->config_dir()) / "sets"), d_end ;
                d != d_end ; ++d)
        {
            if (is_file_with_extension(*d, ".conf", IsFileWithOptions()))
                result->insert(SetName(strip_trailing_string(d->basename(), ".conf")));
            else if (is_file_with_extension(*d, ".bash", IsFileWithOptions()))
                result->insert(SetName(strip_trailing_string(d->basename(), ".bash")));
        }

    return result;
}

std::tr1::shared_ptr<const MirrorsCollection>
PaludisEnvironment::mirrors(const std::string & m) const
{
    return _imp->config->mirrors_conf()->query(m);
}

std::tr1::shared_ptr<const UseFlagNameCollection>
PaludisEnvironment::known_use_expand_names(const UseFlagName & prefix, const PackageDatabaseEntry & e) const
{
    return _imp->config->use_conf()->known_use_expand_names(prefix, e);
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

