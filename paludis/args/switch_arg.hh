/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaranm@gentoo.org>
 * Copyright (c) 2006 Stephen Bennett <spb@gentoo.org>
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

#ifndef PALUDIS_GUARD_ARGS_SWITCH_ARG_HH
#define PALUDIS_GUARD_ARGS_SWITCH_ARG_HH 1

#include <paludis/args/args_option.hh>
#include <string>

/** \file
 * Declaration for SwitchArg.
 *
 * \ingroup Args
 */

namespace paludis
{
    namespace args
    {
        /**
         * A SwitchArg is an option that can either be specified or not
         * specified, and that takes no value (for example, --help).
         *
         * \ingroup Args
         */
        class SwitchArg : public ArgsOption, public Visitable<SwitchArg, ArgsVisitorTypes>
        {
            public:
                /**
                 * Constructor.
                 */
                SwitchArg(ArgsGroup * const group, std::string long_name, char short_name,
                        std::string description);
        };
    }
}

#endif
