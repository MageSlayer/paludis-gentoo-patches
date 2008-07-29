/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_ELIKE_DEP_PARSER_HH
#define PALUDIS_GUARD_PALUDIS_ELIKE_DEP_PARSER_HH 1

#include <paludis/elike_dep_parser-fwd.hh>
#include <paludis/util/named_value.hh>

namespace paludis
{
    namespace n
    {
        struct on_all;
        struct on_annotations;
        struct on_any;
        struct on_arrow;
        struct on_error;
        struct on_label;
        struct on_pop;
        struct on_should_be_empty;
        struct on_string;
        struct on_use;
        struct on_use_under_any;
    }

    struct ELikeDepParserCallbacks
    {
        NamedValue<n::on_all, ELikeDepParserAllFunction> on_all;
        NamedValue<n::on_annotations, ELikeDepParserAnnotationsFunction> on_annotations;
        NamedValue<n::on_any, ELikeDepParserAnyFunction> on_any;
        NamedValue<n::on_arrow, ELikeDepParserArrowFunction> on_arrow;
        NamedValue<n::on_error, ELikeDepParserErrorFunction> on_error;
        NamedValue<n::on_label, ELikeDepParserLabelFunction> on_label;
        NamedValue<n::on_pop, ELikeDepParserPopFunction> on_pop;
        NamedValue<n::on_should_be_empty, ELikeDepParserShouldBeEmptyFunction> on_should_be_empty;
        NamedValue<n::on_string, ELikeDepParserStringFunction> on_string;
        NamedValue<n::on_use, ELikeDepParserUseFunction> on_use;
        NamedValue<n::on_use_under_any, ELikeDepParserUseUnderAnyFunction> on_use_under_any;
    };
}

#endif
