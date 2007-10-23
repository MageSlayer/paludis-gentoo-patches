/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#include <paludis/util/visitor.hh>
#include <paludis/util/attributes.hh>
#include <string>

#include <libwrapiter/libwrapiter_forward_iterator.hh>

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

        /**
         * Visitor types for visitors that can visit Args.
         *
         * \ingroup g_args
         */
        struct ArgsVisitorTypes :
            VisitorTypes<
                ArgsVisitorTypes,
                ArgsOption,
                StringArg,
                AliasArg,
                SwitchArg,
                IntegerArg,
                EnumArg,
                StringSetArg>
        {
        };

        /**
         * Visitor class. Processes command-line options as they are found.
         *
         * \ingroup g_args
         */
        class PALUDIS_VISIBLE ArgsVisitor :
            public Visitor<ArgsVisitorTypes>
        {
            private:
                libwrapiter::ForwardIterator<ArgsVisitor, std::string> * _args_index, _args_end;
                std::string _env_prefix;

                const std::string & get_param(const ArgsOption &);

                std::string env_name(const std::string & long_name) const;

            public:
                /**
                 * Constructor
                 */
                ArgsVisitor(libwrapiter::ForwardIterator<ArgsVisitor, std::string> *,
                        libwrapiter::ForwardIterator<ArgsVisitor, std::string>,
                        const std::string & env_prefix = "");

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
        };
    }
}

#endif
