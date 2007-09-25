/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/sr.hh>
#include <paludis/util/tr1_memory.hh>
#include <paludis/name.hh>
#include <paludis/version_spec.hh>
#include <libwrapiter/libwrapiter_forward_iterator-fwd.hh>

#include <string>

namespace paludis
{

#include <paludis/repositories/e/glsa-sr.hh>

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
        private PrivateImplementationPattern<GLSAPackage>
    {
        public:
            ///\name Basic operations
            ///\{

            GLSAPackage(const QualifiedPackageName & name);
            ~GLSAPackage();

            ///\}

            ///\name Iterate over our archs.
            ///\{

            typedef libwrapiter::ForwardIterator<GLSAPackage, const UseFlagName> ArchsConstIterator;
            ArchsConstIterator begin_archs() const;
            ArchsConstIterator end_archs() const;

            ///\}

            /**
             * Add an arch.
             */
            void add_arch(const UseFlagName &);

            ///\name Iterate over our ranges.
            ///\{

            typedef libwrapiter::ForwardIterator<GLSAPackage, const GLSARange> RangesConstIterator;
            RangesConstIterator begin_unaffected() const;
            RangesConstIterator end_unaffected() const;
            RangesConstIterator begin_vulnerable() const;
            RangesConstIterator end_vulnerable() const;

            ///\}

            /**
             * Add an unaffected package.
             */
            void add_unaffected(const GLSARange &);

            /**
             * Add a vulnerable package.
             */
            void add_vulnerable(const GLSARange &);

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
        private PrivateImplementationPattern<GLSA>
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
            static tr1::shared_ptr<GLSA> create_from_xml_file(const std::string & filename);

            ///\name Iterate over our packages.
            ///\{

            typedef libwrapiter::ForwardIterator<GLSA, const GLSAPackage> PackagesConstIterator;
            PackagesConstIterator begin_packages() const;
            PackagesConstIterator end_packages() const;

            ///\}

            /**
             * Add a package.
             */
            void add_package(tr1::shared_ptr<const GLSAPackage>);

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
