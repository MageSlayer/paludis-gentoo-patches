/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#include <libwrapiter/libwrapiter_forward_iterator.hh>

/** \file
 * Declaration for ArgsHandler.
 *
 * \ingroup grplibpaludisargs
 */

namespace paludis
{
    namespace args
    {
        /**
         * Handles command line arguments.
         *
         * \ingroup grplibpaludisargs
         */
        class ArgsHandler :
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
                void add_enviromnent_variable(const std::string & e, const std::string & desc);

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
                 * Parse command line arguments.
                 */
                void run(const int, const char * const * const);

                ///\name Iterate over our parameters (non - and -- switches and their values)
                ///\{

                typedef libwrapiter::ForwardIterator<ArgsHandler, const std::string> ParametersIterator;

                ParametersIterator begin_parameters() const;

                ParametersIterator end_parameters() const;

                bool empty() const;

                ///\}

                /**
                 * Add an ArgsOption instance.
                 */
                void add_option(ArgsOption * const, const std::string & long_name,
                        const char short_name = '\0');

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

                typedef libwrapiter::ForwardIterator<ArgsHandler, const std::string> UsageLineIterator;

                UsageLineIterator begin_usage_lines() const;

                UsageLineIterator end_usage_lines() const;

                ///\}

                ///\name Iterate over our environment lines (for documentation)
                ///\{

                typedef libwrapiter::ForwardIterator<ArgsHandler,
                        const std::pair<std::string, std::string> > EnvironmentLineIterator;

                EnvironmentLineIterator begin_environment_lines() const;

                EnvironmentLineIterator end_environment_lines() const;

                ///\}

                ///\name Iterate over our groups
                ///\{

                typedef libwrapiter::ForwardIterator<ArgsHandler, ArgsGroup * const> ArgsGroupsIterator;

                ArgsGroupsIterator begin_args_groups() const;

                ArgsGroupsIterator end_args_groups() const;

                ///\}
        };

        /**
         * Output an ArgsHandler to an ostream, for --help output.
         *
         * \ingroup grplibpaludisargs
         */
        std::ostream & operator<< (std::ostream &, const ArgsHandler &);
    }
}

#endif
