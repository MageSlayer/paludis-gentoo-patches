/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_GEMS_PARAMS_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_GEMS_PARAMS_HH 1

#include <paludis/repositories/gems/params-fwd.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/named_value.hh>
#include <string>

namespace paludis
{
    class Environment;

    namespace n
    {
        typedef Name<struct builddir_name> builddir;
        typedef Name<struct environment_name> environment;
        typedef Name<struct install_dir_name> install_dir;
        typedef Name<struct location_name> location;
        typedef Name<struct root_name> root;
        typedef Name<struct sync_name> sync;
        typedef Name<struct sync_options_name> sync_options;
    }

    namespace gems
    {
        struct RepositoryParams
        {
            NamedValue<n::builddir, FSEntry> builddir;
            NamedValue<n::environment, Environment *> environment;
            NamedValue<n::install_dir, FSEntry> install_dir;
            NamedValue<n::location, FSEntry> location;
            NamedValue<n::sync, std::string> sync;
            NamedValue<n::sync_options, std::string> sync_options;
        };

        struct InstalledRepositoryParams
        {
            NamedValue<n::builddir, FSEntry> builddir;
            NamedValue<n::environment, Environment *> environment;
            NamedValue<n::install_dir, FSEntry> install_dir;
            NamedValue<n::root, FSEntry> root;
        };
    }
}

#endif
