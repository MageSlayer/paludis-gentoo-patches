/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh
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
#include <paludis/repositories/unpackaged/ndbam.hh>
#include <paludis/repositories/unpackaged/ndbam_unmerger.hh>
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
#include <paludis/action.hh>
#include <paludis/hashed_containers.hh>
#include <fstream>

using namespace paludis;
using namespace paludis::unpackaged_repositories;

namespace
{
    class InstalledUnpackagedFSEntryKey :
        public MetadataFSEntryKey
    {
        private:
            const FSEntry _location;

        public:
            InstalledUnpackagedFSEntryKey(const FSEntry & l) :
                MetadataFSEntryKey("location", "Location", mkt_internal),
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
        public MetadataContentsKey
    {
        private:
            const PackageID * const _id;
            const NDBAM * const _db;
            mutable Mutex _mutex;
            mutable tr1::shared_ptr<Contents> _v;

        public:
            InstalledUnpackagedContentsKey(const PackageID * const i, const NDBAM * const d) :
                MetadataContentsKey("contents", "Contents", mkt_internal),
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

            const time_t value() const
            {
                return _time;
            }
    };

    class InstalledUnpackagedStringKey :
        public MetadataStringKey
    {
        private:
            mutable tr1::shared_ptr<const std::string> _v;
            mutable Mutex _mutex;
            const FSEntry _f;

        public:
            InstalledUnpackagedStringKey(const std::string & r, const std::string & h, const FSEntry & f) :
                MetadataStringKey(r, h, mkt_normal),
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

        tr1::shared_ptr<InstalledUnpackagedFSEntryKey> fs_location_key;
        tr1::shared_ptr<InstalledUnpackagedContentsKey> contents_key;
        tr1::shared_ptr<InstalledUnpackagedTimeKey> installed_time_key;
        tr1::shared_ptr<InstalledUnpackagedStringKey> source_origin_key;
        tr1::shared_ptr<InstalledUnpackagedStringKey> binary_origin_key;

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
            fs_location_key(new InstalledUnpackagedFSEntryKey(l))
        {
            if ((l / "contents").exists())
            {
                contents_key.reset(new InstalledUnpackagedContentsKey(id, d));
                installed_time_key.reset(new InstalledUnpackagedTimeKey(l / "contents"));
            }

            if ((l / "source_repository").exists())
                source_origin_key.reset(new InstalledUnpackagedStringKey("source_repository", "Source repository", l / "source_repository"));

            if ((l / "binary_repository").exists())
                binary_origin_key.reset(new InstalledUnpackagedStringKey("binary_repository", "Binary repository", l / "binary_repository"));
        }
    };
}

InstalledUnpackagedID::InstalledUnpackagedID(const Environment * const e, const QualifiedPackageName & q,
        const VersionSpec & v, const SlotName & s, const RepositoryName & n, const FSEntry & l,
        const std::string &, const FSEntry & ro, const NDBAM * const d) :
    PrivateImplementationPattern<InstalledUnpackagedID>(new Implementation<InstalledUnpackagedID>(e, this, q, v, s, n, l, ro, d)),
    _imp(PrivateImplementationPattern<InstalledUnpackagedID>::_imp.get())
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

const tr1::shared_ptr<const MetadataPackageIDKey>
InstalledUnpackagedID::virtual_for_key() const
{
    return tr1::shared_ptr<const MetadataPackageIDKey>();
}

const tr1::shared_ptr<const MetadataSetKey<KeywordNameSet> >
InstalledUnpackagedID::keywords_key() const
{
    return tr1::shared_ptr<const MetadataSetKey<KeywordNameSet> >();
}

const tr1::shared_ptr<const MetadataSetKey<IUseFlagSet> >
InstalledUnpackagedID::iuse_key() const
{
    return tr1::shared_ptr<const MetadataSetKey<IUseFlagSet> >();
}

const tr1::shared_ptr<const MetadataSpecTreeKey<ProvideSpecTree> >
InstalledUnpackagedID::provide_key() const
{
    return tr1::shared_ptr<const MetadataSpecTreeKey<ProvideSpecTree> >();
}

const tr1::shared_ptr<const MetadataSetKey<PackageIDSequence> >
InstalledUnpackagedID::contains_key() const
{
    return tr1::shared_ptr<const MetadataSetKey<PackageIDSequence> >();
}

const tr1::shared_ptr<const MetadataPackageIDKey>
InstalledUnpackagedID::contained_in_key() const
{
    return tr1::shared_ptr<const MetadataPackageIDKey>();
}

const tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
InstalledUnpackagedID::build_dependencies_key() const
{
    return tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >();
}

const tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
InstalledUnpackagedID::run_dependencies_key() const
{
    return tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >();
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

const tr1::shared_ptr<const MetadataStringKey>
InstalledUnpackagedID::short_description_key() const
{
    return tr1::shared_ptr<const MetadataStringKey>();
}

const tr1::shared_ptr<const MetadataStringKey>
InstalledUnpackagedID::long_description_key() const
{
    return tr1::shared_ptr<const MetadataStringKey>();
}

const tr1::shared_ptr<const MetadataContentsKey>
InstalledUnpackagedID::contents_key() const
{
    return _imp->contents_key;
}

const tr1::shared_ptr<const MetadataTimeKey>
InstalledUnpackagedID::installed_time_key() const
{
    return _imp->installed_time_key;
}

const tr1::shared_ptr<const MetadataStringKey>
InstalledUnpackagedID::source_origin_key() const
{
    return _imp->source_origin_key;
}

const tr1::shared_ptr<const MetadataStringKey>
InstalledUnpackagedID::binary_origin_key() const
{
    return _imp->binary_origin_key;
}

const tr1::shared_ptr<const MetadataFSEntryKey>
InstalledUnpackagedID::fs_location_key() const
{
    return _imp->fs_location_key;
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
            NDBAMUnmergerOptions::create()
            .environment(_imp->env)
            .root(_imp->root)
            .contents_file(ver_dir / "contents")
            .config_protect(getenv_with_default("CONFIG_PROTECT", ""))
            .config_protect_mask(getenv_with_default("CONFIG_PROTECT_MASK", ""))
            .ndbam(_imp->ndbam)
            .package_id(shared_from_this()));

    unmerger.unmerge();

    for (DirIterator d(ver_dir, false), d_end ; d != d_end ; ++d)
        FSEntry(*d).unlink();
    ver_dir.rmdir();

    if (last)
    {
        FSEntry pkg_dir(fs_location_key()->value().dirname());
        pkg_dir.rmdir();

        tr1::static_pointer_cast<const InstalledUnpackagedRepository>(repository())->deindex(name());
    }
}

