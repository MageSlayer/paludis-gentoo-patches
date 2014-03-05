/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010, 2011 Ciaran McCreesh
 * Copyright (c) 2014 Dimitry Ishenko
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

#include <paludis/resolver/selection_with_promotion.hh>
#include <paludis/resolver/package_id_comparator_with_promotion.hh>
#include <paludis/selection_handler.hh>
#include <paludis/util/sequence-impl.hh>
#include <paludis/util/set-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/name.hh>
#include <paludis/repository.hh>
#include <paludis/package_id.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/environment.hh>

#include <memory>
#include <algorithm>

using namespace paludis;

namespace
{
    class AllVersionsSortedWithPromotionSelectionHandler :
        public SelectionHandler
    {
        public:
            AllVersionsSortedWithPromotionSelectionHandler(const FilteredGenerator & g) :
                SelectionHandler(g)
            {
            }

            virtual std::shared_ptr<PackageIDSequence> perform_select(const Environment * const env) const
            {
                std::shared_ptr<PackageIDSequence> result(std::make_shared<PackageIDSequence>());
                RepositoryContentMayExcludes may_excludes(_fg.filter().may_excludes());

                std::shared_ptr<const RepositoryNameSet> r(_fg.filter().repositories(env, _fg.generator().repositories(env, may_excludes)));
                if (r->empty())
                    return result;

                std::shared_ptr<const CategoryNamePartSet> c(_fg.filter().categories(env, r, _fg.generator().categories(env, r, may_excludes)));
                if (c->empty())
                    return result;

                std::shared_ptr<const QualifiedPackageNameSet> p(_fg.filter().packages(env, r, _fg.generator().packages(env, r, c, may_excludes)));
                if (p->empty())
                    return result;

                std::shared_ptr<const PackageIDSet> i(_fg.filter().ids(env, _fg.generator().ids(env, r, p, may_excludes)));
                std::copy(i->begin(), i->end(), result->back_inserter());
                result->sort(resolver::PackageIDComparatorWithPromotion(env));

                return result;
            }

            virtual std::string as_string() const
            {
                return "all versions sorted from " + stringify(_fg);
            }
    };
}

selection::AllVersionsSortedWithPromotion::AllVersionsSortedWithPromotion(const FilteredGenerator & f) :
    Selection(std::make_shared<AllVersionsSortedWithPromotionSelectionHandler>(f))
{
}
