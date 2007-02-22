/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <paludis/dep_spec.hh>
#include <paludis/dep_spec_flattener.hh>
#include <paludis/dep_spec_pretty_printer.hh>
#include <paludis/dep_list/dep_list.hh>
#include <paludis/dep_list/exceptions.hh>
#include <paludis/dep_list/range_rewriter.hh>
#include <paludis/match_package.hh>
#include <paludis/hashed_containers.hh>
#include <paludis/util/collection_concrete.hh>
#include <paludis/util/compare.hh>
#include <paludis/util/iterator.hh>
#include <paludis/util/join.hh>
#include <paludis/util/log.hh>
#include <paludis/util/save.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/tokeniser.hh>

#include <algorithm>
#include <functional>
#include <vector>
#include <set>

using namespace paludis;

#include <paludis/dep_list/dep_list-sr.cc>

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
        std::tr1::shared_ptr<DepListOptions> opts;

        MergeList merge_list;
        MergeList::const_iterator current_merge_list_entry;
        MergeList::iterator merge_list_insert_position;
        long merge_list_generation;

        MergeListIndex merge_list_index;

        std::tr1::shared_ptr<const DepSpec> current_top_level_target;

        bool throw_on_blocker;

        const PackageDatabaseEntry * current_pde() const
        {
            if (current_merge_list_entry != merge_list.end())
                return &current_merge_list_entry->package;
            return 0;
        }

        Implementation(const Environment * const e, const DepListOptions & o) :
            env(e),
            opts(new DepListOptions(o)),
            current_merge_list_entry(merge_list.end()),
            merge_list_insert_position(merge_list.end()),
            merge_list_generation(0),
            throw_on_blocker(o.blocks == dl_blocks_error)
        {
        }
    };
}

namespace
{
    class FakedVirtualVersionMetadata :
        public VersionMetadata,
        public VersionMetadataVirtualInterface
    {
        public:
            FakedVirtualVersionMetadata(const SlotName & s, const PackageDatabaseEntry & e) :
                VersionMetadata(
                        VersionMetadataBase(s, "", "", "paludis-1"),
                        VersionMetadataCapabilities::create()
                        .cran_interface(0)
                        .virtual_interface(this)
                        .ebuild_interface(0)
                        .deps_interface(0)
                        .origins_interface(0)
                        .ebin_interface(0)
                        .license_interface(0)),
                VersionMetadataVirtualInterface(e)
            {
            }
    };

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
            std::tr1::shared_ptr<DepListEntryTags> t(new DepListEntryTags::Concrete);
            GenerationGreaterThan pred(g);
            for (DepListEntryTags::Iterator i(e.tags->begin()), i_end(e.tags->end()) ;
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
                                    _index.equal_range(i->package.name)) ; p.first != p.second ; )
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
        const PackageDepSpec * const a;

        MatchDepListEntryAgainstPackageDepSpec(const Environment * const ee,
                const PackageDepSpec * const aa) :
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
                    return match_package(*env, *a, e.second->package);

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

    struct IsViableAnyDepSpecChild
    {
        const Environment * const env;
        const PackageDatabaseEntry * const pde;

        IsViableAnyDepSpecChild(const Environment * const e, const PackageDatabaseEntry * const p) :
            env(e),
            pde(p)
        {
        }

        bool operator() (std::tr1::shared_ptr<const DepSpec> spec)
        {
            const UseDepSpec * const u(spec->as_use_dep_spec());
            if (0 != u)
                return env->query_use(u->flag(), pde) ^ u->inverse();
            else
                return true;
        }
    };

    struct IsInterestingPDADepSpecChild
    {
        const Environment * const env;

        IsInterestingPDADepSpecChild(const Environment * const e) :
            env(e)
        {
        }

        bool operator() (std::tr1::shared_ptr<const DepSpec> spec)
        {
            const PackageDepSpec * const u(spec->as_package_dep_spec());
            if (0 != u)
            {
                return ! env->package_database()->query(PackageDepSpec(u->package()),
                        is_installed_only, qo_whatever)->empty();
            }
            else
                return false;
        }
    };
}

struct DepList::QueryVisitor :
    DepSpecVisitorTypes::ConstVisitor
{
    bool result;
    const DepList * const d;
    std::tr1::shared_ptr<const DestinationsCollection> destinations;

    QueryVisitor(const DepList * const dd, std::tr1::shared_ptr<const DestinationsCollection> ddd) :
        result(true),
        d(dd),
        destinations(ddd)
    {
    }

    void visit(const PlainTextDepSpec * const) PALUDIS_ATTRIBUTE((noreturn));
    void visit(const PackageDepSpec * const);
    void visit(const UseDepSpec * const);
    void visit(const AnyDepSpec * const);
    void visit(const BlockDepSpec * const);
    void visit(const AllDepSpec * const);
};

void
DepList::QueryVisitor::visit(const PlainTextDepSpec * const)
{
    throw InternalError(PALUDIS_HERE, "Got PlainTextDepSpec?");
}

void
DepList::QueryVisitor::visit(const PackageDepSpec * const a)
{
    /* a pda matches if we'll be installed by the time we reach the current point. This
     * means that merely being installed is not enough, if we'll have our version changed
     * by something in the merge list. */

    result = false;

    // TODO: check destinations
    std::tr1::shared_ptr<const PackageDatabaseEntryCollection> matches(d->_imp->env->package_database()->query(
                *a, is_installed_only, qo_whatever));

    for (PackageDatabaseEntryCollection::Iterator m(matches->begin()), m_end(matches->end()) ;
            m != m_end ; ++m)
    {
        /* check that we haven't been replaced by something in the same slot */
        std::tr1::shared_ptr<const VersionMetadata> vm(d->_imp->env->package_database()->fetch_repository(m->repository)->
                version_metadata(m->name, m->version));
        SlotName slot(vm->slot);

        std::pair<MergeListIndex::const_iterator, MergeListIndex::const_iterator> p(
                d->_imp->merge_list_index.equal_range(a->package()));

        bool replaced(false);
        PackageDepSpec spec(a->package());
        while (p.second != ((p.first = std::find_if(p.first, p.second,
                            MatchDepListEntryAgainstPackageDepSpec(d->_imp->env, &spec)))))
        {
            if (p.first->second->metadata->slot != slot)
                p.first = next(p.first);
            else
            {
                replaced = true;
                break;
            }
        }

        if (! replaced)
        {
            result = true;
            return;
        }
    }

    /* check the merge list for any new packages that match */
    std::pair<MergeListIndex::const_iterator, MergeListIndex::const_iterator> p(
            d->_imp->merge_list_index.equal_range(a->package()));

    if (p.second != std::find_if(p.first, p.second,
                MatchDepListEntryAgainstPackageDepSpec(d->_imp->env, a)))
    {
        // TODO: check destination
        result = true;
        return;
    }
}

void
DepList::QueryVisitor::visit(const UseDepSpec * const a)
{
    /* for use? ( ) dep specs, return true if we're not enabled, so that
     * weird || ( ) cases work. */
    if (d->_imp->env->query_use(a->flag(), d->_imp->current_pde()) ^ a->inverse())
    {
        result = true;
        for (CompositeDepSpec::Iterator c(a->begin()), c_end(a->end()) ; c != c_end ; ++c)
        {
            (*c)->accept(this);
            if (! result)
                return;
        }
    }
    else
        result = true;
}

void
DepList::QueryVisitor::visit(const AnyDepSpec * const a)
{
    /* empty || ( ) must resolve to true */
    std::list<std::tr1::shared_ptr<const DepSpec> > viable_children;
    std::copy(a->begin(), a->end(), filter_inserter(std::back_inserter(viable_children),
                IsViableAnyDepSpecChild(d->_imp->env, d->_imp->current_pde())));

    RangeRewriter r;
    std::for_each(viable_children.begin(), viable_children.end(), accept_visitor(&r));
    if (r.spec())
    {
        viable_children.clear();
        viable_children.push_back(r.spec());
    }

    result = true;
    for (std::list<std::tr1::shared_ptr<const DepSpec> >::const_iterator c(viable_children.begin()),
            c_end(viable_children.end()) ; c != c_end ; ++c)
    {
        (*c)->accept(this);
        if (result)
            return;
    }
}

void
DepList::QueryVisitor::visit(const BlockDepSpec * const a)
{
    a->blocked_spec()->accept(this);
    result = !result;
}

void
DepList::QueryVisitor::visit(const AllDepSpec * const a)
{
    for (CompositeDepSpec::Iterator c(a->begin()), c_end(a->end()) ; c != c_end ; ++c)
    {
        (*c)->accept(this);
        if (! result)
            return;
    }
}

struct DepList::AddVisitor :
    DepSpecVisitorTypes::ConstVisitor,
    DepSpecVisitorTypes::ConstVisitor::VisitChildren<AddVisitor, AllDepSpec>
{
    DepList * const d;
    std::tr1::shared_ptr<const DestinationsCollection> destinations;

    AddVisitor(DepList * const dd, std::tr1::shared_ptr<const DestinationsCollection> ddd) :
        d(dd),
        destinations(ddd)
    {
    }

    void visit(const PlainTextDepSpec * const) PALUDIS_ATTRIBUTE((noreturn));
    void visit(const PackageDepSpec * const);
    void visit(const UseDepSpec * const);
    void visit(const AnyDepSpec * const);
    void visit(const BlockDepSpec * const);
    using DepSpecVisitorTypes::ConstVisitor::VisitChildren<AddVisitor, AllDepSpec>::visit;
};

void
DepList::AddVisitor::visit(const PlainTextDepSpec * const)
{
    throw InternalError(PALUDIS_HERE, "Got PlainTextDepSpec?");
}

void
DepList::AddVisitor::visit(const PackageDepSpec * const a)
{
    Context context("When adding PackageDepSpec '" + stringify(*a) + "':");

    /* find already installed things */
    // TODO: check destinations
    std::tr1::shared_ptr<const PackageDatabaseEntryCollection> already_installed(d->_imp->env->package_database()->query(
                *a, is_installed_only, qo_order_by_version));

    /* are we already on the merge list? */
    std::pair<MergeListIndex::iterator, MergeListIndex::iterator> q(
            d->_imp->merge_list_index.equal_range(a->package()));
    MergeListIndex::iterator qq(std::find_if(q.first, q.second,
                MatchDepListEntryAgainstPackageDepSpec(d->_imp->env, a)));

    MergeList::iterator existing_merge_list_entry(qq == q.second ? d->_imp->merge_list.end() : qq->second);
    if (existing_merge_list_entry != d->_imp->merge_list.end())
    {
        /* tag it */
        if (a->tag())
            existing_merge_list_entry->tags->insert(DepTagEntry::create()
                    .tag(a->tag())
                    .generation(d->_imp->merge_list_generation));

        if (d->_imp->opts->dependency_tags && d->_imp->current_pde())
            existing_merge_list_entry->tags->insert(DepTagEntry::create()
                    .tag(std::tr1::shared_ptr<DepTag>(new DependencyDepTag(*d->_imp->current_pde())))
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
                        + stringify(existing_merge_list_entry->package) + "'");
                return;
            }
            else if (d->_imp->opts->circular == dl_circular_discard_silently)
                return;

            throw CircularDependencyError("Atom '" + stringify(*a) + "' matched by merge list entry '" +
                    stringify(existing_merge_list_entry->package) + "', which does not yet have its "
                    "dependencies installed");
        }
        else
            return;
    }

    /* find installable candidates, and find the best visible candidate */
    const PackageDatabaseEntry * best_visible_candidate(0);
    std::tr1::shared_ptr<const PackageDatabaseEntryCollection> installable_candidates(
            d->_imp->env->package_database()->query(*a, is_installable_only, qo_order_by_version));

    for (PackageDatabaseEntryCollection::ReverseIterator p(installable_candidates->rbegin()),
            p_end(installable_candidates->rend()) ; p != p_end ; ++p)
        if (! d->_imp->env->mask_reasons(*p).any())
        {
            best_visible_candidate = &*p;
            break;
        }

    /* are we allowed to override mask reasons? */
    if (! best_visible_candidate && d->_imp->opts->override_masks.any())
    {
        DepListOverrideMask next(static_cast<DepListOverrideMask>(0));
        DepListOverrideMasks masks_to_override;

        do
        {
            while (next != last_dl_override)
            {
                if (masks_to_override.test(next))
                    next = static_cast<DepListOverrideMask>(static_cast<int>(next) + 1);
                else if (d->_imp->opts->override_masks.test(next))
                {
                    masks_to_override.set(next);
                    break;
                }
                else
                    next = static_cast<DepListOverrideMask>(static_cast<int>(next) + 1);
            }

            if (next == last_dl_override)
                break;

            MaskReasons mask_mask;
            if (masks_to_override.test(dl_override_repository_masks))
                mask_mask.set(mr_repository_mask);
            if (masks_to_override.test(dl_override_profile_masks))
                mask_mask.set(mr_profile_mask);
            if (masks_to_override.test(dl_override_licenses))
                mask_mask.set(mr_license);
            mask_mask.set(mr_by_association);
            mask_mask.flip();

            bool override_tilde_keywords(masks_to_override.test(dl_override_tilde_keywords));
            bool override_unkeyworded(masks_to_override.test(dl_override_unkeyworded));

            for (PackageDatabaseEntryCollection::ReverseIterator p(installable_candidates->rbegin()),
                    p_end(installable_candidates->rend()) ; p != p_end ; ++p)
            {
                if (! (d->_imp->env->mask_reasons(*p, override_tilde_keywords, override_unkeyworded)
                            & mask_mask).any())
                {
                    d->add_error_package(*p, dlk_masked);
                    best_visible_candidate = &*p;
                    break;
                }
            }
        } while (! best_visible_candidate);
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
                    if (! d->_imp->current_pde())
                        can_fall_back = false;
                    else if (already_installed->empty())
                        can_fall_back = true;
                    else
                        can_fall_back = ! d->is_top_level_target(*already_installed->last());

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
            if (! a->use_requirements_ptr())
                throw AllMaskedError(stringify(*a));

            std::tr1::shared_ptr<const PackageDatabaseEntryCollection> match_except_reqs(d->_imp->env->package_database()->query(
                        *a->without_use_requirements(), is_any, qo_whatever));

            for (PackageDatabaseEntryCollection::Iterator i(match_except_reqs->begin()),
                    i_end(match_except_reqs->end()) ; i != i_end ; ++i)
                if (! (d->_imp->env->mask_reasons(*i).any()))
                    throw UseRequirementsNotMetError(stringify(*a));

            throw AllMaskedError(stringify(*a));
        }
        else
        {
            Log::get_instance()->message(ll_warning, lc_context, "No visible packages matching '"
                    + stringify(*a) + "', falling back to installed package '"
                    + stringify(*already_installed->last()) + "'");
            d->add_already_installed_package(*already_installed->last(), a->tag(), destinations);
            return;
        }
    }

    std::tr1::shared_ptr<const VersionMetadata> best_visible_candidate_metadata(
            d->_imp->env->package_database()->fetch_repository(best_visible_candidate->repository)->
            version_metadata(best_visible_candidate->name, best_visible_candidate->version));
    SlotName slot(best_visible_candidate_metadata->slot);
    std::string best_visible_candidate_as_string(stringify(*best_visible_candidate));
    if (best_visible_candidate_metadata->virtual_interface)
        best_visible_candidate_as_string.append(" (for " + stringify(
                        best_visible_candidate_metadata->virtual_interface->virtual_for) + ")");

    std::tr1::shared_ptr<PackageDatabaseEntryCollection> already_installed_in_same_slot(
            new PackageDatabaseEntryCollection::Concrete);
    for (PackageDatabaseEntryCollection::Iterator aa(already_installed->begin()),
            aa_end(already_installed->end()) ; aa != aa_end ; ++aa)
        if (d->_imp->env->package_database()->fetch_repository(aa->repository)->
                version_metadata(aa->name, aa->version)->slot == slot)
            already_installed_in_same_slot->push_back(*aa);
    /* no need to sort already_installed_in_same_slot here, although if the above is
     * changed then check that this still holds... */

    /* we have an already installed version. do we want to use it? */
    if (! already_installed_in_same_slot->empty())
    {
        if (d->prefer_installed_over_uninstalled(*already_installed_in_same_slot->last(), *best_visible_candidate))
        {
            Log::get_instance()->message(ll_debug, lc_context, "Taking installed package '"
                    + stringify(*already_installed_in_same_slot->last()) + "' over '" +
                    best_visible_candidate_as_string + "'");
            d->add_already_installed_package(*already_installed_in_same_slot->last(), a->tag(), destinations);
            return;
        }
        else
            Log::get_instance()->message(ll_debug, lc_context, "Not taking installed package '"
                    + stringify(*already_installed_in_same_slot->last()) + "' over '" +
                    best_visible_candidate_as_string + "'");
    }
    else if ((! already_installed->empty()) && (dl_new_slots_as_needed == d->_imp->opts->new_slots))
    {
        /* we have an already installed, but not in the same slot, and our options
         * allow us to take this. */
        if (d->prefer_installed_over_uninstalled(*already_installed->last(), *best_visible_candidate))
        {
            Log::get_instance()->message(ll_debug, lc_context, "Taking installed package '"
                    + stringify(*already_installed->last()) + "' over '" + best_visible_candidate_as_string
                    + "' (in different slot)");
            d->add_already_installed_package(*already_installed->last(), a->tag(), destinations);
            return;
        }
        else
            Log::get_instance()->message(ll_debug, lc_context, "Not taking installed package '"
                    + stringify(*already_installed->last()) + "' over '" +
                    best_visible_candidate_as_string + "' (in different slot)");
    }
    else
        Log::get_instance()->message(ll_debug, lc_context, "No installed packages in SLOT '"
                + stringify(slot) + "', taking uninstalled package '" + best_visible_candidate_as_string + "'");

    /* if this is a downgrade, make sure that that's ok */
    switch (d->_imp->opts->downgrade)
    {
        case dl_downgrade_as_needed:
            break;

        case dl_downgrade_error:
        case dl_downgrade_warning:
            {
                std::tr1::shared_ptr<PackageDatabaseEntryCollection> are_we_downgrading(
                        d->_imp->env->package_database()->query(PackageDepSpec(
                                stringify(a->package()) + ":" + stringify(slot)),
                            is_installed_only, qo_order_by_version));

                if (are_we_downgrading->empty())
                    break;

                if (are_we_downgrading->last()->version <= best_visible_candidate->version)
                    break;

                std::tr1::shared_ptr<const VersionMetadata> are_we_downgrading_last_metadata(
                        d->_imp->env->package_database()->fetch_repository(
                            are_we_downgrading->last()->repository)->version_metadata(
                            are_we_downgrading->last()->name, are_we_downgrading->last()->version));
                std::string are_we_downgrading_last_as_string(stringify(*are_we_downgrading->last()));
                if (are_we_downgrading_last_metadata->virtual_interface)
                    are_we_downgrading_last_as_string.append(" (for " + stringify(
                                    are_we_downgrading_last_metadata->virtual_interface->virtual_for) + ")");

                if (d->_imp->opts->downgrade == dl_downgrade_error)
                    throw DowngradeNotAllowedError(best_visible_candidate_as_string,
                            are_we_downgrading_last_as_string);

                Log::get_instance()->message(ll_warning, lc_context, "Downgrade to '" +
                        best_visible_candidate_as_string
                        + "' from '" + are_we_downgrading_last_as_string + "' forced");
            }
            break;

        case last_dl_downgrade:
            ;
    }

    d->add_package(*best_visible_candidate, a->tag(), destinations);
}

void
DepList::AddVisitor::visit(const UseDepSpec * const a)
{
    if (d->_imp->opts->use == dl_use_deps_standard)
    {
        if (d->_imp->env->query_use(a->flag(), d->_imp->current_pde()) ^ a->inverse())
            std::for_each(a->begin(), a->end(), accept_visitor(this));
    }
    else
    {
        RepositoryUseInterface * u(0);
        if ((! d->_imp->current_pde()) || (! ((u = d->_imp->env->package_database()->fetch_repository(
                                d->_imp->current_pde()->repository)->use_interface))))
            std::for_each(a->begin(), a->end(), accept_visitor(this));
        else if (a->inverse())
        {
            if (! u->query_use_force(a->flag(), d->_imp->current_pde()))
                std::for_each(a->begin(), a->end(), accept_visitor(this));
        }
        else
        {
            if (! u->query_use_mask(a->flag(), d->_imp->current_pde()))
                std::for_each(a->begin(), a->end(), accept_visitor(this));
        }
    }
}

void
DepList::AddVisitor::visit(const AnyDepSpec * const a)
{
    /* annoying requirement: || ( foo? ( ... ) ) resolves to empty if !foo. */
    std::list<std::tr1::shared_ptr<const DepSpec> > viable_children;
    std::copy(a->begin(), a->end(), filter_inserter(std::back_inserter(viable_children),
                IsViableAnyDepSpecChild(d->_imp->env, d->_imp->current_pde())));

    if (viable_children.empty())
        return;

    RangeRewriter r;
    std::for_each(viable_children.begin(), viable_children.end(), accept_visitor(&r));
    if (r.spec())
    {
        viable_children.clear();
        viable_children.push_back(r.spec());
    }

    /* see if any of our children is already installed. if any is, add it so that
     * any upgrades kick in */
    for (std::list<std::tr1::shared_ptr<const DepSpec> >::const_iterator c(viable_children.begin()),
            c_end(viable_children.end()) ; c != c_end ; ++c)
    {
        if (d->already_installed(**c, destinations))
        {
            d->add(*c, destinations);
            return;
        }
    }

    /* if we have something like || ( a >=b-2 ) and b-1 is installed, try to go for
     * the b-2 bit first */
    std::list<std::tr1::shared_ptr<const DepSpec> > pda_children;
    std::copy(viable_children.begin(), viable_children.end(),
            filter_inserter(std::back_inserter(pda_children), IsInterestingPDADepSpecChild(d->_imp->env)));

    for (std::list<std::tr1::shared_ptr<const DepSpec> >::const_iterator c(pda_children.begin()),
            c_end(pda_children.end()) ; c != c_end ; ++c)
    {
        try
        {
            Save<bool> save_t(&d->_imp->throw_on_blocker,
                    dl_blocks_discard_completely != d->_imp->opts->blocks);
            Save<DepListOverrideMasks> save_o(&d->_imp->opts->override_masks, DepListOverrideMasks());
            d->add(*c, destinations);
            return;
        }
        catch (const DepListError &)
        {
        }
    }

    /* install first available viable option */
    for (std::list<std::tr1::shared_ptr<const DepSpec> >::const_iterator c(viable_children.begin()),
            c_end(viable_children.end()) ; c != c_end ; ++c)
    {
        try
        {
            Save<bool> save_t(&d->_imp->throw_on_blocker,
                    dl_blocks_discard_completely != d->_imp->opts->blocks);
            Save<DepListOverrideMasks> save_o(&d->_imp->opts->override_masks, DepListOverrideMasks());
            d->add(*c, destinations);
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
        d->add(*viable_children.begin(), destinations);
    }
}

void
DepList::AddVisitor::visit(const BlockDepSpec * const a)
{
    if (dl_blocks_discard_completely == d->_imp->opts->blocks)
        return;

    // TODO: check destinations

    Context context("When checking BlockDepSpec '!" + stringify(*a->blocked_spec()) + "':");

    PackageDepSpec just_package(a->blocked_spec()->package());
    std::tr1::shared_ptr<const PackageDatabaseEntryCollection> already_installed(d->_imp->env->package_database()->query(
                just_package, is_installed_only, qo_whatever));

    std::list<MergeList::const_iterator> will_be_installed;
    MatchDepListEntryAgainstPackageDepSpec m(d->_imp->env, &just_package);
    for (std::pair<MergeListIndex::const_iterator, MergeListIndex::const_iterator> p(
                d->_imp->merge_list_index.equal_range(a->blocked_spec()->package())) ;
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

    if (already_installed->empty() && will_be_installed.empty())
        return;

    for (PackageDatabaseEntryCollection::Iterator aa(already_installed->begin()),
            aa_end(already_installed->end()) ; aa != aa_end ; ++aa)
    {
        if (! match_package(*d->_imp->env, *a->blocked_spec(), *aa))
            continue;

        std::tr1::shared_ptr<const VersionMetadata> metadata(d->_imp->env->package_database()->fetch_repository(
                    aa->repository)->version_metadata(aa->name, aa->version));
        bool replaced(false);
        for (std::list<MergeList::const_iterator>::const_iterator r(will_be_installed.begin()),
                r_end(will_be_installed.end()) ; r != r_end && ! replaced ; ++r)
            if ((*r)->metadata->slot == metadata->slot)
            {
                /* if it's a virtual, it only replaces if it's the same package. */
                if ((*r)->metadata->virtual_interface)
                {
                    if ((*r)->metadata->virtual_interface->virtual_for.name == aa->name)
                        replaced = true;
                }
                else
                    replaced = true;
            }

        if (replaced)
            continue;

        /* ignore if it's a virtual/blah (not <virtual/blah-1) block and it's blocking
         * ourself */
        if (! (a->blocked_spec()->version_requirements_ptr() || a->blocked_spec()->slot_ptr()
                    || a->blocked_spec()->use_requirements_ptr() || a->blocked_spec()->repository_ptr())
                && d->_imp->current_pde())
        {
            if (aa->name == d->_imp->current_pde()->name)
                continue;

            if (metadata->virtual_interface &&
                    metadata->virtual_interface->virtual_for.name == d->_imp->current_pde()->name)
                continue;
        }

        switch (d->_imp->throw_on_blocker ? dl_blocks_error : d->_imp->opts->blocks)
        {
            case dl_blocks_error:
                throw BlockError(stringify(*a->blocked_spec()));

            case dl_blocks_discard:
                Log::get_instance()->message(ll_warning, lc_context, "Discarding block '!"
                        + stringify(*a->blocked_spec()) + "'");
                break;

            case dl_blocks_discard_completely:
                break;

            case dl_blocks_accumulate:
                d->add_error_package(*aa, dlk_block);
                break;

            case last_dl_blocks:
                break;
        }
    }

    for (std::list<MergeList::const_iterator>::const_iterator r(will_be_installed.begin()),
            r_end(will_be_installed.end()) ; r != r_end ; ++r)
    {
        if (! match_package(*d->_imp->env, *a->blocked_spec(), (*r)->package))
            continue;

        /* ignore if it's a virtual/blah (not <virtual/blah-1) block and it's blocking
         * ourself */
        if (! (a->blocked_spec()->version_requirements_ptr() || a->blocked_spec()->slot_ptr()
                    || a->blocked_spec()->use_requirements_ptr() || a->blocked_spec()->repository_ptr())
                && d->_imp->current_pde())
        {
            if ((*r)->package.name == d->_imp->current_pde()->name)
                continue;

            if ((*r)->metadata->virtual_interface &&
                    (*r)->metadata->virtual_interface->virtual_for.name == d->_imp->current_pde()->name)
                continue;
        }

        throw BlockError(stringify(*a->blocked_spec()));
    }
}

struct DepList::ShowSuggestVisitor :
    DepSpecVisitorTypes::ConstVisitor,
    DepSpecVisitorTypes::ConstVisitor::VisitChildren<ShowSuggestVisitor, AllDepSpec>,
    DepSpecVisitorTypes::ConstVisitor::VisitChildren<ShowSuggestVisitor, AnyDepSpec>
{
    DepList * const d;
    std::tr1::shared_ptr<const DestinationsCollection> destinations;

    ShowSuggestVisitor(DepList * const dd, std::tr1::shared_ptr<const DestinationsCollection> ddd) :
        d(dd),
        destinations(ddd)
    {
    }

    void visit(const PlainTextDepSpec * const) PALUDIS_ATTRIBUTE((noreturn));
    void visit(const PackageDepSpec * const);
    void visit(const UseDepSpec * const);
    void visit(const BlockDepSpec * const);
    using DepSpecVisitorTypes::ConstVisitor::VisitChildren<ShowSuggestVisitor, AllDepSpec>::visit;
    using DepSpecVisitorTypes::ConstVisitor::VisitChildren<ShowSuggestVisitor, AnyDepSpec>::visit;
};

void
DepList::ShowSuggestVisitor::visit(const PlainTextDepSpec * const)
{
    throw InternalError(PALUDIS_HERE, "Got PlainTextDepSpec?");
}

void
DepList::ShowSuggestVisitor::visit(const UseDepSpec * const a)
{
    if (d->_imp->env->query_use(a->flag(), d->_imp->current_pde()) ^ a->inverse())
        std::for_each(a->begin(), a->end(), accept_visitor(this));
}

void
DepList::ShowSuggestVisitor::visit(const BlockDepSpec * const)
{
}

void
DepList::ShowSuggestVisitor::visit(const PackageDepSpec * const a)
{
    Context context("When adding suggested dep '" + stringify(*a) + "':");

    std::tr1::shared_ptr<const PackageDatabaseEntryCollection> matches(d->_imp->env->package_database()->query(
                *a, is_installable_only, qo_order_by_version));
    if (matches->empty())
    {
        Log::get_instance()->message(ll_warning, lc_context, "Nothing found for '" + stringify(*a) + "'");
        return;
    }

    for (PackageDatabaseEntryCollection::Iterator m(matches->begin()), m_end(matches->end()) ;
            m != m_end ; ++m)
    {
        if (d->_imp->env->mask_reasons(*m).any())
            continue;

        d->add_suggested_package(*m, destinations);
        return;
    }

    Log::get_instance()->message(ll_warning, lc_context, "Nothing visible found for '" + stringify(*a) + "'");
}

DepList::DepList(const Environment * const e, const DepListOptions & o) :
    PrivateImplementationPattern<DepList>(new Implementation<DepList>(e, o))
{
}

DepList::~DepList()
{
}

std::tr1::shared_ptr<DepListOptions>
DepList::options()
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
DepList::add_in_role(std::tr1::shared_ptr<const DepSpec> spec, const std::string & role,
        std::tr1::shared_ptr<const DestinationsCollection> destinations)
{
    Context context("When adding " + role + ":");
    add(spec, destinations);
}

void
DepList::add(std::tr1::shared_ptr<const DepSpec> spec, std::tr1::shared_ptr<const DestinationsCollection> destinations)
{
    DepListTransaction transaction(_imp->merge_list, _imp->merge_list_index, _imp->merge_list_generation);

    Save<std::tr1::shared_ptr<const DepSpec> > save_current_top_level_target(&_imp->current_top_level_target,
            _imp->current_top_level_target ? _imp->current_top_level_target : spec);

    AddVisitor visitor(this, destinations);
    spec->accept(&visitor);
    transaction.commit();
}

void
DepList::add_package(const PackageDatabaseEntry & p, std::tr1::shared_ptr<const DepTag> tag,
        std::tr1::shared_ptr<const DestinationsCollection> destinations)
{
    Context context("When adding package '" + stringify(p) + "':");

    Save<MergeList::iterator> save_merge_list_insert_position(&_imp->merge_list_insert_position);

    std::tr1::shared_ptr<const VersionMetadata> metadata(_imp->env->package_database()->fetch_repository(
                p.repository)->version_metadata(p.name, p.version));

    /* create our merge list entry. insert pre deps before ourself in the list. insert
     * post deps after ourself, and after any provides. */

    std::tr1::shared_ptr<SortedCollection<DepListEntryDestination> > our_merge_entry_destinations(
            new SortedCollection<DepListEntryDestination>::Concrete);
    if (! metadata->virtual_interface)
        our_merge_entry_destinations->insert(
                DepListEntryDestination(find_destination(p, destinations), _imp->merge_list_generation));

    MergeList::iterator our_merge_entry_position(
            _imp->merge_list.insert(_imp->merge_list_insert_position,
                DepListEntry::create()
                .package(p)
                .metadata(metadata)
                .generation(_imp->merge_list_generation)
                .state(dle_no_deps)
                .tags(std::tr1::shared_ptr<DepListEntryTags>(new DepListEntryTags::Concrete))
                .destinations(our_merge_entry_destinations)
                .associated_entry(0)
                .kind(metadata->virtual_interface ? dlk_virtual : dlk_package))),
        our_merge_entry_post_position(our_merge_entry_position);

    _imp->merge_list_index.insert(std::make_pair(p.name, our_merge_entry_position));

    if (tag)
        our_merge_entry_position->tags->insert(DepTagEntry::create()
                .generation(_imp->merge_list_generation)
                .tag(tag));

    if (_imp->opts->dependency_tags && _imp->current_pde())
        our_merge_entry_position->tags->insert(DepTagEntry::create()
                .tag(std::tr1::shared_ptr<DepTag>(new DependencyDepTag(*_imp->current_pde())))
                .generation(_imp->merge_list_generation));

    Save<MergeList::const_iterator> save_current_merge_list_entry(&_imp->current_merge_list_entry,
            our_merge_entry_position);

    _imp->merge_list_insert_position = our_merge_entry_position;

    /* add provides */
    if (metadata->ebuild_interface)
    {
        DepSpecFlattener f(_imp->env, _imp->current_pde(), metadata->ebuild_interface->provide());
        for (DepSpecFlattener::Iterator i(f.begin()), i_end(f.end()) ; i != i_end ; ++i)
        {
            std::tr1::shared_ptr<PackageDepSpec> pp(new PackageDepSpec("=" + (*i)->text() + "-" + stringify(p.version)));

            std::pair<MergeListIndex::iterator, MergeListIndex::iterator> z(
                    _imp->merge_list_index.equal_range(pp->package()));
            MergeListIndex::iterator zz(std::find_if(z.first, z.second,
                MatchDepListEntryAgainstPackageDepSpec(_imp->env, pp.get())));

            if (z.first != z.second)
                continue;

            std::tr1::shared_ptr<const VersionMetadata> m;

            if (_imp->env->package_database()->fetch_repository(RepositoryName("virtuals"))->has_version(
                        QualifiedPackageName((*i)->text()), p.version))
                m = _imp->env->package_database()->fetch_repository(RepositoryName("virtuals"))->version_metadata(
                        QualifiedPackageName((*i)->text()), p.version);
            else
            {
                std::tr1::shared_ptr<VersionMetadata> mm(new FakedVirtualVersionMetadata(metadata->slot,
                            PackageDatabaseEntry(p.name, p.version, RepositoryName("virtuals"))));
                m = mm;
            }

            our_merge_entry_post_position = _imp->merge_list.insert(next(our_merge_entry_post_position),
                    DepListEntry(DepListEntry::create()
                        .package(PackageDatabaseEntry((*i)->text(), p.version, RepositoryName("virtuals")))
                        .metadata(m)
                        .generation(_imp->merge_list_generation)
                        .state(dle_has_all_deps)
                        .tags(std::tr1::shared_ptr<DepListEntryTags>(new DepListEntryTags::Concrete))
                        .destinations(std::tr1::shared_ptr<SortedCollection<DepListEntryDestination> >(
                                new SortedCollection<DepListEntryDestination>::Concrete))
                        .associated_entry(&*_imp->current_merge_list_entry)
                        .kind(dlk_provided)));
            _imp->merge_list_index.insert(std::make_pair((*i)->text(), our_merge_entry_post_position));
        }
    }

    if (metadata->deps_interface)
    {
        /* add suggests */
        if (_imp->opts->suggested == dl_suggested_show)
        {
            Context c("When showing suggestions:");
            Save<MergeList::iterator> suggest_save_merge_list_insert_position(&_imp->merge_list_insert_position,
                    next(our_merge_entry_position));
            ShowSuggestVisitor visitor(this, destinations);
            metadata->deps_interface->suggested_depend()->accept(&visitor);
        }

        /* add pre dependencies */
        add_predeps(metadata->deps_interface->build_depend(), _imp->opts->uninstalled_deps_pre, "build", destinations);
        add_predeps(metadata->deps_interface->run_depend(), _imp->opts->uninstalled_deps_runtime, "run", destinations);
        add_predeps(metadata->deps_interface->post_depend(), _imp->opts->uninstalled_deps_post, "post", destinations);
        if (_imp->opts->suggested == dl_suggested_install)
            add_predeps(metadata->deps_interface->suggested_depend(), _imp->opts->uninstalled_deps_suggested, "suggest", destinations);
    }

    our_merge_entry_position->state = dle_has_pre_deps;
    _imp->merge_list_insert_position = next(our_merge_entry_post_position);

    if (metadata->deps_interface)
    {
        /* add post dependencies */
        add_postdeps(metadata->deps_interface->build_depend(), _imp->opts->uninstalled_deps_pre, "build", destinations);
        add_postdeps(metadata->deps_interface->run_depend(), _imp->opts->uninstalled_deps_runtime, "run", destinations);
        add_postdeps(metadata->deps_interface->post_depend(), _imp->opts->uninstalled_deps_post, "post", destinations);

        if (_imp->opts->suggested == dl_suggested_install)
            add_postdeps(metadata->deps_interface->suggested_depend(), _imp->opts->uninstalled_deps_suggested, "suggest", destinations);
    }

    our_merge_entry_position->state = dle_has_all_deps;
}

void
DepList::add_error_package(const PackageDatabaseEntry & p, const DepListEntryKind kind)
{
    std::pair<MergeListIndex::iterator, MergeListIndex::const_iterator> pp(
            _imp->merge_list_index.equal_range(p.name));

    for ( ; pp.second != pp.first ; ++pp.first)
    {
        if (pp.first->second->kind == kind && pp.first->second->package == p)
        {
            if (_imp->current_pde())
                pp.first->second->tags->insert(DepTagEntry::create()
                        .tag(std::tr1::shared_ptr<DepTag>(new DependencyDepTag(*_imp->current_pde())))
                        .generation(_imp->merge_list_generation));
            return;
        }
    }

    MergeList::iterator our_merge_entry_position(
            _imp->merge_list.insert(_imp->merge_list.begin(),
                DepListEntry::create()
                .package(p)
                .metadata(_imp->env->package_database()->fetch_repository(
                        p.repository)->version_metadata(p.name, p.version))
                .generation(_imp->merge_list_generation)
                .state(dle_has_all_deps)
                .tags(std::tr1::shared_ptr<DepListEntryTags>(new DepListEntryTags::Concrete))
                .destinations(std::tr1::shared_ptr<SortedCollection<DepListEntryDestination> >(
                        new SortedCollection<DepListEntryDestination>::Concrete))
                .associated_entry(&*_imp->current_merge_list_entry)
                .kind(kind)));

    if (_imp->current_pde())
        our_merge_entry_position->tags->insert(DepTagEntry::create()
                .tag(std::tr1::shared_ptr<DepTag>(new DependencyDepTag(*_imp->current_pde())))
                .generation(_imp->merge_list_generation));

    _imp->merge_list_index.insert(std::make_pair(p.name, our_merge_entry_position));
}

void
DepList::add_suggested_package(const PackageDatabaseEntry & p,
        const std::tr1::shared_ptr<const DestinationsCollection> destinations)
{
    std::pair<MergeListIndex::iterator, MergeListIndex::const_iterator> pp(
            _imp->merge_list_index.equal_range(p.name));

    for ( ; pp.second != pp.first ; ++pp.first)
    {
        if ((pp.first->second->kind == dlk_suggested || pp.first->second->kind == dlk_already_installed
                    || pp.first->second->kind == dlk_package || pp.first->second->kind == dlk_provided
                    || pp.first->second->kind == dlk_subpackage) && pp.first->second->package == p)
            return;
    }

    std::tr1::shared_ptr<SortedCollection<DepListEntryDestination> > our_merge_entry_destinations(
            new SortedCollection<DepListEntryDestination>::Concrete);
    our_merge_entry_destinations->insert(DepListEntryDestination(find_destination(p, destinations),
                _imp->merge_list_generation));

    MergeList::iterator our_merge_entry_position(
            _imp->merge_list.insert(_imp->merge_list_insert_position,
                DepListEntry::create()
                .package(p)
                .metadata(_imp->env->package_database()->fetch_repository(
                        p.repository)->version_metadata(p.name, p.version))
                .generation(_imp->merge_list_generation)
                .state(dle_has_all_deps)
                .tags(std::tr1::shared_ptr<DepListEntryTags>(new DepListEntryTags::Concrete))
                .destinations(our_merge_entry_destinations)
                .associated_entry(&*_imp->current_merge_list_entry)
                .kind(dlk_suggested)));

    if (_imp->current_pde())
        our_merge_entry_position->tags->insert(DepTagEntry::create()
                .tag(std::tr1::shared_ptr<DepTag>(new DependencyDepTag(*_imp->current_pde())))
                .generation(_imp->merge_list_generation));

    _imp->merge_list_index.insert(std::make_pair(p.name, our_merge_entry_position));
}

void
DepList::add_predeps(std::tr1::shared_ptr<const DepSpec> d, const DepListDepsOption opt, const std::string & s,
        std::tr1::shared_ptr<const DestinationsCollection> destinations)
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
DepList::add_postdeps(std::tr1::shared_ptr<const DepSpec> d, const DepListDepsOption opt, const std::string & s,
        std::tr1::shared_ptr<const DestinationsCollection> destinations)
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
DepList::add_already_installed_package(const PackageDatabaseEntry & p, std::tr1::shared_ptr<const DepTag> tag,
        const std::tr1::shared_ptr<const DestinationsCollection> destinations)
{
    Context context("When adding installed package '" + stringify(p) + "':");

    Save<MergeList::iterator> save_merge_list_insert_position(&_imp->merge_list_insert_position);
    std::tr1::shared_ptr<const VersionMetadata> metadata(_imp->env->package_database()->fetch_repository(
                p.repository)->version_metadata(p.name, p.version));

    MergeList::iterator our_merge_entry(_imp->merge_list.insert(_imp->merge_list_insert_position,
                DepListEntry::create()
                .package(p)
                .metadata(metadata)
                .generation(_imp->merge_list_generation)
                .tags(std::tr1::shared_ptr<DepListEntryTags>(new DepListEntryTags::Concrete))
                .state(dle_has_pre_deps)
                .destinations(std::tr1::shared_ptr<SortedCollection<DepListEntryDestination> >(
                        new SortedCollection<DepListEntryDestination>::Concrete))
                .associated_entry(0)
                .kind(dlk_already_installed)));
    _imp->merge_list_index.insert(std::make_pair(p.name, our_merge_entry));

    if (tag)
        our_merge_entry->tags->insert(DepTagEntry::create()
                .generation(_imp->merge_list_generation)
                .tag(tag));

    if (_imp->opts->dependency_tags && _imp->current_pde())
        our_merge_entry->tags->insert(DepTagEntry::create()
                .tag(std::tr1::shared_ptr<DepTag>(new DependencyDepTag(*_imp->current_pde())))
                .generation(_imp->merge_list_generation));

    Save<MergeList::const_iterator> save_current_merge_list_entry(&_imp->current_merge_list_entry,
            our_merge_entry);

    if (metadata->deps_interface)
    {
        add_predeps(metadata->deps_interface->build_depend(), _imp->opts->installed_deps_pre, "build", destinations);
        add_predeps(metadata->deps_interface->run_depend(), _imp->opts->installed_deps_runtime, "run", destinations);
        add_predeps(metadata->deps_interface->post_depend(), _imp->opts->installed_deps_post, "post", destinations);
    }

    our_merge_entry->state = dle_has_pre_deps;
    _imp->merge_list_insert_position = next(our_merge_entry);

    if (metadata->deps_interface)
    {
        add_postdeps(metadata->deps_interface->build_depend(), _imp->opts->installed_deps_pre, "build", destinations);
        add_postdeps(metadata->deps_interface->run_depend(), _imp->opts->installed_deps_runtime, "run", destinations);
        add_postdeps(metadata->deps_interface->post_depend(), _imp->opts->installed_deps_post, "post", destinations);
    }
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
DepList::prefer_installed_over_uninstalled(const PackageDatabaseEntry & installed,
        const PackageDatabaseEntry & uninstalled)
{
    do
    {
        switch (_imp->opts->target_type)
        {
            case dl_target_package:
                if (! _imp->current_pde())
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
        if (uninstalled.version == installed.version &&
                (installed.version.is_scm() || is_scm(installed.name)))
        {
            static time_t current_time(time(0)); /* static to avoid weirdness */
            time_t installed_time(_imp->env->package_database()->fetch_repository(installed.repository
                        )->installed_interface->installed_time(installed.name, installed.version));
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
    if (uninstalled.version != installed.version)
        return false;

    if (dl_reinstall_if_use_changed == _imp->opts->reinstall)
    {
        const VersionMetadataEbuildInterface * const evm_i(_imp->env->package_database()->fetch_repository(
                    installed.repository)->version_metadata(installed.name, installed.version)->ebuild_interface);
        const VersionMetadataEbuildInterface * const evm_u(_imp->env->package_database()->fetch_repository(
                    uninstalled.repository)->version_metadata(uninstalled.name, uninstalled.version)->ebuild_interface);

        std::set<std::string> use_i, use_u, use_common;
        if (evm_i)
            WhitespaceTokeniser::get_instance()->tokenise(evm_i->iuse, std::inserter(use_i, use_i.end()));
        if (evm_u)
            WhitespaceTokeniser::get_instance()->tokenise(evm_u->iuse, std::inserter(use_u, use_u.end()));

        std::set_intersection(use_i.begin(), use_i.end(), use_u.begin(), use_u.end(),
                std::inserter(use_common, use_common.end()));

        for (std::set<std::string>::const_iterator f(use_common.begin()), f_end(use_common.end()) ;
                f != f_end ; ++f)
            if (_imp->env->query_use(UseFlagName(*f), &installed) != _imp->env->query_use(UseFlagName(*f), &uninstalled))
                return false;
    }

    return true;
}

bool
DepList::already_installed(const DepSpec & spec, std::tr1::shared_ptr<const DestinationsCollection> destinations) const
{
    QueryVisitor visitor(this, destinations);
    spec.accept(&visitor);
    return visitor.result;
}

DepList::Iterator
DepList::begin() const
{
    return Iterator(_imp->merge_list.begin());
}

DepList::Iterator
DepList::end() const
{
    return Iterator(_imp->merge_list.end());
}

bool
DepList::is_top_level_target(const PackageDatabaseEntry & e) const
{
    if (! _imp->current_top_level_target)
        throw InternalError(PALUDIS_HERE, "current_top_level_target not set?");

    return match_package_in_heirarchy(*_imp->env, *_imp->current_top_level_target, e);
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

std::tr1::shared_ptr<Repository>
DepList::find_destination(const PackageDatabaseEntry & p,
        std::tr1::shared_ptr<const DestinationsCollection> dd)
{
    for (DestinationsCollection::Iterator d(dd->begin()), d_end(dd->end()) ;
             d != d_end ; ++d)
        if ((*d)->destination_interface)
            if ((*d)->destination_interface->is_suitable_destination_for(p))
                return *d;

    throw NoDestinationError(p, dd);
}

std::tr1::shared_ptr<DestinationsCollection>
paludis::extract_dep_list_entry_destinations(std::tr1::shared_ptr<SortedCollection<DepListEntryDestination> > d)
{
    std::tr1::shared_ptr<DestinationsCollection> result(new DestinationsCollection::Concrete);
    for (SortedCollection<DepListEntryDestination>::Iterator i(d->begin()), i_end(d->end()) ;
            i != i_end ; ++i)
        result->insert(i->destination);
    return result;
}

