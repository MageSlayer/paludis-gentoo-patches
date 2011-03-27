/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#include <paludis/repositories/e/exheres_profile.hh>
#include <paludis/repositories/e/e_repository_exceptions.hh>
#include <paludis/repositories/e/e_repository.hh>
#include <paludis/repositories/e/profile_file.hh>
#include <paludis/repositories/e/eapi.hh>

#include <paludis/util/log.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/config_file.hh>
#include <paludis/util/map.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/set.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/system.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/util/fs_stat.hh>

#include <paludis/choice.hh>
#include <paludis/environment.hh>
#include <paludis/match_package.hh>
#include <paludis/distribution.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/paludislike_options_conf.hh>
#include <unordered_map>
#include <list>

using namespace paludis;
using namespace paludis::erepository;

namespace
{
    const std::shared_ptr<const LineConfigFile> make_config_file(
            const FSPath & f,
            const LineConfigFileOptions & o)
    {
        return std::make_shared<LineConfigFile>(f, o);
    }

    typedef std::unordered_map<std::string, std::string, Hash<std::string> > EnvironmentVariablesMap;
    typedef std::unordered_map<QualifiedPackageName,
            std::list<std::pair<std::shared_ptr<const PackageDepSpec>, std::shared_ptr<const MaskInfo> > >,
            Hash<QualifiedPackageName> > PackageMaskMap;
}

namespace paludis
{
    template<>
    struct Imp<ExheresProfile>
    {
        const Environment * const env;
        const bool has_master_repositories;
        const EAPIForFileFunction eapi_for_file;

        std::shared_ptr<FSPathSequence> profiles_with_parents;

        PaludisLikeOptionsConf options_conf;
        EnvironmentVariablesMap environment_variables;

        ProfileFile<LineConfigFile> packages_file;

        const std::shared_ptr<Set<std::string> > use_expand;
        const std::shared_ptr<Set<std::string> > use_expand_hidden;
        const std::shared_ptr<Set<std::string> > use_expand_unprefixed;
        const std::shared_ptr<Set<std::string> > use_expand_implicit;
        const std::shared_ptr<Set<std::string> > iuse_implicit;
        const std::shared_ptr<Set<std::string> > use_expand_values;

        const std::shared_ptr<SetSpecTree> system_packages;

        Imp(
                const Environment * const e,
                const EAPIForFileFunction & f,
                const bool h) :
            env(e),
            has_master_repositories(h),
            eapi_for_file(f),
            profiles_with_parents(std::make_shared<FSPathSequence>()),
            options_conf(make_named_values<PaludisLikeOptionsConfParams>(
                        n::allow_locking() = true,
                        n::environment() = e,
                        n::make_config_file() = &make_config_file
                        )),
            packages_file(eapi_for_file),
            use_expand(std::make_shared<Set<std::string>>()),
            use_expand_hidden(std::make_shared<Set<std::string>>()),
            use_expand_unprefixed(std::make_shared<Set<std::string>>()),
            use_expand_implicit(std::make_shared<Set<std::string>>()),
            iuse_implicit(std::make_shared<Set<std::string>>()),
            use_expand_values(std::make_shared<Set<std::string>>()),
            system_packages(std::make_shared<SetSpecTree>(std::make_shared<AllDepSpec>()))
        {
            environment_variables["CONFIG_PROTECT"] = getenv_with_default("CONFIG_PROTECT", "/etc");
            environment_variables["CONFIG_PROTECT_MASK"] = getenv_with_default("CONFIG_PROTECT_MASK", "");
        }
    };
}

ExheresProfile::ExheresProfile(
        const Environment * const env,
        const RepositoryName &,
        const EAPIForFileFunction & eapi_for_file,
        const IsArchFlagFunction &,
        const FSPathSequence & location,
        const std::string &,
        const bool,
        const bool has_master_repositories,
        const bool) :
    _imp(env, eapi_for_file, has_master_repositories)
{
    for (FSPathSequence::ConstIterator l(location.begin()), l_end(location.end()) ;
            l != l_end ; ++l)
        _load_dir(*l);

    const std::shared_ptr<const Set<UnprefixedChoiceName> > s(_imp->options_conf.known_choice_value_names(
                make_null_shared_ptr(), ChoicePrefixName("suboptions")));
    for (Set<UnprefixedChoiceName>::ConstIterator f(s->begin()), f_end(s->end()) ;
            f != f_end ; ++f)
        if (_imp->options_conf.want_choice_enabled_locked(make_null_shared_ptr(),
                    ChoicePrefixName("suboptions"), *f).first.is_true())
            _imp->use_expand->insert(stringify(*f));

    const std::shared_ptr<const Set<UnprefixedChoiceName> > sh(_imp->options_conf.known_choice_value_names(
                make_null_shared_ptr(), ChoicePrefixName("hidden_suboptions")));
    for (Set<UnprefixedChoiceName>::ConstIterator f(sh->begin()), f_end(sh->end()) ;
            f != f_end ; ++f)
        if (_imp->options_conf.want_choice_enabled_locked(make_null_shared_ptr(),
                    ChoicePrefixName("hidden_suboptions"), *f).first.is_true())
            _imp->use_expand_hidden->insert(stringify(*f));

    if (! _imp->has_master_repositories)
        for (ProfileFile<LineConfigFile>::ConstIterator i(_imp->packages_file.begin()),
                i_end(_imp->packages_file.end()) ; i != i_end ; ++i)
        {
            if (0 != i->second.compare(0, 1, "*", 0, 1))
                continue;

            Context context_spec("When parsing '" + i->second + "':");
            std::shared_ptr<PackageDepSpec> spec(std::make_shared<PackageDepSpec>(
                        parse_elike_package_dep_spec(i->second.substr(1),
                            i->first->supported()->package_dep_spec_parse_options(),
                            i->first->supported()->version_spec_options())));

            _imp->system_packages->top()->append(spec);
        }
}

ExheresProfile::~ExheresProfile()
{
}

void
ExheresProfile::_load_dir(const FSPath & f)
{
    if (! f.stat().is_directory_or_symlink_to_directory())
    {
        Log::get_instance()->message("e.exheres_profile.not_a_directory", ll_warning, lc_context) <<
            "Profile component '" << f << "' is not a directory";
        return;
    }

    if ((f / "parents.conf").stat().exists())
    {
        LineConfigFile file(f / "parents.conf", { });
        for (LineConfigFile::ConstIterator line(file.begin()), line_end(file.end()) ;
                line != line_end ; ++line)
            _load_dir((f / *line).realpath());
    }

    if ((f / "options.conf").stat().exists())
        _imp->options_conf.add_file(f / "options.conf");

    if ((f / "packages").stat().exists())
        _imp->packages_file.add_file(f / "packages");

    if ((f / "make.defaults").stat().exists())
    {
        auto eapi(EAPIData::get_instance()->eapi_from_string(_imp->eapi_for_file(f / "make.defaults")));
        if (! eapi->supported())
            throw ERepositoryConfigurationError("Can't use profile directory '" + stringify(f) +
                    "' because it uses an unsupported EAPI");

        KeyValueConfigFile file(f / "make.defaults", { kvcfo_disallow_source, kvcfo_disallow_space_inside_unquoted_values,
                kvcfo_allow_inline_comments, kvcfo_allow_multiple_assigns_per_line },
                &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation);

        for (KeyValueConfigFile::ConstIterator k(file.begin()), k_end(file.end()) ;
                k != k_end ; ++k)
            _imp->environment_variables[k->first] = k->second;
    }

    _imp->profiles_with_parents->push_back(f);
}

std::shared_ptr<const FSPathSequence>
ExheresProfile::profiles_with_parents() const
{
    return _imp->profiles_with_parents;
}

bool
ExheresProfile::use_masked(
        const std::shared_ptr<const PackageID> & id,
        const std::shared_ptr<const Choice> & choice,
        const UnprefixedChoiceName & value_unprefixed,
        const ChoiceNameWithPrefix &
        ) const
{
    std::pair<Tribool, bool> enabled_locked(_imp->options_conf.want_choice_enabled_locked(
                id, choice->prefix(), value_unprefixed));
    return enabled_locked.first.is_false() && enabled_locked.second;
}

bool
ExheresProfile::use_forced(
        const std::shared_ptr<const PackageID> & id,
        const std::shared_ptr<const Choice> & choice,
        const UnprefixedChoiceName & value_unprefixed,
        const ChoiceNameWithPrefix &
        ) const
{
    std::pair<Tribool, bool> enabled_locked(_imp->options_conf.want_choice_enabled_locked(
                id, choice->prefix(), value_unprefixed));
    return enabled_locked.first.is_true() && enabled_locked.second;
}

Tribool
ExheresProfile::use_state_ignoring_masks(
        const std::shared_ptr<const PackageID> & id,
        const std::shared_ptr<const Choice> & choice,
        const UnprefixedChoiceName & value_unprefixed,
        const ChoiceNameWithPrefix &
        ) const
{
    std::pair<Tribool, bool> enabled_locked(_imp->options_conf.want_choice_enabled_locked(
                id, choice->prefix(), value_unprefixed));
    return enabled_locked.first;
}

const std::shared_ptr<const Set<UnprefixedChoiceName> >
ExheresProfile::known_choice_value_names(
        const std::shared_ptr<const erepository::ERepositoryID> & id,
        const std::shared_ptr<const Choice> & choice
        ) const
{
    return _imp->options_conf.known_choice_value_names(id, choice->prefix());
}

const std::shared_ptr<const Set<std::string> >
ExheresProfile::use_expand() const
{
    return _imp->use_expand;
}

const std::shared_ptr<const Set<std::string> >
ExheresProfile::use_expand_hidden() const
{
    return _imp->use_expand_hidden;
}

const std::shared_ptr<const Set<std::string> >
ExheresProfile::use_expand_unprefixed() const
{
    return _imp->use_expand_unprefixed;
}

const std::shared_ptr<const Set<std::string> >
ExheresProfile::use_expand_implicit() const
{
    return _imp->use_expand_implicit;
}

const std::shared_ptr<const Set<std::string> >
ExheresProfile::iuse_implicit() const
{
    return _imp->iuse_implicit;
}

const std::shared_ptr<const Set<std::string> >
ExheresProfile::use_expand_values(const std::string &) const
{
    return _imp->use_expand_values;
}

const std::string
ExheresProfile::environment_variable(const std::string & s) const
{
    EnvironmentVariablesMap::const_iterator i(_imp->environment_variables.find(s));
    if (_imp->environment_variables.end() == i)
    {
        Log::get_instance()->message("e.exheres_profile.unknown", ll_warning, lc_context) <<
            "Something is asking for environment variable '" << s << "' from profiles, but that isn't in "
            "our list of special vars. This is probably a bug.";
        return "";
    }
    else
        return i->second;
}

const std::shared_ptr<const MasksInfo>
ExheresProfile::profile_masks(const std::shared_ptr<const PackageID> &) const
{
    auto result(std::make_shared<MasksInfo>());
    return result;
}

const std::shared_ptr<const SetSpecTree>
ExheresProfile::system_packages() const
{
    return _imp->system_packages;
}

const std::shared_ptr<const Map<QualifiedPackageName, PackageDepSpec> >
ExheresProfile::virtuals() const
{
    return std::make_shared<Map<QualifiedPackageName, PackageDepSpec>>();
}

