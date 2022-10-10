/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2010, 2011 Ciaran McCreesh
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

#include <paludis/elike_dep_parser.hh>

#include <paludis/util/map.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/options.hh>

#include <gtest/gtest.h>

using namespace paludis;

namespace
{
    void handler(std::string & s, const std::string & a1, const std::string & a2, const std::string & a3,
            const std::string & a4, const std::string & a5)
    {
        s.append(a1);
        s.append(a2);
        s.append(a3);
        s.append(a4);
        s.append(a5);
    }

    void do_nothing()
    {
    }

    void handle_annotations(std::string & s, const std::shared_ptr<const Map<std::string, std::string> > & m)
    {
        s.append("[");
        for (Map<std::string, std::string>::ConstIterator i(m->begin()), i_end(m->end()) ;
                i != i_end ; ++i)
            s.append(i->first + ":" + i->second + ";");
        s.append("]");
    }
}

TEST(ELikeDepParser, Basic)
{
    using namespace std::placeholders;

    std::string in("|| ( a b ( c d e ) )");
    std::string out;
    ELikeDepParserCallbacks callbacks(make_named_values<ELikeDepParserCallbacks>(
                n::on_all() = std::bind(&handler, std::ref(out), "all<", "", "", "", ""),
                n::on_annotations() = std::bind(&handle_annotations, std::ref(out), _1),
                n::on_any() = std::bind(&handler, std::ref(out), "any<", "", "", "", ""),
                n::on_arrow() = std::bind(&handler, std::ref(out), "a<", _1, ", ", _2, ">"),
                n::on_at_most_one() = std::bind(&handler, std::ref(out), "m<", "", "", "", ""),
                n::on_error() = std::bind(&handler, std::ref(out), "error<", _1, ">", "", ""),
                n::on_exactly_one() = std::bind(&handler, std::ref(out), "x<", "", "", "", ""),
                n::on_label() = std::bind(&handler, std::ref(out), "label<", _1, "", "", ""),
                n::on_no_annotations() = &do_nothing,
                n::on_pop() = std::bind(&handler, std::ref(out), ">", "", "", "", ""),
                n::on_should_be_empty() = std::bind(&handler, std::ref(out), "EMPTY", "", "", "", ""),
                n::on_string() = std::bind(&handler, std::ref(out), "s<", _1, ">", "", ""),
                n::on_use() = std::bind(&handler, std::ref(out), "use<", _1, ", ", "", ""),
                n::on_use_under_any() = &do_nothing
                ));
    parse_elike_dependencies(in, callbacks, { });
    EXPECT_EQ("any<s<a>s<b>all<s<c>s<d>s<e>>>EMPTY", out);
}

TEST(ELikeDepParser, Empty)
{
    using namespace std::placeholders;

    std::string in("( ( ) )");
    std::string out;
    ELikeDepParserCallbacks callbacks(make_named_values<ELikeDepParserCallbacks>(
                n::on_all() = std::bind(&handler, std::ref(out), "all<", "", "", "", ""),
                n::on_annotations() = std::bind(&handle_annotations, std::ref(out), _1),
                n::on_any() = std::bind(&handler, std::ref(out), "any<", "", "", "", ""),
                n::on_arrow() = std::bind(&handler, std::ref(out), "a<", _1, ", ", _2, ">"),
                n::on_at_most_one() = std::bind(&handler, std::ref(out), "m<", "", "", "", ""),
                n::on_error() = std::bind(&handler, std::ref(out), "error<", _1, ">", "", ""),
                n::on_exactly_one() = std::bind(&handler, std::ref(out), "x<", "", "", "", ""),
                n::on_label() = std::bind(&handler, std::ref(out), "label<", _1, "", "", ""),
                n::on_no_annotations() = &do_nothing,
                n::on_pop() = std::bind(&handler, std::ref(out), ">", "", "", "", ""),
                n::on_should_be_empty() = std::bind(&handler, std::ref(out), "EMPTY", "", "", "", ""),
                n::on_string() = std::bind(&handler, std::ref(out), "s<", _1, ">", "", ""),
                n::on_use() = std::bind(&handler, std::ref(out), "use<", _1, ", ", "", ""),
                n::on_use_under_any() = &do_nothing
                ));
    parse_elike_dependencies(in, callbacks, { });
    EXPECT_EQ("all<all<>>EMPTY", out);
}

TEST(ELikeDepParser, Annotations)
{
    using namespace std::placeholders;

    std::string in("a [[ first = foo second = [ bar baz ] ]]");
    std::string out;
    ELikeDepParserCallbacks callbacks(make_named_values<ELikeDepParserCallbacks>(
                n::on_all() = std::bind(&handler, std::ref(out), "all<", "", "", "", ""),
                n::on_annotations() = std::bind(&handle_annotations, std::ref(out), _1),
                n::on_any() = std::bind(&handler, std::ref(out), "any<", "", "", "", ""),
                n::on_arrow() = std::bind(&handler, std::ref(out), "a<", _1, ", ", _2, ">"),
                n::on_at_most_one() = std::bind(&handler, std::ref(out), "m<", "", "", "", ""),
                n::on_error() = std::bind(&handler, std::ref(out), "error<", _1, ">", "", ""),
                n::on_exactly_one() = std::bind(&handler, std::ref(out), "x<", "", "", "", ""),
                n::on_label() = std::bind(&handler, std::ref(out), "label<", _1, "", "", ""),
                n::on_no_annotations() = &do_nothing,
                n::on_pop() = std::bind(&handler, std::ref(out), ">", "", "", "", ""),
                n::on_should_be_empty() = std::bind(&handler, std::ref(out), "EMPTY", "", "", "", ""),
                n::on_string() = std::bind(&handler, std::ref(out), "s<", _1, ">", "", ""),
                n::on_use() = std::bind(&handler, std::ref(out), "use<", _1, ", ", "", ""),
                n::on_use_under_any() = &do_nothing
                ));
    parse_elike_dependencies(in, callbacks, { });
    EXPECT_EQ("s<a>[first:foo;second:bar baz;]EMPTY", out);
}

TEST(ELikeDepParser, Comments)
{
    using namespace std::placeholders;

    std::string in("# comment\na [[ first = foo second = [ bar baz ] ]] # comment");
    std::string out;
    ELikeDepParserCallbacks callbacks(make_named_values<ELikeDepParserCallbacks>(
                n::on_all() = std::bind(&handler, std::ref(out), "all<", "", "", "", ""),
                n::on_annotations() = std::bind(&handle_annotations, std::ref(out), _1),
                n::on_any() = std::bind(&handler, std::ref(out), "any<", "", "", "", ""),
                n::on_arrow() = std::bind(&handler, std::ref(out), "a<", _1, ", ", _2, ">"),
                n::on_at_most_one() = std::bind(&handler, std::ref(out), "m<", "", "", "", ""),
                n::on_error() = std::bind(&handler, std::ref(out), "error<", _1, ">", "", ""),
                n::on_exactly_one() = std::bind(&handler, std::ref(out), "x<", "", "", "", ""),
                n::on_label() = std::bind(&handler, std::ref(out), "label<", _1, "", "", ""),
                n::on_no_annotations() = &do_nothing,
                n::on_pop() = std::bind(&handler, std::ref(out), ">", "", "", "", ""),
                n::on_should_be_empty() = std::bind(&handler, std::ref(out), "EMPTY", "", "", "", ""),
                n::on_string() = std::bind(&handler, std::ref(out), "s<", _1, ">", "", ""),
                n::on_use() = std::bind(&handler, std::ref(out), "use<", _1, ", ", "", ""),
                n::on_use_under_any() = &do_nothing
                ));
    parse_elike_dependencies(in, callbacks, { edpo_allow_embedded_comments });
    EXPECT_EQ("s<a>[first:foo;second:bar baz;]EMPTY", out);
}

