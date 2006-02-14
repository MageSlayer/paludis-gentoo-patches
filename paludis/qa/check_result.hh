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

#ifndef PALUDIS_GUARD_PALUDIS_QA_CHECK_RESULT_HH
#define PALUDIS_GUARD_PALUDIS_QA_CHECK_RESULT_HH 1

#include <paludis/fs_entry.hh>
#include <paludis/qa/message.hh>
#include <list>

/** \file
 * Declarations for the CheckResult class.
 *
 * \ingroup QA
 */

namespace paludis
{
    namespace qa
    {
        /**
         * The result of a QA check.
         *
         * \ingroup QA
         */
        class CheckResult
        {
            private:
                std::list<Message> _messages;

                const std::string _item;
                const std::string _rule;

            public:
                CheckResult(const FSEntry &, const std::string &);

                CheckResult(const std::string &, const std::string &);

                bool empty() const
                {
                    return _messages.empty();
                }

                CheckResult & operator<< (const Message & m)
                {
                    _messages.push_back(m);
                    return *this;
                }

                typedef std::list<Message>::const_iterator Iterator;

                Iterator begin() const
                {
                    return _messages.begin();
                }

                Iterator end() const
                {
                    return _messages.end();
                }

                const std::string & item() const
                {
                    return _item;
                }

                const std::string & rule() const
                {
                    return _rule;
                }

                QALevel most_severe_level() const;
        };
    }
}

#endif
