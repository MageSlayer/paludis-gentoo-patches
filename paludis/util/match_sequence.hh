/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#include <paludis/util/counted_ptr.hh>
#include <string>

/** \file
 * Declarations for the MatchRule classes.
 *
 * \ingroup grpmatch
 */

namespace paludis
{
    /**
     * MatchRule is used to match text.
     *
     * \ingroup grpmatch
     */
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
            /**
             * Constructor.
             */
            MatchRule(const std::string &);

            /**
             * Copy constructor.
             */
            MatchRule(const MatchRule &);

            /**
             * A rule matching end of line.
             */
            static const MatchRule eol();

            /**
             * Destructor.
             */
            ~MatchRule();

            /**
             * Followed by rule.
             */
            const MatchRule operator>> (const MatchRule &) const;

            /**
             * Alternation rule.
             */
            const MatchRule operator|| (const MatchRule &) const;

            /**
             * Repetition rule.
             */
            const MatchRule operator* () const;

            /**
             * Match against a string.
             */
            bool match(const std::string &) const;
    };
}

#endif
