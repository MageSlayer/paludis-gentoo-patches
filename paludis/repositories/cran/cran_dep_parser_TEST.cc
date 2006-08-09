/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Danny van Dyk <kugelfang@gentoo.org>
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

#include <paludis/dep_atom.hh>
#include <paludis/dep_atom_flattener.hh>
#include <paludis/repositories/cran/cran_dep_parser.hh>
#include <paludis/test_environment.hh>
#include <paludis/util/system.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace test;
using namespace paludis;

#ifndef DOXYGEN
namespace
{
    class DepAtomDumper :
        public DepAtomVisitorTypes::ConstVisitor,
        private InstantiationPolicy<DepAtomDumper, instantiation_method::NonCopyableTag>
    {
        private:
            std::ostream * const _o;

        public:
            DepAtomDumper(std::ostream * const o);

            void visit(const AllDepAtom * const);

            void visit(const AnyDepAtom * const);

            void visit(const UseDepAtom * const);

            void visit(const PackageDepAtom * const);

            void visit(const PlainTextDepAtom * const);

            void visit(const BlockDepAtom * const);
    };

    DepAtomDumper::DepAtomDumper(std::ostream * const o) :
        _o(o)
    {
    }

    void
    DepAtomDumper::visit(const AllDepAtom * const a)
    {
        *_o << "<all>";
        std::for_each(a->begin(), a->end(), accept_visitor(this));
        *_o << "</all>";
    }

    void
    DepAtomDumper::visit(const AnyDepAtom * const a)
    {
        *_o << "<any>";
        std::for_each(a->begin(), a->end(), accept_visitor(this));
        *_o << "</any>";
    }

    void
    DepAtomDumper::visit(const UseDepAtom * const a)
    {
        *_o << "<use flag=\"" << a->flag() << "\" inverse=\""
            << (a->inverse() ? "true" : "false") << "\">";
        std::for_each(a->begin(), a->end(), accept_visitor(this));
        *_o << "</use>";
    }

    void
    DepAtomDumper::visit(const PackageDepAtom * const p)
    {
        *_o << "<package";
        if (p->slot_ptr())
            *_o << " slot=\"" << *p->slot_ptr() << "\"";
        if (p->version_spec_ptr())
            *_o << " version=\"" << p->version_operator() << *p->version_spec_ptr() << "\"";
        *_o << ">" << p->package() << "</package>";
    }

    void
    DepAtomDumper::visit(const PlainTextDepAtom * const t)
    {
        *_o << "<text>" << t->text() << "</text>";
    }

    void
    DepAtomDumper::visit(const BlockDepAtom * const b)
    {
        *_o << "<block>";
        b->blocked_atom()->accept(this);
        *_o << "</block>";
    }
}
#endif

/** \file
 * Test cases for CRANRepository.
 *
 * \ingroup grptestcases
 */

namespace test_cases
{
    /**
     * \test Test CRANDepParser::parse to parse well formed CRAN Depends: strings.
     *
     * \ingroup grptestcases
     */
    struct CRANDepParserTest : TestCase
    {
        CRANDepParserTest() : TestCase("DepParser") { }

        void run()
        {
            std::stringstream s1, s2, s3;
            DepAtomDumper d1(&s1), d2(&s2), d3(&s3);
            // test R dependency
            std::string dep1("R (>= 2.0.0)");
            CRANDepParser::parse(dep1)->accept(&d1);
            TEST_CHECK_EQUAL(s1.str(), "<all><package version=\">=2.0.0\">dev-lang/R</package></all>");
            // test varying whitespaces
            std::string dep2("testpackage1   \t(<1.9)");
            CRANDepParser::parse(dep2)->accept(&d2);
            TEST_CHECK_EQUAL(s2.str(), "<all><package version=\"<1.9\">cran/testpackage1</package></all>");
            // test for package-name and version normalisation
            std::string dep3("R.matlab (>= 2.3-1)");
            CRANDepParser::parse(dep3)->accept(&d3);
            TEST_CHECK_EQUAL(s3.str(), "<all><package version=\">=2.3.1\">cran/R-matlab</package></all>");
        }
    } test_cran_dep_parser;
}
