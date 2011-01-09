/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_FS_PATH_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_FS_PATH_HH 1

#include <paludis/util/fs_path-fwd.hh>
#include <paludis/util/fs_stat-fwd.hh>
#include <paludis/util/timestamp-fwd.hh>
#include <paludis/util/pimp.hh>
#include <sys/stat.h>

namespace paludis
{
    class PALUDIS_VISIBLE FSPath
    {
        friend std::ostream & paludis::operator<< (std::ostream & s, const FSPath & f);
        friend bool operator== (const FSPath &, const FSPath &);

        private:
            Pimp<FSPath> _imp;

            void _normalise();

        public:
            explicit FSPath(const std::string &);

            FSPath(const FSPath &);

            FSPath & operator= (const FSPath &);

            ~FSPath();

            FSStat stat() const PALUDIS_ATTRIBUTE((warn_unused_result));

            FSPath operator/ (const std::string &) const PALUDIS_ATTRIBUTE((warn_unused_result));
            FSPath operator/ (const FSPath &) const PALUDIS_ATTRIBUTE((warn_unused_result));

            FSPath & operator/= (const std::string &);
            FSPath & operator/= (const FSPath &);

            /**
             * Return the last part of our path (eg '/foo/bar' => 'bar').
             */
            const std::string basename() const PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Return the first part of our path (eg '/foo/bar' => '/foo').
             */
            FSPath dirname() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Return the canonicalised version of our path.
             */
            FSPath realpath() const PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Return the canonicalised version of our path, if it exists, or
             * ourself if it doesn't.
             */
            FSPath realpath_if_exists() const PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Return the path without a given prefix (eg '/foo/bar/baz'->strip_leading('/foo') => '/bar/baz').
             */
            FSPath strip_leading(const FSPath & prefix) const;

            /**
             * Try to make a directory.
             *
             * \return True, if we succeeded, and false if the directory
             *     already exists and is a directory and options includes
             *     fspmkdo_ok_if_exists.
             *
             * \exception FSError If an error other than the directory already
             *   existing occurs, or if the directory already exists and
             *   options does not include fspmkdo_ok_if_exists.
             */
            bool mkdir(const mode_t mode, const FSPathMkdirOptions &) const;

            /**
             * Try to make a symlink.
             *
             * \return True, if we succeeded, and false if the target already
             *   exists and is a symlink to the same target.
             *
             * \exception FSError If an error other than the symlink already
             *   existing occurs, or if the symlink exists and points elsewhere.
             */
            bool symlink(const std::string & target) const;

            /**
             * Try to unlink.
             *
             * \return True, if we succeeded, and false if we don't exist
             *   already.
             *
             * \exception FSError If an error other than us already not
             *   existing occurs.
             */
            bool unlink() const;

            /**
             * Try to rmdir.
             *
             * \return True, if we succeeded, and false if we don't exist
             *   already.
             *
             * \exception FSError If an error other than us already not
             *   existing occurs.
             */
            bool rmdir() const;

            /**
             * Change our ownership, following symlinks.
             *
             * \exception FSError If the chown failed.
             */
            void chown(const uid_t owner, const gid_t group) const;

            /**
             * Change our ownership, not following symlinks.
             *
             * \exception FSError If the lchown failed.
             */
            void lchown(const uid_t owner, const gid_t group) const;

            /**
             * Change our permissions.
             *
             * \exception FSError If the chmod failed.
             */
            void chmod(const mode_t mode) const;

            /**
             * Try to set atime and mtime
             *
             * \return True, if we succeeded, and false if we don't exist
             *   already.
             *
             * \exception FSError If an error other than us already not
             *   existing ocurrs.
             */
            bool utime(const Timestamp &) const;

            /**
             * Return our destination, if we are a symlink.
             *
             * \exception FSError if we are not a symlink, or if the system call
             * fails.
             */
            std::string readlink() const PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Rename ourself (will not work across mount points).
             *
             * \exception FSError If the rename failed.
             */
            void rename(const FSPath & new_name) const;

            /**
             * Return the current working directory
             */
            static FSPath cwd() PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE FSPathComparator
    {
        public:
            bool operator() (const FSPath &, const FSPath &) const;
    };

    extern template class Pimp<FSPath>;
}

#endif
