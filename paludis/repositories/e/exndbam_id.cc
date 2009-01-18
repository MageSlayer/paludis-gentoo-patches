/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009 Ciaran McCreesh
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

#include <paludis/repositories/e/exndbam_id.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/set.hh>
#include <paludis/util/sequence.hh>
#include <paludis/contents.hh>
#include <paludis/ndbam.hh>
#include <tr1/functional>

using namespace paludis;
using namespace paludis::erepository;

namespace
{
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

    class ExndbamContentsKey :
        public MetadataValueKey<std::tr1::shared_ptr<const Contents> > 
    {
        private:
            const PackageID * const _id;
            const NDBAM * const _db;
            mutable Mutex _mutex;
            mutable std::tr1::shared_ptr<Contents> _v;

        public:
            ExndbamContentsKey(const PackageID * const i, const NDBAM * const d) :
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
                        std::tr1::bind(&create_file, std::tr1::ref(*_v), _1),
                        std::tr1::bind(&create_dir, std::tr1::ref(*_v), _1),
                        std::tr1::bind(&create_sym, std::tr1::ref(*_v), _1, _2)
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
}

ExndbamID::ExndbamID(const QualifiedPackageName & q, const VersionSpec & v,
        const Environment * const e,
        const std::tr1::shared_ptr<const Repository> & r,
        const FSEntry & f, const NDBAM * const n) :
    EInstalledRepositoryID(q, v, e, r, f),
    _ndbam(n)
{
}

std::string
ExndbamID::fs_location_raw_name() const
{
    return "EXNDBAMDIR";
}

std::string
ExndbamID::fs_location_human_name() const
{
    return "Exndbam Directory";
}

std::string
ExndbamID::contents_filename() const
{
    return "contents";
}

std::tr1::shared_ptr<MetadataValueKey<std::tr1::shared_ptr<const Contents> > >
ExndbamID::make_contents_key() const
{
    return make_shared_ptr(new ExndbamContentsKey(this, _ndbam));
}


