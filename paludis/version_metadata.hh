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
#include <set>
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
            EbuildVersionMetadata * _ebuild_if;

        protected:
            /**
             * Constructor.
             */
            VersionMetadata(ParserFunction, EbuildVersionMetadata * ebuild_if);

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

            /**
             * Fetch our licence, as a dep atom structure.
             */
            DepAtom::ConstPointer license() const;

            class Ebuild;
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
}

#endif
