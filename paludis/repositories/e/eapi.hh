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
#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/options.hh>
#include <paludis/util/named_value.hh>
#include <paludis/name.hh>
#include <paludis/dep_spec-fwd.hh>
#include <paludis/merger-fwd.hh>
#include <tr1/memory>

namespace paludis
{
    namespace n
    {
        struct annotations;
        struct binary_from_env_variables;
        struct bracket_merged_variables;
        struct bracket_merged_variables_annotatable;
        struct bracket_merged_variables_annotation;
        struct breaks_portage;
        struct bugs_to;
        struct build_depend;
        struct can_be_pbin;
        struct choices_options;
        struct defined_phases;
        struct dependencies;
        struct dependency_labels;
        struct dependency_spec_tree_parse_options;
        struct description;
        struct description_choices;
        struct description_use;
        struct directory_if_exists_variables;
        struct directory_variables;
        struct dodoc_r;
        struct doins_symlink;
        struct doman_lang_filenames;
        struct dosym_mkdir;
        struct eapi;
        struct ebuild_bad_options;
        struct ebuild_config;
        struct ebuild_environment_variables;
        struct ebuild_fetch_extra;
        struct ebuild_functions;
        struct ebuild_info;
        struct ebuild_install;
        struct ebuild_metadata;
        struct ebuild_metadata_variables;
        struct ebuild_module_suffixes;
        struct ebuild_must_not_set_variables;
        struct ebuild_nofetch;
        struct ebuild_options;
        struct ebuild_phases;
        struct ebuild_pretend;
        struct ebuild_uninstall;
        struct ebuild_variable;
        struct ebuild_new_upgrade_phase_order;
        struct econf_extra_options;
        struct eclass_must_not_set_variables;
        struct env_a;
        struct env_aa;
        struct env_accept_keywords;
        struct env_arch;
        struct env_d;
        struct env_distdir;
        struct env_ebuild_phase;
        struct env_ed;
        struct env_eprefix;
        struct env_eroot;
        struct env_filesdir;
        struct env_iuse_implicit;
        struct env_jobs;
        struct env_kv;
        struct env_p;
        struct env_pf;
        struct env_portdir;
        struct env_replaced_by_id;
        struct env_replaced_by_version;
        struct env_replacing_ids;
        struct env_replacing_versions;
        struct env_t;
        struct env_use;
        struct env_use_expand;
        struct env_use_expand_hidden;
        struct env_use_expand_implicit;
        struct env_use_expand_unprefixed;
        struct env_use_expand_values_part;
        struct exported_name;
        struct f_function_prefix;
        struct failure_is_fatal;
        struct fancy_test_flag;
        struct fix_mtimes;
        struct flat_list_index;
        struct has_expensive_tests;
        struct has_optional_tests;
        struct has_recommended_tests;
        struct homepage;
        struct ignore_pivot_env_functions;
        struct ignore_pivot_env_variables;
        struct inherited;
        struct iuse;
        struct iuse_effective;
        struct iuse_flag_parse_options;
        struct keywords;
        struct license;
        struct load_modules;
        struct long_description;
        struct merger_options;
        struct metadata_key;
        struct minimum_flat_list_size;
        struct myoptions;
        struct myoptions_description;
        struct myoptions_number_selected;
        struct myoptions_number_selected_at_least_one;
        struct myoptions_number_selected_at_most_one;
        struct myoptions_number_selected_exactly_one;
        struct myoptions_requires;
        struct must_not_change_after_source_variables;
        struct must_not_change_variables;
        struct must_not_set_vars_starting_with;
        struct name;
        struct no_s_workdir_fallback;
        struct no_slot_or_repo;
        struct non_empty_variables;
        struct package_dep_spec_parse_options;
        struct pdepend;
        struct pipe_commands;
        struct profile_iuse_injection;
        struct properties;
        struct provide;
        struct rdepend_defaults_to_depend;
        struct remote_ids;
        struct require_use_expand_in_iuse;
        struct restrict_fetch;
        struct restrict_mirror;
        struct restrict_primaryuri;
        struct restrictions;
        struct rewrite_virtuals;
        struct run_depend;
        struct save_base_variables;
        struct save_unmodifiable_variables;
        struct save_variables;
        struct shell_options;
        struct short_description;
        struct slot;
        struct source_merged_variables;
        struct src_uri;
        struct support_eclasses;
        struct support_exlibs;
        struct supported;
        struct tools_options;
        struct unpack_fix_permissions;
        struct unpack_suffixes;
        struct unpack_unrecognised_is_fatal;
        struct upstream_changelog;
        struct upstream_documentation;
        struct upstream_release_notes;
        struct uri_labels;
        struct use;
        struct use_expand;
        struct use_expand_hidden;
        struct use_expand_separator;
        struct userpriv_cannot_use_root;
        struct utility_path_suffixes;
        struct vdb_from_env_unless_empty_variables;
        struct vdb_from_env_variables;
        struct version_spec_options;
        struct want_portage_emulation_vars;
    }

    namespace erepository
    {
        class PALUDIS_VISIBLE EAPILabels :
            private PrivateImplementationPattern<EAPILabels>
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
            private PrivateImplementationPattern<EAPIData>,
            public InstantiationPolicy<EAPIData, instantiation_method::SingletonTag>
        {
            friend class InstantiationPolicy<EAPIData, instantiation_method::SingletonTag>;

            private:
                EAPIData();
                ~EAPIData();

            public:
                /**
                 * Make an EAPI.
                 */
                std::tr1::shared_ptr<const EAPI> eapi_from_string(const std::string &) const;

                /**
                 * Make the unknown EAPI.
                 */
                std::tr1::shared_ptr<const EAPI> unknown_eapi() const;
        };

        struct EAPI
        {
            NamedValue<n::exported_name, std::string> exported_name;
            NamedValue<n::name, std::string> name;
            NamedValue<n::supported, std::tr1::shared_ptr<const SupportedEAPI> > supported;
        };

        struct SupportedEAPI
        {
            NamedValue<n::annotations, std::tr1::shared_ptr<const EAPIAnnotations> > annotations;
            NamedValue<n::breaks_portage, bool> breaks_portage;
            NamedValue<n::can_be_pbin, bool> can_be_pbin;
            NamedValue<n::choices_options, std::tr1::shared_ptr<const EAPIChoicesOptions> > choices_options;
            NamedValue<n::dependency_labels, std::tr1::shared_ptr<const EAPILabels> > dependency_labels;
            NamedValue<n::dependency_spec_tree_parse_options, erepository::DependencySpecTreeParseOptions> dependency_spec_tree_parse_options;
            NamedValue<n::ebuild_environment_variables, std::tr1::shared_ptr<const EAPIEbuildEnvironmentVariables> > ebuild_environment_variables;
            NamedValue<n::ebuild_metadata_variables, std::tr1::shared_ptr<const EAPIEbuildMetadataVariables> > ebuild_metadata_variables;
            NamedValue<n::ebuild_options, std::tr1::shared_ptr<const EAPIEbuildOptions> > ebuild_options;
            NamedValue<n::ebuild_phases, std::tr1::shared_ptr<const EAPIEbuildPhases> > ebuild_phases;
            NamedValue<n::iuse_flag_parse_options, IUseFlagParseOptions> iuse_flag_parse_options;
            NamedValue<n::merger_options, MergerOptions> merger_options;
            NamedValue<n::package_dep_spec_parse_options, ELikePackageDepSpecOptions> package_dep_spec_parse_options;
            NamedValue<n::pipe_commands, std::tr1::shared_ptr<const EAPIPipeCommands> > pipe_commands;
            NamedValue<n::tools_options, std::tr1::shared_ptr<const EAPIToolsOptions> > tools_options;
            NamedValue<n::uri_labels, std::tr1::shared_ptr<const EAPILabels> > uri_labels;
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
            NamedValue<n::bugs_to, std::tr1::shared_ptr<const EAPIMetadataVariable> > bugs_to;
            NamedValue<n::build_depend, std::tr1::shared_ptr<const EAPIMetadataVariable> > build_depend;
            NamedValue<n::defined_phases, std::tr1::shared_ptr<const EAPIMetadataVariable> > defined_phases;
            NamedValue<n::dependencies, std::tr1::shared_ptr<const EAPIMetadataVariable> > dependencies;
            NamedValue<n::eapi, std::tr1::shared_ptr<const EAPIMetadataVariable> > eapi;
            NamedValue<n::homepage, std::tr1::shared_ptr<const EAPIMetadataVariable> > homepage;
            NamedValue<n::inherited, std::tr1::shared_ptr<const EAPIMetadataVariable> > inherited;
            NamedValue<n::iuse, std::tr1::shared_ptr<const EAPIMetadataVariable> > iuse;
            NamedValue<n::iuse_effective, std::tr1::shared_ptr<const EAPIMetadataVariable> > iuse_effective;
            NamedValue<n::keywords, std::tr1::shared_ptr<const EAPIMetadataVariable> > keywords;
            NamedValue<n::license, std::tr1::shared_ptr<const EAPIMetadataVariable> > license;
            NamedValue<n::long_description, std::tr1::shared_ptr<const EAPIMetadataVariable> > long_description;
            NamedValue<n::minimum_flat_list_size, int> minimum_flat_list_size;
            NamedValue<n::myoptions, std::tr1::shared_ptr<const EAPIMetadataVariable> > myoptions;
            NamedValue<n::pdepend, std::tr1::shared_ptr<const EAPIMetadataVariable> > pdepend;
            NamedValue<n::properties, std::tr1::shared_ptr<const EAPIMetadataVariable> > properties;
            NamedValue<n::provide, std::tr1::shared_ptr<const EAPIMetadataVariable> > provide;
            NamedValue<n::remote_ids, std::tr1::shared_ptr<const EAPIMetadataVariable> > remote_ids;
            NamedValue<n::restrictions, std::tr1::shared_ptr<const EAPIMetadataVariable> > restrictions;
            NamedValue<n::run_depend, std::tr1::shared_ptr<const EAPIMetadataVariable> > run_depend;
            NamedValue<n::short_description, std::tr1::shared_ptr<const EAPIMetadataVariable> > short_description;
            NamedValue<n::slot, std::tr1::shared_ptr<const EAPIMetadataVariable> > slot;
            NamedValue<n::src_uri, std::tr1::shared_ptr<const EAPIMetadataVariable> > src_uri;
            NamedValue<n::upstream_changelog, std::tr1::shared_ptr<const EAPIMetadataVariable> > upstream_changelog;
            NamedValue<n::upstream_documentation, std::tr1::shared_ptr<const EAPIMetadataVariable> > upstream_documentation;
            NamedValue<n::upstream_release_notes, std::tr1::shared_ptr<const EAPIMetadataVariable> > upstream_release_notes;
            NamedValue<n::use, std::tr1::shared_ptr<const EAPIMetadataVariable> > use;
            NamedValue<n::use_expand, std::tr1::shared_ptr<const EAPIMetadataVariable> > use_expand;
            NamedValue<n::use_expand_hidden, std::tr1::shared_ptr<const EAPIMetadataVariable> > use_expand_hidden;
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
            NamedValue<n::restrict_fetch, std::tr1::shared_ptr<Set<std::string> > > restrict_fetch;
            NamedValue<n::restrict_mirror, std::tr1::shared_ptr<Set<std::string> > > restrict_mirror;
            NamedValue<n::restrict_primaryuri, std::tr1::shared_ptr<Set<std::string> > > restrict_primaryuri;
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
