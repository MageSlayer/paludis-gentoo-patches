/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <test/test_runner.hh>
#include <test/test_framework.hh>
#include <paludis/repositories/gems/yaml.hh>
#include <paludis/util/iterator.hh>
#include <paludis/util/visitor-impl.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>
#include <algorithm>
#include <sstream>

using namespace test;
using namespace paludis;
using namespace paludis::yaml;

namespace
{
    struct CountedStringNode :
        StringNode
    {
        static unsigned alloc_count;

        void * operator new (const std::size_t sz) throw (std::bad_alloc)
        {
            ++alloc_count;
            return ::operator new (sz);
        }

        void operator delete (void * n) throw ()
        {
            --alloc_count;
            ::operator delete (n);
        }

        CountedStringNode() :
            StringNode("x")
        {
        }
    };

    unsigned CountedStringNode::alloc_count(0);

    struct FakeDocument
    {
        FakeDocument()
        {
            NodeManager::get_instance()->register_document(this);
        }

        ~FakeDocument()
        {
            NodeManager::get_instance()->deregister_document(this);
        }
    };

    struct Dumper :
        ConstVisitor<NodeVisitorTypes>
    {
        std::stringstream s;

        void visit(const StringNode & n)
        {
            s << "str(" << n.text() << ")";
        }

        void visit(const MapNode & n)
        {
            s << "map(";
            bool w(false);
            for (MapNode::Iterator i(n.begin()), i_end(n.end()) ; i != i_end ; ++i)
            {
                if (w)
                    s << ", ";

                i->first->accept(*this);
                s << " -> ";
                i->second->accept(*this);
                w = true;
            }
            s << ")";
        }

        void visit(const SequenceNode & n)
        {
            s << "seq(";
            bool w(false);
            for (SequenceNode::Iterator i(n.begin()), i_end(n.end()) ; i != i_end ; ++i)
            {
                if (w)
                    s << ", ";
                (*i)->accept(*this);
                w = true;
            }
            s << ")";
        }
    };
}

namespace test_cases
{
    struct ManagementTest : TestCase
    {
        ManagementTest() : TestCase("management") { }

        void run()
        {
            TEST_CHECK_EQUAL(CountedStringNode::alloc_count, 0u);
            {
                FakeDocument d;
                NodeManager::get_instance()->manage_node(&d, new CountedStringNode);
                NodeManager::get_instance()->manage_node(&d, new CountedStringNode);
                NodeManager::get_instance()->manage_node(&d, new CountedStringNode);
                TEST_CHECK_EQUAL(CountedStringNode::alloc_count, 3u);
            }
            TEST_CHECK_EQUAL(CountedStringNode::alloc_count, 0u);
        }
    } test_management;

    struct ParseTest : TestCase
    {
        ParseTest() : TestCase("parse") { }

        void run()
        {
            Document doc("foo: [ bar, baz ]");
            TEST_CHECK(doc.top());

            Dumper dumper;
            doc.top()->accept(dumper);
            TEST_CHECK_EQUAL(dumper.s.str(), "map(str(foo) -> seq(str(bar), str(baz)))");
        }
    } test_parse;

    struct ParseErrorTest : TestCase
    {
        ParseErrorTest() : TestCase("parse error") { }

        void run()
        {
            TEST_CHECK_THROWS(Document("foo: [ bar, baz"), ParseError);
        }
    } test_parse_error;

    struct MapFindTest : TestCase
    {
        MapFindTest() : TestCase("map find") { }

        void run()
        {
            Document doc("{ foo: bar, bar: baz, monkey: pants }");
            TEST_CHECK(doc.top());

            Dumper dumper;
            doc.top()->accept(dumper);
            TEST_CHECK_EQUAL(dumper.s.str(), "map(str(foo) -> str(bar), str(bar) -> str(baz), str(monkey) -> str(pants))");

            const MapNode * m(static_cast<const MapNode *>(doc.top()));
            TEST_CHECK(m->find("foo") != m->end());
            TEST_CHECK(m->find("bar") != m->end());
            TEST_CHECK(m->find("monkey") != m->end());
            TEST_CHECK(m->find("baz") == m->end());

            TEST_CHECK_EQUAL(static_cast<const StringNode *>(m->find("foo")->second)->text(), "bar");
            TEST_CHECK_EQUAL(static_cast<const StringNode *>(m->find("bar")->second)->text(), "baz");
            TEST_CHECK_EQUAL(static_cast<const StringNode *>(m->find("monkey")->second)->text(), "pants");
        }
    } test_map_find;
}

