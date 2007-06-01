/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <paludis/qa/ebuild_check.hh>
#include <paludis/qa/create_metadata_check.hh>
#include <paludis/qa/dep_any_check.hh>
#include <paludis/qa/dep_flags_check.hh>
#include <paludis/qa/dep_packages_check.hh>
#include <paludis/qa/deps_exist_check.hh>
#include <paludis/qa/description_check.hh>
#include <paludis/qa/extract_check.hh>
#include <paludis/qa/homepage_check.hh>
#include <paludis/qa/inherits_check.hh>
#include <paludis/qa/iuse_check.hh>
#include <paludis/qa/keywords_check.hh>
#include <paludis/qa/license_check.hh>
#include <paludis/qa/parse_deps_check.hh>
#include <paludis/qa/restrict_check.hh>
#include <paludis/qa/src_uri_check.hh>
#include <paludis/qa/pdepend_overlap_check.hh>
#include <paludis/qa/slot_check.hh>
#include <paludis/util/instantiation_policy-impl.hh>
#include <paludis/util/virtual_constructor-impl.hh>

using namespace paludis;
using namespace paludis::qa;

template class VirtualConstructor<std::string, tr1::shared_ptr<EbuildCheck> (*) (),
         virtual_constructor_not_found::ThrowException<NoSuchEbuildCheckTypeError> >;

template class InstantiationPolicy<EbuildCheckMaker, instantiation_method::SingletonTag>;

#include <paludis/qa/ebuild_check-sr.cc>

EbuildCheck::EbuildCheck()
{
}

NoSuchEbuildCheckTypeError::NoSuchEbuildCheckTypeError(const std::string & s) throw () :
    Exception("No such ebuild check type: '" + s + "'")
{
}

EbuildCheckMaker::EbuildCheckMaker()
{
    register_maker(CreateMetadataCheck::identifier(), &MakeEbuildCheck<CreateMetadataCheck>::make_ebuild_check);
    register_maker(DepAnyCheck::identifier(), &MakeEbuildCheck<DepAnyCheck>::make_ebuild_check);
    register_maker(DepFlagsCheck::identifier(), &MakeEbuildCheck<DepFlagsCheck>::make_ebuild_check);
    register_maker(DepPackagesCheck::identifier(), &MakeEbuildCheck<DepPackagesCheck>::make_ebuild_check);
    register_maker(DepsExistCheck::identifier(), &MakeEbuildCheck<DepsExistCheck>::make_ebuild_check);
    register_maker(DescriptionCheck::identifier(), &MakeEbuildCheck<DescriptionCheck>::make_ebuild_check);
    register_maker(ExtractCheck::identifier(), &MakeEbuildCheck<ExtractCheck>::make_ebuild_check);
    register_maker(HomepageCheck::identifier(), &MakeEbuildCheck<HomepageCheck>::make_ebuild_check);
    register_maker(InheritsCheck::identifier(), &MakeEbuildCheck<InheritsCheck>::make_ebuild_check);
    register_maker(IuseCheck::identifier(), &MakeEbuildCheck<IuseCheck>::make_ebuild_check);
    register_maker(KeywordsCheck::identifier(), &MakeEbuildCheck<KeywordsCheck>::make_ebuild_check);
    register_maker(LicenseCheck::identifier(), &MakeEbuildCheck<LicenseCheck>::make_ebuild_check);
    register_maker(ParseDepsCheck::identifier(), &MakeEbuildCheck<ParseDepsCheck>::make_ebuild_check);
    register_maker(RestrictCheck::identifier(), &MakeEbuildCheck<RestrictCheck>::make_ebuild_check);
    register_maker(SrcUriCheck::identifier(), &MakeEbuildCheck<SrcUriCheck>::make_ebuild_check);
    register_maker(PdependOverlapCheck::identifier(), &MakeEbuildCheck<PdependOverlapCheck>::make_ebuild_check);
    register_maker(SlotCheck::identifier(), &MakeEbuildCheck<SlotCheck>::make_ebuild_check);
}

