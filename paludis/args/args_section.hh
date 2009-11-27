/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_ARGS_ARGS_SECTION_HH
#define PALUDIS_GUARD_PALUDIS_ARGS_ARGS_SECTION_HH 1

#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <string>

namespace paludis
{
    namespace args
    {
        struct ArgsGroup;
        struct ArgsHandler;

        /**
         * Holds a number of ArgsGroup instances.
         *
         * \since 0.40
         * \ingroup g_args
         */
        class PALUDIS_VISIBLE ArgsSection :
            private PrivateImplementationPattern<ArgsSection>
        {
            public:
                ArgsSection(ArgsHandler * const, const std::string &);
                ~ArgsSection();

                struct GroupsConstIteratorTag;
                typedef WrappedForwardIterator<GroupsConstIteratorTag, const ArgsGroup> GroupsConstIterator;
                GroupsConstIterator begin() const PALUDIS_ATTRIBUTE((warn_unused_result));
                GroupsConstIterator end() const PALUDIS_ATTRIBUTE((warn_unused_result));

                ArgsHandler * handler() const PALUDIS_ATTRIBUTE((warn_unused_result));
                const std::string name() const PALUDIS_ATTRIBUTE((warn_unused_result));

                void add(ArgsGroup * const);
        };
    }

#ifdef PALUDIS_HAVE_EXTERN_TEMPLATE
    extern template class PrivateImplementationPattern<args::ArgsSection>;
    extern template class WrappedForwardIterator<args::ArgsSection::GroupsConstIteratorTag, const args::ArgsGroup>;
#endif
}

#endif
