/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_GLSA_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_GLSA_HH 1

#include <paludis/util/pimp.hh>
#include <paludis/util/wrapped_forward_iterator-fwd.hh>
#include <paludis/util/named_value.hh>
#include <paludis/name.hh>
#include <paludis/version_spec.hh>
#include <memory>
#include <string>

namespace paludis
{
    namespace n
    {
        typedef Name<struct name_op> op;
        typedef Name<struct name_slot> slot;
        typedef Name<struct name_version> version;
    }

    namespace erepository
    {
        struct GLSARange
        {
            NamedValue<n::op, std::string> op;
            NamedValue<n::slot, std::string> slot;
            NamedValue<n::version, std::string> version;
        };
    }

    /**
     * Thrown if a bad GLSA is found.
     *
     * \ingroup grpexceptions
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE GLSAError :
        public ConfigurationError
    {
        public:
            ///\name Basic operations
            ///\{

            GLSAError(const std::string & message,
                    const std::string & filename = "") throw ();

            ///\}
    };

    /**
     * Represents a package entry in a GLSA.
     *
     * \see GLSA
     * \ingroup grperepository
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE GLSAPackage :
        private Pimp<GLSAPackage>
    {
        public:
            ///\name Basic operations
            ///\{

            GLSAPackage(const QualifiedPackageName & name);
            ~GLSAPackage();

            ///\}

            ///\name Iterate over our archs.
            ///\{

            struct ArchsConstIteratorTag;
            typedef WrappedForwardIterator<ArchsConstIteratorTag, const std::string> ArchsConstIterator;
            ArchsConstIterator begin_archs() const;
            ArchsConstIterator end_archs() const;

            ///\}

            /**
             * Add an arch.
             */
            void add_arch(const std::string &);

            ///\name Iterate over our ranges.
            ///\{

            struct RangesConstIteratorTag;
            typedef WrappedForwardIterator<RangesConstIteratorTag, const erepository::GLSARange> RangesConstIterator;
            RangesConstIterator begin_unaffected() const;
            RangesConstIterator end_unaffected() const;
            RangesConstIterator begin_vulnerable() const;
            RangesConstIterator end_vulnerable() const;

            ///\}

            /**
             * Add an unaffected package.
             */
            void add_unaffected(const erepository::GLSARange &);

            /**
             * Add a vulnerable package.
             */
            void add_vulnerable(const erepository::GLSARange &);

            /**
             * Our package's name.
             */
            QualifiedPackageName name() const;
    };

    /**
     * Represents a GLSA (security advisory).
     *
     * \ingroup grperepository
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE GLSA :
        private Pimp<GLSA>
    {
        public:
            ///\name Basic operations
            ///\{

            GLSA();
            ~GLSA();

            ///\}

            /**
             * Create a GLSA from an XML file.
             */
            static std::shared_ptr<GLSA> create_from_xml_file(const std::string & filename);

            ///\name Iterate over our packages.
            ///\{

            struct PackagesConstIteratorTag;
            typedef WrappedForwardIterator<PackagesConstIteratorTag, const GLSAPackage> PackagesConstIterator;
            PackagesConstIterator begin_packages() const;
            PackagesConstIterator end_packages() const;

            ///\}

            /**
             * Add a package.
             */
            void add_package(const std::shared_ptr<const GLSAPackage> &);

            /**
             * Set our ID.
             */
            void set_id(const std::string &);

            /**
             * Fetch our ID.
             */
            std::string id() const;

            /**
             * Set our title.
             */
            void set_title(const std::string &);

            /**
             * Fetch our title.
             */
            std::string title() const;
    };
}

#endif
