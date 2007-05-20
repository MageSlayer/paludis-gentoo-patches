/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <paludis/util/virtual_constructor.hh>
#include <paludis/util/virtual_constructor-impl.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include <set>
#include <paludis/util/tr1_memory.hh>

/** \file
 * Test cases for VirtualConstructor.
 *
 */

using namespace test;
using namespace paludis;

namespace
{
    enum CookieSize
    {
        cs_small,
        cs_large
    };

    class Cookie
    {
        private:
            CookieSize _size;

        protected:
            Cookie(CookieSize sz) :
                _size(sz)
            {
            }

        public:
            virtual std::string flavour() const = 0;

            virtual ~Cookie()
            {
            }

            CookieSize size() const
            {
                return _size;
            }
    };

    struct NoCookie
    {
        NoCookie(const std::string &)
        {
        }
    };

    class ChocolateChipCookie : public Cookie
    {
        public:
            ChocolateChipCookie(CookieSize sz) :
                Cookie(sz)
            {
            }

            std::string flavour() const
            {
                return "Chocolate Chip";
            }

            static tr1::shared_ptr<Cookie> make(CookieSize size)
            {
                return tr1::shared_ptr<Cookie>(new ChocolateChipCookie(size));
            }
    };

    class GingerCookie : public Cookie
    {
        private:
            bool _with_crunchy_bits;

        public:
            GingerCookie(CookieSize sz, bool with_crunchy_bits) :
                Cookie(sz),
                _with_crunchy_bits(with_crunchy_bits)
            {
            }

            std::string flavour() const
            {
                return _with_crunchy_bits ? "Crunchy Ginger" : "Ginger";
            }

            static tr1::shared_ptr<Cookie> make(CookieSize size)
            {
                return tr1::shared_ptr<Cookie>(new GingerCookie(size, false));
            }

            static tr1::shared_ptr<Cookie> make_crunchy(CookieSize size)
            {
                return tr1::shared_ptr<Cookie>(new GingerCookie(size, true));
            }
    };

    class CookieMaker :
        public VirtualConstructor<std::string,
            tr1::shared_ptr<Cookie> (*) (CookieSize),
            virtual_constructor_not_found::ThrowException<NoCookie> >,
        public InstantiationPolicy<CookieMaker, instantiation_method::SingletonTag>
    {
        friend class InstantiationPolicy<CookieMaker, instantiation_method::SingletonTag>;

        private:
            CookieMaker()
            {
                register_maker("chocolate chip", &ChocolateChipCookie::make);
                register_maker("ginger", &GingerCookie::make);
                register_maker("crunchy ginger", &GingerCookie::make_crunchy);
            }
    };
}

namespace test_cases
{
    /**
     * \test Test VirtualConstructor.
     *
     */
    struct VirtualConstructorTest : TestCase
    {
        VirtualConstructorTest() : TestCase("virtual constructor") { }

        void run()
        {
            TEST_CHECK_EQUAL(CookieMaker::get_instance()->find_maker(
                        "chocolate chip")(cs_large)->flavour(), "Chocolate Chip");
            TEST_CHECK_EQUAL(CookieMaker::get_instance()->find_maker(
                        "chocolate chip")(cs_large)->size(), cs_large);
            TEST_CHECK_EQUAL(CookieMaker::get_instance()->find_maker(
                        "chocolate chip")(cs_small)->size(), cs_small);

            TEST_CHECK_EQUAL((*CookieMaker::get_instance())["ginger"](cs_small)->flavour(),
                    "Ginger");
            TEST_CHECK_EQUAL((*CookieMaker::get_instance())["crunchy ginger"](cs_small)->flavour(),
                    "Crunchy Ginger");

            TEST_CHECK_THROWS(CookieMaker::get_instance()->find_maker(
                        "gerbil")(cs_large)->flavour(), NoCookie);
        }
    } test_virtual_constructor;

    /**
     * \test Test VirtualConstructor keys
     *
     */
    struct VirtualConstructorKeysTest : TestCase
    {
        VirtualConstructorKeysTest() : TestCase("virtual constructor keys") { }

        void run()
        {
            std::set<std::string> keys;

            TEST_CHECK(keys.empty());
            CookieMaker::get_instance()->copy_keys(std::inserter(keys, keys.begin()));
            TEST_CHECK(! keys.empty());
            TEST_CHECK(keys.end() != keys.find("crunchy ginger"));
            TEST_CHECK(keys.end() != keys.find("ginger"));
            TEST_CHECK(keys.end() != keys.find("chocolate chip"));
            TEST_CHECK(keys.end() == keys.find("gerbil"));
        }
    } test_virtual_constructor_keys;
}

