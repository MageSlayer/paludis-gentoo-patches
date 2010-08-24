/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_EBUILD_FLAT_METADATA_CACHE_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_EBUILD_FLAT_METADATA_CACHE_HH 1

#include <paludis/repositories/e/ebuild.hh>
#include <paludis/repositories/e/ebuild_id.hh>
#include <paludis/repositories/e/eclass_mtimes.hh>
#include <paludis/util/pimp.hh>

namespace paludis
{
    namespace erepository
    {
        /**
         * Implements metadata cache handling for a ERepository using
         * EbuildEntries.
         *
         * \see EbuildEntries
         * \see ERepository
         * \ingroup grperepository
         * \nosubgrouping
         */
        class EbuildFlatMetadataCache :
            private Pimp<EbuildFlatMetadataCache>
        {
            public:
                ///\name Basic operations
                ///\{

                EbuildFlatMetadataCache(const Environment * const, const FSPath & filename, const FSPath & ebuild,
                        time_t master_mtime, const std::shared_ptr<const EclassMtimes> & eclass_mtimes, bool silent);
                ~EbuildFlatMetadataCache();

                ///\}

                ///\name Cache operations
                ///\{

                bool load(const std::shared_ptr<const EbuildID> &, const bool silent_on_stale);
                void save(const std::shared_ptr<const EbuildID> &);

                ///\}
        };
    }

    extern template class Pimp<erepository::EbuildFlatMetadataCache>;
}

#endif
