/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
 * Copyright (c) 2006 Stephen Bennett
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

#include <paludis/util/visitor.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/pimp.hh>
#include <paludis/args/args_handler.hh>
#include <string>

/** \file
 * Declarations for ArgsVisitor.
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
        class StringArg;
        class AliasArg;
        class SwitchArg;
        class IntegerArg;
        class EnumArg;
        class StringSetArg;
        class StringSequenceArg;

        /**
         * Visitor class. Processes command-line options as they are found.
         *
         * \ingroup g_args
         */
        class PALUDIS_VISIBLE ArgsVisitor
        {
            private:
                Pimp<ArgsVisitor> _imp;

                const std::string & get_param(const ArgsOption &);

                std::string env_name(const std::string & long_name) const;

            public:
                /**
                 * Constructor
                 *
                 * \since 0.49
                 */
                ArgsVisitor(
                        ArgsHandler::ArgsIterator *,
                        ArgsHandler::ArgsIterator,
                        const std::string &,
                        std::string & remaining_chars,
                        bool no);

                ~ArgsVisitor();

                /// Visit a StringArg.
                void visit(StringArg &);

                /// Visit an AliasArg.
                void visit(AliasArg &);

                /// Visit a SwitchArg.
                void visit(SwitchArg &);

                /// Visit an IntegerArg.
                void visit(IntegerArg &);

                /// Visit an EnumArg.
                void visit(EnumArg &);

                /// Visit a StringSetArg.
                void visit(StringSetArg &);

                /// Visit a StringSequenceArg.
                void visit(StringSequenceArg &);
        };
    }
}

#endif
