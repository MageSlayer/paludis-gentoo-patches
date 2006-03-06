/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include <paludis/util/virtual_constructor.hh>
#include <paludis/util/counted_ptr.hh>
#include <test/test_runner.hh>
#include <test/test_framework.hh>
#include <set>

using namespace test;
using namespace paludis;

#ifndef DOXYGEN
enum CookieSize
{
    cs_small,
    cs_large
};

class Cookie :
    public InternalCounted<Cookie>
{
    private:
        CookieSize _size;

    protected:
        Cookie(CookieSize size) :
            _size(size)
        {
        };

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

typedef VirtualConstructor<std::string,
        CountedPtr<Cookie> (*) (CookieSize),
        virtual_constructor_not_found::ThrowException<NoCookie> > CookieMaker;

class ChocolateChipCookie : public Cookie
{
    public:
        ChocolateChipCookie(CookieSize size) :
            Cookie(size)
        {
        }

        std::string flavour() const
        {
            return "Chocolate Chip";
        }

        static CountedPtr<Cookie> make(CookieSize size)
        {
            return CountedPtr<Cookie>(new ChocolateChipCookie(size));
        }
};

CookieMaker::RegisterMaker register_chocolate_chip("chocolate chip", &ChocolateChipCookie::make);

class GingerCookie : public Cookie
{
    private:
        bool _with_crunchy_bits;

    public:
        GingerCookie(CookieSize size, bool with_crunchy_bits) :
            Cookie(size),
            _with_crunchy_bits(with_crunchy_bits)
        {
        }

        std::string flavour() const
        {
            return _with_crunchy_bits ? "Crunchy Ginger" : "Ginger";
        }

        bool with_crunchy_bits() const
        {
            return _with_crunchy_bits;
        }

        static CountedPtr<Cookie> make(CookieSize size)
        {
            return CountedPtr<Cookie>(new GingerCookie(size, false));
        }

        static CountedPtr<Cookie> make_crunchy(CookieSize size)
        {
            return CountedPtr<Cookie>(new GingerCookie(size, true));
        }
};

CookieMaker::RegisterMaker register_ginger("ginger", &GingerCookie::make);
CookieMaker::RegisterMaker register_crunchy_ginger("crunchy ginger", &GingerCookie::make_crunchy);

#endif

namespace test_cases
{
    /**
     * \test Test VirtualConstructor.
     *
     * \ingroup Test
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
     * \ingroup Test
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

