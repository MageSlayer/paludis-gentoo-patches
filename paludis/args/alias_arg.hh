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

#ifndef PALUDIS_GUARD_PALUDIS_ARGS_ALIAS_ARG_HH
#define PALUDIS_GUARD_PALUDIS_ARGS_ALIAS_ARG_HH 1

#include <paludis/args/args_option.hh>
#include <string>

/** \file
 * Declaration for AliasArg.
 *
 * \ingroup Args
 */

namespace paludis
{
    namespace args
    {
        /**
         * An AliasArg is an alias for another argument.
         *
         * \ingroup Args
         */
        class AliasArg : public ArgsOption, public Visitable<AliasArg, ArgsVisitorTypes>
        {
            private:
                ArgsOption * const _other;

            public:
                /**
                 * Constructor.
                 */
                AliasArg(ArgsOption * const other, const std::string & new_long_name);

                virtual bool specified() const
                {
                    return _other->specified();
                }

                virtual void set_specified(const bool value)
                {
                    _other->set_specified(value);
                }
        };
    }
}

#endif
