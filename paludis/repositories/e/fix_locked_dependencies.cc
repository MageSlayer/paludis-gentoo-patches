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

#include <paludis/repositories/e/fix_locked_dependencies.hh>
#include <paludis/repositories/e/eapi.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/visitor_cast.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/options.hh>
#include <paludis/dep_spec.hh>
#include <paludis/environment.hh>
#include <paludis/package_database.hh>
#include <paludis/package_id.hh>
#include <paludis/elike_slot_requirement.hh>
#include <paludis/selection.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <tr1/functional>
#include <algorithm>
#include <list>

using namespace paludis;
using namespace paludis::erepository;

namespace
{
    void cannot_add(const std::tr1::shared_ptr<const DependencySpecTree::ConstItem> &) PALUDIS_ATTRIBUTE((noreturn));

    void cannot_add(const std::tr1::shared_ptr<const DependencySpecTree::ConstItem> &)
    {
        throw InternalError(PALUDIS_HERE, "Got weird tree");
    }

    struct Fixer :
        ConstVisitor<DependencySpecTree>
    {
        std::list<std::pair<
            std::tr1::shared_ptr<DependencySpecTree::ConstItem>,
            std::tr1::function<void (const std::tr1::shared_ptr<DependencySpecTree::ConstItem> &)> > > stack;

        std::tr1::shared_ptr<const DependencySpecTree::ConstItem> result;

        const Environment * const env;
        const EAPI & eapi;
        const std::tr1::shared_ptr<const PackageID> id;

        Fixer(const Environment * const e, const EAPI & a, const std::tr1::shared_ptr<const PackageID> & i) :
            env(e),
            eapi(a),
            id(i)
        {
        }

        void visit_sequence(const AllDepSpec & s,
                DependencySpecTree::ConstSequenceIterator cur,
                DependencySpecTree::ConstSequenceIterator end)
        {
            std::tr1::shared_ptr<ConstTreeSequence<DependencySpecTree, AllDepSpec> > c(
                    new ConstTreeSequence<DependencySpecTree, AllDepSpec>(
                        std::tr1::static_pointer_cast<AllDepSpec>(s.clone())));

            if (! stack.empty())
                stack.begin()->second(c);
            else
                result = c;

            using namespace std::tr1::placeholders;
            stack.push_front(std::make_pair(c, std::tr1::bind(&ConstTreeSequence<DependencySpecTree, AllDepSpec>::add, c.get(), _1)));
            std::for_each(cur, end, accept_visitor(*this));
            stack.pop_front();
        }

        void visit_sequence(const AnyDepSpec & s,
                DependencySpecTree::ConstSequenceIterator cur,
                DependencySpecTree::ConstSequenceIterator end)
        {
            std::tr1::shared_ptr<ConstTreeSequence<DependencySpecTree, AnyDepSpec> > c(
                    new ConstTreeSequence<DependencySpecTree, AnyDepSpec>(
                        std::tr1::static_pointer_cast<AnyDepSpec>(s.clone())));

            if (! stack.empty())
                stack.begin()->second(c);
            else
                result = c;

            using namespace std::tr1::placeholders;
            stack.push_front(std::make_pair(c, std::tr1::bind(&ConstTreeSequence<DependencySpecTree, AnyDepSpec>::add, c.get(), _1)));
            std::for_each(cur, end, accept_visitor(*this));
            stack.pop_front();
        }

        void visit_sequence(const ConditionalDepSpec & s,
                DependencySpecTree::ConstSequenceIterator cur,
                DependencySpecTree::ConstSequenceIterator end)
        {
            std::tr1::shared_ptr<ConstTreeSequence<DependencySpecTree, ConditionalDepSpec> > c(
                    new ConstTreeSequence<DependencySpecTree, ConditionalDepSpec>(
                        std::tr1::static_pointer_cast<ConditionalDepSpec>(s.clone())));

            if (! stack.empty())
                stack.begin()->second(c);
            else
                result = c;

            using namespace std::tr1::placeholders;
            stack.push_front(std::make_pair(c, std::tr1::bind(&ConstTreeSequence<DependencySpecTree, ConditionalDepSpec>::add, c.get(), _1)));
            std::for_each(cur, end, accept_visitor(*this));
            stack.pop_front();
        }

        void visit_leaf(const PackageDepSpec & s)
        {
            std::tr1::shared_ptr<TreeLeaf<DependencySpecTree, PackageDepSpec> > c;

            do
            {
                if (! s.slot_requirement_ptr())
                    break;

                const SlotAnyLockedRequirement * const r(visitor_cast<const SlotAnyLockedRequirement>(*s.slot_requirement_ptr()));
                if (! r)
                    break;

                std::tr1::shared_ptr<const PackageIDSequence> matches((*env)[selection::AllVersionsSorted(
                            generator::Matches(s) | filter::InstalledAtRoot(FSEntry("/")))]);
                if (matches->empty())
                    break;

                PackageDepSpec new_s(partial_parse_elike_package_dep_spec(stringify(s),
                            eapi.supported()->package_dep_spec_parse_options(), id).slot_requirement(
                            make_shared_ptr(new ELikeSlotExactRequirement((*matches->last())->slot(), true))));

                c.reset(new TreeLeaf<DependencySpecTree, PackageDepSpec>(std::tr1::static_pointer_cast<PackageDepSpec>(
                                PackageDepSpec(new_s).clone())));
            } while (false);

            if (! c)
                c.reset(new TreeLeaf<DependencySpecTree, PackageDepSpec>(std::tr1::static_pointer_cast<PackageDepSpec>(s.clone())));

            if (stack.empty())
            {
                stack.push_front(std::make_pair(c, &cannot_add));
                result = c;
            }
            else
                stack.begin()->second(c);
        }

        void visit_leaf(const NamedSetDepSpec & s)
        {
            std::tr1::shared_ptr<TreeLeaf<DependencySpecTree, NamedSetDepSpec> > c(
                    new TreeLeaf<DependencySpecTree, NamedSetDepSpec>(std::tr1::static_pointer_cast<NamedSetDepSpec>(s.clone())));

            if (stack.empty())
            {
                stack.push_front(std::make_pair(c, &cannot_add));
                result = c;
            }
            else
                stack.begin()->second(c);
        }

        void visit_leaf(const BlockDepSpec & s)
        {
            std::tr1::shared_ptr<TreeLeaf<DependencySpecTree, BlockDepSpec> > c(
                    new TreeLeaf<DependencySpecTree, BlockDepSpec>(std::tr1::static_pointer_cast<BlockDepSpec>(s.clone())));

            if (stack.empty())
            {
                stack.push_front(std::make_pair(c, &cannot_add));
                result = c;
            }
            else
                stack.begin()->second(c);
        }

        void visit_leaf(const DependencyLabelsDepSpec & s)
        {
            std::tr1::shared_ptr<TreeLeaf<DependencySpecTree, DependencyLabelsDepSpec> > c(
                    new TreeLeaf<DependencySpecTree, DependencyLabelsDepSpec>(std::tr1::static_pointer_cast<DependencyLabelsDepSpec>(s.clone())));

            if (stack.empty())
            {
                stack.push_front(std::make_pair(c, &cannot_add));
                result = c;
            }
            else
                stack.begin()->second(c);
        }
    };
}

const std::tr1::shared_ptr<const DependencySpecTree::ConstItem>
paludis::erepository::fix_locked_dependencies(
        const Environment * const env,
        const EAPI & e, const std::tr1::shared_ptr<const PackageID> & id,
        const std::tr1::shared_ptr<const DependencySpecTree::ConstItem> & b)
{
    Fixer f(env, e, id);
    b->accept(f);
    return f.result;
}

