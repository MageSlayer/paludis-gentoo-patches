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

#include <paludis/qa/metadata_file.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace paludis;
using namespace paludis::qa;
using namespace test;

namespace test_cases
{
    struct MetadataFileTest : TestCase
    {
        MetadataFileTest() : TestCase("metadata file") { }

        void run()
        {
            MetadataFile f(
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
"<!DOCTYPE pkgmetadata SYSTEM \"http://www.gentoo.org/dtd/metadata.dtd\">\n"
"<pkgmetadata>\n"
"       <herd>vim</herd>\n"
"       <herd>    cookie  </herd>\n"
"       <maintainer>\n"
"               <email>  foo@bar.baz  </email>\n"
"               <name>  Foo  Bar  </name>\n"
"       </maintainer>\n"
"       <maintainer>\n"
"               <email>oink@oink</email>\n"
"       </maintainer>\n"
"       <maintainer>\n"
"               <name> Fred the Fish    </name>\n"
"       </maintainer>\n"
"       <longdescription lang=\"en\">\n"
"           Some text\n"
"       </longdescription>\n"
"</pkgmetadata>\n"
                    );

            TEST_CHECK(f.end_herds() != std::find(f.begin_herds(), f.end_herds(), "vim"));
            TEST_CHECK(f.end_herds() != std::find(f.begin_herds(), f.end_herds(), "cookie"));
            TEST_CHECK(f.end_herds() == std::find(f.begin_herds(), f.end_herds(), "monster"));

            TEST_CHECK(f.end_maintainers() != std::find(f.begin_maintainers(),
                        f.end_maintainers(), std::make_pair(std::string("foo@bar.baz"), std::string("Foo Bar"))));
            TEST_CHECK(f.end_maintainers() != std::find(f.begin_maintainers(),
                        f.end_maintainers(), std::make_pair(std::string("oink@oink"), std::string(""))));
            TEST_CHECK(f.end_maintainers() != std::find(f.begin_maintainers(),
                        f.end_maintainers(), std::make_pair(std::string(""), std::string("Fred the Fish"))));
        }
    } test_has_ebuilds_check;
}


