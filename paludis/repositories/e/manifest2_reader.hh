/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Mike Kelly
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
#include <paludis/util/wrapped_forward_iterator-fwd.hh>
#include <string>

/** \file
 * Declarations for the Manifest2Reader class
 *
 * \ingroup grpmanifest2reader
 */

namespace paludis
{
    namespace n
    {
        typedef Name<struct md5_name> md5;
        typedef Name<struct name_name> name;
        typedef Name<struct rmd160_name> rmd160;
        typedef Name<struct sha1_name> sha1;
        typedef Name<struct sha256_name> sha256;
        typedef Name<struct size_name> size;
        typedef Name<struct type_name> type;
    }

    namespace erepository
    {
        struct Manifest2Entry
        {
            NamedValue<n::md5, std::string> md5;
            NamedValue<n::name, std::string> name;
            NamedValue<n::rmd160, std::string> rmd160;
            NamedValue<n::sha1, std::string> sha1;
            NamedValue<n::sha256, std::string> sha256;
            NamedValue<n::size, off_t> size;
            NamedValue<n::type, std::string> type;
        };

        /**
         * Thrown if a Manifest2 file cannot be read properly.
         *
         * \ingroup grpexceptions
         * \nosubgrouping
         */
        class PALUDIS_VISIBLE Manifest2Error :
            public Exception
        {
            public:
                Manifest2Error(const std::string & msg) throw ();
        };

        /**
         * Gives an ConstIterator of Manifest2Entrys from a given Manifest 2 style
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

                struct ConstIteratorTag;
                typedef WrappedForwardIterator<ConstIteratorTag, const Manifest2Entry> ConstIterator;
                ConstIterator begin() const PALUDIS_ATTRIBUTE((warn_unused_result));
                ConstIterator end() const PALUDIS_ATTRIBUTE((warn_unused_result));

                ConstIterator find(const std::string & type, const std::string & name) const PALUDIS_ATTRIBUTE((warn_unused_result));
                ConstIterator find(const std::pair<const std::string, const std::string> & p) const PALUDIS_ATTRIBUTE((warn_unused_result));

                ///}
        };
    }
}

#endif
