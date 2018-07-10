/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_E_REPOSITORY_NEWS_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_E_REPOSITORY_NEWS_HH 1

#include <paludis/util/pimp.hh>
#include <paludis/util/config_file.hh>
#include <paludis/util/wrapped_forward_iterator-fwd.hh>

/** \file
 * Declaration for the ERepositoryNews class.
 *
 * \ingroup grperepository
 */

namespace paludis
{
    class Environment;
    class ERepository;

    /**
     * Holds the news/ data for a ERepository instance.
     *
     * \ingroup grperepository
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE ERepositoryNews
    {
        private:
            Pimp<ERepositoryNews> _imp;

        public:
            ///\name Basic operations
            ///\{

            ERepositoryNews(const Environment * const, const ERepository * const,
                    const erepository::ERepositoryParams &);
            ~ERepositoryNews();

            ERepositoryNews(const ERepositoryNews &) = delete;
            ERepositoryNews & operator= (const ERepositoryNews &) = delete;

            ///\}

            void update_news() const;
    };

    class PALUDIS_VISIBLE NewsError :
        public Exception
    {
        public:
            NewsError(const FSPath &, const std::string &) noexcept;
    };

    /**
     * A NewsFile represents a GLEP 42 news file.
     *
     * \ingroup grpnewsconfigfile
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE NewsFile
    {
        private:
            Pimp<NewsFile> _imp;

        public:
            ///\name Basic operations
            ///\{

            /**
             * Constructor, from a filename.
             */
            NewsFile(const FSPath & filename);

            ~NewsFile();

            ///\}

            ///\name Iterate over our Display-If-Installed headers
            ///\{

            struct DisplayIfInstalledConstIteratorTag;
            typedef WrappedForwardIterator<DisplayIfInstalledConstIteratorTag,
                    const std::string> DisplayIfInstalledConstIterator;

            DisplayIfInstalledConstIterator begin_display_if_installed() const;

            DisplayIfInstalledConstIterator end_display_if_installed() const;

            ///\}

            ///\name Iterate over our Display-If-Keyword headers
            ///\{

            struct DisplayIfKeywordConstIteratorTag;
            typedef WrappedForwardIterator<DisplayIfKeywordConstIteratorTag,
                    const std::string> DisplayIfKeywordConstIterator;

            DisplayIfKeywordConstIterator begin_display_if_keyword() const;

            DisplayIfKeywordConstIterator end_display_if_keyword() const;

            ///\}

            ///\name Iterate over our Display-If-Profile headers
            ///\{

            struct DisplayIfProfileConstIteratorTag;
            typedef WrappedForwardIterator<DisplayIfProfileConstIteratorTag,
                    const std::string> DisplayIfProfileConstIterator;

            DisplayIfProfileConstIterator begin_display_if_profile() const;

            DisplayIfProfileConstIterator end_display_if_profile() const;

            ///\}

	    int version() const;
    };
}

#endif
