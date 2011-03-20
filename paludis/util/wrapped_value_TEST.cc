/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010, 2011 Ciaran McCreesh
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

#include <string>

#include <gtest/gtest.h>

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
}

TEST(Dormouse, Works)
{
    Dormouse dormouse("glis glis");
    ASSERT_EQ("glis glis", dormouse.value());

    dormouse = Dormouse("muscardinus avellanarius");
    ASSERT_EQ( "muscardinus avellanarius", dormouse.value());
}

TEST(Dormouse, Throws)
{
    Dormouse dormouse("glis glis");
    ASSERT_THROW(dormouse = Dormouse("mesocricetus auratus"), NotADormouseError);
}

TEST(Cheese, Works)
{
    Cheese cheese("stilton", false);
    ASSERT_THROW(cheese = Cheese("camembert", true), NoCheeseError);

    cheese = Cheese("camembert", false);
    ASSERT_EQ("camembert", cheese.value());
}

