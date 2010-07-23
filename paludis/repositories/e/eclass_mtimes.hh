/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_ECLASS_MTIMES_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_ECLASS_MTIMES_HH 1

#include <paludis/name-fwd.hh>
#include <paludis/util/pimp.hh>
#include <paludis/util/fs_entry-fwd.hh>
#include <memory>

namespace paludis
{
    class ERepository;

    /**
     * Holds an eclass mtimes cache for an ERepository.
     *
     * \see ERepository
     * \ingroup grperepository
     * \nosubgrouping
     */
    class EclassMtimes :
        private Pimp<EclassMtimes>
    {
        public:
            ///\name Basic operations
            ///\{

            EclassMtimes(const ERepository *, const std::shared_ptr<const FSEntrySequence> &);
            ~EclassMtimes();

            ///\}

            /**
             * Fetch the full path of a given eclass.
             */
            const FSEntry * eclass(const std::string &) const;

            /**
             * Fetch the full path of a given exlib, on the path of a given package.
             */
            const FSEntry * exlib(const std::string &, const QualifiedPackageName &) const;
    };
}

#endif
