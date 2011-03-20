/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_ENV_VAR_NAMES_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_ENV_VAR_NAMES_HH 1

#include <paludis/util/attributes.hh>
#include <string>

namespace paludis
{
    namespace env_vars
    {
        const std::string bypass_userpriv_checks("PALUDIS_BYPASS_USERPRIV_CHECKS");
        const std::string default_output_conf("PALUDIS_DEFAULT_OUTPUT_CONF");
        const std::string distribution("PALUDIS_DISTRIBUTION");
        const std::string distributions_dir("PALUDIS_DISTRIBUTIONS_DIR");
        const std::string do_nothing_sandboxy("PALUDIS_DO_NOTHING_SANDBOXY");
        const std::string eapis_dir("PALUDIS_EAPIS_DIR");
        const std::string ebuild_dir("PALUDIS_EBUILD_DIR");
        const std::string fetchers_dir("PALUDIS_FETCHERS_DIR");
        const std::string home("PALUDIS_HOME");
        const std::string hooker_dir("PALUDIS_HOOKER_DIR");
        const std::string ignore_hooks_named("PALUDIS_IGNORE_HOOKS_NAMED");
        const std::string no_chown("PALUDIS_NO_CHOWN");
        const std::string no_global_fetchers("PALUDIS_NO_GLOBAL_FETCHERS");
        const std::string no_global_hooks("PALUDIS_NO_GLOBAL_HOOKS");
        const std::string no_global_sets("PALUDIS_NO_GLOBAL_SETS");
        const std::string no_global_syncers("PALUDIS_NO_GLOBAL_SYNCERS");
        const std::string no_xml("PALUDIS_NO_XML");
        const std::string portage_bashrc("PALUDIS_PORTAGE_BASHRC");
        const std::string python_dir("PALUDIS_PYTHON_DIR");
        const std::string reduced_gid("PALUDIS_REDUCED_GID");
        const std::string reduced_uid("PALUDIS_REDUCED_UID");
        const std::string reduced_username("PALUDIS_REDUCED_USERNAME");
        const std::string suffixes_file("PALUDIS_SUFFIXES_FILE");
    }
}

#endif
