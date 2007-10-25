/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_ARGS_ARGS_HANDLER_HH
#define PALUDIS_GUARD_ARGS_ARGS_HANDLER_HH 1

#include <paludis/args/args_group.hh>
#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/private_implementation_pattern.hh>

#include <iosfwd>
#include <string>

#include <libwrapiter/libwrapiter_forward_iterator-fwd.hh>

/** \file
 * Declarations for ArgsHandler.
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
        /**
         * Handles command line arguments.
         *
         * \ingroup g_args
         * \nosubgrouping
         */
        class PALUDIS_VISIBLE ArgsHandler :
            private InstantiationPolicy<ArgsHandler, instantiation_method::NonCopyableTag>,
            private PrivateImplementationPattern<ArgsHandler>
        {
            friend class ArgsGroup;
            friend std::ostream & operator<< (std::ostream &, const ArgsHandler &);

            protected:
                /**
                 * Add a new usage line.
                 */
                void add_usage_line(const std::string & l);

                /**
                 * Add a new environment line.
                 */
                void add_environment_variable(const std::string & e, const std::string & desc);

                /**
                 * Add a new example.
                 */
                void add_example(const std::string & e, const std::string & desc);

                /**
                 * Add an new ArgsGroup (called by the ArgsGroup constructor).
                 */
                void add(ArgsGroup * const);

                /**
                 * Dump, for --help output (called by operator<<).
                 */
                void dump_to_stream(std::ostream & s) const;

            public:
                ///\name Basic operations
                ///\{

                ArgsHandler();

                virtual ~ArgsHandler();

                ///\}

                /**
                 * \deprecated Use the five arg form.
                 */
                void run(const int, const char * const * const, const std::string & env_var = "",
                        const std::string & env_prefix = "") PALUDIS_ATTRIBUTE((deprecated));

                /**
                 * Parse command line arguments. The third argument is used to
                 * set PALUDIS_CLIENT.  The fourth argument is the name of an
                 * environment variable holding arguments which are prepended
                 * to the command line arguments. The fifth argument is used as
                 * a prefix to export our command line via the environment.
                 */
                virtual void run(const int, const char * const * const,
                        const std::string & client, const std::string & env_var,
                        const std::string & env_prefix);

                ///\name Iterate over our parameters (non - and -- switches and their values)
                ///\{

                typedef libwrapiter::ForwardIterator<ArgsHandler, const std::string> ParametersConstIterator;

                ParametersConstIterator begin_parameters() const;

                ParametersConstIterator end_parameters() const;

                bool empty() const;

                ///\}

                /**
                 * Add an ArgsOption instance.
                 */
                void add_option(ArgsOption * const, const std::string & long_name,
                        const char short_name = '\0');

                /**
                 * Remove an ArgsOption instance.
                 */
                void remove_option(const std::string & long_name, const char short_name = '\0');

                ///\name About our application (for documentation)
                ///\{

                /**
                 * What is our application name?
                 */
                virtual std::string app_name() const = 0;

                /**
                 * What is our application's Unix manual section?
                 */
                virtual std::string man_section() const
                {
                    return "1";
                }

                /**
                 * One line synopsis of what our application is.
                 */
                virtual std::string app_synopsis() const = 0;

                /**
                 * Long description of what our application is.
                 */
                virtual std::string app_description() const = 0;

                ///\}

                ///\name Iterate over our usage lines (for documentation)
                ///\{

                typedef libwrapiter::ForwardIterator<ArgsHandler, const std::string> UsageLineConstIterator;

                UsageLineConstIterator begin_usage_lines() const;

                UsageLineConstIterator end_usage_lines() const;

                ///\}

                ///\name Iterate over our environment lines (for documentation)
                ///\{

                typedef libwrapiter::ForwardIterator<ArgsHandler,
                        const std::pair<std::string, std::string> > EnvironmentLineConstIterator;

                EnvironmentLineConstIterator begin_environment_lines() const;

                EnvironmentLineConstIterator end_environment_lines() const;

                ///\}

                ///\name Iterate over our examples (for documentation)
                ///\{

                typedef libwrapiter::ForwardIterator<ArgsHandler,
                        const std::pair<std::string, std::string> > ExamplesConstIterator;

                ExamplesConstIterator begin_examples() const;

                ExamplesConstIterator end_examples() const;

                ///\}

                ///\name Iterate over our groups
                ///\{

                typedef libwrapiter::ForwardIterator<ArgsHandler, ArgsGroup * const> ArgsGroupsConstIterator;

                ArgsGroupsConstIterator begin_args_groups() const;

                ArgsGroupsConstIterator end_args_groups() const;

                ///\}
        };

        /**
         * Output an ArgsHandler to an ostream, for --help output.
         *
         * \ingroup g_args
         */
        std::ostream & operator<< (std::ostream &, const ArgsHandler &) PALUDIS_VISIBLE;
    }
}

#endif
