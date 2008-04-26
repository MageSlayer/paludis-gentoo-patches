/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_UNPACKAGED_NDBAM_FWD_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_UNPACKAGED_NDBAM_FWD_HH 1

#include <paludis/util/sequence-fwd.hh>
#include <paludis/util/kc-fwd.hh>
#include <paludis/util/keys.hh>
#include <paludis/util/fs_entry-fwd.hh>
#include <paludis/util/mutex-fwd.hh>
#include <paludis/name-fwd.hh>
#include <paludis/version_spec-fwd.hh>
#include <paludis/package_id-fwd.hh>

namespace paludis
{
    class NDBAM;

    typedef kc::KeyedClass<
        kc::Field<k::name, QualifiedPackageName>,
        kc::Field<k::version, VersionSpec>,
        kc::Field<k::slot, SlotName>,
        kc::Field<k::fs_location, FSEntry>,
        kc::Field<k::magic, std::string>,
        kc::Field<k::package_id, std::tr1::shared_ptr<PackageID> >,
        kc::Field<k::mutex, std::tr1::shared_ptr<Mutex> >
            > NDBAMEntry;

    typedef Sequence<std::tr1::shared_ptr<NDBAMEntry> > NDBAMEntrySequence;
}

#endif
