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

#ifndef PALUDIS_GUARD_PALUDIS_GENERATOR_HH
#define PALUDIS_GUARD_PALUDIS_GENERATOR_HH 1

#include <paludis/generator-fwd.hh>
#include <paludis/generator_handler-fwd.hh>
#include <paludis/util/pimp.hh>
#include <paludis/util/options.hh>
#include <paludis/filtered_generator-fwd.hh>
#include <paludis/name-fwd.hh>
#include <paludis/environment-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/dep_spec-fwd.hh>
#include <paludis/match_package-fwd.hh>
#include <paludis/repository-fwd.hh>
#include <memory>

/** \file
 * Declarations for the Generator class.
 *
 * \ingroup g_selections
 *
 * \section Examples
 *
 * - \ref example_selection.cc "example_selection.cc"
 */

namespace paludis
{
    /**
     * A Generator specifies general properties desired from the PackageID
     * instances to be returned by Environment::operator[].
     *
     * A Generator can be converted implicitly to a FilteredGenerator, either
     * for being passed directly to a Selection subclass or for combining with
     * one or more Filter subclasses.
     *
     * \ingroup g_selections
     */
    class PALUDIS_VISIBLE Generator
    {
        private:
            Pimp<Generator> _imp;

        protected:
            Generator(const std::shared_ptr<const GeneratorHandler> &);

        public:
            ///\name Basic operations
            ///\{

            /**
             * Generator subclasses can be copied without losing information.
             */
            Generator(const Generator &);
            Generator & operator= (const Generator &);
            ~Generator();

            ///\}

            /**
             * We can implicitly convert to a FilteredGenerator, for being
             * passed to a Selection subclass or combined with one or more
             * Filter subclasses.
             */
            operator FilteredGenerator () const PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * We are representable as a string, for use when stringifying.
             */
            std::string as_string() const PALUDIS_ATTRIBUTE((warn_unused_result));

            ///\name For use by Selection
            ///\{

            /**
             * Used by Selection subclasses to get a candidate set of
             * repositories for consideration.
             */
            std::shared_ptr<const RepositoryNameSet> repositories(
                    const Environment * const,
                    const RepositoryContentMayExcludes &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Used by Selection subclasses to get a candidate set of categories
             * for consideration.
             */
            std::shared_ptr<const CategoryNamePartSet> categories(
                    const Environment * const,
                    const std::shared_ptr<const RepositoryNameSet> &,
                    const RepositoryContentMayExcludes &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Used by Selection subclasses to get a candidate set of package
             * names for consideration.
             */
            std::shared_ptr<const QualifiedPackageNameSet> packages(
                    const Environment * const,
                    const std::shared_ptr<const RepositoryNameSet> &,
                    const std::shared_ptr<const CategoryNamePartSet> &,
                    const RepositoryContentMayExcludes &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Used by Selection subclasses to get a candidate set of PackageID
             * instances for consideration.
             */
            std::shared_ptr<const PackageIDSet> ids(
                    const Environment * const,
                    const std::shared_ptr<const RepositoryNameSet> &,
                    const std::shared_ptr<const QualifiedPackageNameSet> &,
                    const RepositoryContentMayExcludes &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            ///\}
    };

    namespace generator
    {
        /**
         * A Generator which returns all PackageIDs.
         *
         * \ingroup g_selections
         */
        class PALUDIS_VISIBLE All :
            public Generator
        {
            public:
                All();
        };

        /**
         * A Generator which returns only those PackageIDs which match a
         * particular PackageDepSpec.
         *
         * \ingroup g_selections
         */
        class PALUDIS_VISIBLE Matches :
            public Generator
        {
            public:
                /**
                 * \param spec_id The PackageID the spec comes from. May be null. Used for
                 * [use=] style dependencies.
                 *
                 * \since 0.58 takes spec_id
                 */
                Matches(const PackageDepSpec &, const std::shared_ptr<const PackageID> & from_id, const MatchPackageOptions &);
        };

        /**
         * A Generator which returns only those PackageIDs which have a
         * particular package name.
         *
         * \ingroup g_selections
         */
        class PALUDIS_VISIBLE Package :
            public Generator
        {
            public:
                Package(const QualifiedPackageName &);
        };

        /**
         * A Generator which returns only those PackageIDs which are from a
         * particular repository.
         *
         * \ingroup g_selections
         */
        class PALUDIS_VISIBLE FromRepository :
            public Generator
        {
            public:
                FromRepository(const RepositoryName &);
        };

        /**
         * A Generator which returns only those PackageIDs which are in a
         * particular repository.
         *
         * \ingroup g_selections
         */
        class PALUDIS_VISIBLE InRepository :
            public Generator
        {
            public:
                InRepository(const RepositoryName &);
        };

        /**
         * A Generator which returns only those PackageIDs which are in a
         * particular category.
         *
         * \ingroup g_selections
         */
        class PALUDIS_VISIBLE Category :
            public Generator
        {
            public:
                Category(const CategoryNamePart &);
        };

        /**
         * A Generator which returns the intersection of two other Generator
         * instances.
         *
         * Usually constructed by using operator + on two Generators.
         *
         * \ingroup g_selections
         */
        class PALUDIS_VISIBLE Intersection :
            public Generator
        {
            public:
                Intersection(const Generator &, const Generator &);
        };

        /**
         * A Generator which returns the union of two other Generator
         * instances.
         *
         * Usually constructed by using operator & on two Generators.
         *
         * \ingroup g_selections
         */
        class PALUDIS_VISIBLE Union :
            public Generator
        {
            public:
                Union(const Generator &, const Generator &);
        };

        /**
         * A Generator which returns PackageID instances which might support a
         * particular Action.
         *
         * There is no guarantee that returned PackageID instances will actually
         * support the Action subclass in question. However, explicitly checking
         * whether a PackageID really does support an Action is sometimes more
         * work than is necessary at the query stage.
         *
         * \ingroup g_selections
         */
        template <typename>
        class PALUDIS_VISIBLE SomeIDsMightSupportAction :
            public Generator
        {
            public:
                SomeIDsMightSupportAction();
        };

        /**
         * A Generator which returns no PackageIDs.
         *
         * \ingroup g_selections
         */
        class PALUDIS_VISIBLE Nothing :
            public Generator
        {
            public:
                Nothing();
        };
    }

    extern template class Pimp<Generator>;

}

#endif
