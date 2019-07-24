/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2013 Ciaran McCreesh
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

#include <paludis/repositories/e/traditional_profile.hh>
#include <paludis/repositories/e/traditional_profile_file.hh>
#include <paludis/repositories/e/traditional_mask_file.hh>
#include <paludis/repositories/e/e_repository_exceptions.hh>
#include <paludis/repositories/e/e_repository.hh>
#include <paludis/repositories/e/eapi.hh>

#include <paludis/util/log.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/save.hh>
#include <paludis/util/system.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/join.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/set.hh>
#include <paludis/util/options.hh>
#include <paludis/util/iterator_funcs.hh>
#include <paludis/util/create_iterator-impl.hh>
#include <paludis/util/config_file.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/map.hh>
#include <paludis/util/fs_stat.hh>
#include <paludis/util/fs_error.hh>
#include <paludis/util/upper_lower.hh>

#include <paludis/choice.hh>
#include <paludis/environment.hh>
#include <paludis/match_package.hh>
#include <paludis/distribution.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>

#include <unordered_map>
#include <unordered_set>
#include <list>
#include <algorithm>
#include <set>
#include <map>
#include <vector>
#include <mutex>

#include <strings.h>

using namespace paludis;
using namespace paludis::erepository;

typedef std::unordered_map<std::string, std::shared_ptr<Set<UnprefixedChoiceName> > > KnownMap;

namespace
{
    typedef std::unordered_map<std::string, std::string, Hash<std::string> > EnvironmentVariablesMap;
    typedef std::unordered_map<QualifiedPackageName,
            std::list<std::pair<std::shared_ptr<const PackageDepSpec>, std::shared_ptr<const MaskInfo> > >,
            Hash<QualifiedPackageName> > PackageMaskMap;

    typedef std::unordered_map<ChoiceNameWithPrefix, bool, Hash<ChoiceNameWithPrefix> > FlagStatusMap;
    typedef std::list<std::pair<std::shared_ptr<const PackageDepSpec>, FlagStatusMap> > PackageFlagStatusMapList;

    typedef std::pair<ChoicePrefixName, UnprefixedChoiceName> FlagId;
    typedef std::map<FlagId, bool> FlagIdStatusMap;  // "bool" to support negative flags...

    struct StackedValues
    {
        std::string origin;

        FlagStatusMap use_mask;
        FlagStatusMap use_stable_mask;
        FlagStatusMap use_force;
        FlagStatusMap use_stable_force;
        PackageFlagStatusMapList package_use;
        PackageFlagStatusMapList package_use_mask;
        PackageFlagStatusMapList package_use_stable_mask;
        PackageFlagStatusMapList package_use_force;
        PackageFlagStatusMapList package_use_stable_force;

        StackedValues(const std::string & o) :
            origin(o)
        {
        }
    };

    typedef std::list<StackedValues> StackedValuesList;
}

namespace paludis
{
    template <>
    struct Imp<TraditionalProfile>
    {
        const Environment * const env;
        const EAPIForFileFunction eapi_for_file;
        const IsArchFlagFunction is_arch_flag;
        const bool has_master_repositories;

        TraditionalProfileFile<LineConfigFile> packages_file;
        TraditionalProfileFile<TraditionalMaskFile> package_mask_file;

        std::shared_ptr<FSPathSequence> profiles_with_parents;

        EnvironmentVariablesMap environment_variables;

        std::shared_ptr<SetSpecTree> system_packages;

        FlagIdStatusMap use;
        std::shared_ptr<Set<std::string> > use_expand;
        std::shared_ptr<Set<std::string> > use_expand_hidden;
        std::shared_ptr<Set<std::string> > use_expand_unprefixed;
        std::shared_ptr<Set<std::string> > use_expand_implicit;
        std::shared_ptr<Set<std::string> > iuse_implicit;
        std::unordered_map<std::string, std::shared_ptr<Set<std::string> > > use_expand_values;
        KnownMap known_choice_value_names;
        mutable std::mutex known_choice_value_names_for_separator_mutex;
        mutable std::unordered_map<char, KnownMap> known_choice_value_names_for_separator;
        StackedValuesList stacked_values_list;

        PackageMaskMap package_mask;

        Imp(const Environment * const e,
                const EAPIForFileFunction & p,
                const IsArchFlagFunction & a,
                const bool h) :
            env(e),
            eapi_for_file(p),
            is_arch_flag(a),
            has_master_repositories(h),
            packages_file(eapi_for_file),
            package_mask_file(eapi_for_file),
            profiles_with_parents(std::make_shared<FSPathSequence>()),
            system_packages(std::make_shared<SetSpecTree>(std::make_shared<AllDepSpec>())),
            use_expand(std::make_shared<Set<std::string>>()),
            use_expand_hidden(std::make_shared<Set<std::string>>()),
            use_expand_unprefixed(std::make_shared<Set<std::string>>()),
            use_expand_implicit(std::make_shared<Set<std::string>>()),
            iuse_implicit(std::make_shared<Set<std::string>>())
        {
        }
    };
}

namespace
{
    void load_environment(Pimp<TraditionalProfile> & _imp);

    void load_profile_directory_recursively(
            Pimp<TraditionalProfile> & _imp,
            const FSPath & dir);

    void load_profile_parent(
            Pimp<TraditionalProfile> & _imp,
            const FSPath & dir);

    void make_vars_from_file_vars(
            Pimp<TraditionalProfile> & _imp);

    void load_special_make_defaults_vars(
            Pimp<TraditionalProfile> & _imp,
            std::shared_ptr<const paludis::erepository::EAPI> eapi);

    void add_use_expand_to_use(
            Pimp<TraditionalProfile> & _imp,
            std::shared_ptr<const paludis::erepository::EAPI> eapi);

    void fish_out_use_expand_names(
            Pimp<TraditionalProfile> & _imp);

    void load_profile_make_defaults(
            Pimp<TraditionalProfile> & _imp,
            const FSPath & dir);

    void load_basic_use_file(
            const FSPath & file,
            FlagStatusMap & m);

    void load_spec_use_file(
            const EAPI & eapi,
            const FSPath & file,
            PackageFlagStatusMapList & m);
}

namespace
{
    void insert_use_flag_from_str(bool profile_negative_use, FlagIdStatusMap &m, const std::string& prefix_name, const std::string& str) {
        // store positive / negative use flag into map

        if (str.empty()) {
            // do not expect empty strings here...
            throw InternalError(PALUDIS_HERE, "Use flag is empty");
        }

        bool positive_flag = ('-' != str.at(0));
        if (!profile_negative_use && !positive_flag) {
            throw ERepositoryConfigurationError("Repository does not support negative use flags");
        }

        auto name = positive_flag ? str : str.substr(1);
        auto id = std::make_pair( ChoicePrefixName(prefix_name), UnprefixedChoiceName(name) );
        m.insert( std::make_pair(id, positive_flag) );
    }

    void load_environment(Pimp<TraditionalProfile> & _imp)
    {
        _imp->environment_variables["CONFIG_PROTECT"] = getenv_with_default("CONFIG_PROTECT", "/etc");
        _imp->environment_variables["CONFIG_PROTECT_MASK"] = getenv_with_default("CONFIG_PROTECT_MASK", "");
    }

    void load_profile_directory_recursively(
            Pimp<TraditionalProfile> & _imp,
            const FSPath & dir)
    {
        Context context("When adding profile directory '" + stringify(dir) + ":");

        if (! dir.stat().is_directory_or_symlink_to_directory())
        {
            Log::get_instance()->message("e.profile.not_a_directory", ll_warning, lc_context)
                << "Profile component '" << dir << "' is not a directory";
            return;
        }

        auto eapi(EAPIData::get_instance()->eapi_from_string(_imp->eapi_for_file(dir / "use.mask")));
        if (! eapi->supported())
            throw ERepositoryConfigurationError("Can't use profile directory '" + stringify(dir) +
                    "' because it uses an unsupported EAPI");

        load_profile_parent(_imp, dir);
        load_profile_make_defaults(_imp, dir);

        _imp->stacked_values_list.push_back(StackedValues(stringify(dir)));
        load_basic_use_file(dir / "use.mask", _imp->stacked_values_list.back().use_mask);
        load_basic_use_file(dir / "use.force", _imp->stacked_values_list.back().use_force);
        load_spec_use_file(*eapi, dir / "package.use", _imp->stacked_values_list.back().package_use);
        load_spec_use_file(*eapi, dir / "package.use.mask", _imp->stacked_values_list.back().package_use_mask);
        load_spec_use_file(*eapi, dir / "package.use.force", _imp->stacked_values_list.back().package_use_force);
        if (eapi->supported()->profile_options()->use_stable_mask_force())
        {
            load_basic_use_file(dir / "use.stable.mask", _imp->stacked_values_list.back().use_stable_mask);
            load_basic_use_file(dir / "use.stable.force", _imp->stacked_values_list.back().use_stable_force);
            load_spec_use_file(*eapi, dir / "package.use.stable.mask", _imp->stacked_values_list.back().package_use_stable_mask);
            load_spec_use_file(*eapi, dir / "package.use.stable.force", _imp->stacked_values_list.back().package_use_stable_force);
        }

        _imp->packages_file.add_file(dir / "packages");
        _imp->package_mask_file.add_file(dir / "package.mask");

        _imp->profiles_with_parents->push_back(dir);
    }

    void load_profile_parent(
            Pimp<TraditionalProfile> & _imp,
            const FSPath & dir)
    {
        Context context("When handling parent file for profile directory '" + stringify(dir) + ":");

        if (! (dir / "parent").stat().exists())
            return;

        LineConfigFile file(dir / "parent", { lcfo_disallow_continuations });

        LineConfigFile::ConstIterator i(file.begin());
        LineConfigFile::ConstIterator i_end(file.end());
        bool once(false);
        if (i == i_end)
            Log::get_instance()->message("e.profile.parent.empty", ll_warning, lc_context) << "parent file is empty";
        else
            for ( ; i != i_end ; ++i)
            {
                if ('#' == i->at(0))
                {
                    if (! once)
                        Log::get_instance()->message("e.profile.parent.no_comments", ll_qa, lc_context)
                            << "Comments not allowed in '" << (dir / "parent") << "'";
                    once = true;
                    continue;
                }

                FSPath parent_dir(dir);
                do
                {
                    try
                    {
                        parent_dir = (parent_dir / *i).realpath();
                    }
                    catch (const FSError & e)
                    {
                        Log::get_instance()->message("e.profile.parent.skipping", ll_warning, lc_context)
                            << "Skipping parent '" << *i << "' due to exception: " << e.message() << " (" << e.what() << ")";
                        continue;
                    }

                    load_profile_directory_recursively(_imp, parent_dir);

                } while (false);
            }
    }

    void make_vars_from_file_vars(
            Pimp<TraditionalProfile> & _imp)
    {
        try
        {
            if (! _imp->has_master_repositories)
                for (const auto & i : _imp->packages_file)
                {
                    if (0 != i.second.compare(0, 1, "*", 0, 1))
                        continue;

                    Context context_spec("When parsing '" + i.second + "':");
                    std::shared_ptr<PackageDepSpec> spec(std::make_shared<PackageDepSpec>(
                                parse_elike_package_dep_spec(i.second.substr(1),
                                    i.first->supported()->package_dep_spec_parse_options(),
                                    i.first->supported()->version_spec_options())));

                    _imp->system_packages->top()->append(spec);
                }
        }
        catch (const InternalError &)
        {
            throw;
        }
        catch (const Exception & e)
        {
            Log::get_instance()->message("e.profile.packages.failure", ll_warning, lc_context) << "Loading packages "
                    " failed due to exception: " << e.message() << " (" << e.what() << ")";
        }

        for (const auto & line : _imp->package_mask_file)
        {
            if (line.second.first.empty())
                continue;

            try
            {
                std::shared_ptr<const PackageDepSpec> a(std::make_shared<PackageDepSpec>(
                            parse_elike_package_dep_spec(line.second.first,
                                line.first->supported()->package_dep_spec_parse_options(),
                                line.first->supported()->version_spec_options())));

                if (a->package_ptr())
                    _imp->package_mask[*a->package_ptr()].push_back(std::make_pair(a, line.second.second));
                else
                    Log::get_instance()->message("e.profile.package_mask.bad_spec", ll_warning, lc_context)
                        << "Loading package.mask spec '" << line.second.first << "' failed because specification does not restrict to a "
                        "unique package";
            }
            catch (const InternalError &)
            {
                throw;
            }
            catch (const Exception & e)
            {
                Log::get_instance()->message("e.profile.package_mask.bad_spec", ll_warning, lc_context)
                    << "Loading package.mask spec '" << line.second.first << "' failed due to exception '" << e.message() << "' ("
                    << e.what() << ")";
            }
        }
    }

    void load_special_make_defaults_vars(
            Pimp<TraditionalProfile> & _imp,
            std::shared_ptr<const paludis::erepository::EAPI> eapi)
    {
        bool profile_negative_use = eapi->supported()->choices_options()->profile_negative_use();
        std::string use_var(eapi->supported()->ebuild_environment_variables()->env_use());
        try
        {
            _imp->use.clear();
            if (! use_var.empty())
            {
                std::list<std::string> tokens;
                tokenise_whitespace(_imp->environment_variables[use_var], std::back_inserter(tokens));
                for (const auto & token : tokens)
                    insert_use_flag_from_str( profile_negative_use, _imp->use, "", token );
            }
        }
        catch (const InternalError &)
        {
            throw;
        }
        catch (const Exception & e)
        {
            Log::get_instance()->message("e.profile.make_defaults.use_failure", ll_warning, lc_context)
                << "Loading '" << use_var << "' failed due to exception: " << e.message() << " (" << e.what() << ")";
        }

        std::string use_expand_var(eapi->supported()->ebuild_environment_variables()->env_use_expand());
        try
        {
            _imp->use_expand->clear();
            if (! use_expand_var.empty())
                tokenise_whitespace(_imp->environment_variables[use_expand_var], _imp->use_expand->inserter());
        }
        catch (const InternalError &)
        {
            throw;
        }
        catch (const Exception & e)
        {
            Log::get_instance()->message("e.profile.make_defaults.use_expand_failure", ll_warning, lc_context)
                << "Loading '" << use_expand_var << "' failed due to exception: " << e.message() << " (" << e.what() << ")";
        }

        std::string use_expand_hidden_var(eapi->supported()->ebuild_environment_variables()->env_use_expand_hidden());
        try
        {
            _imp->use_expand_hidden->clear();
            if (! use_expand_hidden_var.empty())
                tokenise_whitespace(_imp->environment_variables[use_expand_hidden_var], _imp->use_expand_hidden->inserter());
        }
        catch (const InternalError &)
        {
            throw;
        }
        catch (const Exception & e)
        {
            Log::get_instance()->message("e.profile.make_defaults.use_expand_hidden_failure", ll_warning, lc_context)
                << "Loading '" << use_expand_hidden_var << "' failed due to exception: "
                << e.message() << " (" << e.what() << ")";
        }
    }

    void add_use_expand_to_use( Pimp<TraditionalProfile> & _imp, std::shared_ptr<const paludis::erepository::EAPI> eapi)
    {
        Context context("When adding USE_EXPAND to USE:");

        bool profile_negative_use = eapi->supported()->choices_options()->profile_negative_use();
        _imp->stacked_values_list.push_back(StackedValues("use_expand special values"));

        for (const auto & x : *_imp->use_expand)
        {
            std::string lower_x(tolower(x));
            std::list<std::string> uses;
            tokenise_whitespace(_imp->environment_variables[stringify(x)], std::back_inserter(uses));
            for (const auto & use : uses)
                insert_use_flag_from_str( profile_negative_use, _imp->use, lower_x, use );
        }
    }

    void fish_out_use_expand_names(
            Pimp<TraditionalProfile> & _imp)
    {
        Context context("When finding all known USE_EXPAND names:");

        for (const auto & x : *_imp->use_expand)
            _imp->known_choice_value_names.insert(std::make_pair(tolower(x), std::make_shared<Set<UnprefixedChoiceName>>()));

        for (const auto & u : _imp->use)
        {
            if (! stringify(u.first.first).empty())
            {
                KnownMap::iterator i(_imp->known_choice_value_names.find(stringify(u.first.first)));
                if (i == _imp->known_choice_value_names.end())
                    throw InternalError(PALUDIS_HERE, stringify(u.first.first));
                i->second->insert(u.first.second);
            }
        }
    }

    void handle_profile_arch_var(
            Pimp<TraditionalProfile> & _imp,
            const std::string & s)
    {
        Context context("When handling profile " + s + " variable:");

        std::string arch_s(_imp->environment_variables[s]);
        if (arch_s.empty())
            throw ERepositoryConfigurationError("Variable '" + s + "' is unset or empty");

        _imp->stacked_values_list.push_back(StackedValues("arch special values"));
        try
        {
            std::string arch(arch_s);

            _imp->use.insert(std::make_pair(std::make_pair(ChoicePrefixName(""), UnprefixedChoiceName(arch)), true));
            _imp->stacked_values_list.back().use_force[ChoiceNameWithPrefix(arch)] = true;
        }
        catch (const InternalError &)
        {
            throw;
        }
        catch (const Exception &)
        {
            throw ERepositoryConfigurationError("Variable '" + s + "' has invalid value '" + arch_s + "'");
        }
    }

    bool is_incremental(const EAPI & e, const std::string & s)
    {
        Context c("When checking whether '" + s + "' is incremental:");

        return (! s.empty()) && (
                (s == e.supported()->ebuild_environment_variables()->env_use())
                || (s == e.supported()->ebuild_environment_variables()->env_use_expand())
                || (s == e.supported()->ebuild_environment_variables()->env_use_expand_hidden())
                || (s == e.supported()->ebuild_environment_variables()->env_use_expand_unprefixed())
                || (s == e.supported()->ebuild_environment_variables()->env_use_expand_implicit())
                || (s == e.supported()->ebuild_environment_variables()->env_iuse_implicit())
                || s == "CONFIG_PROTECT"
                || s == "CONFIG_PROTECT_MASK");
    }

    void load_profile_make_defaults(
            Pimp<TraditionalProfile> & _imp,
            const FSPath & dir)
    {
        Context context("When handling make.defaults file for profile directory '" + stringify(dir) + ":");

        if (! (dir / "make.defaults").stat().exists())
            return;

        auto eapi(EAPIData::get_instance()->eapi_from_string(_imp->eapi_for_file(dir / "make.defaults")));
        if (! eapi->supported())
            throw ERepositoryConfigurationError("Can't use profile directory '" + stringify(dir) +
                    "' because it uses an unsupported EAPI");

        KeyValueConfigFile file(dir / "make.defaults", { kvcfo_disallow_source, kvcfo_disallow_space_inside_unquoted_values, kvcfo_allow_inline_comments, kvcfo_allow_multiple_assigns_per_line },
                &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation);

        for (const auto & kv : file)
        {
            if (is_incremental(*eapi, kv.first))
            {
                std::list<std::string> val;
                std::list<std::string> val_add;
                tokenise_whitespace(_imp->environment_variables[kv.first], std::back_inserter(val));
                tokenise_whitespace(kv.second, std::back_inserter(val_add));

                for (const auto & v : val_add)
                {
                    if (v.empty())
                        continue;
                    if (v == "-*")
                        val.clear();
                    else if ('-' == v.at(0))
                        val.remove(v.substr(1));
                    else
                        val.push_back(v);
                }

                _imp->environment_variables[kv.first] = join(val.begin(), val.end(), " ");
            }
            else
                _imp->environment_variables[kv.first] = kv.second;
        }

        std::string use_expand_var(eapi->supported()->ebuild_environment_variables()->env_use_expand());
        try
        {
            _imp->use_expand->clear();
            if (! use_expand_var.empty())
                tokenise_whitespace(_imp->environment_variables[use_expand_var], _imp->use_expand->inserter());
        }
        catch (const InternalError &)
        {
            throw;
        }
        catch (const Exception & e)
        {
            Log::get_instance()->message("e.profile.make_defaults.use_expand_failure", ll_warning, lc_context)
                << "Loading '" << use_expand_var << "' failed due to exception: " << e.message() << " (" << e.what() << ")";
        }

        std::string use_expand_unprefixed_var(eapi->supported()->ebuild_environment_variables()->env_use_expand_unprefixed());
        try
        {
            _imp->use_expand_unprefixed->clear();
            if (! use_expand_unprefixed_var.empty())
                tokenise_whitespace(_imp->environment_variables[use_expand_unprefixed_var], _imp->use_expand_unprefixed->inserter());
        }
        catch (const InternalError &)
        {
            throw;
        }
        catch (const Exception & e)
        {
            Log::get_instance()->message("e.profile.make_defaults.use_expand_unprefixed_failure", ll_warning, lc_context)
                << "Loading '" << use_expand_unprefixed_var << "' failed due to exception: " << e.message() << " (" << e.what() << ")";
        }

        std::string use_expand_implicit_var(eapi->supported()->ebuild_environment_variables()->env_use_expand_implicit());
        try
        {
            _imp->use_expand_implicit->clear();
            if (! use_expand_implicit_var.empty())
                tokenise_whitespace(_imp->environment_variables[use_expand_implicit_var], _imp->use_expand_implicit->inserter());
        }
        catch (const InternalError &)
        {
            throw;
        }
        catch (const Exception & e)
        {
            Log::get_instance()->message("e.profile.make_defaults.use_expand_implicit_failure", ll_warning, lc_context)
                << "Loading '" << use_expand_implicit_var << "' failed due to exception: " << e.message() << " (" << e.what() << ")";
        }

        std::string iuse_implicit_var(eapi->supported()->ebuild_environment_variables()->env_iuse_implicit());
        try
        {
            _imp->iuse_implicit->clear();
            if (! iuse_implicit_var.empty())
                tokenise_whitespace(_imp->environment_variables[iuse_implicit_var], _imp->iuse_implicit->inserter());
        }
        catch (const InternalError &)
        {
            throw;
        }
        catch (const Exception & e)
        {
            Log::get_instance()->message("e.profile.make_defaults.iuse_implicit_failure", ll_warning, lc_context)
                << "Loading '" << iuse_implicit_var << "' failed due to exception: " << e.message() << " (" << e.what() << ")";
        }

        std::string use_expand_values_part_var(eapi->supported()->ebuild_environment_variables()->env_use_expand_values_part());
        try
        {
            _imp->use_expand_values.clear();
            if (! use_expand_values_part_var.empty())
            {
                for (const auto & x : *_imp->use_expand)
                {
                    std::shared_ptr<Set<std::string> > v(std::make_shared<Set<std::string>>());
                    tokenise_whitespace(_imp->environment_variables[use_expand_values_part_var + x], v->inserter());
                    _imp->use_expand_values.insert(std::make_pair(x, v));
                }

                for (const auto & x : *_imp->use_expand_unprefixed)
                {
                    std::shared_ptr<Set<std::string> > v(std::make_shared<Set<std::string>>());
                    tokenise_whitespace(_imp->environment_variables[use_expand_values_part_var + x], v->inserter());
                    _imp->use_expand_values.insert(std::make_pair(x, v));
                }
            }
        }
        catch (const InternalError &)
        {
            throw;
        }
        catch (const Exception & e)
        {
            Log::get_instance()->message("e.profile.make_defaults.iuse_implicit_failure", ll_warning, lc_context)
                << "Loading '" << iuse_implicit_var << "' failed due to exception: " << e.message() << " (" << e.what() << ")";
        }
    }

    void load_basic_use_file(
            const FSPath & file,
            FlagStatusMap & m)
    {
        if (! file.stat().exists())
            return;

        Context context("When loading basic use file '" + stringify(file) + ":");
        LineConfigFile f(file, { lcfo_disallow_continuations });
        for (const auto & line : f)
        {
            std::list<std::string> tokens;
            tokenise_whitespace(line, std::back_inserter(tokens));

            for (const auto & token : tokens)
            {
                try
                {
                    if (token.empty())
                        continue;
                    if ('-' == token.at(0))
                        m[ChoiceNameWithPrefix(token.substr(1))] = false;
                    else
                        m[ChoiceNameWithPrefix(token)] = true;
                }
                catch (const InternalError &)
                {
                    throw;
                }
                catch (const Exception & e)
                {
                    Log::get_instance()->message("e.profile.failure", ll_warning, lc_context) << "Ignoring token '"
                        << token << "' due to exception '" << e.message() << "' (" << e.what() << ")";
                }
            }
        }
    }

    void load_spec_use_file(
            const EAPI & eapi,
            const FSPath & file,
            PackageFlagStatusMapList & m)
    {
        if (! file.stat().exists())
            return;

        Context context("When loading specised use file '" + stringify(file) + ":");
        LineConfigFile f(file, { lcfo_disallow_continuations });
        for (const auto & line : f)
        {
            std::list<std::string> tokens;
            tokenise_whitespace(line, std::back_inserter(tokens));

            if (tokens.empty())
                continue;

            try
            {
                std::shared_ptr<const PackageDepSpec> spec(std::make_shared<PackageDepSpec>(
                            parse_elike_package_dep_spec(*tokens.begin(), eapi.supported()->package_dep_spec_parse_options(),
                                eapi.supported()->version_spec_options())));
                PackageFlagStatusMapList::iterator n(m.insert(m.end(), std::make_pair(spec, FlagStatusMap())));

                for (std::list<std::string>::const_iterator t(next(tokens.begin())), t_end(tokens.end()) ;
                        t != t_end ; ++t)
                {
                    try
                    {
                        if (t->empty())
                            continue;
                        if ('-' == t->at(0))
                            n->second[ChoiceNameWithPrefix(t->substr(1))] = false;
                        else
                            n->second[ChoiceNameWithPrefix(*t)] = true;
                    }
                    catch (const InternalError &)
                    {
                        throw;
                    }
                    catch (const Exception & e)
                    {
                        Log::get_instance()->message("e.profile.failure", ll_warning, lc_context) << "Ignoring token '"
                            << *t << "' due to exception '" << e.message() << "' (" << e.what() << ")";
                    }
                }
            }
            catch (const PackageDepSpecError & e)
            {
                Log::get_instance()->message("e.profile.failure", ll_warning, lc_context) << "Ignoring line '"
                    << line << "' due to exception '" << e.message() << "' (" << e.what() << ")";
            }
        }
    }
}

TraditionalProfile::TraditionalProfile(
        const Environment * const env,
        const RepositoryName & name,
        const EAPIForFileFunction & eapi_for_file,
        const IsArchFlagFunction & is_arch_flag,
        const FSPathSequence & dirs,
        const std::string & arch_var_if_special,
        const bool profiles_explicitly_set,
        const bool has_master_repositories,
        const bool ignore_deprecated_profiles) :
    _imp(env, eapi_for_file, is_arch_flag, has_master_repositories)
{
    Context context("When loading profiles '" + join(dirs.begin(), dirs.end(), "' '") + "' for repository '" + stringify(name) + "':");

    if (dirs.empty())
        throw ERepositoryConfigurationError("No profiles directories specified");

    load_environment(_imp);

    for (const auto & dir : dirs)
    {
        Context subcontext("When using directory '" + stringify(dir) + "':");

        if (profiles_explicitly_set && ! ignore_deprecated_profiles)
            if ((dir / "deprecated").stat().is_regular_file_or_symlink_to_regular_file())
                Log::get_instance()->message("e.profile.deprecated", ll_warning, lc_context) << "Profile directory '" << dir
                    << "' is deprecated. See the file '" << (dir / "deprecated") << "' for details";

        load_profile_directory_recursively(_imp, dir);
    }

    make_vars_from_file_vars(_imp);

    auto dir = *dirs.begin();
    auto eapi(EAPIData::get_instance()->eapi_from_string(_imp->eapi_for_file(dir / "make.defaults")));
    if (! eapi->supported()) {
        throw ERepositoryConfigurationError("Can't use profile directory '" + stringify(dir) +
                                            "' because it uses an unsupported EAPI");
    }

    load_special_make_defaults_vars(_imp, eapi);
    add_use_expand_to_use(_imp, eapi);
    fish_out_use_expand_names(_imp);
    if (! arch_var_if_special.empty())
        handle_profile_arch_var(_imp, arch_var_if_special);
}

TraditionalProfile::~TraditionalProfile() = default;

std::shared_ptr<const FSPathSequence>
TraditionalProfile::profiles_with_parents() const
{
    return _imp->profiles_with_parents;
}

bool
TraditionalProfile::use_masked(
        const std::shared_ptr<const EbuildID> & id,
        const std::shared_ptr<const Choice> & choice,
        const UnprefixedChoiceName & value_unprefixed,
        const ChoiceNameWithPrefix & value_prefixed
        ) const
{
    if (stringify(choice->prefix()).empty() && _imp->is_arch_flag(value_unprefixed) &&
            (! use_state_ignoring_masks(id, choice, value_unprefixed, value_prefixed).is_true()))
        return true;

    bool result(false);
    for (const auto & i : _imp->stacked_values_list)
    {
        FlagStatusMap::const_iterator f(i.use_mask.find(value_prefixed));
        if (i.use_mask.end() != f)
            result = f->second;

        if (id->is_stable())
        {
            FlagStatusMap::const_iterator fs(i.use_stable_mask.find(value_prefixed));
            if (i.use_stable_mask.end() != fs)
                result = fs->second;
        }

        for (const auto & g : i.package_use_mask)
        {
            if (! match_package(*_imp->env, *g.first, id, nullptr, { }))
                continue;

            FlagStatusMap::const_iterator h(g.second.find(value_prefixed));
            if (g.second.end() != h)
                result = h->second;
        }

        if (id->is_stable())
        {
            for (const auto & gs : i.package_use_stable_mask)
            {
                if (! match_package(*_imp->env, *gs.first, id, nullptr, { }))
                    continue;

                FlagStatusMap::const_iterator hs(gs.second.find(value_prefixed));
                if (gs.second.end() != hs)
                    result = hs->second;
            }
        }
    }

    return result;
}

bool
TraditionalProfile::use_forced(
        const std::shared_ptr<const EbuildID> & id,
        const std::shared_ptr<const Choice> & choice,
        const UnprefixedChoiceName & value_unprefixed,
        const ChoiceNameWithPrefix & value_prefixed
        ) const
{
    if (use_masked(id, choice, value_unprefixed, value_prefixed))
        return false;
    if (stringify(choice->prefix()).empty() && _imp->is_arch_flag(value_unprefixed))
        return true;

    bool result(false);
    for (const auto & i : _imp->stacked_values_list)
    {
        FlagStatusMap::const_iterator f(i.use_force.find(value_prefixed));
        if (i.use_force.end() != f)
            result = f->second;

        if (id->is_stable())
        {
            FlagStatusMap::const_iterator fs(i.use_stable_force.find(value_prefixed));
            if (i.use_stable_force.end() != fs)
                result = fs->second;
        }

        for (const auto & g : i.package_use_force)
        {
            if (! match_package(*_imp->env, *g.first, id, nullptr, { }))
                continue;

            FlagStatusMap::const_iterator h(g.second.find(value_prefixed));
            if (g.second.end() != h)
                result = h->second;
        }

        if (id->is_stable())
        {
            for (const auto & gs : i.package_use_stable_force)
            {
                if (! match_package(*_imp->env, *gs.first, id, nullptr, { }))
                    continue;

                FlagStatusMap::const_iterator hs(gs.second.find(value_prefixed));
                if (gs.second.end() != hs)
                    result = hs->second;
            }
        }
    }

    return result;
}

Tribool
TraditionalProfile::use_state_ignoring_masks(
        const std::shared_ptr<const PackageID> & id,
        const std::shared_ptr<const Choice> & choice,
        const UnprefixedChoiceName & value_unprefixed,
        const ChoiceNameWithPrefix & value_prefixed
        ) const
{
    std::pair<ChoicePrefixName, UnprefixedChoiceName> prefix_value(choice->prefix(), value_unprefixed);
    auto u = _imp->use.find(prefix_value);
    Tribool result(_imp->use.end() != u ? Tribool(u->second) : Tribool(indeterminate));

    for (const auto & i : _imp->stacked_values_list)
    {
        for (const auto & g : i.package_use)
        {
            if (! match_package(*_imp->env, *g.first, id, nullptr, { }))
                continue;

            FlagStatusMap::const_iterator h(g.second.find(value_prefixed));
            if (g.second.end() != h)
                result = h->second ? true : false;
        }
    }

    return result;
}

namespace
{
    void
    add_flag_status_map(const std::shared_ptr<Set<UnprefixedChoiceName> > result,
            const FlagStatusMap & m, const std::string & prefix)
    {
        for (const auto & it : m)
            if (0 == stringify(it.first).compare(0, prefix.length(), prefix))
                result->insert(UnprefixedChoiceName(stringify(it.first).substr(prefix.length())));
    }

    void
    add_package_flag_status_map_list(const std::shared_ptr<Set<UnprefixedChoiceName> > result,
            const PackageFlagStatusMapList & m, const std::string & prefix)
    {
        for (const auto & it : m)
            add_flag_status_map(result, it.second, prefix);
    }
}

const std::shared_ptr<const Set<UnprefixedChoiceName> >
TraditionalProfile::known_choice_value_names(
        const std::shared_ptr<const ERepositoryID> & id,
        const std::shared_ptr<const Choice> & choice
        ) const
{
    std::unique_lock<std::mutex> l(_imp->known_choice_value_names_for_separator_mutex);

    char separator(id->eapi()->supported()->choices_options()->use_expand_separator());
    std::unordered_map<char, KnownMap>::iterator it(_imp->known_choice_value_names_for_separator.find(separator));
    if (_imp->known_choice_value_names_for_separator.end() == it)
        it = _imp->known_choice_value_names_for_separator.insert(std::make_pair(separator, KnownMap())).first;

    std::string lower_x(tolower(choice->raw_name()));
    KnownMap::const_iterator it2(it->second.find(lower_x));
    if (it->second.end() == it2)
    {
        std::shared_ptr<Set<UnprefixedChoiceName> > result(std::make_shared<Set<UnprefixedChoiceName>>());
        it2 = it->second.insert(std::make_pair(lower_x, result)).first;

        KnownMap::const_iterator i(_imp->known_choice_value_names.find(lower_x));
        if (_imp->known_choice_value_names.end() == i)
            throw InternalError(PALUDIS_HERE, lower_x);
        std::copy(i->second->begin(), i->second->end(), result->inserter());

        std::string prefix(lower_x);
        prefix += separator;

        for (const auto & sit : _imp->stacked_values_list)
        {
            add_flag_status_map(result, sit.use_mask, prefix);
            add_flag_status_map(result, sit.use_force, prefix);
            add_package_flag_status_map_list(result, sit.package_use, prefix);
            add_package_flag_status_map_list(result, sit.package_use_mask, prefix);
            add_package_flag_status_map_list(result, sit.package_use_force, prefix);
        }
    }

    return it2->second;
}

const std::string
TraditionalProfile::environment_variable(const std::string & s) const
{
    EnvironmentVariablesMap::const_iterator i(_imp->environment_variables.find(s));
    if (_imp->environment_variables.end() == i)
        return "";
    else
        return i->second;
}

const std::shared_ptr<const SetSpecTree>
TraditionalProfile::system_packages() const
{
    return _imp->system_packages;
}

const std::shared_ptr<const MasksInfo>
TraditionalProfile::profile_masks(const std::shared_ptr<const PackageID> & id) const
{
    auto result(std::make_shared<MasksInfo>());
    PackageMaskMap::const_iterator rr(_imp->package_mask.find(id->name()));
    if (_imp->package_mask.end() != rr)
    {
        for (const auto & k : rr->second)
            if (match_package(*_imp->env, *k.first, id, nullptr, { }))
                result->push_back(*k.second);
    }

    return result;
}

const std::shared_ptr<const Set<std::string> >
TraditionalProfile::use_expand() const
{
    return _imp->use_expand;
}

const std::shared_ptr<const Set<std::string> >
TraditionalProfile::use_expand_hidden() const
{
    return _imp->use_expand_hidden;
}

const std::shared_ptr<const Set<std::string>>
TraditionalProfile::use_expand_no_describe() const
{
    return nullptr;
}

const std::shared_ptr<const Set<std::string> >
TraditionalProfile::use_expand_unprefixed() const
{
    return _imp->use_expand_unprefixed;
}

const std::shared_ptr<const Set<std::string> >
TraditionalProfile::use_expand_implicit() const
{
    return _imp->use_expand_implicit;
}

const std::shared_ptr<const Set<std::string> >
TraditionalProfile::use_expand_values(const std::string & x) const
{
    Context context("When finding USE_EXPAND_VALUES_" + x + ":");
    std::unordered_map<std::string, std::shared_ptr<Set<std::string> > >::const_iterator i(_imp->use_expand_values.find(x));
    if (i == _imp->use_expand_values.end())
        throw InternalError(PALUDIS_HERE, "Oops. We need USE_EXPAND_VALUES_" + x + ", but it's not in the map");
    return i->second;
}

const std::shared_ptr<const Set<std::string> >
TraditionalProfile::iuse_implicit() const
{
    return _imp->iuse_implicit;
}
