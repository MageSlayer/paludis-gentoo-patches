/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef PALUDIS_GUARD_PALUDIS_COMPARE_HH
#define PALUDIS_GUARD_PALUDIS_COMPARE_HH 1

#include <paludis/attributes.hh>
#include <paludis/validated.hh>
#include <string>

namespace paludis
{
    inline int compare(int t1, int t2) PALUDIS_ATTRIBUTE((always_inline));

    inline int compare(int t1, int t2)
    {
        if (t1 < t2)
            return -1;
        else if (t2 > t1)
            return 1;
        else
            return 0;
    }

    inline int compare(unsigned t1, unsigned t2) PALUDIS_ATTRIBUTE((always_inline));

    inline int compare(unsigned t1, unsigned t2)
    {
        if (t1 < t2)
            return -1;
        else if (t2 > t1)
            return 1;
        else
            return 0;
    }

    inline int long compare(unsigned long t1, unsigned long t2) PALUDIS_ATTRIBUTE((always_inline));

    inline int long compare(unsigned long t1, unsigned long t2)
    {
        if (t1 < t2)
            return -1;
        else if (t2 > t1)
            return 1;
        else
            return 0;
    }

    inline int compare(long t1, long t2) PALUDIS_ATTRIBUTE((always_inline));

    inline int compare(long t1, long t2)
    {
        if (t1 < t2)
            return -1;
        else if (t2 > t1)
            return 1;
        else
            return 0;
    }

    template <typename T_>
    inline int compare(
            const std::basic_string<T_> & t1,
            const std::basic_string<T_> & t2)
        PALUDIS_ATTRIBUTE((always_inline));

    template <typename T_>
    inline int compare(
            const std::basic_string<T_> & t1,
            const std::basic_string<T_> & t2)
    {
        register int r(t1.compare(t2));
        if (r < 0)
            return -1;
        else if (r > 0)
            return 1;
        else
            return 0;
    }

    template <typename T_, typename U_>
    inline int compare(
            const Validated<T_, U_> & t1,
            const Validated<T_, U_> & t2)
    {
        return compare(t1.data(), t2.data());
    }

    template <typename T_>
    int compare(
            const T_ & t1,
            const T_ & t2)
    {
        if (t1 < t2)
            return -1;
        else if (t1 > t2)
            return 1;
        else
            return 0;
    }
}

#endif
