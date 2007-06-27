/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Danny van Dyk <kugelfang@gentoo.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_CRAN_PACKAGE_ID_HH
#define PALUDIS_GUARD_PALUDIS_CRAN_PACKAGE_ID_HH 1

#include <paludis/package_id.hh>
#include <paludis/util/fs_entry.hh>

namespace paludis
{
    class PALUDIS_VISIBLE CRANPackageID :
        public PackageID
    {
        public:
            static void normalise_name(std::string & s);
            static void denormalise_name(std::string & s);
            static void normalise_version(std::string & s);

            const std::string native_package() const PALUDIS_ATTRIBUTE((warn_unused_result));
            const std::string native_version() const PALUDIS_ATTRIBUTE((warn_unused_result));
            const tr1::shared_ptr<const MetadataPackageIDKey> bundle_key() const PALUDIS_ATTRIBUTE((warn_unused_result));
            const tr1::shared_ptr<const MetadataPackageIDKey> bundle_member_key() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };
}

#endif
