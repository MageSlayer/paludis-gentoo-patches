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

#include <paludis/repositories/gems/metadata.hh>
#include <paludis/repositories/gems/gem_specification.hh>
#include <paludis/util/stringify.hh>
#include <paludis/eapi.hh>

using namespace paludis;
using namespace paludis::gems;

InstalledGemMetadata::InstalledGemMetadata(const VersionSpec & v) :
    VersionMetadata(
            VersionMetadataBase::create()
            .slot(SlotName(stringify(v)))
            .homepage("")
            .description("")
            .eapi(EAPIData::get_instance()->unknown_eapi())
            .interactive(false),
            VersionMetadataCapabilities::create()
            .ebuild_interface(0)
            .cran_interface(0)
            .deps_interface(0)
            .license_interface(0)
            .virtual_interface(0)
            .origins_interface(0)
            .ebin_interface(0))
{
}

InstalledGemMetadata::~InstalledGemMetadata()
{
}

void
InstalledGemMetadata::populate_from_specification(const GemSpecification & spec)
{
    description = spec.summary();
    set_homepage(spec.homepage());
}

GemMetadata::GemMetadata(const VersionSpec & v) :
    VersionMetadata(
            VersionMetadataBase::create()
            .slot(SlotName(stringify(v)))
            .homepage("")
            .description("")
            .eapi(EAPIData::get_instance()->unknown_eapi())
            .interactive(false),
            VersionMetadataCapabilities::create()
            .ebuild_interface(0)
            .cran_interface(0)
            .deps_interface(0)
            .license_interface(0)
            .virtual_interface(0)
            .origins_interface(0)
            .ebin_interface(0))
{
}

GemMetadata::~GemMetadata()
{
}

void
GemMetadata::populate_from_specification(const GemSpecification & spec)
{
    description = spec.summary();
    set_homepage(spec.homepage());
}

