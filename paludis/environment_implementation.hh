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

#ifndef PALUDIS_GUARD_PALUDIS_ENVIRONMENT_IMPLEMENTATION_HH
#define PALUDIS_GUARD_PALUDIS_ENVIRONMENT_IMPLEMENTATION_HH 1

#include <paludis/environment.hh>
#include <paludis/package_id-fwd.hh>

/** \file
 * Declarations for the Environment class.
 *
 * \ingroup g_environment
 *
 * \section Examples
 *
 * - None at this time. The EnvironmentImplementation class is of no relevance
 *   to clients.
 */

namespace paludis
{
    /**
     * Simplifies implementing the Environment interface.
     *
     * Most Environment subclasses derive from this class, rather than directly
     * from the abstract base. It provides default implementations for many
     * methods.
     *
     * \ingroup g_environment
     * \see Environment
     */
    class PALUDIS_VISIBLE EnvironmentImplementation :
        public Environment
    {
        private:
            Pimp<EnvironmentImplementation> _imp;

            void _need_sets() const;

        protected:
            virtual void populate_sets() const = 0;

            virtual void populate_standard_sets() const;
            void set_always_exists(const SetName &) const;

        public:
            ///\name Basic operations
            ///\{

            EnvironmentImplementation();
            ~EnvironmentImplementation() override = 0;

            ///\}

            std::shared_ptr<const FSPathSequence> bashrc_files() const
                override PALUDIS_ATTRIBUTE((warn_unused_result));

            std::shared_ptr<const FSPathSequence> syncers_dirs() const
                override PALUDIS_ATTRIBUTE((warn_unused_result));

            std::shared_ptr<const FSPathSequence> fetchers_dirs() const
                override PALUDIS_ATTRIBUTE((warn_unused_result));

            std::string distribution() const
                override PALUDIS_ATTRIBUTE((warn_unused_result));

            bool is_paludis_package(const QualifiedPackageName &) const
                override PALUDIS_ATTRIBUTE((warn_unused_result));

            std::shared_ptr<PackageIDSequence> operator[] (const Selection &) const
                override PALUDIS_ATTRIBUTE((warn_unused_result));

            NotifierCallbackID add_notifier_callback(const NotifierCallbackFunction &) override;

            void remove_notifier_callback(const NotifierCallbackID) override;

            void trigger_notifier_callback(const NotifierCallbackEvent &) const override;

            void add_set(
                    const SetName &,
                    const SetName &,
                    const std::function<std::shared_ptr<const SetSpecTree> ()> &,
                    const bool combine) const override;

            std::shared_ptr<const SetNameSet> set_names() const
                override PALUDIS_ATTRIBUTE((warn_unused_result));

            const std::shared_ptr<const SetSpecTree> set(const SetName &) const
                override PALUDIS_ATTRIBUTE((warn_unused_result));

            void add_repository(int importance, const std::shared_ptr<Repository> &) override;

            const std::shared_ptr<const Repository> fetch_repository(const RepositoryName &) const
                override PALUDIS_ATTRIBUTE((warn_unused_result));

            const std::shared_ptr<Repository> fetch_repository(const RepositoryName &)
                override PALUDIS_ATTRIBUTE((warn_unused_result));

            bool has_repository_named(const RepositoryName &) const
                override PALUDIS_ATTRIBUTE((warn_unused_result));

            QualifiedPackageName fetch_unique_qualified_package_name(
                    const PackageNamePart &, const Filter & = all_filter(), const bool disambiguate = true) const
                override PALUDIS_ATTRIBUTE((warn_unused_result));

            bool more_important_than(const RepositoryName &, const RepositoryName &) const
                override PALUDIS_ATTRIBUTE((warn_unused_result));

            RepositoryConstIterator begin_repositories() const
                override PALUDIS_ATTRIBUTE((warn_unused_result));

            RepositoryConstIterator end_repositories() const
                override PALUDIS_ATTRIBUTE((warn_unused_result));

            const std::shared_ptr<const Set<std::string> > expand_licence(
                    const std::string &) const override;
    };

    class PALUDIS_VISIBLE DuplicateSetError :
        public Exception
    {
        public:
            DuplicateSetError(const SetName & s) noexcept;
    };
}

#endif
