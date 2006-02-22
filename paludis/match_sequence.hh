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

#ifndef PALUDIS_GUARD_PALUDIS_MATCH_SEQUENCE_HH
#define PALUDIS_GUARD_PALUDIS_MATCH_SEQUENCE_HH 1

#include <string>
#include <paludis/counted_ptr.hh>

namespace paludis
{
    class MatchRule
    {
        private:
            struct Rule;
            struct StringRule;
            struct SequenceRule;
            struct ZeroOrMoreRule;
            struct EitherRule;
            struct EolRule;

            CountedPtr<Rule> _rule;

            void operator= (const MatchRule &);

            MatchRule(CountedPtr<Rule>);

        public:
            MatchRule(const std::string &);

            MatchRule(const MatchRule &);

            static const MatchRule eol();

            ~MatchRule();

            const MatchRule operator>> (const MatchRule &) const;

            const MatchRule operator|| (const MatchRule &) const;

            const MatchRule operator* () const;

            bool match(const std::string &) const;
    };
}

#endif
