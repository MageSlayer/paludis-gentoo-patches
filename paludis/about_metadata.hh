/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_ABOUT_METADATA_HH
#define PALUDIS_GUARD_PALUDIS_ABOUT_METADATA_HH 1

#include <paludis/about_metadata-fwd.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/instantiation_policy.hh>
#include <paludis/metadata_key_holder.hh>

/** \file
 * Provides information about Paludis, similar to that in "paludis/about.hh", in
 * the form of MetadataKey values.
 *
 * \ingroup g_about
 */

namespace paludis
{
    /**
     * Information about Paludis, provided as MetadataKey instances.
     *
     * \ingroup g_about
     * \since 0.46
     */
    class PALUDIS_VISIBLE AboutMetadata :
        private PrivateImplementationPattern<AboutMetadata>,
        public InstantiationPolicy<AboutMetadata, instantiation_method::SingletonTag>,
        public MetadataKeyHolder
    {
        friend class InstantiationPolicy<AboutMetadata, instantiation_method::SingletonTag>;

        private:
            PrivateImplementationPattern<AboutMetadata>::ImpPtr & _imp;

            AboutMetadata();
            ~AboutMetadata();

        protected:
            void need_keys_added() const;
    };

#ifdef PALUDIS_HAVE_EXTERN_TEMPLATE
    extern template class PrivateImplementationPattern<AboutMetadata>;
    extern template class InstantiationPolicy<AboutMetadata, instantiation_method::SingletonTag>;
#endif
}

#endif
