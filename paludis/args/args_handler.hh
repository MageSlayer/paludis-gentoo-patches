/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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
#include <paludis/instantiation_policy.hh>
#include <string>
#include <ostream>
#include <map>

/** \file
 * Declaration for ArgsHandler.
 *
 * \ingroup Args
 */

namespace paludis
{
    namespace args
    {
        /**
         * Handles command line arguments.
         *
         * \ingroup Args
         */
        class ArgsHandler : private InstantiationPolicy<ArgsHandler, instantiation_method::NonCopyableTag>
        {
            friend class ArgsGroup;
            friend std::ostream & operator<< (std::ostream &, const ArgsHandler &);

            private:
                std::list<ArgsGroup *> _groups;
                std::list<std::string> _parameters;

                std::map<std::string, ArgsOption *> _longopts;
                std::map<char, ArgsOption *> _shortopts;

            protected:
                /**
                 * Add an new ArgsGroup (called by the ArgsGroup constructor).
                 */
                void add(ArgsGroup * const);

                /**
                 * Dump, for --help output (called by operator<<).
                 */
                void dump_to_stream(std::ostream & s) const;

            public:
                /**
                 * Constructor.
                 */
                ArgsHandler();

                /**
                 * Parse command line arguments.
                 */
                void run(const int, const char * const * const);

                /**
                 * Iterate over our parameters (non - and -- switches and their
                 * values).
                 */
                typedef std::list<std::string>::const_iterator ParametersIterator;

                /**
                 * Pointer to the start of our parameters.
                 */
                ParametersIterator begin_parameters() const
                {
                    return _parameters.begin();
                }

                /**
                 * Pointer to past the end of our parameters.
                 */
                ParametersIterator end_parameters() const
                {
                    return _parameters.end();
                }

                /**
                 * Do we have no parameters?
                 */
                bool empty() const
                {
                    return _parameters.empty();
                }

                /**
                 * Add an ArgsOption instance.
                 */
                void add_option(ArgsOption *opt, const std::string long_name, const char short_name = '\0')
                {
                    _longopts[long_name] = opt;
                    if (short_name != '\0')
                        _shortopts[short_name] = opt;
                }

        };

        /**
         * Output an ArgsHandler to an ostream, for --help output.
         *
         * \ingroup Args
         */
        std::ostream & operator<< (std::ostream &, const ArgsHandler &);
    }
}

#endif
