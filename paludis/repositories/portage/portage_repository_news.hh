/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_PORTAGE_PORTAGE_REPOSITORY_NEWS_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_PORTAGE_PORTAGE_REPOSITORY_NEWS_HH 1

#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/instantiation_policy.hh>
#include <paludis/config_file.hh>

/** \file
 * Declaration for the PortageRepositoryNews class.
 *
 * \ingroup grpportagerepository
 */

namespace paludis
{
    class Environment;
    class FSEntry;
    class PortageRepository;

    /**
     * Holds the news/ data for a PortageRepository instance.
     *
     * \ingroup grpportagerepository
     */
    class PortageRepositoryNews :
        private PrivateImplementationPattern<PortageRepositoryNews>,
        private InstantiationPolicy<PortageRepositoryNews, instantiation_method::NonCopyableTag>,
        public InternalCounted<PortageRepositoryNews>
    {
        public:
            ///\name Basic operations
            ///\{

            PortageRepositoryNews(const Environment * const, const PortageRepository * const,
                    const PortageRepositoryParams &);
            ~PortageRepositoryNews();

            ///\}

            void update_news() const;
    };

    /**
     * A NewsFile represents a GLEP 42 news file.
     *
     * \ingroup grpnewsconfigfile
     */
    class NewsFile :
        protected ConfigFile,
        private PrivateImplementationPattern<NewsFile>
    {
        protected:
            void accept_line(const std::string &) const;

        public:
            ///\name Basic operations
            ///\{

            /**
             * Constructor, from a filename.
             */
            NewsFile(const FSEntry & filename);

            ~NewsFile();

            ///\}

            ///\name Iterate over our Display-If-Installed headers
            ///\{

            /// Tag for DisplayIfInstalledIterator.
            struct DisplayIfInstalledIteratorTag;

            typedef libwrapiter::ForwardIterator<DisplayIfInstalledIteratorTag,
                    const std::string> DisplayIfInstalledIterator;

            DisplayIfInstalledIterator begin_display_if_installed() const;

            DisplayIfInstalledIterator end_display_if_installed() const;

            ///\}

            ///\name Iterate over our Display-If-Keyword headers
            ///\{

            /// Tag for DisplayIfKeywordIterator.
            struct DisplayIfKeywordIteratorTag;

            typedef libwrapiter::ForwardIterator<DisplayIfKeywordIteratorTag,
                    const std::string> DisplayIfKeywordIterator;

            DisplayIfKeywordIterator begin_display_if_keyword() const;

            DisplayIfKeywordIterator end_display_if_keyword() const;

            ///\}

            ///\name Iterate over our Display-If-Profile headers
            ///\{

            /// Tag for DisplayIfProfileIterator.
            struct DisplayIfProfileIteratorTag;

            typedef libwrapiter::ForwardIterator<DisplayIfProfileIteratorTag,
                    const std::string> DisplayIfProfileIterator;

            DisplayIfProfileIterator begin_display_if_profile() const;

            DisplayIfProfileIterator end_display_if_profile() const;

            ///\}
    };
}

#endif
