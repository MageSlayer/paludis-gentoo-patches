/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2010 Ciaran McCreesh
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
        typedef Name<struct on_all_name> on_all;
        typedef Name<struct on_annotations_name> on_annotations;
        typedef Name<struct on_any_name> on_any;
        typedef Name<struct on_arrow_name> on_arrow;
        typedef Name<struct on_error_name> on_error;
        typedef Name<struct on_exactly_one_name> on_exactly_one;
        typedef Name<struct on_label_name> on_label;
        typedef Name<struct on_pop_name> on_pop;
        typedef Name<struct on_should_be_empty_name> on_should_be_empty;
        typedef Name<struct on_string_name> on_string;
        typedef Name<struct on_use_name> on_use;
        typedef Name<struct on_use_under_any_name> on_use_under_any;
    }

    struct ELikeDepParserCallbacks
    {
        NamedValue<n::on_all, ELikeDepParserAllFunction> on_all;
        NamedValue<n::on_annotations, ELikeDepParserAnnotationsFunction> on_annotations;
        NamedValue<n::on_any, ELikeDepParserAnyFunction> on_any;
        NamedValue<n::on_arrow, ELikeDepParserArrowFunction> on_arrow;
        NamedValue<n::on_error, ELikeDepParserErrorFunction> on_error;
        NamedValue<n::on_exactly_one, ELikeDepParserExactlyOneFunction> on_exactly_one;
        NamedValue<n::on_label, ELikeDepParserLabelFunction> on_label;
        NamedValue<n::on_pop, ELikeDepParserPopFunction> on_pop;
        NamedValue<n::on_should_be_empty, ELikeDepParserShouldBeEmptyFunction> on_should_be_empty;
        NamedValue<n::on_string, ELikeDepParserStringFunction> on_string;
        NamedValue<n::on_use, ELikeDepParserUseFunction> on_use;
        NamedValue<n::on_use_under_any, ELikeDepParserUseUnderAnyFunction> on_use_under_any;
    };
}

#endif
