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
#include <paludis/util/smart_record.hh>
#include <paludis/dep_atom.hh>
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

    /**
     * Keys for VersionMetadataDeps.
     *
     * \see VersionMetadataDeps
     * \ingroup grpversions
     */
    enum VersionMetadataDepsKey
    {
        vmd_parser,                 ///< Our parser
        vmd_build_depend_string,    ///< Our build deps
        vmd_run_depend_string,      ///< Our runtime deps
        vmd_post_depend_string,     ///< Our post deps
        last_vmd                    ///< Number of items
    };

    /**
     * Tag for VersionMetadataDeps.
     *
     * \see VersionMetadataDeps
     * \ingroup grpversions
     */
    struct VersionMetadataDepsTag :
        SmartRecordTag<comparison_mode::NoComparisonTag, void>,
        SmartRecordKeys<VersionMetadataDepsKey, last_vmd>,
        SmartRecordKey<vmd_parser, ParserFunction>,
        SmartRecordKey<vmd_build_depend_string, std::string>,
        SmartRecordKey<vmd_run_depend_string, std::string>,
        SmartRecordKey<vmd_post_depend_string, std::string>
    {
    };

    /**
     * Version metadata dependency information.
     *
     * \see VersionMetadata
     * \ingroup grpversions
     */
    class VersionMetadataDeps :
        public MakeSmartRecord<VersionMetadataDepsTag>::Type
    {
        public:
            /**
             * Constructor.
             */
            VersionMetadataDeps(ParserFunction);

            /**
             * Our build depends.
             */
            DepAtom::ConstPointer build_depend() const;

            /**
             * Our runtime depends.
             */
            DepAtom::ConstPointer run_depend() const;

            /**
             * Our post depends.
             */
            DepAtom::ConstPointer post_depend() const;
    };

    /**
     * Key for VersionMetadata.
     *
     * \see VersionMetadata
     * \ingroup grpversions
     */
    enum VersionMetadataKey
    {
        vm_deps,         ///< Dependencies
        vm_slot,         ///< Slot
        vm_license,      ///< Licence
        vm_eapi,         ///< EAPI
        vm_homepage,     ///< Homepage
        vm_description,  ///< Description
        last_vm          ///< Number of items
    };

    /**
     * Tag for VersionMetadata.
     *
     * \see VersionMetadata
     * \ingroup grpversions
     */
    struct VersionMetadataTag :
        SmartRecordTag<comparison_mode::NoComparisonTag, void>,
        SmartRecordKeys<VersionMetadataKey, last_vm>,
        SmartRecordKey<vm_deps, VersionMetadataDeps>,
        SmartRecordKey<vm_slot, SlotName>,
        SmartRecordKey<vm_license, std::string>,
        SmartRecordKey<vm_eapi, std::string>,
        SmartRecordKey<vm_homepage, std::string>,
        SmartRecordKey<vm_description, std::string>
    {
    };

    /**
     * Key for EbuildVersionMetadata.
     *
     * \see EbuildVersionMetadata
     * \ingroup grpversions
     */
    enum EbuildVersionMetadataKey
    {
        evm_provide,         ///< PROVIDE
        evm_src_uri,         ///< SRC_URI
        evm_restrict,        ///< RESTRICT
        evm_keywords,        ///< KEYWORDS
        evm_iuse,            ///< IUSE
        evm_virtual,         ///< virtual for what?
        evm_inherited,       ///< INHERITED
        last_evm             ///< number of items
    };

    /**
     * Tag for EbuildVersionMetadata.
     *
     * \see EbuildVersionMetadata
     * \ingroup grpversions
     */
    struct EbuildVersionMetadataTag :
        SmartRecordTag<comparison_mode::NoComparisonTag, void>,
        SmartRecordKeys<EbuildVersionMetadataKey, last_evm>,
        SmartRecordKey<evm_provide, std::string>,
        SmartRecordKey<evm_src_uri, std::string>,
        SmartRecordKey<evm_restrict, std::string>,
        SmartRecordKey<evm_keywords, std::string>,
        SmartRecordKey<evm_iuse, std::string>,
        SmartRecordKey<evm_virtual, std::string>,
        SmartRecordKey<evm_inherited, std::string>
    {
    };

    /**
     * Version metadata for an ebuild.
     *
     * \ingroup grpversions
     * \see VersionMetadata
     */
    class EbuildVersionMetadata :
        public MakeSmartRecord<EbuildVersionMetadataTag>::Type
    {
        public:
            /**
             * Constructor.
             */
            EbuildVersionMetadata();
            /**
             * PROVIDE, as a dep atom.
             */
            DepAtom::ConstPointer provide() const;
    };

    /**
     * Key for CRANVersionMetadata.
     *
     * \see CRANVersionMetadata
     * \ingroup grpversions
     */
    enum CRANVersionMetadataKey
    {
        cranvm_keywords,
        cranvm_package,
        cranvm_version,
        cranvm_is_bundle,
        last_cranvm
    };

    /**
     * Tag for CRANVersionMetadata.
     *
     * \see CRANVersionMetadata
     * \ingroup grpversions
     */
    struct CRANVersionMetadataTag :
        SmartRecordTag<comparison_mode::NoComparisonTag, void>,
        SmartRecordKeys<CRANVersionMetadataKey, last_cranvm>,
        SmartRecordKey<cranvm_keywords, std::string>,
        SmartRecordKey<cranvm_package, std::string>,
        SmartRecordKey<cranvm_version, std::string>,
        SmartRecordKey<cranvm_is_bundle, bool>
    {
    };

    /**
     * Version metadata for a CRAN package.
     *
     * \ingroup grpversions
     * \see VersionMetadata
     */
    typedef MakeSmartRecord<CRANVersionMetadataTag>::Type CRANVersionMetadata;

    /**
     * Key for EbinVersionMetadata.
     *
     * \see EbinVersionMetadata
     * \ingroup grpversions
     */
    enum EbinVersionMetadataKey
    {
        ebvm_bin_uri,         ///< BIN_URI
        ebvm_src_repository,  ///< SRC_REPOSITORY
        last_ebvm             ///< number of items
    };

    /**
     * Tag for EbinVersionMetadata.
     *
     * \see EbinVersionMetadata
     * \ingroup grpversions
     */
    struct EbinVersionMetadataTag :
        SmartRecordTag<comparison_mode::NoComparisonTag, void>,
        SmartRecordKeys<EbinVersionMetadataKey, last_ebvm>,
        SmartRecordKey<ebvm_bin_uri, std::string>,
        SmartRecordKey<ebvm_src_repository, RepositoryName>
    {
    };

    /**
     * Version metadata for an ebin.
     *
     * \ingroup grpversions
     * \see VersionMetadata
     */
    class EbinVersionMetadata :
        public MakeSmartRecord<EbinVersionMetadataTag>::Type
    {
        public:
            /**
             * Constructor.
             */
            EbinVersionMetadata();
    };

    /**
     * Version metadata.
     *
     * \ingroup grpversions
     */
    class VersionMetadata :
        private InstantiationPolicy<VersionMetadata, instantiation_method::NonCopyableTag>,
        public MakeSmartRecord<VersionMetadataTag>::Type,
        public InternalCounted<VersionMetadata>
    {
        private:
            CRANVersionMetadata * _cran_if;
            EbuildVersionMetadata * _ebuild_if;
            EbinVersionMetadata * _ebin_if;

        protected:
            /**
             * Constructor.
             */
            VersionMetadata(ParserFunction, CRANVersionMetadata * cran_if,
                    EbuildVersionMetadata * ebuild_if,
                    EbinVersionMetadata * ebin_if);

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

            /**
             * Fetch our licence, as a dep atom structure.
             */
            DepAtom::ConstPointer license() const;
            class Ebin;
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
}

#endif
