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

#include <paludis/repositories/e/traditional_profile.hh>
#include <paludis/repositories/e/profile_file.hh>
#include <paludis/repositories/e/e_repository_mask_file.hh>
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
#include <paludis/util/mutex.hh>
#include <paludis/util/map.hh>
#include <paludis/choice.hh>
#include <paludis/dep_tag.hh>
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
#include <vector>

#include <strings.h>
#include <ctype.h>

using namespace paludis;
using namespace paludis::erepository;

typedef std::unordered_map<std::string, std::shared_ptr<Set<UnprefixedChoiceName> > > KnownMap;

namespace
{
    typedef std::unordered_map<std::string, std::string, Hash<std::string> > EnvironmentVariablesMap;
    typedef std::unordered_map<QualifiedPackageName,
            std::list<std::pair<std::shared_ptr<const PackageDepSpec>, std::shared_ptr<const RepositoryMaskInfo> > >,
            Hash<QualifiedPackageName> > PackageMaskMap;

    typedef std::unordered_map<ChoiceNameWithPrefix, bool, Hash<ChoiceNameWithPrefix> > FlagStatusMap;
    typedef std::list<std::pair<std::shared_ptr<const PackageDepSpec>, FlagStatusMap> > PackageFlagStatusMapList;

    struct StackedValues
    {
        std::string origin;

        FlagStatusMap use_mask;
        FlagStatusMap use_force;
        PackageFlagStatusMapList package_use;
        PackageFlagStatusMapList package_use_mask;
        PackageFlagStatusMapList package_use_force;

        StackedValues(const std::string & o) :
            origin(o)
        {
        }
    };

    typedef std::list<StackedValues> StackedValuesList;
}

namespace paludis
{
    /**
     * Imp for TraditionalProfile.
     *
     * \ingroup grperepository
     * \see TraditionalProfile
     */
    template<>
    class Imp<TraditionalProfile>
    {
        private:
            void load_environment();
            void load_profile_directory_recursively(const FSEntry & dir);
            void load_profile_parent(const FSEntry & dir);
            void load_profile_make_defaults(const FSEntry & dir);

            void load_basic_use_file(const FSEntry & file, FlagStatusMap & m);
            void load_spec_use_file(const EAPI &, const FSEntry & file, PackageFlagStatusMapList & m);

            void add_use_expand_to_use();
            void fish_out_use_expand_names();
            void make_vars_from_file_vars();
            void handle_profile_arch_var(const std::string &);
            void load_special_make_defaults_vars(const FSEntry &);

            ProfileFile<LineConfigFile> packages_file;
            ProfileFile<LineConfigFile> virtuals_file;
            ProfileFile<MaskFile> package_mask_file;

            bool is_incremental(const EAPI &, const std::string & s) const;

        public:
            ///\name General variables
            ///\{

            const Environment * const env;
            const ERepository * const repository;

            std::shared_ptr<FSEntrySequence> profiles_with_parents;

            ///\}

            ///\name Environment variables
            ///\{

            EnvironmentVariablesMap environment_variables;

            ///\}

            ///\name System package set
            ///\{

            std::shared_ptr<SetSpecTree> system_packages;
            std::shared_ptr<GeneralSetDepTag> system_tag;

            ///\}

            ///\name Virtuals
            ///\{

            std::shared_ptr<Map<QualifiedPackageName, PackageDepSpec> > virtuals;

            ///\}

            ///\name USE related values
            ///\{

            std::set<std::pair<ChoicePrefixName, UnprefixedChoiceName> > use;
            std::shared_ptr<Set<std::string> > use_expand;
            std::shared_ptr<Set<std::string> > use_expand_hidden;
            std::shared_ptr<Set<std::string> > use_expand_unprefixed;
            std::shared_ptr<Set<std::string> > use_expand_implicit;
            std::shared_ptr<Set<std::string> > iuse_implicit;
            std::unordered_map<std::string, std::shared_ptr<Set<std::string> > > use_expand_values;
            KnownMap known_choice_value_names;
            mutable Mutex known_choice_value_names_for_separator_mutex;
            mutable std::unordered_map<char, KnownMap> known_choice_value_names_for_separator;
            StackedValuesList stacked_values_list;

            ///\}

            ///\name Masks
            ///\{

            PackageMaskMap package_mask;

            ///\}

            ///\name Basic operations
            ///\{

            Imp(const Environment * const e, const ERepository * const p,
                    const RepositoryName & name, const FSEntrySequence & dirs,
                    const std::string & arch_var_if_special, const bool profiles_explicitly_set) :
                packages_file(p),
                virtuals_file(p),
                package_mask_file(p),
                env(e),
                repository(p),
                profiles_with_parents(std::make_shared<FSEntrySequence>()),
                system_packages(std::make_shared<SetSpecTree>(std::make_shared<AllDepSpec>())),
                system_tag(std::make_shared<GeneralSetDepTag>(SetName("system"), stringify(name))),
                virtuals(std::make_shared<Map<QualifiedPackageName, PackageDepSpec>>()),
                use_expand(std::make_shared<Set<std::string>>()),
                use_expand_hidden(std::make_shared<Set<std::string>>()),
                use_expand_unprefixed(std::make_shared<Set<std::string>>()),
                use_expand_implicit(std::make_shared<Set<std::string>>()),
                iuse_implicit(std::make_shared<Set<std::string>>())
            {
                Context context("When loading profiles '" + join(dirs.begin(), dirs.end(), "' '") + "' for repository '" + stringify(name) + "':");

                if (dirs.empty())
                    throw ERepositoryConfigurationError("No profiles directories specified");

                load_environment();

                for (FSEntrySequence::ConstIterator d(dirs.begin()), d_end(dirs.end()) ;
                        d != d_end ; ++d)
                {
                    Context subcontext("When using directory '" + stringify(*d) + "':");

                    if (profiles_explicitly_set)
                        if (! p->params().ignore_deprecated_profiles())
                            if ((*d / "deprecated").is_regular_file_or_symlink_to_regular_file())
                                Log::get_instance()->message("e.profile.deprecated", ll_warning, lc_context) << "Profile directory '" << *d
                                    << "' is deprecated. See the file '" << (*d / "deprecated") << "' for details";

                    load_profile_directory_recursively(*d);
                }

                make_vars_from_file_vars();
                load_special_make_defaults_vars(*dirs.begin());
                add_use_expand_to_use();
                fish_out_use_expand_names();
                if (! arch_var_if_special.empty())
                    handle_profile_arch_var(arch_var_if_special);
            }

            ~Imp()
            {
            }

            ///\}
    };
}

void
Imp<TraditionalProfile>::load_environment()
{
    environment_variables["CONFIG_PROTECT"] = getenv_with_default("CONFIG_PROTECT", "/etc");
    environment_variables["CONFIG_PROTECT_MASK"] = getenv_with_default("CONFIG_PROTECT_MASK", "");
}

void
Imp<TraditionalProfile>::load_profile_directory_recursively(const FSEntry & dir)
{
    Context context("When adding profile directory '" + stringify(dir) + ":");

    if (! dir.is_directory_or_symlink_to_directory())
    {
        Log::get_instance()->message("e.profile.not_a_directory", ll_warning, lc_context)
            << "Profile component '" << dir << "' is not a directory";
        return;
    }

    const std::shared_ptr<const EAPI> eapi(EAPIData::get_instance()->eapi_from_string(
                repository->eapi_for_file(dir / "use.mask")));

    if (! eapi->supported())
        throw ERepositoryConfigurationError("Can't use profile directory '" + stringify(dir) +
                "' because it uses an unsupported EAPI");

    stacked_values_list.push_back(StackedValues(stringify(dir)));

    load_profile_parent(dir);
    load_profile_make_defaults(dir);

    load_basic_use_file(dir / "use.mask", stacked_values_list.back().use_mask);
    load_basic_use_file(dir / "use.force", stacked_values_list.back().use_force);
    load_spec_use_file(*eapi, dir / "package.use", stacked_values_list.back().package_use);
    load_spec_use_file(*eapi, dir / "package.use.mask", stacked_values_list.back().package_use_mask);
    load_spec_use_file(*eapi, dir / "package.use.force", stacked_values_list.back().package_use_force);

    packages_file.add_file(dir / "packages");
    if ((*DistributionData::get_instance()->distribution_from_string(env->distribution())).support_old_style_virtuals())
        virtuals_file.add_file(dir / "virtuals");
    package_mask_file.add_file(dir / "package.mask");

    profiles_with_parents->push_back(dir);
}

void
Imp<TraditionalProfile>::load_profile_parent(const FSEntry & dir)
{
    Context context("When handling parent file for profile directory '" + stringify(dir) + ":");

    if (! (dir / "parent").exists())
        return;

    LineConfigFile file(dir / "parent", LineConfigFileOptions() + lcfo_disallow_continuations);

    LineConfigFile::ConstIterator i(file.begin()), i_end(file.end());
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

            FSEntry parent_dir(dir);
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

                load_profile_directory_recursively(parent_dir);

            } while (false);
        }
}

void
Imp<TraditionalProfile>::load_profile_make_defaults(const FSEntry & dir)
{
    Context context("When handling make.defaults file for profile directory '" + stringify(dir) + ":");

    if (! (dir / "make.defaults").exists())
        return;

    const std::shared_ptr<const EAPI> eapi(EAPIData::get_instance()->eapi_from_string(
                repository->eapi_for_file(dir / "make.defaults")));
    if (! eapi->supported())
        throw ERepositoryConfigurationError("Can't use profile directory '" + stringify(dir) +
                "' because it uses an unsupported EAPI");

    KeyValueConfigFile file(dir / "make.defaults", KeyValueConfigFileOptions() +
            kvcfo_disallow_source + kvcfo_disallow_space_inside_unquoted_values + kvcfo_allow_inline_comments + kvcfo_allow_multiple_assigns_per_line,
            &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation);

    for (KeyValueConfigFile::ConstIterator k(file.begin()), k_end(file.end()) ;
            k != k_end ; ++k)
    {
        if (is_incremental(*eapi, k->first))
        {
            std::list<std::string> val, val_add;
            tokenise_whitespace(environment_variables[k->first], std::back_inserter(val));
            tokenise_whitespace(k->second, std::back_inserter(val_add));

            for (std::list<std::string>::const_iterator v(val_add.begin()), v_end(val_add.end()) ;
                    v != v_end ; ++v)
            {
                if (v->empty())
                    continue;
                if (*v == "-*")
                    val.clear();
                else if ('-' == v->at(0))
                    val.remove(v->substr(1));
                else
                    val.push_back(*v);
            }

            environment_variables[k->first] = join(val.begin(), val.end(), " ");
        }
        else
            environment_variables[k->first] = k->second;
    }

    std::string use_expand_var(eapi->supported()->ebuild_environment_variables()->env_use_expand());
    try
    {
        use_expand->clear();
        if (! use_expand_var.empty())
            tokenise_whitespace(environment_variables[use_expand_var], use_expand->inserter());
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
        use_expand_unprefixed->clear();
        if (! use_expand_unprefixed_var.empty())
            tokenise_whitespace(environment_variables[use_expand_unprefixed_var], use_expand_unprefixed->inserter());
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
        use_expand_implicit->clear();
        if (! use_expand_implicit_var.empty())
            tokenise_whitespace(environment_variables[use_expand_implicit_var], use_expand_implicit->inserter());
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
        iuse_implicit->clear();
        if (! iuse_implicit_var.empty())
            tokenise_whitespace(environment_variables[iuse_implicit_var], iuse_implicit->inserter());
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
        use_expand_values.clear();
        if (! use_expand_values_part_var.empty())
        {
            for (Set<std::string>::ConstIterator x(use_expand->begin()), x_end(use_expand->end()) ;
                    x != x_end ; ++x)
            {
                std::shared_ptr<Set<std::string> > v(std::make_shared<Set<std::string>>());
                tokenise_whitespace(environment_variables[use_expand_values_part_var + *x], v->inserter());
                use_expand_values.insert(std::make_pair(*x, v));
            }

            for (Set<std::string>::ConstIterator x(use_expand_unprefixed->begin()), x_end(use_expand_unprefixed->end()) ;
                    x != x_end ; ++x)
            {
                std::shared_ptr<Set<std::string> > v(std::make_shared<Set<std::string>>());
                tokenise_whitespace(environment_variables[use_expand_values_part_var + *x], v->inserter());
                use_expand_values.insert(std::make_pair(*x, v));
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

void
Imp<TraditionalProfile>::load_special_make_defaults_vars(const FSEntry & dir)
{
    const std::shared_ptr<const EAPI> eapi(EAPIData::get_instance()->eapi_from_string(
                repository->eapi_for_file(dir / "make.defaults")));
    if (! eapi->supported())
        throw ERepositoryConfigurationError("Can't use profile directory '" + stringify(dir) +
                "' because it uses an unsupported EAPI");

    std::string use_var(eapi->supported()->ebuild_environment_variables()->env_use());
    try
    {
        use.clear();
        if (! use_var.empty())
        {
            std::list<std::string> tokens;
            tokenise_whitespace(environment_variables[use_var], std::back_inserter(tokens));
            for (std::list<std::string>::const_iterator t(tokens.begin()), t_end(tokens.end()) ;
                    t != t_end ; ++t)
                use.insert(std::make_pair("", *t));
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
        use_expand->clear();
        if (! use_expand_var.empty())
            tokenise_whitespace(environment_variables[use_expand_var], use_expand->inserter());
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
        use_expand_hidden->clear();
        if (! use_expand_hidden_var.empty())
            tokenise_whitespace(environment_variables[use_expand_hidden_var], use_expand_hidden->inserter());
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

bool
Imp<TraditionalProfile>::is_incremental(const EAPI & e, const std::string & s) const
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

void
Imp<TraditionalProfile>::make_vars_from_file_vars()
{
    try
    {
        if (! repository->params().master_repositories())
            for (ProfileFile<LineConfigFile>::ConstIterator i(packages_file.begin()),
                    i_end(packages_file.end()) ; i != i_end ; ++i)
            {
                if (0 != i->second.compare(0, 1, "*", 0, 1))
                    continue;

                Context context_spec("When parsing '" + i->second + "':");
                std::shared_ptr<PackageDepSpec> spec(std::make_shared<PackageDepSpec>(
                            parse_elike_package_dep_spec(i->second.substr(1),
                                i->first->supported()->package_dep_spec_parse_options(),
                                i->first->supported()->version_spec_options(),
                                std::shared_ptr<const PackageID>())));

                spec->set_tag(system_tag);
                system_packages->root()->append(spec);
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

    if ((*DistributionData::get_instance()->distribution_from_string(
                env->distribution())).support_old_style_virtuals())
        try
        {
            for (ProfileFile<LineConfigFile>::ConstIterator line(virtuals_file.begin()), line_end(virtuals_file.end()) ;
                    line != line_end ; ++line)
            {
                std::vector<std::string> tokens;
                tokenise_whitespace(line->second, std::back_inserter(tokens));
                if (tokens.size() < 2)
                    continue;

                QualifiedPackageName v(tokens[0]);
                virtuals->erase(v);
                virtuals->insert(v, parse_elike_package_dep_spec(tokens[1],
                            line->first->supported()->package_dep_spec_parse_options(),
                            line->first->supported()->version_spec_options(),
                            std::shared_ptr<const PackageID>()));
            }
        }
        catch (const InternalError &)
        {
            throw;
        }
        catch (const Exception & e)
        {
            Log::get_instance()->message("e.profile.virtuals.failure", ll_warning, lc_context)
                << "Loading virtuals failed due to exception: " << e.message() << " (" << e.what() << ")";
        }

    for (ProfileFile<MaskFile>::ConstIterator line(package_mask_file.begin()), line_end(package_mask_file.end()) ;
            line != line_end ; ++line)
    {
        if (line->second.first.empty())
            continue;

        try
        {
            std::shared_ptr<const PackageDepSpec> a(std::make_shared<PackageDepSpec>(
                        parse_elike_package_dep_spec(line->second.first,
                            line->first->supported()->package_dep_spec_parse_options(),
                            line->first->supported()->version_spec_options(),
                            std::shared_ptr<const PackageID>())));

            if (a->package_ptr())
                package_mask[*a->package_ptr()].push_back(std::make_pair(a, line->second.second));
            else
                Log::get_instance()->message("e.profile.package_mask.bad_spec", ll_warning, lc_context)
                    << "Loading package.mask spec '" << line->second.first << "' failed because specification does not restrict to a "
                    "unique package";
        }
        catch (const InternalError &)
        {
            throw;
        }
        catch (const Exception & e)
        {
            Log::get_instance()->message("e.profile.package_mask.bad_spec", ll_warning, lc_context)
                << "Loading package.mask spec '" << line->second.first << "' failed due to exception '" << e.message() << "' ("
                << e.what() << ")";
        }
    }
}

void
Imp<TraditionalProfile>::load_basic_use_file(const FSEntry & file, FlagStatusMap & m)
{
    if (! file.exists())
        return;

    Context context("When loading basic use file '" + stringify(file) + ":");
    LineConfigFile f(file, LineConfigFileOptions() + lcfo_disallow_continuations);
    for (LineConfigFile::ConstIterator line(f.begin()), line_end(f.end()) ;
            line != line_end ; ++line)
    {
        std::list<std::string> tokens;
        tokenise_whitespace(*line, std::back_inserter(tokens));

        for (std::list<std::string>::const_iterator t(tokens.begin()), t_end(tokens.end()) ;
                t != t_end ; ++t)
        {
            try
            {
                if (t->empty())
                    continue;
                if ('-' == t->at(0))
                    m[ChoiceNameWithPrefix(t->substr(1))] = false;
                else
                    m[ChoiceNameWithPrefix(*t)] = true;
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
}

void
Imp<TraditionalProfile>::load_spec_use_file(const EAPI & eapi, const FSEntry & file, PackageFlagStatusMapList & m)
{
    if (! file.exists())
        return;

    Context context("When loading specised use file '" + stringify(file) + ":");
    LineConfigFile f(file, LineConfigFileOptions() + lcfo_disallow_continuations);
    for (LineConfigFile::ConstIterator line(f.begin()), line_end(f.end()) ;
            line != line_end ; ++line)
    {
        std::list<std::string> tokens;
        tokenise_whitespace(*line, std::back_inserter(tokens));

        if (tokens.empty())
            continue;

        try
        {
            std::shared_ptr<const PackageDepSpec> spec(std::make_shared<PackageDepSpec>(
                        parse_elike_package_dep_spec(*tokens.begin(), eapi.supported()->package_dep_spec_parse_options(),
                            eapi.supported()->version_spec_options(),
                            std::shared_ptr<const PackageID>())));
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
                << *line << "' due to exception '" << e.message() << "' (" << e.what() << ")";
        }
    }
}

void
Imp<TraditionalProfile>::add_use_expand_to_use()
{
    Context context("When adding USE_EXPAND to USE:");

    stacked_values_list.push_back(StackedValues("use_expand special values"));

    for (Set<std::string>::ConstIterator x(use_expand->begin()), x_end(use_expand->end()) ;
            x != x_end ; ++x)
    {
        std::string lower_x;
        std::transform(x->begin(), x->end(), std::back_inserter(lower_x), &::tolower);

        std::list<std::string> uses;
        tokenise_whitespace(environment_variables[stringify(*x)], std::back_inserter(uses));
        for (std::list<std::string>::const_iterator u(uses.begin()), u_end(uses.end()) ;
                u != u_end ; ++u)
            use.insert(std::make_pair(lower_x, *u));
    }
}

void
Imp<TraditionalProfile>::fish_out_use_expand_names()
{
    Context context("When finding all known USE_EXPAND names:");

    for (Set<std::string>::ConstIterator x(use_expand->begin()), x_end(use_expand->end()) ;
            x != x_end ; ++x)
    {
        std::string lower_x;
        std::transform(x->begin(), x->end(), std::back_inserter(lower_x), &::tolower);
        known_choice_value_names.insert(std::make_pair(lower_x, std::make_shared<Set<UnprefixedChoiceName>>()));
    }

    for (std::set<std::pair<ChoicePrefixName, UnprefixedChoiceName> >::const_iterator u(use.begin()), u_end(use.end()) ;
            u != u_end ; ++u)
    {
        if (! stringify(u->first).empty())
        {
            KnownMap::iterator i(known_choice_value_names.find(stringify(u->first)));
            if (i == known_choice_value_names.end())
                throw InternalError(PALUDIS_HERE, stringify(u->first));
            i->second->insert(u->second);
        }
    }
}

void
Imp<TraditionalProfile>::handle_profile_arch_var(const std::string & s)
{
    Context context("When handling profile " + s + " variable:");

    std::string arch_s(environment_variables[s]);
    if (arch_s.empty())
        throw ERepositoryConfigurationError("Variable '" + s + "' is unset or empty");

    stacked_values_list.push_back(StackedValues("arch special values"));
    try
    {
        std::string arch(arch_s);

        use.insert(std::make_pair(ChoicePrefixName(""), arch));
        stacked_values_list.back().use_force[ChoiceNameWithPrefix(arch)] = true;
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

TraditionalProfile::TraditionalProfile(
        const Environment * const env, const ERepository * const p, const RepositoryName & name,
        const FSEntrySequence & location,
        const std::string & arch_var_if_special, const bool x) :
    Pimp<TraditionalProfile>(env, p, name, location, arch_var_if_special, x)
{
}

TraditionalProfile::~TraditionalProfile()
{
}

std::shared_ptr<const FSEntrySequence>
TraditionalProfile::profiles_with_parents() const
{
    return _imp->profiles_with_parents;
}

bool
TraditionalProfile::use_masked(
        const std::shared_ptr<const PackageID> & id,
        const std::shared_ptr<const Choice> & choice,
        const UnprefixedChoiceName & value_unprefixed,
        const ChoiceNameWithPrefix & value_prefixed
        ) const
{
    if (stringify(choice->prefix()).empty() &&
            _imp->repository->arch_flags()->end() != _imp->repository->arch_flags()->find(value_unprefixed) &&
            (! use_state_ignoring_masks(id, choice, value_unprefixed, value_prefixed).is_true()))
        return true;

    bool result(false);
    for (StackedValuesList::const_iterator i(_imp->stacked_values_list.begin()),
            i_end(_imp->stacked_values_list.end()) ; i != i_end ; ++i)
    {
        FlagStatusMap::const_iterator f(i->use_mask.find(value_prefixed));
        if (i->use_mask.end() != f)
            result = f->second;

        for (PackageFlagStatusMapList::const_iterator g(i->package_use_mask.begin()),
                g_end(i->package_use_mask.end()) ; g != g_end ; ++g)
        {
            if (! match_package(*_imp->env, *g->first, *id, MatchPackageOptions()))
                continue;

            FlagStatusMap::const_iterator h(g->second.find(value_prefixed));
            if (g->second.end() != h)
                result = h->second;
        }
    }

    return result;
}

bool
TraditionalProfile::use_forced(
        const std::shared_ptr<const PackageID> & id,
        const std::shared_ptr<const Choice> & choice,
        const UnprefixedChoiceName & value_unprefixed,
        const ChoiceNameWithPrefix & value_prefixed
        ) const
{
    if (use_masked(id, choice, value_unprefixed, value_prefixed))
        return false;
    if (stringify(choice->prefix()).empty() && (_imp->repository->arch_flags()->end() != _imp->repository->arch_flags()->find(value_unprefixed)))
        return true;

    bool result(false);
    for (StackedValuesList::const_iterator i(_imp->stacked_values_list.begin()),
            i_end(_imp->stacked_values_list.end()) ; i != i_end ; ++i)
    {
        FlagStatusMap::const_iterator f(i->use_force.find(value_prefixed));
        if (i->use_force.end() != f)
            result = f->second;

        for (PackageFlagStatusMapList::const_iterator g(i->package_use_force.begin()),
                g_end(i->package_use_force.end()) ; g != g_end ; ++g)
        {
            if (! match_package(*_imp->env, *g->first, *id, MatchPackageOptions()))
                continue;

            FlagStatusMap::const_iterator h(g->second.find(value_prefixed));
            if (g->second.end() != h)
                result = h->second;
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
    Tribool result(_imp->use.end() != _imp->use.find(prefix_value) ? Tribool(true) : Tribool(indeterminate));

    for (StackedValuesList::const_iterator i(_imp->stacked_values_list.begin()),
            i_end(_imp->stacked_values_list.end()) ; i != i_end ; ++i)
    {
        for (PackageFlagStatusMapList::const_iterator g(i->package_use.begin()),
                g_end(i->package_use.end()) ; g != g_end ; ++g)
        {
            if (! match_package(*_imp->env, *g->first, *id, MatchPackageOptions()))
                continue;

            FlagStatusMap::const_iterator h(g->second.find(value_prefixed));
            if (g->second.end() != h)
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
        for (FlagStatusMap::const_iterator it(m.begin()),
                 it_end(m.end()); it_end != it; ++it)
            if (0 == stringify(it->first).compare(0, prefix.length(), prefix))
                result->insert(UnprefixedChoiceName(stringify(it->first).substr(prefix.length())));
    }

    void
    add_package_flag_status_map_list(const std::shared_ptr<Set<UnprefixedChoiceName> > result,
            const PackageFlagStatusMapList & m, const std::string & prefix)
    {
        for (PackageFlagStatusMapList::const_iterator it(m.begin()),
                 it_end(m.end()); it_end != it; ++it)
            add_flag_status_map(result, it->second, prefix);
    }
}

const std::shared_ptr<const Set<UnprefixedChoiceName> >
TraditionalProfile::known_choice_value_names(
        const std::shared_ptr<const ERepositoryID> & id,
        const std::shared_ptr<const Choice> & choice
        ) const
{
    Lock l(_imp->known_choice_value_names_for_separator_mutex);

    char separator(id->eapi()->supported()->choices_options()->use_expand_separator());
    std::unordered_map<char, KnownMap>::iterator it(_imp->known_choice_value_names_for_separator.find(separator));
    if (_imp->known_choice_value_names_for_separator.end() == it)
        it = _imp->known_choice_value_names_for_separator.insert(std::make_pair(separator, KnownMap())).first;

    std::string lower_x;
    std::transform(choice->raw_name().begin(), choice->raw_name().end(), std::back_inserter(lower_x), &::tolower);
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

        for (StackedValuesList::const_iterator sit(_imp->stacked_values_list.begin()),
                 sit_end(_imp->stacked_values_list.end()); sit_end != sit; ++sit)
        {
            add_flag_status_map(result, sit->use_mask, prefix);
            add_flag_status_map(result, sit->use_force, prefix);
            add_package_flag_status_map_list(result, sit->package_use, prefix);
            add_package_flag_status_map_list(result, sit->package_use_mask, prefix);
            add_package_flag_status_map_list(result, sit->package_use_force, prefix);
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

const std::shared_ptr<const Map<QualifiedPackageName, PackageDepSpec> >
TraditionalProfile::virtuals() const
{
    return _imp->virtuals;
}

const std::shared_ptr<const RepositoryMaskInfo>
TraditionalProfile::profile_masked(const PackageID & id) const
{
    PackageMaskMap::const_iterator rr(_imp->package_mask.find(id.name()));
    if (_imp->package_mask.end() == rr)
        return std::shared_ptr<const RepositoryMaskInfo>();
    else
    {
        for (std::list<std::pair<std::shared_ptr<const PackageDepSpec>, std::shared_ptr<const RepositoryMaskInfo> > >::const_iterator k(rr->second.begin()),
                k_end(rr->second.end()) ; k != k_end ; ++k)
            if (match_package(*_imp->env, *k->first, id, MatchPackageOptions()))
                return k->second;
    }

    return std::shared_ptr<const RepositoryMaskInfo>();
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

