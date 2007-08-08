/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Mike Kelly <pioto@pioto.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_MANIFEST2_READER_HH
#define PALUDIS_GUARD_PALUDIS_MANIFEST2_READER_HH 1

#include <paludis/action.hh>
#include <paludis/util/fs_entry-fwd.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/sr.hh>

#include <paludis/repositories/e/manifest2_entry-sr.hh>

#include <string>

/** \file
 * Declarations for the Manifest2Reader class
 *
 * \ingroup grpmanifest2reader
 */

namespace paludis
{

    namespace erepository
    {
        /**
         * Thrown if a Manifest2 file cannot be read properly.
         *
         * \ingroup grpexceptions
         * \nosubgrouping
         */
        class PALUDIS_VISIBLE Manifest2Error :
            public ActionError
        {
            public:
                Manifest2Error(const std::string & msg) throw ();
        };

        /**
         * Gives an Iterator of Manifest2Entrys from a given Manifest 2 style
         * file.
         *
         * \ingroup grpmanifest2reader
         */
        class PALUDIS_VISIBLE Manifest2Reader :
            private PrivateImplementationPattern<Manifest2Reader>
        {
            public:
                ///\name Basic operations

                Manifest2Reader(const FSEntry & f);
                ~Manifest2Reader();

                ///}
                
                /// \name Iterator functions
                
                typedef libwrapiter::ForwardIterator<Manifest2Reader, const Manifest2Entry> Iterator;
                Iterator begin() const PALUDIS_ATTRIBUTE((warn_unused_result));
                Iterator end() const PALUDIS_ATTRIBUTE((warn_unused_result));

                ///}
        };
    }
}

#endif
