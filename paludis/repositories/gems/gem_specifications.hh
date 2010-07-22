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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_GEMS_GEM_SPECIFICATIONS_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_GEMS_GEM_SPECIFICATIONS_HH 1

#include <paludis/repositories/gems/gem_specification-fwd.hh>
#include <paludis/repositories/gems/yaml-fwd.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/wrapped_forward_iterator-fwd.hh>
#include <paludis/name-fwd.hh>
#include <paludis/version_spec-fwd.hh>
#include <paludis/repository-fwd.hh>
#include <memory>
#include <string>

namespace paludis
{
    namespace gems
    {
        /**
         * Represents a collection of Gem specifications held in a master yaml
         * file.
         *
         * \ingroup grpgemsrepository
         * \nosubgrouping
         */
        class PALUDIS_VISIBLE GemSpecifications :
            private PrivateImplementationPattern<GemSpecifications>
        {
            public:
                ///\name Basic operations
                ///\{

                GemSpecifications(const Environment * const, const std::shared_ptr<const Repository> &, const yaml::Node &);
                ~GemSpecifications();

                ///\}

                ///\name Iterate over our specifications
                ///\{

                struct ConstIteratorTag;
                typedef WrappedForwardIterator<ConstIteratorTag,
                        const std::pair<const std::pair<QualifiedPackageName, VersionSpec>, std::shared_ptr<const GemSpecification> > >
                            ConstIterator;
                ConstIterator begin() const PALUDIS_ATTRIBUTE((warn_unused_result));
                ConstIterator end() const PALUDIS_ATTRIBUTE((warn_unused_result));

                ///\}
        };
    }
}

#endif

