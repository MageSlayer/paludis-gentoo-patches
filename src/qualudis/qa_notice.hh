/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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

#ifndef PALUDIS_GUARD_SRC_QUALUDIS_QA_NOTICE_HH
#define PALUDIS_GUARD_SRC_QUALUDIS_QA_NOTICE_HH 1

#include <paludis/paludis.hh>
#include <string>
#include <deque>
#include <iosfwd>

/**
 * Different levels for QANotice messages.
 *
 * Keep these in order, least severe first. Several methods rely upon
 * std::max() to determine which message is more severe.
 */
enum QANoticeLevel
{
    qal_info,    ///< Information messages
    qal_maybe,   ///< May or may not be a problem
    qal_minor,   ///< Minor (usually syntax) issues
    qal_major,   ///< Serious issues
    qal_fatal    ///< Error preventing further QA checks
};

class QANotice
{
    friend std::ostream & operator<< (std::ostream &, const QANotice &);

    private:
        const QANoticeLevel _level;
        const std::string _affected;
        const std::string _message;

    public:
        QANotice(const QANoticeLevel l, const std::string & affected, const std::string & m) :
            _level(l),
            _affected(affected),
            _message(m)
        {
        }

        bool operator< (const QANotice &) const;
};

class QANotices :
    public paludis::InstantiationPolicy<QANotices, paludis::instantiation_method::SingletonAsNeededTag>
{
    friend class paludis::InstantiationPolicy<QANotices, paludis::instantiation_method::SingletonAsNeededTag>;
    friend std::ostream & operator<< (std::ostream &, const QANotices &);

    private:
        std::multiset<QANotice> _notices;

        QANotices();

        ~QANotices();

    public:
        QANotices &
        operator<< (const QANotice & n)
        {
            _notices.insert(n);
            return *this;
        }
};

std::ostream &
operator<< (std::ostream &, const QANotice &);

std::ostream &
operator<< (std::ostream &, const QANotices &);

#endif
