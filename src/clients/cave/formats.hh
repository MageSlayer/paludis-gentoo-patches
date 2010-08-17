/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_SRC_CLIENTS_CAVE_FORMATS_HH
#define PALUDIS_GUARD_SRC_CLIENTS_CAVE_FORMATS_HH 1

#include <string>

namespace paludis
{
    namespace cave
    {
        namespace c
        {
            const std::string bold_blue();
            const std::string blue();
            const std::string bold_green();
            const std::string green();
            const std::string bold_red();
            const std::string red();
            const std::string bold_yellow();
            const std::string yellow();
            const std::string bold_pink();
            const std::string pink();

            const std::string bold_blue_or_pink();
            const std::string blue_or_pink();
            const std::string bold_green_or_pink();
            const std::string green_or_pink();

            const std::string bold_normal();
            const std::string normal();
        }

        namespace f
        {
            const std::string show_set_heading();
            const std::string show_set_set();
            const std::string show_set_spec_installed();
            const std::string show_set_spec_installable();
            const std::string show_set_spec_unavailable();

            const std::string show_wildcard_heading();
            const std::string show_wildcard_spec_installed();
            const std::string show_wildcard_spec_installable();
            const std::string show_wildcard_spec_unavailable();

            const std::string show_repository_heading();

            const std::string show_package_heading();
            const std::string show_package_repository();
            const std::string show_package_version_installed();
            const std::string show_package_version_installable();
            const std::string show_package_version_unavailable();
            const std::string show_package_best();
            const std::string show_package_slot();
            const std::string show_package_no_slot();

            const std::string show_package_id_heading();
            const std::string show_package_id_masks();
            const std::string show_package_id_masks_overridden();

            const std::string show_metadata_key_value_raw();
            const std::string show_metadata_key_value_human();
            const std::string show_metadata_continued_value();
            const std::string show_metadata_subsection_raw();
            const std::string show_metadata_subsection_human();

            const std::string show_contents_file();
            const std::string show_contents_dir();
            const std::string show_contents_sym();
            const std::string show_contents_other();

            const std::string show_choice_forced_enabled();
            const std::string show_choice_enabled();
            const std::string show_choice_forced_disabled();
            const std::string show_choice_disabled();

            const std::string info_metadata();
            const std::string info_metadata_subsection();
            const std::string info_repository_heading();
            const std::string info_id_heading();
            const std::string info_heading();

            const std::string fix_cache_fixing();

            const std::string colour_formatter_keyword_name_plain();
            const std::string colour_formatter_keyword_name_accepted();
            const std::string colour_formatter_keyword_name_unaccepted();

            const std::string colour_formatter_choice_value_plain();
            const std::string colour_formatter_choice_value_enabled();
            const std::string colour_formatter_choice_value_disabled();
            const std::string colour_formatter_choice_value_forced();
            const std::string colour_formatter_choice_value_masked();

            const std::string colour_formatter_string_plain();

            const std::string colour_formatter_package_id_plain();
            const std::string colour_formatter_package_id_installed();
            const std::string colour_formatter_package_id_installable();

            const std::string colour_formatter_license_dep_spec_plain();
            const std::string colour_formatter_license_dep_spec_accepted();
            const std::string colour_formatter_license_dep_spec_unaccepted();

            const std::string colour_formatter_conditional_dep_spec_plain();
            const std::string colour_formatter_conditional_dep_spec_enabled();
            const std::string colour_formatter_conditional_dep_spec_disabled();
            const std::string colour_formatter_conditional_dep_spec_forced();
            const std::string colour_formatter_conditional_dep_spec_masked();

            const std::string colour_formatter_plain_text_dep_spec_plain();

            const std::string colour_formatter_simple_uri_dep_spec_plain();

            const std::string colour_formatter_fetchable_uri_dep_spec_plain();

            const std::string colour_formatter_uri_labels_dep_spec_plain();

            const std::string colour_formatter_package_dep_spec_plain();
            const std::string colour_formatter_package_dep_spec_installed();
            const std::string colour_formatter_package_dep_spec_installable();

            const std::string colour_formatter_dependency_labels_dep_spec_plain();

            const std::string colour_formatter_block_dep_spec_plain();

            const std::string colour_formatter_named_set_dep_spec_plain();

            const std::string colour_formatter_fsentry_plain();

            const std::string colour_formatter_indent();

            const std::string sync_heading();
            const std::string sync_message_success();
            const std::string sync_message_failure();
            const std::string sync_message_failure_message();
            const std::string sync_repos_title();
            const std::string sync_repo_starting();
            const std::string sync_repo_done_success();
            const std::string sync_repo_done_no_syncing_required();
            const std::string sync_repo_done_failure();
            const std::string sync_repo_active();
            const std::string sync_repo_active_quiet();
            const std::string sync_repo_tail();

            const std::string executables_file();
        }
    }
}

#endif
