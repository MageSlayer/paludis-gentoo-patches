/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_VERSION_SPEC_HH
#define PALUDIS_GUARD_PALUDIS_VERSION_SPEC_HH 1

#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/comparison_policy.hh>
#include <paludis/util/sorted_collection.hh>
#include <paludis/util/exception.hh>
#include <string>
#include <ostream>

namespace paludis
{
    /**
     * Thrown if a VersionSpec is created from an invalid version string.
     *
     * \ingroup Exception
     * \ingroup Database
     */
    class BadVersionSpecError : public NameError
    {
        public:
            /**
             * Constructor.
             */
            BadVersionSpecError(const std::string & name) throw ();
    };

    /**
     * A VersionSpec represents a version number (for example, 1.2.3b-r1).
     */
    class VersionSpec : private PrivateImplementationPattern<VersionSpec>,
                        public ComparisonPolicy<VersionSpec, comparison_mode::FullComparisonTag,
                            comparison_method::CompareByMemberComparisonFunctionTag>
    {
        friend std::ostream & operator<< (std::ostream &, const VersionSpec &);

        protected:
            /**
             * Compare to another version.
             */
            int compare(const VersionSpec & other) const;

        public:
            /**
             * Maximum number of a.b.c.d parts allowed in a version number. We
             * use this to avoid having to dynamically allocate memory.
             */
            static const unsigned max_version_parts_count = 10;

            /**
             * Constructor.
             */
            VersionSpec(const std::string & text);

            /**
             * Copy constructor.
             */
            VersionSpec(const VersionSpec & other);

            /**
             * Destructor.
             */
            ~VersionSpec();

            /**
             * Assignment.
             */
            const VersionSpec & operator= (const VersionSpec & other);

            /**
             * Comparison function for ~ depend operator.
             */
            bool tilde_compare(const VersionSpec & other) const;

            /**
             * Comparison function for =* depend operator.
             */
            bool equal_star_compare(const VersionSpec & other) const;

            /**
             * Fetch a hash value, used to avoid exposing our internals to
             * CRCHash.
             */
            std::size_t hash_value() const;

            /**
             * Remove the revision part.
             */
            VersionSpec remove_revision() const;

            /**
             * Revision part only (or "r0").
             */
            std::string revision_only() const;

    };

    /**
     * Output a VersionSpec to a stream.
     */
    std::ostream & operator<< (std::ostream &, const VersionSpec &);

    /**
     * Holds a collection of VersionSpec instances.
     */
    typedef SortedCollection<VersionSpec> VersionSpecCollection;
}

#endif
