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

#include <map>
#include <ostream>
#include <paludis/args/args_group.hh>
#include <paludis/util/instantiation_policy.hh>
#include <string>

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
        class ArgsHandler : private InstantiationPolicy<ArgsHandler, instantiation_method::NonCopyableTag>
        {
            friend class ArgsGroup;
            friend std::ostream & operator<< (std::ostream &, const ArgsHandler &);

            private:
                std::list<ArgsGroup *> _groups;
                std::list<std::string> _parameters;
                std::list<std::string> _usage_lines;

                std::map<std::string, ArgsOption *> _longopts;
                std::map<char, ArgsOption *> _shortopts;

            protected:
                /**
                 * Add a new usage line.
                 */
                void add_usage_line(const std::string & l)
                {
                    _usage_lines.push_back(l);
                }

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

                typedef std::list<std::string>::const_iterator ParametersIterator;

                ParametersIterator begin_parameters() const
                {
                    return _parameters.begin();
                }

                ParametersIterator end_parameters() const
                {
                    return _parameters.end();
                }

                bool empty() const
                {
                    return _parameters.empty();
                }

                ///\}

                /**
                 * Add an ArgsOption instance.
                 */
                void add_option(ArgsOption *opt, const std::string long_name, const char short_name = '\0')
                {
                    _longopts[long_name] = opt;
                    if (short_name != '\0')
                        _shortopts[short_name] = opt;
                }

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

                typedef std::list<std::string>::const_iterator UsageLineIterator;

                UsageLineIterator begin_usage_lines() const
                {
                    return _usage_lines.begin();
                }

                UsageLineIterator end_usage_lines() const
                {
                    return _usage_lines.end();
                }

                ///\}

                ///\name Iterate over our groups
                ///\{

                typedef std::list<ArgsGroup *>::const_iterator ArgsGroupsIterator;

                ArgsGroupsIterator begin_args_groups() const
                {
                    return _groups.begin();
                }

                ArgsGroupsIterator end_args_groups() const
                {
                    return _groups.end();
                }

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
