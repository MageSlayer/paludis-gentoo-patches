/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "qa_notice.hh"
#include <ostream>
#include <algorithm>
#include <iterator>

QANotices::QANotices()
{
}

QANotices::~QANotices()
{
}

std::ostream &
operator<< (std::ostream & s, const QANotice & n)
{
    do
    {
        switch (n._level)
        {
            case qal_info:
                s << "info:  ";
                continue;

            case qal_maybe:
                s << "maybe: ";
                continue;

            case qal_minor:
                s << "minor: ";
                continue;

            case qal_major:
                s << "major: ";
                continue;

            case qal_fatal:
                s << "fatal: ";
                continue;
        }
        throw paludis::InternalError(PALUDIS_HERE, "Bad n.level");

    } while (false);

    s << n._affected << ":\n    " << n._message;
    return s;
}

bool
QANotice::operator< (const QANotice & other) const
{
    if (_level < other._level)
        return true;
    if (_level > other._level)
        return false;

    if (_affected < other._affected)
        return true;
    if (_affected > other._affected)
        return false;

    return _message < other._message;
}

std::ostream &
operator<< (std::ostream & s, const QANotices & n)
{
    std::copy(n._notices.begin(), n._notices.end(),
            std::ostream_iterator<QANotice>(s, "\n"));
    return s;
}

