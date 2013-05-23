/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010, 2011, 2013 Ciaran McCreesh
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

#include <paludis/repositories/fake/dep_parser.hh>

#include <paludis/environments/test/test_environment.hh>

#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/stringify.hh>

#include <sstream>
#include <algorithm>

#include <gtest/gtest.h>

using namespace paludis;

TEST(DepParser, Works)
{
    TestEnvironment env;
    std::shared_ptr<DependencySpecTree> d(fakerepository::parse_depend("( ( a/a b/b ) )", &env));

    std::stringstream str;
    d->top()->make_accept(
            [&] (const DependencySpecTree::NodeType<PackageDepSpec>::Type & node) {
                str << "p<" << stringify(*node.spec()) << ">";
            },

            [&] (const DependencySpecTree::NodeType<NamedSetDepSpec>::Type & node) {
                str << "s<" << stringify(*node.spec()) << ">";
            },

            [&] (const DependencySpecTree::NodeType<BlockDepSpec>::Type & node) {
                str << "b<" << stringify(*node.spec()) << ">";
            },

            [&] (const DependencySpecTree::NodeType<DependenciesLabelsDepSpec>::Type & node) {
                str << "l<" << stringify(*node.spec()) << ">";
            },

            [&] (const DependencySpecTree::NodeType<AllDepSpec>::Type & node, const Revisit<void, DependencySpecTree::BasicNode> & revisit) {
                str << "all<";
                std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), revisit);
                str << ">";
            },

            [&] (const DependencySpecTree::NodeType<AnyDepSpec>::Type & node, const Revisit<void, DependencySpecTree::BasicNode> & revisit) {
                str << "any<";
                std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), revisit);
                str << ">";
            },

            [&] (const DependencySpecTree::NodeType<ConditionalDepSpec>::Type & node, const Revisit<void, DependencySpecTree::BasicNode> & revisit) {
                str << "cond<" << *node.spec() << ",";
                std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), revisit);
                str << ">";
            }
            );

    EXPECT_EQ("all<all<all<p<a/a>p<b/b>>>>", str.str());
}

