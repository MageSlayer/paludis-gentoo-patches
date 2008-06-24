/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008 David Leverton
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

#ifndef PALUDIS_GUARD_PALUDIS_MERGER_FWD_HH
#define PALUDIS_GUARD_PALUDIS_MERGER_FWD_HH 1

#include <iosfwd>
#include <paludis/util/attributes.hh>
#include <paludis/util/options-fwd.hh>
#include <paludis/util/kc-fwd.hh>
#include <paludis/util/keys.hh>
#include <paludis/util/fs_entry-fwd.hh>
#include <paludis/environment-fwd.hh>
#include <tr1/functional>

/** \file
 * Forward declarations for paludis/merger.hh .
 *
 * \ingroup g_repository
 */

namespace paludis
{
#include <paludis/merger-se.hh>

    /**
     * Boolean options for Merger.
     *
     * \ingroup g_repository
     * \since 0.26
     */
    typedef Options<MergerOption> MergerOptions;

    /**
     * Status flags for Merger.
     *
     * \ingroup g_repository
     * \since 0.26
     */
    typedef Options<MergeStatusFlag> MergeStatusFlags;

    /**
     * Parameters for a basic Merger.
     *
     * \see Merger
     * \ingroup g_repository
     * \nosubgrouping
     * \since 0.26
     */
    typedef kc::KeyedClass<
        kc::Field<k::environment, Environment *>,
        kc::Field<k::image, FSEntry>,
        kc::Field<k::root, FSEntry>,
        kc::Field<k::no_chown, bool>,
        kc::Field<k::options, MergerOptions>,
        kc::Field<k::install_under, FSEntry>,
        kc::Field<k::get_new_ids_or_minus_one, std::tr1::function<std::pair<uid_t, gid_t> (const FSEntry &)> >
            > MergerParams;

    class MergerError;
    class Merger;
}

#endif
