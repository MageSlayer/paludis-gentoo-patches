/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORY_FACTORY_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORY_FACTORY_HH 1

#include <paludis/repository_factory-fwd.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/pimp.hh>
#include <paludis/util/singleton.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/no_type.hh>
#include <paludis/environment-fwd.hh>
#include <paludis/repository-fwd.hh>
#include <paludis/name-fwd.hh>
#include <functional>
#include <memory>

namespace paludis
{
    class PALUDIS_VISIBLE RepositoryFactory :
        public Singleton<RepositoryFactory>
    {
        friend class Singleton<RepositoryFactory>;

        private:
            Pimp<RepositoryFactory> _imp;

            RepositoryFactory();
            ~RepositoryFactory();

        public:
            typedef std::function<std::string (const std::string &)> KeyFunction;

            typedef std::function<const std::shared_ptr<Repository>(
                    Environment * const,
                    const KeyFunction &
                    )> CreateFunction;

            typedef std::function<const std::shared_ptr<const RepositoryNameSet> (
                    const Environment * const,
                    const KeyFunction &
                    )> DependenciesFunction;

            typedef std::function<const RepositoryName (
                    const Environment * const,
                    const KeyFunction &
                    )> NameFunction;

            typedef std::function<int (
                    const Environment * const,
                    const KeyFunction &
                    )> ImportanceFunction;

            /**
             * Construct a given repository, or throw ConfigurationError.
             *
             * If the repository to be created has dependencies upon another
             * repository, that repository must have been created and added
             * to the environment's package database first.
             *
             * The returned repository is <em>not</em> added to the Environment's
             * package database.
             *
             * \param key_function should return the value for a given key. The
             * 'format' key must return a value (e.g. 'ebuild'), which is used
             * to select the return type. The 'repo_name' key's value should be
             * the file that best describes the location of the file containing
             * the repository config, if such a file exists. Other key names are
             * repository defined, but typically include things like 'location'
             * and 'sync'.
             */
            const std::shared_ptr<Repository> create(
                    Environment * const env,
                    const KeyFunction & key_function
                    ) const PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Find the name of the repository that would be constructed if the
             * supplied parameters were passed to RepositoryFactory::create.
             *
             * \see RepositoryFactory::create for parameter documentation.
             */
            const RepositoryName name(
                    const Environment * const env,
                    const KeyFunction & key_function
                    ) const PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Find the importance of the repository that would be constructed if the
             * supplied parameters were passed to RepositoryFactory::create.
             *
             * \see RepositoryFactory::create for parameter documentation.
             */
            int importance(
                    const Environment * const env,
                    const KeyFunction & key_function
                    ) const PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Fetch the names of any repositories depended upon by a particular
             * repository.
             *
             * \see RepositoryFactory::create for parameter documentation.
             */
            const std::shared_ptr<const RepositoryNameSet> dependencies(
                    const Environment * const env,
                    const KeyFunction & key_function
                    ) const PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Add a repository format.
             *
             * \param formats must have at least one value, and no value may be
             * specified more than once across all invocations.
             *
             * \param name_function is used to implement RepositoryFactory::name.
             *
             * \param importance_function is used to implement RepositoryFactory::importance.
             *
             * \param create_function is used to implement RepositoryFactory::create.
             *
             * \param dependencies_function is used to implement
             * RepositoryFactory::dependencies.
             */
            void add_repository_format(
                    const std::shared_ptr<const Set<std::string> > & formats,
                    const NameFunction & name_function,
                    const ImportanceFunction & importance_function,
                    const CreateFunction & create_function,
                    const DependenciesFunction & dependencies_function
                    );

            struct ConstIteratorTag;
            typedef WrappedForwardIterator<ConstIteratorTag, const std::string> ConstIterator;
            ConstIterator begin_keys() const PALUDIS_ATTRIBUTE((warn_unused_result));
            ConstIterator end_keys() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    template <typename RepositoryClass_>
    void register_repositories(const RepositoryClass_ * const, RepositoryFactory * const);

    extern template class Pimp<RepositoryFactory>;
    extern template class Singleton<RepositoryFactory>;
    extern template class WrappedForwardIterator<RepositoryFactory::ConstIteratorTag, const std::string>;
}

#endif
