/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Danny van Dyk <kugelfang@gentoo.org>
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

#include <paludis/repositories/cran/cran_package_id.hh>
#include <paludis/repositories/cran/cran_dep_parser.hh>
#include <paludis/repositories/cran/description_file.hh>
#include <paludis/repositories/cran/masks.hh>
#include <paludis/repositories/cran/keys.hh>
#include <paludis/repositories/cran/normalise.hh>
#include <paludis/config_file.hh>
#include <paludis/repository.hh>
#include <paludis/util/log.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/name.hh>
#include <paludis/version_spec.hh>
#include <paludis/action.hh>
#include <paludis/util/tokeniser.hh>
#include <string>
#include <algorithm>
#include <list>

#include <paludis/util/tr1_functional.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>
#include <libwrapiter/libwrapiter_output_iterator.hh>

using namespace paludis;
using namespace paludis::cranrepository;

namespace paludis
{
    template <>
    struct Implementation<CRANPackageID>
    {
        const tr1::shared_ptr<const Repository> repository;
        QualifiedPackageName name;
        VersionSpec version;

        tr1::shared_ptr<URIKey> homepage_key;
        tr1::shared_ptr<StringKey> short_description_key;
        tr1::shared_ptr<StringKey> long_description_key;
        tr1::shared_ptr<PackageIDKey> contained_in_key;
        tr1::shared_ptr<PackageIDSequenceKey> contains_key;
        tr1::shared_ptr<DepKey> depends_key;
        tr1::shared_ptr<DepKey> suggests_key;

        Implementation(const tr1::shared_ptr<const Repository> & r, const FSEntry & f) :
            repository(r),
            name("cran/" + cran_name_to_internal(strip_trailing_string(f.basename(), ".DESCRIPTION"))),
            version("0")
        {
        }

        Implementation(const CRANPackageID * const r, const std::string & t) :
            repository(r->repository()),
            name("cran/" + cran_name_to_internal(t)),
            version(r->version()),
            contained_in_key(new PackageIDKey("Contained", "Contained in", r, mkt_normal))
        {
        }
    };
}

CRANPackageID::CRANPackageID(const tr1::shared_ptr<const Repository> & r, const FSEntry & f) :
    PrivateImplementationPattern<CRANPackageID>(new Implementation<CRANPackageID>(r, f)),
    _imp(PrivateImplementationPattern<CRANPackageID>::_imp.get())
{
    Context context("When parsing file '" + stringify(f) + "' to create a CRAN Package ID:");

    if (! f.is_regular_file())
    {
        add_mask(make_shared_ptr(new BrokenMask('B', "Broken", "DESCRIPTION file not a file")));
        Log::get_instance()->message(ll_warning, lc_context) << "Unexpected irregular file: '" << stringify(f) << "'";
        return;
    }

    try
    {
        DescriptionFile file(f);

        std::string raw_name;
        if (! file.get("Package").empty())
        {
            Context local_context("When handling Package: key:");
            raw_name = file.get("Package");
            _imp->name = QualifiedPackageName(CategoryNamePart("cran"), PackageNamePart(cran_name_to_internal(raw_name)));
        }
        else if (! file.get("Bundle").empty())
        {
            Context local_context("When handling Bundle: key:");
            raw_name = file.get("Bundle");
            _imp->name = QualifiedPackageName(CategoryNamePart("cran"), PackageNamePart(cran_name_to_internal(raw_name)));
        }
        else
        {
            Log::get_instance()->message(ll_warning, lc_context) << "No Package: or Bundle: key in '" << stringify(f) << "'";
            add_mask(make_shared_ptr(new BrokenMask('B', "Broken", "No Package: or Bundle: key")));
            return;
        }

        if (file.get("Version").empty())
        {
            Context local_context("When handling Version: key:");
            Log::get_instance()->message(ll_warning, lc_context) << "No Version: key in '" << stringify(f) << "'";
            _imp->version = VersionSpec("0");
            add_mask(make_shared_ptr(new BrokenMask('B', "Broken", "No Version: key")));
            return;
        }
        else
        {
            Context local_context("When handling Version: key:");
            _imp->version = VersionSpec(cran_version_to_internal(file.get("Version")));
        }

        if (! file.get("URL").empty())
        {
            Context local_context("When handling URL: key:");
            _imp->homepage_key.reset(new URIKey("URL", "URL", file.get("URL"), mkt_significant));
            add_metadata_key(_imp->homepage_key);
        }

        if (! file.get("Title").empty())
        {
            Context local_context("When handling Title: key:");
            _imp->short_description_key.reset(new StringKey("Title", "Title", file.get("Title"), mkt_significant));
            add_metadata_key(_imp->short_description_key);
        }

        if (! file.get("Description").empty())
        {
            Context local_context("When handling Description: key:");
            _imp->long_description_key.reset(new StringKey("Description", "Description", file.get("Description"), mkt_normal));
            add_metadata_key(_imp->long_description_key);
        }
        else if (! file.get("BundleDescription").empty())
        {
            Context local_context("When handling BundleDescription: key:");
            _imp->long_description_key.reset(new StringKey("BundleDescription", "Bundle Description",
                        file.get("BundleDescription"), mkt_normal));
        }

        if (! file.get("Author").empty())
        {
            Context local_context("When handling Author: key:");
            add_metadata_key(make_shared_ptr(new StringKey("Author", "Author", file.get("Author"), mkt_normal)));
        }

        if (! file.get("Maintainer").empty())
        {
            Context local_context("When handling Maintainer: key:");
            add_metadata_key(make_shared_ptr(new StringKey("Maintainer", "Maintainer", file.get("Maintainer"), mkt_normal)));
        }

        if (! file.get("Contains").empty())
        {
            Context local_context("When handling Contains: key:");
            std::list<std::string> tokens;
            WhitespaceTokeniser::get_instance()->tokenise(file.get("Contains"), std::back_inserter(tokens));
            _imp->contains_key.reset(new PackageIDSequenceKey("Contains", "Contains", mkt_normal));
            add_metadata_key(_imp->contains_key);
            for (std::list<std::string>::const_iterator t(tokens.begin()), t_end(tokens.end()) ;
                    t != t_end ; ++t)
            {
                if (*t != stringify(name().package))
                    _imp->contains_key->push_back(make_shared_ptr(new CRANPackageID(this, *t)));
                else
                {
                    /* yay CRAN... */
                    Log::get_instance()->message(ll_qa, lc_context) << "Bundle '" << stringify(*this) << "' contains itself, but is "
                        "not a Klein bundle";
                }
            }
        }

        if (! file.get("Suggests").empty())
        {
            Context local_context("When handling Suggests: key:");
            _imp->suggests_key.reset(new DepKey("Suggests", "Suggests", file.get("Suggests"), mkt_dependencies));
            add_metadata_key(_imp->suggests_key);
        }

        if (! file.get("Depends").empty())
        {
            Context local_context("When handling Depends: key:");
            _imp->depends_key.reset(new DepKey("Depends", "Depends", file.get("Depends") + ", R", mkt_dependencies));
        }
        else
            _imp->depends_key.reset(new DepKey("Depends", "Depends", "R", mkt_dependencies));
        add_metadata_key(_imp->depends_key);
    }
    catch (const Exception & e)
    {
        Log::get_instance()->message(ll_warning, lc_context) << "Broken CRAN description file '" << stringify(f) << "': '"
            << e.message() << "' (" << e.what() << ")";
        add_mask(make_shared_ptr(new BrokenMask('B', "Broken", "Got exception '" + stringify(e.message()) + "' (" + e.what() + "')")));
    }

#if 0
    for (cranrepository::DescriptionFile::Iterator i(file.begin()), i_end(file.end()) ;
            i != i_end ; ++i)
    {
        if (("Package" == key) || ("Bundle" == key))
        {
            metadata->set_homepage("http://cran.r-project.org/src/contrib/Descriptions/" + value + ".html");
            if ("Package" == key)
            {
                CRANDescription::normalise_name(value);
                if (n != value)
                    Log::get_instance()->message(ll_warning, lc_context) << "Inconsistent package name in file '" <<
                        f << "': '" << value << "'";
            }
            else
                metadata->cran_interface->is_bundle = true;
        }
        else if ("Depends" == key)
        {
            if (value.empty())
                value = "R";
            else
                value.append(", R");
            metadata->deps_interface->set_build_depend(value);
            metadata->deps_interface->set_run_depend(value);
        }
        else if ("Suggests" == key)
            metadata->deps_interface->set_suggested_depend(value);
    }
#endif
}

CRANPackageID::CRANPackageID(const CRANPackageID * const r, const std::string & t) :
    PrivateImplementationPattern<CRANPackageID>(new Implementation<CRANPackageID>(r, t)),
    _imp(PrivateImplementationPattern<CRANPackageID>::_imp.get())
{
    Context context("When creating contained ID '" + stringify(t) + "' in " + stringify(*r) + "':");

    add_metadata_key(_imp->contained_in_key);
}

CRANPackageID::~CRANPackageID()
{
}

void
CRANPackageID::need_keys_added() const
{
}

void
CRANPackageID::need_masks_added() const
{
}


const QualifiedPackageName
CRANPackageID::name() const
{
    return _imp->name;
}

const VersionSpec
CRANPackageID::version() const
{
    return _imp->version;
}

const SlotName
CRANPackageID::slot() const
{
    return SlotName("0");
}

const tr1::shared_ptr<const Repository>
CRANPackageID::repository() const
{
    return _imp->repository;
}

const tr1::shared_ptr<const MetadataPackageIDKey>
CRANPackageID::virtual_for_key() const
{
    return tr1::shared_ptr<const MetadataPackageIDKey>();
}

const tr1::shared_ptr<const MetadataSetKey<KeywordNameSet> >
CRANPackageID::keywords_key() const
{
    return tr1::shared_ptr<const MetadataSetKey<KeywordNameSet> >();
}

const tr1::shared_ptr<const MetadataSetKey<UseFlagNameSet> >
CRANPackageID::use_key() const
{
    return tr1::shared_ptr<const MetadataSetKey<UseFlagNameSet> >();
}

const tr1::shared_ptr<const MetadataSetKey<IUseFlagSet> >
CRANPackageID::iuse_key() const
{
    return tr1::shared_ptr<const MetadataSetKey<IUseFlagSet> >();
}

const tr1::shared_ptr<const MetadataSetKey<InheritedSet> >
CRANPackageID::inherited_key() const
{
    return tr1::shared_ptr<const MetadataSetKey<InheritedSet> >();
}

const tr1::shared_ptr<const MetadataSpecTreeKey<LicenseSpecTree> >
CRANPackageID::license_key() const
{
    return tr1::shared_ptr<const MetadataSpecTreeKey<LicenseSpecTree> >();
}

const tr1::shared_ptr<const MetadataSpecTreeKey<ProvideSpecTree> >
CRANPackageID::provide_key() const
{
    return tr1::shared_ptr<const MetadataSpecTreeKey<ProvideSpecTree> >();
}

const tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
CRANPackageID::build_dependencies_key() const
{
    return tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >();
}

const tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
CRANPackageID::run_dependencies_key() const
{
    return tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >();
}

const tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
CRANPackageID::post_dependencies_key() const
{
    return tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >();
}

const tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
CRANPackageID::suggested_dependencies_key() const
{
    return tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >();
}

const tr1::shared_ptr<const MetadataSpecTreeKey<RestrictSpecTree> >
CRANPackageID::restrict_key() const
{
    return tr1::shared_ptr<const MetadataSpecTreeKey<RestrictSpecTree> >();
}

const tr1::shared_ptr<const MetadataSpecTreeKey<URISpecTree> >
CRANPackageID::src_uri_key() const
{
    return tr1::shared_ptr<const MetadataSpecTreeKey<URISpecTree> >();
}

const tr1::shared_ptr<const MetadataSpecTreeKey<URISpecTree> >
CRANPackageID::bin_uri_key() const
{
    return tr1::shared_ptr<const MetadataSpecTreeKey<URISpecTree> >();
}

const tr1::shared_ptr<const MetadataSpecTreeKey<URISpecTree> >
CRANPackageID::homepage_key() const
{
    return _imp->homepage_key;
}

const tr1::shared_ptr<const MetadataStringKey>
CRANPackageID::short_description_key() const
{
    return _imp->short_description_key;
}

const tr1::shared_ptr<const MetadataStringKey>
CRANPackageID::long_description_key() const
{
    return _imp->long_description_key;
}

const tr1::shared_ptr<const MetadataContentsKey>
CRANPackageID::contents_key() const
{
    return tr1::shared_ptr<const MetadataContentsKey>();
}

const tr1::shared_ptr<const MetadataTimeKey>
CRANPackageID::installed_time_key() const
{
    return tr1::shared_ptr<const MetadataTimeKey>();
}

const tr1::shared_ptr<const MetadataStringKey>
CRANPackageID::source_origin_key() const
{
    return tr1::shared_ptr<const MetadataStringKey>();
}

const tr1::shared_ptr<const MetadataStringKey>
CRANPackageID::binary_origin_key() const
{
    return tr1::shared_ptr<const MetadataStringKey>();
}

std::size_t
CRANPackageID::extra_hash_value() const
{
    return 0;
}

bool
CRANPackageID::arbitrary_less_than_comparison(const PackageID &) const
{
    return false;
}

bool
CRANPackageID::breaks_portage() const
{
    return true;
}

const std::string
CRANPackageID::canonical_form(const PackageIDCanonicalForm f) const
{
    switch (f)
    {
        case idcf_full:
            return stringify(_imp->name) + "-" + stringify(_imp->version) + ":" + stringify(slot()) + "::" + stringify(_imp->repository->name());

        case idcf_version:
            return stringify(_imp->version);

        case idcf_no_version:
            return stringify(_imp->name) + ":" + stringify(slot()) + "::" + stringify(_imp->repository->name());

        case last_idcf:
            break;
    }

    throw InternalError(PALUDIS_HERE, "Bad PackageIDCanonicalForm");
}

bool
CRANPackageID::supports_action(const SupportsActionTestBase &) const
{
    return false;
}

void
CRANPackageID::perform_action(Action & a) const
{
    throw UnsupportedActionError(*this, a);
}

const tr1::shared_ptr<const MetadataSetKey<PackageIDSequence> >
CRANPackageID::contains_key() const
{
    return _imp->contains_key;
}

const tr1::shared_ptr<const MetadataPackageIDKey>
CRANPackageID::contained_in_key() const
{
    return _imp->contained_in_key;
}

