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
    typedef DepAtom::ConstPointer (* ParserFunction) (const std::string &);

    enum VersionMetadataDepsKey
    {
        vmd_parser,
        vmd_build_depend_string,
        vmd_run_depend_string,
        vmd_post_depend_string,
        last_vmd
    };

    struct VersionMetadataDepsTag :
        SmartRecordTag<comparison_mode::NoComparisonTag, void>,
        SmartRecordKeys<VersionMetadataDepsKey, last_vmd>,
        SmartRecordKey<vmd_parser, ParserFunction>,
        SmartRecordKey<vmd_build_depend_string, std::string>,
        SmartRecordKey<vmd_run_depend_string, std::string>,
        SmartRecordKey<vmd_post_depend_string, std::string>
    {
    };

    class VersionMetadataDeps :
        public MakeSmartRecord<VersionMetadataDepsTag>::Type
    {
        public:
            VersionMetadataDeps(ParserFunction);

            DepAtom::ConstPointer build_depend() const;
            DepAtom::ConstPointer run_depend() const;
            DepAtom::ConstPointer post_depend() const;
    };

    enum VersionMetadataKey
    {
        vm_deps,
        vm_slot,
        vm_license,
        vm_eapi,
        vm_homepage,
        vm_description,
        last_vm
    };

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

    enum EbuildVersionMetadataKey
    {
        evm_provide,
        evm_src_uri,
        evm_restrict,
        evm_keywords,
        evm_iuse,
        evm_virtual,
        evm_inherited,
        last_evm
    };

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

    class EbuildVersionMetadata :
        public MakeSmartRecord<EbuildVersionMetadataTag>::Type
    {
        public:
            EbuildVersionMetadata();

            DepAtom::ConstPointer provide() const;
    };

    class VersionMetadata :
        private InstantiationPolicy<VersionMetadata, instantiation_method::NonCopyableTag>,
        public MakeSmartRecord<VersionMetadataTag>::Type,
        public InternalCounted<VersionMetadata>
    {
        private:
            EbuildVersionMetadata * _ebuild_if;

        protected:
            VersionMetadata(ParserFunction, EbuildVersionMetadata * ebuild_if);

        public:
            VersionMetadata(ParserFunction);

            virtual ~VersionMetadata();

            EbuildVersionMetadata *
            get_ebuild_interface()
            {
                return _ebuild_if;
            }

            const EbuildVersionMetadata *
            get_ebuild_interface() const
            {
                return _ebuild_if;
            }

            DepAtom::ConstPointer license() const;

            class Ebuild;
    };

    class VersionMetadata::Ebuild :
        public VersionMetadata
    {
        private:
            EbuildVersionMetadata _e;

        public:
            Ebuild(ParserFunction);

            typedef CountedPtr<VersionMetadata::Ebuild, count_policy::InternalCountTag> Pointer;
            typedef CountedPtr<const VersionMetadata::Ebuild, count_policy::InternalCountTag> ConstPointer;
    };
}

#endif
