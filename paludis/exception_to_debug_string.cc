/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "exception_to_debug_string.hh"

using namespace paludis;

#ifdef PALUDIS_TEST_CASE

std::string exception_to_debug_string(const std::exception & e)
{
    const paludis::Exception * ee;
    if (0 != ((ee = dynamic_cast<const Exception *>(&e))))
        return stringify(ee->what()) + " (message " + ee->message() +
            (ee->empty() ? stringify("") : ", backtrace " + ee->backtrace(" -> ")) + ")";
    else
        return e.what();
}

#endif

