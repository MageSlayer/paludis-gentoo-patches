/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh
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
#include <paludis/util/save.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/iterator.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/set.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/map.hh>
#include <paludis/util/options.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/repository_maker.hh>
#include <paludis/util/config_file.hh>
#include <paludis/hooker.hh>
#include <paludis/hook.hh>
#include <paludis/mask.hh>
#include <paludis/match_package.hh>
#include <paludis/package_database.hh>
#include <paludis/package_id.hh>
#include <algorithm>
#include <paludis/util/tr1_functional.hh>
#include <paludis/util/mutex.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>
#include <libwrapiter/libwrapiter_output_iterator.hh>
#include <functional>
#include <set>
#include <map>
#include <vector>
#include <list>

using namespace paludis;
using namespace paludis::portage_environment;

typedef std::list<std::pair<tr1::shared_ptr<const PackageDepSpec>, std::string> > PackageUse;
typedef std::list<std::pair<tr1::shared_ptr<const PackageDepSpec>, std::string> > PackageKeywords;
typedef std::list<tr1::shared_ptr<const PackageDepSpec> > PackageMask;
typedef std::list<tr1::shared_ptr<const PackageDepSpec> > PackageUnmask;

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

        tr1::shared_ptr<KeyValueConfigFile> vars;

        std::set<std::string> use_with_expands;
        std::set<std::string> use_expand;
        std::set<std::string> accept_keywords;
        std::multimap<std::string, std::string> mirrors;

        PackageUse package_use;
        PackageKeywords package_keywords;
        PackageMask package_mask;
        PackageUnmask package_unmask;

        mutable Mutex hook_mutex;
        mutable bool done_hooks;
        mutable tr1::shared_ptr<Hooker> hooker;
        mutable std::list<FSEntry> hook_dirs;

        int overlay_importance;

        tr1::shared_ptr<PackageDatabase> package_database;

        Implementation(Environment * const e, const std::string & s) :
            conf_dir(FSEntry(s.empty() ? "/" : s) / SYSCONFDIR),
            paludis_command("paludis"),
            done_hooks(false),
            overlay_importance(10),
            package_database(new PackageDatabase(e))
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
        WhitespaceTokeniser::tokenise(k.get("USE_EXPAND"),
                std::inserter(use_expand, use_expand.begin()));
        if (use_expand.end() != use_expand.find(s))
            return true;

        return false;
    }
}

PortageEnvironment::PortageEnvironment(const std::string & s) :
    PrivateImplementationPattern<PortageEnvironment>(new Implementation<PortageEnvironment>(this, s))
{
    using namespace tr1::placeholders;

    Context context("When creating PortageEnvironment using config root '" + s + "':");

    Log::get_instance()->message(ll_warning, lc_no_context,
            "Use of Portage configuration files will lead to sub-optimal performance and loss of "
            "functionality. Full support for Portage configuration formats is not "
            "guaranteed; issues should be reported via trac. You are strongly encouraged "
            "to migrate to a Paludis configuration.");

    _imp->vars.reset(new KeyValueConfigFile(FSEntry("/dev/null"), KeyValueConfigFileOptions()));
    _load_profile((_imp->conf_dir / "make.profile").realpath());
    if ((_imp->conf_dir / "make.globals").exists())
        _imp->vars.reset(new KeyValueConfigFile(_imp->conf_dir / "make.globals", KeyValueConfigFileOptions(), _imp->vars, &is_incremental));
    if ((_imp->conf_dir / "make.conf").exists())
        _imp->vars.reset(new KeyValueConfigFile(_imp->conf_dir / "make.conf", KeyValueConfigFileOptions(), _imp->vars,
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
    WhitespaceTokeniser::tokenise(_imp->vars->get("PORTDIR_OVERLAY"),
            create_inserter<FSEntry>(std::back_inserter(portdir_overlay)));
    std::for_each(portdir_overlay.begin(), portdir_overlay.end(),
            tr1::bind(tr1::mem_fn(&PortageEnvironment::_add_portdir_overlay_repository), this, _1));

    /* use etc */

    WhitespaceTokeniser::tokenise(_imp->vars->get("USE"), std::inserter(_imp->use_with_expands,
                _imp->use_with_expands.begin()));
    WhitespaceTokeniser::tokenise(_imp->vars->get("USE_EXPAND"), std::inserter(_imp->use_expand,
                _imp->use_expand.begin()));
    for (std::set<std::string>::const_iterator i(_imp->use_expand.begin()), i_end(_imp->use_expand.end()) ;
            i != i_end ; ++i)
    {
        std::string lower_i;
        std::transform(i->begin(), i->end(), std::back_inserter(lower_i), ::tolower);

        std::set<std::string> values;
        WhitespaceTokeniser::tokenise(_imp->vars->get(*i), std::inserter(values,
                    values.begin()));
        for (std::set<std::string>::const_iterator v(values.begin()), v_end(values.end()) ;
                v != v_end ; ++v)
            _imp->use_with_expands.insert(lower_i + "_" + *v);
    }

    /* accept keywords */
    WhitespaceTokeniser::tokenise(_imp->vars->get("ACCEPT_KEYWORDS"),
            std::inserter(_imp->accept_keywords, _imp->accept_keywords.begin()));

    /* files */

    _load_atom_file(_imp->conf_dir / "portage" / "package.use", std::back_inserter(_imp->package_use), "");
    _load_atom_file(_imp->conf_dir / "portage" / "package.keywords", std::back_inserter(_imp->package_keywords),
            "~" + _imp->vars->get("ARCH"));

    _load_lined_file(_imp->conf_dir / "portage" / "package.mask", std::back_inserter(_imp->package_mask));
    _load_lined_file(_imp->conf_dir / "portage" / "package.unmask", std::back_inserter(_imp->package_unmask));

    /* mirrors */
    std::list<std::string> gentoo_mirrors;
    WhitespaceTokeniser::tokenise(_imp->vars->get("GENTOO_MIRRORS"),
            std::back_inserter(gentoo_mirrors));
    for (std::list<std::string>::const_iterator m(gentoo_mirrors.begin()), m_end(gentoo_mirrors.end()) ;
            m != m_end ; ++m)
        _imp->mirrors.insert(std::make_pair("*", *m + "/distfiles/"));

    if ((_imp->conf_dir / "portage" / "mirrors").exists())
    {
        LineConfigFile m(_imp->conf_dir / "portage" / "mirrors", LineConfigFileOptions());
        for (LineConfigFile::ConstIterator line(m.begin()), line_end(m.end()) ;
                line != line_end ; ++line)
        {
            std::vector<std::string> tokens;
            WhitespaceTokeniser::tokenise(*line, std::back_inserter(tokens));
            if (tokens.size() < 2)
                continue;

            for (std::vector<std::string>::const_iterator t(next(tokens.begin())), t_end(tokens.end()) ;
                    t != t_end ; ++t)
                _imp->mirrors.insert(std::make_pair(tokens.at(0), *t));
        }
    }
}

template<typename I_>
void
PortageEnvironment::_load_atom_file(const FSEntry & f, I_ i, const std::string & def_value)
{
    using namespace tr1::placeholders;

    Context context("When loading '" + stringify(f) + "':");

    if (! f.exists())
        return;

    if (f.is_directory())
    {
        std::for_each(DirIterator(f), DirIterator(), tr1::bind(tr1::mem_fn(
                        &PortageEnvironment::_load_atom_file<I_>), this, _1, i, def_value));
    }
    else
    {
        LineConfigFile file(f, LineConfigFileOptions());
        for (LineConfigFile::ConstIterator line(file.begin()), line_end(file.end()) ;
                line != line_end ; ++line)
        {
            std::vector<std::string> tokens;
            WhitespaceTokeniser::tokenise(*line, std::back_inserter(tokens));

            if (tokens.empty())
                continue;

            tr1::shared_ptr<PackageDepSpec> p(new PackageDepSpec(tokens.at(0), pds_pm_permissive));
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
    using namespace tr1::placeholders;

    Context context("When loading '" + stringify(f) + "':");

    if (! f.exists())
        return;

    if (f.is_directory())
    {
        std::for_each(DirIterator(f), DirIterator(), tr1::bind(tr1::mem_fn(
                        &PortageEnvironment::_load_lined_file<I_>), this, _1, i));
    }
    else
    {
        LineConfigFile file(f, LineConfigFileOptions());
        for (LineConfigFile::ConstIterator line(file.begin()), line_end(file.end()) ;
                line != line_end ; ++line)
            *i++ = tr1::shared_ptr<PackageDepSpec>(new PackageDepSpec(
                        strip_trailing(strip_leading(*line, " \t"), " \t"), pds_pm_permissive));
    }
}

void
PortageEnvironment::_load_profile(const FSEntry & d)
{
    Context context("When loading profile directory '" + stringify(d) + "':");

    if ((d / "parent").exists())
    {
        Context context_local("When loading parent profiles:");

        LineConfigFile f(d / "parent", LineConfigFileOptions() + lcfo_disallow_continuations + lcfo_disallow_comments);
        for (LineConfigFile::ConstIterator line(f.begin()), line_end(f.end()) ;
                line != line_end ; ++line)
            _load_profile((d / *line).realpath());
    }

    if ((d / "make.defaults").exists())
        _imp->vars.reset(new KeyValueConfigFile(d / "make.defaults", KeyValueConfigFileOptions(), _imp->vars, &is_incremental));
}

void
PortageEnvironment::_add_virtuals_repository()
{
    tr1::shared_ptr<Map<std::string, std::string> > keys(
            new Map<std::string, std::string>);
    package_database()->add_repository(-2,
            RepositoryMaker::get_instance()->find_maker("virtuals")(this, keys));
}

void
PortageEnvironment::_add_installed_virtuals_repository()
{
    tr1::shared_ptr<Map<std::string, std::string> > keys(
            new Map<std::string, std::string>);
    keys->insert("root", stringify(root()));
    package_database()->add_repository(-1,
            RepositoryMaker::get_instance()->find_maker("installed_virtuals")(this, keys));
}

void
PortageEnvironment::_add_portdir_repository(const FSEntry & portdir)
{
    Context context("When creating PORTDIR repository:");
    _add_ebuild_repository(portdir, "", _imp->vars->get("SYNC"), 1);
}

void
PortageEnvironment::_add_ebuild_repository(const FSEntry & portdir, const std::string & master,
        const std::string & sync, int importance)
{
    tr1::shared_ptr<Map<std::string, std::string> > keys(
            new Map<std::string, std::string>);
    keys->insert("root", stringify(root()));
    keys->insert("location", stringify(portdir));
    keys->insert("profiles", stringify((_imp->conf_dir / "make.profile").realpath()) + " " +
            ((_imp->conf_dir / "portage" / "profile").is_directory() ?
             stringify(_imp->conf_dir / "portage" / "profile") : ""));
    keys->insert("format", "ebuild");
    keys->insert("names_cache", "/var/empty");
    keys->insert("master_repository", master);
    keys->insert("sync", sync);
    keys->insert("distdir", stringify(_imp->vars->get("DISTDIR")));
    std::string builddir(_imp->vars->get("PORTAGE_TMPDIR"));
    if (! builddir.empty())
        builddir.append("/portage");
    keys->insert("builddir", builddir);

    package_database()->add_repository(importance,
            RepositoryMaker::get_instance()->find_maker("ebuild")(this, keys));
}

void
PortageEnvironment::_add_portdir_overlay_repository(const FSEntry & portdir)
{
    Context context("When creating PORTDIR_OVERLAY repository '" + stringify(portdir) + "':");
    _add_ebuild_repository(portdir, "gentoo", "", ++_imp->overlay_importance);
}

void
PortageEnvironment::_add_vdb_repository()
{
    Context context("When creating vdb repository:");

    tr1::shared_ptr<Map<std::string, std::string> > keys(
            new Map<std::string, std::string>);
    keys->insert("root", stringify(root()));
    keys->insert("location", stringify(root() / "/var/db/pkg"));
    keys->insert("format", "vdb");
    keys->insert("names_cache", "/var/empty");
    keys->insert("provides_cache", "/var/empty");
    keys->insert("world", "/var/lib/portage/world");
    std::string builddir(_imp->vars->get("PORTAGE_TMPDIR"));
    if (! builddir.empty())
        builddir.append("/portage");
    keys->insert("builddir", builddir);
    package_database()->add_repository(1,
            RepositoryMaker::get_instance()->find_maker("vdb")(this, keys));
}

PortageEnvironment::~PortageEnvironment()
{
}

bool
PortageEnvironment::query_use(const UseFlagName & f, const PackageID & e) const
{
    Context context("When querying use flag '" + stringify(f) + "' for '" + stringify(e) +
            "' in Portage environment:");

    PALUDIS_TLS bool recursive(false);
    if (recursive)
    {
        Log::get_instance()->message(ll_warning, lc_context) <<
            "use flag state is defined recursively, forcing it to disabled instead";
        return false;
    }
    Save<bool> save_recursive(&recursive, true);

    /* first check package database use masks... */
    if (e.repository()->use_interface)
    {
        if (e.repository()->use_interface->query_use_mask(f, e))
            return false;
        if (e.repository()->use_interface->query_use_force(f, e))
            return true;
    }

    UseFlagState state(use_unspecified);

    /* check use: repo */
    if (e.repository()->use_interface)
        state = e.repository()->use_interface->query_use(f, e);

    /* check use: general user config */
    std::set<std::string>::const_iterator u(_imp->use_with_expands.find(stringify(f)));
    if (u != _imp->use_with_expands.end())
        state = use_enabled;

    /* check use: per package config */
    for (PackageUse::const_iterator i(_imp->package_use.begin()), i_end(_imp->package_use.end()) ;
            i != i_end ; ++i)
    {
        if (! match_package(*this, *i->first, e))
            continue;

        if (i->second == stringify(f))
            state = use_enabled;
        else if (i->second == "-" + stringify(f))
            state = use_disabled;
    }

    switch (state)
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

bool
PortageEnvironment::accept_keywords(tr1::shared_ptr <const KeywordNameSet> keywords,
        const PackageID & d) const
{
    bool result(false);

    if (keywords->end() != keywords->find(KeywordName("*")))
        return true;

    for (KeywordNameSet::ConstIterator k(keywords->begin()), k_end(keywords->end()) ;
            k != k_end ; ++k)
    {
        bool local_result(false);

        if (_imp->accept_keywords.end() != _imp->accept_keywords.find(stringify(*k)))
            local_result = true;

        for (PackageKeywords::const_iterator i(_imp->package_keywords.begin()), i_end(_imp->package_keywords.end()) ;
                i != i_end ; ++i)
        {
            if (! match_package(*this, *i->first, d))
                continue;

            if (i->second == stringify(*k))
                local_result = true;
            else if (i->second == "-" + stringify(*k))
                local_result = false;
            else if (i->second == "-*")
                local_result = false;
            else if (i->second == "**")
                local_result = true;
        }

        result |= local_result;
    }

    return result;
}

const FSEntry
PortageEnvironment::root() const
{
    if (_imp->vars->get("ROOT").empty())
        return FSEntry("/");
    else
        return FSEntry(_imp->vars->get("ROOT"));
}

bool
PortageEnvironment::unmasked_by_user(const PackageID & e) const
{
    for (PackageUnmask::const_iterator i(_imp->package_unmask.begin()), i_end(_imp->package_unmask.end()) ;
            i != i_end ; ++i)
        if (match_package(*this, **i, e))
            return true;

    return false;
}

tr1::shared_ptr<const UseFlagNameSet>
PortageEnvironment::known_use_expand_names(const UseFlagName & prefix,
        const PackageID & pde) const
{
    Context context("When loading known use expand names for prefix '" + stringify(prefix) + ":");

    tr1::shared_ptr<UseFlagNameSet> result(new UseFlagNameSet);
    std::string prefix_lower;
    std::transform(prefix.data().begin(), prefix.data().end(), std::back_inserter(prefix_lower), &::tolower);
    prefix_lower.append("_");

    for (std::set<std::string>::const_iterator i(_imp->use_with_expands.begin()),
            i_end(_imp->use_with_expands.end()) ; i != i_end ; ++i)
        if (0 == i->compare(0, prefix_lower.length(), prefix_lower, 0, prefix_lower.length()))
            result->insert(UseFlagName(*i));

    for (PackageUse::const_iterator i(_imp->package_use.begin()), i_end(_imp->package_use.end()) ;
            i != i_end ; ++i)
    {
        if (! match_package(*this, *i->first, pde))
            continue;

        if (0 == i->second.compare(0, prefix_lower.length(), prefix_lower, 0, prefix_lower.length()))
            result->insert(UseFlagName(i->second));
    }

    Log::get_instance()->message(ll_debug, lc_no_context) << "PortageEnvironment::known_use_expand_names("
            << prefix << ", " << pde << ") -> (" << join(result->begin(), result->end(), ", ") << ")";

    return result;
}

HookResult
PortageEnvironment::perform_hook(const Hook & hook) const
{
    using namespace tr1::placeholders;

    Lock l(_imp->hook_mutex);
    if (! _imp->hooker)
    {
        _imp->need_hook_dirs();
        _imp->hooker.reset(new Hooker(this));
        std::for_each(_imp->hook_dirs.begin(), _imp->hook_dirs.end(),
                tr1::bind(tr1::mem_fn(&Hooker::add_dir), _imp->hooker.get(), _1, false));
    }

    return _imp->hooker->perform_hook(hook);
}

tr1::shared_ptr<const FSEntrySequence>
PortageEnvironment::hook_dirs() const
{
    Lock l(_imp->hook_mutex);
    _imp->need_hook_dirs();
    tr1::shared_ptr<FSEntrySequence> result(new FSEntrySequence);
    std::copy(_imp->hook_dirs.begin(), _imp->hook_dirs.end(), result->back_inserter());
    return result;
}

tr1::shared_ptr<const FSEntrySequence>
PortageEnvironment::bashrc_files() const
{
    tr1::shared_ptr<FSEntrySequence> result(new FSEntrySequence);
    result->push_back(FSEntry(LIBEXECDIR) / "paludis" / "environments" / "portage" / "bashrc");
    result->push_back(_imp->conf_dir / "make.globals");
    result->push_back(_imp->conf_dir / "make.conf");
    return result;
}

tr1::shared_ptr<PackageDatabase>
PortageEnvironment::package_database()
{
    return _imp->package_database;
}

tr1::shared_ptr<const PackageDatabase>
PortageEnvironment::package_database() const
{
    return _imp->package_database;
}

tr1::shared_ptr<const MirrorsSequence>
PortageEnvironment::mirrors(const std::string & m) const
{
    std::pair<std::multimap<std::string, std::string>::const_iterator, std::multimap<std::string, std::string>::const_iterator>
        p(_imp->mirrors.equal_range(m));
    tr1::shared_ptr<MirrorsSequence> result(new MirrorsSequence);
    std::copy(p.first, p.second, transform_inserter(result->back_inserter(),
                tr1::mem_fn(&std::pair<const std::string, std::string>::second)));
    return result;
}

tr1::shared_ptr<SetSpecTree::ConstItem>
PortageEnvironment::local_set(const SetName &) const
{
    return tr1::shared_ptr<SetSpecTree::ConstItem>();
}

bool
PortageEnvironment::accept_license(const std::string &, const PackageID &) const
{
    return true;
}

namespace
{
    class BreaksPortageMask :
        public UnsupportedMask
    {
        const char key() const
        {
            return 'B';
        }

        const std::string description() const
        {
            return "breaks Portage";
        }

        const std::string explanation() const
        {
            return "";
        }
    };

    class UserConfigMask :
        public UserMask
    {
        const char key() const
        {
            return 'U';
        }

        const std::string description() const
        {
            return "user";
        }
    };
}

const tr1::shared_ptr<const Mask>
PortageEnvironment::mask_for_breakage(const PackageID & id) const
{
    if (id.breaks_portage())
        return make_shared_ptr(new BreaksPortageMask);

    return tr1::shared_ptr<const Mask>();
}

const tr1::shared_ptr<const Mask>
PortageEnvironment::mask_for_user(const PackageID & d) const
{
    for (PackageMask::const_iterator i(_imp->package_mask.begin()), i_end(_imp->package_mask.end()) ;
            i != i_end ; ++i)
        if (match_package(*this, **i, d))
            return make_shared_ptr(new UserConfigMask);

    return tr1::shared_ptr<const Mask>();
}

gid_t
PortageEnvironment::reduced_gid() const
{
    return getgid();
}

uid_t
PortageEnvironment::reduced_uid() const
{
    return getuid();
}

