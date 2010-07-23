/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010 Ciaran McCreesh
 * Copyright (c) 2007 Piotr Jaroszyński
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
#include <paludis/util/pimp.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/sequence-fwd.hh>
#include <paludis/util/named_value.hh>
#include <memory>
#include <string>

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
    namespace n
    {
        typedef Name<struct max_exit_status_name> max_exit_status;
        typedef Name<struct output_name> output;
    }

    /**
     * Result of a Hook.
     *
     * \see Hook
     * \ingroup g_hooks
     * \nosubgrouping
     */
    struct HookResult
    {
        NamedValue<n::max_exit_status, int> max_exit_status;
        NamedValue<n::output, std::string> output;
    };

    /**
     * Represents the data for a hook call.
     *
     * \see Environment::perform_hook
     * \ingroup g_hooks
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE Hook :
        private Pimp<Hook>
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

            struct ConstIteratorTag;
            typedef WrappedForwardIterator<ConstIteratorTag, const std::pair<const std::string, std::string> > ConstIterator;

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
        private Pimp<Hook::AllowedOutputValues>
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

    extern template class WrappedForwardIterator<Hook::ConstIteratorTag, const std::pair<const std::string, std::string> >;
}

extern "C" paludis::HookResult PALUDIS_VISIBLE paludis_hook_run(
    const paludis::Environment *, const paludis::Hook &);

extern "C" void PALUDIS_VISIBLE paludis_hook_add_dependencies(
    const paludis::Environment *, const paludis::Hook &, paludis::DirectedGraph<std::string, int> &);

extern "C" const std::shared_ptr<const paludis::Sequence<std::string> > PALUDIS_VISIBLE paludis_hook_auto_phases(
    const paludis::Environment *);

#endif
