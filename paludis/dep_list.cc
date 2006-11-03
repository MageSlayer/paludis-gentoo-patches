/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <paludis/dep_atom.hh>
#include <paludis/dep_atom_flattener.hh>
#include <paludis/dep_list.hh>
#include <paludis/match_package.hh>
#include <paludis/util/collection_concrete.hh>
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

#include <paludis/dep_list-sr.cc>

DepListError::DepListError(const std::string & m) throw () :
    Exception(m)
{
}

AllMaskedError::AllMaskedError(const std::string & q) throw () :
    DepListError("Error searching for '" + q + "': no available versions"),
    _query(q)
{
}

UseRequirementsNotMetError::UseRequirementsNotMetError(const std::string & q) throw () :
    DepListError("Error searching for '" + q + "': use requirements are not met"),
    _query(q)
{
}

BlockError::BlockError(const std::string & msg) throw () :
    DepListError("Block: " + msg)
{
}

CircularDependencyError::CircularDependencyError(const std::string & msg) throw () :
    DepListError("Circular dependency: " + msg)
{
}

DepListOptions::DepListOptions() :
    reinstall(dl_reinstall_never),
    target_type(dl_target_package),
    upgrade(dl_upgrade_always),
    installed_deps_pre(dl_deps_discard),
    installed_deps_runtime(dl_deps_try_post),
    installed_deps_post(dl_deps_try_post),
    uninstalled_deps_pre(dl_deps_pre),
    uninstalled_deps_runtime(dl_deps_pre_or_post),
    uninstalled_deps_post(dl_deps_post),
    circular(dl_circular_error),
    dependency_tags(false)
{
    /* when changing the above, also see src/paludis/command_line.cc. */
}

namespace paludis
{
    typedef std::list<DepListEntry> MergeList;

    template<>
    struct Implementation<DepList> :
        InternalCounted<Implementation<DepList> >
    {
        const Environment * const env;
        DepListOptions opts;

        MergeList merge_list;
        MergeList::const_iterator current_merge_list_entry;
        MergeList::iterator merge_list_insert_position;
        long merge_list_generation;

        const PackageDatabaseEntry * current_pde() const
        {
            if (current_merge_list_entry != merge_list.end())
                return &current_merge_list_entry->package;
            return 0;
        }

        Implementation(const Environment * const e, const DepListOptions & o) :
            env(e),
            opts(o),
            current_merge_list_entry(merge_list.end()),
            merge_list_insert_position(merge_list.end()),
            merge_list_generation(0)
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
            DepListEntryTags::Pointer t(new DepListEntryTags::Concrete);
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
            long & _generation;
            int _initial_generation;
            bool _committed;

        public:
            DepListTransaction(MergeList & l, long & g) :
                _list(l),
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
                if (! _committed)
                {
                    _list.remove_if(GenerationGreaterThan(_initial_generation));
                    std::for_each(_list.begin(), _list.end(), RemoveTagsWithGenerationGreaterThan(_initial_generation));
                }
            }
    };

    struct MatchDepListEntryAgainstPackageDepAtom
    {
        const Environment * const env;
        const PackageDepAtom * const a;

        MatchDepListEntryAgainstPackageDepAtom(const Environment * const ee,
                const PackageDepAtom * const aa) :
            env(ee),
            a(aa)
        {
        }

        bool operator() (const DepListEntry & e)
        {
            return match_package(env, a, e);
        }
    };

    struct IsViableAnyDepAtomChild
    {
        const Environment * const env;
        const PackageDatabaseEntry * const pde;

        IsViableAnyDepAtomChild(const Environment * const e, const PackageDatabaseEntry * const p) :
            env(e),
            pde(p)
        {
        }

        bool operator() (PackageDepAtom::ConstPointer atom)
        {
            const UseDepAtom * const u(atom->as_use_dep_atom());
            if (0 != u)
                return env->query_use(u->flag(), pde) ^ u->inverse();
            else
                return true;
        }
    };
}

struct DepList::QueryVisitor :
    DepAtomVisitorTypes::ConstVisitor
{
    bool result;
    const DepList * const d;

    QueryVisitor(const DepList * const dd) :
        result(true),
        d(dd)
    {
    }

    void visit(const PlainTextDepAtom * const) PALUDIS_ATTRIBUTE((noreturn));
    void visit(const PackageDepAtom * const);
    void visit(const UseDepAtom * const);
    void visit(const AnyDepAtom * const);
    void visit(const BlockDepAtom * const);
    void visit(const AllDepAtom * const);
};

void
DepList::QueryVisitor::visit(const PlainTextDepAtom * const)
{
    throw InternalError(PALUDIS_HERE, "Got PlainTextDepAtom?");
}

void
DepList::QueryVisitor::visit(const PackageDepAtom * const a)
{
    /* a pda matches either if we're already installed, or if we will be installed
     * by the time the current point in the dep list is reached. */

    if (! d->_imp->env->package_database()->query(*a, is_installed_only)->empty())
        result = true;
    else if (d->_imp->merge_list.end() != std::find_if(
                d->_imp->merge_list.begin(),
                d->_imp->merge_list.end(),
                MatchDepListEntryAgainstPackageDepAtom(d->_imp->env, a)))
        result = true;
    else
        result = false;
}

void
DepList::QueryVisitor::visit(const UseDepAtom * const a)
{
    /* for use? ( ) dep atoms, return true if we're not enabled, so that
     * weird || ( ) cases work. */
    if (d->_imp->env->query_use(a->flag(), d->_imp->current_pde()) ^ a->inverse())
    {
        result = true;
        for (CompositeDepAtom::Iterator c(a->begin()), c_end(a->end()) ; c != c_end ; ++c)
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
DepList::QueryVisitor::visit(const AnyDepAtom * const a)
{
    /* empty || ( ) must resolve to true */
    std::list<DepAtom::ConstPointer> viable_children;
    std::copy(a->begin(), a->end(), filter_inserter(std::back_inserter(viable_children),
                IsViableAnyDepAtomChild(d->_imp->env, d->_imp->current_pde())));

    result = true;
    for (std::list<DepAtom::ConstPointer>::const_iterator c(viable_children.begin()),
            c_end(viable_children.end()) ; c != c_end ; ++c)
    {
        (*c)->accept(this);
        if (result)
            return;
    }
}

void
DepList::QueryVisitor::visit(const BlockDepAtom * const a)
{
    a->blocked_atom()->accept(this);
    result = !result;
}

void
DepList::QueryVisitor::visit(const AllDepAtom * const a)
{
    for (CompositeDepAtom::Iterator c(a->begin()), c_end(a->end()) ; c != c_end ; ++c)
    {
        (*c)->accept(this);
        if (! result)
            return;
    }
}

struct DepList::AddVisitor :
    DepAtomVisitorTypes::ConstVisitor
{
    DepList * const d;

    AddVisitor(DepList * const dd) :
        d(dd)
    {
    }

    void visit(const PlainTextDepAtom * const) PALUDIS_ATTRIBUTE((noreturn));
    void visit(const PackageDepAtom * const);
    void visit(const UseDepAtom * const);
    void visit(const AnyDepAtom * const);
    void visit(const BlockDepAtom * const);
    void visit(const AllDepAtom * const);
};

void
DepList::AddVisitor::visit(const PlainTextDepAtom * const)
{
    throw InternalError(PALUDIS_HERE, "Got PlainTextDepAtom?");
}

void
DepList::AddVisitor::visit(const PackageDepAtom * const a)
{
    Context context("When adding PackageDepAtom '" + stringify(*a) + "':");

    /* find already installed things */
    PackageDatabaseEntryCollection::ConstPointer already_installed(d->_imp->env->package_database()->query(
                *a, is_installed_only));

    /* are we already on the merge list? */
    MergeList::iterator existing_merge_list_entry(std::find_if(d->_imp->merge_list.begin(),
                d->_imp->merge_list.end(), MatchDepListEntryAgainstPackageDepAtom(d->_imp->env, a)));
    if (existing_merge_list_entry != d->_imp->merge_list.end())
    {
        /* tag it */
        if (a->tag())
            existing_merge_list_entry->tags->insert(DepTagEntry::create()
                    .tag(a->tag())
                    .generation(d->_imp->merge_list_generation));

        if (d->_imp->opts.dependency_tags && d->_imp->current_pde())
            existing_merge_list_entry->tags->insert(DepTagEntry::create()
                    .tag(DepTag::Pointer(new DependencyDepTag(*d->_imp->current_pde())))
                    .generation(d->_imp->merge_list_generation));

        /* have our deps been merged already, or is this a circular dep? */
        if (dle_no_deps == existing_merge_list_entry->state)
        {
            /* is a sufficiently good version installed? */
            if (! already_installed->empty())
                return;

            if (d->_imp->opts.circular == dl_circular_discard)
            {
                Log::get_instance()->message(ll_warning, lc_context, "Dropping circular dependency on '"
                        + stringify(existing_merge_list_entry->package) + "'");
                return;
            }
            throw CircularDependencyError("Atom '" + stringify(*a) + "' matched by merge list entry '" +
                    stringify(existing_merge_list_entry->package) + "', which does not yet have its "
                    "dependencies installed");
        }
        else
            return;
    }

    /* find installable candidates, and find the best visible candidate */
    const PackageDatabaseEntry * best_visible_candidate(0);
    PackageDatabaseEntryCollection::ConstPointer installable_candidates(
            d->_imp->env->package_database()->query(*a, is_uninstalled_only));

    for (PackageDatabaseEntryCollection::ReverseIterator p(installable_candidates->rbegin()),
            p_end(installable_candidates->rend()) ; p != p_end ; ++p)
        if (! d->_imp->env->mask_reasons(*p).any())
        {
            best_visible_candidate = &*p;
            break;
        }

    /* no installable candidates. if we're already installed, that's ok (except for top level
     * package targets), otherwise error. */
    if (! best_visible_candidate)
    {
        if (already_installed->empty())
        {
            if (a->use_requirements_ptr() && d->_imp->env->package_database()->query(
                        a->without_use_requirements(), is_either))
                throw UseRequirementsNotMetError(stringify(*a));
            else
                throw AllMaskedError(stringify(*a));
        }
        else
        {
            // todo: top level
            Log::get_instance()->message(ll_warning, lc_context, "No visible packages matching '"
                    + stringify(*a) + "', falling back to installed package '"
                    + stringify(*already_installed->last()) + "'");
            d->add_already_installed_package(*already_installed->last(), a->tag());
            return;
        }
    }

    SlotName slot(d->_imp->env->package_database()->fetch_repository(best_visible_candidate->repository)->
            version_metadata(best_visible_candidate->name, best_visible_candidate->version)->slot);
    PackageDatabaseEntryCollection::Pointer already_installed_in_same_slot(
            new PackageDatabaseEntryCollection::Concrete);
    for (PackageDatabaseEntryCollection::Iterator aa(already_installed->begin()),
            aa_end(already_installed->end()) ; aa != aa_end ; ++aa)
        if (d->_imp->env->package_database()->fetch_repository(aa->repository)->
                version_metadata(aa->name, aa->version)->slot == slot)
            already_installed_in_same_slot->insert(*aa);

    /* we have an already installed version. do we want to use it? */
    if (! already_installed_in_same_slot->empty())
    {
        if (d->prefer_installed_over_uninstalled(*already_installed_in_same_slot->last(), *best_visible_candidate))
        {
            Log::get_instance()->message(ll_debug, lc_context, "Taking installed package '"
                    + stringify(*already_installed_in_same_slot->last()) + "' over '" + stringify(*best_visible_candidate) + "'");
            d->add_already_installed_package(*already_installed_in_same_slot->last(), a->tag());
            return;
        }
        else
            Log::get_instance()->message(ll_debug, lc_context, "Not taking installed package '"
                    + stringify(*already_installed_in_same_slot->last()) + "' over '" + stringify(*best_visible_candidate) + "'");
    }
    else
        Log::get_instance()->message(ll_debug, lc_context, "No installed packages in SLOT '"
                + stringify(slot) + "', taking uninstalled package '" + stringify(*best_visible_candidate) + "'");

    d->add_package(*best_visible_candidate, a->tag());
}

void
DepList::AddVisitor::visit(const UseDepAtom * const a)
{
    if (d->_imp->env->query_use(a->flag(), d->_imp->current_pde()) ^ a->inverse())
        std::for_each(a->begin(), a->end(), accept_visitor(this));
}

void
DepList::AddVisitor::visit(const AnyDepAtom * const a)
{
    /* annoying requirement: || ( foo? ( ... ) ) resolves to empty if !foo. */
    std::list<DepAtom::ConstPointer> viable_children;
    std::copy(a->begin(), a->end(), filter_inserter(std::back_inserter(viable_children),
                IsViableAnyDepAtomChild(d->_imp->env, d->_imp->current_pde())));

    if (viable_children.empty())
        return;

    /* see if any of our children is already installed. if any is, add it so that
     * any upgrades kick in */
    for (std::list<DepAtom::ConstPointer>::const_iterator c(viable_children.begin()),
            c_end(viable_children.end()) ; c != c_end ; ++c)
    {
        if (d->already_installed(*c))
        {
            d->add(*c);
            return;
        }
    }

    /* install first available viable option */
    for (std::list<DepAtom::ConstPointer>::const_iterator c(viable_children.begin()),
            c_end(viable_children.end()) ; c != c_end ; ++c)
    {
        try
        {
            d->add(*c);
            return;
        }
        catch (const DepListError & e)
        {
        }
    }

    Log::get_instance()->message(ll_warning, lc_context, "No resolvable item in || ( ) block. Using "
            "first item for error message");
    d->add(*viable_children.begin());
}

void
DepList::AddVisitor::visit(const BlockDepAtom * const a)
{
    if (! d->already_installed(a->blocked_atom()))
        return;

    Context context("When checking BlockDepAtom '!" + stringify(*a->blocked_atom()) + "':");

    /* special case: the provider of virtual/blah can DEPEND upon !virtual/blah. */
    /* special case: foo/bar can DEPEND upon !foo/bar. */

    if (d->_imp->current_pde())
    {
        if (d->_imp->current_pde()->name == a->blocked_atom()->package())
        {
            Log::get_instance()->message(ll_debug, lc_context, "Ignoring self block '"
                    + stringify(*a->blocked_atom()) + "' for package '"
                    + stringify(*d->_imp->current_pde()) + "'");
            return;
        }

        VersionMetadata::ConstPointer metadata(d->_imp->env->package_database()->fetch_repository(
                    d->_imp->current_pde()->repository)->version_metadata(d->_imp->current_pde()->name,
                    d->_imp->current_pde()->version));
        if (metadata->get_ebuild_interface())
        {
            bool skip(false);
            DepAtomFlattener f(d->_imp->env, d->_imp->current_pde(), metadata->get_ebuild_interface()->provide());
            for (DepAtomFlattener::Iterator i(f.begin()), i_end(f.end()) ; i != i_end && ! skip ; ++i)
                if ((*i)->text() == stringify(a->blocked_atom()->package()))
                    skip = true;

            if (skip)
            {
                Log::get_instance()->message(ll_debug, lc_context,
                        "Ignoring self block (via PROVIDE) '" + stringify(*a->blocked_atom())
                        + "' for package '" + stringify(*d->_imp->current_pde()) + "'");
                return;
            }
        }
    }

    throw BlockError(stringify(*a->blocked_atom()));
}

void
DepList::AddVisitor::visit(const AllDepAtom * const a)
{
    std::for_each(a->begin(), a->end(), accept_visitor(this));
}

DepList::DepList(const Environment * const e, const DepListOptions & o) :
    PrivateImplementationPattern<DepList>(new Implementation<DepList>(e, o)),
    options(_imp->opts)
{
}

DepList::~DepList()
{
}

void
DepList::clear()
{
    _imp.assign(new Implementation<DepList>(_imp->env, _imp->opts));
}

void
DepList::add_in_role(DepAtom::ConstPointer atom, const std::string & role)
{
    Context context("When adding " + role + ":");
    add(atom);
}

void
DepList::add(DepAtom::ConstPointer atom)
{
    DepListTransaction transaction(_imp->merge_list, _imp->merge_list_generation);
    AddVisitor visitor(this);
    atom->accept(&visitor);
    transaction.commit();
}

void
DepList::add_package(const PackageDatabaseEntry & p, DepTag::ConstPointer tag)
{
    Context context("When adding package '" + stringify(p) + "':");

    Save<MergeList::iterator> save_merge_list_insert_position(&_imp->merge_list_insert_position);

    VersionMetadata::ConstPointer metadata(_imp->env->package_database()->fetch_repository(
                p.repository)->version_metadata(p.name, p.version));

    /* create our merge list entry. insert pre deps before ourself in the list. insert
     * post deps after ourself, and after any provides. */
    MergeList::iterator our_merge_entry_position(
            _imp->merge_list.insert(_imp->merge_list_insert_position,
                DepListEntry::create()
                .package(p)
                .metadata(metadata)
                .generation(_imp->merge_list_generation)
                .state(dle_no_deps)
                .tags(DepListEntryTags::Pointer(new DepListEntryTags::Concrete))
                .already_installed(false))),
        our_merge_entry_post_position(our_merge_entry_position);

    if (tag)
        our_merge_entry_position->tags->insert(DepTagEntry::create()
                .generation(_imp->merge_list_generation)
                .tag(tag));

    if (_imp->opts.dependency_tags && _imp->current_pde())
        our_merge_entry_position->tags->insert(DepTagEntry::create()
                .tag(DepTag::Pointer(new DependencyDepTag(*_imp->current_pde())))
                .generation(_imp->merge_list_generation));

    Save<MergeList::const_iterator> save_current_merge_list_entry(&_imp->current_merge_list_entry,
            our_merge_entry_position);

    _imp->merge_list_insert_position = our_merge_entry_position;

    /* add provides */
    if (metadata->get_ebuild_interface())
    {
        DepAtomFlattener f(_imp->env, _imp->current_pde(), metadata->get_ebuild_interface()->provide());
        for (DepAtomFlattener::Iterator i(f.begin()), i_end(f.end()) ; i != i_end ; ++i)
        {
            PackageDepAtom::Pointer pp(new PackageDepAtom("=" + (*i)->text() + "-" + stringify(p.version)));
            if (_imp->merge_list.end() != std::find_if(_imp->merge_list.begin(),
                        _imp->merge_list.end(), MatchDepListEntryAgainstPackageDepAtom(_imp->env,
                            pp.raw_pointer())))
                continue;

            VersionMetadata::ConstPointer m(0);

            if (_imp->env->package_database()->fetch_repository(RepositoryName("virtuals"))->has_version(
                        QualifiedPackageName((*i)->text()), p.version))
                m = _imp->env->package_database()->fetch_repository(RepositoryName("virtuals"))->version_metadata(
                        QualifiedPackageName((*i)->text()), p.version);
            else
            {
                VersionMetadata::Pointer mm(0);
                mm.assign(new VersionMetadata::Virtual(metadata->deps.parser,
                            PackageDatabaseEntry(p.name, p.version, RepositoryName("virtuals"))));
                mm->slot = metadata->slot;
                m = mm;
            }

            our_merge_entry_post_position = _imp->merge_list.insert(next(our_merge_entry_post_position),
                    DepListEntry(DepListEntry::create()
                        .package(PackageDatabaseEntry((*i)->text(), p.version, RepositoryName("virtuals")))
                        .metadata(m)
                        .generation(_imp->merge_list_generation)
                        .state(dle_has_all_deps)
                        .tags(DepListEntryTags::Pointer(new DepListEntryTags::Concrete))
                        .already_installed(false)));
        }
    }

    /* add pre dependencies */
    add_predeps(metadata->deps.build_depend(), _imp->opts.uninstalled_deps_pre, "build");
    add_predeps(metadata->deps.run_depend(), _imp->opts.uninstalled_deps_runtime, "run");
    add_predeps(metadata->deps.post_depend(), _imp->opts.uninstalled_deps_post, "post");

    our_merge_entry_position->state = dle_has_pre_deps;
    _imp->merge_list_insert_position = next(our_merge_entry_post_position);

    add_postdeps(metadata->deps.build_depend(), _imp->opts.uninstalled_deps_pre, "build");
    add_postdeps(metadata->deps.run_depend(), _imp->opts.uninstalled_deps_runtime, "run");
    add_postdeps(metadata->deps.post_depend(), _imp->opts.uninstalled_deps_post, "post");

    our_merge_entry_position->state = dle_has_all_deps;
}

void
DepList::add_predeps(DepAtom::ConstPointer d, const DepListDepsOption opt, const std::string & s)
{
    if (dl_deps_pre == opt || dl_deps_pre_or_post == opt)
    {
        try
        {
            add_in_role(d, s + " dependencies as pre dependencies");
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
DepList::add_postdeps(DepAtom::ConstPointer d, const DepListDepsOption opt, const std::string & s)
{
    if (dl_deps_pre_or_post == opt || dl_deps_post == opt || dl_deps_try_post == opt)
    {
        try
        {
            add_in_role(d, s + " dependencies as post dependencies");
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
DepList::add_already_installed_package(const PackageDatabaseEntry & p, DepTag::ConstPointer tag)
{
    Context context("When adding installed package '" + stringify(p) + "':");

    Save<MergeList::iterator> save_merge_list_insert_position(&_imp->merge_list_insert_position);
    VersionMetadata::ConstPointer metadata(_imp->env->package_database()->fetch_repository(
                p.repository)->version_metadata(p.name, p.version));

    MergeList::iterator our_merge_entry(_imp->merge_list.insert(_imp->merge_list_insert_position,
                DepListEntry::create()
                .package(p)
                .metadata(metadata)
                .generation(_imp->merge_list_generation)
                .tags(DepListEntryTags::Pointer(new DepListEntryTags::Concrete))
                .state(dle_has_pre_deps)
                .already_installed(true)));

    if (tag)
        our_merge_entry->tags->insert(DepTagEntry::create()
                .generation(_imp->merge_list_generation)
                .tag(tag));

    if (_imp->opts.dependency_tags && _imp->current_pde())
        our_merge_entry->tags->insert(DepTagEntry::create()
                .tag(DepTag::Pointer(new DependencyDepTag(*_imp->current_pde())))
                .generation(_imp->merge_list_generation));

    Save<MergeList::const_iterator> save_current_merge_list_entry(&_imp->current_merge_list_entry,
            our_merge_entry);

    add_predeps(metadata->deps.build_depend(), _imp->opts.installed_deps_pre, "build");
    add_predeps(metadata->deps.run_depend(), _imp->opts.installed_deps_runtime, "run");
    add_predeps(metadata->deps.post_depend(), _imp->opts.installed_deps_post, "post");

    our_merge_entry->state = dle_has_pre_deps;
    _imp->merge_list_insert_position = next(our_merge_entry);

    add_postdeps(metadata->deps.build_depend(), _imp->opts.installed_deps_pre, "build");
    add_postdeps(metadata->deps.run_depend(), _imp->opts.installed_deps_runtime, "run");
    add_postdeps(metadata->deps.post_depend(), _imp->opts.installed_deps_post, "post");
}

bool
DepList::prefer_installed_over_uninstalled(const PackageDatabaseEntry & installed,
        const PackageDatabaseEntry & uninstalled)
{
    if (dl_target_package == _imp->opts.target_type)
        if (! _imp->current_pde())
            return false;

    if (dl_reinstall_always == _imp->opts.reinstall)
            return false;

    if (dl_upgrade_as_needed == _imp->opts.upgrade)
        return true;

    /* use != rather than > to correctly force a downgrade when packages are
     * removed. */
    if (uninstalled.version != installed.version)
        return false;

    if (dl_reinstall_if_use_changed == _imp->opts.reinstall)
    {
        const EbuildVersionMetadata * const evm_i(_imp->env->package_database()->fetch_repository(
                    installed.repository)->version_metadata(installed.name, installed.version)->get_ebuild_interface());
        const EbuildVersionMetadata * const evm_u(_imp->env->package_database()->fetch_repository(
                    uninstalled.repository)->version_metadata(uninstalled.name, uninstalled.version)->get_ebuild_interface());

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
DepList::already_installed(DepAtom::ConstPointer atom) const
{
    return already_installed(atom.raw_pointer());
}

bool
DepList::already_installed(const DepAtom * const atom) const
{
    QueryVisitor visitor(this);
    atom->accept(&visitor);
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

