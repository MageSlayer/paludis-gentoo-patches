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

#ifndef PALUDIS_GUARD_ARGS_ARGS_VISITOR_HH
#define PALUDIS_GUARD_ARGS_ARGS_VISITOR_HH 1

#include <list>
#include <paludis/visitor.hh>

/** \file
 * Declaration for ArgsVisitor
 *
 * \ingroup Args
 */

namespace paludis
{
    namespace args
    {
        class ArgsOption;
        class StringArg;
        class AliasArg;
        class SwitchArg;
        class IntegerArg;
        class EnumArg;

        typedef VisitorTypes<ArgsOption *, StringArg *, AliasArg *, SwitchArg *,
                IntegerArg *, EnumArg *> ArgsVisitorTypes;

        /**
         * Visitor class. Processes command-line options as they are found.
         */
        class ArgsVisitor : public ArgsVisitorTypes::Visitor
        {
            private:
                std::list<std::string>::iterator *_args_index;

            public:
                /**
                 * Constructor
                 */
                ArgsVisitor(std::list<std::string>::iterator *);

                /// Visit an ArgsOption.
                void visit(ArgsOption * const);

                /// Visit a StringArg.
                void visit(StringArg * const);

                /// Visit an AliasArg.
                void visit(AliasArg * const);

                /// Visit a SwitchArg.
                void visit(SwitchArg * const);

                /// Visit an IntegerArg.
                void visit(IntegerArg * const);

                /// Visit an EnumArg.
                void visit(EnumArg * const);
        };
    }
}

#endif
