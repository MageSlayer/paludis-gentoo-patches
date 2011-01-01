/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_UNAVAILABLE_UNAVAILABLE_REPOSITORY_FILE_FWD_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_UNAVAILABLE_UNAVAILABLE_REPOSITORY_FILE_FWD_HH 1

#include <paludis/util/attributes.hh>
#include <paludis/util/named_value.hh>
#include <paludis/name.hh>
#include <paludis/version_spec.hh>
#include <paludis/metadata_key-fwd.hh>
#include <memory>

namespace paludis
{
    namespace n
    {
        typedef Name<struct name_description> description;
        typedef Name<struct name_name> name;
        typedef Name<struct name_slot> slot;
        typedef Name<struct name_version> version;
    }

    namespace unavailable_repository
    {
        class UnavailableRepositoryFile;

        struct UnavailableRepositoryFileEntry
        {
            NamedValue<n::description, std::shared_ptr<const MetadataValueKey<std::string> > > description;
            NamedValue<n::name, QualifiedPackageName> name;
            NamedValue<n::slot, SlotName> slot;
            NamedValue<n::version, VersionSpec> version;
        };
    }
}


#endif
