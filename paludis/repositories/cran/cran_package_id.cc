/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Danny van Dyk
 * Copyright (c) 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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
#include <paludis/repositories/cran/cran_repository.hh>
#include <paludis/repositories/cran/cran_installed_repository.hh>
#include <paludis/util/config_file.hh>
#include <paludis/repository.hh>
#include <paludis/util/log.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/return_literal_function.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/util/fs_stat.hh>
#include <paludis/literal_metadata_key.hh>
#include <paludis/name.hh>
#include <paludis/version_spec.hh>
#include <paludis/action.hh>
#include <paludis/dep_label.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/environment.hh>
#include <paludis/package_database.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/singleton-impl.hh>
#include <paludis/always_enabled_dependency_label.hh>
#include <string>
#include <algorithm>
#include <list>
#include <functional>

using namespace paludis;
using namespace paludis::cranrepository;

namespace
{
    struct CRANPackageIDData :
        Singleton<CRANPackageIDData>
    {
        std::shared_ptr<DependenciesLabelSequence> suggests_labels;
        std::shared_ptr<DependenciesLabelSequence> depends_labels;

        CRANPackageIDData() :
            suggests_labels(std::make_shared<DependenciesLabelSequence>()),
            depends_labels(std::make_shared<DependenciesLabelSequence>())
        {
            suggests_labels->push_back(std::make_shared<AlwaysEnabledDependencyLabel<DependenciesSuggestionLabelTag> >("Suggests"));
            depends_labels->push_back(std::make_shared<AlwaysEnabledDependencyLabel<DependenciesBuildLabelTag> >("Depends"));
        }
    };
}

namespace paludis
{
    template <>
    struct Imp<CRANPackageID>
    {
        const Environment * const env;
        const RepositoryName repository_name;

        QualifiedPackageName name;
        VersionSpec version;

        std::shared_ptr<LiteralMetadataValueKey<FSPath> > fs_location_key;
        std::shared_ptr<LiteralMetadataValueKey<std::string> > url_key;
        std::shared_ptr<LiteralMetadataValueKey<std::string> > short_description_key;
        std::shared_ptr<LiteralMetadataValueKey<std::string> > long_description_key;
        std::shared_ptr<LiteralMetadataValueKey<std::string> > license_key;
        std::shared_ptr<PackageIDKey> contained_in_key;
        std::shared_ptr<PackageIDSequenceKey> contains_key;
        std::shared_ptr<DepKey> depends_key;
        std::shared_ptr<DepKey> suggests_key;

        Imp(const Environment * const e, const RepositoryName & r, const FSPath & f) :
            env(e),
            repository_name(r),
            name("cran/" + cran_name_to_internal(strip_trailing_string(f.basename(), ".DESCRIPTION"))),
            version("0", { })
        {
        }

        Imp(const Environment * const e, const RepositoryName & c, const CRANPackageID * const r, const std::string & t) :
            env(e),
            repository_name(c),
            name("cran/" + cran_name_to_internal(t)),
            version(r->version()),
            contained_in_key(std::make_shared<PackageIDKey>(e, "Contained", "Contained in", r, mkt_normal))
        {
        }
    };
}

CRANPackageID::CRANPackageID(const Environment * const env, const RepositoryName & r, const FSPath & f) :
    _imp(env, r, f)
{
    Context context("When parsing file '" + stringify(f) + "' to create a CRAN Package ID:");

    if (! f.stat().is_regular_file())
    {
        add_mask(std::make_shared<BrokenMask>('B', "Broken", "DESCRIPTION file not a file"));
        Log::get_instance()->message("cran.id.not_a_file", ll_warning, lc_context) << "Unexpected irregular file: '" << stringify(f) << "'";
        return;
    }

    _imp->fs_location_key = std::make_shared<LiteralMetadataValueKey<FSPath> >("DescriptionFileLocation", "Description File Location",
                mkt_internal, f);
    add_metadata_key(_imp->fs_location_key);

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
            Log::get_instance()->message("cran.id.broken", ll_warning, lc_context) << "No Package: or Bundle: key in '" << stringify(f) << "'";
            add_mask(std::make_shared<BrokenMask>('B', "Broken", "No Package: or Bundle: key"));
            return;
        }

        if (file.get("Version").empty())
        {
            Context local_context("When handling Version: key:");
            Log::get_instance()->message("cran.id.broken", ll_warning, lc_context) << "No Version: key in '" << stringify(f) << "'";
            _imp->version = VersionSpec("0", { });
            add_mask(std::make_shared<BrokenMask>('B', "Broken", "No Version: key"));
            return;
        }
        else
        {
            Context local_context("When handling Version: key:");
            _imp->version = VersionSpec(cran_version_to_internal(file.get("Version")), { });
        }

        if (! file.get("License").empty())
        {
            /* License often isn't in the format the spec requires, so we can't parse it. */
            Context local_context("When handling License: key:");
            _imp->license_key = std::make_shared<LiteralMetadataValueKey<std::string>>("License", "License", mkt_dependencies, file.get("License"));
            add_metadata_key(_imp->license_key);
        }

        if (! file.get("URL").empty())
        {
            /* URL is also in stupid formats */
            Context local_context("When handling URL: key:");
            _imp->url_key = std::make_shared<LiteralMetadataValueKey<std::string>>("URL", "URL", mkt_significant, file.get("URL"));
            add_metadata_key(_imp->url_key);
        }

        if (! file.get("Title").empty())
        {
            Context local_context("When handling Title: key:");
            _imp->short_description_key = std::make_shared<LiteralMetadataValueKey<std::string>>("Title", "Title", mkt_significant, file.get("Title"));
            add_metadata_key(_imp->short_description_key);
        }

        if (! file.get("Description").empty())
        {
            Context local_context("When handling Description: key:");
            _imp->long_description_key = std::make_shared<LiteralMetadataValueKey<std::string>>("Description", "Description", mkt_normal, file.get("Description"));
            add_metadata_key(_imp->long_description_key);
        }
        else if (! file.get("BundleDescription").empty())
        {
            Context local_context("When handling BundleDescription: key:");
            _imp->long_description_key = std::make_shared<LiteralMetadataValueKey<std::string>>("BundleDescription", "Bundle Description",
                        mkt_normal, file.get("BundleDescription"));
        }

        if (! file.get("Date").empty())
        {
            Context local_context("When handling Date: key:");
            /* no guarantee on the format */
            add_metadata_key(std::make_shared<LiteralMetadataValueKey<std::string>>("Date", "Date", mkt_normal, file.get("Date")));
        }

        if (! file.get("Author").empty())
        {
            Context local_context("When handling Author: key:");
            add_metadata_key(std::make_shared<LiteralMetadataValueKey<std::string>>("Author", "Author", mkt_author, file.get("Author")));
        }

        if (! file.get("Maintainer").empty())
        {
            Context local_context("When handling Maintainer: key:");
            add_metadata_key(std::make_shared<LiteralMetadataValueKey<std::string>>("Maintainer", "Maintainer", mkt_author, file.get("Maintainer")));
        }

        if (! file.get("Contains").empty())
        {
            Context local_context("When handling Contains: key:");
            std::list<std::string> tokens;
            tokenise_whitespace(file.get("Contains"), std::back_inserter(tokens));
            _imp->contains_key = std::make_shared<PackageIDSequenceKey>(_imp->env, "Contains", "Contains", mkt_normal);
            add_metadata_key(_imp->contains_key);
            for (std::list<std::string>::const_iterator t(tokens.begin()), t_end(tokens.end()) ;
                    t != t_end ; ++t)
            {
                if (*t != stringify(name().package()))
                    _imp->contains_key->push_back(std::make_shared<CRANPackageID>(_imp->env, r, this, *t));
                else
                {
                    /* yay CRAN... */
                    Log::get_instance()->message("cran.bundle.recursive", ll_qa, lc_context)
                        << "Bundle '" << stringify(*this) << "' contains itself, but is not a Klein bundle";
                }
            }
        }

        if (! file.get("Suggests").empty())
        {
            Context local_context("When handling Suggests: key:");
            _imp->suggests_key = std::make_shared<DepKey>(_imp->env, "Suggests", "Suggests", file.get("Suggests"),
                        CRANPackageIDData::get_instance()->suggests_labels, mkt_dependencies);
            add_metadata_key(_imp->suggests_key);
        }

        if (! file.get("SystemRequirements").empty())
        {
            Context local_context("When handling SystemRequirements: key:");
            add_metadata_key(std::make_shared<LiteralMetadataValueKey<std::string>>("SystemRequirements", "System Requirements", mkt_normal,
                            file.get("SystemRequirements")));
        }

        if (! file.get("Depends").empty())
        {
            Context local_context("When handling Depends: key:");
            _imp->depends_key = std::make_shared<DepKey>(_imp->env, "Depends", "Depends", file.get("Depends") + ", R",
                        CRANPackageIDData::get_instance()->depends_labels, mkt_dependencies);
        }
        else
            _imp->depends_key = std::make_shared<DepKey>(_imp->env, "Depends", "Depends", "R",
                    CRANPackageIDData::get_instance()->depends_labels, mkt_dependencies);
        add_metadata_key(_imp->depends_key);
    }
    catch (const InternalError &)
    {
        throw;
    }
    catch (const Exception & e)
    {
        Log::get_instance()->message("cran.id.broken", ll_warning, lc_context) << "Broken CRAN description file '" << stringify(f) << "': '"
            << e.message() << "' (" << e.what() << ")";
        add_mask(std::make_shared<BrokenMask>('B', "Broken", "Got exception '" + stringify(e.message()) + "' (" + e.what() + "')"));
    }
}

CRANPackageID::CRANPackageID(const Environment * const e,
        const RepositoryName & c, const CRANPackageID * const r, const std::string & t) :
    _imp(e, c, r, t)
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

const RepositoryName
CRANPackageID::repository_name() const
{
    return _imp->repository_name;
}

const std::shared_ptr<const MetadataValueKey<std::shared_ptr<const PackageID> > >
CRANPackageID::virtual_for_key() const
{
    return std::shared_ptr<const MetadataValueKey<std::shared_ptr<const PackageID> > >();
}

const std::shared_ptr<const MetadataCollectionKey<KeywordNameSet> >
CRANPackageID::keywords_key() const
{
    return std::shared_ptr<const MetadataCollectionKey<KeywordNameSet> >();
}

const std::shared_ptr<const MetadataSpecTreeKey<ProvideSpecTree> >
CRANPackageID::provide_key() const
{
    return std::shared_ptr<const MetadataSpecTreeKey<ProvideSpecTree> >();
}

const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
CRANPackageID::dependencies_key() const
{
    return std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >();
}

const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
CRANPackageID::build_dependencies_key() const
{
    return std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >();
}

const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
CRANPackageID::run_dependencies_key() const
{
    return std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >();
}

const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
CRANPackageID::post_dependencies_key() const
{
    return std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >();
}

const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
CRANPackageID::suggested_dependencies_key() const
{
    return std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >();
}

const std::shared_ptr<const MetadataSpecTreeKey<FetchableURISpecTree> >
CRANPackageID::fetches_key() const
{
    return std::shared_ptr<const MetadataSpecTreeKey<FetchableURISpecTree> >();
}

const std::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> >
CRANPackageID::homepage_key() const
{
    return std::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> >();
}

const std::shared_ptr<const MetadataValueKey<std::string> >
CRANPackageID::short_description_key() const
{
    return _imp->short_description_key;
}

const std::shared_ptr<const MetadataValueKey<std::string> >
CRANPackageID::long_description_key() const
{
    return _imp->long_description_key;
}

const std::shared_ptr<const MetadataValueKey<std::shared_ptr<const Contents> > >
CRANPackageID::contents_key() const
{
    return std::shared_ptr<const MetadataValueKey<std::shared_ptr<const Contents> > >();
}

const std::shared_ptr<const MetadataTimeKey>
CRANPackageID::installed_time_key() const
{
    return std::shared_ptr<const MetadataTimeKey>();
}

const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > >
CRANPackageID::from_repositories_key() const
{
    return std::shared_ptr<const MetadataCollectionKey<Set<std::string> > >();
}

const std::shared_ptr<const MetadataValueKey<SlotName> >
CRANPackageID::slot_key() const
{
    return make_null_shared_ptr();
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

std::shared_ptr<const Set<std::string> >
CRANPackageID::breaks_portage() const
{
    std::shared_ptr<Set<std::string> > why(std::make_shared<Set<std::string>>());
    why->insert("format");
    return why;
}

const std::string
CRANPackageID::canonical_form(const PackageIDCanonicalForm f) const
{
    switch (f)
    {
        case idcf_full:
            return stringify(_imp->name) + "-" + stringify(_imp->version) + "::" + stringify(_imp->repository_name);

        case idcf_version:
            return stringify(_imp->version);

        case idcf_no_version:
            return stringify(_imp->name) + "::" + stringify(_imp->repository_name);

        case idcf_no_name:
            return stringify(version()) + "::" + stringify(_imp->repository_name);

        case last_idcf:
            break;
    }

    throw InternalError(PALUDIS_HERE, "Bad PackageIDCanonicalForm");
}

PackageDepSpec
CRANPackageID::uniquely_identifying_spec() const
{
    return parse_user_package_dep_spec("=" + stringify(name()) + "-" + stringify(version()) + "::" + stringify(_imp->repository_name),
            _imp->env, { });
}

namespace
{
    struct SupportsActionQuery
    {
        bool is_installed;

        bool visit(const SupportsActionTest<FetchAction> &) const
        {
            return ! is_installed;
        }

        bool visit(const SupportsActionTest<PretendFetchAction> &) const
        {
            return ! is_installed;
        }

        bool visit(const SupportsActionTest<InstallAction> &) const
        {
            return ! is_installed;
        }

        bool visit(const SupportsActionTest<ConfigAction> &) const
        {
            return false;
        }

        bool visit(const SupportsActionTest<PretendAction> &) const
        {
            return false;
        }

        bool visit(const SupportsActionTest<UninstallAction> &) const
        {
            return false;
        }

        bool visit(const SupportsActionTest<InfoAction> &) const
        {
            return false;
        }
    };
}

bool
CRANPackageID::supports_action(const SupportsActionTestBase & b) const
{
    auto repo(_imp->env->package_database()->fetch_repository(repository_name()));
    SupportsActionQuery q{bool(repo->installed_root_key())};
    return b.accept_returning<bool>(q);
}

void
CRANPackageID::perform_action(Action & a) const
{
    throw ActionFailedError("Unsupported action '" + a.simple_name() + "'");
}

const std::shared_ptr<const MetadataCollectionKey<PackageIDSequence> >
CRANPackageID::contains_key() const
{
    return _imp->contains_key;
}

const std::shared_ptr<const MetadataValueKey<std::shared_ptr<const PackageID> > >
CRANPackageID::contained_in_key() const
{
    return _imp->contained_in_key;
}

const std::shared_ptr<const MetadataValueKey<FSPath> >
CRANPackageID::fs_location_key() const
{
    return _imp->fs_location_key;
}

const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > >
CRANPackageID::behaviours_key() const
{
    return make_null_shared_ptr();
}

const std::shared_ptr<const MetadataValueKey<std::shared_ptr<const Choices> > >
CRANPackageID::choices_key() const
{
    return std::shared_ptr<const MetadataValueKey<std::shared_ptr<const Choices> > >();
}

