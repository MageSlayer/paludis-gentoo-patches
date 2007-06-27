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

#include <paludis/repositories/gentoo/vdb_id.hh>
#include <paludis/repositories/gentoo/e_key.hh>

#include <paludis/name.hh>
#include <paludis/version_spec.hh>
#include <paludis/repository.hh>
#include <paludis/distribution.hh>
#include <paludis/eapi.hh>
#include <paludis/environment.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/log.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/strip.hh>
#include <iterator>
#include <fstream>

using namespace paludis;
using namespace paludis::erepository;

namespace
{
    std::string file_contents(const FSEntry & f)
    {
        Context c("When reading '" + stringify(f) + "':");
        std::ifstream i(stringify(f).c_str());
        if (! i)
            throw ConfigurationError("Cannot open '" + stringify(f) + "' for read");

        return strip_trailing(std::string((std::istreambuf_iterator<char>(i)), std::istreambuf_iterator<char>()), "\r\n");
    }
}

namespace paludis
{
    template <>
    struct Implementation<VDBID>
    {
        const QualifiedPackageName name;
        const VersionSpec version;
        const Environment * const environment;
        const tr1::shared_ptr<const Repository> repository;
        const FSEntry dir;
        mutable bool has_keys;

        tr1::shared_ptr<const SlotName> slot;
        tr1::shared_ptr<const EAPI> eapi;

        tr1::shared_ptr<const MetadataCollectionKey<UseFlagNameCollection> > use;
        tr1::shared_ptr<const MetadataCollectionKey<InheritedCollection> > inherited;
        tr1::shared_ptr<const MetadataCollectionKey<IUseFlagCollection> > iuse;
        tr1::shared_ptr<const MetadataSpecTreeKey<LicenseSpecTree> > license;
        tr1::shared_ptr<const MetadataSpecTreeKey<ProvideSpecTree> > provide;
        tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > build_dependencies;
        tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > run_dependencies;
        tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > post_dependencies;
        tr1::shared_ptr<const MetadataSpecTreeKey<RestrictSpecTree> > restrictions;
        tr1::shared_ptr<const MetadataSpecTreeKey<URISpecTree> > src_uri;
        tr1::shared_ptr<const MetadataSpecTreeKey<URISpecTree> > homepage;
        tr1::shared_ptr<const MetadataStringKey> short_description;

        Implementation(const QualifiedPackageName & q, const VersionSpec & v,
                const Environment * const e,
                const tr1::shared_ptr<const Repository> r, const FSEntry & f) :
            name(q),
            version(v),
            environment(e),
            repository(r),
            dir(f),
            has_keys(false)
        {
        }
    };
}

VDBID::VDBID(const QualifiedPackageName & q, const VersionSpec & v,
        const Environment * const e,
        const tr1::shared_ptr<const Repository> & r,
        const FSEntry & f) :
    PrivateImplementationPattern<VDBID>(new Implementation<VDBID>(q, v, e, r, f)),
    _imp(PrivateImplementationPattern<VDBID>::_imp.get())
{
}

VDBID::~VDBID()
{
}

void
VDBID::need_keys_added() const
{
    if (_imp->has_keys)
        return;
    _imp->has_keys = true;

    Context context("When loading VDB ID keys from '" + stringify(_imp->dir) + "':");

#if 0
    source_origin;
#endif

    if ((_imp->dir / "USE").exists())
    {
        _imp->use.reset(new EUseKey(shared_from_this(), "USE", "Use flags", file_contents(_imp->dir / "USE")));
        add_key(_imp->use);
    }

    if ((_imp->dir / "INHERITED").exists())
    {
        _imp->inherited.reset(new EInheritedKey(shared_from_this(), "INHERITED", "Inherited", file_contents(_imp->dir / "INHERITED")));
        add_key(_imp->inherited);
    }

    if ((_imp->dir / "IUSE").exists())
    {
        _imp->iuse.reset(new EIUseKey(shared_from_this(), "IUSE", "Used use flags", file_contents(_imp->dir / "IUSE")));
        add_key(_imp->iuse);
    }

    if ((_imp->dir / "LICENSE").exists())
    {
        _imp->license.reset(new ELicenseKey(shared_from_this(), "LICENSE", "License", file_contents(_imp->dir / "LICENSE")));
        add_key(_imp->license);
    }

    if ((_imp->dir / "PROVIDE").exists())
    {
        _imp->provide.reset(new EProvideKey(shared_from_this(), "PROVIDE", "Provides", file_contents(_imp->dir / "PROVIDE")));
        add_key(_imp->provide);
    }

    if ((_imp->dir / "DEPEND").exists())
    {
        _imp->build_dependencies.reset(new EDependenciesKey(shared_from_this(), "DEPEND", "Build dependencies",
                    file_contents(_imp->dir / "DEPEND")));
        add_key(_imp->build_dependencies);
    }

    if ((_imp->dir / "RDEPEND").exists())
    {
        _imp->run_dependencies.reset(new EDependenciesKey(shared_from_this(), "RDEPEND", "Run dependencies",
                    file_contents(_imp->dir / "RDEPEND")));
        add_key(_imp->run_dependencies);
    }

    if ((_imp->dir / "PDEPEND").exists())
    {
        _imp->post_dependencies.reset(new EDependenciesKey(shared_from_this(), "PDEPEND", "Post dependencies",
                    file_contents(_imp->dir / "PDEPEND")));
        add_key(_imp->post_dependencies);
    }

    if ((_imp->dir / "RESTRICT").exists())
    {
        _imp->restrictions.reset(new ERestrictKey(shared_from_this(), "RESTRICT", "Restrictions",
                    file_contents(_imp->dir / "RESTRICT")));
        add_key(_imp->restrictions);
    }

    if ((_imp->dir / "SRC_URI").exists())
    {
        _imp->src_uri.reset(new EURIKey(shared_from_this(), "SRC_URI", "Source URI",
                    file_contents(_imp->dir / "SRC_URI")));
        add_key(_imp->src_uri);
    }

    if ((_imp->dir / "HOMEPAGE").exists())
    {
        _imp->homepage.reset(new EURIKey(shared_from_this(), "HOMEPAGE", "Homepage",
                    file_contents(_imp->dir / "HOMEPAGE")));
        add_key(_imp->homepage);
    }

    if ((_imp->dir / "DESCRIPTION").exists())
    {
        _imp->short_description.reset(new EStringKey(shared_from_this(), "DESCRIPTION", "Description",
                    file_contents(_imp->dir / "DESCRIPTION")));
        add_key(_imp->short_description);
    }
}

const std::string
VDBID::canonical_form(const PackageIDCanonicalForm f) const
{
    switch (f)
    {
        case idcf_full:
            if (_imp->slot)
                return stringify(name()) + "-" + stringify(version()) + ":" + stringify(slot()) + "::" +
                    stringify(repository()->name());

            return stringify(name()) + "-" + stringify(version()) + "::" + stringify(repository()->name());

        case idcf_no_version:
            if (_imp->slot)
                return stringify(name()) + ":" + stringify(slot()) + "::" +
                    stringify(repository()->name());

            return stringify(name()) + "::" + stringify(repository()->name());

        case idcf_version:
            return stringify(version());

        case last_idcf:
            break;
    }

    throw InternalError(PALUDIS_HERE, "Bad PackageIDCanonicalForm");
}

const QualifiedPackageName
VDBID::name() const
{
    return _imp->name;
}

const VersionSpec
VDBID::version() const
{
    return _imp->version;
}

const SlotName
VDBID::slot() const
{
    if (_imp->slot)
        return *_imp->slot;

    Context context("When finding SLOT for '" + stringify(name()) + "-" + stringify(version()) + "::"
            + stringify(repository()->name()) + "':");

    if ((_imp->dir / "SLOT").exists())
        _imp->slot.reset(new SlotName(file_contents(_imp->dir / "SLOT")));
    else
    {
        Log::get_instance()->message(ll_warning, lc_context) << "No SLOT entry in '" << _imp->dir << "', pretending '0'";
        _imp->slot.reset(new SlotName("0"));
    }

    return *_imp->slot;
}

const tr1::shared_ptr<const Repository>
VDBID::repository() const
{
    return _imp->repository;
}

const tr1::shared_ptr<const EAPI>
VDBID::eapi() const
{
    if (_imp->eapi)
        return _imp->eapi;

    Context context("When finding EAPI for '" + canonical_form(idcf_full) + "':");

    if ((_imp->dir / "EAPI").exists())
        _imp->eapi = EAPIData::get_instance()->eapi_from_string(file_contents(_imp->dir / "EAPI"));
    else
    {
        Log::get_instance()->message(ll_debug, lc_context) << "No EAPI entry in '" << _imp->dir << "', pretending '"
            << _imp->environment->default_distribution() << "'";
        _imp->eapi = EAPIData::get_instance()->eapi_from_string(
                DistributionData::get_instance()->distribution_from_string(
                    _imp->environment->default_distribution())->default_ebuild_eapi_when_unspecified);
    }

    return _imp->eapi;
}

const tr1::shared_ptr<const MetadataPackageIDKey>
VDBID::virtual_for_key() const
{
    return tr1::shared_ptr<const MetadataPackageIDKey>();
}

const tr1::shared_ptr<const MetadataCollectionKey<KeywordNameCollection> >
VDBID::keywords_key() const
{
    return tr1::shared_ptr<const MetadataCollectionKey<KeywordNameCollection> >();
}

const tr1::shared_ptr<const MetadataCollectionKey<UseFlagNameCollection> >
VDBID::use_key() const
{
    need_keys_added();
    return _imp->use;
}

const tr1::shared_ptr<const MetadataCollectionKey<IUseFlagCollection> >
VDBID::iuse_key() const
{
    need_keys_added();
    return _imp->iuse;
}

const tr1::shared_ptr<const MetadataCollectionKey<InheritedCollection> >
VDBID::inherited_key() const
{
    need_keys_added();
    return _imp->inherited;
}

const tr1::shared_ptr<const MetadataSpecTreeKey<LicenseSpecTree> >
VDBID::license_key() const
{
    need_keys_added();
    return _imp->license;
}

const tr1::shared_ptr<const MetadataSpecTreeKey<ProvideSpecTree> >
VDBID::provide_key() const
{
    need_keys_added();
    return _imp->provide;
}

const tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
VDBID::build_dependencies_key() const
{
    need_keys_added();
    return _imp->build_dependencies;
}

const tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
VDBID::run_dependencies_key() const
{
    need_keys_added();
    return _imp->run_dependencies;
}

const tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
VDBID::post_dependencies_key() const
{
    need_keys_added();
    return _imp->post_dependencies;
}

const tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
VDBID::suggested_dependencies_key() const
{
    return tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >();
}

const tr1::shared_ptr<const MetadataSpecTreeKey<RestrictSpecTree> >
VDBID::restrict_key() const
{
    need_keys_added();
    return _imp->restrictions;
}

const tr1::shared_ptr<const MetadataSpecTreeKey<URISpecTree> >
VDBID::src_uri_key() const
{
    need_keys_added();
    return _imp->src_uri;
}

const tr1::shared_ptr<const MetadataSpecTreeKey<URISpecTree> >
VDBID::homepage_key() const
{
    need_keys_added();
    return _imp->homepage;
}

const tr1::shared_ptr<const MetadataSpecTreeKey<URISpecTree> >
VDBID::bin_uri_key() const
{
    return tr1::shared_ptr<const MetadataSpecTreeKey<URISpecTree> >();
}

const tr1::shared_ptr<const MetadataStringKey>
VDBID::short_description_key() const
{
    need_keys_added();
    return _imp->short_description;
}

const tr1::shared_ptr<const MetadataStringKey>
VDBID::long_description_key() const
{
    return tr1::shared_ptr<const MetadataStringKey>();
}

bool
VDBID::arbitrary_less_than_comparison(const PackageID &) const
{
    return false;
}

std::size_t
VDBID::extra_hash_value() const
{
    return 0;
}

