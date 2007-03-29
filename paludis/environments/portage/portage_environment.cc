/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include "portage_environment.hh"
#include <paludis/util/log.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/system.hh>
#include <paludis/util/collection_concrete.hh>
#include <paludis/util/iterator.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/strip.hh>
#include <paludis/repositories/repository_maker.hh>
#include <paludis/config_file.hh>
#include <paludis/hooker.hh>
#include <paludis/match_package.hh>
#include <algorithm>
#include <tr1/functional>
#include <functional>
#include <set>
#include <vector>

using namespace paludis;

typedef std::list<std::pair<std::tr1::shared_ptr<const PackageDepSpec>, std::string> > PackageUse;
typedef std::list<std::pair<std::tr1::shared_ptr<const PackageDepSpec>, std::string> > PackageKeywords;
typedef std::list<std::tr1::shared_ptr<const PackageDepSpec> > PackageMask;
typedef std::list<std::tr1::shared_ptr<const PackageDepSpec> > PackageUnmask;

PortageEnvironmentConfigurationError::PortageEnvironmentConfigurationError(const std::string & s) throw () :
    ConfigurationError(s)
{
}

namespace paludis
{
    template<>
    struct Implementation<PortageEnvironment>
    {
        const FSEntry conf_dir;
        std::string paludis_command;

        std::tr1::shared_ptr<KeyValueConfigFile> vars;

        std::set<std::string> use_with_expands;
        std::set<std::string> use_expand;
        std::set<std::string> accept_keywords;

        PackageUse package_use;
        PackageKeywords package_keywords;
        PackageMask package_mask;
        PackageUnmask package_unmask;

        mutable bool done_hooks;
        mutable std::tr1::shared_ptr<Hooker> hooker;
        mutable std::list<FSEntry> hook_dirs;

        Implementation(const std::string & s) :
            conf_dir(FSEntry(s.empty() ? "/" : s) / SYSCONFDIR),
            paludis_command("paludis"),
            done_hooks(false)
        {
        }

        void add_one_hook(const FSEntry & r) const
        {
            try
            {
                if (r.is_directory())
                {
                    Log::get_instance()->message(ll_debug, lc_no_context, "Adding hook directory '"
                            + stringify(r) + "'");
                    hook_dirs.push_back(r);
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

        void need_hook_dirs() const
        {
            if (! done_hooks)
            {
                if (getenv_with_default("PALUDIS_NO_GLOBAL_HOOKS", "").empty())
                    add_one_hook(FSEntry(LIBEXECDIR) / "paludis" / "hooks");

                done_hooks = true;
            }
        }

    };
}

namespace
{
    bool is_incremental_excluding_use_expand(const std::string & s, const KeyValueConfigFile &)
    {
        return (s == "USE" || s == "USE_EXPAND" || s == "USE_EXPAND_HIDDEN" ||
                s == "CONFIG_PROTECT" || s == "CONFIG_PROTECT_MASK" || s == "FEATURES"
                || s == "ACCEPT_KEYWORDS");
    }

    bool is_incremental(const std::string & s, const KeyValueConfigFile & k)
    {
        if (is_incremental_excluding_use_expand(s, k))
            return true;

        std::set<std::string> use_expand;
        WhitespaceTokeniser::get_instance()->tokenise(k.get("USE_EXPAND"),
                std::inserter(use_expand, use_expand.begin()));
        if (use_expand.end() != use_expand.find(s))
            return true;

        return false;
    }
}

PortageEnvironment::PortageEnvironment(const std::string & s) :
    Environment(std::tr1::shared_ptr<PackageDatabase>(new PackageDatabase(this))),
    PrivateImplementationPattern<PortageEnvironment>(new Implementation<PortageEnvironment>(s))
{
    using namespace std::tr1::placeholders;

    Context context("When creating PortageEnvironment using config root '" + s + "':");

    Log::get_instance()->message(ll_warning, lc_no_context,
            "Use of Portage configuration files will lead to sub-optimal performance and loss of "
            "functionality. Full support for Portage configuration formats is not "
            "guaranteed; issues should be reported via trac. You are strongly encouraged "
            "to migrate to a Paludis configuration.");

    _imp->vars.reset(new KeyValueConfigFile(FSEntry("/dev/null")));
    _load_profile((_imp->conf_dir / "make.profile").realpath());
    if ((_imp->conf_dir / "make.globals").exists())
        _imp->vars.reset(new KeyValueConfigFile(_imp->conf_dir / "make.globals", _imp->vars, &is_incremental));
    if ((_imp->conf_dir / "make.conf").exists())
        _imp->vars.reset(new KeyValueConfigFile(_imp->conf_dir / "make.conf", _imp->vars,
                    &is_incremental_excluding_use_expand));

    /* TODO: load USE etc from env? */

    /* repositories */

    _add_virtuals_repository();
    _add_installed_virtuals_repository();
    if (_imp->vars->get("PORTDIR").empty())
        throw PortageEnvironmentConfigurationError("PORTDIR empty or unset");
    _add_portdir_repository(FSEntry(_imp->vars->get("PORTDIR")));
    _add_vdb_repository();
    std::list<FSEntry> portdir_overlay;
    WhitespaceTokeniser::get_instance()->tokenise(_imp->vars->get("PORTDIR_OVERLAY"),
            create_inserter<FSEntry>(std::back_inserter(portdir_overlay)));
    std::for_each(portdir_overlay.begin(), portdir_overlay.end(),
            std::tr1::bind(std::tr1::mem_fn(&PortageEnvironment::_add_portdir_overlay_repository), this, _1));

    /* use etc */

    WhitespaceTokeniser::get_instance()->tokenise(_imp->vars->get("USE"), std::inserter(_imp->use_with_expands,
                _imp->use_with_expands.begin()));
    WhitespaceTokeniser::get_instance()->tokenise(_imp->vars->get("USE_EXPAND"), std::inserter(_imp->use_expand,
                _imp->use_expand.begin()));
    for (std::set<std::string>::const_iterator i(_imp->use_expand.begin()), i_end(_imp->use_expand.end()) ;
            i != i_end ; ++i)
    {
        std::string lower_i;
        std::transform(i->begin(), i->end(), std::back_inserter(lower_i), ::tolower);

        std::set<std::string> values;
        WhitespaceTokeniser::get_instance()->tokenise(_imp->vars->get(*i), std::inserter(values,
                    values.begin()));
        for (std::set<std::string>::const_iterator v(values.begin()), v_end(values.end()) ;
                v != v_end ; ++v)
            _imp->use_with_expands.insert(lower_i + "_" + *v);
    }

    /* accept keywords */
    WhitespaceTokeniser::get_instance()->tokenise(_imp->vars->get("ACCEPT_KEYWORDS"),
            std::inserter(_imp->accept_keywords, _imp->accept_keywords.begin()));

    /* files */

    _load_atom_file(_imp->conf_dir / "portage" / "package.use", std::back_inserter(_imp->package_use), "");
    _load_atom_file(_imp->conf_dir / "portage" / "package.keywords", std::back_inserter(_imp->package_keywords),
            "~" + _imp->vars->get("ARCH"));

    _load_lined_file(_imp->conf_dir / "portage" / "package.mask", std::back_inserter(_imp->package_mask));
    _load_lined_file(_imp->conf_dir / "portage" / "package.unmask", std::back_inserter(_imp->package_unmask));
}

template<typename I_>
void
PortageEnvironment::_load_atom_file(const FSEntry & f, I_ i, const std::string & def_value)
{
    Context context("When loading '" + stringify(f) + "':");

    if (! f.exists())
        return;

    if (f.is_directory())
    {
        for (DirIterator d(f), d_end ; d != d_end ; ++d)
            _load_atom_file(*d, i, def_value);
    }
    else
    {
        LineConfigFile file(f);
        for (LineConfigFile::Iterator line(file.begin()), line_end(file.end()) ;
                line != line_end ; ++line)
        {
            std::vector<std::string> tokens;
            WhitespaceTokeniser::get_instance()->tokenise(*line, std::back_inserter(tokens));

            if (tokens.empty())
                continue;

            std::tr1::shared_ptr<PackageDepSpec> p(new PackageDepSpec(tokens.at(0), pds_pm_unspecific));
            if (1 == tokens.size())
            {
                if (! def_value.empty())
                    *i++ = std::make_pair(p, def_value);
            }
            else
            {
                for (std::vector<std::string>::const_iterator t(next(tokens.begin())), t_end(tokens.end()) ;
                        t != t_end ; ++t)
                    *i++ = std::make_pair(p, *t);
            }
        }
    }
}

template<typename I_>
void
PortageEnvironment::_load_lined_file(const FSEntry & f, I_ i)
{
    Context context("When loading '" + stringify(f) + "':");

    if (! f.exists())
        return;

    if (f.is_directory())
    {
        for (DirIterator d(f), d_end ; d != d_end ; ++d)
            _load_lined_file(*d, i);
    }
    else
    {
        LineConfigFile file(f);
        for (LineConfigFile::Iterator line(file.begin()), line_end(file.end()) ;
                line != line_end ; ++line)
            *i++ = std::tr1::shared_ptr<PackageDepSpec>(new PackageDepSpec(
                        strip_trailing(strip_leading(*line, " \t"), " \t"), pds_pm_unspecific));
    }
}

void
PortageEnvironment::_load_profile(const FSEntry & d)
{
    Context context("When loading profile directory '" + stringify(d) + "':");

    if ((d / "parent").exists())
    {
        Context context_local("When loading parent profiles:");

        LineConfigFile f(d / "parent");
        for (LineConfigFile::Iterator line(f.begin()), line_end(f.end()) ;
                line != line_end ; ++line)
            _load_profile((d / *line).realpath());
    }

    if ((d / "make.defaults").exists())
        _imp->vars.reset(new KeyValueConfigFile(d / "make.defaults", _imp->vars, &is_incremental));
}

void
PortageEnvironment::_add_virtuals_repository()
{
    std::tr1::shared_ptr<AssociativeCollection<std::string, std::string> > keys(
            new AssociativeCollection<std::string, std::string>::Concrete);
    package_database()->add_repository(-2,
            RepositoryMaker::get_instance()->find_maker("virtuals")(this, keys));
}

void
PortageEnvironment::_add_installed_virtuals_repository()
{
    std::tr1::shared_ptr<AssociativeCollection<std::string, std::string> > keys(
            new AssociativeCollection<std::string, std::string>::Concrete);
    keys->insert("root", stringify(root()));
    package_database()->add_repository(-1,
            RepositoryMaker::get_instance()->find_maker("installed_virtuals")(this, keys));
}

void
PortageEnvironment::_add_portdir_repository(const FSEntry & portdir)
{
    Context context("When creating PORTDIR repository:");

    std::tr1::shared_ptr<AssociativeCollection<std::string, std::string> > keys(
            new AssociativeCollection<std::string, std::string>::Concrete);
    keys->insert("root", stringify(root()));
    keys->insert("location", stringify(portdir));
    keys->insert("profiles", stringify((_imp->conf_dir / "make.profile").realpath()) + " " +
            ((_imp->conf_dir / "portage" / "profile").is_directory() ?
             stringify(_imp->conf_dir / "portage" / "profile") : ""));
    keys->insert("format", "ebuild");
    keys->insert("names_cache", "/var/empty");
    package_database()->add_repository(2,
            RepositoryMaker::get_instance()->find_maker("ebuild")(this, keys));
}

void
PortageEnvironment::_add_portdir_overlay_repository(const FSEntry & portdir)
{
    Context context("When creating PORTDIR_OVERLAY repository '" + stringify(portdir) + "':");

    std::tr1::shared_ptr<AssociativeCollection<std::string, std::string> > keys(
            new AssociativeCollection<std::string, std::string>::Concrete);
    keys->insert("root", stringify(root()));
    keys->insert("location", stringify(portdir));
    keys->insert("format", "ebuild");
    keys->insert("names_cache", "/var/empty");
    keys->insert("master_repository", "gentoo");
    package_database()->add_repository(2,
            RepositoryMaker::get_instance()->find_maker("ebuild")(this, keys));
}

void
PortageEnvironment::_add_vdb_repository()
{
    Context context("When creating vdb repository:");

    std::tr1::shared_ptr<AssociativeCollection<std::string, std::string> > keys(
            new AssociativeCollection<std::string, std::string>::Concrete);
    keys->insert("root", stringify(root()));
    keys->insert("location", stringify(root() / "/var/db/pkg"));
    keys->insert("format", "vdb");
    keys->insert("names_cache", "/var/empty");
    keys->insert("provides_cache", "/var/empty");
    package_database()->add_repository(1,
            RepositoryMaker::get_instance()->find_maker("vdb")(this, keys));
}

PortageEnvironment::~PortageEnvironment()
{
}

bool
PortageEnvironment::query_use(const UseFlagName & f, const PackageDatabaseEntry * e) const
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

    UseFlagState state(use_unspecified);

    /* check use: general user config */
    std::set<std::string>::const_iterator u(_imp->use_with_expands.find(stringify(f)));
    if (u != _imp->use_with_expands.end())
        state = use_enabled;

    /* check use: per package config */
    if (e)
    {
        for (PackageUse::const_iterator i(_imp->package_use.begin()), i_end(_imp->package_use.end()) ;
                i != i_end ; ++i)
        {
            if (! match_package(*this, *i->first, *e))
                continue;

            if (i->second == stringify(f))
                state = use_enabled;
            else if (i->second == "-" + stringify(f))
                state = use_disabled;
        }
    }

    switch (state)
    {
        case use_disabled:
        case use_unspecified:
            return false;

        case use_enabled:
            return true;
    }

    throw InternalError(PALUDIS_HERE, "bad state");
}

std::string
PortageEnvironment::paludis_command() const
{
    return _imp->paludis_command;
}

void
PortageEnvironment::set_paludis_command(const std::string & s)
{
    _imp->paludis_command = s;
}

void
PortageEnvironment::force_use(std::tr1::shared_ptr<const PackageDepSpec>,
        const UseFlagName &, const UseFlagState)
{
    throw InternalError(PALUDIS_HERE, "force_use not currently available for PortageEnvironment");
}

void
PortageEnvironment::clear_forced_use()
{
}

bool
PortageEnvironment::accept_keyword(const KeywordName & k, const PackageDatabaseEntry * const d,
        const bool override_tilde_keywords) const
{
    bool result(false);

    if (stringify(k) == "*")
        return true;

    if (_imp->accept_keywords.end() != _imp->accept_keywords.find(stringify(k)))
        result = true;

    if (d)
    {
        for (PackageKeywords::const_iterator i(_imp->package_keywords.begin()), i_end(_imp->package_keywords.end()) ;
                i != i_end ; ++i)
        {
            if (! match_package(*this, *i->first, *d))
                continue;

            if (i->second == stringify(k))
                result = true;
            else if (i->second == "-" + stringify(k))
                result = false;
            else if (i->second == "-*")
                result = false;
            else if (i->second == "**")
                result = true;
        }
    }

    if ((! result) && override_tilde_keywords && ('~' == stringify(k).at(0)))
        result = accept_keyword(KeywordName(stringify(k).substr(1)), d, false);

    return result;
}

FSEntry
PortageEnvironment::root() const
{
    if (_imp->vars->get("ROOT").empty())
        return FSEntry("/");
    else
        return FSEntry(_imp->vars->get("ROOT"));
}

bool
PortageEnvironment::query_user_masks(const PackageDatabaseEntry & e) const
{
    for (PackageMask::const_iterator i(_imp->package_mask.begin()), i_end(_imp->package_mask.end()) ;
            i != i_end ; ++i)
        if (match_package(*this, **i, e))
            return true;

    return false;
}

bool
PortageEnvironment::query_user_unmasks(const PackageDatabaseEntry & e) const
{
    for (PackageMask::const_iterator i(_imp->package_mask.begin()), i_end(_imp->package_mask.end()) ;
            i != i_end ; ++i)
        if (match_package(*this, **i, e))
            return true;

    return false;
}

std::tr1::shared_ptr<const UseFlagNameCollection>
PortageEnvironment::known_use_expand_names(const UseFlagName & prefix,
        const PackageDatabaseEntry * pde) const
{
    Context context("When loading known use expand names for prefix '" + stringify(prefix) + ":");

    std::tr1::shared_ptr<UseFlagNameCollection> result(new UseFlagNameCollection::Concrete);
    std::string prefix_lower;
    std::transform(prefix.data().begin(), prefix.data().end(), std::back_inserter(prefix_lower), &::tolower);

    for (std::set<std::string>::const_iterator i(_imp->use_with_expands.begin()),
            i_end(_imp->use_with_expands.end()) ; i != i_end ; ++i)
        if (0 == i->compare(0, prefix_lower.length(), prefix_lower, 0, prefix_lower.length()))
            result->insert(UseFlagName(*i));

    if (pde)
    {
        for (PackageUse::const_iterator i(_imp->package_use.begin()), i_end(_imp->package_use.end()) ;
                i != i_end ; ++i)
        {
            if (! match_package(*this, *i->first, *pde))
                continue;

            if (0 == i->second.compare(0, prefix_lower.length(), prefix_lower, 0, prefix_lower.length()))
                result->insert(UseFlagName(i->second));
        }
    }

    Log::get_instance()->message(ll_debug, lc_no_context, "PortageEnvironment::known_use_expand_names("
            + stringify(prefix) + ", " + (pde ? stringify(*pde) : stringify("0")) + ") -> ("
            + join(result->begin(), result->end(), ", ") + ")");

    return result;
}

int
PortageEnvironment::perform_hook(const Hook & hook) const
{
    using namespace std::tr1::placeholders;

    if (! _imp->hooker)
    {
        _imp->need_hook_dirs();
        _imp->hooker.reset(new Hooker(this));
        std::for_each(_imp->hook_dirs.begin(), _imp->hook_dirs.end(),
                std::tr1::bind(std::tr1::mem_fn(&Hooker::add_dir), _imp->hooker.get(), _1));
    }

    return _imp->hooker->perform_hook(hook);
}


std::string
PortageEnvironment::hook_dirs() const
{
    _imp->need_hook_dirs();
    return join(_imp->hook_dirs.begin(), _imp->hook_dirs.end(), " ");
}

