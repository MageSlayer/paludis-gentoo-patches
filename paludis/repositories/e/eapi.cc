/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010, 2011, 2014 Ciaran McCreesh
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

#include <paludis/repositories/e/eapi.hh>

#include <paludis/name.hh>
#include <paludis/choice.hh>
#include <paludis/dep_spec.hh>

#include <paludis/util/attributes.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/system.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/destringify.hh>
#include <paludis/util/set.hh>
#include <paludis/util/map.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/singleton-impl.hh>
#include <paludis/util/config_file.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/fs_iterator.hh>
#include <paludis/util/env_var_names.hh>

#include <unordered_map>
#include <map>
#include <vector>
#include <list>

using namespace paludis;
using namespace paludis::erepository;

namespace
{
    std::string predefined(const std::string & d, const KeyValueConfigFile &, const std::string & v)
    {
        if (v == "PALUDIS_EAPIS_DIR")
            return d;
        else
            return "";
    }

    std::string check_get(const KeyValueConfigFile & k, const std::string & key)
    {
        return k.get(key);
    }

    template <typename T_>
    T_ destringify_key(const KeyValueConfigFile & k, const std::string & key)
    {
        Context context("When getting key '" + key + "':");
        return destringify<T_>(check_get(k, key));
    }

    std::shared_ptr<const EAPIEbuildEnvironmentVariables> make_ebuild_environment_variables(const KeyValueConfigFile & k)
    {
        return std::make_shared<EAPIEbuildEnvironmentVariables>(make_named_values<EAPIEbuildEnvironmentVariables>(
                        n::description_choices() = check_get(k, "description_choices"),
                        n::description_use() = check_get(k, "description_use"),
                        n::env_a() = check_get(k, "env_a"),
                        n::env_aa() = check_get(k, "env_aa"),
                        n::env_accept_license() = check_get(k, "env_accept_license"),
                        n::env_arch() = check_get(k, "env_arch"),
                        n::env_d() = check_get(k, "env_d"),
                        n::env_desttree() = check_get(k, "env_desttree"),
                        n::env_distdir() = check_get(k, "env_distdir"),
                        n::env_ebuild_phase() = check_get(k, "env_ebuild_phase"),
                        n::env_ebuild_phase_func() = check_get(k, "env_ebuild_phase_func"),
                        n::env_ed() = check_get(k, "env_ed"),
                        n::env_eprefix() = check_get(k, "env_eprefix"),
                        n::env_eroot() = check_get(k, "env_eroot"),
                        n::env_filesdir() = check_get(k, "env_filesdir"),
                        n::env_insdesttree() = check_get(k, "env_insdesttree"),
                        n::env_iuse_implicit() = check_get(k, "env_iuse_implicit"),
                        n::env_jobs() = check_get(k, "env_jobs"),
                        n::env_kv() = check_get(k, "env_kv"),
                        n::env_merge_type() = check_get(k, "env_merge_type"),
                        n::env_p() = check_get(k, "env_p"),
                        n::env_pf() = check_get(k, "env_pf"),
                        n::env_portdir() = check_get(k, "env_portdir"),
                        n::env_replaced_by_id() = check_get(k, "env_replaced_by_id"),
                        n::env_replaced_by_version() = check_get(k, "env_replaced_by_version"),
                        n::env_replacing_ids() = check_get(k, "env_replacing_ids"),
                        n::env_replacing_versions() = check_get(k, "env_replacing_versions"),
                        n::env_t() = check_get(k, "env_t"),
                        n::env_use() = check_get(k, "env_use"),
                        n::env_use_expand() = check_get(k, "env_use_expand"),
                        n::env_use_expand_hidden() = check_get(k, "env_use_expand_hidden"),
                        n::env_use_expand_implicit() = check_get(k, "env_use_expand_implicit"),
                        n::env_use_expand_unprefixed() = check_get(k, "env_use_expand_unprefixed"),
                        n::env_use_expand_values_part() = check_get(k, "env_use_expand_values_part")
            ));
    }

    std::shared_ptr<EAPIMetadataVariable> make_metadata_variable(const KeyValueConfigFile & k, const std::string & s)
    {
        return std::make_shared<EAPIMetadataVariable>(make_named_values<EAPIMetadataVariable>(
                        n::description() = check_get(k, "description_" + s),
                        n::flat_list_index() = destringify_key<int>(k, "flat_list_" + s),
                        n::name() = check_get(k, "metadata_" + s)
                        ));
    }

    std::shared_ptr<const EAPIEbuildMetadataVariables> make_ebuild_metadata_variables(const KeyValueConfigFile & k)
    {
        return std::make_shared<EAPIEbuildMetadataVariables>(make_named_values<EAPIEbuildMetadataVariables>(
            n::bugs_to() = make_metadata_variable(k, "bugs_to"),
            n::build_depend_target() = make_metadata_variable(k, "build_depend_target"),
            n::build_depend_host() = make_metadata_variable(k, "build_depend_host"),
            n::defined_phases() = make_metadata_variable(k, "defined_phases"),
            n::dependencies() = make_metadata_variable(k, "dependencies"),
            n::eapi() = make_metadata_variable(k, "eapi"),
            n::generated_from() = make_metadata_variable(k, "generated_from"),
            n::generated_time() = make_metadata_variable(k, "generated_time"),
            n::generated_using() = make_metadata_variable(k, "generated_using"),
            n::homepage() = make_metadata_variable(k, "homepage"),
            n::inherited() = make_metadata_variable(k, "inherited"),
            n::iuse() = make_metadata_variable(k, "iuse"),
            n::iuse_effective() = make_metadata_variable(k, "iuse_effective"),
            n::keywords() = make_metadata_variable(k, "keywords"),
            n::license() = make_metadata_variable(k, "license"),
            n::long_description() = make_metadata_variable(k, "long_description"),
            n::minimum_flat_list_size() = destringify_key<int>(k, "flat_list_minimum_size"),
            n::myoptions() = make_metadata_variable(k, "myoptions"),
            n::pdepend() = make_metadata_variable(k, "pdepend"),
            n::properties() = make_metadata_variable(k, "properties"),
            n::remote_ids() = make_metadata_variable(k, "remote_ids"),
            n::required_use() = make_metadata_variable(k, "required_use"),
            n::restrictions() = make_metadata_variable(k, "restrict"),
            n::run_depend() = make_metadata_variable(k, "run_depend"),
            n::scm_revision() = make_metadata_variable(k, "scm_revision"),
            n::short_description() = make_metadata_variable(k, "short_description"),
            n::slot() = make_metadata_variable(k, "slot"),
            n::src_uri() = make_metadata_variable(k, "src_uri"),
            n::upstream_changelog() = make_metadata_variable(k, "upstream_changelog"),
            n::upstream_documentation() = make_metadata_variable(k, "upstream_documentation"),
            n::upstream_release_notes() = make_metadata_variable(k, "upstream_release_notes"),
            n::use() = make_metadata_variable(k, "use"),
            n::use_expand() = make_metadata_variable(k, "use_expand"),
            n::use_expand_hidden() = make_metadata_variable(k, "use_expand_hidden")
            ));
    }

    std::shared_ptr<Set<std::string> > make_set(const std::string & s)
    {
        std::shared_ptr<Set<std::string> > result(std::make_shared<Set<std::string>>());
        tokenise_whitespace(s, result->inserter());
        return result;
    }

    std::shared_ptr<const EAPIEbuildOptions> make_ebuild_options(const KeyValueConfigFile & k)
    {
        return std::make_shared<EAPIEbuildOptions>(make_named_values<EAPIEbuildOptions>(
                        n::banneddir() = check_get(k, "banneddir"),
                        n::bash_compat() = check_get(k, "bash_compat"),
                        n::binary_from_env_variables() = check_get(k, "binary_from_env_variables"),
                        n::bracket_merged_variables() = check_get(k, "bracket_merged_variables"),
                        n::bracket_merged_variables_annotatable() = check_get(k, "bracket_merged_variables_annotatable"),
                        n::bracket_merged_variables_annotation() = check_get(k, "bracket_merged_variables_annotation"),
                        n::directory_if_exists_variables() = check_get(k, "directory_if_exists_variables"),
                        n::directory_variables() = check_get(k, "directory_variables"),
                        n::ebuild_functions() = check_get(k, "ebuild_functions"),
                        n::ebuild_module_suffixes() = check_get(k, "ebuild_module_suffixes"),
                        n::ebuild_must_not_set_variables() = check_get(k, "ebuild_must_not_set_variables"),
                        n::eclass_must_not_set_variables() = check_get(k, "eclass_must_not_set_variables"),
                        n::f_function_prefix() = check_get(k, "f_function_prefix"),
                        n::fix_mtimes() = destringify_key<bool>(k, "fix_mtimes"),
                        n::fs_location_description() = check_get(k, "fs_location_description"),
                        n::fs_location_name() = check_get(k, "fs_location_name"),
                        n::has_subslots() = destringify_key<bool>(k, "has_subslots"),
                        n::ignore_pivot_env_functions() = check_get(k, "ignore_pivot_env_functions"),
                        n::ignore_pivot_env_variables() = check_get(k, "ignore_pivot_env_variables"),
                        n::load_modules() = check_get(k, "load_modules"),
                        n::must_not_change_after_source_variables() = check_get(k, "must_not_change_after_source_variables"),
                        n::must_not_change_variables() = check_get(k, "must_not_change_variables"),
                        n::must_not_set_vars_starting_with() = check_get(k, "must_not_set_vars_starting_with"),
                        n::no_s_workdir_fallback() = destringify_key<bool>(k, "no_s_workdir_fallback"),
                        n::non_empty_variables() = check_get(k, "non_empty_variables"),
                        n::rdepend_defaults_to_depend() = destringify_key<bool>(k, "rdepend_defaults_to_depend"),
                        n::require_use_expand_in_iuse() = destringify_key<bool>(k, "require_use_expand_in_iuse"),
                        n::restrict_fetch() = make_set(check_get(k, "restrict_fetch")),
                        n::restrict_mirror() = make_set(check_get(k, "restrict_mirror")),
                        n::restrict_primaryuri() = make_set(check_get(k, "restrict_primaryuri")),
                        n::save_base_variables() = check_get(k, "save_base_variables"),
                        n::save_unmodifiable_variables() = check_get(k, "save_unmodifiable_variables"),
                        n::save_variables() = check_get(k, "save_variables"),
                        n::shell_options() = check_get(k, "shell_options"),
                        n::shell_options_disabled() = check_get(k, "shell_options_disabled"),
                        n::shell_options_global() = check_get(k, "shell_options_global"),
                        n::source_merged_variables() = check_get(k, "source_merged_variables"),
                        n::support_eclasses() = destringify_key<bool>(k, "support_eclasses"),
                        n::support_eclass_dir() = destringify_key<bool>(k, "support_eclass_dir"),
                        n::support_portdir() = destringify_key<bool>(k, "support_portdir"),
                        n::support_exlibs() = destringify_key<bool>(k, "support_exlibs"),
                        n::utility_path_suffixes() = check_get(k, "utility_path_suffixes"),
                        n::vdb_from_env_unless_empty_variables() = check_get(k, "vdb_from_env_unless_empty_variables"),
                        n::vdb_from_env_variables() = check_get(k, "vdb_from_env_variables"),
                        n::want_portage_emulation_vars() = destringify_key<bool>(k, "want_portage_emulation_vars")
                ));
    }

    std::shared_ptr<const EAPIEbuildPhases> make_ebuild_phases(const KeyValueConfigFile & k)
    {
        return std::make_shared<EAPIEbuildPhases>(make_named_values<EAPIEbuildPhases>(
                        n::ebuild_bad_options() = check_get(k, "ebuild_bad_options"),
                        n::ebuild_config() = check_get(k, "ebuild_config"),
                        n::ebuild_fetch_extra() = check_get(k, "ebuild_fetch_extra"),
                        n::ebuild_info() = check_get(k, "ebuild_info"),
                        n::ebuild_install() = check_get(k, "ebuild_install"),
                        n::ebuild_metadata() = check_get(k, "ebuild_metadata"),
                        n::ebuild_new_upgrade_phase_order() = destringify_key<bool>(k, "ebuild_new_upgrade_phase_order"),
                        n::ebuild_nofetch() = check_get(k, "ebuild_nofetch"),
                        n::ebuild_pretend() = check_get(k, "ebuild_pretend"),
                        n::ebuild_uninstall() = check_get(k, "ebuild_uninstall"),
                        n::ebuild_variable() = check_get(k, "ebuild_variable")
            ));
    }

    std::shared_ptr<const EAPIPipeCommands> make_pipe_commands(const KeyValueConfigFile & k)
    {
        return std::make_shared<EAPIPipeCommands>(make_named_values<EAPIPipeCommands>(
                        n::no_slot_or_repo() = destringify_key<bool>(k, "pipe_commands_no_slot_or_repo")
                        ));
    }

    std::shared_ptr<const EAPIProfileOptions> make_profile_options(const KeyValueConfigFile & k)
    {
        return std::make_shared<EAPIProfileOptions>(make_named_values<EAPIProfileOptions>(
                        n::use_stable_mask_force() = destringify_key<bool>(k, "use_stable_mask_force")
                        ));
    }

    std::shared_ptr<const EAPIToolsOptions> make_tool_options(const KeyValueConfigFile & k)
    {
        return std::make_shared<EAPIToolsOptions>(make_named_values<EAPIToolsOptions>(
                        n::best_has_version_host_root() = destringify_key<bool>(k, "best_has_version_host_root"),
                        n::controllable_strip() = destringify_key<bool>(k, "controllable_strip"),
                        n::die_supports_dash_n() = destringify_key<bool>(k, "die_supports_dash_n"),
                        n::dodoc_r() = destringify_key<bool>(k, "dodoc_r"),
                        n::doins_symlink() = destringify_key<bool>(k, "doins_symlink"),
                        n::doman_lang_filenames() = destringify_key<bool>(k, "doman_lang_filenames"),
                        n::doman_lang_filenames_overrides() = destringify_key<bool>(k, "doman_lang_filenames_overrides"),
                        n::domo_respects_into() = destringify_key<bool>(k, "domo_respects_into"),
                        n::dosym_mkdir() = destringify_key<bool>(k, "dosym_mkdir"),
                        n::econf_extra_options() = k.get("econf_extra_options"),
                        n::econf_extra_options_help_dependent() = k.get("econf_extra_options_help_dependent"),
                        n::failure_is_fatal() = destringify_key<bool>(k, "failure_is_fatal"),
                        n::log_to_stdout() = destringify_key<bool>(k, "log_to_stdout"),
                        n::new_stdin() = destringify_key<bool>(k, "new_stdin"),
                        n::unpack_any_path() = destringify_key<bool>(k, "unpack_any_path"),
                        n::unpack_case_insensitive() = destringify_key<bool>(k, "unpack_case_insensitive"),
                        n::unpack_fix_permissions() = destringify_key<bool>(k, "unpack_fix_permissions"),
                        n::unpack_suffixes() = k.get("unpack_suffixes"),
                        n::unpack_unrecognised_is_fatal() = destringify_key<bool>(k, "unpack_unrecognised_is_fatal"),
                        n::use_with_enable_empty_third_argument() = destringify_key<bool>(k, "use_with_enable_empty_third_argument")
                        ));
    }

    std::shared_ptr<const EAPIAnnotations> make_annotations(const KeyValueConfigFile & k)
    {
        return std::make_shared<EAPIAnnotations>(make_named_values<EAPIAnnotations>(
                    n::blocker_resolution() = k.get("annotations_blocker_resolution"),
                    n::blocker_resolution_manual() = k.get("annotations_blocker_resolution_manual"),
                    n::blocker_resolution_uninstall_blocked_after() = k.get("annotations_blocker_resolution_uninstall_blocked_after"),
                    n::blocker_resolution_uninstall_blocked_before() = k.get("annotations_blocker_resolution_uninstall_blocked_before"),
                    n::blocker_resolution_upgrade_blocked_before() = k.get("annotations_blocker_resolution_upgrade_blocked_before"),
                    n::general_author() = k.get("annotations_general_author"),
                    n::general_date() = k.get("annotations_general_date"),
                    n::general_description() = k.get("annotations_general_description"),
                    n::general_lang() = k.get("annotations_general_lang"),
                    n::general_note() = k.get("annotations_general_note"),
                    n::general_token() = k.get("annotations_general_token"),
                    n::general_url() = k.get("annotations_general_url"),
                    n::licence_last_checked() = k.get("annotations_licence_last_checked"),
                    n::myoptions_number_selected() = k.get("annotations_myoptions_number_selected"),
                    n::myoptions_number_selected_at_least_one() = k.get("annotations_myoptions_number_selected_at_least_one"),
                    n::myoptions_number_selected_at_most_one() = k.get("annotations_myoptions_number_selected_at_most_one"),
                    n::myoptions_number_selected_exactly_one() = k.get("annotations_myoptions_number_selected_exactly_one"),
                    n::myoptions_presumed() = k.get("annotations_myoptions_presumed"),
                    n::myoptions_requires() = k.get("annotations_myoptions_requires"),
                    n::suggestions_group_name() = k.get("annotations_suggesions_group_name"),
                    n::system_implicit() = k.get("annotations_system_implicit")
                    ));
    }

    std::shared_ptr<const EAPIChoicesOptions> make_choices_options(const KeyValueConfigFile & k)
    {
        return std::make_shared<EAPIChoicesOptions>(make_named_values<EAPIChoicesOptions>(
                        n::fancy_test_flag() = check_get(k, "fancy_test_flag"),
                        n::has_expensive_tests() = destringify_key<bool>(k, "has_expensive_tests"),
                        n::has_optional_tests() = destringify_key<bool>(k, "has_optional_tests"),
                        n::has_recommended_tests() = destringify_key<bool>(k, "has_recommended_tests"),
                        n::profile_iuse_injection() = destringify_key<bool>(k, "profile_iuse_injection"),
                        n::use_expand_separator() = destringify_key<char>(k, "use_expand_separator"),
                        n::profile_negative_use() = destringify_key<bool>(k, "profile_negative_use")
                        ));
    }

    std::shared_ptr<const SupportedEAPI> make_supported_eapi(const KeyValueConfigFile & k)
    {
        ELikePackageDepSpecOptions package_dep_spec_parse_options;
        {
            std::list<std::string> package_dep_spec_parse_options_tokens;
            tokenise_whitespace(check_get(k, "package_dep_spec_parse_options"), std::back_inserter(package_dep_spec_parse_options_tokens));
            for (const auto & package_dep_spec_parse_options_token : package_dep_spec_parse_options_tokens)
                package_dep_spec_parse_options += destringify<ELikePackageDepSpecOption>(package_dep_spec_parse_options_token);
        }

        DependencySpecTreeParseOptions dependency_spec_tree_parse_options;
        {
            std::list<std::string> dependency_spec_tree_parse_options_tokens;
            tokenise_whitespace(check_get(k, "dependency_spec_tree_parse_options"), std::back_inserter(dependency_spec_tree_parse_options_tokens));
            for (const auto & dependency_spec_tree_parse_options_token : dependency_spec_tree_parse_options_tokens)
                dependency_spec_tree_parse_options += destringify<DependencySpecTreeParseOption>(dependency_spec_tree_parse_options_token);
        }

        IUseFlagParseOptions iuse_flag_parse_options;
        {
            std::list<std::string> iuse_flag_parse_options_tokens;
            tokenise_whitespace(check_get(k, "iuse_flag_parse_options"), std::back_inserter(iuse_flag_parse_options_tokens));
            for (const auto & iuse_flag_parse_options_token : iuse_flag_parse_options_tokens)
                iuse_flag_parse_options += destringify<IUseFlagParseOption>(iuse_flag_parse_options_token);
        }

        VersionSpecOptions version_spec_options;
        {
            std::list<std::string> version_spec_options_tokens;
            tokenise_whitespace(check_get(k, "version_spec_options"), std::back_inserter(version_spec_options_tokens));
            for (const auto & version_spec_options_token : version_spec_options_tokens)
                version_spec_options += destringify<VersionSpecOption>(version_spec_options_token);
        }

        bool has_allow_empty_dirs(false);
        MergerOptions merger_options;
        {
            std::list<std::string> merger_options_tokens;
            tokenise_whitespace(check_get(k, "merger_options"), std::back_inserter(merger_options_tokens));
            for (const auto & merger_options_token : merger_options_tokens)
            {
                if (std::string("allow_empty_dirs") == merger_options_token) has_allow_empty_dirs = true;
                merger_options += destringify<MergerOption>(merger_options_token);
            }
        }

        FSMergerOptions fs_merger_options;
        {
            std::list<std::string> fs_merger_options_tokens;
            tokenise_whitespace(check_get(k, "fs_merger_options"), std::back_inserter(fs_merger_options_tokens));
            for (const auto & fs_merger_options_token : fs_merger_options_tokens)
                fs_merger_options += destringify<FSMergerOption>(fs_merger_options_token);
        }

        auto permitted_directories = check_get(k, "permitted_directories");
        if (has_allow_empty_dirs && std::string("") != permitted_directories)
        {
            throw EAPIConfigurationError("Merger code doesn't handle having both allow_empty_dirs and permitted_directories");
        }

        return std::make_shared<SupportedEAPI>(make_named_values<SupportedEAPI>(
                        n::allow_tokens_in_mask_files() = destringify_key<bool>(k, "allow_tokens_in_mask_files"),
                        n::annotations() = make_annotations(k),
                        n::can_be_pbin() = destringify_key<bool>(k, "can_be_pbin"),
                        n::choices_options() = make_choices_options(k),
                        n::dependency_labels() = std::make_shared<const EAPILabels>(check_get(k, "dependency_labels")),
                        n::dependency_spec_tree_parse_options() = dependency_spec_tree_parse_options,
                        n::ebuild_environment_variables() = make_ebuild_environment_variables(k),
                        n::ebuild_metadata_variables() = make_ebuild_metadata_variables(k),
                        n::ebuild_options() = make_ebuild_options(k),
                        n::ebuild_phases() = make_ebuild_phases(k),
                        n::fs_merger_options() = fs_merger_options,
                        n::is_pbin() = destringify_key<bool>(k, "is_pbin"),
                        n::iuse_flag_parse_options() = iuse_flag_parse_options,
                        n::merger_options() = merger_options,
                        n::package_dep_spec_parse_options() = package_dep_spec_parse_options,
                        n::parts_prefix() =
                            std::make_shared<ChoicePrefixName>(check_get(k, "parts_prefix")),
                        n::permitted_directories() = permitted_directories,
                        n::pipe_commands() = make_pipe_commands(k),
                        n::profile_options() = make_profile_options(k),
                        n::tools_options() = make_tool_options(k),
                        n::uri_labels() = std::make_shared<const EAPILabels>(check_get(k, "uri_labels")),
                        n::userpriv_cannot_use_root() = destringify_key<bool>(k, "userpriv_cannot_use_root"),
                        n::version_spec_options() = version_spec_options
                        ));
    }
}

namespace paludis
{
    template<>
    struct Imp<EAPIData>
    {
        std::unordered_map<std::string, std::shared_ptr<const EAPI>, Hash<std::string> > values;

        Imp()
        {
            Context c("When loading EAPI data:");

            for (FSIterator d(FSPath(getenv_with_default(env_vars::eapis_dir, DATADIR "/paludis/eapis")), { fsio_inode_sort }), d_end ;
                    d != d_end ; ++d)
            {
                if (! is_file_with_extension(*d, ".conf", { }))
                    continue;

                Context cc("When loading EAPI file '" + stringify(*d) + "':");
                KeyValueConfigFile k(*d, { },
                        std::bind(&predefined, stringify(d->dirname()), std::placeholders::_1, std::placeholders::_2),
                        &KeyValueConfigFile::no_transformation);

                std::shared_ptr<EAPI> eapi(std::make_shared<EAPI>(make_named_values<EAPI>(
                                n::exported_name() = check_get(k, "exported_name"),
                                n::name() = strip_trailing_string(d->basename(), ".conf"),
                                n::supported() = make_supported_eapi(k)
                                )));

                values.insert(std::make_pair(strip_trailing_string(d->basename(), ".conf"), eapi));
            }

            std::unordered_map<std::string, std::shared_ptr<const EAPI>, Hash<std::string> >::const_iterator i(values.find("0"));
            if (i == values.end())
                throw EAPIConfigurationError("No EAPI configuration found for EAPI 0");
            else
                values.insert(std::make_pair("", i->second));
        }
    };
}

EAPIConfigurationError::EAPIConfigurationError(const std::string & s) noexcept :
    ConfigurationError("EAPI configuration error: " + s)
{
}

EAPIData::EAPIData() :
    _imp()
{
}

EAPIData::~EAPIData() = default;

std::shared_ptr<const EAPI>
EAPIData::eapi_from_string(const std::string & s) const
{
    std::unordered_map<std::string, std::shared_ptr<const EAPI>, Hash<std::string> >::const_iterator i(_imp->values.find(s));
    if (i != _imp->values.end())
        return i->second;

    return std::make_shared<EAPI>(make_named_values<EAPI>(
                    n::exported_name() = s,
                    n::name() = s,
                    n::supported() = nullptr)
                );
}

std::shared_ptr<const EAPI>
EAPIData::unknown_eapi() const
{
    return std::make_shared<EAPI>(make_named_values<EAPI>(
                    n::exported_name() = "UNKNOWN",
                    n::name() = "UNKNOWN",
                    n::supported() = std::shared_ptr<const SupportedEAPI>())
                );
}

namespace paludis
{
    template <>
    struct Imp<EAPILabels>
    {
        std::map<std::string, std::string> v;

        Imp();
        Imp(const Imp &);
    };
}

Imp<EAPILabels>::Imp() = default;

Imp<EAPILabels>::Imp(const Imp &) = default;

EAPILabels::EAPILabels(const std::string & s) :
    _imp()
{
    std::vector<std::string> tokens;

    tokenise<delim_kind::AnyOfTag, delim_mode::DelimiterTag>(s, ";", "", std::back_inserter(tokens));

    for (const auto & token : tokens)
    {
        std::vector<std::string> values;
        tokenise<delim_kind::AnyOfTag, delim_mode::DelimiterTag>(token, "=", "", std::back_inserter(values));

        if (values.size() != 2)
            throw EAPIConfigurationError("EAPI labels value '" + s + "' has bad values size '" + stringify(values.size()) + "'");

        _imp->v.insert(std::make_pair(strip_leading(strip_trailing(values[0], " \t\r\n"), " \t\r\n"),
                    strip_leading(strip_trailing(values[1], " \t\r\n"), " \t\r\n")));
    }
}

EAPILabels::EAPILabels(const EAPILabels & other) :
    _imp(*other._imp.operator-> ())
{
}

EAPILabels::~EAPILabels() = default;

const std::string
EAPILabels::class_for_label(const std::string & l) const
{
    std::map<std::string, std::string>::const_iterator i(_imp->v.find(l));
    if (_imp->v.end() == i)
        return "";
    return i->second;
}

namespace paludis
{
    template class Singleton<EAPIData>;
}
