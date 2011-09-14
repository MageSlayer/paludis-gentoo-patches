/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_MAINTAINER_HH
#define PALUDIS_GUARD_PALUDIS_MAINTAINER_HH 1

#include <paludis/maintainer-fwd.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/named_value.hh>

namespace paludis
{
    namespace n
    {
        typedef Name<struct name_author> author;
        typedef Name<struct name_description> description;
        typedef Name<struct name_email> email;
    }

    /**
     * Represents a package maintainer.
     *
     * \since 0.68
     * \ingroup g_metadata_key
     */
    struct Maintainer
    {
        NamedValue<n::author, std::string> author;
        NamedValue<n::description, std::string> description;
        NamedValue<n::email, std::string> email;
    };

    extern template class Sequence<Maintainer>;
    extern template class WrappedForwardIterator<Sequence<Maintainer>::ConstIteratorTag, const Maintainer>;
}

#endif
