/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010 Ciaran McCreesh
 * Copyright (c) 2006 Mark Loeser
 * Copyright (c) 2008 Fernando J. Pereda
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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_FS_STAT_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_FS_STAT_HH 1

#include <paludis/util/fs_stat-fwd.hh>
#include <paludis/util/fs_path-fwd.hh>
#include <paludis/util/pimp.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/timestamp-fwd.hh>
#include <utility>
#include <sys/stat.h>

namespace paludis
{
    class PALUDIS_VISIBLE FSStat :
        private Pimp<FSStat>
    {
        public:
            explicit FSStat(const FSPath &);

            FSStat(const FSStat &);

            FSStat & operator= (const FSStat &);

            ~FSStat();

            bool exists() const PALUDIS_ATTRIBUTE((warn_unused_result));
            bool is_regular_file() const PALUDIS_ATTRIBUTE((warn_unused_result));
            bool is_regular_file_or_symlink_to_regular_file() const PALUDIS_ATTRIBUTE((warn_unused_result));
            bool is_directory() const PALUDIS_ATTRIBUTE((warn_unused_result));
            bool is_directory_or_symlink_to_directory() const PALUDIS_ATTRIBUTE((warn_unused_result));
            bool is_symlink() const PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Return the time the inode for the filesystem entry was last modified
             *
             * \exception FSError if there was a problem accessing the filesystem entry
             */
            Timestamp ctim() const PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Return the time the filesystem entry was last modified
             *
             * \exception FSError if there was a problem accessing the filesystem entry
             */
            Timestamp mtim() const PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Return the permissions for our item.
             *
             * \exception FSError if there was a problem accessing the filesystem entry
             */
            mode_t permissions() const PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Return the size of our file, in bytes.
             *
             * \exception FSError if we don't have a size.
             */
            off_t file_size() const PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Fetch our owner.
             *
             * \exception FSError If we don't exist or the stat call fails.
             */
            uid_t owner() const PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Fetch our group.
             *
             * \exception FSError If we don't exist or the stat call fails.
             */
            gid_t group() const PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Return an unique low-level id for this entry
             */
            std::pair<dev_t, ino_t> lowlevel_id() const
                PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    extern template class Pimp<FSStat>;
}

#endif
