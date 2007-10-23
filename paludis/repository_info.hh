/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORY_INFO_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORY_INFO_HH 1

#include <paludis/repository_info-fwd.hh>

/** \file
 * Declarations for RepositoryInfo classes.
 *
 * \ingroup g_repository
 *
 * \section Examples
 *
 * - \ref example_repository.cc "example_repository.cc"
 */

namespace paludis
{
    /**
     * A section of information about a Repository.
     *
     * \see RepositoryInfo
     * \ingroup g_repository
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE RepositoryInfoSection :
        private PrivateImplementationPattern<RepositoryInfoSection>
    {
        public:
            ///\name Basic operations
            ///\{

            RepositoryInfoSection(const std::string & heading);

            ~RepositoryInfoSection();

            ///\}

            /// Our heading.
            std::string heading() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            ///\name Iterate over our key/values
            ///\{

            typedef libwrapiter::ForwardIterator<RepositoryInfoSection,
                    const std::pair<const std::string, std::string> > KeyValueConstIterator;

            KeyValueConstIterator begin_kvs() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            KeyValueConstIterator end_kvs() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            ///\}

            /// Add a key/value pair.
            RepositoryInfoSection & add_kv(const std::string &, const std::string &);

            /// Fetch a value, with default.
            std::string get_key_with_default(const std::string &, const std::string &) const;
    };

    /**
     * Information about a Repository, for the end user.
     *
     * \ingroup g_repository
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE RepositoryInfo :
        private PrivateImplementationPattern<RepositoryInfo>
    {
        public:
            ///\name Basic operations
            ///\{

            RepositoryInfo();
            ~RepositoryInfo();

            ///\}

            ///\name ConstIterator over our sections
            ///\{

            typedef libwrapiter::ForwardIterator<RepositoryInfo,
                    const tr1::shared_ptr<const RepositoryInfoSection> > SectionConstIterator;

            SectionConstIterator begin_sections() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            SectionConstIterator end_sections() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            ///\}

            /// Add a section.
            RepositoryInfo & add_section(tr1::shared_ptr<const RepositoryInfoSection>);

            /// Fetch a value from any of our sections, with default.
            std::string get_key_with_default(const std::string &, const std::string &) const;
    };
}

#endif
