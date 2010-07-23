/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010 Ciaran McCreesh
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

#include <paludis/version_spec-fwd.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/pimp.hh>
#include <paludis/util/operators.hh>
#include <paludis/util/named_value.hh>
#include <paludis/util/wrapped_forward_iterator-fwd.hh>

#include <iosfwd>
#include <string>

/** \file
 * Declarations for VersionSpec classes.
 *
 * \ingroup g_names
 *
 * \section Examples
 *
 * - \ref example_version_spec.cc "example_version_spec.cc"
 * - \ref example_version_operator.cc "example_version_operator.cc"
 */

namespace paludis
{
    /**
     * Thrown if a VersionSpec is created from an invalid version string.
     *
     * \ingroup g_exceptions
     * \ingroup g_names
     */
    class PALUDIS_VISIBLE BadVersionSpecError :
        public NameError
    {
        public:
            ///\name Basic operations
            ///\{

            BadVersionSpecError(const std::string & name) throw ();
            BadVersionSpecError(const std::string & name, const std::string & msg) throw ();

            ///\}
    };

    namespace n
    {
        typedef Name<struct number_value_name> number_value;
        typedef Name<struct text_name> text;
        typedef Name<struct type_name> type;
    }

    /**
     * A VersionSpec component part.
     *
     * This is mostly for use by the Exheres 'ever' API.
     *
     * The version 1.2x_pre3_rc-scm would be split up into these parts:
     *
     * - (type,        number_value,   text)
     * - (vsct_number, "1",            "1")
     * - (vsct_number, "2",            ".2")
     * - (vsct_letter, "x",            "x")
     * - (vsct_pre,    "3",            "_pre3")
     * - (vsct_rc,     "MAX"           "_rc0")
     * - (vsct_scm,    "0"             "-scm")
     *
     * The type and number_value are mostly for internal use. In particular, the
     * number_value is not always what you might expect it to be.
     *
     * The text includes any preceeding delimiter.
     *
     * \ingroup g_names
     * \since 0.32
     */
    struct VersionSpecComponent
    {
        NamedValue<n::number_value, std::string> number_value;
        NamedValue<n::text, std::string> text;
        NamedValue<n::type, VersionSpecComponentType> type;
    };

    /**
     * A VersionSpec represents a version number (for example, 1.2.3b-r1).
     *
     * \ingroup g_names
     */
    class PALUDIS_VISIBLE VersionSpec :
        private Pimp<VersionSpec>,
        public relational_operators::HasRelationalOperators
    {
        friend std::ostream & operator<< (std::ostream &, const VersionSpec &);

        public:
            ///\name Basic operations
            ///\{

            /**
             * Constructor.
             *
             * \param options \since 0.38
             */
            VersionSpec(const std::string & text, const VersionSpecOptions & options);

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

            std::size_t hash() const PALUDIS_ATTRIBUTE((warn_unused_result));

            ///\}

            ///\name Comparison operators
            ///\{

            /**
             * Comparison function for ~ depend operator.
             */
            bool tilde_compare(const VersionSpec & other) const;

            /**
             * Comparison function for ~> depend operator (gems).
             */
            bool tilde_greater_compare(const VersionSpec & other) const;

            /**
             * Comparison function for =* depend operator in sensible EAPIs.
             */
            bool nice_equal_star_compare(const VersionSpec & other) const;

            /**
             * Comparison function for =* depend operator in Gentooish EAPIs.
             */
            bool stupid_equal_star_compare(const VersionSpec & other) const;

            /**
             * Compare to another version.
             */
            int compare(const VersionSpec & other) const;

            bool operator== (const VersionSpec &) const;

            bool operator< (const VersionSpec &) const;

            ///\}

            /**
             * Remove the revision part.
             */
            VersionSpec remove_revision() const;

            /**
             * Revision part only (or "r0").
             */
            std::string revision_only() const;

            /**
             * Bump ourself.
             *
             * This is used by the ~> operator. It returns a version where the
             * next to last number is one greater (e.g. 5.3.1 => 5.4). Any non
             * number parts are stripped (e.g. 1.2.3_alpha4-r5 => 1.3).
             */
            VersionSpec bump() const;

            /**
             * Are we an -scm package, or something pretending to be one?
             */
            bool is_scm() const;

            /**
             * Do we have a -try part?
             */
            bool has_try_part() const;

            /**
             * Do we have an -scm part?
             *
             * Use is_scm() if -9999 etc is desired.
             */
            bool has_scm_part() const;

            /**
             * Do we have a local revision (-r1.2...)?
             */
            bool has_local_revision() const;

            struct ConstIteratorTag;
            typedef WrappedForwardIterator<ConstIteratorTag, const VersionSpecComponent> ConstIterator;
            ConstIterator begin() const;
            ConstIterator end() const;
    };
}

#endif
