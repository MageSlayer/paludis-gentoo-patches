/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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

DepAtom::ConstPointer
VersionMetadataDeps::build_depend() const
{
    return parser(build_depend_string);
}

DepAtom::ConstPointer
VersionMetadataDeps::run_depend() const
{
    return parser(run_depend_string);
}

DepAtom::ConstPointer
VersionMetadataDeps::post_depend() const
{
    return parser(post_depend_string);
}

DepAtom::ConstPointer
VersionMetadataDeps::suggested_depend() const
{
    return parser(suggested_depend_string);
}

VersionMetadataDeps::VersionMetadataDeps(const ParserFunction & p) :
    parser(p)
{
}

VersionMetadata::CRAN::CRAN(ParserFunction f) :
    VersionMetadata(f, &_c, 0, 0, 0),
    _c(CRANVersionMetadata::create()
            .keywords("")
            .package("")
            .version("")
            .is_bundle(false)
            .is_bundle_member(false))
{
}

VersionMetadata::Ebuild::Ebuild(ParserFunction f) :
    VersionMetadata(f, 0, &_e, 0, 0),
    _e()
{
}

VersionMetadata::Ebin::Ebin(ParserFunction f) :
    VersionMetadata(f, 0, &_e, &_eb, 0),
    _e(),
    _eb()
{
}

VersionMetadata::~VersionMetadata()
{
}

VersionMetadata::VersionMetadata(ParserFunction p) :
    VersionMetadataBase(VersionMetadataBase::create()
            .slot(SlotName("unset"))
            .deps(VersionMetadataDeps(p))
            .origins(VersionMetadataOrigins())
            .homepage("")
            .license_string("")
            .description("")
            .eapi("")),
    _cran_if(0),
    _ebuild_if(0),
    _ebin_if(0),
    _virtual_if(0)
{
}

VersionMetadata::VersionMetadata(ParserFunction p,
        CRANVersionMetadata * cran_if,
        EbuildVersionMetadata * ebuild_if,
        EbinVersionMetadata * ebin_if,
        VirtualMetadata * virtual_if) :
    VersionMetadataBase(VersionMetadataBase::create()
            .slot(SlotName("unset"))
            .deps(VersionMetadataDeps(p))
            .origins(VersionMetadataOrigins())
            .homepage("")
            .license_string("")
            .description("")
            .eapi("")),
     _cran_if(cran_if),
     _ebuild_if(ebuild_if),
     _ebin_if(ebin_if),
     _virtual_if(virtual_if)
{
}

EbuildVersionMetadata::EbuildVersionMetadata() :
    provide_string(""),
    src_uri(""),
    restrict_string(""),
    keywords(""),
    iuse(""),
    inherited("")
{
}

EbinVersionMetadata::EbinVersionMetadata() :
    bin_uri(""),
    src_repository(RepositoryName("unset"))
{
}

DepAtom::ConstPointer
EbuildVersionMetadata::provide() const
{
    return PortageDepParser::parse(provide_string, PortageDepParserPolicy<PackageDepAtom,
            false>::get_instance());
}

DepAtom::ConstPointer
VersionMetadata::license() const
{
    return PortageDepParser::parse(license_string, PortageDepParserPolicy<PlainTextDepAtom,
            true>::get_instance());
}

VersionMetadata::Virtual::Virtual(ParserFunction p, const PackageDatabaseEntry & e) :
    VersionMetadata(p, 0, 0, 0, &_v),
    _v(e)
{
}

VersionMetadataOrigins::VersionMetadataOrigins() :
    source(0),
    binary(0)
{
}

