/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008 Ciaran McCreesh
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

#include <paludis/repositories/e/e_repository_profile.hh>
#include <paludis/repositories/e/e_repository_profile_file.hh>
#include <paludis/repositories/e/e_repository_mask_file.hh>
#include <paludis/repositories/e/e_repository_exceptions.hh>
#include <paludis/repositories/e/e_repository.hh>
#include <paludis/repositories/e/eapi.hh>

#include <paludis/util/log.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/save.hh>
#include <paludis/util/system.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/join.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/set.hh>
#include <paludis/util/options.hh>
#include <paludis/util/iterator_funcs.hh>
#include <paludis/util/create_iterator-impl.hh>
#include <paludis/util/config_file.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/choice.hh>
#include <paludis/dep_tag.hh>
#include <paludis/environment.hh>
#include <paludis/match_package.hh>
#include <paludis/distribution.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>

#include <tr1/unordered_map>
#include <tr1/unordered_set>
#include <list>
#include <algorithm>
#include <set>
#include <vector>

#include <strings.h>
#include <ctype.h>

using namespace paludis;
using namespace paludis::erepository;

template class WrappedForwardIterator<ERepositoryProfile::VirtualsConstIteratorTag,
         const std::pair<const QualifiedPackageName, std::tr1::shared_ptr<const PackageDepSpec> > >;

typedef std::tr1::unordered_map<std::string, std::tr1::shared_ptr<Set<UnprefixedChoiceName> > > KnownMap;

namespace
{
    typedef std::tr1::unordered_map<std::string, std::string, Hash<std::string> > EnvironmentVariablesMap;
    typedef std::tr1::unordered_map<QualifiedPackageName, std::tr1::shared_ptr<const PackageDepSpec>, Hash<QualifiedPackageName> > VirtualsMap;
    typedef std::tr1::unordered_map<QualifiedPackageName,
            std::list<std::pair<std::tr1::shared_ptr<const PackageDepSpec>, std::tr1::shared_ptr<const RepositoryMaskInfo> > >,
            Hash<QualifiedPackageName> > PackageMaskMap;

    typedef std::tr1::unordered_map<ChoiceNameWithPrefix, bool, Hash<ChoiceNameWithPrefix> > FlagStatusMap;
    typedef std::list<std::pair<std::tr1::shared_ptr<const PackageDepSpec>, FlagStatusMap> > PackageFlagStatusMapList;

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
     * Implementation for ERepositoryProfile.
     *
     * \ingroup grperepository
     * \see ERepositoryProfile
     */
    template<>
    class Implementation<ERepositoryProfile>
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

            ///\}

            ///\name Environment variables
            ///\{

            EnvironmentVariablesMap environment_variables;

            ///\}

            ///\name System package set
            ///\{

            std::tr1::shared_ptr<ConstTreeSequence<SetSpecTree, AllDepSpec> > system_packages;
            std::tr1::shared_ptr<GeneralSetDepTag> system_tag;

            ///\}

            ///\name Virtuals
            ///\{

            VirtualsMap virtuals;

            ///\}

            ///\name USE related values
            ///\{

            std::set<std::pair<ChoicePrefixName, UnprefixedChoiceName> > use;
            std::tr1::shared_ptr<Set<std::string> > use_expand;
            std::tr1::shared_ptr<Set<std::string> > use_expand_hidden;
            KnownMap known_choice_value_names;
            StackedValuesList stacked_values_list;

            ///\}

            ///\name Masks
            ///\{

            PackageMaskMap package_mask;

            ///\}

            ///\name Basic operations
            ///\{

            Implementation(const Environment * const e, const ERepository * const p,
                    const RepositoryName & name, const FSEntrySequence & dirs,
                    const std::string & arch_var_if_special) :
                packages_file(p),
                virtuals_file(p),
                package_mask_file(p),
                env(e),
                repository(p),
                system_packages(new ConstTreeSequence<SetSpecTree, AllDepSpec>(
                            std::tr1::shared_ptr<AllDepSpec>(new AllDepSpec))),
                system_tag(new GeneralSetDepTag(SetName("system"), stringify(name))),
                use_expand(new Set<std::string>),
                use_expand_hidden(new Set<std::string>)
            {
                Context context("When loading profiles '" + join(dirs.begin(), dirs.end(), "' '") + "' for repository '" + stringify(name) + "':");

                if (dirs.empty())
                    throw ERepositoryConfigurationError("No profiles directories specified");

                load_environment();

                for (FSEntrySequence::ConstIterator d(dirs.begin()), d_end(dirs.end()) ;
                        d != d_end ; ++d)
                {
                    Context subcontext("When using directory '" + stringify(*d) + "':");

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

            ~Implementation()
            {
            }

            ///\}
    };
}

void
Implementation<ERepositoryProfile>::load_environment()
{
    environment_variables["CONFIG_PROTECT"] = getenv_with_default("CONFIG_PROTECT", "/etc");
    environment_variables["CONFIG_PROTECT_MASK"] = getenv_with_default("CONFIG_PROTECT_MASK", "");
}

void
Implementation<ERepositoryProfile>::load_profile_directory_recursively(const FSEntry & dir)
{
    Context context("When adding profile directory '" + stringify(dir) + ":");

    if (! dir.is_directory_or_symlink_to_directory())
    {
        Log::get_instance()->message("e.profile.not_a_directory", ll_warning, lc_context)
            << "Profile component '" << dir << "' is not a directory";
        return;
    }

    const std::tr1::shared_ptr<const EAPI> eapi(EAPIData::get_instance()->eapi_from_string(
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
}

void
Implementation<ERepositoryProfile>::load_profile_parent(const FSEntry & dir)
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
Implementation<ERepositoryProfile>::load_profile_make_defaults(const FSEntry & dir)
{
    Context context("When handling make.defaults file for profile directory '" + stringify(dir) + ":");

    if (! (dir / "make.defaults").exists())
        return;

    const std::tr1::shared_ptr<const EAPI> eapi(EAPIData::get_instance()->eapi_from_string(
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
}

void
Implementation<ERepositoryProfile>::load_special_make_defaults_vars(const FSEntry & dir)
{
    const std::tr1::shared_ptr<const EAPI> eapi(EAPIData::get_instance()->eapi_from_string(
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
Implementation<ERepositoryProfile>::is_incremental(const EAPI & e, const std::string & s) const
{
    Context c("When checking whether '" + s + "' is incremental:");

    return (! s.empty()) && (
            (s == e.supported()->ebuild_environment_variables()->env_use())
            || (s == e.supported()->ebuild_environment_variables()->env_use_expand())
            || (s == e.supported()->ebuild_environment_variables()->env_use_expand_hidden())
            || s == "CONFIG_PROTECT"
            || s == "CONFIG_PROTECT_MASK"
            || use_expand->end() != use_expand->find(s));
}

void
Implementation<ERepositoryProfile>::make_vars_from_file_vars()
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
                std::tr1::shared_ptr<PackageDepSpec> spec(new PackageDepSpec(
                            parse_elike_package_dep_spec(i->second.substr(1),
                                i->first->supported()->package_dep_spec_parse_options(),
                                std::tr1::shared_ptr<const PackageID>())));

                spec->set_tag(system_tag);
                system_packages->add(std::tr1::shared_ptr<SetSpecTree::ConstItem>(new TreeLeaf<SetSpecTree, PackageDepSpec>(spec)));
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
                virtuals.erase(v);
                virtuals.insert(std::make_pair(v, std::tr1::shared_ptr<PackageDepSpec>(new PackageDepSpec(
                                    parse_elike_package_dep_spec(tokens[1],
                                        line->first->supported()->package_dep_spec_parse_options(),
                                        std::tr1::shared_ptr<const PackageID>())))));
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
            std::tr1::shared_ptr<const PackageDepSpec> a(new PackageDepSpec(
                        parse_elike_package_dep_spec(line->second.first,
                            line->first->supported()->package_dep_spec_parse_options(),
                            std::tr1::shared_ptr<const PackageID>())));

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
Implementation<ERepositoryProfile>::load_basic_use_file(const FSEntry & file, FlagStatusMap & m)
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
Implementation<ERepositoryProfile>::load_spec_use_file(const EAPI & eapi, const FSEntry & file, PackageFlagStatusMapList & m)
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
            std::tr1::shared_ptr<const PackageDepSpec> spec(new PackageDepSpec(
                        parse_elike_package_dep_spec(*tokens.begin(), eapi.supported()->package_dep_spec_parse_options(),
                            std::tr1::shared_ptr<const PackageID>())));
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
Implementation<ERepositoryProfile>::add_use_expand_to_use()
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
Implementation<ERepositoryProfile>::fish_out_use_expand_names()
{
    Context context("When finding all known USE_EXPAND names:");

    for (Set<std::string>::ConstIterator x(use_expand->begin()), x_end(use_expand->end()) ;
            x != x_end ; ++x)
    {
        std::string lower_x;
        std::transform(x->begin(), x->end(), std::back_inserter(lower_x), &::tolower);
        known_choice_value_names.insert(std::make_pair(lower_x, make_shared_ptr(new Set<UnprefixedChoiceName>)));
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
Implementation<ERepositoryProfile>::handle_profile_arch_var(const std::string & s)
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

ERepositoryProfile::ERepositoryProfile(
        const Environment * const env, const ERepository * const p, const RepositoryName & name,
        const FSEntrySequence & location,
        const std::string & arch_var_if_special) :
    PrivateImplementationPattern<ERepositoryProfile>(
            new Implementation<ERepositoryProfile>(env, p, name, location, arch_var_if_special))
{
}

ERepositoryProfile::~ERepositoryProfile()
{
}

bool
ERepositoryProfile::use_masked(
        const std::tr1::shared_ptr<const PackageID> & id,
        const std::tr1::shared_ptr<const Choice> & choice,
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
ERepositoryProfile::use_forced(
        const std::tr1::shared_ptr<const PackageID> & id,
        const std::tr1::shared_ptr<const Choice> & choice,
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
ERepositoryProfile::use_state_ignoring_masks(
        const std::tr1::shared_ptr<const PackageID> & id,
        const std::tr1::shared_ptr<const Choice> & choice,
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

std::tr1::shared_ptr<const Set<UnprefixedChoiceName> >
ERepositoryProfile::known_choice_value_names(
        const std::tr1::shared_ptr<const PackageID> &,
        const std::tr1::shared_ptr<const Choice> & choice
        ) const
{
    std::string lower_x;
    std::transform(choice->raw_name().begin(), choice->raw_name().end(), std::back_inserter(lower_x), &::tolower);
    KnownMap::const_iterator i(_imp->known_choice_value_names.find(lower_x));
    if (_imp->known_choice_value_names.end() == i)
        throw InternalError(PALUDIS_HERE, lower_x);
    return i->second;
}

std::string
ERepositoryProfile::environment_variable(const std::string & s) const
{
    EnvironmentVariablesMap::const_iterator i(_imp->environment_variables.find(s));
    if (_imp->environment_variables.end() == i)
        return "";
    else
        return i->second;
}

std::tr1::shared_ptr<SetSpecTree::ConstItem>
ERepositoryProfile::system_packages() const
{
    return _imp->system_packages;
}

ERepositoryProfile::VirtualsConstIterator
ERepositoryProfile::begin_virtuals() const
{
    return VirtualsConstIterator(_imp->virtuals.begin());
}

ERepositoryProfile::VirtualsConstIterator
ERepositoryProfile::find_virtual(const QualifiedPackageName & n) const
{
    return VirtualsConstIterator(_imp->virtuals.find(n));
}

ERepositoryProfile::VirtualsConstIterator
ERepositoryProfile::end_virtuals() const
{
    return VirtualsConstIterator(_imp->virtuals.end());
}

std::tr1::shared_ptr<const RepositoryMaskInfo>
ERepositoryProfile::profile_masked(const PackageID & id) const
{
    PackageMaskMap::const_iterator rr(_imp->package_mask.find(id.name()));
    if (_imp->package_mask.end() == rr)
        return std::tr1::shared_ptr<const RepositoryMaskInfo>();
    else
    {
        for (std::list<std::pair<std::tr1::shared_ptr<const PackageDepSpec>, std::tr1::shared_ptr<const RepositoryMaskInfo> > >::const_iterator k(rr->second.begin()),
                k_end(rr->second.end()) ; k != k_end ; ++k)
            if (match_package(*_imp->env, *k->first, id, MatchPackageOptions()))
                return k->second;
    }

    return std::tr1::shared_ptr<const RepositoryMaskInfo>();
}

const std::tr1::shared_ptr<const Set<std::string> >
ERepositoryProfile::use_expand() const
{
    return _imp->use_expand;
}

const std::tr1::shared_ptr<const Set<std::string> >
ERepositoryProfile::use_expand_hidden() const
{
    return _imp->use_expand_hidden;
}

