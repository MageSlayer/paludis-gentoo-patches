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

#include <paludis/selection.hh>
#include <paludis/selection_handler.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/sequence-impl.hh>
#include <paludis/util/set-impl.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/iterator_funcs.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/join.hh>
#include <paludis/name.hh>
#include <paludis/version_spec.hh>
#include <paludis/package_id.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/environment.hh>
#include <paludis/metadata_key.hh>
#include <paludis/slot.hh>

#include <algorithm>
#include <functional>
#include <map>

using namespace paludis;

DidNotGetExactlyOneError::DidNotGetExactlyOneError(const std::string & s, const std::shared_ptr<const PackageIDSet> & r) noexcept :
    Exception("Did not get unique result for '" + stringify(s) + "' (got { " + join(indirect_iterator(r->begin()),
                    indirect_iterator(r->end()), ", ") + "})")
{
}

namespace paludis
{
    template <>
    struct Imp<Selection>
    {
        std::shared_ptr<const SelectionHandler> handler;

        Imp(const std::shared_ptr<const SelectionHandler> & h) :
            handler(h)
        {
        }
    };
}

Selection::Selection(const std::shared_ptr<const SelectionHandler> & h) :
    _imp(h)
{
}

Selection::Selection(const Selection & other) :
    _imp(other._imp->handler)
{
}

Selection::~Selection() = default;

Selection &
Selection::operator= (const Selection & other)
{
    if (this != &other)
        _imp->handler = other._imp->handler;
    return *this;
}

std::shared_ptr<PackageIDSequence>
Selection::perform_select(const Environment * const env) const
{
    Context context("When finding " + _imp->handler->as_string() + ":");
    return _imp->handler->perform_select(env);
}

std::string
Selection::as_string() const
{
    return _imp->handler->as_string();
}

namespace
{
    std::string slot_as_string(const std::shared_ptr<const PackageID> & id)
    {
        if (id->slot_key())
            return stringify(id->slot_key()->parse_value().parallel_value());
        else
            return "(none)";
    }

    class SomeArbitraryVersionSelectionHandler :
        public SelectionHandler
    {
        public:
            SomeArbitraryVersionSelectionHandler(const FilteredGenerator & g) :
                SelectionHandler(g)
            {
            }

            std::shared_ptr<PackageIDSequence> perform_select(const Environment * const env) const override
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

                for (QualifiedPackageNameSet::ConstIterator q(p->begin()), q_end(p->end()) ; q != q_end ; ++q)
                {
                    std::shared_ptr<QualifiedPackageNameSet> s(std::make_shared<QualifiedPackageNameSet>());
                    s->insert(*q);
                    std::shared_ptr<const PackageIDSet> i(_fg.filter().ids(env, _fg.generator().ids(env, r, s, may_excludes)));
                    if (! i->empty())
                    {
                        result->push_back(*i->begin());
                        break;
                    }
                }

                return result;
            }

            std::string as_string() const override
            {
                return "some arbitrary version from " + stringify(_fg);
            }
    };

    class BestVersionOnlySelectionHandler :
        public SelectionHandler
    {
        public:
            BestVersionOnlySelectionHandler(const FilteredGenerator & g) :
                SelectionHandler(g)
            {
            }

            std::shared_ptr<PackageIDSequence> perform_select(const Environment * const env) const override
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

                for (QualifiedPackageNameSet::ConstIterator q(p->begin()), q_end(p->end()) ; q != q_end ; ++q)
                {
                    std::shared_ptr<QualifiedPackageNameSet> s(std::make_shared<QualifiedPackageNameSet>());
                    s->insert(*q);
                    std::shared_ptr<const PackageIDSet> i(_fg.filter().ids(env, _fg.generator().ids(env, r, s, may_excludes)));
                    if (! i->empty())
                        result->push_back(*std::max_element(i->begin(), i->end(), PackageIDComparator(env)));
                }

                return result;
            }

            std::string as_string() const override
            {
                return "best version of each package from " + stringify(_fg);
            }
    };

    class AllVersionsSortedSelectionHandler :
        public SelectionHandler
    {
        public:
            AllVersionsSortedSelectionHandler(const FilteredGenerator & g) :
                SelectionHandler(g)
            {
            }

            std::shared_ptr<PackageIDSequence> perform_select(const Environment * const env) const override
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
                result->sort(PackageIDComparator(env));

                return result;
            }

            std::string as_string() const override
            {
                return "all versions sorted from " + stringify(_fg);
            }
    };

    class AllVersionsUnsortedSelectionHandler :
        public SelectionHandler
    {
        public:
            AllVersionsUnsortedSelectionHandler(const FilteredGenerator & g) :
                SelectionHandler(g)
            {
            }

            std::shared_ptr<PackageIDSequence> perform_select(const Environment * const env) const override
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

                return result;
            }

            std::string as_string() const override
            {
                return "all versions in some arbitrary order from " + stringify(_fg);
            }
    };

    class AllVersionsGroupedBySlotSelectioHandler :
        public SelectionHandler
    {
        public:
            AllVersionsGroupedBySlotSelectioHandler(const FilteredGenerator & g) :
                SelectionHandler(g)
            {
            }

            std::shared_ptr<PackageIDSequence> perform_select(const Environment * const env) const override
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

                std::shared_ptr<const PackageIDSet> id(_fg.filter().ids(env, _fg.generator().ids(env, r, p, may_excludes)));

                typedef std::map<std::pair<QualifiedPackageName, std::string>, std::shared_ptr<PackageIDSequence> > SlotMap;
                SlotMap by_slot;
                for (PackageIDSet::ConstIterator i(id->begin()), i_end(id->end()) ;
                        i != i_end ; ++i)
                {
                    SlotMap::iterator m(by_slot.find(std::make_pair((*i)->name(), slot_as_string(*i))));
                    if (m == by_slot.end())
                        m = by_slot.insert(std::make_pair(std::make_pair((*i)->name(), slot_as_string(*i)),
                                    std::make_shared<PackageIDSequence>())).first;
                    m->second->push_back(*i);
                }

                PackageIDComparator comparator(env);
                for (auto & i : by_slot)
                    i.second->sort(comparator);

                while (! by_slot.empty())
                {
                    SlotMap::iterator m(by_slot.begin());
                    for (SlotMap::iterator n(by_slot.begin()), n_end(by_slot.end()) ; n != n_end ; ++n)
                        if (! comparator(*m->second->last(), *n->second->last()))
                            m = n;

                    std::copy(m->second->begin(), m->second->end(), result->back_inserter());
                    by_slot.erase(m);
                }

                return result;
            }

            std::string as_string() const override
            {
                return "all versions grouped by slot from " + stringify(_fg);
            }
    };

    class BestVersionInEachSlotSelectionHandler :
        public SelectionHandler
    {
        public:
            BestVersionInEachSlotSelectionHandler(const FilteredGenerator & g) :
                SelectionHandler(g)
            {
            }

            std::shared_ptr<PackageIDSequence> perform_select(const Environment * const env) const override
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

                std::shared_ptr<const PackageIDSet> id(_fg.filter().ids(env, _fg.generator().ids(env, r, p, may_excludes)));

                typedef std::map<std::pair<QualifiedPackageName, std::string>, std::shared_ptr<PackageIDSequence> > SlotMap;
                SlotMap by_slot;
                for (PackageIDSet::ConstIterator i(id->begin()), i_end(id->end()) ;
                        i != i_end ; ++i)
                {
                    SlotMap::iterator m(by_slot.find(std::make_pair((*i)->name(), slot_as_string(*i))));
                    if (m == by_slot.end())
                        m = by_slot.insert(std::make_pair(std::make_pair((*i)->name(), slot_as_string(*i)),
                                    std::make_shared<PackageIDSequence>())).first;
                    m->second->push_back(*i);
                }

                PackageIDComparator comparator(env);
                for (auto & i : by_slot)
                    i.second->sort(comparator);

                while (! by_slot.empty())
                {
                    SlotMap::iterator m(by_slot.begin());
                    for (SlotMap::iterator n(by_slot.begin()), n_end(by_slot.end()) ; n != n_end ; ++n)
                        if (! comparator(*m->second->last(), *n->second->last()))
                            m = n;

                    std::copy(m->second->last(), m->second->end(), result->back_inserter());
                    by_slot.erase(m);
                }

                return result;
            }

            std::string as_string() const override
            {
                return "best version in each slot from " + stringify(_fg);
            }
    };

    class RequireExactlyOneSelectionHandler :
        public SelectionHandler
    {
        public:
            RequireExactlyOneSelectionHandler(const FilteredGenerator & g) :
                SelectionHandler(g)
            {
            }

            std::shared_ptr<PackageIDSequence> perform_select(const Environment * const env) const override
            {
                std::shared_ptr<PackageIDSequence> result(std::make_shared<PackageIDSequence>());
                RepositoryContentMayExcludes may_excludes(_fg.filter().may_excludes());

                std::shared_ptr<const RepositoryNameSet> r(_fg.filter().repositories(env, _fg.generator().repositories(env, may_excludes)));
                if (r->empty())
                    throw DidNotGetExactlyOneError(as_string(), std::make_shared<PackageIDSet>());

                std::shared_ptr<const CategoryNamePartSet> c(_fg.filter().categories(env, r, _fg.generator().categories(env, r, may_excludes)));
                if (c->empty())
                    throw DidNotGetExactlyOneError(as_string(), std::make_shared<PackageIDSet>());

                std::shared_ptr<const QualifiedPackageNameSet> p(_fg.filter().packages(env, r, _fg.generator().packages(env, r, c, may_excludes)));
                if (p->empty())
                    throw DidNotGetExactlyOneError(as_string(), std::make_shared<PackageIDSet>());

                std::shared_ptr<const PackageIDSet> i(_fg.filter().ids(env, _fg.generator().ids(env, r, p, may_excludes)));

                if (i->empty() || next(i->begin()) != i->end())
                    throw DidNotGetExactlyOneError(as_string(), i);

                result->push_back(*i->begin());

                return result;
            }

            std::string as_string() const override
            {
                return "the single version from " + stringify(_fg);
            }
    };
}

selection::SomeArbitraryVersion::SomeArbitraryVersion(const FilteredGenerator & f) :
    Selection(std::make_shared<SomeArbitraryVersionSelectionHandler>(f))
{
}

selection::BestVersionOnly::BestVersionOnly(const FilteredGenerator & f) :
    Selection(std::make_shared<BestVersionOnlySelectionHandler>(f))
{
}

selection::AllVersionsSorted::AllVersionsSorted(const FilteredGenerator & f) :
    Selection(std::make_shared<AllVersionsSortedSelectionHandler>(f))
{
}

selection::AllVersionsUnsorted::AllVersionsUnsorted(const FilteredGenerator & f) :
    Selection(std::make_shared<AllVersionsUnsortedSelectionHandler>(f))
{
}

selection::AllVersionsGroupedBySlot::AllVersionsGroupedBySlot(const FilteredGenerator & f) :
    Selection(std::make_shared<AllVersionsGroupedBySlotSelectioHandler>(f))
{
}

selection::BestVersionInEachSlot::BestVersionInEachSlot(const FilteredGenerator & f) :
    Selection(std::make_shared<BestVersionInEachSlotSelectionHandler>(f))
{
}

selection::RequireExactlyOne::RequireExactlyOne(const FilteredGenerator & f) :
    Selection(std::make_shared<RequireExactlyOneSelectionHandler>(f))
{
}

std::ostream &
paludis::operator<< (std::ostream & s, const Selection & g)
{
    s << g.as_string();
    return s;
}

namespace paludis
{
    template class Pimp<Selection>;
}

