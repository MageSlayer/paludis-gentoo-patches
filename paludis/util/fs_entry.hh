/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
 * Copyright (c) 2006 Mark Loeser <halcy0n@gentoo.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_FS_ENTRY_HH
#define PALUDIS_GUARD_PALUDIS_FS_ENTRY_HH 1

#include <paludis/util/comparison_policy.hh>
#include <paludis/util/exception.hh>
#include <string>
#include <iosfwd>
#include <tr1/memory>

/** \file
 * Declarations for paludis::Filesystem.
 *
 * \ingroup grpfilesystem
 */

struct stat;

namespace paludis
{
    /**
     * Generic filesystem error class.
     *
     * \ingroup grpexceptions
     * \ingroup grpfilesystem
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE FSError :
        public Exception
    {
        public:
            ///\name Basic operations
            ///\{

            FSError(const std::string & message) throw ();

            ///\}
    };

    /**
     * File permissions used by FSEntry.
     *
     * \ingroup grpfilesystem
     */
    enum FSPermission
    {
        fs_perm_read,       ///< read permission on file
        fs_perm_write,      ///< write permission on file
        fs_perm_execute     ///< execute permission on file
    };

    /**
     * User classes used by FSEntry.
     *
     * \ingroup grpfilesystem
     */
    enum FSUserGroup
    {
        fs_ug_owner,         ///< owner permission
        fs_ug_group,        ///< group permission
        fs_ug_others        ///< others permission
    };

    /**
     * Represents an entry (which may or may not exist) in the filesystem.
     *
     * \ingroup grpfilesystem
     */
    class PALUDIS_VISIBLE FSEntry :
        public ComparisonPolicy<
            FSEntry,
            comparison_mode::FullComparisonTag,
            comparison_method::CompareByMemberTag<std::string> >
    {
        friend std::ostream & operator<< (std::ostream & s, const FSEntry & f);

        private:
            std::string _path;

            mutable std::tr1::shared_ptr<struct ::stat> _stat_info;

            mutable bool _exists;

            /**
             * Whether or not we have run _stat() on this location yet
             */
            mutable bool _checked;

            void _normalise();

            /**
             * Runs lstat() on the current path if we have not done so already.
             * Note: lstat() will give information on the symlink itself, and not what
             * the link points to, which is how stat() works.
             */
            void _stat() const;

        public:
            ///\name Basic operations
            ///\{

            FSEntry(const std::string & path);

            FSEntry(const FSEntry & other);

            ~FSEntry();

            const FSEntry & operator= (const FSEntry & other);

            ///\}

            ///\name Modification operations
            ///\{

            /**
             * Join with another FSEntry.
             */
            FSEntry operator/ (const FSEntry & rhs) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Append another FSEntry.
             */
            const FSEntry & operator/= (const FSEntry & rhs);

            /**
             * Join with another path.
             */
            FSEntry operator/ (const std::string & rhs) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Append another path.
             */
            const FSEntry & operator/= (const std::string & rhs)
            {
                return operator/= (FSEntry(rhs));
            }

            /**
             * Return the last part of our path (eg '/foo/bar' => 'bar').
             */
            std::string basename() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Return the path without a given prefix (eg '/foo/bar/baz'->strip_leading('/foo') => '/bar/baz').
             */
            FSEntry strip_leading(const FSEntry & prefix) const;

            /**
             * Return the first part of our path (eg '/foo/bar' => '/foo').
             */
            FSEntry dirname() const
                PALUDIS_ATTRIBUTE((warn_unused_result));


            ///\}

            ///\name Filesystem queries
            ///\{

            /**
             * Does a filesystem entry exist at our location?
             */
            bool exists() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Does a filesystem entry exist at our location, and if it does,
             * is it a directory?
             */
            bool is_directory() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Does a filesystem entry exist at our location, and if it does,
             * is it a directory?
             */
            bool is_directory_or_symlink_to_directory() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Does a filesystem entry exist at our location, and if it does,
             * is it a regular file?
             */
            bool is_regular_file() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Does a filesystem entry exist at our location, and if it does,
             * is it a regular file?
             */
            bool is_regular_file_or_symlink_to_regular_file() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Does a filesystem entry exist at our location, and if it does,
             * is it a symbolic link?
             */
            bool is_symbolic_link() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Whether we exist and are a device file.
             */
            bool is_device() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Whether we exist and are a fifo.
             */
            bool is_fifo() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Check if filesystem entry has `perm` for `user_group`.
             *
             * \exception FSError if there was a problem accessing the filesystem entry
             */
            bool has_permission(const FSUserGroup & user_group, const FSPermission & fs_perm) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Return the permissions for our item.
             *
             * \exception FSError if there was a problem accessing the filesystem entry
             */
            mode_t permissions() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Return the canonicalised version of our path.
             */
            FSEntry realpath() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Return the canonicalised version of our path, if it exists, or
             * ourself if it doesn't.
             */
            FSEntry realpath_if_exists() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Return our destination, if we are a symlink.
             *
             * \exception FSError if we are not a symlink, or if the system call
             * fails.
             */
            std::string readlink() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Return the time the inode for the filesystem entry was last modified
             * \exception FSError if there was a problem accessing the filesystem entry
             */
            time_t ctime() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Return the time the filesystem entry was last modified
             * \exception FSError if there was a problem accessing the filesystem entry
             */
            time_t mtime() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Return the size of our file, in bytes.
             *
             * \exception FSError if we don't have a size.
             */
            off_t file_size() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Fetch our owner.
             *
             * \exception FSError If we don't exist or the stat call fails.
             */
            uid_t owner() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Fetch our group.
             *
             * \exception FSError If we don't exist or the stat call fails.
             */
            gid_t group() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Return the current working directory
             */
            static FSEntry cwd()
                PALUDIS_ATTRIBUTE((warn_unused_result));

            ///\}

            ///\name Filesystem operations
            ///\{

            /**
             * Try to make a directory.
             *
             * \return True, if we succeeded, and false if the directory
             *   already exists and is a directory.
             *
             * \exception FSError If an error other than the directory already
             *   existing occurs.
             */
            bool mkdir(const mode_t mode = 0755);

            /**
             * Try to unlink.
             *
             * \return True, if we succeeded, and false if we don't exist
             *   already.
             *
             * \exception FSError If an error other than us already not
             *   existing occurs.
             */
            bool unlink();

            /**
             * Try to rmdir.
             *
             * \return True, if we succeeded, and false if we don't exist
             *   already.
             *
             * \exception FSError If an error other than us already not
             *   existing occurs.
             */
            bool rmdir();

            /**
             * Change our permissions.
             *
             * \exception FSError If the chown failed.
             */
            void chown(const uid_t owner, const gid_t group = -1);

            /**
             * Change our permissions.
             *
             * \exception FSError If the chmod failed.
             */
            void chmod(const mode_t mode);

            /**
             * Rename ourself (will not work across mount points).
             *
             * \exception FSError If the rename failed.
             */
            void rename(const FSEntry & new_name);

            ///\}
    };

    /**
     * An FSEntry can be written to an ostream.
     *
     * \ingroup grpfilesystem
     */
    std::ostream & operator<< (std::ostream & s, const FSEntry & f) PALUDIS_VISIBLE;

    template <typename T_> class SequentialCollection;

    /**
     * An ordered group of FSEntry instances.
     *
     * \ingroup grpfilesystem
     */
    typedef SequentialCollection<FSEntry> FSEntryCollection;
}

#endif
