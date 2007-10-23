/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#include <paludis/dep_list.hh>
#include <paludis/dep_list_exceptions.hh>
#include <paludis/query_visitor.hh>
#include <paludis/range_rewriter.hh>
#include <paludis/show_suggest_visitor.hh>
#include <paludis/condition_tracker.hh>
#include <paludis/handled_information.hh>

#include <paludis/dep_spec.hh>
#include <paludis/action.hh>
#include <paludis/dep_spec_flattener.hh>
#include <paludis/distribution.hh>
#include <paludis/hashed_containers.hh>
#include <paludis/match_package.hh>
#include <paludis/metadata_key.hh>
#include <paludis/query.hh>
#include <paludis/package_id.hh>
#include <paludis/version_requirements.hh>

#include <paludis/util/iterator.hh>
#include <paludis/util/join.hh>
#include <paludis/util/log.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/save.hh>
#include <paludis/util/set.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/tr1_functional.hh>
#include <paludis/util/visitor-impl.hh>

#include <libwrapiter/libwrapiter_forward_iterator.hh>
#include <libwrapiter/libwrapiter_output_iterator.hh>

#include <algorithm>
#include <functional>
#include <vector>
#include <list>
#include <set>

using namespace paludis;

template class Sequence<tr1::function<bool (const PackageID &, const Mask &)> >;

#include <paludis/dep_list-sr.cc>

DepListOptions::DepListOptions() :
    reinstall(dl_reinstall_never),
    reinstall_scm(dl_reinstall_scm_never),
    target_type(dl_target_package),
    upgrade(dl_upgrade_always),
    downgrade(dl_downgrade_as_needed),
    new_slots(dl_new_slots_always),
    fall_back(dl_fall_back_as_needed_except_targets),
    installed_deps_pre(dl_deps_discard),
    installed_deps_runtime(dl_deps_try_post),
    installed_deps_post(dl_deps_try_post),
    uninstalled_deps_pre(dl_deps_pre),
    uninstalled_deps_runtime(dl_deps_pre_or_post),
    uninstalled_deps_post(dl_deps_post),
    uninstalled_deps_suggested(dl_deps_try_post),
    suggested(dl_suggested_show),
    circular(dl_circular_error),
    use(dl_use_deps_standard),
    blocks(dl_blocks_accumulate),
    dependency_tags(false)
{
    /* when changing the above, also see src/paludis/command_line.cc. */
}

namespace paludis
{
    typedef std::list<DepListEntry> MergeList;
    typedef MakeHashedMultiMap<QualifiedPackageName, MergeList::iterator>::Type MergeListIndex;

    template<>
    struct Implementation<DepList>
    {
        const Environment * const env;
        tr1::shared_ptr<DepListOptions> opts;

        MergeList merge_list;
        MergeList::const_iterator current_merge_list_entry;
        MergeList::iterator merge_list_insert_position;
        long merge_list_generation;

        MergeListIndex merge_list_index;

        SetSpecTree::ConstItem * current_top_level_target;

        bool throw_on_blocker;

        const tr1::shared_ptr<const PackageID> current_package_id() const
        {
            if (current_merge_list_entry != merge_list.end())
                return current_merge_list_entry->package_id;
            return tr1::shared_ptr<const PackageID>();
        }

        Implementation(const Environment * const e, const DepListOptions & o) :
            env(e),
            opts(new DepListOptions(o)),
            current_merge_list_entry(merge_list.end()),
            merge_list_insert_position(merge_list.end()),
            merge_list_generation(0),
            current_top_level_target(0),
            throw_on_blocker(o.blocks == dl_blocks_error)
        {
        }
    };
}

namespace
{
    struct GenerationGreaterThan
    {
        long g;

        GenerationGreaterThan(long gg) :
            g(gg)
        {
        }

        template <typename T_>
        bool operator() (const T_ & e) const
        {
            return e.generation > g;
        }
    };

    struct RemoveTagsWithGenerationGreaterThan
    {
        long g;

        RemoveTagsWithGenerationGreaterThan(long gg) :
            g(gg)
        {
        }

        void operator() (DepListEntry & e) const
        {
            /* see EffSTL 9 for why this is so painful */
            if (e.tags->empty())
                return;
            tr1::shared_ptr<DepListEntryTags> t(new DepListEntryTags);
            GenerationGreaterThan pred(g);
            for (DepListEntryTags::ConstIterator i(e.tags->begin()), i_end(e.tags->end()) ;
                    i != i_end ; ++i)
                if (! pred(*i))
                    t->insert(*i);
            std::swap(e.tags, t);
        }
    };

    class DepListTransaction
    {
        protected:
            MergeList & _list;
            MergeListIndex & _index;
            long & _generation;
            int _initial_generation;
            bool _committed;

        public:
            DepListTransaction(MergeList & l, MergeListIndex & i, long & g) :
                _list(l),
                _index(i),
                _generation(g),
                _initial_generation(g),
                _committed(false)
            {
                ++_generation;
            }

            void commit()
            {
                _committed = true;
            }

            ~DepListTransaction()
            {
                if (_committed)
                    return;

                /* See EffSTL 9 */
                GenerationGreaterThan pred(_initial_generation);
                for (MergeList::iterator i(_list.begin()) ; i != _list.end() ; )
                {
                    if (! pred(*i))
                        ++i;
                    else
                    {
                        for (std::pair<MergeListIndex::iterator, MergeListIndex::iterator> p(
                                    _index.equal_range(i->package_id->name())) ; p.first != p.second ; )
                            if (p.first->second == i)
                                _index.erase(p.first++);
                            else
                                ++p.first;

                        _list.erase(i++);
                    }
                }

                std::for_each(_list.begin(), _list.end(),
                        RemoveTagsWithGenerationGreaterThan(_initial_generation));
            }
    };

    struct MatchDepListEntryAgainstPackageDepSpec
    {
        const Environment * const env;
        const PackageDepSpec & a;

        MatchDepListEntryAgainstPackageDepSpec(const Environment * const ee,
                const PackageDepSpec & aa) :
            env(ee),
            a(aa)
        {
        }

        bool operator() (const std::pair<const QualifiedPackageName, MergeList::const_iterator> & e)
        {
            switch (e.second->kind)
            {
                case dlk_virtual:
                case dlk_package:
                case dlk_provided:
                case dlk_already_installed:
                case dlk_subpackage:
                    return match_package(*env, a, *e.second->package_id);

                case dlk_block:
                case dlk_masked:
                case dlk_suggested:
                    return false;

                case last_dlk:
                    ;
            }

            throw InternalError(PALUDIS_HERE, "Bad e.second->kind");
        }
    };

    bool is_interesting_any_child(const Environment & env,
            const DependencySpecTree::ConstItem & i)
    {
        const PackageDepSpec * const u(get_const_item(i)->as_package_dep_spec());
        if (0 != u && u->package_ptr())
        {
            return ! env.package_database()->query(
                    query::SupportsAction<InstalledAction>() &
                    query::Matches(PackageDepSpec(
                            tr1::shared_ptr<QualifiedPackageName>(new QualifiedPackageName(*u->package_ptr())))),
                    qo_whatever)->empty();
        }
        else
            return false;
    }
}

struct DepList::AddVisitor :
    ConstVisitor<DependencySpecTree>,
    ConstVisitor<DependencySpecTree>::VisitConstSequence<AddVisitor, AllDepSpec>
{
    DepList * const d;
    tr1::shared_ptr<const DestinationsSet> destinations;
    tr1::shared_ptr<ConstTreeSequence<DependencySpecTree, AllDepSpec> > conditions;
    std::set<SetName> recursing_sets;

    AddVisitor(DepList * const dd, tr1::shared_ptr<const DestinationsSet> ddd,
               tr1::shared_ptr<ConstTreeSequence<DependencySpecTree, AllDepSpec> > c =
               (tr1::shared_ptr<ConstTreeSequence<DependencySpecTree, AllDepSpec> >(
                   new ConstTreeSequence<DependencySpecTree, AllDepSpec>(
                       tr1::shared_ptr<AllDepSpec>(new AllDepSpec))))) :
        d(dd),
        destinations(ddd),
        conditions(c)
    {
    }

    using ConstVisitor<DependencySpecTree>::VisitConstSequence<AddVisitor, AllDepSpec>::visit_sequence;

    void visit_sequence(const AnyDepSpec &,
            DependencySpecTree::ConstSequenceIterator,
            DependencySpecTree::ConstSequenceIterator);

    void visit_sequence(const UseDepSpec &,
            DependencySpecTree::ConstSequenceIterator,
            DependencySpecTree::ConstSequenceIterator);

    void visit_leaf(const PackageDepSpec &);

    void visit_leaf(const BlockDepSpec &);

    void visit_leaf(const DependencyLabelsDepSpec &);

    void visit_leaf(const NamedSetDepSpec &);
};

void
DepList::AddVisitor::visit_leaf(const PackageDepSpec & a)
{
    Context context("When adding PackageDepSpec '" + stringify(a) + "':");

    Save<tr1::shared_ptr<ConstTreeSequence<DependencySpecTree, AllDepSpec> > > save_c(
        &conditions, d->_imp->opts->dependency_tags ? ConditionTracker(conditions).add_condition(a) : conditions);

    /* find already installed things */
    // TODO: check destinations
    tr1::shared_ptr<const PackageIDSequence> already_installed(d->_imp->env->package_database()->query(
                query::SupportsAction<InstalledAction>() &
                query::Matches(a),
                qo_order_by_version));

    /* are we already on the merge list? */
    std::pair<MergeListIndex::iterator, MergeListIndex::iterator> q;
    if (a.package_ptr())
        q = d->_imp->merge_list_index.equal_range(*a.package_ptr());
    else
        q = std::make_pair(d->_imp->merge_list_index.begin(), d->_imp->merge_list_index.end());

    MergeListIndex::iterator qq(std::find_if(q.first, q.second,
                MatchDepListEntryAgainstPackageDepSpec(d->_imp->env, a)));

    MergeList::iterator existing_merge_list_entry(qq == q.second ? d->_imp->merge_list.end() : qq->second);
    if (existing_merge_list_entry != d->_imp->merge_list.end())
    {
        /* tag it */
        if (a.tag())
            existing_merge_list_entry->tags->insert(DepTagEntry::create()
                    .tag(a.tag())
                    .generation(d->_imp->merge_list_generation));

        if (d->_imp->opts->dependency_tags && d->_imp->current_package_id())
            existing_merge_list_entry->tags->insert(DepTagEntry::create()
                    .tag(tr1::shared_ptr<DepTag>(new DependencyDepTag(d->_imp->current_package_id(), a, conditions)))
                    .generation(d->_imp->merge_list_generation));

        /* add an appropriate destination */
        // TODO

        /* have our deps been merged already, or is this a circular dep? */
        if (dle_no_deps == existing_merge_list_entry->state)
        {
            /* is a sufficiently good version installed? */
            if (! already_installed->empty())
                return;

            if (d->_imp->opts->circular == dl_circular_discard)
            {
                Log::get_instance()->message(ll_qa, lc_context, "Dropping circular dependency on '"
                        + stringify(*existing_merge_list_entry->package_id) + "'");
                return;
            }
            else if (d->_imp->opts->circular == dl_circular_discard_silently)
                return;

            throw CircularDependencyError("Atom '" + stringify(a) + "' matched by merge list entry '" +
                    stringify(*existing_merge_list_entry->package_id) + "', which does not yet have its "
                    "dependencies installed");
        }
        else
            return;
    }

    /* find installable candidates, and find the best visible candidate */
    tr1::shared_ptr<const PackageID> best_visible_candidate;
    tr1::shared_ptr<const PackageIDSequence> installable_candidates(
            d->_imp->env->package_database()->query(
                query::MaybeSupportsAction<InstallAction>() &
                query::Matches(a),
                qo_order_by_version));

    for (PackageIDSequence::ReverseConstIterator p(installable_candidates->rbegin()),
            p_end(installable_candidates->rend()) ; p != p_end ; ++p)
        if ((*p)->supports_action(SupportsActionTest<InstallAction>()) && ! (*p)->masked())
        {
            best_visible_candidate = *p;
            break;
        }

    /* are we allowed to override mask reasons? */
    if (! best_visible_candidate && d->_imp->opts->override_masks)
    {
        for (DepListOverrideMasksFunctions::ConstIterator of(d->_imp->opts->override_masks->begin()),
                of_end(d->_imp->opts->override_masks->end()) ; of != of_end ; ++of)
        {
            if (best_visible_candidate)
                break;

            for (PackageIDSequence::ReverseConstIterator p(installable_candidates->rbegin()),
                    p_end(installable_candidates->rend()) ; p != p_end ; ++p)
            {
                if (! (*p)->supports_action(SupportsActionTest<InstallAction>()))
                    continue;

                bool success(true);
                for (PackageID::MasksConstIterator m((*p)->begin_masks()), m_end((*p)->end_masks()) ;
                        m != m_end ; ++m)
                {
                    bool local_success(false);
                    for (DepListOverrideMasksFunctions::ConstIterator o(d->_imp->opts->override_masks->begin()),
                            o_end(next(of)) ; o != o_end ; ++o)
                        if ((*o)(**p, **m))
                            local_success = true;

                    success &= local_success;
                    if (! success)
                        break;
                }

                if (success)
                {
                    d->add_error_package(*p, dlk_masked, a, conditions);
                    best_visible_candidate = *p;
                }
            }
        }
    }

    /* no installable candidates. if we're already installed, that's ok (except for top level
     * package targets), otherwise error. */
    if (! best_visible_candidate)
    {
        bool can_fall_back;
        do
        {
            switch (d->_imp->opts->fall_back)
            {
                case dl_fall_back_never:
                    can_fall_back = false;
                    continue;

                case dl_fall_back_as_needed_except_targets:
                    if (! d->_imp->current_package_id())
                        can_fall_back = false;
                    else if (already_installed->empty())
                        can_fall_back = true;
                    else
                        can_fall_back = ! d->is_top_level_target(**already_installed->last());

                    continue;

                case dl_fall_back_as_needed:
                    can_fall_back = true;
                    continue;

                case last_dl_fall_back:
                    ;
            }

            throw InternalError(PALUDIS_HERE, "Bad fall_back value '" + stringify(d->_imp->opts->fall_back) + "'");
        } while (false);

        if (already_installed->empty() || ! can_fall_back)
        {
            if (! a.use_requirements_ptr())
                throw AllMaskedError(a);

            tr1::shared_ptr<const PackageIDSequence> match_except_reqs(d->_imp->env->package_database()->query(
                        query::Matches(*a.without_use_requirements()), qo_whatever));

            for (PackageIDSequence::ConstIterator i(match_except_reqs->begin()),
                    i_end(match_except_reqs->end()) ; i != i_end ; ++i)
                if (! (*i)->masked())
                    throw UseRequirementsNotMetError(stringify(a));

            throw AllMaskedError(a);
        }
        else
        {
            Log::get_instance()->message(ll_warning, lc_context, "No visible packages matching '"
                    + stringify(a) + "', falling back to installed package '"
                    + stringify(**already_installed->last()) + "'");
            d->add_already_installed_package(*already_installed->last(), a.tag(), a, conditions, destinations);
            return;
        }
    }

    tr1::shared_ptr<PackageIDSequence> already_installed_in_same_slot(new PackageIDSequence);
    for (PackageIDSequence::ConstIterator aa(already_installed->begin()),
            aa_end(already_installed->end()) ; aa != aa_end ; ++aa)
        if ((*aa)->slot() == best_visible_candidate->slot())
            already_installed_in_same_slot->push_back(*aa);
    /* no need to sort already_installed_in_same_slot here, although if the above is
     * changed then check that this still holds... */

    /* we have an already installed version. do we want to use it? */
    if (! already_installed_in_same_slot->empty())
    {
        if (d->prefer_installed_over_uninstalled(**already_installed_in_same_slot->last(), *best_visible_candidate))
        {
            Log::get_instance()->message(ll_debug, lc_context, "Taking installed package '"
                    + stringify(**already_installed_in_same_slot->last()) + "' over '" +
                    stringify(*best_visible_candidate) + "'");
            d->add_already_installed_package(*already_installed_in_same_slot->last(), a.tag(), a, conditions, destinations);
            return;
        }
        else
            Log::get_instance()->message(ll_debug, lc_context, "Not taking installed package '"
                    + stringify(**already_installed_in_same_slot->last()) + "' over '" +
                    stringify(*best_visible_candidate) + "'");
    }
    else if ((! already_installed->empty()) && (dl_new_slots_as_needed == d->_imp->opts->new_slots))
    {
        /* we have an already installed, but not in the same slot, and our options
         * allow us to take this. */
        if (d->prefer_installed_over_uninstalled(**already_installed->last(), *best_visible_candidate))
        {
            Log::get_instance()->message(ll_debug, lc_context, "Taking installed package '"
                    + stringify(**already_installed->last()) + "' over '" + stringify(*best_visible_candidate)
                    + "' (in different slot)");
            d->add_already_installed_package(*already_installed->last(), a.tag(), a, conditions, destinations);
            return;
        }
        else
            Log::get_instance()->message(ll_debug, lc_context, "Not taking installed package '"
                    + stringify(**already_installed->last()) + "' over '" +
                    stringify(*best_visible_candidate) + "' (in different slot)");
    }
    else
        Log::get_instance()->message(ll_debug, lc_context) << "No installed packages in SLOT '"
            << best_visible_candidate->slot() << "', taking uninstalled package '"
            << *best_visible_candidate << "'";

    /* if this is a downgrade, make sure that that's ok */
    switch (d->_imp->opts->downgrade)
    {
        case dl_downgrade_as_needed:
            break;

        case dl_downgrade_error:
        case dl_downgrade_warning:
            {
                tr1::shared_ptr<const PackageIDSequence> are_we_downgrading(
                        d->_imp->env->package_database()->query(
                            query::SupportsAction<InstalledAction>() &
                            query::Matches(PackageDepSpec(
                                    make_shared_ptr(new QualifiedPackageName(best_visible_candidate->name())),
                                    tr1::shared_ptr<CategoryNamePart>(),
                                    tr1::shared_ptr<PackageNamePart>(),
                                    tr1::shared_ptr<VersionRequirements>(),
                                    vr_and,
                                    make_shared_ptr(new SlotName(best_visible_candidate->slot())))),
                            qo_order_by_version));

                if (are_we_downgrading->empty())
                    break;

                if ((*are_we_downgrading->last())->version() <= best_visible_candidate->version())
                    break;

                if (d->_imp->opts->downgrade == dl_downgrade_error)
                    throw DowngradeNotAllowedError(stringify(*best_visible_candidate),
                            stringify(**are_we_downgrading->last()));

                Log::get_instance()->message(ll_warning, lc_context) << "Downgrade to '"
                    << *best_visible_candidate << "' from '" << **are_we_downgrading->last() << "' forced";
            }
            break;

        case last_dl_downgrade:
            ;
    }

    d->add_package(best_visible_candidate, a.tag(), a, conditions, destinations);
}

void
DepList::AddVisitor::visit_leaf(const NamedSetDepSpec & a)
{
    Context context("When adding NamedSetDepSpec '" + stringify(a) + "':");

    tr1::shared_ptr<const SetSpecTree::ConstItem> set(d->_imp->env->set(a.name()));

    if (! set)
        throw NoSuchSetError(stringify(a.name()));

    if (! recursing_sets.insert(a.name()).second)
    {
        Log::get_instance()->message(ll_warning, lc_context) << "Recursively defined set '" << a.name() << "'";
        throw RecursivelyDefinedSetError(stringify(a.name()));
    }

    set->accept(*this);

    recursing_sets.erase(a.name());
}

void
DepList::AddVisitor::visit_sequence(const UseDepSpec & a,
        DependencySpecTree::ConstSequenceIterator cur,
        DependencySpecTree::ConstSequenceIterator end)
{
    Save<tr1::shared_ptr<ConstTreeSequence<DependencySpecTree, AllDepSpec> > > save_c(
        &conditions, d->_imp->opts->dependency_tags ? ConditionTracker(conditions).add_condition(a) : conditions);

    if (d->_imp->opts->use == dl_use_deps_standard)
    {
        if ((d->_imp->current_package_id() ? d->_imp->env->query_use(a.flag(), *d->_imp->current_package_id()) : false) ^ a.inverse())
            std::for_each(cur, end, accept_visitor(*this));
    }
    else
    {
        RepositoryUseInterface * u(0);
        if ((! d->_imp->current_package_id()) || (! ((u = d->_imp->current_package_id()->repository()->use_interface))))
            std::for_each(cur, end, accept_visitor(*this));
        else if (a.inverse())
        {
            if ((! d->_imp->current_package_id()) || (! u->query_use_force(a.flag(), *d->_imp->current_package_id())))
                std::for_each(cur, end, accept_visitor(*this));
        }
        else
        {
            if ((! d->_imp->current_package_id()) || (! u->query_use_mask(a.flag(), *d->_imp->current_package_id())))
                std::for_each(cur, end, accept_visitor(*this));
        }
    }
}

void
DepList::AddVisitor::visit_sequence(const AnyDepSpec & a,
        DependencySpecTree::ConstSequenceIterator cur,
        DependencySpecTree::ConstSequenceIterator end)
{
    using namespace tr1::placeholders;

    /* annoying requirement: || ( foo? ( ... ) ) resolves to empty if !foo. */
    if (end == std::find_if(cur, end,
                tr1::bind(&is_viable_any_child, tr1::cref(*d->_imp->env), d->_imp->current_package_id(), _1)))
        return;

    RangeRewriter r;
    std::for_each(cur, end, accept_visitor(r));
    if (r.spec())
    {
        Context context("When using rewritten range '" + stringify(*r.spec()) + "':");
        TreeLeaf<DependencySpecTree, PackageDepSpec> rr(r.spec());
        d->add_not_top_level(rr, destinations, conditions);
        return;
    }

    Save<tr1::shared_ptr<ConstTreeSequence<DependencySpecTree, AllDepSpec> > > save_c(
        &conditions, d->_imp->opts->dependency_tags ? ConditionTracker(conditions).add_condition(a) : conditions);

    /* see if any of our children is already installed. if any is, add it so that
     * any upgrades kick in */
    for (DependencySpecTree::ConstSequenceIterator c(cur) ; c != end ; ++c)
    {
        if (! is_viable_any_child(*d->_imp->env, d->_imp->current_package_id(), *c))
            continue;

        if (d->already_installed(*c, destinations))
        {
            Context context("When using already installed group to resolve dependencies:");
            d->add_not_top_level(*c, destinations, conditions);
            return;
        }
    }

    /* if we have something like || ( a >=b-2 ) and b-1 is installed, try to go for
     * the b-2 bit first */
    for (DependencySpecTree::ConstSequenceIterator c(cur) ; c != end ; ++c)
    {
        if (! is_viable_any_child(*d->_imp->env, d->_imp->current_package_id(), *c))
            continue;
        if (! is_interesting_any_child(*d->_imp->env, *c))
            continue;

        try
        {
            Context context("When using already installed package to resolve dependencies:");

            Save<bool> save_t(&d->_imp->throw_on_blocker,
                    dl_blocks_discard_completely != d->_imp->opts->blocks);
            Save<tr1::shared_ptr<DepListOverrideMasksFunctions> > save_o(&d->_imp->opts->override_masks,
                    tr1::shared_ptr<DepListOverrideMasksFunctions>());
            d->add_not_top_level(*c, destinations, conditions);
            return;
        }
        catch (const DepListError &)
        {
        }
    }

    /* install first available viable option */
    for (DependencySpecTree::ConstSequenceIterator c(cur) ; c != end ; ++c)
    {
        if (! is_viable_any_child(*d->_imp->env, d->_imp->current_package_id(), *c))
            continue;

        try
        {
            Context context("When using new group to resolve dependencies:");

            Save<bool> save_t(&d->_imp->throw_on_blocker,
                    dl_blocks_discard_completely != d->_imp->opts->blocks);
            Save<tr1::shared_ptr<DepListOverrideMasksFunctions> > save_o(&d->_imp->opts->override_masks,
                    tr1::shared_ptr<DepListOverrideMasksFunctions>());
            d->add_not_top_level(*c, destinations, conditions);
            return;
        }
        catch (const DepListError &)
        {
        }
    }

    Log::get_instance()->message(ll_debug, lc_context, "No resolvable item in || ( ) block. Using "
            "first item for error message");
    {
        Context block_context("Inside || ( ) block with other options:");
        for (DependencySpecTree::ConstSequenceIterator c(cur) ; c != end ; ++c)
        {
            if (! is_viable_any_child(*d->_imp->env, d->_imp->current_package_id(), *c))
                continue;

            d->add_not_top_level(*c, destinations, conditions);
            return;
        }
    }
}

void
DepList::AddVisitor::visit_leaf(const BlockDepSpec & a)
{
    if (dl_blocks_discard_completely == d->_imp->opts->blocks)
        return;

    // TODO: check destinations

    Context context("When checking BlockDepSpec '!" + stringify(*a.blocked_spec()) + "':");

    Save<tr1::shared_ptr<ConstTreeSequence<DependencySpecTree, AllDepSpec> > > save_c(
        &conditions, d->_imp->opts->dependency_tags ? ConditionTracker(conditions).add_condition(a) : conditions);

    bool check_whole_list(false);
    std::list<MergeList::const_iterator> will_be_installed;
    tr1::shared_ptr<const PackageIDSequence> already_installed;

    if (a.blocked_spec()->package_ptr())
    {
        PackageDepSpec just_package(tr1::shared_ptr<QualifiedPackageName>(new QualifiedPackageName(
                        *a.blocked_spec()->package_ptr())));
        already_installed = d->_imp->env->package_database()->query(
                query::SupportsAction<InstalledAction>() &
                query::Matches(just_package),
                qo_whatever);

        MatchDepListEntryAgainstPackageDepSpec m(d->_imp->env, just_package);
        for (std::pair<MergeListIndex::const_iterator, MergeListIndex::const_iterator> p(
                    d->_imp->merge_list_index.equal_range(*a.blocked_spec()->package_ptr())) ;
                p.first != p.second ; ++p.first)
        {
            if (d->_imp->current_merge_list_entry != d->_imp->merge_list.end())
            {
                if (d->_imp->current_merge_list_entry == p.first->second)
                    continue;

                if (d->_imp->current_merge_list_entry->associated_entry == &*p.first->second)
                    continue;
            }

            if (m(*p.first))
                will_be_installed.push_back(p.first->second);
        }
    }
    else
    {
        check_whole_list = true;
        /* TODO: InstalledAtRoot? */
        already_installed = d->_imp->env->package_database()->query(
                query::SupportsAction<InstalledAction>(),
                qo_whatever);
    }

    if (already_installed->empty() && will_be_installed.empty() && ! check_whole_list)
        return;

    for (PackageIDSequence::ConstIterator aa(already_installed->begin()),
            aa_end(already_installed->end()) ; aa != aa_end ; ++aa)
    {
        if (! match_package(*d->_imp->env, *a.blocked_spec(), **aa))
            continue;

        bool replaced(false);
        for (std::list<MergeList::const_iterator>::const_iterator r(will_be_installed.begin()),
                r_end(will_be_installed.end()) ; r != r_end && ! replaced ; ++r)
            if ((*r)->package_id->slot() == (*aa)->slot())
            {
                /* if it's a virtual, it only replaces if it's the same package. */
                if ((*r)->package_id->virtual_for_key())
                {
                    if ((*r)->package_id->virtual_for_key()->value()->name() == (*aa)->name())
                        replaced = true;
                }
                else
                    replaced = true;
            }

        if (replaced)
            continue;

        /* ignore if it's a virtual/blah (not <virtual/blah-1) block and it's blocking
         * ourself */
        if (! (a.blocked_spec()->version_requirements_ptr() || a.blocked_spec()->slot_ptr()
                    || a.blocked_spec()->use_requirements_ptr() || a.blocked_spec()->repository_ptr())
                && d->_imp->current_package_id())
        {
            if ((*aa)->name() == d->_imp->current_package_id()->name())
                continue;

            if ((*aa)->virtual_for_key() && (*aa)->virtual_for_key()->value()->name() == d->_imp->current_package_id()->name())
                continue;
        }

        switch (d->_imp->throw_on_blocker ? dl_blocks_error : d->_imp->opts->blocks)
        {
            case dl_blocks_error:
                throw BlockError(stringify(*a.blocked_spec()));

            case dl_blocks_discard:
                Log::get_instance()->message(ll_warning, lc_context, "Discarding block '!"
                        + stringify(*a.blocked_spec()) + "'");
                break;

            case dl_blocks_discard_completely:
                break;

            case dl_blocks_accumulate:
                d->add_error_package(*aa, dlk_block, *a.blocked_spec(), conditions);
                break;

            case last_dl_blocks:
                break;
        }
    }

    for (std::list<MergeList::const_iterator>::const_iterator r(will_be_installed.begin()),
            r_end(will_be_installed.end()) ; r != r_end ; ++r)
    {
        if (! match_package(*d->_imp->env, *a.blocked_spec(), *(*r)->package_id))
            continue;

        /* ignore if it's a virtual/blah (not <virtual/blah-1) block and it's blocking
         * ourself */
        if (! (a.blocked_spec()->version_requirements_ptr() || a.blocked_spec()->slot_ptr()
                    || a.blocked_spec()->use_requirements_ptr() || a.blocked_spec()->repository_ptr())
                && d->_imp->current_package_id())
        {
            if ((*r)->package_id->name() == d->_imp->current_package_id()->name())
                continue;

            if ((*r)->package_id->virtual_for_key() && (*r)->package_id->virtual_for_key()->value()->name() ==
                    d->_imp->current_package_id()->name())
                continue;
        }

        throw BlockError(stringify(*a.blocked_spec()));
    }

    if (check_whole_list)
    {
        for (MergeList::const_iterator r(d->_imp->merge_list.begin()),
                r_end(d->_imp->merge_list.end()) ; r != r_end ; ++r)
        {
            if (! match_package(*d->_imp->env, *a.blocked_spec(), *r->package_id))
                continue;

            /* ignore if it's a virtual/blah (not <virtual/blah-1) block and it's blocking
             * ourself */
            if (! (a.blocked_spec()->version_requirements_ptr() || a.blocked_spec()->slot_ptr()
                        || a.blocked_spec()->use_requirements_ptr() || a.blocked_spec()->repository_ptr())
                    && d->_imp->current_package_id())
            {
                if (r->package_id->name() == d->_imp->current_package_id()->name())
                    continue;

                if (r->package_id->virtual_for_key() &&
                        r->package_id->virtual_for_key()->value()->name() == d->_imp->current_package_id()->name())
                    continue;
            }

            throw BlockError(stringify(*a.blocked_spec()));
        }
    }
}

void
DepList::AddVisitor::visit_leaf(const DependencyLabelsDepSpec &)
{
    // XXX implement
}

DepList::DepList(const Environment * const e, const DepListOptions & o) :
    PrivateImplementationPattern<DepList>(new Implementation<DepList>(e, o))
{
}

DepList::~DepList()
{
}

tr1::shared_ptr<DepListOptions>
DepList::options()
{
    return _imp->opts;
}

const tr1::shared_ptr<const DepListOptions>
DepList::options() const
{
    return _imp->opts;
}

void
DepList::clear()
{
    DepListOptions o(*options());
    _imp.reset(new Implementation<DepList>(_imp->env, o));
}

void
DepList::add_in_role(DependencySpecTree::ConstItem & spec, const std::string & role,
        tr1::shared_ptr<const DestinationsSet> destinations)
{
    Context context("When adding " + role + ":");
    add_not_top_level(spec, destinations,
        tr1::shared_ptr<ConstTreeSequence<DependencySpecTree, AllDepSpec> >(
            new ConstTreeSequence<DependencySpecTree, AllDepSpec>(tr1::shared_ptr<AllDepSpec>(new AllDepSpec))));
}

void
DepList::add_not_top_level(DependencySpecTree::ConstItem & spec, tr1::shared_ptr<const DestinationsSet> destinations,
                           tr1::shared_ptr<ConstTreeSequence<DependencySpecTree, AllDepSpec> > conditions)
{
    DepListTransaction transaction(_imp->merge_list, _imp->merge_list_index, _imp->merge_list_generation);

    AddVisitor visitor(this, destinations, conditions);
    spec.accept(visitor);
    transaction.commit();
}

void
DepList::add(SetSpecTree::ConstItem & spec, tr1::shared_ptr<const DestinationsSet> destinations)
{
    DepListTransaction transaction(_imp->merge_list, _imp->merge_list_index, _imp->merge_list_generation);

    Save<SetSpecTree::ConstItem *> save_current_top_level_target(&_imp->current_top_level_target,
            _imp->current_top_level_target ? _imp->current_top_level_target : &spec);

    AddVisitor visitor(this, destinations);
    spec.accept(visitor);
    transaction.commit();
}

void
DepList::add(const PackageDepSpec & spec, tr1::shared_ptr<const DestinationsSet> destinations)
{
    TreeLeaf<SetSpecTree, PackageDepSpec> l(tr1::shared_ptr<PackageDepSpec>(new PackageDepSpec(spec)));
    add(l, destinations);
}

void
DepList::add_package(const tr1::shared_ptr<const PackageID> & p, tr1::shared_ptr<const DepTag> tag,
        const PackageDepSpec & pds, tr1::shared_ptr<DependencySpecTree::ConstItem> conditions,
        tr1::shared_ptr<const DestinationsSet> destinations)
{
    Context context("When adding package '" + stringify(*p) + "':");

    Save<MergeList::iterator> save_merge_list_insert_position(&_imp->merge_list_insert_position);

    /* create our merge list entry. insert pre deps before ourself in the list. insert
     * post deps after ourself, and after any provides. */

    MergeList::iterator our_merge_entry_position(
            _imp->merge_list.insert(_imp->merge_list_insert_position,
                DepListEntry::create()
                .package_id(p)
                .generation(_imp->merge_list_generation)
                .state(dle_no_deps)
                .tags(tr1::shared_ptr<DepListEntryTags>(new DepListEntryTags))
                .destination(p->virtual_for_key() ? tr1::shared_ptr<Repository>() : find_destination(*p, destinations))
                .associated_entry(0)
                .handled(p->virtual_for_key() ?
                    tr1::shared_ptr<DepListEntryHandled>(new DepListEntryNoHandlingRequired) :
                    tr1::shared_ptr<DepListEntryHandled>(new DepListEntryUnhandled))
                .kind(p->virtual_for_key() ? dlk_virtual : dlk_package))),
        our_merge_entry_post_position(our_merge_entry_position);

    _imp->merge_list_index.insert(std::make_pair(p->name(), our_merge_entry_position));

    if (tag)
        our_merge_entry_position->tags->insert(DepTagEntry::create()
                .generation(_imp->merge_list_generation)
                .tag(tag));

    if (_imp->opts->dependency_tags && _imp->current_package_id())
        our_merge_entry_position->tags->insert(DepTagEntry::create()
                .tag(tr1::shared_ptr<DepTag>(new DependencyDepTag(_imp->current_package_id(), pds, conditions)))
                .generation(_imp->merge_list_generation));

    Save<MergeList::const_iterator> save_current_merge_list_entry(&_imp->current_merge_list_entry,
            our_merge_entry_position);

    _imp->merge_list_insert_position = our_merge_entry_position;

    /* add provides */
    if (p->provide_key())
    {
        DepSpecFlattener<ProvideSpecTree, PackageDepSpec> f(_imp->env, *_imp->current_package_id());
        p->provide_key()->value()->accept(f);

        if (f.begin() != f.end() && ! DistributionData::get_instance()->distribution_from_string(
                    _imp->env->default_distribution())->support_old_style_virtuals)
            throw DistributionConfigurationError("Package '" + stringify(*p) + "' has PROVIDEs, but this distribution "
                    "does not support old style virtuals");

        for (DepSpecFlattener<ProvideSpecTree, PackageDepSpec>::ConstIterator i(f.begin()), i_end(f.end()) ; i != i_end ; ++i)
        {
            tr1::shared_ptr<VersionRequirements> v(new VersionRequirements);
            v->push_back(VersionRequirement(vo_equal, p->version()));
            tr1::shared_ptr<PackageDepSpec> pp(new PackageDepSpec(
                        tr1::shared_ptr<QualifiedPackageName>(new QualifiedPackageName((*i)->text())),
                        tr1::shared_ptr<CategoryNamePart>(),
                        tr1::shared_ptr<PackageNamePart>(),
                        v, vr_and));

            std::pair<MergeListIndex::iterator, MergeListIndex::iterator> z;
            if (pp->package_ptr())
                z = _imp->merge_list_index.equal_range(*pp->package_ptr());
            else
                z = std::make_pair(_imp->merge_list_index.begin(), _imp->merge_list_index.end());

            MergeListIndex::iterator zz(std::find_if(z.first, z.second,
                MatchDepListEntryAgainstPackageDepSpec(_imp->env, *pp)));

            if (zz != z.second)
                continue;

            our_merge_entry_post_position = _imp->merge_list.insert(next(our_merge_entry_post_position),
                    DepListEntry(DepListEntry::create()
                        .package_id(_imp->env->package_database()->fetch_repository(
                                RepositoryName("virtuals"))->make_virtuals_interface->make_virtual_package_id(
                                QualifiedPackageName((*i)->text()), p))
                        .generation(_imp->merge_list_generation)
                        .state(dle_has_all_deps)
                        .tags(tr1::shared_ptr<DepListEntryTags>(new DepListEntryTags))
                        .destination(tr1::shared_ptr<Repository>())
                        .associated_entry(&*_imp->current_merge_list_entry)
                        .handled(make_shared_ptr(new DepListEntryNoHandlingRequired))
                        .kind(dlk_provided)));
            _imp->merge_list_index.insert(std::make_pair((*i)->text(), our_merge_entry_post_position));
        }
    }

    /* add suggests */
    if (_imp->opts->suggested == dl_suggested_show && p->suggested_dependencies_key())
    {
        Context c("When showing suggestions:");
        Save<MergeList::iterator> suggest_save_merge_list_insert_position(&_imp->merge_list_insert_position,
                next(our_merge_entry_position));
        ShowSuggestVisitor visitor(this, destinations, _imp->env, _imp->current_package_id(), _imp->opts->dependency_tags);
        p->suggested_dependencies_key()->value()->accept(visitor);
    }

    /* add pre dependencies */
    if (p->build_dependencies_key())
        add_predeps(*p->build_dependencies_key()->value(), _imp->opts->uninstalled_deps_pre, "build", destinations);
    if (p->run_dependencies_key())
        add_predeps(*p->run_dependencies_key()->value(), _imp->opts->uninstalled_deps_runtime, "run", destinations);
    if (p->post_dependencies_key())
        add_predeps(*p->post_dependencies_key()->value(), _imp->opts->uninstalled_deps_post, "post", destinations);
    if (_imp->opts->suggested == dl_suggested_install && p->suggested_dependencies_key())
        add_predeps(*p->suggested_dependencies_key()->value(), _imp->opts->uninstalled_deps_suggested, "suggest", destinations);

    our_merge_entry_position->state = dle_has_pre_deps;
    _imp->merge_list_insert_position = next(our_merge_entry_post_position);

    /* add post dependencies */
    if (p->build_dependencies_key())
        add_postdeps(*p->build_dependencies_key()->value(), _imp->opts->uninstalled_deps_pre, "build", destinations);
    if (p->run_dependencies_key())
        add_postdeps(*p->run_dependencies_key()->value(), _imp->opts->uninstalled_deps_runtime, "run", destinations);
    if (p->post_dependencies_key())
        add_postdeps(*p->post_dependencies_key()->value(), _imp->opts->uninstalled_deps_post, "post", destinations);

    if (_imp->opts->suggested == dl_suggested_install && p->suggested_dependencies_key())
        add_postdeps(*p->suggested_dependencies_key()->value(), _imp->opts->uninstalled_deps_suggested, "suggest", destinations);

    our_merge_entry_position->state = dle_has_all_deps;
}

void
DepList::add_error_package(const tr1::shared_ptr<const PackageID> & p, const DepListEntryKind kind,
        const PackageDepSpec & pds, tr1::shared_ptr<DependencySpecTree::ConstItem> conditions)
{
    std::pair<MergeListIndex::iterator, MergeListIndex::const_iterator> pp(
            _imp->merge_list_index.equal_range(p->name()));

    for ( ; pp.second != pp.first ; ++pp.first)
    {
        if (pp.first->second->kind == kind && *pp.first->second->package_id == *p)
        {
            if (_imp->current_package_id())
                pp.first->second->tags->insert(DepTagEntry::create()
                        .tag(tr1::shared_ptr<DepTag>(new DependencyDepTag(_imp->current_package_id(), pds, conditions)))
                        .generation(_imp->merge_list_generation));
            return;
        }
    }

    MergeList::iterator our_merge_entry_position(
            _imp->merge_list.insert(_imp->merge_list.begin(),
                DepListEntry::create()
                .package_id(p)
                .generation(_imp->merge_list_generation)
                .state(dle_has_all_deps)
                .tags(tr1::shared_ptr<DepListEntryTags>(new DepListEntryTags))
                .destination(tr1::shared_ptr<Repository>())
                .associated_entry(&*_imp->current_merge_list_entry)
                .handled(make_shared_ptr(new DepListEntryNoHandlingRequired))
                .kind(kind)));

    if (_imp->current_package_id())
        our_merge_entry_position->tags->insert(DepTagEntry::create()
                .tag(tr1::shared_ptr<DepTag>(new DependencyDepTag(_imp->current_package_id(), pds, conditions)))
                .generation(_imp->merge_list_generation));

    _imp->merge_list_index.insert(std::make_pair(p->name(), our_merge_entry_position));
}

void
DepList::add_suggested_package(const tr1::shared_ptr<const PackageID> & p,
        const PackageDepSpec & pds, tr1::shared_ptr<DependencySpecTree::ConstItem> conditions,
        const tr1::shared_ptr<const DestinationsSet> destinations)
{
    std::pair<MergeListIndex::iterator, MergeListIndex::const_iterator> pp(
            _imp->merge_list_index.equal_range(p->name()));

    for ( ; pp.second != pp.first ; ++pp.first)
    {
        if ((pp.first->second->kind == dlk_suggested || pp.first->second->kind == dlk_already_installed
                    || pp.first->second->kind == dlk_package || pp.first->second->kind == dlk_provided
                    || pp.first->second->kind == dlk_subpackage) && *pp.first->second->package_id == *p)
            return;
    }

    MergeList::iterator our_merge_entry_position(
            _imp->merge_list.insert(_imp->merge_list_insert_position,
                DepListEntry::create()
                .package_id(p)
                .generation(_imp->merge_list_generation)
                .state(dle_has_all_deps)
                .tags(tr1::shared_ptr<DepListEntryTags>(new DepListEntryTags))
                .destination(find_destination(*p, destinations))
                .associated_entry(&*_imp->current_merge_list_entry)
                .handled(make_shared_ptr(new DepListEntryNoHandlingRequired))
                .kind(dlk_suggested)));

    if (_imp->current_package_id())
        our_merge_entry_position->tags->insert(DepTagEntry::create()
                .tag(tr1::shared_ptr<DepTag>(new DependencyDepTag(_imp->current_package_id(), pds, conditions)))
                .generation(_imp->merge_list_generation));

    _imp->merge_list_index.insert(std::make_pair(p->name(), our_merge_entry_position));
}

void
DepList::add_predeps(DependencySpecTree::ConstItem & d, const DepListDepsOption opt, const std::string & s,
        tr1::shared_ptr<const DestinationsSet> destinations)
{
    if (dl_deps_pre == opt || dl_deps_pre_or_post == opt)
    {
        try
        {
            add_in_role(d, s + " dependencies as pre dependencies", destinations);
        }
        catch (const DepListError & e)
        {
            if (dl_deps_pre == opt)
                throw;
            else
                Log::get_instance()->message(ll_warning, lc_context, "Dropping " + s + " dependencies to "
                        "post dependencies because of exception '" + e.message() + "' (" + e.what() + ")");
        }
    }
}

void
DepList::add_postdeps(DependencySpecTree::ConstItem & d, const DepListDepsOption opt, const std::string & s,
        tr1::shared_ptr<const DestinationsSet> destinations)
{
    if (dl_deps_pre_or_post == opt || dl_deps_post == opt || dl_deps_try_post == opt)
    {
        try
        {
            try
            {
                add_in_role(d, s + " dependencies as post dependencies", destinations);
            }
            catch (const CircularDependencyError &)
            {
                Save<DepListCircularOption> save_circular(&_imp->opts->circular,
                        _imp->opts->circular == dl_circular_discard_silently ?
                        dl_circular_discard_silently : dl_circular_discard);
                Save<MergeList::iterator> save_merge_list_insert_position(&_imp->merge_list_insert_position,
                        _imp->merge_list.end());
                add_in_role(d, s + " dependencies as post dependencies with cycle breaking", destinations);
            }
        }
        catch (const DepListError & e)
        {
            if (dl_deps_try_post != opt)
                throw;
            else
                Log::get_instance()->message(ll_warning, lc_context, "Ignoring " + s +
                        " dependencies due to exception '" + e.message() + "' (" + e.what() + ")");
        }
    }
}

void
DepList::add_already_installed_package(const tr1::shared_ptr<const PackageID> & p, tr1::shared_ptr<const DepTag> tag,
        const PackageDepSpec & pds, tr1::shared_ptr<DependencySpecTree::ConstItem> conditions,
        const tr1::shared_ptr<const DestinationsSet> destinations)
{
    Context context("When adding installed package '" + stringify(*p) + "':");

    Save<MergeList::iterator> save_merge_list_insert_position(&_imp->merge_list_insert_position);

    MergeList::iterator our_merge_entry(_imp->merge_list.insert(_imp->merge_list_insert_position,
                DepListEntry::create()
                .package_id(p)
                .generation(_imp->merge_list_generation)
                .tags(tr1::shared_ptr<DepListEntryTags>(new DepListEntryTags))
                .state(dle_has_pre_deps)
                .destination(tr1::shared_ptr<Repository>())
                .associated_entry(0)
                .handled(make_shared_ptr(new DepListEntryNoHandlingRequired))
                .kind(dlk_already_installed)));
    _imp->merge_list_index.insert(std::make_pair(p->name(), our_merge_entry));

    if (tag)
        our_merge_entry->tags->insert(DepTagEntry::create()
                .generation(_imp->merge_list_generation)
                .tag(tag));

    if (_imp->opts->dependency_tags && _imp->current_package_id())
        our_merge_entry->tags->insert(DepTagEntry::create()
                .tag(tr1::shared_ptr<DepTag>(new DependencyDepTag(_imp->current_package_id(), pds, conditions)))
                .generation(_imp->merge_list_generation));

    Save<MergeList::const_iterator> save_current_merge_list_entry(&_imp->current_merge_list_entry,
            our_merge_entry);

    if (p->build_dependencies_key())
        add_predeps(*p->build_dependencies_key()->value(), _imp->opts->installed_deps_pre, "build", destinations);
    if (p->run_dependencies_key())
        add_predeps(*p->run_dependencies_key()->value(), _imp->opts->installed_deps_runtime, "run", destinations);
    if (p->post_dependencies_key())
        add_predeps(*p->post_dependencies_key()->value(), _imp->opts->installed_deps_post, "post", destinations);

    our_merge_entry->state = dle_has_pre_deps;
    _imp->merge_list_insert_position = next(our_merge_entry);

    if (p->build_dependencies_key())
        add_postdeps(*p->build_dependencies_key()->value(), _imp->opts->installed_deps_pre, "build", destinations);
    if (p->run_dependencies_key())
        add_postdeps(*p->run_dependencies_key()->value(), _imp->opts->installed_deps_runtime, "run", destinations);
    if (p->post_dependencies_key())
        add_postdeps(*p->post_dependencies_key()->value(), _imp->opts->installed_deps_post, "post", destinations);
}

namespace
{
    bool is_scm(const QualifiedPackageName & n)
    {
        std::string pkg(stringify(n.package));
        switch (pkg.length())
        {
            case 0:
            case 1:
            case 2:
            case 3:
                return false;

            default:
                if (0 == pkg.compare(pkg.length() - 6, 6, "-darcs"))
                    return true;

            case 5:
                if (0 == pkg.compare(pkg.length() - 5, 5, "-live"))
                    return true;

            case 4:
                if (0 == pkg.compare(pkg.length() - 4, 4, "-cvs"))
                    return true;
                if (0 == pkg.compare(pkg.length() - 4, 4, "-svn"))
                    return true;
                return false;
        }
    }
}

bool
DepList::prefer_installed_over_uninstalled(const PackageID & installed,
        const PackageID & uninstalled)
{
    do
    {
        switch (_imp->opts->target_type)
        {
            case dl_target_package:
                if (! _imp->current_package_id())
                    return false;

                if (is_top_level_target(uninstalled))
                    return false;

                continue;

            case dl_target_set:
                continue;

            case last_dl_target:
                ;
        }

        throw InternalError(PALUDIS_HERE, "Bad target_type value '" + stringify(_imp->opts->target_type) + "'");
    } while (false);

    if (dl_reinstall_always == _imp->opts->reinstall)
            return false;

    if (dl_upgrade_as_needed == _imp->opts->upgrade)
        return true;

    if (dl_reinstall_scm_never != _imp->opts->reinstall_scm)
        if (uninstalled.version() == installed.version() &&
                (installed.version().is_scm() || is_scm(installed.name())))
        {
            static time_t current_time(time(0)); /* static to avoid weirdness */
            time_t installed_time(current_time);
            if (installed.installed_time_key())
                installed_time = installed.installed_time_key()->value();

            do
            {
                switch (_imp->opts->reinstall_scm)
                {
                    case dl_reinstall_scm_always:
                        return false;

                    case dl_reinstall_scm_daily:
                        if (current_time - installed_time > (24 * 60 * 60))
                            return false;
                        continue;

                    case dl_reinstall_scm_weekly:
                        if (current_time - installed_time > (24 * 60 * 60 * 7))
                            return false;
                        continue;

                    case dl_reinstall_scm_never:
                        ; /* nothing */

                    case last_dl_reinstall_scm:
                        ;
                }

                throw InternalError(PALUDIS_HERE, "Bad value for opts->reinstall_scm");
            } while (false);
        }

    /* use != rather than > to correctly force a downgrade when packages are
     * removed. */
    if (uninstalled.version() != installed.version())
        return false;

    if (dl_reinstall_if_use_changed == _imp->opts->reinstall)
    {
        std::set<UseFlagName> use_common;
        if (installed.iuse_key() && uninstalled.iuse_key())
            std::set_intersection(
                    installed.iuse_key()->value()->begin(), installed.iuse_key()->value()->end(),
                    uninstalled.iuse_key()->value()->begin(), uninstalled.iuse_key()->value()->end(),
                    transform_inserter(std::inserter(use_common, use_common.end()),
                        paludis::tr1::mem_fn(&IUseFlag::flag)));

        for (std::set<UseFlagName>::const_iterator f(use_common.begin()), f_end(use_common.end()) ;
                f != f_end ; ++f)
            if (_imp->env->query_use(*f, installed) != _imp->env->query_use(*f, uninstalled))
                return false;
    }

    return true;
}

bool
DepList::already_installed(const DependencySpecTree::ConstItem & spec,
        tr1::shared_ptr<const DestinationsSet> destinations) const
{
    QueryVisitor visitor(this, destinations, _imp->env, _imp->current_package_id());
    spec.accept(visitor);
    return visitor.result();
}

DepList::Iterator
DepList::begin()
{
    return Iterator(_imp->merge_list.begin());
}

DepList::Iterator
DepList::end()
{
    return Iterator(_imp->merge_list.end());
}

DepList::ConstIterator
DepList::begin() const
{
    return ConstIterator(_imp->merge_list.begin());
}

DepList::ConstIterator
DepList::end() const
{
    return ConstIterator(_imp->merge_list.end());
}

bool
DepList::is_top_level_target(const PackageID & e) const
{
    if (! _imp->current_top_level_target)
        throw InternalError(PALUDIS_HERE, "current_top_level_target not set?");

    return match_package_in_set(*_imp->env, *_imp->current_top_level_target, e);
}

namespace
{
    struct IsError
    {
        bool operator() (const DepListEntry & e) const
        {
            switch (e.kind)
            {
                case dlk_virtual:
                case dlk_package:
                case dlk_provided:
                case dlk_already_installed:
                case dlk_subpackage:
                case dlk_suggested:
                    return false;

                case dlk_block:
                case dlk_masked:
                    return true;

                case last_dlk:
                    ;
            }

            throw InternalError(PALUDIS_HERE, "Bad e.kind");
        }
    };
}

bool
DepList::has_errors() const
{
    return end() != std::find_if(begin(), end(), IsError());
}

tr1::shared_ptr<Repository>
DepList::find_destination(const PackageID & p,
        tr1::shared_ptr<const DestinationsSet> dd)
{
    for (DestinationsSet::ConstIterator d(dd->begin()), d_end(dd->end()) ;
             d != d_end ; ++d)
        if ((*d)->destination_interface)
            if ((*d)->destination_interface->is_suitable_destination_for(p))
                return *d;

    throw NoDestinationError(p, dd);
}

bool
DepList::replaced(const PackageID & m) const
{
    std::pair<MergeListIndex::const_iterator, MergeListIndex::const_iterator> p(
            _imp->merge_list_index.equal_range(m.name()));

    PackageDepSpec spec(make_shared_ptr(new QualifiedPackageName(m.name())));
    while (p.second != ((p.first = std::find_if(p.first, p.second,
                        MatchDepListEntryAgainstPackageDepSpec(_imp->env, spec)))))
    {
        if (p.first->second->package_id->slot() != m.slot())
            p.first = next(p.first);
        else
            return true;
    }

    return false;
}

bool
DepList::match_on_list(const PackageDepSpec & a) const
{
    std::pair<MergeListIndex::const_iterator, MergeListIndex::const_iterator> p;
    if (a.package_ptr())
        p = _imp->merge_list_index.equal_range(*a.package_ptr());
    else
        p = std::make_pair(_imp->merge_list_index.begin(), _imp->merge_list_index.end());

    return p.second != std::find_if(p.first, p.second,
            MatchDepListEntryAgainstPackageDepSpec(_imp->env, a));
}

bool
paludis::is_viable_any_child(const Environment & env, const tr1::shared_ptr<const PackageID> & id,
        const DependencySpecTree::ConstItem & i)
{
    const UseDepSpec * const u(get_const_item(i)->as_use_dep_spec());
    if (0 != u)
        return (id ? env.query_use(u->flag(), *id) : false) ^ u->inverse();
    else
        return true;
}

