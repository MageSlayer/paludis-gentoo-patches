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

#include <paludis/repositories/e/fix_locked_dependencies.hh>
#include <paludis/repositories/e/eapi.hh>
#include <paludis/util/visitor_cast.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/options.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/accept_visitor.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/dep_spec.hh>
#include <paludis/environment.hh>
#include <paludis/package_id.hh>
#include <paludis/elike_slot_requirement.hh>
#include <paludis/selection.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/metadata_key.hh>
#include <paludis/partially_made_package_dep_spec.hh>
#include <paludis/slot.hh>
#include <functional>
#include <algorithm>
#include <list>

using namespace paludis;
using namespace paludis::erepository;

namespace
{
    void cannot_add(const std::shared_ptr<const DependencySpecTree> &) PALUDIS_ATTRIBUTE((noreturn));

    void cannot_add(const std::shared_ptr<const DependencySpecTree> &)
    {
        throw InternalError(PALUDIS_HERE, "Got weird tree");
    }

    struct SlotRewriter
    {
        const Environment * const env;
        const EAPI & eapi;
        const std::shared_ptr<const PackageID> id;
        const std::shared_ptr<const PackageDepSpec> spec;

        std::shared_ptr<const SlotRequirement> visit(const SlotExactPartialRequirement &) const
        {
            return make_null_shared_ptr();
        }

        std::shared_ptr<const SlotRequirement> visit(const SlotExactFullRequirement &) const
        {
            return make_null_shared_ptr();
        }

        std::shared_ptr<const SlotRequirement> visit(const SlotAnyUnlockedRequirement &) const
        {
            return make_null_shared_ptr();
        }

        std::shared_ptr<const SlotRequirement> visit(const SlotAnyPartialLockedRequirement &) const
        {
            std::shared_ptr<const PackageIDSequence> matches((*env)[selection::AllVersionsSorted(
                        generator::Matches(*spec, id, { }) | filter::InstalledAtRoot(env->system_root_key()->parse_value()))]);
            if (matches->empty())
                return make_null_shared_ptr();

            if ((*matches->last())->slot_key())
            {
                auto ss((*matches->last())->slot_key()->parse_value());
                if (ss.match_values().first == ss.match_values().second)
                    return std::make_shared<ELikeSlotExactPartialRequirement>(ss.match_values().first, spec->slot_requirement_ptr());
                else
                    return std::make_shared<ELikeSlotExactFullRequirement>(ss.match_values(), spec->slot_requirement_ptr());
            }
            else
                return make_null_shared_ptr();
        }

        std::shared_ptr<const SlotRequirement> visit(const SlotAnyAtAllLockedRequirement &) const
        {
            std::shared_ptr<const PackageIDSequence> matches((*env)[selection::AllVersionsSorted(
                        generator::Matches(*spec, id, { }) | filter::InstalledAtRoot(env->system_root_key()->parse_value()))]);
            if (matches->empty())
                return make_null_shared_ptr();

            if ((*matches->last())->slot_key())
            {
                auto ss((*matches->last())->slot_key()->parse_value());
                if (ss.match_values().first == ss.match_values().second)
                    return std::make_shared<ELikeSlotExactPartialRequirement>(ss.match_values().first, spec->slot_requirement_ptr());
                else
                    return std::make_shared<ELikeSlotExactFullRequirement>(ss.match_values(), spec->slot_requirement_ptr());
            }
            else
                return make_null_shared_ptr();
        }
    };

    struct Fixer
    {
        std::list<std::shared_ptr<DependencySpecTree::BasicInnerNode> > stack;
        std::shared_ptr<DependencySpecTree> result;

        const Environment * const env;
        const EAPI & eapi;
        const std::shared_ptr<const PackageID> id;

        Fixer(const Environment * const e, const EAPI & a, const std::shared_ptr<const PackageID> & i) :
            result(std::make_shared<DependencySpecTree>(std::make_shared<AllDepSpec>())),
            env(e),
            eapi(a),
            id(i)
        {
            stack.push_front(result->top());
        }

        void visit(const DependencySpecTree::NodeType<AllDepSpec>::Type & node)
        {
            std::shared_ptr<AllDepSpec> spec(std::static_pointer_cast<AllDepSpec>(node.spec()->clone()));
            stack.push_front((*stack.begin())->append(spec));
            std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
            stack.pop_front();
        }

        void visit(const DependencySpecTree::NodeType<AnyDepSpec>::Type & node)
        {
            std::shared_ptr<AnyDepSpec> spec(std::static_pointer_cast<AnyDepSpec>(node.spec()->clone()));
            stack.push_front((*stack.begin())->append(spec));
            std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
            stack.pop_front();
        }

        void visit(const DependencySpecTree::NodeType<ConditionalDepSpec>::Type & node)
        {
            std::shared_ptr<ConditionalDepSpec> spec(std::static_pointer_cast<ConditionalDepSpec>(node.spec()->clone()));
            stack.push_front((*stack.begin())->append(spec));
            std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
            stack.pop_front();
        }

        void visit(const DependencySpecTree::NodeType<PackageDepSpec>::Type & node)
        {
            std::shared_ptr<const PackageDepSpec> c;

            do
            {
                if (! node.spec()->slot_requirement_ptr())
                    break;

                auto rewritten(node.spec()->slot_requirement_ptr()->accept_returning<std::shared_ptr<const SlotRequirement> >(SlotRewriter{env, eapi, id, node.spec()}));
                if (! rewritten)
                    break;

                PackageDepSpec new_s(PartiallyMadePackageDepSpec(*node.spec()).slot_requirement(rewritten));
                new_s.set_annotations(node.spec()->maybe_annotations());
                c = std::make_shared<PackageDepSpec>(new_s);
            } while (false);

            if (! c)
                c = node.spec();

            (*stack.begin())->append(c);
        }

        void visit(const DependencySpecTree::NodeType<NamedSetDepSpec>::Type & node)
        {
            (*stack.begin())->append(node.spec());
        }

        void visit(const DependencySpecTree::NodeType<BlockDepSpec>::Type & node)
        {
            (*stack.begin())->append(node.spec());
        }

        void visit(const DependencySpecTree::NodeType<DependenciesLabelsDepSpec>::Type & node)
        {
            (*stack.begin())->append(node.spec());
        }
    };
}

const std::shared_ptr<const DependencySpecTree>
paludis::erepository::fix_locked_dependencies(
        const Environment * const env,
        const EAPI & e, const std::shared_ptr<const PackageID> & id,
        const std::shared_ptr<const DependencySpecTree> & b)
{
    Fixer f(env, e, id);
    b->top()->accept(f);
    return f.result;
}

