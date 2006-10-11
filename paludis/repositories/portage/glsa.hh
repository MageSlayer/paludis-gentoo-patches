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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_PORTAGE_GLSA_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_PORTAGE_GLSA_HH 1

#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/counted_ptr.hh>
#include <paludis/util/sr.hh>
#include <paludis/name.hh>
#include <paludis/version_spec.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>

#include <string>

namespace paludis
{

#include <paludis/repositories/portage/glsa-sr.hh>

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

    class PALUDIS_VISIBLE GLSAPackage :
        private PrivateImplementationPattern<GLSAPackage>,
        public InternalCounted<GLSAPackage>
    {
        public:
            GLSAPackage(const QualifiedPackageName & name);
            ~GLSAPackage();

            typedef libwrapiter::ForwardIterator<GLSAPackage, const UseFlagName> ArchsIterator;
            ArchsIterator begin_archs() const;
            ArchsIterator end_archs() const;
            void add_arch(const UseFlagName &);

            typedef libwrapiter::ForwardIterator<GLSAPackage, const GLSARange> RangesIterator;
            RangesIterator begin_unaffected() const;
            RangesIterator end_unaffected() const;
            RangesIterator begin_vulnerable() const;
            RangesIterator end_vulnerable() const;
            void add_unaffected(const GLSARange &);
            void add_vulnerable(const GLSARange &);

            QualifiedPackageName name() const;
    };

    class PALUDIS_VISIBLE GLSA :
        private PrivateImplementationPattern<GLSA>,
        public InternalCounted<GLSA>
    {
        public:
            GLSA();
            ~GLSA();

            static GLSA::Pointer create_from_xml_file(const std::string & filename);

            typedef libwrapiter::ForwardIterator<GLSA, const GLSAPackage> PackagesIterator;
            PackagesIterator begin_packages() const;
            PackagesIterator end_packages() const;
            void add_package(GLSAPackage::ConstPointer);

            void set_id(const std::string &);
            std::string id() const;

            void set_title(const std::string &);
            std::string title() const;
    };
}

#endif
