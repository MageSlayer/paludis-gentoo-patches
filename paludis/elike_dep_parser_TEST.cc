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

#include <paludis/elike_dep_parser.hh>
#include <paludis/util/map.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/make_named_values.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace paludis;
using namespace test;

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

    void handle_annotations(std::string & s, const std::tr1::shared_ptr<const Map<std::string, std::string> > & m)
    {
        s.append("[");
        for (Map<std::string, std::string>::ConstIterator i(m->begin()), i_end(m->end()) ;
                i != i_end ; ++i)
            s.append(i->first + ":" + i->second + ";");
        s.append("]");
    }
}

namespace test_cases
{
    struct ELikeDepParserBasicTest : TestCase
    {
        ELikeDepParserBasicTest() : TestCase("basic") { }

        void run()
        {
            using namespace std::tr1::placeholders;

            std::string in("|| ( a b ( c d e ) )"), out;
            ELikeDepParserCallbacks callbacks(make_named_values<ELikeDepParserCallbacks>(
                        n::on_all() = std::tr1::bind(&handler, std::tr1::ref(out), "all<", "", "", "", ""),
                        n::on_annotations() = std::tr1::bind(&handle_annotations, std::tr1::ref(out), _1),
                        n::on_any() = std::tr1::bind(&handler, std::tr1::ref(out), "any<", "", "", "", ""),
                        n::on_arrow() = std::tr1::bind(&handler, std::tr1::ref(out), "a<", _1, ", ", _2, ">"),
                        n::on_error() = std::tr1::bind(&handler, std::tr1::ref(out), "error<", _1, ">", "", ""),
                        n::on_label() = std::tr1::bind(&handler, std::tr1::ref(out), "label<", _1, "", "", ""),
                        n::on_pop() = std::tr1::bind(&handler, std::tr1::ref(out), ">", "", "", "", ""),
                        n::on_should_be_empty() = std::tr1::bind(&handler, std::tr1::ref(out), "EMPTY", "", "", "", ""),
                        n::on_string() = std::tr1::bind(&handler, std::tr1::ref(out), "s<", _1, ">", "", ""),
                        n::on_use() = std::tr1::bind(&handler, std::tr1::ref(out), "use<", _1, ", ", "", ""),
                        n::on_use_under_any() = &do_nothing
                        ));
            parse_elike_dependencies(in, callbacks);
            TEST_CHECK_EQUAL(out, "any<s<a>s<b>all<s<c>s<d>s<e>>>EMPTY");
        }
    } elike_dep_parser_basic_test;

    struct ELikeDepParserEmptyBlocksTest : TestCase
    {
        ELikeDepParserEmptyBlocksTest() : TestCase("empty blocks") { }

        void run()
        {
            using namespace std::tr1::placeholders;

            std::string in("( ( ) )"), out;
            ELikeDepParserCallbacks callbacks(make_named_values<ELikeDepParserCallbacks>(
                        n::on_all() = std::tr1::bind(&handler, std::tr1::ref(out), "all<", "", "", "", ""),
                        n::on_annotations() = std::tr1::bind(&handle_annotations, std::tr1::ref(out), _1),
                        n::on_any() = std::tr1::bind(&handler, std::tr1::ref(out), "any<", "", "", "", ""),
                        n::on_arrow() = std::tr1::bind(&handler, std::tr1::ref(out), "a<", _1, ", ", _2, ">"),
                        n::on_error() = std::tr1::bind(&handler, std::tr1::ref(out), "error<", _1, ">", "", ""),
                        n::on_label() = std::tr1::bind(&handler, std::tr1::ref(out), "label<", _1, "", "", ""),
                        n::on_pop() = std::tr1::bind(&handler, std::tr1::ref(out), ">", "", "", "", ""),
                        n::on_should_be_empty() = std::tr1::bind(&handler, std::tr1::ref(out), "EMPTY", "", "", "", ""),
                        n::on_string() = std::tr1::bind(&handler, std::tr1::ref(out), "s<", _1, ">", "", ""),
                        n::on_use() = std::tr1::bind(&handler, std::tr1::ref(out), "use<", _1, ", ", "", ""),
                        n::on_use_under_any() = &do_nothing
                    ));
            parse_elike_dependencies(in, callbacks);
            TEST_CHECK_EQUAL(out, "all<all<>>EMPTY");
        }
    } elike_dep_parser_test_empty_blocks;

    struct ELikeDepParserAnnotationsTest : TestCase
    {
        ELikeDepParserAnnotationsTest() : TestCase("annotations") { }

        void run()
        {
            using namespace std::tr1::placeholders;

            std::string in("a [[ first = foo second = [ bar baz ] ]]"), out;
            ELikeDepParserCallbacks callbacks(make_named_values<ELikeDepParserCallbacks>(
                        n::on_all() = std::tr1::bind(&handler, std::tr1::ref(out), "all<", "", "", "", ""),
                        n::on_annotations() = std::tr1::bind(&handle_annotations, std::tr1::ref(out), _1),
                        n::on_any() = std::tr1::bind(&handler, std::tr1::ref(out), "any<", "", "", "", ""),
                        n::on_arrow() = std::tr1::bind(&handler, std::tr1::ref(out), "a<", _1, ", ", _2, ">"),
                        n::on_error() = std::tr1::bind(&handler, std::tr1::ref(out), "error<", _1, ">", "", ""),
                        n::on_label() = std::tr1::bind(&handler, std::tr1::ref(out), "label<", _1, "", "", ""),
                        n::on_pop() = std::tr1::bind(&handler, std::tr1::ref(out), ">", "", "", "", ""),
                        n::on_should_be_empty() = std::tr1::bind(&handler, std::tr1::ref(out), "EMPTY", "", "", "", ""),
                        n::on_string() = std::tr1::bind(&handler, std::tr1::ref(out), "s<", _1, ">", "", ""),
                        n::on_use() = std::tr1::bind(&handler, std::tr1::ref(out), "use<", _1, ", ", "", ""),
                        n::on_use_under_any() = &do_nothing
                    ));
            parse_elike_dependencies(in, callbacks);
            TEST_CHECK_EQUAL(out, "s<a>[first:foo;second:bar baz;]EMPTY");
        }
    } elike_dep_parser_annotations_test;
}

