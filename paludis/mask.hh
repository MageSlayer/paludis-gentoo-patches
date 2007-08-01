/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_MASK_HH
#define PALUDIS_GUARD_PALUDIS_MASK_HH 1

#include <paludis/mask-fwd.hh>
#include <paludis/metadata_key-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/util/visitor.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/sequence-fwd.hh>
#include <string>

namespace paludis
{

#include <paludis/mask-sr.hh>

    struct MaskVisitorTypes :
        VisitorTypes<
            MaskVisitorTypes,
            Mask,
            UserMask,
            UnacceptedMask,
            RepositoryMask,
            UnsupportedMask,
            AssociationMask
        >
    {
    };

    class PALUDIS_VISIBLE Mask :
        public virtual ConstAcceptInterface<MaskVisitorTypes>
    {
        public:
            virtual ~Mask() = 0;

            virtual const char key() const = 0;
            virtual const std::string description() const = 0;
    };

    class PALUDIS_VISIBLE UserMask :
        public Mask,
        public ConstAcceptInterfaceVisitsThis<MaskVisitorTypes, UserMask>
    {
    };

    class PALUDIS_VISIBLE UnacceptedMask :
        public Mask,
        public ConstAcceptInterfaceVisitsThis<MaskVisitorTypes, UnacceptedMask>
    {
        public:
            virtual const tr1::shared_ptr<const MetadataKey> unaccepted_key() const = 0;
    };

    class PALUDIS_VISIBLE RepositoryMask :
        public Mask,
        public ConstAcceptInterfaceVisitsThis<MaskVisitorTypes, RepositoryMask>
    {
        public:
            virtual const tr1::shared_ptr<const MetadataKey> mask_key() const = 0;
    };

    class PALUDIS_VISIBLE UnsupportedMask :
        public Mask,
        public ConstAcceptInterfaceVisitsThis<MaskVisitorTypes, UnsupportedMask>
    {
        public:
            virtual const std::string explanation() const = 0;
    };

    class PALUDIS_VISIBLE AssociationMask :
        public Mask,
        public ConstAcceptInterfaceVisitsThis<MaskVisitorTypes, AssociationMask>
    {
        public:
            virtual const tr1::shared_ptr<const PackageID> associated_package() const = 0;
    };
}

#endif
