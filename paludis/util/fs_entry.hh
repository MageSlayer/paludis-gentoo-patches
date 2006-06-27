/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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
#include <paludis/util/counted_ptr.hh>
#include <paludis/util/exception.hh>
#include <string>
#include <iosfwd>

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
     */
    class FSError : public Exception
    {
        public:
            /**
             * Constructor.
             */
            FSError(const std::string & message) throw ();
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
    class FSEntry : public ComparisonPolicy<
                        FSEntry,
                        comparison_mode::FullComparisonTag,
                        comparison_method::CompareByMemberTag<std::string> >
    {
        friend std::ostream & operator<< (std::ostream & s, const FSEntry & f);

        private:
            std::string _path;

            mutable CountedPtr<struct ::stat, count_policy::ExternalCountTag> _stat_info;

            mutable bool _exists;

            /**
             * Whether or not we have run _stat() on this location yet
             */
            mutable bool _checked;

            void _normalise();

            /**
             * Runs lstat() on the current path if we have not done so already
             * Note: lstat() will give information on the symlink itself, and not what
             * the link points to, which is how stat() works.
             */
            void _stat() const;

        public:
            /**
             * Constructor, from a path.
             */
            FSEntry(const std::string & path);

            /**
             * Copy constructor.
             */
            FSEntry(const FSEntry & other);

            /**
             * Destructor.
             */
            ~FSEntry();

            /**
             * Assignment, from another FSEntry.
             */
            const FSEntry & operator= (const FSEntry & other);

            /**
             * Join with another FSEntry.
             */
            FSEntry operator/ (const FSEntry & rhs) const;

            /**
             * Append another FSEntry.
             */
            const FSEntry & operator/= (const FSEntry & rhs);

            /**
             * Join with another path.
             */
            FSEntry operator/ (const std::string & rhs) const;

            /**
             * Append another path.
             */
            const FSEntry & operator/= (const std::string & rhs)
            {
                return operator/= (FSEntry(rhs));
            }

            /**
             * Does a filesystem entry exist at our location?
             */
            bool exists() const;

            /**
             * Does a filesystem entry exist at our location, and if it does,
             * is it a directory?
             */
            bool is_directory() const;

            /**
             * Does a filesystem entry exist at our location, and if it does,
             * is it a regular file?
             */
            bool is_regular_file() const;

            /**
             * Does a filesystem entry exist at our location, and if it does,
             * is it a symbolic link?
             */
            bool is_symbolic_link() const;

            /**
             * Check if filesystem entry has `perm` for `user_group`.
             *
             * \exception FSError if there was a problem accessing the filesystem entry
             */
            bool has_permission(const FSUserGroup & user_group, const FSPermission & fs_perm) const;

            /**
             * Return the permissions for our item.
             *
             * \exception FSError if there was a problem accessing the filesystem entry
             */
            mode_t permissions() const;

            /**
             * Return the last part of our path (eg '/foo/bar' => 'bar').
             */
            std::string basename() const;

            /**
             * Return the first part of our path (eg '/foo/bar' => '/foo').
             */
            FSEntry dirname() const;

            /**
             * Return the canonicalised version of our path.
             */
            FSEntry realpath() const;

            /**
             * Return the time the filesystem entry was created
             * \exception FSError if there was a problem accessing the filesystem entry
             */
            time_t ctime() const;

            /**
             * Return the time the filesystem entry was last modified
             * \exception FSError if there was a problem accessing the filesystem entry
             */
            time_t mtime() const;

            /**
             * Return the size of our file, in bytes.
             *
             * \exception FSError if we don't have a size.
             */
            off_t file_size() const;

            /**
             * Try to make a directory.
             *
             * \return True, if we succeeded, and false if the directory
             *   already exists and is a directory.
             *
             * \exception FSError If an error other than the directory already
             *   existing occurs.
             */
            bool mkdir(mode_t mode = 0755);

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
             * Return the current working directory
             */
            static FSEntry cwd();
    };

    /**
     * An FSEntry can be written to an ostream.
     *
     * \ingroup grpfilesystem
     */
    std::ostream & operator<< (std::ostream & s, const FSEntry & f);

    template <typename T_> class SequentialCollection;

    /**
     * An ordered group of FSEntry instances.
     *
     * \ingroup grpfilesystem
     */
    typedef SequentialCollection<FSEntry> FSEntryCollection;
}

#endif
