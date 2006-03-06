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

#ifndef PALUDIS_GUARD_PALUDIS_TOKENISER_HH
#define PALUDIS_GUARD_PALUDIS_TOKENISER_HH 1

#include <string>
#include <iterator>
#include <paludis/util/instantiation_policy.hh>

/** \file
 * Declarations for Tokeniser and related utilities.
 *
 * \ingroup Tokeniser
 */

namespace paludis
{
    /**
     * Delimiter policy for Tokeniser.
     *
     * \ingroup Tokeniser
     */
    namespace delim_kind
    {
        /**
         * Any of the characters split, and the delimiter is discarded.
         *
         * \ingroup Tokeniser
         */
        struct AnyOfTag
        {
        };
    }

    /**
     * Delimiter mode for Tokeniser.
     *
     * \ingroup Tokeniser
     */
    namespace delim_mode
    {
        /**
         * Discard the delimiters.
         *
         * \ingroup Tokeniser
         */
        struct DelimiterTag
        {
        };

        /**
         * Keep the delimiters.
         *
         * \ingroup Tokeniser
         */
        struct BoundaryTag
        {
        };
    }

    /**
     * Tokeniser internal use only.
     *
     * \ingroup Tokeniser
     */
    namespace tokeniser_internals
    {
        /**
         * A Writer handles Tokeniser's writes.
         *
         * \ingroup Tokeniser
         */
        template <typename DelimMode_, typename Char_, typename Iter_>
        struct Writer;

        /**
         * A Writer handles Tokeniser's writes (specialisation for
         * delim_mode::DelimiterTag).
         *
         * \ingroup Tokeniser
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
         * \ingroup Tokeniser
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
     * \ingroup Tokeniser
     */
    template <typename DelimKind_, typename DelimMode_, typename Char_ = std::string::value_type>
    struct Tokeniser;

    /**
     * Tokeniser: specialisation for delim_kind::AnyOfTag.
     *
     * \ingroup Tokeniser
     */
    template <typename DelimMode_, typename Char_>
    class Tokeniser<delim_kind::AnyOfTag, DelimMode_, Char_> :
        private InstantiationPolicy<Tokeniser<delim_kind::AnyOfTag, DelimMode_, Char_>,
                instantiation_method::NonCopyableTag>
    {
        private:
            const std::basic_string<Char_> _delims;

        public:
            /**
             * Constructor.
             */
            Tokeniser(const std::basic_string<Char_> & delims) :
                _delims(delims)
            {
            }

            /**
             * Do the tokenisation.
             */
            template <typename Iter_>
            void tokenise(const std::basic_string<Char_> & s, Iter_ iter) const;
    };

    template <typename DelimMode_, typename Char_>
    template <typename Iter_>
    void
    Tokeniser<delim_kind::AnyOfTag, DelimMode_, Char_>::tokenise(
            const std::basic_string<Char_> & s, Iter_ iter) const
    {
        typename std::basic_string<Char_>::size_type p(0), old_p(0);
        bool in_delim((! s.empty()) && std::basic_string<Char_>::npos != _delims.find(s[0]));

        for ( ; p < s.length() ; ++p)
        {
            if (in_delim)
            {
                if (std::basic_string<Char_>::npos == _delims.find(s[p]))
                {
                    tokeniser_internals::Writer<DelimMode_, Char_, Iter_>::handle_delim(
                            s.substr(old_p, p - old_p), iter);
                    in_delim = false;
                    old_p = p;
                }
            }
            else
            {
                if (std::basic_string<Char_>::npos != _delims.find(s[p]))
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

}

#endif
