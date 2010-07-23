/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010 Ciaran McCreesh
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
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/create_iterator-impl.hh>
#include <paludis/util/iterator_funcs.hh>
#include <paludis/util/save.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/set.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/map.hh>
#include <paludis/util/options.hh>
#include <paludis/util/config_file.hh>
#include <paludis/util/tribool.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/standard_output_manager.hh>
#include <paludis/util/safe_ofstream.hh>
#include <paludis/hooker.hh>
#include <paludis/hook.hh>
#include <paludis/mask.hh>
#include <paludis/match_package.hh>
#include <paludis/package_database.hh>
#include <paludis/package_id.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/set_file.hh>
#include <paludis/dep_tag.hh>
#include <paludis/util/mutex.hh>
#include <paludis/literal_metadata_key.hh>
#include <paludis/repository_factory.hh>
#include <paludis/choice.hh>
#include <functional>
#include <algorithm>
#include <set>
#include <map>
#include <vector>
#include <list>
#include "config.h"

using namespace paludis;
using namespace paludis::portage_environment;

typedef std::list<std::pair<std::shared_ptr<const PackageDepSpec>, std::string> > PackageUse;
typedef std::list<std::pair<std::shared_ptr<const PackageDepSpec>, std::string> > PackageKeywords;
typedef std::list<std::shared_ptr<const PackageDepSpec> > PackageMask;
typedef std::list<std::shared_ptr<const PackageDepSpec> > PackageUnmask;

PortageEnvironmentConfigurationError::PortageEnvironmentConfigurationError(const std::string & s) throw () :
    ConfigurationError(s)
{
}

namespace paludis
{
    template<>
    struct Imp<PortageEnvironment>
    {
        const FSEntry conf_dir;
        std::string paludis_command;

        std::shared_ptr<KeyValueConfigFile> vars;

        std::multimap<ChoicePrefixName, std::string> use_and_expands;
        std::set<std::string> use_expand;
        std::set<std::string> accept_keywords;
        std::multimap<std::string, std::string> mirrors;

        PackageUse package_use;
        PackageKeywords package_keywords;
        PackageMask package_mask;
        PackageUnmask package_unmask;

        std::set<std::string> ignore_breaks_portage;
        bool ignore_all_breaks_portage;

        mutable Mutex hook_mutex;
        mutable bool done_hooks;
        mutable std::shared_ptr<Hooker> hooker;
        mutable std::list<FSEntry> hook_dirs;

        int overlay_importance;

        std::shared_ptr<PackageDatabase> package_database;

        const FSEntry world_file;
        mutable Mutex world_mutex;

        std::shared_ptr<LiteralMetadataValueKey<std::string> > format_key;
        std::shared_ptr<LiteralMetadataValueKey<FSEntry> > config_location_key;
        std::shared_ptr<LiteralMetadataValueKey<FSEntry> > world_file_key;

        Imp(Environment * const e, const std::string & s) :
            conf_dir(FSEntry(s.empty() ? "/" : s) / SYSCONFDIR),
            paludis_command("paludis"),
            ignore_all_breaks_portage(false),
            done_hooks(false),
            overlay_importance(10),
            package_database(new PackageDatabase(e)),
            world_file(s + "/var/lib/portage/world"),
            format_key(new LiteralMetadataValueKey<std::string>("format", "Format", mkt_significant, "portage")),
            config_location_key(new LiteralMetadataValueKey<FSEntry>("conf_dir", "Config dir", mkt_normal,
                        conf_dir)),
            world_file_key(new LiteralMetadataValueKey<FSEntry>("world_file", "World file", mkt_normal,
                        world_file))
        {
        }

        void add_one_hook(const FSEntry & r) const
        {
            try
            {
                if (r.is_directory())
                {
                    Log::get_instance()->message("portage_environment.hooks.add_dir", ll_debug, lc_no_context)
                        << "Adding hook directory '" << r << "'";
                    hook_dirs.push_back(r);
                }
                else
                    Log::get_instance()->message("portage_environment.hooks.skipping", ll_debug, lc_no_context)
                        << "Skipping hook directory candidate '" << r << "'";
            }
            catch (const FSError & e)
            {
                Log::get_instance()->message("portage_environment.hooks.failure", ll_warning, lc_no_context)
                    << "Caught exception '" << e.message() << "' (" << e.what() << ") when checking hook "
                    "directory '" << r << "'";
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
    bool is_incremental(const KeyValueConfigFile &, const std::string & s)
    {
        return (s == "USE" || s == "USE_EXPAND" || s == "USE_EXPAND_HIDDEN" ||
                s == "CONFIG_PROTECT" || s == "CONFIG_PROTECT_MASK" || s == "FEATURES"
                || s == "ACCEPT_KEYWORDS");
    }

    std::string predefined(const std::shared_ptr<const KeyValueConfigFile> & k,
            const KeyValueConfigFile &, const std::string & s)
    {
        return k->get(s);
    }

    std::string make_incremental(const KeyValueConfigFile &, const std::string &, const std::string & before, const std::string & value)
    {
        if (before.empty())
            return value;

        std::list<std::string> values;
        std::set<std::string> new_values;
        tokenise_whitespace(before, std::back_inserter(values));
        tokenise_whitespace(value, std::back_inserter(values));

        for (std::list<std::string>::const_iterator v(values.begin()), v_end(values.end()) ;
                v != v_end ; ++v)
        {
            if (v->empty())
                continue;
            else if ("-*" == *v)
                new_values.clear();
            else if ('-' == v->at(0))
                new_values.erase(v->substr(1));
            new_values.insert(*v);
        }

        return join(new_values.begin(), new_values.end(), " ");
    }

    std::string do_incremental(const KeyValueConfigFile & k, const std::string & var, const std::string & before, const std::string & value)
    {
        if (! is_incremental(k, var))
            return value;
        return make_incremental(k, var, before, value);
    }

    std::string from_keys(const std::shared_ptr<const Map<std::string, std::string> > & m,
            const std::string & k)
    {
        Map<std::string, std::string>::ConstIterator mm(m->find(k));
        if (m->end() == mm)
            return "";
        else
            return mm->second;
    }
}

PortageEnvironment::PortageEnvironment(const std::string & s) :
    Pimp<PortageEnvironment>(this, s),
    _imp(Pimp<PortageEnvironment>::_imp)
{
    using namespace std::placeholders;

    Context context("When creating PortageEnvironment using config root '" + s + "':");

    Log::get_instance()->message("portage_environment.dodgy", ll_warning, lc_no_context) <<
        "Use of Portage configuration files will lead to sub-optimal performance and loss of "
        "functionality. Full support for Portage configuration formats is not "
        "guaranteed; issues should be reported via trac.";

    _imp->vars.reset(new KeyValueConfigFile(FSEntry("/dev/null"), KeyValueConfigFileOptions(),
                &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation));
    _load_profile((_imp->conf_dir / "make.profile").realpath());

    if ((_imp->conf_dir / "make.globals").exists())
        _imp->vars.reset(new KeyValueConfigFile(_imp->conf_dir / "make.globals", KeyValueConfigFileOptions() +
                    kvcfo_disallow_space_inside_unquoted_values + kvcfo_allow_inline_comments + kvcfo_allow_multiple_assigns_per_line,
                    std::bind(&predefined, _imp->vars, std::placeholders::_1, std::placeholders::_2),
                    &do_incremental));
    if ((_imp->conf_dir / "make.conf").exists())
        _imp->vars.reset(new KeyValueConfigFile(_imp->conf_dir / "make.conf", KeyValueConfigFileOptions() +
                    kvcfo_disallow_space_inside_unquoted_values + kvcfo_allow_inline_comments + kvcfo_allow_multiple_assigns_per_line,
                    std::bind(&predefined, _imp->vars, std::placeholders::_1, std::placeholders::_2),
                    &do_incremental));

    /* TODO: load USE etc from env? */

    /* repositories */

    _add_virtuals_repository();
    _add_installed_virtuals_repository();
    if (_imp->vars->get("PORTDIR").empty())
        throw PortageEnvironmentConfigurationError("PORTDIR empty or unset");
    _add_portdir_repository(FSEntry(_imp->vars->get("PORTDIR")));
    _add_vdb_repository();
    std::list<FSEntry> portdir_overlay;
    tokenise_whitespace(_imp->vars->get("PORTDIR_OVERLAY"),
            create_inserter<FSEntry>(std::back_inserter(portdir_overlay)));
    std::for_each(portdir_overlay.begin(), portdir_overlay.end(),
            std::bind(std::mem_fn(&PortageEnvironment::_add_portdir_overlay_repository), this, _1));

    /* use etc */

    {
        std::list<std::string> use;
        tokenise_whitespace(_imp->vars->get("USE"), std::back_inserter(use));
        for (std::list<std::string>::const_iterator u(use.begin()), u_end(use.end()) ;
                u != u_end ; ++u)
            _imp->use_and_expands.insert(std::make_pair(ChoicePrefixName(""), *u));
    }

    tokenise_whitespace(_imp->vars->get("USE_EXPAND"), std::inserter(_imp->use_expand, _imp->use_expand.begin()));
    for (std::set<std::string>::const_iterator i(_imp->use_expand.begin()), i_end(_imp->use_expand.end()) ;
            i != i_end ; ++i)
    {
        std::string lower_i;
        std::transform(i->begin(), i->end(), std::back_inserter(lower_i), ::tolower);

        std::set<std::string> values;
        tokenise_whitespace(_imp->vars->get(*i), std::inserter(values, values.begin()));
        for (std::set<std::string>::const_iterator v(values.begin()), v_end(values.end()) ;
                v != v_end ; ++v)
            _imp->use_and_expands.insert(std::make_pair(ChoicePrefixName(lower_i), *v));
    }

    /* accept keywords */
    tokenise_whitespace(_imp->vars->get("ACCEPT_KEYWORDS"),
            std::inserter(_imp->accept_keywords, _imp->accept_keywords.begin()));

    /* files */

    _load_atom_file(_imp->conf_dir / "portage" / "package.use", std::back_inserter(_imp->package_use), "", true);
    _load_atom_file(_imp->conf_dir / "portage" / "package.keywords", std::back_inserter(_imp->package_keywords),
            "~" + _imp->vars->get("ARCH"), false);

    _load_lined_file(_imp->conf_dir / "portage" / "package.mask", std::back_inserter(_imp->package_mask));
    _load_lined_file(_imp->conf_dir / "portage" / "package.unmask", std::back_inserter(_imp->package_unmask));

    /* mirrors */
    std::list<std::string> gentoo_mirrors;
    tokenise_whitespace(_imp->vars->get("GENTOO_MIRRORS"),
            std::back_inserter(gentoo_mirrors));
    for (std::list<std::string>::const_iterator m(gentoo_mirrors.begin()), m_end(gentoo_mirrors.end()) ;
            m != m_end ; ++m)
        _imp->mirrors.insert(std::make_pair("*", *m + "/distfiles/"));

    if ((_imp->conf_dir / "portage" / "mirrors").exists())
    {
        LineConfigFile m(_imp->conf_dir / "portage" / "mirrors", LineConfigFileOptions() + lcfo_disallow_continuations);
        for (LineConfigFile::ConstIterator line(m.begin()), line_end(m.end()) ;
                line != line_end ; ++line)
        {
            std::vector<std::string> tokens;
            tokenise_whitespace(*line, std::back_inserter(tokens));
            if (tokens.size() < 2)
                continue;

            for (std::vector<std::string>::const_iterator t(next(tokens.begin())), t_end(tokens.end()) ;
                    t != t_end ; ++t)
                _imp->mirrors.insert(std::make_pair(tokens.at(0), *t));
        }
    }

    std::list<std::string> ignore_breaks_portage;
    tokenise_whitespace(_imp->vars->get("PALUDIS_IGNORE_BREAKS_PORTAGE"), std::back_inserter(ignore_breaks_portage));
    for (std::list<std::string>::const_iterator it(ignore_breaks_portage.begin()),
             it_end(ignore_breaks_portage.end()); it_end != it; ++it)
        if ("*" == *it)
        {
            _imp->ignore_all_breaks_portage = true;
            break;
        }
        else
            _imp->ignore_breaks_portage.insert(*it);

    add_metadata_key(_imp->format_key);
    add_metadata_key(_imp->config_location_key);
    add_metadata_key(_imp->world_file_key);
}

template<typename I_>
void
PortageEnvironment::_load_atom_file(const FSEntry & f, I_ i, const std::string & def_value, const bool reject_additional)
{
    using namespace std::placeholders;

    Context context("When loading '" + stringify(f) + "':");

    if (! f.exists())
        return;

    if (f.is_directory())
    {
        std::for_each(DirIterator(f), DirIterator(), std::bind(
                    &PortageEnvironment::_load_atom_file<I_>, this, _1, i, def_value, reject_additional));
    }
    else
    {
        LineConfigFile file(f, LineConfigFileOptions() + lcfo_disallow_continuations);
        for (LineConfigFile::ConstIterator line(file.begin()), line_end(file.end()) ;
                line != line_end ; ++line)
        {
            std::vector<std::string> tokens;
            tokenise_whitespace(*line, std::back_inserter(tokens));

            if (tokens.empty())
                continue;

            std::shared_ptr<PackageDepSpec> p(new PackageDepSpec(parse_user_package_dep_spec(
                            tokens.at(0), this, UserPackageDepSpecOptions() + updso_no_disambiguation)));
            if (reject_additional && p->additional_requirements_ptr())
            {
                Log::get_instance()->message("portage_environment.bad_spec", ll_warning, lc_context)
                    << "Dependency specification '" << stringify(*p)
                    << "' includes use requirements, which cannot be used here";
                continue;
            }

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
    using namespace std::placeholders;

    Context context("When loading '" + stringify(f) + "':");

    if (! f.exists())
        return;

    if (f.is_directory())
    {
        std::for_each(DirIterator(f), DirIterator(), std::bind(
                    &PortageEnvironment::_load_lined_file<I_>, this, _1, i));
    }
    else
    {
        LineConfigFile file(f, LineConfigFileOptions() + lcfo_disallow_continuations);
        for (LineConfigFile::ConstIterator line(file.begin()), line_end(file.end()) ;
                line != line_end ; ++line)
            *i++ = std::shared_ptr<PackageDepSpec>(new PackageDepSpec(
                        parse_user_package_dep_spec(strip_trailing(strip_leading(*line, " \t"), " \t"),
                            this, UserPackageDepSpecOptions() + updso_no_disambiguation)));
    }
}

void
PortageEnvironment::_load_profile(const FSEntry & d)
{
    Context context("When loading profile directory '" + stringify(d) + "':");

    if ((d / "parent").exists())
    {
        Context context_local("When loading parent profiles:");

        LineConfigFile f(d / "parent", LineConfigFileOptions() + lcfo_disallow_continuations);
        for (LineConfigFile::ConstIterator line(f.begin()), line_end(f.end()) ;
                line != line_end ; ++line)
            _load_profile((d / *line).realpath());
    }

    if ((d / "make.defaults").exists())
        _imp->vars.reset(new KeyValueConfigFile(d / "make.defaults", KeyValueConfigFileOptions()
                    + kvcfo_disallow_source + kvcfo_disallow_space_inside_unquoted_values + kvcfo_allow_inline_comments + kvcfo_allow_multiple_assigns_per_line,
                    std::bind(&predefined, _imp->vars, std::placeholders::_1, std::placeholders::_2),
                    &do_incremental));

}

void
PortageEnvironment::_add_virtuals_repository()
{
#ifdef ENABLE_VIRTUALS_REPOSITORY
    std::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
    keys->insert("format", "virtuals");
    package_database()->add_repository(-2,
            RepositoryFactory::get_instance()->create(this, std::bind(from_keys, keys, std::placeholders::_1)));
#endif
}

void
PortageEnvironment::_add_installed_virtuals_repository()
{
#ifdef ENABLE_VIRTUALS_REPOSITORY
    std::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
    keys->insert("root", stringify(root()));
    keys->insert("format", "installed_virtuals");
    package_database()->add_repository(-1,
            RepositoryFactory::get_instance()->create(this, std::bind(from_keys, keys, std::placeholders::_1)));
#endif
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
    std::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
    keys->insert("root", stringify(root()));
    keys->insert("location", stringify(portdir));
    keys->insert("profiles", stringify((_imp->conf_dir / "make.profile").realpath()) + " " +
            ((_imp->conf_dir / "portage" / "profile").is_directory() ?
             stringify(_imp->conf_dir / "portage" / "profile") : ""));
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("master_repository", master);
    keys->insert("sync", sync);
    keys->insert("distdir", stringify(_imp->vars->get("DISTDIR")));
    std::string builddir(_imp->vars->get("PORTAGE_TMPDIR"));
    if (! builddir.empty())
        builddir.append("/portage");
    keys->insert("builddir", builddir);

    package_database()->add_repository(importance,
            RepositoryFactory::get_instance()->create(this, std::bind(from_keys, keys, std::placeholders::_1)));
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

    std::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
    keys->insert("root", stringify(root()));
    keys->insert("location", stringify(root() / "/var/db/pkg"));
    keys->insert("format", "vdb");
    keys->insert("names_cache", "/var/empty");
    keys->insert("provides_cache", "/var/empty");
    std::string builddir(_imp->vars->get("PORTAGE_TMPDIR"));
    if (! builddir.empty())
        builddir.append("/portage");
    keys->insert("builddir", builddir);
    package_database()->add_repository(1,
            RepositoryFactory::get_instance()->create(this, std::bind(from_keys, keys, std::placeholders::_1)));
}

PortageEnvironment::~PortageEnvironment()
{
}

const Tribool
PortageEnvironment::want_choice_enabled(
        const std::shared_ptr<const PackageID> & id,
        const std::shared_ptr<const Choice> & choice,
        const UnprefixedChoiceName & suffix) const
{
    Context context("When querying flag '" + stringify(suffix) + "' for choice '" + choice->human_name() + "' for ID '" + stringify(*id) +
            "' in Portage environment:");

    Tribool state(indeterminate);

    /* check use: general user config */
    std::pair<std::multimap<ChoicePrefixName, std::string>::const_iterator,
        std::multimap<ChoicePrefixName, std::string>::const_iterator> p(_imp->use_and_expands.equal_range(choice->prefix()));

    /* use expand? if it's mentioned at all, pretend it's like -* */
    if ((! stringify(choice->prefix()).empty()) && p.first != p.second)
        state = false;

    for (std::multimap<ChoicePrefixName, std::string>::const_iterator i(p.first), i_end(p.second) ;
            i != i_end ; ++i)
    {
        if (i->second == "-*")
            state = false;
        else if (i->second == stringify(suffix))
            state = true;
        else if (i->second == "-" + stringify(suffix))
            state = false;
    }

    ChoiceNameWithPrefix f(stringify(choice->prefix()) + (stringify(choice->prefix()).empty() ? "" : "_") + stringify(suffix));

    /* check use: per package config */
    for (PackageUse::const_iterator i(_imp->package_use.begin()), i_end(_imp->package_use.end()) ;
            i != i_end ; ++i)
    {
        if (! match_package(*this, *i->first, *id, MatchPackageOptions()))
            continue;

        if (i->second == stringify(f))
            state = true;
        else if (i->second == "-" + stringify(f))
            state = false;
    }

    return state;
}

const std::string
PortageEnvironment::value_for_choice_parameter(
        const std::shared_ptr<const PackageID> &,
        const std::shared_ptr<const Choice> &,
        const UnprefixedChoiceName &) const
{
    return "";
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

namespace
{
    bool is_tilde_keyword(const KeywordName & k)
    {
        std::string k_s(stringify(k));
        return 0 == k_s.compare(0, 1, "~");
    }
}

bool
PortageEnvironment::accept_keywords(const std::shared_ptr <const KeywordNameSet> & keywords,
        const PackageID & d) const
{
    if (keywords->end() != keywords->find(KeywordName("*")))
        return true;

    std::set<std::string> accepted;
    bool accept_star_star(false), accept_tilde_star(false);

    std::copy(_imp->accept_keywords.begin(), _imp->accept_keywords.end(), std::inserter(accepted, accepted.begin()));
    for (PackageKeywords::const_iterator it(_imp->package_keywords.begin()),
             it_end(_imp->package_keywords.end()); it_end != it; ++it)
    {
        if (! match_package(*this, *it->first, d, MatchPackageOptions()))
            continue;

        if ("-*" == it->second)
            accepted.clear();
        else if ('-' == it->second.at(0))
            accepted.erase(it->second.substr(1));
        else if ("**" == it->second)
            accept_star_star = true;
        else if ("~*" == it->second)
            accept_tilde_star = true;
        else
            accepted.insert(it->second);
    }

    if (accept_star_star)
        return true;

    if (accept_tilde_star)
        if (keywords->end() != std::find_if(keywords->begin(), keywords->end(), is_tilde_keyword))
            return true;

    for (KeywordNameSet::ConstIterator it(keywords->begin()),
             it_end(keywords->end()); it_end != it; ++it)
        if (accepted.end() != accepted.find(stringify(*it)))
            return true;

    return false;
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
        if (match_package(*this, **i, e, MatchPackageOptions()))
            return true;

    return false;
}

std::shared_ptr<const Set<UnprefixedChoiceName> >
PortageEnvironment::known_choice_value_names(const std::shared_ptr<const PackageID> & id,
        const std::shared_ptr<const Choice> & choice) const
{
    Context context("When loading known use expand names for prefix '" + stringify(choice->prefix()) + ":");

    std::shared_ptr<Set<UnprefixedChoiceName> > result(new Set<UnprefixedChoiceName>);
    std::string prefix_lower(stringify(choice->prefix()) + "_");

    std::pair<std::multimap<ChoicePrefixName, std::string>::const_iterator,
        std::multimap<ChoicePrefixName, std::string>::const_iterator> p(_imp->use_and_expands.equal_range(choice->prefix()));
    for (std::multimap<ChoicePrefixName, std::string>::const_iterator i(p.first), i_end(p.second) ;
            i != i_end ; ++i)
        if ('-' != i->second.at(0))
            result->insert(UnprefixedChoiceName(i->second));

    for (PackageUse::const_iterator i(_imp->package_use.begin()), i_end(_imp->package_use.end()) ;
            i != i_end ; ++i)
    {
        if (! match_package(*this, *i->first, *id, MatchPackageOptions()))
            continue;

        if (0 == i->second.compare(0, prefix_lower.length(), prefix_lower, 0, prefix_lower.length()))
            result->insert(UnprefixedChoiceName(i->second.substr(prefix_lower.length())));
    }

    return result;
}

HookResult
PortageEnvironment::perform_hook(const Hook & hook) const
{
    using namespace std::placeholders;

    Lock l(_imp->hook_mutex);
    if (! _imp->hooker)
    {
        _imp->need_hook_dirs();
        _imp->hooker.reset(new Hooker(this));
        std::for_each(_imp->hook_dirs.begin(), _imp->hook_dirs.end(),
                std::bind(std::mem_fn(&Hooker::add_dir), _imp->hooker.get(), _1, false));
    }

    return _imp->hooker->perform_hook(hook);
}

std::shared_ptr<const FSEntrySequence>
PortageEnvironment::hook_dirs() const
{
    Lock l(_imp->hook_mutex);
    _imp->need_hook_dirs();
    std::shared_ptr<FSEntrySequence> result(new FSEntrySequence);
    std::copy(_imp->hook_dirs.begin(), _imp->hook_dirs.end(), result->back_inserter());
    return result;
}

std::shared_ptr<const FSEntrySequence>
PortageEnvironment::bashrc_files() const
{
    std::shared_ptr<FSEntrySequence> result(new FSEntrySequence);
    if (! getenv_with_default("PALUDIS_PORTAGE_BASHRC", "").empty())
        result->push_back(FSEntry(getenv_with_default("PALUDIS_PORTAGE_BASHRC", "")).realpath());
    else
        result->push_back(FSEntry(LIBEXECDIR) / "paludis" / "environments" / "portage" / "bashrc");
    result->push_back(_imp->conf_dir / "make.globals");
    result->push_back(_imp->conf_dir / "make.conf");
    return result;
}

std::shared_ptr<PackageDatabase>
PortageEnvironment::package_database()
{
    return _imp->package_database;
}

std::shared_ptr<const PackageDatabase>
PortageEnvironment::package_database() const
{
    return _imp->package_database;
}

std::shared_ptr<const MirrorsSequence>
PortageEnvironment::mirrors(const std::string & m) const
{
    std::pair<std::multimap<std::string, std::string>::const_iterator, std::multimap<std::string, std::string>::const_iterator>
        p(_imp->mirrors.equal_range(m));
    std::shared_ptr<MirrorsSequence> result(new MirrorsSequence);
    std::transform(p.first, p.second, result->back_inserter(),
            std::mem_fn(&std::pair<const std::string, std::string>::second));
    return result;
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

const std::shared_ptr<const Mask>
PortageEnvironment::mask_for_breakage(const PackageID & id) const
{
    if (! _imp->ignore_all_breaks_portage)
    {
        std::shared_ptr<const Set<std::string> > breakages(id.breaks_portage());
        if (breakages)
        {
            std::set<std::string> bad_breakages;
            std::set_difference(breakages->begin(), breakages->end(),
                    _imp->ignore_breaks_portage.begin(), _imp->ignore_breaks_portage.end(),
                    std::inserter(bad_breakages, bad_breakages.end()));
            if (! bad_breakages.empty())
                return std::make_shared<BreaksPortageMask>(join(breakages->begin(), breakages->end(), " "));
        }
    }

    return std::shared_ptr<const Mask>();
}

const std::shared_ptr<const Mask>
PortageEnvironment::mask_for_user(const PackageID & d, const bool o) const
{
    for (PackageMask::const_iterator i(_imp->package_mask.begin()), i_end(_imp->package_mask.end()) ;
            i != i_end ; ++i)
        if (match_package(*this, **i, d, MatchPackageOptions()))
            return std::make_shared<UserConfigMask>(o);

    return std::shared_ptr<const Mask>();
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

bool
PortageEnvironment::add_to_world(const QualifiedPackageName & q) const
{
    return _add_string_to_world(stringify(q));
}

bool
PortageEnvironment::add_to_world(const SetName & s) const
{
    return _add_string_to_world(stringify(s));
}

bool
PortageEnvironment::remove_from_world(const QualifiedPackageName & q) const
{
    return _remove_string_from_world(stringify(q));
}

bool
PortageEnvironment::remove_from_world(const SetName & s) const
{
    return _remove_string_from_world(stringify(s));
}

bool
PortageEnvironment::_add_string_to_world(const std::string & s) const
{
    Lock l(_imp->world_mutex);

    Context context("When adding '" + s + "' to world file '" + stringify(_imp->world_file) + "':");

    using namespace std::placeholders;

    if (! _imp->world_file.exists())
    {
        try
        {
            SafeOFStream f(_imp->world_file);
        }
        catch (const SafeOFStreamError & e)
        {
            Log::get_instance()->message("portage_environment.world.write_failed", ll_warning, lc_no_context)
                << "Cannot create world file '" << _imp->world_file << "': '" << e.message() << "' (" << e.what() << ")";
            return false;
        }
    }

    SetFile world(make_named_values<SetFileParams>(
                n::environment() = this,
                n::file_name() = _imp->world_file,
                n::parser() = std::bind(&parse_user_package_dep_spec, _1, this, UserPackageDepSpecOptions() + updso_no_disambiguation, filter::All()),
                n::set_operator_mode() = sfsmo_natural,
                n::tag() = std::shared_ptr<DepTag>(),
                n::type() = sft_simple
            ));
    bool result(world.add(s));
    world.rewrite();

    return result;
}

bool
PortageEnvironment::_remove_string_from_world(const std::string & s) const
{
    Lock l(_imp->world_mutex);

    Context context("When removing '" + s + "' from world file '" + stringify(_imp->world_file) + "':");
    bool result(false);

    using namespace std::placeholders;

    if (_imp->world_file.exists())
    {
        SetFile world(make_named_values<SetFileParams>(
                n::environment() = this,
                n::file_name() = _imp->world_file,
                n::parser() = std::bind(&parse_user_package_dep_spec, _1, this,
                        UserPackageDepSpecOptions() + updso_no_disambiguation, filter::All()),
                n::set_operator_mode() = sfsmo_natural,
                n::tag() = std::shared_ptr<DepTag>(),
                n::type() = sft_simple
                ));

        result = world.remove(s);
        world.rewrite();
    }

    return result;
}

void
PortageEnvironment::update_config_files_for_package_move(const PackageDepSpec & s, const QualifiedPackageName & n) const
{
    if (_remove_string_from_world(stringify(s)))
        _add_string_to_world(stringify(PartiallyMadePackageDepSpec(s).package(n)));
}

void
PortageEnvironment::need_keys_added() const
{
}

const std::shared_ptr<const MetadataValueKey<std::string> >
PortageEnvironment::format_key() const
{
    return _imp->format_key;
}

const std::shared_ptr<const MetadataValueKey<FSEntry> >
PortageEnvironment::config_location_key() const
{
    return _imp->config_location_key;
}

const std::shared_ptr<OutputManager>
PortageEnvironment::create_output_manager(const CreateOutputManagerInfo &) const
{
    return std::make_shared<StandardOutputManager>();
}

namespace
{
    std::shared_ptr<const SetSpecTree> make_world_set(
            const Environment * const env,
            const FSEntry & f)
    {
        Context context("When loading world from '" + stringify(f) + "':");

        if (! f.exists())
        {
            Log::get_instance()->message("portage_environment.world.does_not_exist", ll_warning, lc_no_context)
                << "World file '" << f << "' doesn't exist";
            return std::make_shared<SetSpecTree>(std::make_shared<AllDepSpec>());
        }

        const std::shared_ptr<GeneralSetDepTag> tag(new GeneralSetDepTag(SetName("world::environment"), "Environment"));
        SetFile world(make_named_values<SetFileParams>(
                    n::environment() = env,
                    n::file_name() = f,
                    n::parser() = std::bind(&parse_user_package_dep_spec, std::placeholders::_1,
                            env, UserPackageDepSpecOptions() + updso_no_disambiguation, filter::All()),
                    n::set_operator_mode() = sfsmo_natural,
                    n::tag() = tag,
                    n::type() = sft_simple
                    ));
        return world.contents();
    }
}

void
PortageEnvironment::populate_sets() const
{
    Lock l(_imp->world_mutex);
    add_set(SetName("world::environment"), SetName("world"), std::bind(&make_world_set, this, _imp->world_file), true);
}

const std::shared_ptr<Repository>
PortageEnvironment::repository_from_new_config_file(const FSEntry &)
{
    throw InternalError(PALUDIS_HERE, "can't create repositories on the fly for PortageEnvironment");
}

