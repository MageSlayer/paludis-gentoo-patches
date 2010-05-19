/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010 Ciaran McCreesh
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
#include <paludis/util/simple_visitor.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/sequence-fwd.hh>
#include <paludis/util/named_value.hh>
#include <paludis/util/type_list.hh>
#include <string>

/** \file
 * Declarations for mask classes.
 *
 * \ingroup g_mask
 *
 * \section Examples
 *
 * - \ref example_mask.cc "example_mask.cc" (for masks)
 */

namespace paludis
{
    namespace n
    {
        typedef Name<struct comment_name> comment;
        typedef Name<struct mask_name> mask;
        typedef Name<struct mask_file_name> mask_file;
        typedef Name<struct override_reason_name> override_reason;
    }

    /**
     * Information about a RepositoryMask.
     *
     * The mask_file key holds the file whence the mask originates.
     *
     * The comment key is a sequence of lines explaining the mask.
     *
     * \ingroup g_package_id
     * \since 0.30
     * \nosubgrouping
     */
    struct RepositoryMaskInfo
    {
        NamedValue<n::comment, std::tr1::shared_ptr<const Sequence<std::string> > > comment;
        NamedValue<n::mask_file, FSEntry> mask_file;
    };

    /**
     * A Mask represents one reason why a PackageID is masked (not available to
     * be installed).
     *
     * A basic Mask has:
     *
     * - A single character key, which can be used by clients if they need a
     *   very compact way of representing a mask.
     *
     * - A description.
     *
     * Subclasses provide additional information.
     *
     * \ingroup g_mask
     * \since 0.26
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE Mask :
        public virtual DeclareAbstractAcceptMethods<Mask, MakeTypeList<
            UserMask, UnacceptedMask, RepositoryMask, UnsupportedMask, AssociationMask>::Type>
    {
        public:
            ///\name Basic operations
            ///\{

            virtual ~Mask() = 0;

            ///\}

            /**
             * A single character key, which can be used by clients if they need
             * a very compact way of representing a mask.
             */
            virtual char key() const = 0;

            /**
             * A description of the mask.
             */
            virtual const std::string description() const = 0;
    };

    /**
     * A UserMask is a Mask due to user configuration.
     *
     * \ingroup g_mask
     * \since 0.26
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE UserMask :
        public Mask,
        public ImplementAcceptMethods<Mask, UserMask>
    {
    };

    /**
     * An UnacceptedMask is a Mask that signifies that a particular value or
     * combination of values in (for example) a MetadataCollectionKey or
     * MetadataSpecTreeKey is not accepted by user configuration.
     *
     * \ingroup g_mask
     * \since 0.26
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE UnacceptedMask :
        public Mask,
        public ImplementAcceptMethods<Mask, UnacceptedMask>
    {
        public:
            /**
             * Fetch the metadata key that is not accepted.
             */
            virtual const std::tr1::shared_ptr<const MetadataKey> unaccepted_key() const = 0;
    };

    /**
     * A RepositoryMask is a Mask that signifies that a PackageID has been
     * marked as masked by a Repository.
     *
     * \ingroup g_mask
     * \since 0.26
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE RepositoryMask :
        public Mask,
        public ImplementAcceptMethods<Mask, RepositoryMask>
    {
        public:
            /**
             * Fetch a metadata key explaining the mask. May return a zero
             * pointer, if no more information is available.
             */
            virtual const std::tr1::shared_ptr<const MetadataKey> mask_key() const = 0;
    };

    /**
     * An UnsupportedMask is a Mask that signifies that a PackageID is not
     * supported, for example because it is broken or because it uses an
     * unrecognised EAPI.
     *
     * \ingroup g_mask
     * \since 0.26
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE UnsupportedMask :
        public Mask,
        public ImplementAcceptMethods<Mask, UnsupportedMask>
    {
        public:
            /**
             * An explanation of why we are unsupported.
             */
            virtual const std::string explanation() const = 0;
    };

    /**
     * An AssociationMask is a Mask that signifies that a PackageID is masked
     * because of its association with another PackageID that is itself masked.
     *
     * This is used by old-style virtuals. If the provider of a virtual is
     * masked then the virtual itself is masked by association.
     *
     * \ingroup g_mask
     * \since 0.26
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE AssociationMask :
        public Mask,
        public ImplementAcceptMethods<Mask, AssociationMask>
    {
        public:
            /**
             * Fetch the associated package.
             */
            virtual const std::tr1::shared_ptr<const PackageID> associated_package() const = 0;
    };

    /**
     * An OverriddenMask holds a Mask and an explanation of why it has been overridden.
     *
     * \ingroup g_mask
     * \since 0.34
     */
    struct OverriddenMask
    {
        NamedValue<n::mask, std::tr1::shared_ptr<const Mask> > mask;
        NamedValue<n::override_reason, MaskOverrideReason> override_reason;

    };
}

#endif
