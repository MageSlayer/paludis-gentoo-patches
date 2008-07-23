/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008 Ciaran McCreesh
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
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/filtered_generator-fwd.hh>
#include <paludis/name-fwd.hh>
#include <paludis/environment-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/dep_spec-fwd.hh>
#include <tr1/memory>

namespace paludis
{
    class PALUDIS_VISIBLE Generator :
        private PrivateImplementationPattern<Generator>
    {
        protected:
            Generator(const std::tr1::shared_ptr<const GeneratorHandler> &);

        public:
            Generator(const Generator &);
            Generator & operator= (const Generator &);
            ~Generator();

            operator FilteredGenerator () const PALUDIS_ATTRIBUTE((warn_unused_result));

            std::string as_string() const PALUDIS_ATTRIBUTE((warn_unused_result));

            std::tr1::shared_ptr<const RepositoryNameSet> repositories(
                    const Environment * const) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            std::tr1::shared_ptr<const CategoryNamePartSet> categories(
                    const Environment * const,
                    const std::tr1::shared_ptr<const RepositoryNameSet> &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            std::tr1::shared_ptr<const QualifiedPackageNameSet> packages(
                    const Environment * const,
                    const std::tr1::shared_ptr<const RepositoryNameSet> &,
                    const std::tr1::shared_ptr<const CategoryNamePartSet> &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            std::tr1::shared_ptr<const PackageIDSet> ids(
                    const Environment * const,
                    const std::tr1::shared_ptr<const RepositoryNameSet> &,
                    const std::tr1::shared_ptr<const QualifiedPackageNameSet> &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    namespace generator
    {
        class PALUDIS_VISIBLE All :
            public Generator
        {
            public:
                All();
        };

        class PALUDIS_VISIBLE Matches :
            public Generator
        {
            public:
                Matches(const PackageDepSpec &);
        };

        class PALUDIS_VISIBLE Package :
            public Generator
        {
            public:
                Package(const QualifiedPackageName &);
        };

        class PALUDIS_VISIBLE FromRepository :
            public Generator
        {
            public:
                FromRepository(const RepositoryName &);
        };

        class PALUDIS_VISIBLE InRepository :
            public Generator
        {
            public:
                InRepository(const RepositoryName &);
        };

        class PALUDIS_VISIBLE Category :
            public Generator
        {
            public:
                Category(const CategoryNamePart &);
        };

        class PALUDIS_VISIBLE Intersection :
            public Generator
        {
            public:
                Intersection(const Generator &, const Generator &);
        };

        template <typename>
        class PALUDIS_VISIBLE SomeIDsMightSupportAction :
            public Generator
        {
            public:
                SomeIDsMightSupportAction();
        };
    }

#ifdef PALUDIS_HAVE_EXTERN_TEMPLATE
    extern template class PrivateImplementationPattern<Generator>;
#endif

}

#endif
