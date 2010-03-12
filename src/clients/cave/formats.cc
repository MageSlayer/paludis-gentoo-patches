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

#include "formats.hh"

using namespace paludis;
using namespace cave;

const std::string
paludis::cave::c::bold_blue()
{
    return "\033[1;34m";
}

const std::string
paludis::cave::c::blue()
{
    return "\033[0;34m";
}

const std::string
paludis::cave::c::bold_green()
{
    return "\033[1;32m";
}

const std::string
paludis::cave::c::green()
{
    return "\033[0;32m";
}

const std::string
paludis::cave::c::red()
{
    return "\033[0;31m";
}

const std::string
paludis::cave::c::bold_red()
{
    return "\033[1;31m";
}

const std::string
paludis::cave::c::yellow()
{
    return "\033[0;33m";
}

const std::string
paludis::cave::c::bold_yellow()
{
    return "\033[1;33m";
}

const std::string
paludis::cave::c::normal()
{
    return "\033[0;0m";
}

const std::string
paludis::cave::c::bold_normal()
{
    return "\033[1m";
}

const std::string
paludis::cave::f::show_set_heading()
{
    return "* " + c::bold_blue() + "%s" + c::normal() + "\\n";
}

const std::string
paludis::cave::f::show_set_set()
{
    return "%i%i%i%i" + c::blue() + "%s" + c::normal() + "\\n";
}

const std::string
paludis::cave::f::show_set_spec_installed()
{
    return "%i%i%i%i" + c::bold_green() + "%s" + c::normal() + "\\n";
}

const std::string
paludis::cave::f::show_set_spec_installable()
{
    return "%i%i%i%i" + c::green() + "%s" + c::normal() + "\\n";
}

const std::string
paludis::cave::f::show_set_spec_unavailable()
{
    return "%i%i%i%i" + c::red() + "%s" + c::normal() + "\\n";
}

const std::string
paludis::cave::f::show_wildcard_heading()
{
    return "* " + c::bold_blue() + "%s" + c::normal() + "\\n";
}

const std::string
paludis::cave::f::show_wildcard_spec_installed()
{
    return "    " + c::green() + "%s" + c::normal() + "\\n";
}

const std::string
paludis::cave::f::show_wildcard_spec_installable()
{
    return "    %s\\n";
}

const std::string
paludis::cave::f::show_wildcard_spec_unavailable()
{
    return "    " + c::red() + "%s" + c::normal() + "\\n";
}

const std::string
paludis::cave::f::show_repository_heading()
{
    return "* " + c::bold_blue() + "%s" + c::normal() + "\\n";
}

const std::string
paludis::cave::f::show_metadata_key_value_raw()
{
    return "    %i%i%i%i%{if b}" + c::bold_normal() + "%{endif}" + "%r" + c::normal() + "%{column 30}%v\\n";
}

const std::string
paludis::cave::f::show_metadata_key_value_human()
{
    return "    %i%i%i%i%{if b}" + c::bold_normal() + "%{endif}" + "%h" + c::normal() + "%{column 30}%v\\n";
}

const std::string
paludis::cave::f::show_metadata_continued_value()
{
    return "    %{column 30}%i%i%v\\n";
}

const std::string
paludis::cave::f::show_metadata_subsection_human()
{
    return "    %i%i%i%i" + c::bold_blue() + "%h" + c::normal() + "\\n";
}

const std::string
paludis::cave::f::show_metadata_subsection_raw()
{
    return "    %i%i%i%i" + c::bold_blue() + "%r" + c::normal() + "\\n";
}

const std::string
paludis::cave::f::show_package_heading()
{
    return "* " + c::bold_blue() + "%s" + c::normal() + "\\n";
}

const std::string
paludis::cave::f::show_package_version_installed()
{
    return c::bold_green() + "%s" + c::normal();
}

const std::string
paludis::cave::f::show_package_version_installable()
{
    return c::green() + "%s" + c::normal();
}

const std::string
paludis::cave::f::show_package_version_unavailable()
{
    return c::red() + "(%s)%r" + c::normal();
}

const std::string
paludis::cave::f::show_package_repository()
{
    return "    ::%s%{column 30}";
}

const std::string
paludis::cave::f::show_package_best()
{
    return "*";
}

const std::string
paludis::cave::f::show_package_slot()
{
    return " {:%s}";
}

const std::string
paludis::cave::f::show_package_id_heading()
{
    return "    " + c::bold_blue() + "%s" + c::normal() + "\\n";
}

const std::string
paludis::cave::f::show_package_id_masks()
{
    return "        " + c::bold_red() + "%s" + c::normal() + "\\n";
}

const std::string
paludis::cave::f::info_metadata()
{
    return "%i%i%i%i%h%{column 30}%s\\n";
}

const std::string
paludis::cave::f::info_metadata_subsection()
{
    return "%i%i%i%i" + c::blue() + "%s" + c::normal() + ":\\n";
}

const std::string
paludis::cave::f::info_repository_heading()
{
    return "Repository " + c::blue() + "%s" + c::normal() + ":\\n";
}

const std::string
paludis::cave::f::info_heading()
{
    return c::blue() + "%s" + c::normal() + ":\\n";
}

const std::string
paludis::cave::f::colour_formatter_keyword_name_plain()
{
    return "%s";
}

const std::string
paludis::cave::f::colour_formatter_keyword_name_accepted()
{
    return c::green() + "%s" + c::normal();
}

const std::string
paludis::cave::f::colour_formatter_keyword_name_unaccepted()
{
    return c::red() + "%s" + c::normal();
}

const std::string
paludis::cave::f::colour_formatter_string_plain()
{
    return "%s";
}

const std::string
paludis::cave::f::colour_formatter_package_id_plain()
{
    return "%s";
}

const std::string
paludis::cave::f::colour_formatter_package_id_installed()
{
    return c::bold_green() + "%s" + c::normal();
}

const std::string
paludis::cave::f::colour_formatter_package_id_installable()
{
    return c::green() + "%s" + c::normal();
}

const std::string
paludis::cave::f::colour_formatter_license_dep_spec_plain()
{
    return "%s";
}

const std::string
paludis::cave::f::colour_formatter_license_dep_spec_accepted()
{
    return c::green() + "%s" + c::normal();
}

const std::string
paludis::cave::f::colour_formatter_license_dep_spec_unaccepted()
{
    return c::red() + "%s" + c::normal();
}

const std::string
paludis::cave::f::colour_formatter_choice_value_plain()
{
    return "%s";
}

const std::string
paludis::cave::f::colour_formatter_choice_value_enabled()
{
    return c::green() + "%k" + c::normal() + "%v";
}

const std::string
paludis::cave::f::colour_formatter_choice_value_disabled()
{
    return c::red() + "-%s" + c::normal();
}

const std::string
paludis::cave::f::colour_formatter_choice_value_forced()
{
    return c::green() + "(%k%v)" + c::normal();
}

const std::string
paludis::cave::f::colour_formatter_choice_value_masked()
{
    return c::red() + "(-%s)" + c::normal();
}

const std::string
paludis::cave::f::colour_formatter_conditional_dep_spec_plain()
{
    return "%s";
}

const std::string
paludis::cave::f::colour_formatter_conditional_dep_spec_enabled()
{
    return c::green() + "%s" + c::normal();
}

const std::string
paludis::cave::f::colour_formatter_conditional_dep_spec_disabled()
{
    return c::red() + "%s" + c::normal();
}

const std::string
paludis::cave::f::colour_formatter_conditional_dep_spec_forced()
{
    return c::green() + "(%s)" + c::normal();
}

const std::string
paludis::cave::f::colour_formatter_conditional_dep_spec_masked()
{
    return c::red() + "(%s)" + c::normal();
}

const std::string
paludis::cave::f::colour_formatter_plain_text_dep_spec_plain()
{
    return "%s";
}

const std::string
paludis::cave::f::colour_formatter_simple_uri_dep_spec_plain()
{
    return "%s";
}

const std::string
paludis::cave::f::colour_formatter_fetchable_uri_dep_spec_plain()
{
    return "%s";
}

const std::string
paludis::cave::f::colour_formatter_uri_labels_dep_spec_plain()
{
    return "%s";
}

const std::string
paludis::cave::f::colour_formatter_package_dep_spec_plain()
{
    return "%s";
}

const std::string
paludis::cave::f::colour_formatter_package_dep_spec_installed()
{
    return c::bold_green() + "%s" + c::normal();
}

const std::string
paludis::cave::f::colour_formatter_package_dep_spec_installable()
{
    return c::green() + "%s" + c::normal();
}

const std::string
paludis::cave::f::colour_formatter_dependency_labels_dep_spec_plain()
{
    return "%s";
}

const std::string
paludis::cave::f::colour_formatter_block_dep_spec_plain()
{
    return "%s";
}

const std::string
paludis::cave::f::colour_formatter_named_set_dep_spec_plain()
{
    return c::blue() + "%s" + c::normal();
}

const std::string
paludis::cave::f::show_contents_file()
{
    return "%{if b}%{column 30}%{endif}%r%{if b}\\n%{else} %{endif}";
}

const std::string
paludis::cave::f::show_contents_other()
{
    return "%{if b}%{column 30}%{endif}%r%{if b}\\n%{else} %{endif}";
}

const std::string
paludis::cave::f::show_contents_dir()
{
    return "%{if b}%{column 30}%{endif}%r%{if b}\\n%{else} %{endif}";
}

const std::string
paludis::cave::f::show_contents_sym()
{
    return "%{if b}%{column 30}%{endif}%r -> %v%{if b}\\n%{else} %{endif}";
}

const std::string
paludis::cave::f::show_choice_forced_enabled()
{
    return c::green() + "(%s)" + c::normal();
}

const std::string
paludis::cave::f::show_choice_enabled()
{
    return c::green() + "%s" + c::normal();
}

const std::string
paludis::cave::f::show_choice_forced_disabled()
{
    return c::red() + "(-%s)" + c::normal();
}

const std::string
paludis::cave::f::show_choice_disabled()
{
    return c::red() + "-%s" + c::normal();
}

const std::string
paludis::cave::f::colour_formatter_indent()
{
    return "%{column 30}%i%i%i%i";
}

const std::string
paludis::cave::f::sync_heading()
{
    return c::bold_blue() + "%s" + c::normal() + "\\n\\n";
}

const std::string
paludis::cave::f::sync_message_success()
{
    return "* " + c::bold_green() + "%k:" + c::normal() + "%{column 30}%v\\n";
}

const std::string
paludis::cave::f::sync_message_failure()
{
    return "* " + c::bold_red() + "%k:" + c::normal() + "%{column 30}%v\\n";
}

const std::string
paludis::cave::f::sync_message_failure_message()
{
    return "    %s\\n";
}

const std::string
paludis::cave::f::sync_repos_title()
{
    return c::bold_normal() + "Repository%{column 30}Status%{column 52}Pending%{column 60}Active%{column 68}Done" + c::normal() + "\\n";
}

const std::string
paludis::cave::f::sync_repo_starting()
{
    return "-> " + c::bold_blue() + "%s" + c::normal() + "%{column 30}starting%{column 52}%p%{column 60}%a%{column 68}%d\\n";
}

const std::string
paludis::cave::f::sync_repo_done_success()
{
    return "-> " + c::bold_green() + "%s" + c::normal() + "%{column 30}success%{column 52}%p%{column 60}%a%{column 68}%d\\n";
}

const std::string
paludis::cave::f::sync_repo_done_no_syncing_required()
{
    return "-> " + c::bold_green() + "%s" + c::normal() + "%{column 30}no syncing required%{column 52}%p%{column 60}%a%{column 68}%d\\n";
}

const std::string
paludis::cave::f::sync_repo_done_failure()
{
    return "-> " + c::bold_red() + "%s" + c::normal() + "%{column 30}failed%{column 52}%p%{column 60}%a%{column 68}%d\\n";
}

const std::string
paludis::cave::f::sync_repo_active()
{
    return "-> " + c::bold_yellow() + "%s" + c::normal() + "%{column 30}active%{column 52}%p%{column 60}%a%{column 68}%d\\n";
}

const std::string
paludis::cave::f::sync_repo_active_quiet()
{
    return "-> (no output for %s seconds)\\n";
}

const std::string
paludis::cave::f::sync_repo_tail()
{
    return "    ... %s\\n";
}

