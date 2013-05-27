/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2013 Saleem Abdulrasool <compnerd@compnerd.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_PARTITIONING_HH
#define PALUDIS_GUARD_PALUDIS_PARTITIONING_HH 1

#include <paludis/name-fwd.hh>
#include <paludis/partitioning-fwd.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/fs_path.hh>
#include <paludis/util/pimp.hh>

#include <string>
#include <vector>

/** \file
 * Declarations for the Partitioning class.
 *
 * \ingroup g_contents
 */

namespace paludis
{
    /**
     * Package partitioning as defined by the package installation metadata.
     *
     * \ingroup g_contents
     * \since 1.1.0
     */
    class PALUDIS_VISIBLE Partitioning
    {
        private:
            Pimp<Partitioning> _imp;

        public:
            ///\name Basic operations
            ///\{

            Partitioning();
            ~Partitioning();

            ///\}

            /**
             * Mark a set of paths as belonging to a partition.
             *
             * \arg [in] paths  the paths to partition
             * \arg [in] name   the partition name (or empty to indicate core)
             *
             * \since 1.1.0
             */
            void mark(const std::vector<FSPath> &, const PartName &);

            /**
             * Classify a path into a partition.
             *
             * \arg [in] path   the path to classify
             *
             * \return the partition the path was classified into, empty if none
             *
             * \since 1.1.0
             */
            PartName classify(const FSPath &) const;

            /**
             * Check if a path is partitioned.
             *
             * \arg [in] path   the path to check
             *
             * \return %true if the path contains a partioned image
             *
             * \since 1.99.0
             */
            bool is_partitioned(const FSPath &) const;
    };
}

#endif
