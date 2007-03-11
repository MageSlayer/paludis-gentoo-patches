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

#include "cran_version_metadata.hh"
#include "cran_dep_parser.hh"

using namespace paludis;

CRANVersionMetadata::CRANVersionMetadata(bool want_origins) :
    VersionMetadata(
            VersionMetadataBase(SlotName("unset"), "", "", "UNKNOWN", false),
            VersionMetadataCapabilities::create()
            .cran_interface(this)
            .ebuild_interface(0)
            .license_interface(0)
            .virtual_interface(0)
            .origins_interface(want_origins ? _origins : 0)
            .deps_interface(this)
            .ebin_interface(0)
            ),
    VersionMetadataCRANInterface("", "", "", false, false),
    VersionMetadataDepsInterface(CRANDepParser::parse),
    _origins(want_origins ? new VersionMetadataOriginsInterface : 0)
{
}

CRANVersionMetadata::~CRANVersionMetadata()
{
    if (_origins)
        delete _origins;
}

