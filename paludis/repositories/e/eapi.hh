/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_EAPI_HH
#define PALUDIS_GUARD_PALUDIS_EAPI_HH 1

#include <paludis/repositories/e/eapi-fwd.hh>
#include <paludis/repositories/e/dep_parser-fwd.hh>
#include <paludis/repositories/e/iuse.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/pimp.hh>
#include <paludis/util/options.hh>
#include <paludis/util/named_value.hh>
#include <paludis/util/singleton.hh>
#include <paludis/name.hh>
#include <paludis/dep_spec-fwd.hh>
#include <paludis/merger-fwd.hh>
#include <paludis/fs_merger-fwd.hh>
#include <memory>

namespace paludis
{
    namespace n
    {
        typedef Name<struct name_allow_tokens_in_mask_files> allow_tokens_in_mask_files;
        typedef Name<struct name_annotations> annotations;
        typedef Name<struct name_best_has_version_host_root> best_has_version_host_root;
        typedef Name<struct name_binary_from_env_variables> binary_from_env_variables;
        typedef Name<struct name_blocker_resolution> blocker_resolution;
        typedef Name<struct name_blocker_resolution_manual> blocker_resolution_manual;
        typedef Name<struct name_blocker_resolution_uninstall_blocked_after> blocker_resolution_uninstall_blocked_after;
        typedef Name<struct name_blocker_resolution_uninstall_blocked_before> blocker_resolution_uninstall_blocked_before;
        typedef Name<struct name_blocker_resolution_upgrade_blocked_before> blocker_resolution_upgrade_blocked_before;
        typedef Name<struct name_bracket_merged_variables> bracket_merged_variables;
        typedef Name<struct name_bracket_merged_variables_annotatable> bracket_merged_variables_annotatable;
        typedef Name<struct name_bracket_merged_variables_annotation> bracket_merged_variables_annotation;
        typedef Name<struct name_breaks_portage> breaks_portage;
        typedef Name<struct name_bugs_to> bugs_to;
        typedef Name<struct name_build_depend> build_depend;
        typedef Name<struct name_can_be_pbin> can_be_pbin;
        typedef Name<struct name_choices_options> choices_options;
        typedef Name<struct name_defined_phases> defined_phases;
        typedef Name<struct name_dependencies> dependencies;
        typedef Name<struct name_dependency_labels> dependency_labels;
        typedef Name<struct name_dependency_spec_tree_parse_options> dependency_spec_tree_parse_options;
        typedef Name<struct name_description> description;
        typedef Name<struct name_description_choices> description_choices;
        typedef Name<struct name_description_use> description_use;
        typedef Name<struct name_directory_if_exists_variables> directory_if_exists_variables;
        typedef Name<struct name_directory_variables> directory_variables;
        typedef Name<struct name_dodoc_r> dodoc_r;
        typedef Name<struct name_doins_symlink> doins_symlink;
        typedef Name<struct name_doman_lang_filenames> doman_lang_filenames;
        typedef Name<struct name_doman_lang_filenames_overrides> doman_lang_filenames_overrides;
        typedef Name<struct name_dosym_mkdir> dosym_mkdir;
        typedef Name<struct name_eapi> eapi;
        typedef Name<struct name_ebuild_bad_options> ebuild_bad_options;
        typedef Name<struct name_ebuild_config> ebuild_config;
        typedef Name<struct name_ebuild_environment_variables> ebuild_environment_variables;
        typedef Name<struct name_ebuild_fetch_extra> ebuild_fetch_extra;
        typedef Name<struct name_ebuild_functions> ebuild_functions;
        typedef Name<struct name_ebuild_info> ebuild_info;
        typedef Name<struct name_ebuild_install> ebuild_install;
        typedef Name<struct name_ebuild_metadata> ebuild_metadata;
        typedef Name<struct name_ebuild_metadata_variables> ebuild_metadata_variables;
        typedef Name<struct name_ebuild_module_suffixes> ebuild_module_suffixes;
        typedef Name<struct name_ebuild_must_not_set_variables> ebuild_must_not_set_variables;
        typedef Name<struct name_ebuild_nofetch> ebuild_nofetch;
        typedef Name<struct name_ebuild_options> ebuild_options;
        typedef Name<struct name_ebuild_phases> ebuild_phases;
        typedef Name<struct name_ebuild_pretend> ebuild_pretend;
        typedef Name<struct name_ebuild_uninstall> ebuild_uninstall;
        typedef Name<struct name_ebuild_variable> ebuild_variable;
        typedef Name<struct name_ebuild_new_upgrade_phase_order> ebuild_new_upgrade_phase_order;
        typedef Name<struct name_econf_extra_options> econf_extra_options;
        typedef Name<struct name_econf_extra_options_help_dependent> econf_extra_options_help_dependent;
        typedef Name<struct name_eclass_must_not_set_variables> eclass_must_not_set_variables;
        typedef Name<struct name_env_a> env_a;
        typedef Name<struct name_env_aa> env_aa;
        typedef Name<struct name_env_accept_license> env_accept_license;
        typedef Name<struct name_env_arch> env_arch;
        typedef Name<struct name_env_d> env_d;
        typedef Name<struct name_env_distdir> env_distdir;
        typedef Name<struct name_env_ebuild_phase> env_ebuild_phase;
        typedef Name<struct name_env_ed> env_ed;
        typedef Name<struct name_env_eprefix> env_eprefix;
        typedef Name<struct name_env_eroot> env_eroot;
        typedef Name<struct name_env_filesdir> env_filesdir;
        typedef Name<struct name_env_iuse_implicit> env_iuse_implicit;
        typedef Name<struct name_env_jobs> env_jobs;
        typedef Name<struct name_env_kv> env_kv;
        typedef Name<struct name_env_merge_type> env_merge_type;
        typedef Name<struct name_env_p> env_p;
        typedef Name<struct name_env_pf> env_pf;
        typedef Name<struct name_env_portdir> env_portdir;
        typedef Name<struct name_env_replaced_by_id> env_replaced_by_id;
        typedef Name<struct name_env_replaced_by_version> env_replaced_by_version;
        typedef Name<struct name_env_replacing_ids> env_replacing_ids;
        typedef Name<struct name_env_replacing_versions> env_replacing_versions;
        typedef Name<struct name_env_t> env_t;
        typedef Name<struct name_env_use> env_use;
        typedef Name<struct name_env_use_expand> env_use_expand;
        typedef Name<struct name_env_use_expand_hidden> env_use_expand_hidden;
        typedef Name<struct name_env_use_expand_implicit> env_use_expand_implicit;
        typedef Name<struct name_env_use_expand_unprefixed> env_use_expand_unprefixed;
        typedef Name<struct name_env_use_expand_values_part> env_use_expand_values_part;
        typedef Name<struct name_exported_name> exported_name;
        typedef Name<struct name_f_function_prefix> f_function_prefix;
        typedef Name<struct name_failure_is_fatal> failure_is_fatal;
        typedef Name<struct name_fancy_test_flag> fancy_test_flag;
        typedef Name<struct name_fix_mtimes> fix_mtimes;
        typedef Name<struct name_flat_list_index> flat_list_index;
        typedef Name<struct name_fs_location_description> fs_location_description;
        typedef Name<struct name_fs_location_name> fs_location_name;
        typedef Name<struct name_fs_merger_options> fs_merger_options;
        typedef Name<struct name_general_author> general_author;
        typedef Name<struct name_general_date> general_date;
        typedef Name<struct name_general_description> general_description;
        typedef Name<struct name_general_lang> general_lang;
        typedef Name<struct name_general_note> general_note;
        typedef Name<struct name_general_token> general_token;
        typedef Name<struct name_general_url> general_url;
        typedef Name<struct name_generated_from> generated_from;
        typedef Name<struct name_generated_time> generated_time;
        typedef Name<struct name_generated_using> generated_using;
        typedef Name<struct name_has_expensive_tests> has_expensive_tests;
        typedef Name<struct name_has_optional_tests> has_optional_tests;
        typedef Name<struct name_has_recommended_tests> has_recommended_tests;
        typedef Name<struct name_homepage> homepage;
        typedef Name<struct name_ignore_pivot_env_functions> ignore_pivot_env_functions;
        typedef Name<struct name_ignore_pivot_env_variables> ignore_pivot_env_variables;
        typedef Name<struct name_inherited> inherited;
        typedef Name<struct name_is_pbin> is_pbin;
        typedef Name<struct name_iuse> iuse;
        typedef Name<struct name_iuse_effective> iuse_effective;
        typedef Name<struct name_iuse_flag_parse_options> iuse_flag_parse_options;
        typedef Name<struct name_keywords> keywords;
        typedef Name<struct name_licence_last_checked> licence_last_checked;
        typedef Name<struct name_license> license;
        typedef Name<struct name_load_modules> load_modules;
        typedef Name<struct name_long_description> long_description;
        typedef Name<struct name_merger_options> merger_options;
        typedef Name<struct name_metadata_key> metadata_key;
        typedef Name<struct name_minimum_flat_list_size> minimum_flat_list_size;
        typedef Name<struct name_myoptions> myoptions;
        typedef Name<struct name_myoptions_number_selected> myoptions_number_selected;
        typedef Name<struct name_myoptions_number_selected_at_least_one> myoptions_number_selected_at_least_one;
        typedef Name<struct name_myoptions_number_selected_at_most_one> myoptions_number_selected_at_most_one;
        typedef Name<struct name_myoptions_number_selected_exactly_one> myoptions_number_selected_exactly_one;
        typedef Name<struct name_myoptions_presumed> myoptions_presumed;
        typedef Name<struct name_myoptions_requires> myoptions_requires;
        typedef Name<struct name_must_not_change_after_source_variables> must_not_change_after_source_variables;
        typedef Name<struct name_must_not_change_variables> must_not_change_variables;
        typedef Name<struct name_must_not_set_vars_starting_with> must_not_set_vars_starting_with;
        typedef Name<struct name_name> name;
        typedef Name<struct name_no_s_workdir_fallback> no_s_workdir_fallback;
        typedef Name<struct name_no_slot_or_repo> no_slot_or_repo;
        typedef Name<struct name_non_empty_variables> non_empty_variables;
        typedef Name<struct name_package_dep_spec_parse_options> package_dep_spec_parse_options;
        typedef Name<struct name_pdepend> pdepend;
        typedef Name<struct name_permitted_directories> permitted_directories;
        typedef Name<struct name_pipe_commands> pipe_commands;
        typedef Name<struct name_profile_iuse_injection> profile_iuse_injection;
        typedef Name<struct name_properties> properties;
        typedef Name<struct name_rdepend_defaults_to_depend> rdepend_defaults_to_depend;
        typedef Name<struct name_remote_ids> remote_ids;
        typedef Name<struct name_require_use_expand_in_iuse> require_use_expand_in_iuse;
        typedef Name<struct name_required_use> required_use;
        typedef Name<struct name_restrict_fetch> restrict_fetch;
        typedef Name<struct name_restrict_mirror> restrict_mirror;
        typedef Name<struct name_restrict_primaryuri> restrict_primaryuri;
        typedef Name<struct name_restrictions> restrictions;
        typedef Name<struct name_run_depend> run_depend;
        typedef Name<struct name_save_base_variables> save_base_variables;
        typedef Name<struct name_save_unmodifiable_variables> save_unmodifiable_variables;
        typedef Name<struct name_save_variables> save_variables;
        typedef Name<struct name_scm_revision> scm_revision;
        typedef Name<struct name_shell_options> shell_options;
        typedef Name<struct name_short_description> short_description;
        typedef Name<struct name_slot> slot;
        typedef Name<struct name_source_merged_variables> source_merged_variables;
        typedef Name<struct name_src_uri> src_uri;
        typedef Name<struct name_suggestions_group_name> suggestions_group_name;
        typedef Name<struct name_support_eclasses> support_eclasses;
        typedef Name<struct name_support_exlibs> support_exlibs;
        typedef Name<struct name_supported> supported;
        typedef Name<struct name_system_implicit> system_implicit;
        typedef Name<struct name_tools_options> tools_options;
        typedef Name<struct name_unpack_fix_permissions> unpack_fix_permissions;
        typedef Name<struct name_unpack_suffixes> unpack_suffixes;
        typedef Name<struct name_unpack_unrecognised_is_fatal> unpack_unrecognised_is_fatal;
        typedef Name<struct name_upstream_changelog> upstream_changelog;
        typedef Name<struct name_upstream_documentation> upstream_documentation;
        typedef Name<struct name_upstream_release_notes> upstream_release_notes;
        typedef Name<struct name_uri_labels> uri_labels;
        typedef Name<struct name_use> use;
        typedef Name<struct name_use_expand> use_expand;
        typedef Name<struct name_use_expand_hidden> use_expand_hidden;
        typedef Name<struct name_use_expand_separator> use_expand_separator;
        typedef Name<struct name_use_with_enable_empty_third_argument> use_with_enable_empty_third_argument;
        typedef Name<struct name_userpriv_cannot_use_root> userpriv_cannot_use_root;
        typedef Name<struct name_utility_path_suffixes> utility_path_suffixes;
        typedef Name<struct name_vdb_from_env_unless_empty_variables> vdb_from_env_unless_empty_variables;
        typedef Name<struct name_vdb_from_env_variables> vdb_from_env_variables;
        typedef Name<struct name_version_spec_options> version_spec_options;
        typedef Name<struct name_want_portage_emulation_vars> want_portage_emulation_vars;
    }

    namespace erepository
    {
        class PALUDIS_VISIBLE EAPILabels
        {
            private:
                Pimp<EAPILabels> _imp;

            public:
                EAPILabels(const std::string &);
                EAPILabels(const EAPILabels &);
                ~EAPILabels();

                const std::string class_for_label(const std::string &) const PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        /**
         * Thrown if an EAPI configuration is broken.
         *
         * \see EAPI
         * \ingroup grpeapi
         * \ingroup grpexceptions
         * \nosubgrouping
         */
        class PALUDIS_VISIBLE EAPIConfigurationError :
            public ConfigurationError
        {
            public:
                EAPIConfigurationError(const std::string &) throw ();
        };

        /**
         * Holds information on recognised EAPIs.
         *
         * \see EAPI
         * \ingroup grpeapi
         * \nosubgrouping
         */
        class PALUDIS_VISIBLE EAPIData :
            public Singleton<EAPIData>
        {
            friend class Singleton<EAPIData>;

            private:
                Pimp<EAPIData> _imp;

                EAPIData();
                ~EAPIData();

            public:
                /**
                 * Make an EAPI.
                 */
                std::shared_ptr<const EAPI> eapi_from_string(const std::string &) const;

                /**
                 * Make the unknown EAPI.
                 */
                std::shared_ptr<const EAPI> unknown_eapi() const;
        };

        struct EAPI
        {
            NamedValue<n::exported_name, std::string> exported_name;
            NamedValue<n::name, std::string> name;
            NamedValue<n::supported, std::shared_ptr<const SupportedEAPI> > supported;
        };

        struct SupportedEAPI
        {
            NamedValue<n::allow_tokens_in_mask_files, bool> allow_tokens_in_mask_files;
            NamedValue<n::annotations, std::shared_ptr<const EAPIAnnotations> > annotations;
            NamedValue<n::breaks_portage, bool> breaks_portage;
            NamedValue<n::can_be_pbin, bool> can_be_pbin;
            NamedValue<n::choices_options, std::shared_ptr<const EAPIChoicesOptions> > choices_options;
            NamedValue<n::dependency_labels, std::shared_ptr<const EAPILabels> > dependency_labels;
            NamedValue<n::dependency_spec_tree_parse_options, erepository::DependencySpecTreeParseOptions> dependency_spec_tree_parse_options;
            NamedValue<n::ebuild_environment_variables, std::shared_ptr<const EAPIEbuildEnvironmentVariables> > ebuild_environment_variables;
            NamedValue<n::ebuild_metadata_variables, std::shared_ptr<const EAPIEbuildMetadataVariables> > ebuild_metadata_variables;
            NamedValue<n::ebuild_options, std::shared_ptr<const EAPIEbuildOptions> > ebuild_options;
            NamedValue<n::ebuild_phases, std::shared_ptr<const EAPIEbuildPhases> > ebuild_phases;
            NamedValue<n::fs_merger_options, FSMergerOptions> fs_merger_options;
            NamedValue<n::is_pbin, bool> is_pbin;
            NamedValue<n::iuse_flag_parse_options, IUseFlagParseOptions> iuse_flag_parse_options;
            NamedValue<n::merger_options, MergerOptions> merger_options;
            NamedValue<n::package_dep_spec_parse_options, ELikePackageDepSpecOptions> package_dep_spec_parse_options;
            NamedValue<n::permitted_directories, std::string> permitted_directories;
            NamedValue<n::pipe_commands, std::shared_ptr<const EAPIPipeCommands> > pipe_commands;
            NamedValue<n::tools_options, std::shared_ptr<const EAPIToolsOptions> > tools_options;
            NamedValue<n::uri_labels, std::shared_ptr<const EAPILabels> > uri_labels;
            NamedValue<n::userpriv_cannot_use_root, bool> userpriv_cannot_use_root;
            NamedValue<n::version_spec_options, VersionSpecOptions> version_spec_options;
        };

        struct EAPIChoicesOptions
        {
            NamedValue<n::fancy_test_flag, std::string> fancy_test_flag;
            NamedValue<n::has_expensive_tests, bool> has_expensive_tests;
            NamedValue<n::has_optional_tests, bool> has_optional_tests;
            NamedValue<n::has_recommended_tests, bool> has_recommended_tests;
            NamedValue<n::profile_iuse_injection, bool> profile_iuse_injection;
            NamedValue<n::use_expand_separator, char> use_expand_separator;
        };

        struct EAPIEbuildEnvironmentVariables
        {
            NamedValue<n::description_choices, std::string> description_choices;
            NamedValue<n::description_use, std::string> description_use;
            NamedValue<n::env_a, std::string> env_a;
            NamedValue<n::env_aa, std::string> env_aa;
            NamedValue<n::env_accept_license, std::string> env_accept_license;
            NamedValue<n::env_arch, std::string> env_arch;
            NamedValue<n::env_d, std::string> env_d;
            NamedValue<n::env_distdir, std::string> env_distdir;
            NamedValue<n::env_ebuild_phase, std::string> env_ebuild_phase;
            NamedValue<n::env_ed, std::string> env_ed;
            NamedValue<n::env_eprefix, std::string> env_eprefix;
            NamedValue<n::env_eroot, std::string> env_eroot;
            NamedValue<n::env_filesdir, std::string> env_filesdir;
            NamedValue<n::env_iuse_implicit, std::string> env_iuse_implicit;
            NamedValue<n::env_jobs, std::string> env_jobs;
            NamedValue<n::env_kv, std::string> env_kv;
            NamedValue<n::env_merge_type, std::string> env_merge_type;
            NamedValue<n::env_p, std::string> env_p;
            NamedValue<n::env_pf, std::string> env_pf;
            NamedValue<n::env_portdir, std::string> env_portdir;
            NamedValue<n::env_replaced_by_id, std::string> env_replaced_by_id;
            NamedValue<n::env_replaced_by_version, std::string> env_replaced_by_version;
            NamedValue<n::env_replacing_ids, std::string> env_replacing_ids;
            NamedValue<n::env_replacing_versions, std::string> env_replacing_versions;
            NamedValue<n::env_t, std::string> env_t;
            NamedValue<n::env_use, std::string> env_use;
            NamedValue<n::env_use_expand, std::string> env_use_expand;
            NamedValue<n::env_use_expand_hidden, std::string> env_use_expand_hidden;
            NamedValue<n::env_use_expand_implicit, std::string> env_use_expand_implicit;
            NamedValue<n::env_use_expand_unprefixed, std::string> env_use_expand_unprefixed;
            NamedValue<n::env_use_expand_values_part, std::string> env_use_expand_values_part;
        };

        struct EAPIMetadataVariable
        {
            NamedValue<n::description, std::string> description;
            NamedValue<n::flat_list_index, int> flat_list_index;
            NamedValue<n::name, std::string> name;
        };

        struct EAPIEbuildMetadataVariables
        {
            NamedValue<n::bugs_to, std::shared_ptr<const EAPIMetadataVariable> > bugs_to;
            NamedValue<n::build_depend, std::shared_ptr<const EAPIMetadataVariable> > build_depend;
            NamedValue<n::defined_phases, std::shared_ptr<const EAPIMetadataVariable> > defined_phases;
            NamedValue<n::dependencies, std::shared_ptr<const EAPIMetadataVariable> > dependencies;
            NamedValue<n::eapi, std::shared_ptr<const EAPIMetadataVariable> > eapi;
            NamedValue<n::generated_from, std::shared_ptr<const EAPIMetadataVariable> > generated_from;
            NamedValue<n::generated_time, std::shared_ptr<const EAPIMetadataVariable> > generated_time;
            NamedValue<n::generated_using, std::shared_ptr<const EAPIMetadataVariable> > generated_using;
            NamedValue<n::homepage, std::shared_ptr<const EAPIMetadataVariable> > homepage;
            NamedValue<n::inherited, std::shared_ptr<const EAPIMetadataVariable> > inherited;
            NamedValue<n::iuse, std::shared_ptr<const EAPIMetadataVariable> > iuse;
            NamedValue<n::iuse_effective, std::shared_ptr<const EAPIMetadataVariable> > iuse_effective;
            NamedValue<n::keywords, std::shared_ptr<const EAPIMetadataVariable> > keywords;
            NamedValue<n::license, std::shared_ptr<const EAPIMetadataVariable> > license;
            NamedValue<n::long_description, std::shared_ptr<const EAPIMetadataVariable> > long_description;
            NamedValue<n::minimum_flat_list_size, int> minimum_flat_list_size;
            NamedValue<n::myoptions, std::shared_ptr<const EAPIMetadataVariable> > myoptions;
            NamedValue<n::pdepend, std::shared_ptr<const EAPIMetadataVariable> > pdepend;
            NamedValue<n::properties, std::shared_ptr<const EAPIMetadataVariable> > properties;
            NamedValue<n::remote_ids, std::shared_ptr<const EAPIMetadataVariable> > remote_ids;
            NamedValue<n::required_use, std::shared_ptr<const EAPIMetadataVariable> > required_use;
            NamedValue<n::restrictions, std::shared_ptr<const EAPIMetadataVariable> > restrictions;
            NamedValue<n::run_depend, std::shared_ptr<const EAPIMetadataVariable> > run_depend;
            NamedValue<n::scm_revision, std::shared_ptr<const EAPIMetadataVariable> > scm_revision;
            NamedValue<n::short_description, std::shared_ptr<const EAPIMetadataVariable> > short_description;
            NamedValue<n::slot, std::shared_ptr<const EAPIMetadataVariable> > slot;
            NamedValue<n::src_uri, std::shared_ptr<const EAPIMetadataVariable> > src_uri;
            NamedValue<n::upstream_changelog, std::shared_ptr<const EAPIMetadataVariable> > upstream_changelog;
            NamedValue<n::upstream_documentation, std::shared_ptr<const EAPIMetadataVariable> > upstream_documentation;
            NamedValue<n::upstream_release_notes, std::shared_ptr<const EAPIMetadataVariable> > upstream_release_notes;
            NamedValue<n::use, std::shared_ptr<const EAPIMetadataVariable> > use;
            NamedValue<n::use_expand, std::shared_ptr<const EAPIMetadataVariable> > use_expand;
            NamedValue<n::use_expand_hidden, std::shared_ptr<const EAPIMetadataVariable> > use_expand_hidden;
        };

        struct EAPIEbuildOptions
        {
            NamedValue<n::binary_from_env_variables, std::string> binary_from_env_variables;
            NamedValue<n::bracket_merged_variables, std::string> bracket_merged_variables;
            NamedValue<n::bracket_merged_variables_annotatable, std::string> bracket_merged_variables_annotatable;
            NamedValue<n::bracket_merged_variables_annotation, std::string> bracket_merged_variables_annotation;
            NamedValue<n::directory_if_exists_variables, std::string> directory_if_exists_variables;
            NamedValue<n::directory_variables, std::string> directory_variables;
            NamedValue<n::ebuild_functions, std::string> ebuild_functions;
            NamedValue<n::ebuild_module_suffixes, std::string> ebuild_module_suffixes;
            NamedValue<n::ebuild_must_not_set_variables, std::string> ebuild_must_not_set_variables;
            NamedValue<n::eclass_must_not_set_variables, std::string> eclass_must_not_set_variables;
            NamedValue<n::f_function_prefix, std::string> f_function_prefix;
            NamedValue<n::fix_mtimes, bool> fix_mtimes;
            NamedValue<n::fs_location_description, std::string> fs_location_description;
            NamedValue<n::fs_location_name, std::string> fs_location_name;
            NamedValue<n::ignore_pivot_env_functions, std::string> ignore_pivot_env_functions;
            NamedValue<n::ignore_pivot_env_variables, std::string> ignore_pivot_env_variables;
            NamedValue<n::load_modules, std::string> load_modules;
            NamedValue<n::must_not_change_after_source_variables, std::string> must_not_change_after_source_variables;
            NamedValue<n::must_not_change_variables, std::string> must_not_change_variables;
            NamedValue<n::must_not_set_vars_starting_with, std::string> must_not_set_vars_starting_with;
            NamedValue<n::no_s_workdir_fallback, bool> no_s_workdir_fallback;
            NamedValue<n::non_empty_variables, std::string> non_empty_variables;
            NamedValue<n::rdepend_defaults_to_depend, bool> rdepend_defaults_to_depend;
            NamedValue<n::require_use_expand_in_iuse, bool> require_use_expand_in_iuse;
            NamedValue<n::restrict_fetch, std::shared_ptr<Set<std::string> > > restrict_fetch;
            NamedValue<n::restrict_mirror, std::shared_ptr<Set<std::string> > > restrict_mirror;
            NamedValue<n::restrict_primaryuri, std::shared_ptr<Set<std::string> > > restrict_primaryuri;
            NamedValue<n::save_base_variables, std::string> save_base_variables;
            NamedValue<n::save_unmodifiable_variables, std::string> save_unmodifiable_variables;
            NamedValue<n::save_variables, std::string> save_variables;
            NamedValue<n::shell_options, std::string> shell_options;
            NamedValue<n::source_merged_variables, std::string> source_merged_variables;
            NamedValue<n::support_eclasses, bool> support_eclasses;
            NamedValue<n::support_exlibs, bool> support_exlibs;
            NamedValue<n::utility_path_suffixes, std::string> utility_path_suffixes;
            NamedValue<n::vdb_from_env_unless_empty_variables, std::string> vdb_from_env_unless_empty_variables;
            NamedValue<n::vdb_from_env_variables, std::string> vdb_from_env_variables;
            NamedValue<n::want_portage_emulation_vars, bool> want_portage_emulation_vars;
        };

        struct EAPIEbuildPhases
        {
            NamedValue<n::ebuild_bad_options, std::string> ebuild_bad_options;
            NamedValue<n::ebuild_config, std::string> ebuild_config;
            NamedValue<n::ebuild_fetch_extra, std::string> ebuild_fetch_extra;
            NamedValue<n::ebuild_info, std::string> ebuild_info;
            NamedValue<n::ebuild_install, std::string> ebuild_install;
            NamedValue<n::ebuild_metadata, std::string> ebuild_metadata;
            NamedValue<n::ebuild_new_upgrade_phase_order, bool> ebuild_new_upgrade_phase_order;
            NamedValue<n::ebuild_nofetch, std::string> ebuild_nofetch;
            NamedValue<n::ebuild_pretend, std::string> ebuild_pretend;
            NamedValue<n::ebuild_uninstall, std::string> ebuild_uninstall;
            NamedValue<n::ebuild_variable, std::string> ebuild_variable;
        };

        struct EAPIToolsOptions
        {
            NamedValue<n::best_has_version_host_root, bool> best_has_version_host_root;
            NamedValue<n::dodoc_r, bool> dodoc_r;
            NamedValue<n::doins_symlink, bool> doins_symlink;
            NamedValue<n::doman_lang_filenames, bool> doman_lang_filenames;
            NamedValue<n::doman_lang_filenames_overrides, bool> doman_lang_filenames_overrides;
            NamedValue<n::dosym_mkdir, bool> dosym_mkdir;
            NamedValue<n::econf_extra_options, std::string> econf_extra_options;
            NamedValue<n::econf_extra_options_help_dependent, std::string> econf_extra_options_help_dependent;
            NamedValue<n::failure_is_fatal, bool> failure_is_fatal;
            NamedValue<n::unpack_fix_permissions, bool> unpack_fix_permissions;
            NamedValue<n::unpack_suffixes, std::string> unpack_suffixes;
            NamedValue<n::unpack_unrecognised_is_fatal, bool> unpack_unrecognised_is_fatal;
            NamedValue<n::use_with_enable_empty_third_argument, bool> use_with_enable_empty_third_argument;
        };

        struct EAPIPipeCommands
        {
            NamedValue<n::no_slot_or_repo, bool> no_slot_or_repo;
        };

        struct EAPIAnnotations
        {
            NamedValue<n::blocker_resolution, std::string> blocker_resolution;
            NamedValue<n::blocker_resolution_manual, std::string> blocker_resolution_manual;
            NamedValue<n::blocker_resolution_uninstall_blocked_after, std::string> blocker_resolution_uninstall_blocked_after;
            NamedValue<n::blocker_resolution_uninstall_blocked_before, std::string> blocker_resolution_uninstall_blocked_before;
            NamedValue<n::blocker_resolution_upgrade_blocked_before, std::string> blocker_resolution_upgrade_blocked_before;
            NamedValue<n::general_author, std::string> general_author;
            NamedValue<n::general_date, std::string> general_date;
            NamedValue<n::general_description, std::string> general_description;
            NamedValue<n::general_lang, std::string> general_lang;
            NamedValue<n::general_note, std::string> general_note;
            NamedValue<n::general_token, std::string> general_token;
            NamedValue<n::general_url, std::string> general_url;
            NamedValue<n::licence_last_checked, std::string> licence_last_checked;
            NamedValue<n::myoptions_number_selected, std::string> myoptions_number_selected;
            NamedValue<n::myoptions_number_selected_at_least_one, std::string> myoptions_number_selected_at_least_one;
            NamedValue<n::myoptions_number_selected_at_most_one, std::string> myoptions_number_selected_at_most_one;
            NamedValue<n::myoptions_number_selected_exactly_one, std::string> myoptions_number_selected_exactly_one;
            NamedValue<n::myoptions_presumed, std::string> myoptions_presumed;
            NamedValue<n::myoptions_requires, std::string> myoptions_requires;
            NamedValue<n::suggestions_group_name, std::string> suggestions_group_name;
            NamedValue<n::system_implicit, std::string> system_implicit;
        };
    }
}

#endif
