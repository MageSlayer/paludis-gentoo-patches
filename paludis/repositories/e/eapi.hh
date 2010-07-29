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
#include <memory>

namespace paludis
{
    namespace n
    {
        typedef Name<struct annotations_name> annotations;
        typedef Name<struct binary_from_env_variables_name> binary_from_env_variables;
        typedef Name<struct bracket_merged_variables_name> bracket_merged_variables;
        typedef Name<struct bracket_merged_variables_annotatable_name> bracket_merged_variables_annotatable;
        typedef Name<struct bracket_merged_variables_annotation_name> bracket_merged_variables_annotation;
        typedef Name<struct breaks_portage_name> breaks_portage;
        typedef Name<struct bugs_to_name> bugs_to;
        typedef Name<struct build_depend_name> build_depend;
        typedef Name<struct can_be_pbin_name> can_be_pbin;
        typedef Name<struct choices_options_name> choices_options;
        typedef Name<struct defined_phases_name> defined_phases;
        typedef Name<struct dependencies_name> dependencies;
        typedef Name<struct dependency_labels_name> dependency_labels;
        typedef Name<struct dependency_spec_tree_parse_options_name> dependency_spec_tree_parse_options;
        typedef Name<struct description_name> description;
        typedef Name<struct description_choices_name> description_choices;
        typedef Name<struct description_use_name> description_use;
        typedef Name<struct directory_if_exists_variables_name> directory_if_exists_variables;
        typedef Name<struct directory_variables_name> directory_variables;
        typedef Name<struct dodoc_r_name> dodoc_r;
        typedef Name<struct doins_symlink_name> doins_symlink;
        typedef Name<struct doman_lang_filenames_name> doman_lang_filenames;
        typedef Name<struct dosym_mkdir_name> dosym_mkdir;
        typedef Name<struct eapi_name> eapi;
        typedef Name<struct ebuild_bad_options_name> ebuild_bad_options;
        typedef Name<struct ebuild_config_name> ebuild_config;
        typedef Name<struct ebuild_environment_variables_name> ebuild_environment_variables;
        typedef Name<struct ebuild_fetch_extra_name> ebuild_fetch_extra;
        typedef Name<struct ebuild_functions_name> ebuild_functions;
        typedef Name<struct ebuild_info_name> ebuild_info;
        typedef Name<struct ebuild_install_name> ebuild_install;
        typedef Name<struct ebuild_metadata_name> ebuild_metadata;
        typedef Name<struct ebuild_metadata_variables_name> ebuild_metadata_variables;
        typedef Name<struct ebuild_module_suffixes_name> ebuild_module_suffixes;
        typedef Name<struct ebuild_must_not_set_variables_name> ebuild_must_not_set_variables;
        typedef Name<struct ebuild_nofetch_name> ebuild_nofetch;
        typedef Name<struct ebuild_options_name> ebuild_options;
        typedef Name<struct ebuild_phases_name> ebuild_phases;
        typedef Name<struct ebuild_pretend_name> ebuild_pretend;
        typedef Name<struct ebuild_uninstall_name> ebuild_uninstall;
        typedef Name<struct ebuild_variable_name> ebuild_variable;
        typedef Name<struct ebuild_new_upgrade_phase_order_name> ebuild_new_upgrade_phase_order;
        typedef Name<struct econf_extra_options_name> econf_extra_options;
        typedef Name<struct eclass_must_not_set_variables_name> eclass_must_not_set_variables;
        typedef Name<struct env_a_name> env_a;
        typedef Name<struct env_aa_name> env_aa;
        typedef Name<struct env_accept_keywords_name> env_accept_keywords;
        typedef Name<struct env_arch_name> env_arch;
        typedef Name<struct env_d_name> env_d;
        typedef Name<struct env_distdir_name> env_distdir;
        typedef Name<struct env_ebuild_phase_name> env_ebuild_phase;
        typedef Name<struct env_ed_name> env_ed;
        typedef Name<struct env_eprefix_name> env_eprefix;
        typedef Name<struct env_eroot_name> env_eroot;
        typedef Name<struct env_filesdir_name> env_filesdir;
        typedef Name<struct env_iuse_implicit_name> env_iuse_implicit;
        typedef Name<struct env_jobs_name> env_jobs;
        typedef Name<struct env_kv_name> env_kv;
        typedef Name<struct env_p_name> env_p;
        typedef Name<struct env_pf_name> env_pf;
        typedef Name<struct env_portdir_name> env_portdir;
        typedef Name<struct env_replaced_by_id_name> env_replaced_by_id;
        typedef Name<struct env_replaced_by_version_name> env_replaced_by_version;
        typedef Name<struct env_replacing_ids_name> env_replacing_ids;
        typedef Name<struct env_replacing_versions_name> env_replacing_versions;
        typedef Name<struct env_t_name> env_t;
        typedef Name<struct env_use_name> env_use;
        typedef Name<struct env_use_expand_name> env_use_expand;
        typedef Name<struct env_use_expand_hidden_name> env_use_expand_hidden;
        typedef Name<struct env_use_expand_implicit_name> env_use_expand_implicit;
        typedef Name<struct env_use_expand_unprefixed_name> env_use_expand_unprefixed;
        typedef Name<struct env_use_expand_values_part_name> env_use_expand_values_part;
        typedef Name<struct exported_name_name> exported_name;
        typedef Name<struct f_function_prefix_name> f_function_prefix;
        typedef Name<struct failure_is_fatal_name> failure_is_fatal;
        typedef Name<struct fancy_test_flag_name> fancy_test_flag;
        typedef Name<struct fix_mtimes_name> fix_mtimes;
        typedef Name<struct flat_list_index_name> flat_list_index;
        typedef Name<struct generated_from_name> generated_from;
        typedef Name<struct generated_time_name> generated_time;
        typedef Name<struct generated_using_name> generated_using;
        typedef Name<struct has_expensive_tests_name> has_expensive_tests;
        typedef Name<struct has_optional_tests_name> has_optional_tests;
        typedef Name<struct has_recommended_tests_name> has_recommended_tests;
        typedef Name<struct homepage_name> homepage;
        typedef Name<struct ignore_pivot_env_functions_name> ignore_pivot_env_functions;
        typedef Name<struct ignore_pivot_env_variables_name> ignore_pivot_env_variables;
        typedef Name<struct inherited_name> inherited;
        typedef Name<struct is_pbin_name> is_pbin;
        typedef Name<struct iuse_name> iuse;
        typedef Name<struct iuse_effective_name> iuse_effective;
        typedef Name<struct iuse_flag_parse_options_name> iuse_flag_parse_options;
        typedef Name<struct keywords_name> keywords;
        typedef Name<struct license_name> license;
        typedef Name<struct load_modules_name> load_modules;
        typedef Name<struct long_description_name> long_description;
        typedef Name<struct merger_options_name> merger_options;
        typedef Name<struct metadata_key_name> metadata_key;
        typedef Name<struct minimum_flat_list_size_name> minimum_flat_list_size;
        typedef Name<struct myoptions_name> myoptions;
        typedef Name<struct myoptions_description_name> myoptions_description;
        typedef Name<struct myoptions_number_selected_name> myoptions_number_selected;
        typedef Name<struct myoptions_number_selected_at_least_one_name> myoptions_number_selected_at_least_one;
        typedef Name<struct myoptions_number_selected_at_most_one_name> myoptions_number_selected_at_most_one;
        typedef Name<struct myoptions_number_selected_exactly_one_name> myoptions_number_selected_exactly_one;
        typedef Name<struct myoptions_requires_name> myoptions_requires;
        typedef Name<struct must_not_change_after_source_variables_name> must_not_change_after_source_variables;
        typedef Name<struct must_not_change_variables_name> must_not_change_variables;
        typedef Name<struct must_not_set_vars_starting_with_name> must_not_set_vars_starting_with;
        typedef Name<struct name_name> name;
        typedef Name<struct no_s_workdir_fallback_name> no_s_workdir_fallback;
        typedef Name<struct no_slot_or_repo_name> no_slot_or_repo;
        typedef Name<struct non_empty_variables_name> non_empty_variables;
        typedef Name<struct package_dep_spec_parse_options_name> package_dep_spec_parse_options;
        typedef Name<struct pdepend_name> pdepend;
        typedef Name<struct pipe_commands_name> pipe_commands;
        typedef Name<struct profile_iuse_injection_name> profile_iuse_injection;
        typedef Name<struct properties_name> properties;
        typedef Name<struct provide_name> provide;
        typedef Name<struct rdepend_defaults_to_depend_name> rdepend_defaults_to_depend;
        typedef Name<struct remote_ids_name> remote_ids;
        typedef Name<struct require_use_expand_in_iuse_name> require_use_expand_in_iuse;
        typedef Name<struct restrict_fetch_name> restrict_fetch;
        typedef Name<struct restrict_mirror_name> restrict_mirror;
        typedef Name<struct restrict_primaryuri_name> restrict_primaryuri;
        typedef Name<struct restrictions_name> restrictions;
        typedef Name<struct rewrite_virtuals_name> rewrite_virtuals;
        typedef Name<struct run_depend_name> run_depend;
        typedef Name<struct save_base_variables_name> save_base_variables;
        typedef Name<struct save_unmodifiable_variables_name> save_unmodifiable_variables;
        typedef Name<struct save_variables_name> save_variables;
        typedef Name<struct shell_options_name> shell_options;
        typedef Name<struct short_description_name> short_description;
        typedef Name<struct slot_name> slot;
        typedef Name<struct source_merged_variables_name> source_merged_variables;
        typedef Name<struct src_uri_name> src_uri;
        typedef Name<struct support_eclasses_name> support_eclasses;
        typedef Name<struct support_exlibs_name> support_exlibs;
        typedef Name<struct supported_name> supported;
        typedef Name<struct tools_options_name> tools_options;
        typedef Name<struct unpack_fix_permissions_name> unpack_fix_permissions;
        typedef Name<struct unpack_suffixes_name> unpack_suffixes;
        typedef Name<struct unpack_unrecognised_is_fatal_name> unpack_unrecognised_is_fatal;
        typedef Name<struct upstream_changelog_name> upstream_changelog;
        typedef Name<struct upstream_documentation_name> upstream_documentation;
        typedef Name<struct upstream_release_notes_name> upstream_release_notes;
        typedef Name<struct uri_labels_name> uri_labels;
        typedef Name<struct use_name> use;
        typedef Name<struct use_expand_name> use_expand;
        typedef Name<struct use_expand_hidden_name> use_expand_hidden;
        typedef Name<struct use_expand_separator_name> use_expand_separator;
        typedef Name<struct userpriv_cannot_use_root_name> userpriv_cannot_use_root;
        typedef Name<struct utility_path_suffixes_name> utility_path_suffixes;
        typedef Name<struct vdb_from_env_unless_empty_variables_name> vdb_from_env_unless_empty_variables;
        typedef Name<struct vdb_from_env_variables_name> vdb_from_env_variables;
        typedef Name<struct version_spec_options_name> version_spec_options;
        typedef Name<struct want_portage_emulation_vars_name> want_portage_emulation_vars;
    }

    namespace erepository
    {
        class PALUDIS_VISIBLE EAPILabels :
            private Pimp<EAPILabels>
        {
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
            private Pimp<EAPIData>,
            public Singleton<EAPIData>
        {
            friend class Singleton<EAPIData>;

            private:
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
            NamedValue<n::is_pbin, bool> is_pbin;
            NamedValue<n::iuse_flag_parse_options, IUseFlagParseOptions> iuse_flag_parse_options;
            NamedValue<n::merger_options, MergerOptions> merger_options;
            NamedValue<n::package_dep_spec_parse_options, ELikePackageDepSpecOptions> package_dep_spec_parse_options;
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
            NamedValue<n::env_accept_keywords, std::string> env_accept_keywords;
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
            NamedValue<n::provide, std::shared_ptr<const EAPIMetadataVariable> > provide;
            NamedValue<n::remote_ids, std::shared_ptr<const EAPIMetadataVariable> > remote_ids;
            NamedValue<n::restrictions, std::shared_ptr<const EAPIMetadataVariable> > restrictions;
            NamedValue<n::run_depend, std::shared_ptr<const EAPIMetadataVariable> > run_depend;
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
            NamedValue<n::dodoc_r, bool> dodoc_r;
            NamedValue<n::doins_symlink, bool> doins_symlink;
            NamedValue<n::doman_lang_filenames, bool> doman_lang_filenames;
            NamedValue<n::dosym_mkdir, bool> dosym_mkdir;
            NamedValue<n::econf_extra_options, std::string> econf_extra_options;
            NamedValue<n::failure_is_fatal, bool> failure_is_fatal;
            NamedValue<n::unpack_fix_permissions, bool> unpack_fix_permissions;
            NamedValue<n::unpack_suffixes, std::string> unpack_suffixes;
            NamedValue<n::unpack_unrecognised_is_fatal, bool> unpack_unrecognised_is_fatal;
        };

        struct EAPIPipeCommands
        {
            NamedValue<n::no_slot_or_repo, bool> no_slot_or_repo;
            NamedValue<n::rewrite_virtuals, bool> rewrite_virtuals;
        };

        struct EAPIAnnotations
        {
            NamedValue<n::myoptions_description, std::string> myoptions_description;
            NamedValue<n::myoptions_number_selected, std::string> myoptions_number_selected;
            NamedValue<n::myoptions_number_selected_at_least_one, std::string> myoptions_number_selected_at_least_one;
            NamedValue<n::myoptions_number_selected_at_most_one, std::string> myoptions_number_selected_at_most_one;
            NamedValue<n::myoptions_number_selected_exactly_one, std::string> myoptions_number_selected_exactly_one;
            NamedValue<n::myoptions_requires, std::string> myoptions_requires;
        };
    }
#endif
}
