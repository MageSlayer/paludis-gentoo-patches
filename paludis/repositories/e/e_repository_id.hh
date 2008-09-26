/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008 Ciaran McCreesh
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
#include <paludis/repositories/e/eapi-fwd.hh>

namespace paludis
{
    namespace erepository
    {
        class ERepositoryID :
            public PackageID
        {
            public:
                virtual const std::tr1::shared_ptr<const EAPI> eapi() const PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
                virtual const std::tr1::shared_ptr<const MetadataCollectionKey<Set<std::string> > > inherited_key() const = 0;
                virtual const std::tr1::shared_ptr<const MetadataSpecTreeKey<LicenseSpecTree> > license_key() const = 0;
                virtual const std::tr1::shared_ptr<const MetadataSpecTreeKey<PlainTextSpecTree> > restrict_key() const = 0;
                virtual const std::tr1::shared_ptr<const MetadataSpecTreeKey<PlainTextSpecTree> > properties_key() const = 0;
                virtual const std::tr1::shared_ptr<const MetadataCollectionKey<UseFlagNameSet> > use_key() const = 0;

                virtual std::tr1::shared_ptr<const Set<std::string> > breaks_portage() const PALUDIS_ATTRIBUTE((warn_unused_result));
        };
    }
}

#endif
