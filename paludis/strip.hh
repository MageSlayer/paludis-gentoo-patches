/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef PALUDIS_GUARD_PALUDIS_STRIP_HH
#define PALUDIS_GUARD_PALUDIS_STRIP_HH 1

#include <string>
#include <functional>

namespace paludis
{
    std::string strip_leading_string(const std::string & s, const std::string & prefix);

    std::string strip_leading(const std::string & s, const std::string & remove);

    std::string strip_trailing_string(const std::string & s, const std::string & suffix);

    std::string strip_trailing(const std::string & s, const std::string & remove);

    template <std::string (* f_)(const std::string &, const std::string &)>
    class StripAdapter :
        public std::unary_function<std::string, const std::string>
    {
        private:
            const std::string _second;

        public:
            StripAdapter(const std::string & second) :
                _second(second)
            {
            }

            std::string operator() (const std::string & first) const
            {
                return (*f_)(first, _second);
            }
    };

    typedef StripAdapter<&strip_leading_string> StripLeadingString;
    typedef StripAdapter<&strip_leading> StripLeading;
    typedef StripAdapter<&strip_trailing_string> StripTrailingString;
    typedef StripAdapter<&strip_trailing> StripTrailing;
}

#endif
