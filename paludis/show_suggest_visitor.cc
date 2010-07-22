/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010 Ciaran McCreesh
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

#include <paludis/show_suggest_visitor.hh>
#include <paludis/dep_list.hh>
#include <paludis/dep_spec.hh>
#include <paludis/package_id.hh>
#include <paludis/package_database.hh>
#include <paludis/dep_label.hh>
#include <paludis/selection.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/util/log.hh>
#include <paludis/util/save.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/join.hh>
#include <functional>
#include <set>
#include <list>

using namespace paludis;

typedef std::list<std::shared_ptr<DependenciesLabelSequence> > LabelsStack;

namespace paludis
{
    template <>
    struct Implementation<ShowSuggestVisitor>
    {
        DepList * const dep_list;
        std::shared_ptr<const DestinationsSet> destinations;
        const Environment * const environment;
        const std::shared_ptr<const PackageID> id;
        bool dependency_tags;
        const bool only_if_suggested_label;
        std::set<SetName> recursing_sets;
        LabelsStack labels;

        Implementation(DepList * const d, std::shared_ptr<const DestinationsSet> dd,
                const Environment * const e, const std::shared_ptr<const PackageID> & p, bool t, bool l) :
            dep_list(d),
            destinations(dd),
            environment(e),
            id(p),
            dependency_tags(t),
            only_if_suggested_label(l)
        {
            labels.push_front(std::make_shared<DependenciesLabelSequence>());
        }
    };
}

ShowSuggestVisitor::ShowSuggestVisitor(DepList * const d, const std::shared_ptr<const DestinationsSet> & dd,
        const Environment * const e, const std::shared_ptr<const PackageID> & p, bool t, bool l) :
    PrivateImplementationPattern<ShowSuggestVisitor>(new Implementation<ShowSuggestVisitor>(d, dd, e, p, t, l))
{
}

ShowSuggestVisitor::~ShowSuggestVisitor()
{
}

void
ShowSuggestVisitor::visit(const DependencySpecTree::NodeType<ConditionalDepSpec>::Type & node)
{
    if (node.spec()->condition_met())
    {
        _imp->labels.push_front(*_imp->labels.begin());
        RunOnDestruction restore_labels(std::bind(std::mem_fn(&LabelsStack::pop_front), &_imp->labels));

        std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
    }
}

void
ShowSuggestVisitor::visit(const DependencySpecTree::NodeType<AnyDepSpec>::Type & node)
{
    _imp->labels.push_front(*_imp->labels.begin());
    RunOnDestruction restore_labels(std::bind(std::mem_fn(&LabelsStack::pop_front), &_imp->labels));

    std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
}

void
ShowSuggestVisitor::visit(const DependencySpecTree::NodeType<AllDepSpec>::Type & node)
{
    _imp->labels.push_front(*_imp->labels.begin());
    RunOnDestruction restore_labels(std::bind(std::mem_fn(&LabelsStack::pop_front), &_imp->labels));

    std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
}

void
ShowSuggestVisitor::visit(const DependencySpecTree::NodeType<BlockDepSpec>::Type &)
{
}

namespace
{
    struct SuggestActiveVisitor
    {
        bool visit(const DependenciesBuildLabel &) const
        {
            return false;
        }

        bool visit(const DependenciesTestLabel &) const
        {
            return false;
        }

        bool visit(const DependenciesRunLabel &) const
        {
            return false;
        }

        bool visit(const DependenciesPostLabel &) const
        {
            return false;
        }

        bool visit(const DependenciesCompileAgainstLabel &) const
        {
            return false;
        }

        bool visit(const DependenciesFetchLabel &) const
        {
            return false;
        }

        bool visit(const DependenciesInstallLabel &) const
        {
            return false;
        }

        bool visit(const DependenciesRecommendationLabel &) const
        {
            return false;
        }

        bool visit(const DependenciesSuggestionLabel &) const
        {
            return true;
        }
    };

    bool is_suggest_label(const std::shared_ptr<const DependenciesLabel> & l)
    {
        return l->accept_returning<bool>(SuggestActiveVisitor());
    }
}

void
ShowSuggestVisitor::visit(const DependencySpecTree::NodeType<PackageDepSpec>::Type & node)
{
    Context context("When adding suggested dep '" + stringify(*node.spec()) + "':");

    if (_imp->only_if_suggested_label)
    {
        if ((*_imp->labels.begin())->end() == std::find_if(((*_imp->labels.begin())->begin()),
                    ((*_imp->labels.begin())->end()),
                    is_suggest_label))
        {
            Log::get_instance()->message("dep_list.show_suggest_visitor.skipping_suggested", ll_debug, lc_context)
                << "Skipping dep '" << *node.spec() << "' because no suggested label is active";
            return;
        }
    }

    std::shared_ptr<const PackageIDSequence> installed_matches((*_imp->environment)[selection::AllVersionsSorted(
                generator::Matches(*node.spec(), _imp->dep_list->options()->match_package_options())
                | filter::InstalledAtRoot(_imp->environment->root()))]);
    if (! installed_matches->empty())
    {
        Log::get_instance()->message("dep_list.show_suggest_visitor.already_installed", ll_debug, lc_context)
            << "Suggestion '" << *node.spec() << "' already matched by installed packages '"
            << join(indirect_iterator(installed_matches->begin()), indirect_iterator(installed_matches->end()), ", ")
            << "', not suggesting it";
        return;
    }

    std::shared_ptr<const PackageIDSequence> matches((*_imp->environment)[selection::AllVersionsSorted(
                generator::Matches(*node.spec(), _imp->dep_list->options()->match_package_options())
                | filter::SupportsAction<InstallAction>())]);
    if (matches->empty())
    {
        Log::get_instance()->message("dep_list.show_suggest_visitor.nothing_found", ll_warning, lc_context)
            << "Nothing found for '" << *node.spec() << "'";
        return;
    }

    for (PackageIDSequence::ConstIterator m(matches->begin()), m_end(matches->end()) ;
            m != m_end ; ++m)
    {
        if ((*m)->masked())
            continue;

        _imp->dep_list->add_suggested_package(*m, *node.spec(), _imp->destinations);
        return;
    }

    Log::get_instance()->message("dep_list.show_suggest_visitor.nothing_visible_found", ll_warning, lc_context)
        << "Nothing visible found for '" << *node.spec() << "'";
}

void
ShowSuggestVisitor::visit(const DependencySpecTree::NodeType<DependenciesLabelsDepSpec>::Type & node)
{
    std::shared_ptr<DependenciesLabelSequence> labels(new DependenciesLabelSequence);
    std::copy(node.spec()->begin(), node.spec()->end(), labels->back_inserter());
    *_imp->labels.begin() = labels;
}

void
ShowSuggestVisitor::visit(const DependencySpecTree::NodeType<NamedSetDepSpec>::Type & node)
{
    Context context("When expanding named set '" + stringify(*node.spec()) + "':");

    const std::shared_ptr<const SetSpecTree> set(_imp->environment->set(node.spec()->name()));

    if (! set)
    {
        Log::get_instance()->message("dep_list.show_suggest_visitor.unknown_set", ll_warning, lc_context)
            << "Unknown set '" << node.spec()->name() << "'";
        return;
    }

    if (! _imp->recursing_sets.insert(node.spec()->name()).second)
    {
        Log::get_instance()->message("dep_list.show_suggest_visitor.recursive_set", ll_warning, lc_context)
            << "Recursively defined set '" << node.spec()->name() << "'";
        return;
    }

    set->root()->accept(*this);

    _imp->recursing_sets.erase(node.spec()->name());
}

