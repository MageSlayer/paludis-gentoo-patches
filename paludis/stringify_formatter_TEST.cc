/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010 Ciaran McCreesh
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

#include <paludis/stringify_formatter.hh>
#include <paludis/stringify_formatter-impl.hh>
#include <paludis/util/options.hh>
#include <paludis/dep_spec.hh>
#include <paludis/spec_tree.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/environments/test/test_environment.hh>
#include <paludis/choice.hh>
#include <test/test_runner.hh>
#include <test/test_framework.hh>
#include <paludis/util/clone-impl.hh>

using namespace test;
using namespace paludis;

namespace
{
    std::string format_two(const KeywordName & k, const LicenseDepSpec & n,
            const Formatter<KeywordName, LicenseDepSpec> & f)
    {
        return f.format(k, format::Accepted()) + " " + f.format(n, format::Accepted());
    }

    std::string format_three(const PackageDepSpec & k, const BlockDepSpec & d, const NamedSetDepSpec & u,
            const GenericSpecTree::ItemFormatter & f)
    {
        return f.format(k, format::Plain()) + " " + f.format(d, format::Plain()) + " " + f.format(u, format::Plain());
    }

    class PartialFormatter :
        public CanFormat<KeywordName>,
        public CanFormat<PackageDepSpec>
    {
        private:
            PartialFormatter(const PartialFormatter &);

        public:
            PartialFormatter()
            {
            }

            virtual std::string format(const KeywordName & s, const format::Accepted &) const
            {
                return "<" + stringify(s) + ">";
            }

            virtual std::string format(const KeywordName & s, const format::Unaccepted &) const
            {
                return "<" + stringify(s) + ">";
            }

            virtual std::string format(const KeywordName & s, const format::Plain &) const
            {
                return "<" + stringify(s) + ">";
            }

            virtual std::string format(const PackageDepSpec & s, const format::Plain &) const
            {
                return "<" + stringify(s) + ">";
            }

            virtual std::string format(const PackageDepSpec & s, const format::Installed &) const
            {
                return "<" + stringify(s) + ">";
            }

            virtual std::string format(const PackageDepSpec & s, const format::Installable &) const
            {
                return "<" + stringify(s) + ">";
            }
    };
}

namespace test_cases
{
    struct StringifyFormatterTest : TestCase
    {
        StringifyFormatterTest() : TestCase("stringify formatter") { }

        void run()
        {
            StringifyFormatter ff;
            LicenseDepSpec l("two");
            std::string s(format_two(KeywordName("one"), l, ff));
            TEST_CHECK_EQUAL(s, "one two");
        }
    } test_stringify_formatter;

    struct StringifyFormatterPartialTest : TestCase
    {
        StringifyFormatterPartialTest() : TestCase("stringify formatter partial") { }

        void run()
        {
            TestEnvironment env;

            PartialFormatter f;
            StringifyFormatter ff(f);
            BlockDepSpec b("!!!!!cat/pkg", parse_user_package_dep_spec("cat/pkg",
                        &env, UserPackageDepSpecOptions()), false);
            NamedSetDepSpec u(SetName("foo"));
            std::string s(format_three(
                        parse_user_package_dep_spec("cat/pkg", &env, UserPackageDepSpecOptions()),
                        b, u,
                        ff));
            TEST_CHECK_EQUAL(s, "<cat/pkg> !!!!!cat/pkg foo");
        }
    } test_stringify_formatter_partial;
}


