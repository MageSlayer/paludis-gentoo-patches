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

#ifndef PALUDIS_GUARD_PALUDIS_PACKAGE_DEP_ATOM_HH
#define PALUDIS_GUARD_PALUDIS_PACKAGE_DEP_ATOM_HH 1

#include <paludis/dep_atom.hh>
#include <paludis/qualified_package_name.hh>
#include <paludis/version_operator.hh>
#include <paludis/slot_name.hh>
#include <paludis/version_spec.hh>
#include <paludis/dep_atom_visitor.hh>
#include <ostream>

/** \file
 * Declarations for the PackageDepAtom class.
 *
 * \ingroup DepResolver
 */

namespace paludis
{
    /**
     * A PackageDepAtom represents a package name (for example,
     * 'app-editors/vim'), possibly with associated version and SLOT
     * restrictions.
     *
     * \ingroup DepResolver
     */
    class PackageDepAtom :
        public DepAtom,
        public Visitable<PackageDepAtom, DepAtomVisitorTypes>
    {
        private:
            QualifiedPackageName _package;
            VersionOperator _version_operator;
            CountedPtr<VersionSpec, count_policy::ExternalCountTag> _version_spec;
            CountedPtr<SlotName, count_policy::ExternalCountTag> _slot;

        public:
            /**
             * Constructor, no version or SLOT restrictions.
             */
            PackageDepAtom(const QualifiedPackageName & package);

            /**
             * Constructor, with restrictions.
             */
            PackageDepAtom(const QualifiedPackageName & package,
                    VersionOperator version_operator,
                    CountedPtr<VersionSpec, count_policy::ExternalCountTag> version_spec,
                    CountedPtr<SlotName, count_policy::ExternalCountTag> slot);

            /**
             * Constructor, parse restrictions ourself.
             */
            PackageDepAtom(const std::string &);

            /**
             * Destructor.
             */
            ~PackageDepAtom();

            /**
             * Fetch the package name.
             */
            const QualifiedPackageName & package() const
            {
                return _package;
            }

            /**
             * Fetch the version operator.
             */
            const VersionOperator version_operator() const
            {
                return _version_operator;
            }

            /**
             * Fetch the version spec (may be a zero pointer).
             */
            CountedPtr<VersionSpec, count_policy::ExternalCountTag> version_spec_ptr() const
            {
                return _version_spec;
            }

            /**
             * Fetch the slot name (may be a zero pointer).
             */
            CountedPtr<SlotName, count_policy::ExternalCountTag> slot_ptr() const
            {
                return _slot;
            }

            typedef CountedPtr<PackageDepAtom, count_policy::InternalCountTag> Pointer;
            typedef CountedPtr<const PackageDepAtom, count_policy::InternalCountTag> ConstPointer;
    };

    class PackageDepAtomError :
        public Exception
    {
        public:
            PackageDepAtomError(const std::string & msg) throw ();
    };

    /**
     * A PackageDepAtom can be written to an ostream.
     */
    std::ostream & operator<< (std::ostream &, const PackageDepAtom &);
}

#endif
