/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_METADATA_XML_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_METADATA_XML_HH 1

#include <paludis/util/attributes.hh>
#include <paludis/util/named_value.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/sequence-fwd.hh>
#include <paludis/util/map-fwd.hh>
#include <paludis/util/fs_entry-fwd.hh>
#include <paludis/choice-fwd.hh>
#include <tr1/memory>

namespace paludis
{
    namespace n
    {
        typedef Name<struct herds_name> herds;
        typedef Name<struct long_description_name> long_description;
        typedef Name<struct maintainers_name> maintainers;
        typedef Name<struct uses_name> uses;
    }

    namespace erepository
    {
        struct MetadataXML
        {
            NamedValue<n::herds, std::tr1::shared_ptr<Sequence<std::string> > > herds;
            NamedValue<n::long_description, std::string> long_description;
            NamedValue<n::maintainers, std::tr1::shared_ptr<Sequence<std::string> > > maintainers;
            NamedValue<n::uses, std::tr1::shared_ptr<Map<ChoiceNameWithPrefix, std::string> > > uses;
        };

        class PALUDIS_VISIBLE MetadataXMLPool :
            private PrivateImplementationPattern<MetadataXMLPool>,
            public InstantiationPolicy<MetadataXMLPool, instantiation_method::SingletonTag>
        {
            friend class InstantiationPolicy<MetadataXMLPool, instantiation_method::SingletonTag>;

            private:
                MetadataXMLPool();
                ~MetadataXMLPool();

            public:
                const std::tr1::shared_ptr<const MetadataXML> metadata_if_exists(const FSEntry &) const PALUDIS_ATTRIBUTE((warn_unused_result));
        };
    }

#ifdef PALUDIS_HAVE_EXTERN_TEMPLATE
    extern template class PrivateImplementationPattern<erepository::MetadataXMLPool>;
    extern template class InstantiationPolicy<erepository::MetadataXMLPool, instantiation_method::SingletonTag>;
#endif
}

#endif
