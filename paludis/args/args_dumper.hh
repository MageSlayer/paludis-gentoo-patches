/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Stephen Bennett <spb@gentoo.org>
 *
 * This file is part of the Paludis package manager. Paludis is free software;
 * you can redistribute it and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
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

#include <paludis/visitor.hh>
#include <ostream>

#include "args_visitor.hh"

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
         * Visitor class. Prints help text appropriate to each command line option.
         */
        class ArgsDumper : public ArgsVisitorTypes::ConstVisitor
        {
            private:
                std::ostream & _os;

            public:
                /**
                 * Constructor.
                 */
                ArgsDumper(std::ostream & os);

                /// Visit an ArgsOption.
                void visit(const ArgsOption * const);

                /// Visit a SwitchArg.
                void visit(const SwitchArg * const);

                /// Visit a StringArg.
                void visit(const StringArg * const);

                /// Visit an IntegerArg.
                void visit(const IntegerArg * const);

                /// Visit an AliasArg.
                void visit(const AliasArg * const);

                /// Visit an EnumArg.
                void visit(const EnumArg * const);

                /// Visit a StringSetArg.
                void visit(const StringSetArg * const);
        };
    }
}

#endif
