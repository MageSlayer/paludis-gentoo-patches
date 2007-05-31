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
#include <paludis/eapi.hh>
#include <paludis/util/log.hh>
#include <paludis/util/collection_concrete.hh>

/** \file
 * Implementation of VersionMetadata.
 *
 * \ingroup grpversions
 */

using namespace paludis;

#include <paludis/version_metadata-sr.cc>

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

VersionMetadataOriginsInterface::VersionMetadataOriginsInterface()
{
}

VersionMetadataLicenseInterface::VersionMetadataLicenseInterface(const LicenseParserFunction & f) :
    parser(f)
{
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

tr1::shared_ptr<const DependencySpecTree::ConstItem>
VersionMetadataDepsInterface::_make_depend(const std::string & s) const
{
    if (version_metadata()->eapi->supported)
        return parser(s, *version_metadata()->eapi);
    else
    {
        Log::get_instance()->message(ll_warning, lc_context) <<
            "Don't know how to parse dependency strings for EAPI '" + version_metadata()->eapi->name + "'";
        return tr1::shared_ptr<DependencySpecTree::ConstItem>(new ConstTreeSequence<DependencySpecTree, AllDepSpec>(
                    tr1::shared_ptr<AllDepSpec>(new AllDepSpec)));
    }
}

tr1::shared_ptr<const RestrictSpecTree::ConstItem>
VersionMetadataEbuildInterface::_make_restrict(const std::string & s) const
{
    if (version_metadata()->eapi->supported)
        return PortageDepParser::parse_restrict(s, *version_metadata()->eapi);
    else
    {
        Log::get_instance()->message(ll_warning, lc_context) <<
            "Don't know how to parse restrict strings for EAPI '" + version_metadata()->eapi->name + "'";
        return tr1::shared_ptr<RestrictSpecTree::ConstItem>(new ConstTreeSequence<RestrictSpecTree, AllDepSpec>(
                    tr1::shared_ptr<AllDepSpec>(new AllDepSpec)));
    }
}

tr1::shared_ptr<const ProvideSpecTree::ConstItem>
VersionMetadataEbuildInterface::_make_provide(const std::string & s) const
{
    if (version_metadata()->eapi->supported)
        return PortageDepParser::parse_provide(s, *version_metadata()->eapi);
    else
    {
        Log::get_instance()->message(ll_warning, lc_context) <<
            "Don't know how to parse provide strings for EAPI '" + version_metadata()->eapi->name + "'";
        return tr1::shared_ptr<ProvideSpecTree::ConstItem>(new ConstTreeSequence<ProvideSpecTree, AllDepSpec>(
                    tr1::shared_ptr<AllDepSpec>(new AllDepSpec)));
    }
}

tr1::shared_ptr<const LicenseSpecTree::ConstItem>
VersionMetadataLicenseInterface::_make_license(const std::string & s) const
{
    if (version_metadata()->eapi->supported)
        return parser(s, *version_metadata()->eapi);
    else
    {
        Log::get_instance()->message(ll_warning, lc_context) <<
            "Don't know how to parse license strings for EAPI '" + version_metadata()->eapi->name + "'";
        return tr1::shared_ptr<LicenseSpecTree::ConstItem>(new ConstTreeSequence<LicenseSpecTree, AllDepSpec>(
                    tr1::shared_ptr<AllDepSpec>(new AllDepSpec)));
    }
}

template <typename Item_, typename Container_>
tr1::shared_ptr<const Container_>
VersionMetadataEbuildInterface::_make_collection(const std::string & s) const
{
    tr1::shared_ptr<Container_> result(new typename Container_::Concrete);
    WhitespaceTokeniser::get_instance()->tokenise(s, create_inserter<Item_>(result->inserter()));
    return result;
}

tr1::shared_ptr<const IUseFlagCollection>
VersionMetadataEbuildInterface::_make_iuse_collection(const std::string & s) const
{
    tr1::shared_ptr<IUseFlagCollection> result(new IUseFlagCollection::Concrete);
    std::list<std::string> t;
    WhitespaceTokeniser::get_instance()->tokenise(s, std::back_inserter(t));

    if (version_metadata()->eapi->supported)
    {
        IUseFlagParseMode m(version_metadata()->eapi->supported->iuse_flag_parse_mode);
        for (std::list<std::string>::const_iterator u(t.begin()), u_end(t.end()) ;
                u != u_end ; ++u)
            result->insert(IUseFlag(*u, m));
    }
    else
        Log::get_instance()->message(ll_warning, lc_context) <<
            "Don't know how to parse IUSE strings for EAPI '" + version_metadata()->eapi->name + "'";

    return result;
}

tr1::shared_ptr<const URISpecTree::ConstItem>
VersionMetadataEbinInterface::_make_uri(const std::string & s) const
{
    return PortageDepParser::parse_uri(s, *version_metadata()->eapi);
}

tr1::shared_ptr<const URISpecTree::ConstItem>
VersionMetadataEbuildInterface::_make_uri(const std::string & s) const
{
    return PortageDepParser::parse_uri(s, *version_metadata()->eapi);
}

tr1::shared_ptr<const URISpecTree::ConstItem>
VersionMetadataBase::_make_text(const std::string & s) const
{
    return PortageDepParser::parse_uri(s, *version_metadata()->eapi);
}

template <typename Item_, typename Container_>
tr1::shared_ptr<const Container_>
VersionMetadataCRANInterface::_make_collection(const std::string & s) const
{
    tr1::shared_ptr<Container_> result(new typename Container_::Concrete);
    WhitespaceTokeniser::get_instance()->tokenise(s, create_inserter<Item_>(result->inserter()));
    return result;
}

