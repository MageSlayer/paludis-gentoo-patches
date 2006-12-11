/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <paludis/repositories/gems/yaml.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace test;
using namespace paludis;

namespace
{
    struct Dumper :
        YamlNodeVisitorTypes::ConstVisitor
    {
        std::string str;

        void visit(const YamlSequenceNode * n)
        {
            str.append("seq<");
            std::for_each(n->begin(), n->end(), accept_visitor(this));
            str.append(">");
        }

        void visit(const YamlScalarNode * n)
        {
            str.append("scalar<");
            str.append(n->value());
            str.append(">");
        }

        void visit(const YamlMappingNode * n)
        {
            str.append("map<");
            bool need_comma(false);
            for (YamlMappingNode::Iterator i(n->begin()), i_end(n->end()) ; i != i_end ; ++i)
            {
                if (need_comma)
                    str.append(",");

                i->first->accept(this);
                str.append("=");
                i->second->accept(this);
                need_comma = true;
            }
            str.append(">");
        }
    };
}

namespace test_cases
{
    struct YamlTest : TestCase
    {
        YamlTest() : TestCase("yaml") { }

        void run()
        {
            YamlDocument yaml("foo");
            Dumper d;
            yaml.top()->accept(&d);
            TEST_CHECK_EQUAL(d.str, "seq<scalar<foo>>");
        }
    } test_yaml;

    struct YamlSequenceTest : TestCase
    {
        YamlSequenceTest() : TestCase("yaml sequence") { }

        void run()
        {
            YamlDocument yaml(
                    "- a\n"
                    "- b\n"
                    "- c\n");
            Dumper d;
            yaml.top()->accept(&d);
            TEST_CHECK_EQUAL(d.str, "seq<seq<scalar<a>scalar<b>scalar<c>>>");
        }
    } test_yaml_sequence;

    struct YamlMappingTest : TestCase
    {
        YamlMappingTest() : TestCase("yaml mapping") { }

        void run()
        {
            YamlDocument yaml(
                    "a: b\n"
                    "c: d\n");
            Dumper d;
            yaml.top()->accept(&d);
            TEST_CHECK_EQUAL(d.str, "seq<map<scalar<a>=scalar<b>,scalar<c>=scalar<d>>>");
        }
    } test_yaml_map;

    struct YamlMixedTest : TestCase
    {
        YamlMixedTest() : TestCase("yaml mixed") { }

        void run()
        {
            YamlDocument yaml(
"a:\n"
"  b: !c\n"
"    d: e\n"
"    f:\n"
"    - g\n"
"    - h\n"
"i: j\n");
            Dumper d;
            yaml.top()->accept(&d);
            TEST_CHECK_EQUAL(d.str, "seq<map<scalar<a>=map<scalar<b>=map<scalar<d>=scalar<e>,"
                    "scalar<f>=seq<scalar<g>scalar<h>>>>,scalar<i>=scalar<j>>>");
        }
    } test_yaml_mixed;

    struct YamlBadTest : TestCase
    {
        YamlBadTest() : TestCase("yaml bad") { }

        void run()
        {
            TEST_CHECK_THROWS(YamlDocument yaml("[ [ foo:\n"), YamlError);
        }
    } test_yaml_bad;

}

