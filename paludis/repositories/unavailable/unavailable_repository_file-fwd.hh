/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008 Ciaran McCreesh
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
#include <paludis/util/keys.hh>
#include <paludis/util/kc-fwd.hh>
#include <paludis/name-fwd.hh>
#include <paludis/version_spec-fwd.hh>
#include <paludis/metadata_key-fwd.hh>
#include <tr1/memory>

namespace paludis
{
    namespace unavailable_repository
    {
        class UnavailableRepositoryFile;

        typedef kc::KeyedClass<
            kc::Field<k::name, QualifiedPackageName>,
            kc::Field<k::version, VersionSpec>,
            kc::Field<k::slot, SlotName>,
            kc::Field<k::description, std::tr1::shared_ptr<const MetadataValueKey<std::string> > >
                > UnavailableRepositoryFileEntry;
    }
}


#endif
