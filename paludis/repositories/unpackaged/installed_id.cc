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

#include <paludis/repositories/unpackaged/installed_id.hh>
#include <paludis/repositories/unpackaged/installed_repository.hh>
#include <paludis/ndbam.hh>
#include <paludis/ndbam_unmerger.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/system.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/safe_ifstream.hh>
#include <paludis/util/return_literal_function.hh>
#include <paludis/util/timestamp.hh>
#include <paludis/output_manager.hh>
#include <paludis/name.hh>
#include <paludis/version_spec.hh>
#include <paludis/package_database.hh>
#include <paludis/contents.hh>
#include <paludis/environment.hh>
#include <paludis/metadata_key.hh>
#include <paludis/literal_metadata_key.hh>
#include <paludis/action.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/comma_separated_dep_parser.hh>
#include <paludis/comma_separated_dep_printer.hh>
#include <paludis/formatter.hh>
#include <tr1/functional>

using namespace paludis;
using namespace paludis::unpackaged_repositories;

namespace
{
    std::string format_string(const std::string & i, const Formatter<std::string> & f)
    {
        return f.format(i, format::Plain());
    }

    class InstalledUnpackagedFSEntryKey :
        public MetadataValueKey<FSEntry>
    {
        private:
            const FSEntry _location;

        public:
            InstalledUnpackagedFSEntryKey(const FSEntry & l) :
                _location(l)
            {
            }

            const FSEntry value() const
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

    void create_file(Contents & c, const FSEntry & f)
    {
        c.add(make_shared_ptr(new ContentsFileEntry(stringify(f))));
    }

    void create_dir(Contents & c, const FSEntry & f)
    {
        c.add(make_shared_ptr(new ContentsDirEntry(stringify(f))));
    }

    void create_sym(Contents & c, const FSEntry & f, const FSEntry & t)
    {
        c.add(make_shared_ptr(new ContentsSymEntry(stringify(f), stringify(t))));
    }

    class InstalledUnpackagedContentsKey :
        public MetadataValueKey<std::tr1::shared_ptr<const Contents> >
    {
        private:
            const PackageID * const _id;
            const NDBAM * const _db;
            mutable Mutex _mutex;
            mutable std::tr1::shared_ptr<Contents> _v;

        public:
            InstalledUnpackagedContentsKey(const PackageID * const i, const NDBAM * const d) :
                _id(i),
                _db(d)
            {
            }

            const std::tr1::shared_ptr<const Contents> value() const
            {
                Lock l(_mutex);
                if (_v)
                    return _v;

                using namespace std::tr1::placeholders;
                _v.reset(new Contents);
                _db->parse_contents(*_id,
                        std::tr1::bind(&Contents::add, _v.get(), std::tr1::placeholders::_1),
                        std::tr1::bind(&Contents::add, _v.get(), std::tr1::placeholders::_1),
                        std::tr1::bind(&Contents::add, _v.get(), std::tr1::placeholders::_1)
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
            InstalledUnpackagedTimeKey(const FSEntry & f) :
                _time(f.mtim())
            {
            }

            Timestamp value() const
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
            mutable std::tr1::shared_ptr<const std::string> _v;
            mutable Mutex _mutex;
            const FSEntry _f;

            const std::string _raw_name;
            const std::string _human_name;
            const MetadataKeyType _type;

        public:
            InstalledUnpackagedStringKey(const std::string & r, const std::string & h, const FSEntry & f, const MetadataKeyType t) :
                _f(f),
                _raw_name(r),
                _human_name(h),
                _type(t)
            {
            }

            const std::string value() const
            {
                Lock l(_mutex);
                if (_v)
                    return *_v;

                Context context("When reading '" + stringify(_f) + "' as an InstalledUnpackagedStringKey:");
                SafeIFStream f(_f);
                _v.reset(new std::string(
                            strip_trailing(std::string((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>()), "\n")));
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
            mutable std::tr1::shared_ptr<Set<std::string> > _v;
            mutable Mutex _mutex;
            FSEntrySequence _f;

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

            void add_source(const FSEntry & f)
            {
                _f.push_back(f);
            }

            const std::tr1::shared_ptr<const Set<std::string> > value() const
            {
                Lock l(_mutex);
                if (_v)
                    return _v;

                _v.reset(new Set<std::string>());
                for (FSEntrySequence::ConstIterator a(_f.begin()), a_end(_f.end()) ;
                        a != a_end ; ++a)
                {
                    SafeIFStream f(*a);
                    _v->insert(strip_trailing(std::string((std::istreambuf_iterator<char>(f)),
                                    std::istreambuf_iterator<char>()), "\n"));
                }
                return _v;
            }

            std::string pretty_print_flat(const Formatter<std::string> & f) const
            {
                using namespace std::tr1::placeholders;
                return join(value()->begin(), value()->end(), " ", std::tr1::bind(&format_string, _1, f));
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

    class InstalledUnpackagedDependencyKey :
        public MetadataSpecTreeKey<DependencySpecTree>
    {
        private:
            const Environment * const _env;
            mutable std::tr1::shared_ptr<const DependencySpecTree> _v;
            mutable Mutex _mutex;
            const FSEntry _f;
            const std::tr1::shared_ptr<const DependenciesLabelSequence> _labels;

            const std::string _raw_name;
            const std::string _human_name;
            const MetadataKeyType _type;

        public:
            InstalledUnpackagedDependencyKey(const Environment * const e,
                    const std::string & r, const std::string & h, const FSEntry & f,
                    const std::tr1::shared_ptr<const DependenciesLabelSequence> & l, const MetadataKeyType t) :
                _env(e),
                _f(f),
                _labels(l),
                _raw_name(r),
                _human_name(h),
                _type(t)
            {
            }

            const std::tr1::shared_ptr<const DependencySpecTree> value() const
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

            std::string
            pretty_print(const DependencySpecTree::ItemFormatter & f) const
            {
                CommaSeparatedDepPrinter p(_env, f, false);
                value()->root()->accept(p);
                return p.result();
            }

            std::string
            pretty_print_flat(const DependencySpecTree::ItemFormatter & f) const
            {
                CommaSeparatedDepPrinter p(_env, f, true);
                value()->root()->accept(p);
                return p.result();
            }

            const std::tr1::shared_ptr<const DependenciesLabelSequence> initial_labels() const
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
    };
}

namespace paludis
{
    template <>
    struct Implementation<InstalledUnpackagedID>
    {
        const Environment * const env;
        const QualifiedPackageName name;
        const VersionSpec version;
        const RepositoryName repository_name;
        const FSEntry root;
        const NDBAM * const ndbam;

        std::tr1::shared_ptr<DependenciesLabelSequence> build_dependencies_labels;
        std::tr1::shared_ptr<DependenciesLabelSequence> run_dependencies_labels;

        std::tr1::shared_ptr<LiteralMetadataValueKey<SlotName> > slot_key;
        std::tr1::shared_ptr<InstalledUnpackagedFSEntryKey> fs_location_key;
        std::tr1::shared_ptr<InstalledUnpackagedContentsKey> contents_key;
        std::tr1::shared_ptr<InstalledUnpackagedTimeKey> installed_time_key;
        std::tr1::shared_ptr<InstalledUnpackagedStringSetKey> from_repositories_key;
        std::tr1::shared_ptr<InstalledUnpackagedStringKey> description_key;
        std::tr1::shared_ptr<InstalledUnpackagedDependencyKey> build_dependencies_key;
        std::tr1::shared_ptr<InstalledUnpackagedDependencyKey> run_dependencies_key;
        std::tr1::shared_ptr<LiteralMetadataStringSetKey> behaviours_key;

        static const std::tr1::shared_ptr<Set<std::string> > behaviours_set;

        Implementation(
                const Environment * const e,
                const PackageID * const id,
                const QualifiedPackageName & q,
                const VersionSpec & v,
                const SlotName & s,
                const RepositoryName & r,
                const FSEntry & l,
                const FSEntry & ro,
                const NDBAM * const d) :
            env(e),
            name(q),
            version(v),
            repository_name(r),
            root(ro),
            ndbam(d),
            build_dependencies_labels(new DependenciesLabelSequence),
            run_dependencies_labels(new DependenciesLabelSequence),
            slot_key(new LiteralMetadataValueKey<SlotName> ("slot", "Slot", mkt_internal, s)),
            fs_location_key(new InstalledUnpackagedFSEntryKey(l)),
            behaviours_key(new LiteralMetadataStringSetKey("behaviours", "behaviours", mkt_internal, behaviours_set))
        {
            build_dependencies_labels->push_back(make_shared_ptr(new DependenciesBuildLabel("build_dependencies",
                            return_literal_function(true))));
            run_dependencies_labels->push_back(make_shared_ptr(new DependenciesRunLabel("run_dependencies",
                            return_literal_function(true))));

            if ((l / "contents").exists())
            {
                contents_key.reset(new InstalledUnpackagedContentsKey(id, d));
                installed_time_key.reset(new InstalledUnpackagedTimeKey(l / "contents"));
            }

            from_repositories_key.reset(new InstalledUnpackagedStringSetKey("source_repository",
                        "Source repository", mkt_normal));
            if ((l / "source_repository").exists())
                from_repositories_key->add_source(l / "source_repository");
            if ((l / "binary_repository").exists())
                from_repositories_key->add_source(l / "binary_repository");

            if ((l / "description").exists())
                description_key.reset(new InstalledUnpackagedStringKey("description", "Description", l / "description", mkt_significant));

            if ((l / "build_dependencies").exists())
                build_dependencies_key.reset(new InstalledUnpackagedDependencyKey(env,
                            "build_dependencies", "Build dependencies", l / "build_dependencies",
                            build_dependencies_labels, mkt_dependencies));

            if ((l / "run_dependencies").exists())
                run_dependencies_key.reset(new InstalledUnpackagedDependencyKey(env,
                            "run_dependencies", "Run dependencies", l / "run_dependencies",
                            run_dependencies_labels, mkt_dependencies));
        }
    };
}

namespace
{
    std::tr1::shared_ptr<Set<std::string> > make_behaviours()
    {
        std::tr1::shared_ptr<Set<std::string> > result(new Set<std::string>);
        result->insert("transient");
        return result;
    }
}

const std::tr1::shared_ptr<Set<std::string> > Implementation<InstalledUnpackagedID>::behaviours_set = make_behaviours();

InstalledUnpackagedID::InstalledUnpackagedID(const Environment * const e, const QualifiedPackageName & q,
        const VersionSpec & v, const SlotName & s, const RepositoryName & n, const FSEntry & l,
        const std::string &, const FSEntry & ro, const NDBAM * const d) :
    PrivateImplementationPattern<InstalledUnpackagedID>(new Implementation<InstalledUnpackagedID>(e, this, q, v, s, n, l, ro, d)),
    _imp(PrivateImplementationPattern<InstalledUnpackagedID>::_imp)
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
                stringify(slot_key()->value()) + "::" + stringify(_imp->repository_name);

        case idcf_version:
            return stringify(_imp->version);

        case idcf_no_version:
            return stringify(_imp->name) + ":" + stringify(slot_key()->value()) + "::" +
                stringify(_imp->repository_name);

        case idcf_no_name:
            return stringify(_imp->version) + ":" +
                stringify(slot_key()->value()) + "::" + stringify(_imp->repository_name);

        case last_idcf:
            break;
    }

    throw InternalError(PALUDIS_HERE, "Bad PackageIDCanonicalForm");
}

PackageDepSpec
InstalledUnpackagedID::uniquely_identifying_spec() const
{
    return parse_user_package_dep_spec("=" + stringify(name()) + "-" + stringify(version()) +
            (slot_key() ? ":" + stringify(slot_key()->value()) : "") + "::" + stringify(repository()->name()),
            _imp->env, UserPackageDepSpecOptions());
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

const std::tr1::shared_ptr<const Repository>
InstalledUnpackagedID::repository() const
{
    return _imp->env->package_database()->fetch_repository(_imp->repository_name);
}

const std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const PackageID> > >
InstalledUnpackagedID::virtual_for_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const PackageID> > >();
}

const std::tr1::shared_ptr<const MetadataCollectionKey<KeywordNameSet> >
InstalledUnpackagedID::keywords_key() const
{
    return std::tr1::shared_ptr<const MetadataCollectionKey<KeywordNameSet> >();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<ProvideSpecTree> >
InstalledUnpackagedID::provide_key() const
{
    return std::tr1::shared_ptr<const MetadataSpecTreeKey<ProvideSpecTree> >();
}

const std::tr1::shared_ptr<const MetadataCollectionKey<PackageIDSequence> >
InstalledUnpackagedID::contains_key() const
{
    return std::tr1::shared_ptr<const MetadataCollectionKey<PackageIDSequence> >();
}

const std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const PackageID> > >
InstalledUnpackagedID::contained_in_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const PackageID> > >();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
InstalledUnpackagedID::dependencies_key() const
{
    return make_null_shared_ptr();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
InstalledUnpackagedID::build_dependencies_key() const
{
    return _imp->build_dependencies_key;
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
InstalledUnpackagedID::run_dependencies_key() const
{
    return _imp->run_dependencies_key;
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
InstalledUnpackagedID::post_dependencies_key() const
{
    return std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
InstalledUnpackagedID::suggested_dependencies_key() const
{
    return std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<FetchableURISpecTree> >
InstalledUnpackagedID::fetches_key() const
{
    return std::tr1::shared_ptr<const MetadataSpecTreeKey<FetchableURISpecTree> >();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> >
InstalledUnpackagedID::homepage_key() const
{
    return std::tr1::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> >();
}

const std::tr1::shared_ptr<const MetadataValueKey<std::string> >
InstalledUnpackagedID::short_description_key() const
{
    return _imp->description_key;
}

const std::tr1::shared_ptr<const MetadataValueKey<std::string> >
InstalledUnpackagedID::long_description_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<std::string> >();
}

const std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const Contents> > >
InstalledUnpackagedID::contents_key() const
{
    return _imp->contents_key;
}

const std::tr1::shared_ptr<const MetadataTimeKey>
InstalledUnpackagedID::installed_time_key() const
{
    return _imp->installed_time_key;
}

const std::tr1::shared_ptr<const MetadataCollectionKey<Set<std::string> > >
InstalledUnpackagedID::from_repositories_key() const
{
    return _imp->from_repositories_key;
}

const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> >
InstalledUnpackagedID::fs_location_key() const
{
    return _imp->fs_location_key;
}

const std::tr1::shared_ptr<const MetadataValueKey<SlotName> >
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
    return stringify(value());
}

const std::tr1::shared_ptr<const MetadataCollectionKey<Set<std::string> > >
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
            std::tr1::shared_ptr<OutputManager> output_manager(a.options.make_output_manager()(a));
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

void
InstalledUnpackagedID::invalidate_masks() const
{
}

std::tr1::shared_ptr<const Set<std::string> >
InstalledUnpackagedID::breaks_portage() const
{
    std::tr1::shared_ptr<Set<std::string> > why(new Set<std::string>);
    why->insert("format");
    return why;
}

bool
InstalledUnpackagedID::arbitrary_less_than_comparison(const PackageID & other) const
{
    return slot_key()->value().data() < (other.slot_key() ? stringify(other.slot_key()->value()) : "");
}

std::size_t
InstalledUnpackagedID::extra_hash_value() const
{
    return Hash<SlotName>()(slot_key()->value());
}

namespace
{
    bool ignore_nothing(const FSEntry &)
    {
        return false;
    }
}

void
InstalledUnpackagedID::uninstall(const bool replace,
        const std::tr1::shared_ptr<const PackageID> & if_for_install_id,
        const std::tr1::shared_ptr<OutputManager> & output_manager) const
{
    Context context("When uninstalling '" + stringify(*this) + "':");

    bool last((! replace) && (! if_for_install_id));
    if (last)
    {
        std::tr1::shared_ptr<const PackageIDSequence> ids(repository()->package_ids(name()));
        for (PackageIDSequence::ConstIterator v(ids->begin()), v_end(ids->end()) ;
                v != v_end ; ++v)
            if (**v != *this)
            {
                last = false;
                break;
            }
    }

    if (! _imp->root.is_directory())
        throw ActionFailedError("Couldn't uninstall '" + stringify(*this) +
                "' because root ('" + stringify(_imp->root) + "') is not a directory");

    FSEntry ver_dir(fs_location_key()->value());

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

    for (DirIterator d(ver_dir, DirIteratorOptions() + dio_include_dotfiles), d_end ; d != d_end ; ++d)
        FSEntry(*d).unlink();
    ver_dir.rmdir();

    if (last)
    {
        FSEntry pkg_dir(fs_location_key()->value().dirname());
        pkg_dir.rmdir();

        std::tr1::static_pointer_cast<const InstalledUnpackagedRepository>(repository())->deindex(name());
    }
}

const std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const Choices> > >
InstalledUnpackagedID::choices_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const Choices> > >();
}

