/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_METADATA_KEY_HOLDER_HH
#define PALUDIS_GUARD_PALUDIS_METADATA_KEY_HOLDER_HH 1

#include <paludis/metadata_key_holder-fwd.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/metadata_key-fwd.hh>
#include <tr1/memory>

namespace paludis
{
    /**
     * Generic interface for any class that holds a number of MetadataKey
     * instances.
     *
     * \since 0.26
     * \ingroup g_metadata_key
     */
    class PALUDIS_VISIBLE MetadataKeyHolder :
        private PrivateImplementationPattern<MetadataKeyHolder>
    {
        protected:
            /**
             * Add a new MetadataKey, which must not use the same raw name as
             * any previous MetadataKey added to this ID.
             */
            virtual void add_metadata_key(const std::tr1::shared_ptr<const MetadataKey> &) const;

            /**
             * Clear all MetadataKey instances added using add_metadata_key.
             */
            virtual void clear_metadata_keys() const;

            /**
             * This method will be called before any of the metadata key
             * iteration methods does its work. It can be used by subclasses to
             * implement as-needed loading of keys.
             */
            virtual void need_keys_added() const = 0;

        public:
            MetadataKeyHolder();
            virtual ~MetadataKeyHolder() = 0;

            ///\name Finding and iterating over metadata keys
            ///\{

            struct MetadataConstIteratorTag;
            typedef WrappedForwardIterator<MetadataConstIteratorTag, const std::tr1::shared_ptr<const MetadataKey> > MetadataConstIterator;

            MetadataConstIterator begin_metadata() const PALUDIS_ATTRIBUTE((warn_unused_result));
            MetadataConstIterator end_metadata() const PALUDIS_ATTRIBUTE((warn_unused_result));
            MetadataConstIterator find_metadata(const std::string &) const PALUDIS_ATTRIBUTE((warn_unused_result));

            ///\}
    };

#ifdef PALUDIS_HAVE_EXTERN_TEMPLATE
    extern template class WrappedForwardIterator<MetadataKeyHolder::MetadataConstIteratorTag, const std::tr1::shared_ptr<const MetadataKey> >;
#endif
}

#endif
