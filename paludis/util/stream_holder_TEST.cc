/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2013 Wouter van Kesteren <woutershep@gmail.com>
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

#include <paludis/util/stream_holder.hh>

#include <sstream>
#include <iostream>
#include <complex>

#include <gtest/gtest.h>

using namespace paludis;

TEST(StreamHolder, Construct)
{
    StreamHolder<std::stringstream> s1;
    StreamHolder<std::ostream> s2(std::cout.rdbuf());
    StreamHolder<std::ostringstream> s3(std::string("Goat"), std::ios_base::ate);
}

TEST(StreamHolder, Convert)
{
    StreamHolder<std::stringstream> s("Dog");
    ASSERT_FALSE(!s);
    ASSERT_TRUE(static_cast<bool>(s));

    std::stringstream& r(s);
    std::stringstream* p(s);
    ASSERT_EQ(&r, p);

    ASSERT_STREQ("Dog", p->str().c_str());
}

TEST(StreamHolder, Formatted)
{
    StreamHolder<std::ostringstream> s;
    std::ostringstream* p(s);
    s << "Hello";
    ASSERT_STREQ("Hello", p->str().c_str());
    s << 1337;
    ASSERT_STREQ("Hello1337", p->str().c_str());
    s << "\n" << "World!";
    ASSERT_STREQ("Hello1337\nWorld!", p->str().c_str());
    s << "a" << 2.2f << 'c' << 4.4 << "e";
    ASSERT_STREQ("Hello1337\nWorld!a2.2c4.4e", p->str().c_str());
}

// there is currently no non-member basic_ios manipulator in the standard
// so we make our own here that does nothing but increment 'called'
struct ios_manip {
    static int called;
    static std::ios& func(std::ios& rhs) {
        called++;
        return rhs;
    }
};

int ios_manip::called(0);

TEST(StreamHolder, Manip)
{
    StreamHolder<std::ostringstream> s;
    std::ostringstream* p(s);

    // basic_ostream<charT,traits>& operator<<
    // (basic_ostream<charT,traits>& (*pf)(basic_ostream<charT,traits>&))
    s << "J";
    s << std::hex;
    s << 255 << std::dec << 255;
    ASSERT_STREQ("Jff255", p->str().c_str());

    // basic_ostream<charT,traits>& operator<<
    // (basic_ios<charT,traits>& (*pf)(basic_ios<charT,traits>&))
    ASSERT_EQ(0, ios_manip::called);
    s << ios_manip::func;
    ASSERT_EQ(1, ios_manip::called);
    s << " " << ios_manip::func << " ";
    ASSERT_EQ(2, ios_manip::called);
    ASSERT_STREQ("Jff255  ", p->str().c_str());

    // basic_ostream<charT,traits>& operator<<
    // (ios_base& (*pf)(ios_base&))
    s << std::endl;
    s << "K" << std::endl << "L";
    ASSERT_STREQ("Jff255  \nK\nL", p->str().c_str());

}

TEST(StreamHolder, NonMember)
{
    StreamHolder<std::ostringstream> s("Complex: ", std::ios_base::ate);
    std::ostringstream* p(s);

    std::complex<float> c1(1.2f, 3.4f);
    s << c1 << std::endl;
    ASSERT_STREQ("Complex: (1.2,3.4)\n", p->str().c_str());

    std::complex<double> c2(5.6, 7.8);
    s << 'X' << c2 << std::endl;
    ASSERT_STREQ("Complex: (1.2,3.4)\nX(5.6,7.8)\n", p->str().c_str());
}
