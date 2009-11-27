/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2009 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_ARGS_ARGS_GROUP_HH
#define PALUDIS_GUARD_ARGS_ARGS_GROUP_HH 1

#include <paludis/args/args_option.hh>
#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <string>

/** \file
 * Declarations for ArgsGroup.
 *
 * \ingroup g_args
 *
 * \section Examples
 *
 * - None at this time.
 */

namespace paludis
{

    namespace args
    {
        class ArgsSection;

        /**
         * Contains a related group of command line arguments.
         *
         * \ingroup g_args
         * \nosubgrouping
         */
        class PALUDIS_VISIBLE ArgsGroup :
            private PrivateImplementationPattern<ArgsGroup>,
            private InstantiationPolicy<ArgsGroup, instantiation_method::NonCopyableTag>
        {
            private:
                const std::string _name;
                const std::string _description;

                ArgsSection * _section;

            public:
                /**
                 * Fetch our section.
                 */
                ArgsSection * section() const
                {
                    return _section;
                }

                /**
                 * Add an ArgsOption instance (called by the ArgsOption
                 * constructor).
                 */
                void add(ArgsOption * const value);

                /**
                 * Remove an ArgsOption instance (called by
                 * ArgsOption::remove).
                 */
                void remove(ArgsOption * const value);

                ///\name Iterate over our ArgsOptions.
                ///\{

                struct ConstIteratorTag;
                typedef WrappedForwardIterator<ConstIteratorTag, ArgsOption * const> ConstIterator;

                ConstIterator begin() const;
                ConstIterator end() const;

                ///\}

                ///\name Basic operations
                ///\{

                ArgsGroup(ArgsSection * s, const std::string & name,
                        const std::string & description);

                ~ArgsGroup();

                ///\}

                /**
                 * Fetch our name.
                 */
                const std::string & name() const
                {
                    return _name;
                }

                /**
                 * Fetch our description.
                 */
                const std::string & description() const
                {
                    return _description;
                }
        };
    }

#ifdef PALUDIS_HAVE_EXTERN_TEMPLATE
    extern template class WrappedForwardIterator<args::ArgsGroup::ConstIteratorTag, args::ArgsOption * const>;
#endif
}

#endif
