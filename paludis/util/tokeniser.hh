/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_TOKENISER_HH
#define PALUDIS_GUARD_PALUDIS_TOKENISER_HH 1

#include <iterator>
#include <paludis/util/instantiation_policy.hh>
#include <string>

/** \file
 * Declarations for Tokeniser and related utilities.
 *
 * \ingroup g_strings
 *
 * \section Examples
 *
 * - None at this time.
 */

namespace paludis
{
    /**
     * Delimiter policy for Tokeniser.
     *
     * \ingroup g_strings
     */
    namespace delim_kind
    {
        /**
         * Any of the characters split, and the delimiter is discarded.
         *
         * \ingroup g_strings
         */
        struct AnyOfTag
        {
        };
    }

    /**
     * Delimiter mode for Tokeniser.
     *
     * \ingroup g_strings
     */
    namespace delim_mode
    {
        /**
         * Discard the delimiters.
         *
         * \ingroup g_strings
         */
        struct DelimiterTag
        {
        };

        /**
         * Keep the delimiters.
         *
         * \ingroup g_strings
         */
        struct BoundaryTag
        {
        };
    }

    /**
     * Tokeniser internal use only.
     *
     * \ingroup g_strings
     */
    namespace tokeniser_internals
    {
        /**
         * A Writer handles Tokeniser's writes.
         *
         * \ingroup g_strings
         */
        template <typename DelimMode_, typename Char_, typename Iter_>
        struct Writer;

        /**
         * A Writer handles Tokeniser's writes (specialisation for
         * delim_mode::DelimiterTag).
         *
         * \ingroup g_strings
         */
        template <typename Char_, typename Iter_>
        struct Writer<delim_mode::DelimiterTag, Char_, Iter_>
        {
            /**
             * Handle a token.
             */
            static void handle_token(const std::basic_string<Char_> & s, Iter_ & i)
            {
                *i++ = s;
            }

            /**
             * Handle a delimiter.
             */
            static void handle_delim(const std::basic_string<Char_> &, const Iter_ &)
            {
            }
        };

        /**
         * A Writer handles Tokeniser's writes (specialisation for
         * delim_mode::BoundaryTag).
         *
         * \ingroup g_strings
         */
        template <typename Char_, typename Iter_>
        struct Writer<delim_mode::BoundaryTag, Char_, Iter_>
        {
            /**
             * Handle a token.
             */
            static void handle_token(const std::basic_string<Char_> & s, Iter_ & i)
            {
                *i++ = s;
            }

            /**
             * Handle a delimiter.
             */
            static void handle_delim(const std::basic_string<Char_> & s, Iter_ & i)
            {
                *i++ = s;
            }
        };

    }

    /**
     * Tokeniser splits up strings into smaller strings.
     *
     * \ingroup g_strings
     */
    template <typename DelimKind_, typename DelimMode_ = delim_mode::DelimiterTag,
             typename Char_ = std::string::value_type>
    struct Tokeniser;

    /**
     * Tokeniser: specialisation for delim_kind::AnyOfTag.
     *
     * \ingroup g_strings
     * \nosubgrouping
     */
    template <typename DelimMode_, typename Char_>
    class Tokeniser<delim_kind::AnyOfTag, DelimMode_, Char_>
    {
        private:
            Tokeniser();

        public:
            ///\name Basic operations
            ///\{

            /**
             * Do the tokenisation.
             */
            template <typename Iter_>
            static void tokenise(const std::basic_string<Char_> & s,
                    const std::basic_string<Char_> & delims, Iter_ iter);
    };

    template <typename DelimMode_, typename Char_>
    template <typename Iter_>
    void
    Tokeniser<delim_kind::AnyOfTag, DelimMode_, Char_>::tokenise(
            const std::basic_string<Char_> & s, const std::basic_string<Char_> & delims, Iter_ iter)
    {
        typename std::basic_string<Char_>::size_type p(0), old_p(0);
        bool in_delim((! s.empty()) && std::basic_string<Char_>::npos != delims.find(s[0]));

        for ( ; p < s.length() ; ++p)
        {
            if (in_delim)
            {
                if (std::basic_string<Char_>::npos == delims.find(s[p]))
                {
                    tokeniser_internals::Writer<DelimMode_, Char_, Iter_>::handle_delim(
                            s.substr(old_p, p - old_p), iter);
                    in_delim = false;
                    old_p = p;
                }
            }
            else
            {
                if (std::basic_string<Char_>::npos != delims.find(s[p]))
                {
                    tokeniser_internals::Writer<DelimMode_, Char_, Iter_>::handle_token(
                            s.substr(old_p, p - old_p), iter);
                    in_delim = true;
                    old_p = p;
                }
            }
        }

        if (old_p != p)
        {
            if (in_delim)
                tokeniser_internals::Writer<DelimMode_, Char_, Iter_>::handle_delim(
                        s.substr(old_p, p - old_p), iter);
            else
                tokeniser_internals::Writer<DelimMode_, Char_, Iter_>::handle_token(
                        s.substr(old_p, p - old_p), iter);
        }
    }

    /**
     * Convenience class for tokenising on whitespace.
     *
     * \ingroup g_strings
     */
    class PALUDIS_VISIBLE WhitespaceTokeniser
    {
        public:
            template <typename Iter_>
            static void tokenise(const std::string & s, Iter_ iter)
            {
                Tokeniser<delim_kind::AnyOfTag>::tokenise(s, " \t\r\n", iter);
            }
    };
}

#endif
