/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_HOOK_HH
#define PALUDIS_GUARD_PALUDIS_HOOK_HH 1

#include <paludis/util/attributes.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <utility>
#include <string>
#include <libwrapiter/libwrapiter_forward_iterator.hh>

namespace paludis
{
    /**
     * Represents the data for an Environment hook call.
     *
     * \ingroup grpenvironment
     * \nosubgrouping
     */
    class Hook :
        private PrivateImplementationPattern<Hook>
    {
        public:
            ///\name Basic operations
            ///\{

            Hook(const std::string & name);

            Hook(const Hook &);

            ~Hook();

            ///\}

            /// Add data to the hook.
            Hook operator() (const std::string & key, const std::string & value) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            ///\name Iterate over environment data
            ///\{

            typedef libwrapiter::ForwardIterator<Hook, const std::pair<const std::string, std::string> > Iterator;

            Iterator begin() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            Iterator end() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            ///\}

            /// Our name.
            std::string name() const
                PALUDIS_ATTRIBUTE((warn_unused_result));
    };
}

#endif
