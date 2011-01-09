/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_FAKE_REPOSITORY_BASE_HH
#define PALUDIS_GUARD_PALUDIS_FAKE_REPOSITORY_BASE_HH 1

#include <paludis/repository.hh>
#include <paludis/action-fwd.hh>
#include <paludis/util/pimp.hh>

/** \file
 * Declarations for the FakeRepositoryBase class.
 *
 * \ingroup grpfakerepository
 */

namespace paludis
{
    class FakePackageID;

    /**
     * A FakeRepositoryBase is a Repository subclass whose subclasses are used for
     * various test cases.
     *
     * \see FakeRepository
     * \see FakeInstalledRepository
     * \ingroup grpfakerepository
     */
    class PALUDIS_VISIBLE FakeRepositoryBase :
        public Repository,
        public std::enable_shared_from_this<FakeRepositoryBase>
    {
        private:
            Pimp<FakeRepositoryBase> _imp;

        protected:
            /**
             * Constructor.
             */
            FakeRepositoryBase(const Environment * const env, const RepositoryName & name,
                    const RepositoryCapabilities & caps);

            virtual void need_keys_added() const;

        public:
            /**
             * Destructor.
             */
            ~FakeRepositoryBase();

            /**
             * Add a category.
             */
            void add_category(const CategoryNamePart &);

            /**
             * Add a package, and a category if necessary.
             */
            void add_package(const QualifiedPackageName &);

            /**
             * Add a version, and a package and category if necessary, and set some
             * default values for its metadata, and return said metadata.
             */
            std::shared_ptr<FakePackageID> add_version(const QualifiedPackageName &, const VersionSpec &);

            /**
             * Add a version, and a package and category if necessary, and set some
             * default values for its metadata, and return said metadata (convenience
             * overload taking strings).
             */
            std::shared_ptr<FakePackageID> add_version(const std::string & c, const std::string & p,
                    const std::string & v);

            virtual void invalidate();

            virtual void invalidate_masks();

            /**
             * Fetch our associated environment.
             */
            const Environment * environment() const;

            /* Repository */

            virtual std::shared_ptr<const PackageIDSequence> package_ids(
                    const QualifiedPackageName &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::shared_ptr<const QualifiedPackageNameSet> package_names(
                    const CategoryNamePart &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::shared_ptr<const CategoryNamePartSet> category_names() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool has_package_named(const QualifiedPackageName &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool has_category_named(const CategoryNamePart &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool sync(const std::string &, const std::shared_ptr<OutputManager> &) const;

            ///\name Set methods
            ///\{

            virtual void populate_sets() const;

            ///\}

            virtual HookResult perform_hook(const Hook & hook, const std::shared_ptr<OutputManager> &);
    };
}


#endif
