/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#ifndef PALUDIS_GUARD_PALUDIS_VERSION_METADATA_HH
#define PALUDIS_GUARD_PALUDIS_VERSION_METADATA_HH 1

#include <paludis/name.hh>
#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/sr.hh>
#include <paludis/dep_atom.hh>
#include <paludis/package_database_entry.hh>
#include <string>

/** \file
 * Declarations for the VersionMetadata class.
 *
 * \ingroup grpversions
 */

namespace paludis
{
    /**
     * A pointer to a parse function.
     *
     * \ingroup grpversions
     */
    typedef DepAtom::ConstPointer (* ParserFunction) (const std::string &);

#include <paludis/version_metadata-sr.hh>

    /**
     * Version metadata.
     *
     * \ingroup grpversions
     */
    class VersionMetadata :
        private InstantiationPolicy<VersionMetadata, instantiation_method::NonCopyableTag>,
        public VersionMetadataBase,
        public InternalCounted<VersionMetadata>
    {
        private:
            CRANVersionMetadata * _cran_if;
            EbuildVersionMetadata * _ebuild_if;
            EbinVersionMetadata * _ebin_if;
            VirtualMetadata * _virtual_if;

        protected:
            /**
             * Constructor.
             */
            VersionMetadata(ParserFunction, CRANVersionMetadata * cran_if,
                    EbuildVersionMetadata * ebuild_if,
                    EbinVersionMetadata * ebin_if,
                    VirtualMetadata * virtual_if);

        public:
            /**
             * Constructor.
             */
            VersionMetadata(ParserFunction);

            /**
             * Destructor.
             */
            virtual ~VersionMetadata();

            /**
             * Fetch out CRAN interface, or 0.
             */
            CRANVersionMetadata *
            get_cran_interface()
            {
                return _cran_if;
            }

            /**
             * Fetch out CRAN interface, or 0.
             */
            const CRANVersionMetadata *
            get_cran_interface() const
            {
                return _cran_if;
            }

            class CRAN;

            /**
             * Fetch our ebuild interface, or 0.
             */
            EbuildVersionMetadata *
            get_ebuild_interface()
            {
                return _ebuild_if;
            }

            /**
             * Fetch our ebuild interface, or 0.
             */
            const EbuildVersionMetadata *
            get_ebuild_interface() const
            {
                return _ebuild_if;
            }

            class Ebuild;

            /**
             * Fetch our ebin interface, or 0.
             */
            EbinVersionMetadata *
            get_ebin_interface()
            {
                return _ebin_if;
            }

            /**
             * Fetch our ebin interface, or 0.
             */
            const EbinVersionMetadata *
            get_ebin_interface() const
            {
                return _ebin_if;
            }

            class Ebin;

            /**
             * Fetch our virtual interface, or 0.
             */
            VirtualMetadata *
            get_virtual_interface()
            {
                return _virtual_if;
            }

            /**
             * Fetch our virtual interface, or 0.
             */
            const VirtualMetadata *
            get_virtual_interface() const
            {
                return _virtual_if;
            }

            class Virtual;

            /**
             * Fetch our licence, as a dep atom structure.
             */
            DepAtom::ConstPointer license() const;
    };

    /**
     * VersionMetadata subclass, for CRAN repositories.
     *
     * \ingroup grpversions
     * \see VersionMetadata
     */
    class VersionMetadata::CRAN :
        public VersionMetadata
    {
        private:
            CRANVersionMetadata _c;

        public:
            /**
             * Constructor.
             */
            CRAN(ParserFunction);

            /**
             * Pointer to us.
             */
            typedef CountedPtr<VersionMetadata::CRAN, count_policy::InternalCountTag> Pointer;

            /**
             * Const pointer to us.
             */
            typedef CountedPtr<const VersionMetadata::Ebuild, count_policy::InternalCountTag> ConstPointer;
    };

   /**
     * VersionMetadata subclass, for ebuilds.
     *
     * \ingroup grpversions
     * \see VersionMetadata
     */
    class VersionMetadata::Ebuild :
        public VersionMetadata
    {
        private:
            EbuildVersionMetadata _e;

        public:
            /**
             * Constructor.
             */
            Ebuild(ParserFunction);

            /**
             * Pointer to us.
             */
            typedef CountedPtr<VersionMetadata::Ebuild, count_policy::InternalCountTag> Pointer;

            /**
             * Const pointer to us.
             */
            typedef CountedPtr<const VersionMetadata::Ebuild, count_policy::InternalCountTag> ConstPointer;
    };

    /**
     * VersionMetadata subclass, for ebins.
     *
     * \ingroup grpversions
     * \see VersionMetadata
     */
    class VersionMetadata::Ebin :
        public VersionMetadata
    {
        private:
            EbuildVersionMetadata _e;
            EbinVersionMetadata _eb;

        public:
            /**
             * Constructor.
             */
            Ebin(ParserFunction);

            /**
             * Pointer to us.
             */
            typedef CountedPtr<VersionMetadata::Ebin, count_policy::InternalCountTag> Pointer;

            /**
             * Const pointer to us.
             */
            typedef CountedPtr<const VersionMetadata::Ebin, count_policy::InternalCountTag> ConstPointer;
    };

    /**
     * VersionMetadata subclass, for virtuals.
     *
     * \ingroup grpversions
     * \see VersionMetadata
     */
    class VersionMetadata::Virtual :
        public VersionMetadata
    {
        private:
            VirtualMetadata _v;

        public:
            /**
             * Constructor.
             */
            Virtual(ParserFunction, const PackageDatabaseEntry &);

            /**
             * Pointer to us.
             */
            typedef CountedPtr<VersionMetadata::Virtual, count_policy::InternalCountTag> Pointer;

            /**
             * Const pointer to us.
             */
            typedef CountedPtr<const VersionMetadata::Virtual, count_policy::InternalCountTag> ConstPointer;
    };
}

#endif
