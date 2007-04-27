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
#include <paludis/util/log.hh>
#include <paludis/util/collection_concrete.hh>

/** \file
 * Implementation of VersionMetadata.
 *
 * \ingroup grpversions
 */

using namespace paludis;

#include <paludis/version_metadata-sr.cc>

std::tr1::shared_ptr<const DepSpec>
VersionMetadataDepsInterface::build_depend() const
{
    return parser(build_depend_string, version_metadata()->eapi_as_package_dep_spec_parse_mode());
}

std::tr1::shared_ptr<const DepSpec>
VersionMetadataDepsInterface::run_depend() const
{
    return parser(run_depend_string, version_metadata()->eapi_as_package_dep_spec_parse_mode());
}

std::tr1::shared_ptr<const DepSpec>
VersionMetadataDepsInterface::post_depend() const
{
    return parser(post_depend_string, version_metadata()->eapi_as_package_dep_spec_parse_mode());
}

std::tr1::shared_ptr<const DepSpec>
VersionMetadataDepsInterface::suggested_depend() const
{
    return parser(suggested_depend_string, version_metadata()->eapi_as_package_dep_spec_parse_mode());
}

std::tr1::shared_ptr<const DepSpec>
VersionMetadataEbuildInterface::src_uri() const
{
    return PortageDepParser::parse(src_uri_string,
            PortageDepParser::Policy::text_is_text_dep_spec(false));
}

std::tr1::shared_ptr<const DepSpec>
VersionMetadataEbinInterface::bin_uri() const
{
    return PortageDepParser::parse(bin_uri_string,
            PortageDepParser::Policy::text_is_text_dep_spec(false));
}

std::tr1::shared_ptr<const KeywordNameCollection>
VersionMetadataEbuildInterface::keywords() const
{
    Context context("When splitting up keyword string '" + keywords_string + "':");

    std::tr1::shared_ptr<KeywordNameCollection> result(new KeywordNameCollection::Concrete);
    WhitespaceTokeniser::get_instance()->tokenise(keywords_string,
            create_inserter<KeywordName>(result->inserter()));
    return result;
}

std::tr1::shared_ptr<const KeywordNameCollection>
VersionMetadataCRANInterface::keywords() const
{
    Context context("When splitting up keyword string '" + keywords_string + "':");

    std::tr1::shared_ptr<KeywordNameCollection> result(new KeywordNameCollection::Concrete);
    WhitespaceTokeniser::get_instance()->tokenise(keywords_string,
            create_inserter<KeywordName>(result->inserter()));
    return result;
}

VersionMetadataDepsInterface::VersionMetadataDepsInterface(const DepParserFunction & p) :
    parser(p)
{
}

VersionMetadata::~VersionMetadata()
{
}

VersionMetadata::VersionMetadata(const VersionMetadataBase::NamedArguments<> & base, const VersionMetadataCapabilities & caps) :
    VersionMetadataBase(base),
    VersionMetadataCapabilities(caps)
{
}

std::tr1::shared_ptr<const DepSpec>
VersionMetadataEbuildInterface::provide() const
{
    return PortageDepParser::parse(provide_string, PortageDepParser::Policy::text_is_package_dep_spec(
                false, version_metadata()->eapi_as_package_dep_spec_parse_mode()));}

VersionMetadataOriginsInterface::VersionMetadataOriginsInterface()
{
}

VersionMetadataLicenseInterface::VersionMetadataLicenseInterface(const TextParserFunction & f) :
    parser(f)
{
}

std::tr1::shared_ptr<const DepSpec>
VersionMetadataLicenseInterface::license() const
{
    return parser(license_string);
}

VersionMetadataEbuildInterface::VersionMetadataEbuildInterface()
{
}

VersionMetadataEbinInterface::VersionMetadataEbinInterface()
{
}

VersionMetadataHasInterfaces::VersionMetadataHasInterfaces()
{
}

VersionMetadataHasInterfaces::~VersionMetadataHasInterfaces()
{
}

PackageDepSpecParseMode
VersionMetadata::eapi_as_package_dep_spec_parse_mode() const
{
    if (eapi == "0" || eapi == "")
        return pds_pm_eapi_0;
    else if (eapi == "CRAN-1" || eapi == "paludis-1")
        return pds_pm_permissive;
    else
    {
        Log::get_instance()->message(ll_warning, lc_context,
                "BUG! Don't know what parse mode to use for EAPI '" + stringify(eapi) + "'");
        return pds_pm_permissive;
    }
}

