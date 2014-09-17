/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010, 2011, 2014 Ciaran McCreesh
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
#include <paludis/repositories/gemcutter/gemcutter_uri_key.hh>
#include <paludis/repositories/gemcutter/gemcutter_dependencies_key.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/visitor_cast.hh>
#include <paludis/util/set.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/singleton-impl.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/name.hh>
#include <paludis/version_spec.hh>
#include <paludis/metadata_key.hh>
#include <paludis/literal_metadata_key.hh>
#include <paludis/action.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/slot.hh>

using namespace paludis;
using namespace paludis::gemcutter_repository;

namespace
{
    std::shared_ptr<LiteralMetadataValueKey<std::string> > make_string_key(
            const std::string & r, const std::string & h, const MetadataKeyType t, const std::string & v)
    {
        if (v.empty())
            return nullptr;
        else
            return std::make_shared<LiteralMetadataValueKey<std::string> >(r, h, t, v);
    }

    std::shared_ptr<GemcutterURIKey> make_uri(
            const std::string * const r, const std::string * const h, const MetadataKeyType t, const std::string & v)
    {
        if (v.empty())
            return nullptr;
        else
            return std::make_shared<GemcutterURIKey>(r, h, t, v);
    }

    std::shared_ptr<GemcutterDependenciesKey> make_dependencies(
            const Environment * const e,
            const std::string * const r, const std::string * const h, const MetadataKeyType t,
            const std::shared_ptr<const GemJSONDependencies> & dd,
            const std::shared_ptr<const GemJSONDependencies> & dr)
    {
        return std::make_shared<GemcutterDependenciesKey>(e, r, h, t, dd, dr);
    }

    struct GemcutterIDStrings :
        Singleton<GemcutterIDStrings>
    {
        const std::string bug_tracker_uri_raw;
        const std::string bug_tracker_uri_human;

        const std::string gem_uri_raw;
        const std::string gem_uri_human;

        const std::string homepage_uri_raw;
        const std::string homepage_uri_human;

        const std::string mailing_list_uri_raw;
        const std::string mailing_list_uri_human;

        const std::string project_uri_raw;
        const std::string project_uri_human;

        const std::string source_code_uri_raw;
        const std::string source_code_uri_human;

        const std::string wiki_uri_raw;
        const std::string wiki_uri_human;

        const std::string dependencies_raw;
        const std::string dependencies_human;

        GemcutterIDStrings() :
            bug_tracker_uri_raw("bug_tracker_uri"),
            bug_tracker_uri_human("Bug tracker URI"),
            gem_uri_raw("gem_uri"),
            gem_uri_human("Gem URI"),
            homepage_uri_raw("homepage_uri"),
            homepage_uri_human("Homepage"),
            mailing_list_uri_raw("mailing_list_uri"),
            mailing_list_uri_human("Mailing list URI"),
            project_uri_raw("project_uri"),
            project_uri_human("Project URI"),
            source_code_uri_raw("source_code_uri"),
            source_code_uri_human("Sourcecode URI"),
            wiki_uri_raw("wiki_uri"),
            wiki_uri_human("Wiki URI"),
            dependencies_raw("dependencies"),
            dependencies_human("Dependencies")
        {
        }
    };
}

namespace paludis
{
    template <>
    struct Imp<GemcutterID>
    {
        const Environment * const env;
        const QualifiedPackageName name;
        const VersionSpec version;
        const RepositoryName repository_name;

        std::shared_ptr<LiteralMetadataValueKey<Slot> > slot_key;
        std::shared_ptr<LiteralMetadataValueKey<std::string> > authors_key;
        std::shared_ptr<LiteralMetadataValueKey<std::string> > info_key;
        std::shared_ptr<GemcutterURIKey> bug_tracker_uri_key;
        std::shared_ptr<GemcutterURIKey> gem_uri_key;
        std::shared_ptr<GemcutterURIKey> homepage_uri_key;
        std::shared_ptr<GemcutterURIKey> mailing_list_uri_key;
        std::shared_ptr<GemcutterURIKey> project_uri_key;
        std::shared_ptr<GemcutterURIKey> source_code_uri_key;
        std::shared_ptr<GemcutterURIKey> wiki_uri_key;
        std::shared_ptr<GemcutterDependenciesKey> dependencies_key;

        Imp(const GemcutterIDParams & e) :
            env(e.environment()),
            name(CategoryNamePart("gem"), PackageNamePart(e.info().name())),
            version(e.info().version(), { }),
            repository_name(e.repository()),
            slot_key(std::make_shared<LiteralMetadataValueKey<Slot> >("slot", "Slot", mkt_internal, make_named_values<Slot>(
                            n::match_values() = std::make_pair(SlotName(e.info().version()), SlotName(e.info().version())),
                            n::parallel_value() = SlotName(e.info().version()),
                            n::raw_value() = e.info().version()))),
            authors_key(make_string_key("authors", "Authors", mkt_author, e.info().authors())),
            info_key(make_string_key("info", "Info", mkt_significant, e.info().info())),
            bug_tracker_uri_key(make_uri(
                        &GemcutterIDStrings::get_instance()->bug_tracker_uri_raw,
                        &GemcutterIDStrings::get_instance()->bug_tracker_uri_human,
                        mkt_normal,
                        e.info().bug_tracker_uri())),
            gem_uri_key(make_uri(
                        &GemcutterIDStrings::get_instance()->gem_uri_raw,
                        &GemcutterIDStrings::get_instance()->gem_uri_human,
                        mkt_normal,
                        e.info().gem_uri())),
            homepage_uri_key(make_uri(
                        &GemcutterIDStrings::get_instance()->homepage_uri_raw,
                        &GemcutterIDStrings::get_instance()->homepage_uri_human,
                        mkt_significant,
                        e.info().homepage_uri())),
            mailing_list_uri_key(make_uri(
                        &GemcutterIDStrings::get_instance()->mailing_list_uri_raw,
                        &GemcutterIDStrings::get_instance()->mailing_list_uri_human,
                        mkt_normal,
                        e.info().mailing_list_uri())),
            project_uri_key(make_uri(
                        &GemcutterIDStrings::get_instance()->project_uri_raw,
                        &GemcutterIDStrings::get_instance()->project_uri_human,
                        mkt_normal,
                        e.info().project_uri())),
            source_code_uri_key(make_uri(
                        &GemcutterIDStrings::get_instance()->source_code_uri_raw,
                        &GemcutterIDStrings::get_instance()->source_code_uri_human,
                        mkt_normal,
                        e.info().source_code_uri())),
            wiki_uri_key(make_uri(
                        &GemcutterIDStrings::get_instance()->wiki_uri_raw,
                        &GemcutterIDStrings::get_instance()->wiki_uri_human,
                        mkt_normal,
                        e.info().wiki_uri())),
            dependencies_key(make_dependencies(
                        env,
                        &GemcutterIDStrings::get_instance()->dependencies_raw,
                        &GemcutterIDStrings::get_instance()->dependencies_human,
                        mkt_dependencies,
                        e.info().development_dependencies(),
                        e.info().runtime_dependencies()
                        ))
        {
        }
    };
}

GemcutterID::GemcutterID(const GemcutterIDParams & entry) :
    _imp(entry)
{
    add_metadata_key(_imp->slot_key);

    if (_imp->authors_key)
        add_metadata_key(_imp->authors_key);
    if (_imp->info_key)
        add_metadata_key(_imp->info_key);
    if (_imp->bug_tracker_uri_key)
        add_metadata_key(_imp->bug_tracker_uri_key);
    if (_imp->gem_uri_key)
        add_metadata_key(_imp->gem_uri_key);
    if (_imp->homepage_uri_key)
        add_metadata_key(_imp->homepage_uri_key);
    if (_imp->mailing_list_uri_key)
        add_metadata_key(_imp->mailing_list_uri_key);
    if (_imp->project_uri_key)
        add_metadata_key(_imp->project_uri_key);
    if (_imp->source_code_uri_key)
        add_metadata_key(_imp->source_code_uri_key);
    if (_imp->wiki_uri_key)
        add_metadata_key(_imp->wiki_uri_key);
    if (_imp->dependencies_key)
        add_metadata_key(_imp->dependencies_key);
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
                "::" + stringify(_imp->repository_name);

        case idcf_no_version:
            return stringify(_imp->name) + "::" + stringify(_imp->repository_name);

        case idcf_version:
            return stringify(_imp->version);

        case idcf_no_name:
            return stringify(_imp->version) + "::" + stringify(_imp->repository_name);

        case last_idcf:
            break;
    }

    throw InternalError(PALUDIS_HERE, "Bad PackageIDCanonicalForm");
}

PackageDepSpec
GemcutterID::uniquely_identifying_spec() const
{
    return parse_user_package_dep_spec("=" + stringify(name()) + "-" + stringify(version()) +
            + "::" + stringify(repository_name()),
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

const RepositoryName
GemcutterID::repository_name() const
{
    return _imp->repository_name;
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

const std::shared_ptr<const MetadataValueKey<FSPath> >
GemcutterID::fs_location_key() const
{
    return std::shared_ptr<const MetadataValueKey<FSPath> >();
}

const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > >
GemcutterID::behaviours_key() const
{
    return nullptr;
}

const std::shared_ptr<const MetadataCollectionKey<KeywordNameSet> >
GemcutterID::keywords_key() const
{
    return std::shared_ptr<const MetadataCollectionKey<KeywordNameSet> >();
}

const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
GemcutterID::dependencies_key() const
{
    return _imp->dependencies_key;
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

const std::shared_ptr<const MetadataValueKey<std::string> >
GemcutterID::short_description_key() const
{
    return _imp->info_key;
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
    return _imp->homepage_uri_key;
}

const std::shared_ptr<const MetadataTimeKey>
GemcutterID::installed_time_key() const
{
    return std::shared_ptr<const MetadataTimeKey>();
}

const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > >
GemcutterID::from_repositories_key() const
{
    return nullptr;
}

const std::shared_ptr<const MetadataValueKey<std::shared_ptr<const Choices> > >
GemcutterID::choices_key() const
{
    return nullptr;
}

const std::shared_ptr<const MetadataValueKey<Slot> >
GemcutterID::slot_key() const
{
    return _imp->slot_key;
}

const std::shared_ptr<const Contents>
GemcutterID::contents() const
{
    return nullptr;
}

namespace paludis
{
    template class Pimp<GemcutterID>;
}

