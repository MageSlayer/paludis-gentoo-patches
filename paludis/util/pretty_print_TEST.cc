/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008 Victor Meyerson
 * Copyright (c) 2011 Ciaran McCreesh
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

#include <paludis/util/pretty_print.hh>

#include <clocale>
#include <cstdlib>
#include <string>

#include <gtest/gtest.h>

using namespace paludis;

TEST(PrettyPrintBytes, Works)
{
    EXPECT_EQ(    "0 Bytes", pretty_print_bytes(0));
    EXPECT_EQ(    "1 Bytes", pretty_print_bytes(1));
    EXPECT_EQ(  "100 Bytes", pretty_print_bytes(100));
    EXPECT_EQ("1.00 kBytes", pretty_print_bytes(1024));
    EXPECT_EQ("1.50 kBytes", pretty_print_bytes(1536));
    EXPECT_EQ("1.00 MBytes", pretty_print_bytes(1048576));
    EXPECT_EQ("1.86 MBytes", pretty_print_bytes(1953497));
    EXPECT_EQ("1.00 GBytes", pretty_print_bytes(1073741824));
    EXPECT_EQ("1.43 GBytes", pretty_print_bytes(1537598292));
}

TEST(PrettyPrintTime, Works)
{
    std::setlocale(LC_TIME, "C");
    setenv("TZ", "America/New_York", 1);
    EXPECT_EQ("Fri Feb 13 18:31:30 EST 2009", pretty_print_time(1234567890));
    EXPECT_EQ("Thu Oct 15 10:43:00 EDT 2009", pretty_print_time(1255617780));
}

