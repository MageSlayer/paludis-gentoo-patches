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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_UNWRITTEN_UNWRITTEN_REPOSITORY_STORE_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_UNWRITTEN_UNWRITTEN_REPOSITORY_STORE_HH 1

#include <paludis/util/pimp.hh>
#include <paludis/util/fs_entry-fwd.hh>
#include <paludis/name-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/environment-fwd.hh>
#include <memory>

namespace paludis
{
    namespace unwritten_repository
    {
        struct UnwrittenRepository;

        class PALUDIS_VISIBLE UnwrittenRepositoryStore :
            private Pimp<UnwrittenRepositoryStore>
        {
            private:
                void _populate_one(const Environment * const env, const FSEntry & f);
                void _populate(const Environment * const env, const FSEntry & f);

            public:
                UnwrittenRepositoryStore(
                        const Environment * const,
                        const UnwrittenRepository * const,
                        const FSEntry &);
                ~UnwrittenRepositoryStore();

                bool has_category_named(const CategoryNamePart &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                bool has_package_named(const QualifiedPackageName & q) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                std::shared_ptr<const CategoryNamePartSet> category_names() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                std::shared_ptr<const CategoryNamePartSet> unimportant_category_names() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                std::shared_ptr<const QualifiedPackageNameSet> package_names(
                        const CategoryNamePart & c) const PALUDIS_ATTRIBUTE((warn_unused_result));

                std::shared_ptr<const PackageIDSequence> package_ids(
                        const QualifiedPackageName & p) const PALUDIS_ATTRIBUTE((warn_unused_result));
        };
    }

    extern template class Pimp<unwritten_repository::UnwrittenRepositoryStore>;
}


#endif
