/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <paludis/util/iterator.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/version_metadata.hh>
#include <paludis/portage_dep_parser.hh>
#include <vector>

/** \file
 * Implementation of VersionMetadata.
 *
 * \ingroup grpversions
 */

using namespace paludis;

#include <paludis/version_metadata-sr.cc>

std::tr1::shared_ptr<const DepAtom>
VersionMetadataDepsInterface::build_depend() const
{
    return parser(build_depend_string);
}

std::tr1::shared_ptr<const DepAtom>
VersionMetadataDepsInterface::run_depend() const
{
    return parser(run_depend_string);
}

std::tr1::shared_ptr<const DepAtom>
VersionMetadataDepsInterface::post_depend() const
{
    return parser(post_depend_string);
}

std::tr1::shared_ptr<const DepAtom>
VersionMetadataDepsInterface::suggested_depend() const
{
    return parser(suggested_depend_string);
}

VersionMetadataDepsInterface::VersionMetadataDepsInterface(const ParserFunction & p) :
    parser(p)
{
}

VersionMetadata::~VersionMetadata()
{
}

VersionMetadata::VersionMetadata(const VersionMetadataBase & base, const VersionMetadataCapabilities & caps) :
    VersionMetadataBase(base),
    VersionMetadataCapabilities(caps)
{
}

std::tr1::shared_ptr<const DepAtom>
VersionMetadataEbuildInterface::provide() const
{
    return PortageDepParser::parse(provide_string, PortageDepParserPolicy<PackageDepAtom,
            false>::get_instance());
}

VersionMetadataOriginsInterface::VersionMetadataOriginsInterface()
{
}

VersionMetadataLicenseInterface::VersionMetadataLicenseInterface(const ParserFunction & f) :
    parser(f)
{
}

std::tr1::shared_ptr<const DepAtom>
VersionMetadataLicenseInterface::license() const
{
    return parser(license_string);
}

VersionMetadataEbuildInterface::VersionMetadataEbuildInterface()
{
}

