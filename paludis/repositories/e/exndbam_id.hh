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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_EXNDBAM_ID_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_EXNDBAM_ID_HH 1

#include <paludis/repositories/e/e_installed_repository_id.hh>
#include <paludis/ndbam-fwd.hh>

namespace paludis
{
    namespace erepository
    {
        class ExndbamID :
            public EInstalledRepositoryID
        {
            private:
                const NDBAM * const _ndbam;

            public:
                ExndbamID(const QualifiedPackageName &, const VersionSpec &,
                        const Environment * const,
                        const RepositoryName &,
                        const FSPath & file,
                        const NDBAM * const);

                virtual std::string fs_location_raw_name() const;
                virtual std::string fs_location_human_name() const;
                virtual std::string contents_filename() const;
                virtual std::shared_ptr<MetadataValueKey<std::shared_ptr<const Contents> > > make_contents_key() const;
        };
    }
}

#endif
