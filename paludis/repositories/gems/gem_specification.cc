/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010 Ciaran McCreesh
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

#include <paludis/repositories/gems/gem_specification.hh>
#include <paludis/repositories/gems/yaml.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/name.hh>
#include <paludis/version_spec.hh>
#include <paludis/repository.hh>
#include <paludis/metadata_key.hh>
#include <paludis/action.hh>
#include <paludis/environment.hh>
#include <paludis/literal_metadata_key.hh>
#include <paludis/mask.hh>
#include <paludis/user_dep_spec.hh>
#include <tr1/functional>

using namespace paludis;
using namespace paludis::gems;

namespace paludis
{
    template <>
    struct Implementation<GemSpecification>
    {
        mutable Mutex mutex;

        std::string name_part;
        std::string version;
        std::string date;
        std::string platform;
        std::string homepage;

        std::tr1::shared_ptr<LiteralMetadataValueKey<std::string> > description_key;
        std::tr1::shared_ptr<LiteralMetadataValueKey<std::string> > summary_key;
        std::tr1::shared_ptr<LiteralMetadataValueKey<std::string> > authors_key;
        std::tr1::shared_ptr<LiteralMetadataValueKey<std::string> > rubyforge_project_key;
        std::tr1::shared_ptr<LiteralMetadataValueKey<FSEntry> > fs_location_key;
        std::tr1::shared_ptr<LiteralMetadataValueKey<SlotName> > slot_key;

        std::tr1::shared_ptr<const FSEntry> load_from_file;

        const Environment * const environment;
        const std::tr1::shared_ptr<const Repository> repository;

        mutable bool has_masks;

        Implementation(const Environment * const e, const std::tr1::shared_ptr<const Repository> & r) :
            environment(e),
            repository(r),
            has_masks(false)
        {
        }
    };
}

namespace
{
    std::string extract_text_only(const yaml::Node & n, const std::string & extra);

    struct VersionVisitor
    {
        std::string text;

        void visit(const yaml::StringNode & n) PALUDIS_ATTRIBUTE((noreturn));
        void visit(const yaml::SequenceNode & n) PALUDIS_ATTRIBUTE((noreturn));

        void visit(const yaml::MapNode & n)
        {
            yaml::MapNode::ConstIterator i(n.find("version"));
            if (i == n.end())
                throw BadSpecificationError("Version has no version: key");
            text = extract_text_only(*i->second, "for Version version: key");
        }
    };

    void VersionVisitor::visit(const yaml::StringNode &)
    {
        throw BadSpecificationError("Version child node is string, not map");
    }

    void VersionVisitor::visit(const yaml::SequenceNode &)
    {
        throw BadSpecificationError("Version child node is sequence, not map");
    }

    struct ExtractTextVisitor
    {
        const std::string extra;
        const bool accept_sequence;
        std::string result;

        ExtractTextVisitor(const std::string & s, const bool b) :
            extra(s),
            accept_sequence(b)
        {
        }

        void visit(const yaml::StringNode & n)
        {
            result = n.text();
        }

        void visit(const yaml::SequenceNode & s)
        {
            if (! accept_sequence)
                throw BadSpecificationError("Found sequence rather than text " + extra);

            bool w(false);
            for (yaml::SequenceNode::ConstIterator i(s.begin()), i_end(s.end()) ; i != i_end ; ++i)
            {
                if (w)
                    result.append(", ");
                result.append(extract_text_only(**i, extra));
                w = true;
            }
        }

        void visit(const yaml::MapNode &) PALUDIS_ATTRIBUTE((noreturn));
    };

    void ExtractTextVisitor::visit(const yaml::MapNode &)
    {
        throw BadSpecificationError("Found map rather than text " + extra);
    }

    std::string extract_text_only(const yaml::Node & n, const std::string & extra)
    {
        ExtractTextVisitor v(extra, false);
        n.accept(v);
        return v.result;
    }

    std::string extract_text_sequence(const yaml::Node & n, const std::string & extra)
    {
        ExtractTextVisitor v(extra, true);
        n.accept(v);
        return v.result;
    }

    std::string required_text_only_key(const yaml::MapNode & n, const std::string & k)
    {
        yaml::MapNode::ConstIterator i(n.find(k));
        if (i == n.end())
            throw BadSpecificationError("Key '" + k + "' not defined");
        return extract_text_only(*i->second, "for key '" + k + "'");
    }

    std::string optional_text_sequence_key(const yaml::MapNode & n, const std::string & k)
    {
        yaml::MapNode::ConstIterator i(n.find(k));
        if (i == n.end())
            return "";
        return extract_text_sequence(*i->second, "for key '" + k + "'");
    }

    std::string optional_text_only_key(const yaml::MapNode & n, const std::string & k)
    {
        yaml::MapNode::ConstIterator i(n.find(k));
        if (i == n.end())
            return "";
        return extract_text_only(*i->second, "for key '" + k + "'");
    }

    std::string required_version(const yaml::MapNode & n, const std::string & k)
    {
        yaml::MapNode::ConstIterator i(n.find(k));
        if (i == n.end())
            throw BadSpecificationError("Key '" + k + "' not defined");

        VersionVisitor v;
        i->second->accept(v);
        return v.text;
    }

    struct TopVisitor
    {
        Implementation<GemSpecification> * const _imp;

        TopVisitor(Implementation<GemSpecification> * const i) :
            _imp(i)
        {
        }

        void visit(const yaml::MapNode & n)
        {
            std::string summary(required_text_only_key(n, "summary"));
            if (! summary.empty())
                _imp->summary_key.reset(new LiteralMetadataValueKey<std::string> ("summary", "Summary", mkt_significant, summary));

            std::string description(optional_text_only_key(n, "description"));
            if (! description.empty())
                _imp->description_key.reset(new LiteralMetadataValueKey<std::string> ("description", "Description", mkt_normal, description));

            std::string authors(optional_text_sequence_key(n, "authors"));
            if (! authors.empty())
                _imp->authors_key.reset(new LiteralMetadataValueKey<std::string> ("authors", "Authors", mkt_normal, authors));

            std::string rubyforge_project(optional_text_sequence_key(n, "rubyforge_project"));
            if (! rubyforge_project.empty())
                _imp->rubyforge_project_key.reset(new LiteralMetadataValueKey<std::string> ("rubyforge_project", "Rubyforge Project",
                            mkt_normal, rubyforge_project));

            _imp->date = required_text_only_key(n, "date");
            _imp->platform = required_text_only_key(n, "platform");
            _imp->name_part = required_text_only_key(n, "name");
            _imp->version = required_version(n, "version");
        }

        void visit(const yaml::SequenceNode & n) PALUDIS_ATTRIBUTE((noreturn));

        void visit(const yaml::StringNode & n) PALUDIS_ATTRIBUTE((noreturn));
    };

    void TopVisitor::visit(const yaml::SequenceNode &)
    {
        throw BadSpecificationError("Top level node is sequence, not map");
    }

    void TopVisitor::visit(const yaml::StringNode & n)
    {
        throw BadSpecificationError("Top level node is text '" + n.text() + "', not map");
    }
}

GemSpecification::GemSpecification(const Environment * const e,
        const std::tr1::shared_ptr<const Repository> & r, const yaml::Node & node) :
    PrivateImplementationPattern<GemSpecification>(new Implementation<GemSpecification>(e, r)),
    _imp(PrivateImplementationPattern<GemSpecification>::_imp)
{
    TopVisitor v(_imp.get());
    node.accept(v);

    if (_imp->summary_key)
        add_metadata_key(_imp->summary_key);

    if (_imp->description_key)
        add_metadata_key(_imp->description_key);

    if (_imp->authors_key)
        add_metadata_key(_imp->authors_key);

    if (_imp->rubyforge_project_key)
        add_metadata_key(_imp->rubyforge_project_key);
}


GemSpecification::GemSpecification(const Environment * const e, const std::tr1::shared_ptr<const Repository> & r,
        const PackageNamePart & q, const VersionSpec & v, const FSEntry & f) :
    PrivateImplementationPattern<GemSpecification>(new Implementation<GemSpecification>(e, r)),
    _imp(PrivateImplementationPattern<GemSpecification>::_imp)
{
    _imp->name_part = stringify(q);
    _imp->version = stringify(v);
    _imp->load_from_file.reset(new FSEntry(f));
    _imp->fs_location_key.reset(new LiteralMetadataValueKey<FSEntry> ("GEM", "Gem Location", mkt_internal, f));
    add_metadata_key(_imp->fs_location_key);
    _imp->slot_key.reset(new LiteralMetadataValueKey<SlotName>("SLOT", "Slot", mkt_internal, SlotName(stringify(v))));
    add_metadata_key(_imp->slot_key);
}

GemSpecification::~GemSpecification()
{
}

BadSpecificationError::BadSpecificationError(const std::string & s) throw () :
    Exception("Bad gem specification: " + s)
{
}

const std::string
GemSpecification::canonical_form(const PackageIDCanonicalForm f) const
{
    switch (f)
    {
        case idcf_full:
            return stringify(name()) + "-" + stringify(version()) + "::" + stringify(repository()->name());

        case idcf_version:
            return stringify(version());

        case idcf_no_version:
            return stringify(name()) + "::" + stringify(_imp->repository->name());

        case last_idcf:
            break;
    }

    throw InternalError(PALUDIS_HERE, "Bad PackageIDCanonicalForm");
}

PackageDepSpec
GemSpecification::uniquely_identifying_spec() const
{
    return parse_user_package_dep_spec("=" + stringify(name()) + "-" + stringify(version()) + "::" + stringify(repository()->name()),
            _imp->environment, UserPackageDepSpecOptions());
}

const QualifiedPackageName
GemSpecification::name() const
{
    return QualifiedPackageName(CategoryNamePart("gems") + PackageNamePart(_imp->name_part));
}

const VersionSpec
GemSpecification::version() const
{
    return VersionSpec(_imp->version, VersionSpecOptions());
}

const std::tr1::shared_ptr<const Repository>
GemSpecification::repository() const
{
    return _imp->repository;
}

const std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const PackageID> > >
GemSpecification::virtual_for_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const PackageID> > >();
}

const std::tr1::shared_ptr<const MetadataCollectionKey<KeywordNameSet> >
GemSpecification::keywords_key() const
{
    return std::tr1::shared_ptr<const MetadataCollectionKey<KeywordNameSet> >();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<ProvideSpecTree> >
GemSpecification::provide_key() const
{
    return std::tr1::shared_ptr<const MetadataSpecTreeKey<ProvideSpecTree> >();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
GemSpecification::dependencies_key() const
{
    return std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
GemSpecification::build_dependencies_key() const
{
    return std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
GemSpecification::run_dependencies_key() const
{
    return std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
GemSpecification::post_dependencies_key() const
{
    return std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<FetchableURISpecTree> >
GemSpecification::fetches_key() const
{
    return std::tr1::shared_ptr<const MetadataSpecTreeKey<FetchableURISpecTree> >();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> >
GemSpecification::homepage_key() const
{
    return std::tr1::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> >();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
GemSpecification::suggested_dependencies_key() const
{
    return std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >();
}

const std::tr1::shared_ptr<const MetadataValueKey<std::string> >
GemSpecification::short_description_key() const
{
    return _imp->summary_key;
}

const std::tr1::shared_ptr<const MetadataValueKey<std::string> >
GemSpecification::long_description_key() const
{
    return _imp->description_key;
}

const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> >
GemSpecification::fs_location_key() const
{
    return _imp->fs_location_key;
}

const std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const Contents> > >
GemSpecification::contents_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const Contents> > >();
}

const std::tr1::shared_ptr<const MetadataTimeKey>
GemSpecification::installed_time_key() const
{
    return std::tr1::shared_ptr<const MetadataTimeKey>();
}

const std::tr1::shared_ptr<const MetadataCollectionKey<Set<std::string> > >
GemSpecification::from_repositories_key() const
{
    return std::tr1::shared_ptr<const MetadataCollectionKey<Set<std::string> > >();
}

const std::tr1::shared_ptr<const MetadataCollectionKey<PackageIDSequence> >
GemSpecification::contains_key() const
{
    return std::tr1::shared_ptr<const MetadataCollectionKey<PackageIDSequence> >();
}

const std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const PackageID> > >
GemSpecification::contained_in_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const PackageID> > >();
}

const std::tr1::shared_ptr<const MetadataValueKey<bool> >
GemSpecification::transient_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<bool> >();
}

const std::tr1::shared_ptr<const MetadataValueKey<SlotName> >
GemSpecification::slot_key() const
{
    return _imp->slot_key;
}

bool
GemSpecification::arbitrary_less_than_comparison(const PackageID &) const
{
    return false;
}

std::size_t
GemSpecification::extra_hash_value() const
{
    return 0;
}

void
GemSpecification::need_keys_added() const
{
    if (_imp->load_from_file)
        throw InternalError(PALUDIS_HERE, "Got to do the load from file thing");
}

#if 0
InstalledGemsRepository::need_version_metadata(const QualifiedPackageName & q, const VersionSpec & v) const
{
    MetadataMap::const_iterator i(_imp->metadata.find(std::make_pair(q, v)));
    if (i != _imp->metadata.end())
        return;

    Context c("When loading version metadata for '" + stringify(PackageDatabaseEntry(q, v, name())) + "':");

    std::tr1::shared_ptr<gems::InstalledGemMetadata> m(new gems::InstalledGemMetadata(v));
    _imp->metadata.insert(std::make_pair(std::make_pair(q, v), m));

    Command cmd(getenv_with_default("PALUDIS_GEMS_DIR", LIBEXECDIR "/paludis") +
            "/gems/gems.bash specification '" + stringify(q.package) + "' '" + stringify(v) + "'");
    cmd.with_stderr_prefix(stringify(q) + "-" + stringify(v) + "::" + stringify(name()) + "> ");
    cmd.with_sandbox();
    cmd.with_uid_gid(_imp->params.environment->reduced_uid(), _imp->params.environment->reduced_gid());

    PStream p(cmd);
    std::string output((std::istreambuf_iterator<char>(p)), std::istreambuf_iterator<char>());

    if (0 != p.exit_status())
    {
        Log::get_instance()->message(ll_warning, lc_context) << "Version metadata extraction returned non-zero";
        return;
    }

    yaml::Document spec_doc(output);
    gems::GemSpecification spec(*spec_doc.top());
    m->populate_from_specification(spec);
    m->eapi = EAPIData::get_instance()->eapi_from_string("gems-1");
}
#endif

bool
GemSpecification::supports_action(const SupportsActionTestBase & b) const
{
    return repository()->some_ids_might_support_action(b);
}

namespace
{
    struct PerformAction
    {
        const PackageID * const id;

        PerformAction(const PackageID * const i) :
            id(i)
        {
        }

        void visit(const InstallAction & a)
        {
            SupportsActionTest<InstallAction> t;
            if (! id->repository()->some_ids_might_support_action(t))
                throw ActionFailedError("Unsupported action '" + a.simple_name() + "'");
        }

        void visit(const UninstallAction & a)
        {
            SupportsActionTest<UninstallAction> t;
            if (! id->repository()->some_ids_might_support_action(t))
                throw ActionFailedError("Unsupported action '" + a.simple_name() + "'");
        }

        void visit(const ConfigAction & a)
        {
            SupportsActionTest<ConfigAction> t;
            if (! id->repository()->some_ids_might_support_action(t))
                throw ActionFailedError("Unsupported action '" + a.simple_name() + "'");
        }

        void visit(const FetchAction & a)
        {
            SupportsActionTest<FetchAction> t;
            if (! id->repository()->some_ids_might_support_action(t))
                throw ActionFailedError("Unsupported action '" + a.simple_name() + "'");
        }

        void visit(const InfoAction & a)
        {
            SupportsActionTest<InfoAction> t;
            if (! id->repository()->some_ids_might_support_action(t))
                throw ActionFailedError("Unsupported action '" + a.simple_name() + "'");
        }

        void visit(const PretendAction & a)
        {
            SupportsActionTest<PretendAction> t;
            if (! id->repository()->some_ids_might_support_action(t))
                throw ActionFailedError("Unsupported action '" + a.simple_name() + "'");
        }

        void visit(const PretendFetchAction & a)
        {
            SupportsActionTest<PretendFetchAction> t;
            if (! id->repository()->some_ids_might_support_action(t))
                throw ActionFailedError("Unsupported action '" + a.simple_name() + "'");
        }
    };
}

void
GemSpecification::perform_action(Action & a) const
{
    PerformAction b(this);
    a.accept(b);
}

void
GemSpecification::need_masks_added() const
{
    Lock l(_imp->mutex);

    if (_imp->has_masks)
        return;

    _imp->has_masks = true;

    Context context("When generating masks for ID '" + canonical_form(idcf_full) + "':");

    if (! _imp->environment->unmasked_by_user(*this))
    {
        /* user */
        std::tr1::shared_ptr<const Mask> user_mask(_imp->environment->mask_for_user(*this, false));
        if (user_mask)
            add_mask(user_mask);
    }
    else
    {
        std::tr1::shared_ptr<const Mask> user_mask(_imp->environment->mask_for_user(*this, true));
        if (user_mask)
            add_overridden_mask(make_shared_ptr(new OverriddenMask(
                            make_named_values<OverriddenMask>(
                                value_for<n::mask>(user_mask),
                                value_for<n::override_reason>(mro_overridden_by_user)
                                ))));
    }

    /* break portage */
    std::tr1::shared_ptr<const Mask> breaks_mask(_imp->environment->mask_for_breakage(*this));
    if (breaks_mask)
        add_mask(breaks_mask);
}

void
GemSpecification::invalidate_masks() const
{
    Lock l(_imp->mutex);

    if (! _imp->has_masks)
        return;

    _imp->has_masks = false;
    PackageID::invalidate_masks();
}

std::tr1::shared_ptr<const Set<std::string> >
GemSpecification::breaks_portage() const
{
    std::tr1::shared_ptr<Set<std::string> > why(new Set<std::string>);
    why->insert("format");
    return why;
}

const std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const Choices> > >
GemSpecification::choices_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const Choices> > >();
}

