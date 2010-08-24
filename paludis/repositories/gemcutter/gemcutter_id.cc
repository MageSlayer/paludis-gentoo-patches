/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010 Ciaran McCreesh
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

#include <paludis/repositories/gemcutter/gemcutter_id.hh>
#include <paludis/repositories/gemcutter/gemcutter_repository.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/simple_visitor_cast.hh>
#include <paludis/util/set.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/util/singleton-impl.hh>
#include <paludis/name.hh>
#include <paludis/version_spec.hh>
#include <paludis/metadata_key.hh>
#include <paludis/literal_metadata_key.hh>
#include <paludis/action.hh>
#include <paludis/unchoices_key.hh>
#include <paludis/user_dep_spec.hh>

using namespace paludis;
using namespace paludis::gemcutter_repository;

namespace paludis
{
    template <>
    struct Imp<GemcutterID>
    {
        const Environment * const env;
        const QualifiedPackageName name;
        const VersionSpec version;
        const GemcutterRepository * const repo;

        Imp(const GemcutterIDParams & e) :
            env(e.environment()),
            name(CategoryNamePart("gem"), PackageNamePart(e.info().name())),
            version(e.info().version(), { }),
            repo(e.repository())
        {
        }
    };
}

GemcutterID::GemcutterID(const GemcutterIDParams & entry) :
    Pimp<GemcutterID>(entry),
    _imp(Pimp<GemcutterID>::_imp)
{
}

GemcutterID::~GemcutterID()
{
}

void
GemcutterID::need_keys_added() const
{
}

void
GemcutterID::need_masks_added() const
{
}

const std::string
GemcutterID::canonical_form(const PackageIDCanonicalForm f) const
{
    switch (f)
    {
        case idcf_full:
            return stringify(_imp->name) + "-" + stringify(_imp->version) +
                "::" + stringify(_imp->repo->name());

        case idcf_no_version:
            return stringify(_imp->name) + "::" + stringify(_imp->repo->name());

        case idcf_version:
            return stringify(_imp->version);

        case idcf_no_name:
            return stringify(_imp->version) + "::" + stringify(_imp->repo->name());

        case last_idcf:
            break;
    }

    throw InternalError(PALUDIS_HERE, "Bad PackageIDCanonicalForm");
}

PackageDepSpec
GemcutterID::uniquely_identifying_spec() const
{
    return parse_user_package_dep_spec("=" + stringify(name()) + "-" + stringify(version()) +
            + "::" + stringify(repository()->name()),
            _imp->env, { });
}

const QualifiedPackageName
GemcutterID::name() const
{
    return _imp->name;
}

const VersionSpec
GemcutterID::version() const
{
    return _imp->version;
}

const std::shared_ptr<const Repository>
GemcutterID::repository() const
{
    return _imp->repo->shared_from_this();
}

bool
GemcutterID::supports_action(const SupportsActionTestBase &) const
{
    return false;
}

void
GemcutterID::perform_action(Action & a) const
{
    throw ActionFailedError("Unsupported action: " + a.simple_name());
}

std::shared_ptr<const Set<std::string> >
GemcutterID::breaks_portage() const
{
    return std::make_shared<Set<std::string>>();
}

bool
GemcutterID::arbitrary_less_than_comparison(const PackageID &) const
{
    return false;
}

std::size_t
GemcutterID::extra_hash_value() const
{
    return 0;
}

const std::shared_ptr<const MetadataCollectionKey<PackageIDSequence> >
GemcutterID::contains_key() const
{
    return std::shared_ptr<const MetadataCollectionKey<PackageIDSequence> >();
}

const std::shared_ptr<const MetadataValueKey<std::shared_ptr<const PackageID> > >
GemcutterID::contained_in_key() const
{
    return std::shared_ptr<const MetadataValueKey<std::shared_ptr<const PackageID> > >();
}

const std::shared_ptr<const MetadataValueKey<FSPath> >
GemcutterID::fs_location_key() const
{
    return std::shared_ptr<const MetadataValueKey<FSPath> >();
}

const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > >
GemcutterID::behaviours_key() const
{
    return make_null_shared_ptr();
}

const std::shared_ptr<const MetadataValueKey<std::shared_ptr<const PackageID> > >
GemcutterID::virtual_for_key() const
{
    return std::shared_ptr<const MetadataValueKey<std::shared_ptr<const PackageID> > >();
}

const std::shared_ptr<const MetadataCollectionKey<KeywordNameSet> >
GemcutterID::keywords_key() const
{
    return std::shared_ptr<const MetadataCollectionKey<KeywordNameSet> >();
}

const std::shared_ptr<const MetadataSpecTreeKey<ProvideSpecTree> >
GemcutterID::provide_key() const
{
    return std::shared_ptr<const MetadataSpecTreeKey<ProvideSpecTree> >();
}

const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
GemcutterID::dependencies_key() const
{
    return std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >();
}

const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
GemcutterID::build_dependencies_key() const
{
    return std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >();
}

const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
GemcutterID::run_dependencies_key() const
{
    return std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >();
}

const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
GemcutterID::post_dependencies_key() const
{
    return std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >();
}

const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
GemcutterID::suggested_dependencies_key() const
{
    return std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >();
}

const std::shared_ptr<const MetadataValueKey<std::string> >
GemcutterID::short_description_key() const
{
    return make_null_shared_ptr();
}

const std::shared_ptr<const MetadataValueKey<std::string> >
GemcutterID::long_description_key() const
{
    return std::shared_ptr<const MetadataValueKey<std::string> >();
}

const std::shared_ptr<const MetadataSpecTreeKey<FetchableURISpecTree> >
GemcutterID::fetches_key() const
{
    return std::shared_ptr<const MetadataSpecTreeKey<FetchableURISpecTree> >();
}

const std::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> >
GemcutterID::homepage_key() const
{
    return std::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> >();
}

const std::shared_ptr<const MetadataValueKey<std::shared_ptr<const Contents> > >
GemcutterID::contents_key() const
{
    return std::shared_ptr<const MetadataValueKey<std::shared_ptr<const Contents> > >();
}

const std::shared_ptr<const MetadataTimeKey>
GemcutterID::installed_time_key() const
{
    return std::shared_ptr<const MetadataTimeKey>();
}

const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > >
GemcutterID::from_repositories_key() const
{
    return make_null_shared_ptr();
}

const std::shared_ptr<const MetadataValueKey<std::shared_ptr<const Choices> > >
GemcutterID::choices_key() const
{
    return make_null_shared_ptr();
}

const std::shared_ptr<const MetadataValueKey<SlotName> >
GemcutterID::slot_key() const
{
    return make_null_shared_ptr();
}

template class Pimp<GemcutterID>;

