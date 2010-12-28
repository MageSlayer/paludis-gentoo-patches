/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010 Ciaran McCreesh
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
#include <paludis/util/set.hh>
#include <paludis/util/sequence.hh>
#include <paludis/contents.hh>
#include <paludis/ndbam.hh>
#include <functional>

using namespace paludis;
using namespace paludis::erepository;

namespace
{
    class ExndbamContentsKey :
        public MetadataValueKey<std::shared_ptr<const Contents> >
    {
        private:
            const PackageID * const _id;
            const NDBAM * const _db;
            mutable Mutex _mutex;
            mutable std::shared_ptr<Contents> _v;

        public:
            ExndbamContentsKey(const PackageID * const i, const NDBAM * const d) :
                _id(i),
                _db(d)
            {
            }

            const std::shared_ptr<const Contents> value() const
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

            virtual void invalidate() const
            {
                Lock l(_mutex);
                _v.reset();
            }
    };
}

ExndbamID::ExndbamID(const QualifiedPackageName & q, const VersionSpec & v,
        const Environment * const e,
        const RepositoryName & r,
        const FSPath & f, const NDBAM * const n) :
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

std::shared_ptr<MetadataValueKey<std::shared_ptr<const Contents> > >
ExndbamID::make_contents_key() const
{
    return std::make_shared<ExndbamContentsKey>(this, _ndbam);
}


