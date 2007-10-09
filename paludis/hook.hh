/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
 * Copyright (c) 2007 Piotr Jaroszyński <peper@gentoo.org>
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

#include <paludis/hook-fwd.hh>
#include <paludis/environment-fwd.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/graph-fwd.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/operators.hh>

#include <string>
#include <libwrapiter/libwrapiter_forward_iterator-fwd.hh>

/** \file
 * Declarations for the Hook classes.
 *
 * \ingroup g_hooks
 *
 * \section Examples
 *
 * - None at this time.
 */

namespace paludis
{

#include <paludis/hook-sr.hh>

    /**
     * Represents the data for a hook call.
     *
     * \see Environment::perform_hook
     * \ingroup g_hooks
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE Hook :
        private PrivateImplementationPattern<Hook>
    {
        public:
            ///\name Basic operations
            ///\{

            class AllowedOutputValues;

            HookOutputDestination output_dest;

            Hook(const std::string & name);

            Hook(const Hook &);

            ~Hook();

            ///\}

            /// Add data to the hook.
            Hook operator() (const std::string & key, const std::string & value) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /// Get data from the hook.
            std::string get(const std::string & key) const
                 PALUDIS_ATTRIBUTE((warn_unused_result));

            Hook grab_output(const AllowedOutputValues & av);

            bool validate_value(const std::string & value) const;

            ///\name Iterate over environment data
            ///\{

            typedef libwrapiter::ForwardIterator<Hook, const std::pair<const std::string, std::string> > ConstIterator;

            ConstIterator begin() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            ConstIterator end() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            ///\}

            /// Our name.
            std::string name() const
                PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    /**
     * Hooks with grabbed output can specify that only certain output values are
     * allowed.
     *
     * \ingroup g_hooks
     * \since 0.26
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE Hook::AllowedOutputValues :
        private PrivateImplementationPattern<Hook::AllowedOutputValues>
    {
        friend class Hook;

        public:
            ///\name Basic operations
            ///\{

            AllowedOutputValues();

            AllowedOutputValues(const AllowedOutputValues & other);

            ~AllowedOutputValues();

            ///\}

            /// Add a new allowed value.
            AllowedOutputValues operator() (const std::string & v) const
                PALUDIS_ATTRIBUTE((warn_unused_result));
    };
}

extern "C" paludis::HookResult PALUDIS_VISIBLE paludis_hook_run(
    const paludis::Environment *, const paludis::Hook &);

extern "C" void PALUDIS_VISIBLE paludis_hook_add_dependencies(
    const paludis::Environment *, const paludis::Hook &, paludis::DirectedGraph<std::string, int> &);

#endif
