/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Stephen Bennett <spb@gentoo.org>
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
#include <paludis/util/visitor.hh>

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
         *
         * \ingroup grplibpaludisargs
         */
        class PALUDIS_VISIBLE ArgsDumper :
            public ConstVisitor<ArgsVisitorTypes>
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
        };
    }
}

#endif
