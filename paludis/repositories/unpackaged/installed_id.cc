/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
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

#include <paludis/repositories/unpackaged/installed_id.hh>
#include <paludis/repositories/unpackaged/installed_repository.hh>

#include <paludis/util/pimp-impl.hh>
#include <paludis/util/system.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/safe_ifstream.hh>
#include <paludis/util/return_literal_function.hh>
#include <paludis/util/timestamp.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/util/singleton-impl.hh>
#include <paludis/util/fs_iterator.hh>
#include <paludis/util/fs_stat.hh>
#include <paludis/util/join.hh>

#include <paludis/ndbam.hh>
#include <paludis/ndbam_unmerger.hh>
#include <paludis/output_manager.hh>
#include <paludis/name.hh>
#include <paludis/version_spec.hh>
#include <paludis/contents.hh>
#include <paludis/environment.hh>
#include <paludis/metadata_key.hh>
#include <paludis/literal_metadata_key.hh>
#include <paludis/action.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/comma_separated_dep_parser.hh>
#include <paludis/comma_separated_dep_pretty_printer.hh>
#include <paludis/always_enabled_dependency_label.hh>
#include <paludis/pretty_printer.hh>
#include <paludis/call_pretty_printer.hh>

#include <functional>

using namespace paludis;
using namespace paludis::unpackaged_repositories;

namespace
{
    struct InstalledUnpackagedIDData :
        Singleton<InstalledUnpackagedIDData>
    {
        std::shared_ptr<Set<std::string> > behaviours_value;
        std::shared_ptr<LiteralMetadataStringSetKey> behaviours_key;

        std::shared_ptr<DependenciesLabelSequence> build_dependencies_labels;
        std::shared_ptr<DependenciesLabelSequence> run_dependencies_labels;

        InstalledUnpackagedIDData() :
            behaviours_value(std::make_shared<Set<std::string>>()),
            behaviours_key(std::make_shared<LiteralMetadataStringSetKey>("behaviours", "behaviours", mkt_internal, behaviours_value)),
            build_dependencies_labels(std::make_shared<DependenciesLabelSequence>()),
            run_dependencies_labels(std::make_shared<DependenciesLabelSequence>())
        {
            behaviours_value->insert("transient");

            build_dependencies_labels->push_back(std::make_shared<AlwaysEnabledDependencyLabel<DependenciesBuildLabelTag> >("build_dependencies"));
            run_dependencies_labels->push_back(std::make_shared<AlwaysEnabledDependencyLabel<DependenciesRunLabelTag> >("run_dependencies"));
        }
    };

    class InstalledUnpackagedFSPathKey :
        public MetadataValueKey<FSPath>
    {
        private:
            const FSPath _location;

        public:
            InstalledUnpackagedFSPathKey(const FSPath & l) :
                _location(l)
            {
            }

            const FSPath parse_value() const
            {
                return _location;
            }

            virtual const std::string raw_name() const PALUDIS_ATTRIBUTE((warn_unused_result))
            {
                return "location";
            }

            virtual const std::string human_name() const PALUDIS_ATTRIBUTE((warn_unused_result))
            {
                return "Location";
            }

            virtual MetadataKeyType type() const PALUDIS_ATTRIBUTE((warn_unused_result))
            {
                return mkt_internal;
            }
    };

    class InstalledUnpackagedContentsKey :
        public MetadataValueKey<std::shared_ptr<const Contents> >
    {
        private:
            const PackageID * const _id;
            const NDBAM * const _db;
            mutable Mutex _mutex;
            mutable std::shared_ptr<Contents> _v;

        public:
            InstalledUnpackagedContentsKey(const PackageID * const i, const NDBAM * const d) :
                _id(i),
                _db(d)
            {
            }

            const std::shared_ptr<const Contents> parse_value() const
            {
                Lock l(_mutex);
                if (_v)
                    return _v;

                using namespace std::placeholders;
                _v = std::make_shared<Contents>();
                _db->parse_contents(*_id,
                        std::bind(&Contents::add, _v.get(), std::placeholders::_1),
                        std::bind(&Contents::add, _v.get(), std::placeholders::_1),
                        std::bind(&Contents::add, _v.get(), std::placeholders::_1)
                        );
                return _v;
            }

            virtual const std::string raw_name() const PALUDIS_ATTRIBUTE((warn_unused_result))
            {
                return "contents";
            }

            virtual const std::string human_name() const PALUDIS_ATTRIBUTE((warn_unused_result))
            {
                return "Contents";
            }

            virtual MetadataKeyType type() const PALUDIS_ATTRIBUTE((warn_unused_result))
            {
                return mkt_internal;
            }
    };

    class InstalledUnpackagedTimeKey :
        public MetadataTimeKey
    {
        private:
            const Timestamp _time;

        public:
            InstalledUnpackagedTimeKey(const FSPath & f) :
                _time(f.stat().mtim())
            {
            }

            Timestamp parse_value() const
            {
                return _time;
            }

            virtual const std::string raw_name() const PALUDIS_ATTRIBUTE((warn_unused_result))
            {
                return "installed_time";
            }

            virtual const std::string human_name() const PALUDIS_ATTRIBUTE((warn_unused_result))
            {
                return "Installed time";
            }

            virtual MetadataKeyType type() const PALUDIS_ATTRIBUTE((warn_unused_result))
            {
                return mkt_normal;
            }
    };

    class InstalledUnpackagedStringKey :
        public MetadataValueKey<std::string>
    {
        private:
            mutable std::shared_ptr<const std::string> _v;
            mutable Mutex _mutex;
            const FSPath _f;

            const std::string _raw_name;
            const std::string _human_name;
            const MetadataKeyType _type;

        public:
            InstalledUnpackagedStringKey(const std::string & r, const std::string & h, const FSPath & f, const MetadataKeyType t) :
                _f(f),
                _raw_name(r),
                _human_name(h),
                _type(t)
            {
            }

            const std::string parse_value() const
            {
                Lock l(_mutex);
                if (_v)
                    return *_v;

                Context context("When reading '" + stringify(_f) + "' as an InstalledUnpackagedStringKey:");
                SafeIFStream f(_f);
                _v = std::make_shared<std::string>(
                            strip_trailing(std::string((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>()), "\n"));
                return *_v;
            }

            virtual const std::string raw_name() const PALUDIS_ATTRIBUTE((warn_unused_result))
            {
                return _raw_name;
            }

            virtual const std::string human_name() const PALUDIS_ATTRIBUTE((warn_unused_result))
            {
                return _human_name;
            }

            virtual MetadataKeyType type() const PALUDIS_ATTRIBUTE((warn_unused_result))
            {
                return _type;
            }
    };

    class InstalledUnpackagedStringSetKey :
        public MetadataCollectionKey<Set<std::string> >
    {
        private:
            mutable std::shared_ptr<Set<std::string> > _v;
            mutable Mutex _mutex;
            FSPathSequence _f;

            const std::string _raw_name;
            const std::string _human_name;
            const MetadataKeyType _type;

        public:
            InstalledUnpackagedStringSetKey(const std::string & r, const std::string & h, const MetadataKeyType t) :
                _raw_name(r),
                _human_name(h),
                _type(t)
            {
            }

            void add_source(const FSPath & f)
            {
                _f.push_back(f);
            }

            const std::shared_ptr<const Set<std::string> > parse_value() const
            {
                Lock l(_mutex);
                if (_v)
                    return _v;

                _v = std::make_shared<Set<std::string>>();
                for (FSPathSequence::ConstIterator a(_f.begin()), a_end(_f.end()) ;
                        a != a_end ; ++a)
                {
                    SafeIFStream f(*a);
                    _v->insert(strip_trailing(std::string((std::istreambuf_iterator<char>(f)),
                                    std::istreambuf_iterator<char>()), "\n"));
                }
                return _v;
            }

            virtual const std::string raw_name() const PALUDIS_ATTRIBUTE((warn_unused_result))
            {
                return _raw_name;
            }

            virtual const std::string human_name() const PALUDIS_ATTRIBUTE((warn_unused_result))
            {
                return _human_name;
            }

            virtual MetadataKeyType type() const PALUDIS_ATTRIBUTE((warn_unused_result))
            {
                return _type;
            }

            virtual const std::string pretty_print_value(
                    const PrettyPrinter & pretty_printer,
                    const PrettyPrintOptions &) const
            {
                auto v(parse_value());
                return join(v->begin(), v->end(), " ", CallPrettyPrinter(pretty_printer));
            }
    };

    class InstalledUnpackagedDependencyKey :
        public MetadataSpecTreeKey<DependencySpecTree>
    {
        private:
            const Environment * const _env;
            mutable std::shared_ptr<const DependencySpecTree> _v;
            mutable Mutex _mutex;
            const FSPath _f;
            const std::shared_ptr<const DependenciesLabelSequence> _labels;

            const std::string _raw_name;
            const std::string _human_name;
            const MetadataKeyType _type;

        public:
            InstalledUnpackagedDependencyKey(const Environment * const e,
                    const std::string & r, const std::string & h, const FSPath & f,
                    const std::shared_ptr<const DependenciesLabelSequence> & l, const MetadataKeyType t) :
                _env(e),
                _f(f),
                _labels(l),
                _raw_name(r),
                _human_name(h),
                _type(t)
            {
            }

            const std::shared_ptr<const DependencySpecTree> parse_value() const
            {
                Lock l(_mutex);
                if (_v)
                    return _v;

                Context context("When reading '" + stringify(_f) + "' as an InstalledUnpackagedDependencyKey:");

                SafeIFStream f(_f);
                _v = CommaSeparatedDepParser::parse(_env, strip_trailing(
                            std::string((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>()), "\n"));
                return _v;
            }

            const std::shared_ptr<const DependenciesLabelSequence> initial_labels() const
            {
                return _labels;
            }

            virtual const std::string raw_name() const PALUDIS_ATTRIBUTE((warn_unused_result))
            {
                return _raw_name;
            }

            virtual const std::string human_name() const PALUDIS_ATTRIBUTE((warn_unused_result))
            {
                return _human_name;
            }

            virtual MetadataKeyType type() const PALUDIS_ATTRIBUTE((warn_unused_result))
            {
                return _type;
            }

            virtual const std::string pretty_print_value(
                    const PrettyPrinter & printer,
                    const PrettyPrintOptions & options) const
            {
                CommaSeparatedDepPrettyPrinter p(printer, options);
                parse_value()->top()->accept(p);
                return p.result();
            }
    };
}

namespace paludis
{
    template <>
    struct Imp<InstalledUnpackagedID>
    {
        const Environment * const env;
        const QualifiedPackageName name;
        const VersionSpec version;
        const RepositoryName repository_name;
        const FSPath root;
        const NDBAM * const ndbam;

        std::shared_ptr<LiteralMetadataValueKey<SlotName> > slot_key;
        std::shared_ptr<InstalledUnpackagedFSPathKey> fs_location_key;
        std::shared_ptr<InstalledUnpackagedContentsKey> contents_key;
        std::shared_ptr<InstalledUnpackagedTimeKey> installed_time_key;
        std::shared_ptr<InstalledUnpackagedStringSetKey> from_repositories_key;
        std::shared_ptr<InstalledUnpackagedStringKey> description_key;
        std::shared_ptr<InstalledUnpackagedDependencyKey> build_dependencies_key;
        std::shared_ptr<InstalledUnpackagedDependencyKey> run_dependencies_key;
        std::shared_ptr<LiteralMetadataStringSetKey> behaviours_key;

        Imp(
                const Environment * const e,
                const PackageID * const id,
                const QualifiedPackageName & q,
                const VersionSpec & v,
                const SlotName & s,
                const RepositoryName & r,
                const FSPath & l,
                const FSPath & ro,
                const NDBAM * const d) :
            env(e),
            name(q),
            version(v),
            repository_name(r),
            root(ro),
            ndbam(d),
            slot_key(std::make_shared<LiteralMetadataValueKey<SlotName> >("slot", "Slot", mkt_internal, s)),
            fs_location_key(std::make_shared<InstalledUnpackagedFSPathKey>(l)),
            behaviours_key(InstalledUnpackagedIDData::get_instance()->behaviours_key)
        {
            if ((l / "contents").stat().exists())
            {
                contents_key = std::make_shared<InstalledUnpackagedContentsKey>(id, d);
                installed_time_key = std::make_shared<InstalledUnpackagedTimeKey>(l / "contents");
            }

            from_repositories_key = std::make_shared<InstalledUnpackagedStringSetKey>("source_repository",
                        "Source repository", mkt_normal);
            if ((l / "source_repository").stat().exists())
                from_repositories_key->add_source(l / "source_repository");
            if ((l / "binary_repository").stat().exists())
                from_repositories_key->add_source(l / "binary_repository");

            if ((l / "description").stat().exists())
                description_key = std::make_shared<InstalledUnpackagedStringKey>("description", "Description", l / "description", mkt_significant);

            if ((l / "build_dependencies").stat().exists())
                build_dependencies_key = std::make_shared<InstalledUnpackagedDependencyKey>(env,
                            "build_dependencies", "Build dependencies", l / "build_dependencies",
                            InstalledUnpackagedIDData::get_instance()->build_dependencies_labels, mkt_dependencies);

            if ((l / "run_dependencies").stat().exists())
                run_dependencies_key = std::make_shared<InstalledUnpackagedDependencyKey>(env,
                            "run_dependencies", "Run dependencies", l / "run_dependencies",
                            InstalledUnpackagedIDData::get_instance()->run_dependencies_labels, mkt_dependencies);
        }
    };
}

InstalledUnpackagedID::InstalledUnpackagedID(const Environment * const e, const QualifiedPackageName & q,
        const VersionSpec & v, const SlotName & s, const RepositoryName & n, const FSPath & l,
        const std::string &, const FSPath & ro, const NDBAM * const d) :
    _imp(e, this, q, v, s, n, l, ro, d)
{
    add_metadata_key(_imp->fs_location_key);
    add_metadata_key(_imp->slot_key);
    if (_imp->contents_key)
        add_metadata_key(_imp->contents_key);
    if (_imp->installed_time_key)
        add_metadata_key(_imp->installed_time_key);
    if (_imp->from_repositories_key)
        add_metadata_key(_imp->from_repositories_key);
    if (_imp->description_key)
        add_metadata_key(_imp->description_key);
    if (_imp->build_dependencies_key)
        add_metadata_key(_imp->build_dependencies_key);
    if (_imp->run_dependencies_key)
        add_metadata_key(_imp->run_dependencies_key);
    add_metadata_key(_imp->behaviours_key);
}

InstalledUnpackagedID::~InstalledUnpackagedID()
{
}

void
InstalledUnpackagedID::need_keys_added() const
{
}

void
InstalledUnpackagedID::need_masks_added() const
{
}

const std::string
InstalledUnpackagedID::canonical_form(const PackageIDCanonicalForm f) const
{
    switch (f)
    {
        case idcf_full:
            return stringify(_imp->name) + "-" + stringify(_imp->version) + ":" +
                stringify(slot_key()->parse_value()) + "::" + stringify(_imp->repository_name);

        case idcf_version:
            return stringify(_imp->version);

        case idcf_no_version:
            return stringify(_imp->name) + ":" + stringify(slot_key()->parse_value()) + "::" +
                stringify(_imp->repository_name);

        case idcf_no_name:
            return stringify(_imp->version) + ":" +
                stringify(slot_key()->parse_value()) + "::" + stringify(_imp->repository_name);

        case last_idcf:
            break;
    }

    throw InternalError(PALUDIS_HERE, "Bad PackageIDCanonicalForm");
}

PackageDepSpec
InstalledUnpackagedID::uniquely_identifying_spec() const
{
    return parse_user_package_dep_spec("=" + stringify(name()) + "-" + stringify(version()) +
            (slot_key() ? ":" + stringify(slot_key()->parse_value()) : "") + "::" + stringify(repository_name()),
            _imp->env, { });
}

const QualifiedPackageName
InstalledUnpackagedID::name() const
{
    return _imp->name;
}

const VersionSpec
InstalledUnpackagedID::version() const
{
    return _imp->version;
}

const RepositoryName
InstalledUnpackagedID::repository_name() const
{
    return _imp->repository_name;
}

const std::shared_ptr<const MetadataValueKey<std::shared_ptr<const PackageID> > >
InstalledUnpackagedID::virtual_for_key() const
{
    return std::shared_ptr<const MetadataValueKey<std::shared_ptr<const PackageID> > >();
}

const std::shared_ptr<const MetadataCollectionKey<KeywordNameSet> >
InstalledUnpackagedID::keywords_key() const
{
    return std::shared_ptr<const MetadataCollectionKey<KeywordNameSet> >();
}

const std::shared_ptr<const MetadataSpecTreeKey<ProvideSpecTree> >
InstalledUnpackagedID::provide_key() const
{
    return std::shared_ptr<const MetadataSpecTreeKey<ProvideSpecTree> >();
}

const std::shared_ptr<const MetadataCollectionKey<PackageIDSequence> >
InstalledUnpackagedID::contains_key() const
{
    return std::shared_ptr<const MetadataCollectionKey<PackageIDSequence> >();
}

const std::shared_ptr<const MetadataValueKey<std::shared_ptr<const PackageID> > >
InstalledUnpackagedID::contained_in_key() const
{
    return std::shared_ptr<const MetadataValueKey<std::shared_ptr<const PackageID> > >();
}

const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
InstalledUnpackagedID::dependencies_key() const
{
    return make_null_shared_ptr();
}

const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
InstalledUnpackagedID::build_dependencies_key() const
{
    return _imp->build_dependencies_key;
}

const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
InstalledUnpackagedID::run_dependencies_key() const
{
    return _imp->run_dependencies_key;
}

const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
InstalledUnpackagedID::post_dependencies_key() const
{
    return std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >();
}

const std::shared_ptr<const MetadataSpecTreeKey<FetchableURISpecTree> >
InstalledUnpackagedID::fetches_key() const
{
    return std::shared_ptr<const MetadataSpecTreeKey<FetchableURISpecTree> >();
}

const std::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> >
InstalledUnpackagedID::homepage_key() const
{
    return std::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> >();
}

const std::shared_ptr<const MetadataValueKey<std::string> >
InstalledUnpackagedID::short_description_key() const
{
    return _imp->description_key;
}

const std::shared_ptr<const MetadataValueKey<std::string> >
InstalledUnpackagedID::long_description_key() const
{
    return std::shared_ptr<const MetadataValueKey<std::string> >();
}

const std::shared_ptr<const MetadataValueKey<std::shared_ptr<const Contents> > >
InstalledUnpackagedID::contents_key() const
{
    return _imp->contents_key;
}

const std::shared_ptr<const MetadataTimeKey>
InstalledUnpackagedID::installed_time_key() const
{
    return _imp->installed_time_key;
}

const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > >
InstalledUnpackagedID::from_repositories_key() const
{
    return _imp->from_repositories_key;
}

const std::shared_ptr<const MetadataValueKey<FSPath> >
InstalledUnpackagedID::fs_location_key() const
{
    return _imp->fs_location_key;
}

const std::shared_ptr<const MetadataValueKey<SlotName> >
InstalledUnpackagedID::slot_key() const
{
    return _imp->slot_key;
}

namespace paludis
{
    class InstalledUnpackagedTransientKey :
        public LiteralMetadataValueKey<bool>
    {
        public:
            InstalledUnpackagedTransientKey(
                    const std::string &,
                    const std::string &,
                    const MetadataKeyType,
                    bool v);

            virtual std::string pretty_print() const;
    };
}

InstalledUnpackagedTransientKey::InstalledUnpackagedTransientKey(
        const std::string & r, const std::string & h, const MetadataKeyType t, bool v) :
    LiteralMetadataValueKey<bool>(r, h, t, v)
{
}

std::string
InstalledUnpackagedTransientKey::pretty_print() const
{
    return stringify(parse_value());
}

const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > >
InstalledUnpackagedID::behaviours_key() const
{
    return _imp->behaviours_key;
}

namespace
{
    struct SupportVisitor
    {
        bool visit(const SupportsActionTest<UninstallAction> &) const
        {
            return true;
        }

        bool visit(const SupportsActionTest<ConfigAction> &) const
        {
           return false;
        }

        bool visit(const SupportsActionTest<InfoAction> &) const
        {
            return false;
        }

        bool visit(const SupportsActionTest<PretendAction> &) const
        {
            return false;
        }

        bool visit(const SupportsActionTest<FetchAction> &) const
        {
            return false;
        }

        bool visit(const SupportsActionTest<InstallAction> &) const
        {
            return false;
        }

        bool visit(const SupportsActionTest<PretendFetchAction> &) const
        {
            return false;
        }
    };

    struct PerformAction
    {
        const InstalledUnpackagedID * const id;

        PerformAction(const InstalledUnpackagedID * const i) :
            id(i)
        {
        }

        void visit(InstallAction & a) PALUDIS_ATTRIBUTE((noreturn))
        {
            throw ActionFailedError("Unsupported action: " + a.simple_name());
        }

        void visit(FetchAction & a) PALUDIS_ATTRIBUTE((noreturn))
        {
            throw ActionFailedError("Unsupported action: " + a.simple_name());
        }

        void visit(ConfigAction & a) PALUDIS_ATTRIBUTE((noreturn))
        {
            throw ActionFailedError("Unsupported action: " + a.simple_name());
        }

        void visit(PretendAction & a) PALUDIS_ATTRIBUTE((noreturn))
        {
            throw ActionFailedError("Unsupported action: " + a.simple_name());
        }

        void visit(PretendFetchAction & a) PALUDIS_ATTRIBUTE((noreturn))
        {
            throw ActionFailedError("Unsupported action: " + a.simple_name());
        }

        void visit(InfoAction & a) PALUDIS_ATTRIBUTE((noreturn))
        {
            throw ActionFailedError("Unsupported action: " + a.simple_name());
        }

        void visit(UninstallAction & a)
        {
            std::shared_ptr<OutputManager> output_manager(a.options.make_output_manager()(a));
            id->uninstall(false, a.options.if_for_install_id(), output_manager);
            output_manager->succeeded();
        }
    };
}

bool
InstalledUnpackagedID::supports_action(const SupportsActionTestBase & test) const
{
    SupportVisitor v;
    return test.accept_returning<bool>(v);
}

void
InstalledUnpackagedID::perform_action(Action & action) const
{
    PerformAction v(this);
    action.accept(v);
}

std::shared_ptr<const Set<std::string> >
InstalledUnpackagedID::breaks_portage() const
{
    std::shared_ptr<Set<std::string> > why(std::make_shared<Set<std::string>>());
    why->insert("format");
    return why;
}

bool
InstalledUnpackagedID::arbitrary_less_than_comparison(const PackageID & other) const
{
    return slot_key()->parse_value().value() < (other.slot_key() ? stringify(other.slot_key()->parse_value()) : "");
}

std::size_t
InstalledUnpackagedID::extra_hash_value() const
{
    return Hash<SlotName>()(slot_key()->parse_value());
}

namespace
{
    bool ignore_nothing(const FSPath &)
    {
        return false;
    }
}

void
InstalledUnpackagedID::uninstall(const bool replace,
        const std::shared_ptr<const PackageID> & if_for_install_id,
        const std::shared_ptr<OutputManager> & output_manager) const
{
    Context context("When uninstalling '" + stringify(*this) + "':");

    auto repo(_imp->env->fetch_repository(repository_name()));
    bool last((! replace) && (! if_for_install_id));
    if (last)
    {
        std::shared_ptr<const PackageIDSequence> ids(repo->package_ids(name(), { }));
        for (PackageIDSequence::ConstIterator v(ids->begin()), v_end(ids->end()) ;
                v != v_end ; ++v)
            if (**v != *this)
            {
                last = false;
                break;
            }
    }

    if (! _imp->root.stat().is_directory())
        throw ActionFailedError("Couldn't uninstall '" + stringify(*this) +
                "' because root ('" + stringify(_imp->root) + "') is not a directory");

    FSPath ver_dir(fs_location_key()->parse_value());

    NDBAMUnmerger unmerger(
            make_named_values<NDBAMUnmergerOptions>(
                n::config_protect() = getenv_with_default("CONFIG_PROTECT", ""),
                n::config_protect_mask() = getenv_with_default("CONFIG_PROTECT_MASK", ""),
                n::contents_file() = ver_dir / "contents",
                n::environment() = _imp->env,
                n::ignore() = &ignore_nothing,
                n::ndbam() = _imp->ndbam,
                n::output_manager() = output_manager,
                n::package_id() = shared_from_this(),
                n::root() = _imp->root
            ));

    unmerger.unmerge();

    for (FSIterator d(ver_dir, { fsio_include_dotfiles, fsio_inode_sort }), d_end ; d != d_end ; ++d)
        d->unlink();
    ver_dir.rmdir();

    if (last)
    {
        FSPath pkg_dir(fs_location_key()->parse_value().dirname());
        pkg_dir.rmdir();

        std::static_pointer_cast<const InstalledUnpackagedRepository>(repo)->deindex(name());
    }
}

const std::shared_ptr<const MetadataValueKey<std::shared_ptr<const Choices> > >
InstalledUnpackagedID::choices_key() const
{
    return std::shared_ptr<const MetadataValueKey<std::shared_ptr<const Choices> > >();
}

