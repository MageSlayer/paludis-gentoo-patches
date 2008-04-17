/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008 Ciaran McCreesh
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
#include <paludis/repositories/unpackaged/dep_parser.hh>
#include <paludis/repositories/unpackaged/dep_printer.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/system.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/visitor_cast.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/tr1_functional.hh>
#include <paludis/name.hh>
#include <paludis/version_spec.hh>
#include <paludis/package_database.hh>
#include <paludis/contents.hh>
#include <paludis/environment.hh>
#include <paludis/metadata_key.hh>
#include <paludis/literal_metadata_key.hh>
#include <paludis/action.hh>
#include <paludis/hashed_containers.hh>
#include <fstream>

using namespace paludis;
using namespace paludis::unpackaged_repositories;

namespace
{
    class InstalledUnpackagedFSEntryKey :
        public MetadataValueKey<FSEntry>
    {
        private:
            const FSEntry _location;

        public:
            InstalledUnpackagedFSEntryKey(const FSEntry & l) :
                MetadataValueKey<FSEntry> ("location", "Location", mkt_internal),
                _location(l)
            {
            }

            const FSEntry value() const
            {
                return _location;
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
        public MetadataValueKey<tr1::shared_ptr<const Contents> >
    {
        private:
            const PackageID * const _id;
            const NDBAM * const _db;
            mutable Mutex _mutex;
            mutable tr1::shared_ptr<Contents> _v;

        public:
            InstalledUnpackagedContentsKey(const PackageID * const i, const NDBAM * const d) :
                MetadataValueKey<tr1::shared_ptr<const Contents> > ("contents", "Contents", mkt_internal),
                _id(i),
                _db(d)
            {
            }

            const tr1::shared_ptr<const Contents> value() const
            {
                Lock l(_mutex);
                if (_v)
                    return _v;

                using namespace tr1::placeholders;
                _v.reset(new Contents);
                _db->parse_contents(*_id,
                        tr1::bind(&create_file, tr1::ref(*_v), _1),
                        tr1::bind(&create_dir, tr1::ref(*_v), _1),
                        tr1::bind(&create_sym, tr1::ref(*_v), _1, _2)
                        );
                return _v;
            }
    };

    class InstalledUnpackagedTimeKey :
        public MetadataTimeKey
    {
        private:
            const time_t _time;

        public:
            InstalledUnpackagedTimeKey(const FSEntry & f) :
                MetadataTimeKey("installed_time", "Installed time", mkt_normal),
                _time(f.mtime())
            {
            }

            time_t value() const
            {
                return _time;
            }
    };

    class InstalledUnpackagedStringKey :
        public MetadataValueKey<std::string>
    {
        private:
            mutable tr1::shared_ptr<const std::string> _v;
            mutable Mutex _mutex;
            const FSEntry _f;

        public:
            InstalledUnpackagedStringKey(const std::string & r, const std::string & h, const FSEntry & f, const MetadataKeyType t) :
                MetadataValueKey<std::string> (r, h, t),
                _f(f)
            {
            }

            const std::string value() const
            {
                Lock l(_mutex);
                if (_v)
                    return *_v;
                std::ifstream f(stringify(_f).c_str());
                if (! f)
                {
                    Context context("When reading '" + stringify(_f) + "' as an InstalledUnpackagedStringKey:");
                    throw FSError("Couldn't open '" + stringify(_f) + "' for read");
                }
                _v.reset(new std::string(
                            strip_trailing(std::string((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>()), "\n")));
                return *_v;
            }
    };

    class InstalledUnpackagedDependencyKey :
        public MetadataSpecTreeKey<DependencySpecTree>
    {
        private:
            const Environment * const _env;
            mutable tr1::shared_ptr<const DependencySpecTree::ConstItem> _v;
            mutable Mutex _mutex;
            const FSEntry _f;
            const tr1::shared_ptr<const DependencyLabelSequence> _labels;

        public:
            InstalledUnpackagedDependencyKey(const Environment * const e,
                    const std::string & r, const std::string & h, const FSEntry & f,
                    const tr1::shared_ptr<const DependencyLabelSequence> & l, const MetadataKeyType t) :
                MetadataSpecTreeKey<DependencySpecTree>(r, h, t),
                _env(e),
                _f(f),
                _labels(l)
            {
            }

            const tr1::shared_ptr<const DependencySpecTree::ConstItem> value() const
            {
                Lock l(_mutex);
                if (_v)
                    return _v;

                Context context("When reading '" + stringify(_f) + "' as an InstalledUnpackagedDependencyKey:");

                std::ifstream f(stringify(_f).c_str());
                if (! f)
                    throw FSError("Couldn't open '" + stringify(_f) + "' for read");

                _v = DepParser::parse(strip_trailing(
                            std::string((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>()), "\n"));
                return _v;
            }

            std::string
            pretty_print(const DependencySpecTree::ItemFormatter & f) const
            {
                DepPrinter p(_env, f, false);
                value()->accept(p);
                return p.result();
            }

            std::string
            pretty_print_flat(const DependencySpecTree::ItemFormatter & f) const
            {
                DepPrinter p(_env, f, true);
                value()->accept(p);
                return p.result();
            }

            const tr1::shared_ptr<const DependencyLabelSequence> initial_labels() const
            {
                return _labels;
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
        const SlotName slot;
        const RepositoryName repository_name;
        const FSEntry root;
        const NDBAM * const ndbam;

        tr1::shared_ptr<DependencyLabelSequence> build_dependencies_labels;
        tr1::shared_ptr<DependencyLabelSequence> run_dependencies_labels;

        tr1::shared_ptr<InstalledUnpackagedFSEntryKey> fs_location_key;
        tr1::shared_ptr<InstalledUnpackagedContentsKey> contents_key;
        tr1::shared_ptr<InstalledUnpackagedTimeKey> installed_time_key;
        tr1::shared_ptr<InstalledUnpackagedStringKey> source_origin_key;
        tr1::shared_ptr<InstalledUnpackagedStringKey> binary_origin_key;
        tr1::shared_ptr<InstalledUnpackagedStringKey> description_key;
        tr1::shared_ptr<InstalledUnpackagedDependencyKey> build_dependencies_key;
        tr1::shared_ptr<InstalledUnpackagedDependencyKey> run_dependencies_key;

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
            slot(s),
            repository_name(r),
            root(ro),
            ndbam(d),
            build_dependencies_labels(new DependencyLabelSequence),
            run_dependencies_labels(new DependencyLabelSequence),
            fs_location_key(new InstalledUnpackagedFSEntryKey(l))
        {
            build_dependencies_labels->push_back(make_shared_ptr(new DependencyBuildLabel("build_dependencies")));
            run_dependencies_labels->push_back(make_shared_ptr(new DependencyRunLabel("run_dependencies")));

            if ((l / "contents").exists())
            {
                contents_key.reset(new InstalledUnpackagedContentsKey(id, d));
                installed_time_key.reset(new InstalledUnpackagedTimeKey(l / "contents"));
            }

            if ((l / "source_repository").exists())
                source_origin_key.reset(new InstalledUnpackagedStringKey("source_repository", "Source repository", l / "source_repository",
                            mkt_normal));

            if ((l / "binary_repository").exists())
                binary_origin_key.reset(new InstalledUnpackagedStringKey("binary_repository", "Binary repository", l / "binary_repository",
                            mkt_normal));

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

InstalledUnpackagedID::InstalledUnpackagedID(const Environment * const e, const QualifiedPackageName & q,
        const VersionSpec & v, const SlotName & s, const RepositoryName & n, const FSEntry & l,
        const std::string &, const FSEntry & ro, const NDBAM * const d) :
    PrivateImplementationPattern<InstalledUnpackagedID>(new Implementation<InstalledUnpackagedID>(e, this, q, v, s, n, l, ro, d)),
    _imp(PrivateImplementationPattern<InstalledUnpackagedID>::_imp)
{
    add_metadata_key(_imp->fs_location_key);
    if (_imp->contents_key)
        add_metadata_key(_imp->contents_key);
    if (_imp->installed_time_key)
        add_metadata_key(_imp->installed_time_key);
    if (_imp->source_origin_key)
        add_metadata_key(_imp->source_origin_key);
    if (_imp->binary_origin_key)
        add_metadata_key(_imp->binary_origin_key);
    if (_imp->description_key)
        add_metadata_key(_imp->description_key);
    if (_imp->build_dependencies_key)
        add_metadata_key(_imp->build_dependencies_key);
    if (_imp->run_dependencies_key)
        add_metadata_key(_imp->run_dependencies_key);
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
                stringify(slot()) + "::" + stringify(_imp->repository_name);

        case idcf_version:
            return stringify(_imp->version);

        case idcf_no_version:
            return stringify(_imp->name) + ":" + stringify(slot()) + "::" +
                stringify(_imp->repository_name);

        case last_idcf:
            break;
    }

    throw InternalError(PALUDIS_HERE, "Bad PackageIDCanonicalForm");
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

const SlotName
InstalledUnpackagedID::slot() const
{
    return _imp->slot;
}

const tr1::shared_ptr<const Repository>
InstalledUnpackagedID::repository() const
{
    return _imp->env->package_database()->fetch_repository(_imp->repository_name);
}

const tr1::shared_ptr<const MetadataValueKey<tr1::shared_ptr<const PackageID> > >
InstalledUnpackagedID::virtual_for_key() const
{
    return tr1::shared_ptr<const MetadataValueKey<tr1::shared_ptr<const PackageID> > >();
}

const tr1::shared_ptr<const MetadataCollectionKey<KeywordNameSet> >
InstalledUnpackagedID::keywords_key() const
{
    return tr1::shared_ptr<const MetadataCollectionKey<KeywordNameSet> >();
}

const tr1::shared_ptr<const MetadataCollectionKey<IUseFlagSet> >
InstalledUnpackagedID::iuse_key() const
{
    return tr1::shared_ptr<const MetadataCollectionKey<IUseFlagSet> >();
}

const tr1::shared_ptr<const MetadataSpecTreeKey<ProvideSpecTree> >
InstalledUnpackagedID::provide_key() const
{
    return tr1::shared_ptr<const MetadataSpecTreeKey<ProvideSpecTree> >();
}

const tr1::shared_ptr<const MetadataCollectionKey<PackageIDSequence> >
InstalledUnpackagedID::contains_key() const
{
    return tr1::shared_ptr<const MetadataCollectionKey<PackageIDSequence> >();
}

const tr1::shared_ptr<const MetadataValueKey<tr1::shared_ptr<const PackageID> > >
InstalledUnpackagedID::contained_in_key() const
{
    return tr1::shared_ptr<const MetadataValueKey<tr1::shared_ptr<const PackageID> > >();
}

const tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
InstalledUnpackagedID::build_dependencies_key() const
{
    return _imp->build_dependencies_key;
}

const tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
InstalledUnpackagedID::run_dependencies_key() const
{
    return _imp->run_dependencies_key;
}

const tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
InstalledUnpackagedID::post_dependencies_key() const
{
    return tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >();
}

const tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
InstalledUnpackagedID::suggested_dependencies_key() const
{
    return tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >();
}

const tr1::shared_ptr<const MetadataSpecTreeKey<FetchableURISpecTree> >
InstalledUnpackagedID::fetches_key() const
{
    return tr1::shared_ptr<const MetadataSpecTreeKey<FetchableURISpecTree> >();
}

const tr1::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> >
InstalledUnpackagedID::homepage_key() const
{
    return tr1::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> >();
}

const tr1::shared_ptr<const MetadataValueKey<std::string> >
InstalledUnpackagedID::short_description_key() const
{
    return _imp->description_key;
}

const tr1::shared_ptr<const MetadataValueKey<std::string> >
InstalledUnpackagedID::long_description_key() const
{
    return tr1::shared_ptr<const MetadataValueKey<std::string> >();
}

const tr1::shared_ptr<const MetadataValueKey<tr1::shared_ptr<const Contents> > >
InstalledUnpackagedID::contents_key() const
{
    return _imp->contents_key;
}

const tr1::shared_ptr<const MetadataTimeKey>
InstalledUnpackagedID::installed_time_key() const
{
    return _imp->installed_time_key;
}

const tr1::shared_ptr<const MetadataValueKey<std::string> >
InstalledUnpackagedID::source_origin_key() const
{
    return _imp->source_origin_key;
}

const tr1::shared_ptr<const MetadataValueKey<std::string> >
InstalledUnpackagedID::binary_origin_key() const
{
    return _imp->binary_origin_key;
}

const tr1::shared_ptr<const MetadataValueKey<FSEntry> >
InstalledUnpackagedID::fs_location_key() const
{
    return _imp->fs_location_key;
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

const tr1::shared_ptr<const MetadataValueKey<bool> >
InstalledUnpackagedID::transient_key() const
{
    return tr1::shared_ptr<const MetadataValueKey<bool> >(
            new InstalledUnpackagedTransientKey("transient", "Transient", mkt_internal, true));
}

namespace
{
    struct SupportVisitor :
        ConstVisitor<SupportsActionTestVisitorTypes>
    {
        bool result;

        void visit(const SupportsActionTest<UninstallAction> &)
        {
            result = true;
        }

        void visit(const SupportsActionTest<InstalledAction> &)
        {
            result = true;
        }

        void visit(const SupportsActionTest<ConfigAction> &)
        {
           result = false;
        }

        void visit(const SupportsActionTest<InfoAction> &)
        {
            result = false;
        }

        void visit(const SupportsActionTest<PretendAction> &)
        {
            result = false;
        }

        void visit(const SupportsActionTest<FetchAction> &)
        {
            result = false;
        }

        void visit(const SupportsActionTest<InstallAction> &)
        {
            result = false;
        }

        void visit(const SupportsActionTest<PretendFetchAction> &)
        {
            result = false;
        }
    };

    struct PerformAction :
        Visitor<ActionVisitorTypes>
    {
        const InstalledUnpackagedID * const id;

        PerformAction(const InstalledUnpackagedID * const i) :
            id(i)
        {
        }

        void visit(InstallAction & a) PALUDIS_ATTRIBUTE((noreturn))
        {
            throw UnsupportedActionError(*id, a);
        }

        void visit(FetchAction & a) PALUDIS_ATTRIBUTE((noreturn))
        {
            throw UnsupportedActionError(*id, a);
        }

        void visit(ConfigAction & a) PALUDIS_ATTRIBUTE((noreturn))
        {
            throw UnsupportedActionError(*id, a);
        }

        void visit(PretendAction & a) PALUDIS_ATTRIBUTE((noreturn))
        {
            throw UnsupportedActionError(*id, a);
        }

        void visit(PretendFetchAction & a) PALUDIS_ATTRIBUTE((noreturn))
        {
            throw UnsupportedActionError(*id, a);
        }

        void visit(InfoAction & a) PALUDIS_ATTRIBUTE((noreturn))
        {
            throw UnsupportedActionError(*id, a);
        }

        void visit(InstalledAction &)
        {
        }

        void visit(UninstallAction & a)
        {
            id->uninstall(a.options, false);
        }
    };
}

bool
InstalledUnpackagedID::supports_action(const SupportsActionTestBase & test) const
{
    SupportVisitor v;
    test.accept(v);
    return v.result;
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

bool
InstalledUnpackagedID::breaks_portage() const
{
    return true;
}

bool
InstalledUnpackagedID::arbitrary_less_than_comparison(const PackageID & other) const
{
    return slot().data() < other.slot().data();
}

std::size_t
InstalledUnpackagedID::extra_hash_value() const
{
    return CRCHash<SlotName>()(slot());
}

void
InstalledUnpackagedID::uninstall(const UninstallActionOptions &, const bool replace) const
{
    Context context("When uninstalling '" + stringify(*this) + "':");

    bool last(! replace);
    if (last)
    {
        tr1::shared_ptr<const PackageIDSequence> ids(repository()->package_ids(name()));
        for (PackageIDSequence::ConstIterator v(ids->begin()), v_end(ids->end()) ;
                v != v_end ; ++v)
            if (**v != *this)
            {
                last = false;
                break;
            }
    }

    if (! _imp->root.is_directory())
        throw InstallActionError("Couldn't uninstall '" + stringify(*this) +
                "' because root ('" + stringify(_imp->root) + "') is not a directory");

    FSEntry ver_dir(fs_location_key()->value());

    NDBAMUnmerger unmerger(
            NDBAMUnmergerOptions::named_create()
            (k::environment(), _imp->env)
            (k::root(), _imp->root)
            (k::contents_file(), ver_dir / "contents")
            (k::config_protect(), getenv_with_default("CONFIG_PROTECT", ""))
            (k::config_protect_mask(), getenv_with_default("CONFIG_PROTECT_MASK", ""))
            (k::ndbam(), _imp->ndbam)
            (k::package_id(), shared_from_this()));

    unmerger.unmerge();

    for (DirIterator d(ver_dir, DirIteratorOptions() + dio_include_dotfiles), d_end ; d != d_end ; ++d)
        FSEntry(*d).unlink();
    ver_dir.rmdir();

    if (last)
    {
        FSEntry pkg_dir(fs_location_key()->value().dirname());
        pkg_dir.rmdir();

        tr1::static_pointer_cast<const InstalledUnpackagedRepository>(repository())->deindex(name());
    }
}

