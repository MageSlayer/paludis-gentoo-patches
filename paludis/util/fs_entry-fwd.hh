/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_FS_ENTRY_FWD_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_FS_ENTRY_FWD_HH 1

#include <iosfwd>
#include <paludis/util/attributes.hh>
#include <paludis/util/set-fwd.hh>
#include <paludis/util/sequence-fwd.hh>

/** \file
 * Forward declarations for paludis/util/fs_entry.hh .
 *
 * \ingroup g_fs
 */

namespace paludis
{
    class FSError;

    /**
     * File permissions used by FSEntry.
     *
     * \ingroup g_fs
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
     * \ingroup g_fs
     */
    enum FSUserGroup
    {
        fs_ug_owner,         ///< owner permission
        fs_ug_group,        ///< group permission
        fs_ug_others        ///< others permission
    };

    class FSEntry;

    /**
     * An FSEntry can be written to an ostream.
     *
     * \ingroup g_fs
     */
    std::ostream & operator<< (std::ostream & s, const FSEntry & f) PALUDIS_VISIBLE;

    /**
     * A sequence of FSEntry instances.
     *
     * \ingroup g_fs
     */
    typedef Sequence<FSEntry> FSEntrySequence;

    /**
     * A set of FSEntry instances.
     *
     * \ingroup g_fs
     */
    typedef Set<FSEntry> FSEntrySet;
}

#endif
