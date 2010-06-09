/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010 Ciaran McCreesh
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

#include <paludis/util/wrapped_value-impl.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include <string>

using namespace test;
using namespace paludis;

namespace
{
    typedef WrappedValue<struct DormouseTag> Dormouse;

    struct PALUDIS_VISIBLE NotADormouseError
    {
        NotADormouseError(const std::string &)
        {
        }
    };

    typedef WrappedValue<struct CheeseTag> Cheese;

    struct PALUDIS_VISIBLE NoCheeseError
    {
        NoCheeseError(const std::string &)
        {
        }
    };
}

namespace paludis
{
    template <>
    struct WrappedValueTraits<DormouseTag>
    {
        typedef std::string UnderlyingType;
        typedef void ValidationParamsType;
        typedef NotADormouseError ExceptionType;

        static bool validate(const std::string & s)
        {
            return s == "glis glis" || s == "muscardinus avellanarius";
        }
    };

    template <>
    struct WrappedValueTraits<CheeseTag>
    {
        typedef std::string UnderlyingType;
        typedef bool ValidationParamsType;
        typedef NoCheeseError ExceptionType;

        static bool validate(const std::string & s, const bool tasty)
        {
            return s == "stilton" || (s == "camembert" && ! tasty);
        }
    };
};

namespace test_cases
{
    struct DormouseTest : TestCase
    {
        DormouseTest() : TestCase("dormouse tests") { }

        void run()
        {
            Dormouse dormouse("glis glis");
            TEST_CHECK_EQUAL(dormouse.value(), "glis glis");
            dormouse = Dormouse("muscardinus avellanarius");
            TEST_CHECK_EQUAL(dormouse.value(), "muscardinus avellanarius");
            TEST_CHECK_THROWS(dormouse = Dormouse("mesocricetus auratus"), NotADormouseError);
        }
    } test_dormouse;

    struct CheeseTest : TestCase
    {
        CheeseTest() : TestCase("cheese tests") { }

        void run()
        {
            Cheese cheese("stilton", false);
            TEST_CHECK_THROWS(cheese = Cheese("camembert", true), NoCheeseError);
            cheese = Cheese("camembert", false);
            TEST_CHECK_EQUAL(cheese.value(), "camembert");
        }
    } test_cheese;
}

