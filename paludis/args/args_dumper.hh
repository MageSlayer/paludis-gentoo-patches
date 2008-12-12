/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Stephen Bennett
 *
 * This file is part of the Paludis package manager. Paludis is free software;
 * you can redistribute it and/or modify it under the terms of the GNU General
 * Public License, version 2, as published by the Free Software Foundation.
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

#ifndef PALUDIS_GUARD_PALUDIS_ARGS_ARGS_DUMPER_HH
#define PALUDIS_GUARD_PALUDIS_ARGS_ARGS_DUMPER_HH 1

#include <iosfwd>
#include <paludis/args/args_visitor.hh>

/** \file
 * Declarations for the ArgsDumper class.
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
        class ArgsOption;
        class SwitchArg;
        class StringArg;
        class IntegerArg;
        class AliasArg;
        class EnumArg;

        /**
         * Prints help text appropriate to each command line option.
         *
         * \ingroup g_args
         */
        class PALUDIS_VISIBLE ArgsDumper
        {
            private:
                std::ostream & _os;

                void generic_visit(const ArgsOption &);

            public:
                /**
                 * Constructor.
                 */
                ArgsDumper(std::ostream & os);

                /// Visit a SwitchArg.
                void visit(const SwitchArg &);

                /// Visit a StringArg.
                void visit(const StringArg &);

                /// Visit an IntegerArg.
                void visit(const IntegerArg &);

                /// Visit an AliasArg.
                void visit(const AliasArg &);

                /// Visit an EnumArg.
                void visit(const EnumArg &);

                /// Visit a StringSetArg.
                void visit(const StringSetArg &);

                /// Visit a StringSequenceArg.
                void visit(const StringSequenceArg &);
        };
    }
}

#endif
