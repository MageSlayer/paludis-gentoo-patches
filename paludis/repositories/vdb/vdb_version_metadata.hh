/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_VDB_VDB_VERSION_METADATA_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_VDB_VDB_VERSION_METADATA_HH 1

#include <paludis/version_metadata.hh>

namespace paludis
{
    class VDBVersionMetadata :
        public VersionMetadata,
        public VersionMetadataDepsInterface,
        public VersionMetadataOriginsInterface,
        public VersionMetadataEbuildInterface,
        public VersionMetadataLicenseInterface
    {
        public:
            VDBVersionMetadata();
            virtual ~VDBVersionMetadata();

            typedef CountedPtr<const VDBVersionMetadata, count_policy::InternalCountTag> ConstPointer;
            typedef CountedPtr<VDBVersionMetadata, count_policy::InternalCountTag> Pointer;
    };

    class VDBVirtualVersionMetadata :
        public VersionMetadata,
        public VersionMetadataDepsInterface,
        public VersionMetadataOriginsInterface,
        public VersionMetadataEbuildInterface,
        public VersionMetadataLicenseInterface,
        public VersionMetadataVirtualInterface
    {
        public:
            VDBVirtualVersionMetadata(const SlotName &, const PackageDatabaseEntry &);
            virtual ~VDBVirtualVersionMetadata();

            typedef CountedPtr<const VDBVirtualVersionMetadata, count_policy::InternalCountTag> ConstPointer;
            typedef CountedPtr<VDBVirtualVersionMetadata, count_policy::InternalCountTag> Pointer;
    };
}

#endif
