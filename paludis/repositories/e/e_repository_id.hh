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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_E_REPOSITORY_ID_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_E_REPOSITORY_ID_HH 1

#include <paludis/package_id.hh>
#include <paludis/util/tribool.hh>
#include <paludis/repositories/e/eapi-fwd.hh>

namespace paludis
{
    namespace erepository
    {
        class ERepositoryID :
            public PackageID
        {
            public:
                virtual const std::shared_ptr<const EAPI> eapi() const PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
                virtual const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > > inherited_key() const = 0;
                virtual const std::shared_ptr<const MetadataSpecTreeKey<LicenseSpecTree> > license_key() const = 0;
                virtual const std::shared_ptr<const MetadataSpecTreeKey<PlainTextSpecTree> > restrict_key() const = 0;
                virtual const std::shared_ptr<const MetadataSpecTreeKey<PlainTextSpecTree> > properties_key() const = 0;
                virtual const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > > raw_use_key() const = 0;
                virtual const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > > raw_iuse_key() const = 0;
                virtual const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > > raw_iuse_effective_key() const = 0;
                virtual const std::shared_ptr<const MetadataSpecTreeKey<PlainTextSpecTree> > raw_myoptions_key() const = 0;
                virtual const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > > raw_use_expand_key() const = 0;
                virtual const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > > raw_use_expand_hidden_key() const = 0;
                virtual const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > > defined_phases_key() const = 0;
                virtual const std::shared_ptr<const MetadataSpecTreeKey<RequiredUseSpecTree> > required_use_key() const = 0;

                virtual std::shared_ptr<const Set<std::string> > breaks_portage() const PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual std::shared_ptr<ChoiceValue> make_choice_value(
                        const std::shared_ptr<const Choice> &, const UnprefixedChoiceName &, const Tribool,
                        const bool, const bool, const std::string &, const bool)
                    const PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

                virtual void add_build_options(const std::shared_ptr<Choices> &) const = 0;

                virtual void purge_invalid_cache() const = 0;
        };
    }
}

#endif
