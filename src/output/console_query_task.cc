/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include "console_query_task.hh"
#include "licence.hh"
#include "use_flag_pretty_printer.hh"
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/collection_concrete.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>
#include <libwrapiter/libwrapiter_output_iterator.hh>
#include <paludis/query.hh>
#include <paludis/metadata_key.hh>
#include <paludis/eapi.hh>
#include <paludis/package_database.hh>
#include <paludis/dep_spec_pretty_printer.hh>
#include <paludis/environment.hh>
#include <paludis/package_id.hh>
#include <list>

using namespace paludis;

namespace paludis
{
    template<>
    struct Implementation<ConsoleQueryTask>
    {
        const Environment * const env;
        mutable MaskReasons mask_reasons_to_explain;

        Implementation(const Environment * const e) :
            env(e)
        {
        }
    };
}

ConsoleQueryTask::ConsoleQueryTask(const Environment * const e) :
    PrivateImplementationPattern<ConsoleQueryTask>(new Implementation<ConsoleQueryTask>(e))
{
}

ConsoleQueryTask::~ConsoleQueryTask()
{
}

void
ConsoleQueryTask::show(const PackageDepSpec & a, tr1::shared_ptr<const PackageID> display_entry) const
{
    /* prefer the best installed version, then the best visible version, then
     * the best version */
    tr1::shared_ptr<const PackageIDSequence>
        entries(_imp->env->package_database()->query(query::Matches(a), qo_order_by_version)),
        preferred_entries(_imp->env->package_database()->query(
                    query::Matches(a) & query::InstalledAtRoot(_imp->env->root()), qo_order_by_version));
    if (entries->empty())
        throw NoSuchPackageError(stringify(a));
    if (preferred_entries->empty())
        preferred_entries = entries;

    if (! display_entry)
    {
        display_entry = *preferred_entries->last();
        for (PackageIDSequence::Iterator i(preferred_entries->begin()),
                i_end(preferred_entries->end()) ; i != i_end ; ++i)
            if (! _imp->env->mask_reasons(**i).any())
                display_entry = *i;
    }

    display_header(a, display_entry);
    display_versions_by_repository(a, entries, display_entry);
    display_metadata(a, display_entry);
    output_endl();
}

void
ConsoleQueryTask::display_header(const PackageDepSpec & a, const tr1::shared_ptr<const PackageID> & e) const
{
    if (a.version_requirements_ptr() || a.slot_ptr() || a.use_requirements_ptr() ||
            a.repository_ptr())
        output_starred_item(render_as_package_name(stringify(a)));
    else
        output_starred_item(render_as_package_name(stringify(e->name())));
}

void
ConsoleQueryTask::display_versions_by_repository(const PackageDepSpec &,
        tr1::shared_ptr<const PackageIDSequence> entries,
        const tr1::shared_ptr<const PackageID> & display_entry) const
{
    /* find all repository names. */
    RepositoryNameCollection::Concrete repo_names;
    PackageIDSequence::Iterator e(entries->begin()), e_end(entries->end());
    for ( ; e != e_end ; ++e)
        if (repo_names.end() == std::find(repo_names.begin(), repo_names.end(), (*e)->repository()->name()))
            repo_names.push_back((*e)->repository()->name());

    /* display versions, by repository. */
    RepositoryNameCollection::Iterator r(repo_names.begin()), r_end(repo_names.end());
    for ( ; r != r_end ; ++r)
    {
        output_left_column(stringify(*r) + ":");

        std::string old_slot, right_column;
        for (e = entries->begin() ; e != e_end ; ++e)
        {
            Context context("When displaying entry '" + stringify(**e) + "':'");

            if ((*e)->repository()->name() == *r)
            {
                /* show the slot, if we're about to move onto a new slot */
                std::string slot_name(stringify((*e)->slot()));
                if (old_slot.empty())
                    old_slot = slot_name;
                else if (old_slot != slot_name)
                    right_column.append(render_as_slot_name("{:" + old_slot + "} "));
                old_slot = slot_name;

                const MaskReasons masks(_imp->env->mask_reasons(**e));

                if (masks.none())
                    right_column.append(render_as_visible((*e)->canonical_form(idcf_version)));
                else
                {
                    std::string reasons;
                    for (MaskReason m(MaskReason(0)) ; m < last_mr ;
                            m = MaskReason(static_cast<int>(m) + 1))
                    {
                        if (! masks[m])
                            continue;

                        switch (m)
                        {
                            case mr_keyword:
                                reasons.append("K");
                                break;
                            case mr_user_mask:
                                reasons.append("U");
                                break;
                            case mr_profile_mask:
                                reasons.append("P");
                                break;
                            case mr_repository_mask:
                                reasons.append("R");
                                break;
                            case mr_eapi:
                                reasons.append("E");
                                break;
                            case mr_license:
                                reasons.append("L");
                                break;
                            case mr_by_association:
                                reasons.append("A");
                                break;
                            case mr_chost:
                                reasons.append("C");
                                break;
                            case mr_breaks_portage:
                                reasons.append("B");
                                break;
                            case last_mr:
                                break;
                        }
                    }
                    _imp->mask_reasons_to_explain |= masks;
                    right_column.append(render_as_masked("(" + (*e)->canonical_form(idcf_version) + ")" + reasons));
                }

                if (**e == *display_entry)
                    right_column.append("*");
                right_column.append(" ");
            }
        }

        /* still need to show the slot for the last item */
        right_column.append(render_as_slot_name("{:" + old_slot + "} "));

        output_right_column(right_column);
    }
}

namespace
{
    class Displayer :
        public ConstVisitor<MetadataKeyVisitorTypes>
    {
        private:
            const ConsoleQueryTask * const task;
            const tr1::shared_ptr<const PackageID> id;
            const MetadataKeyType type;

        public:
            Displayer(const ConsoleQueryTask * const t, const tr1::shared_ptr<const PackageID> & i, const MetadataKeyType k) :
                task(t),
                id(i),
                type(k)
            {
            }

            void visit(const MetadataCollectionKey<IUseFlagCollection> & k)
            {
                if (k.type() == type)
                    task->display_metadata_iuse(k.human_name(), k.raw_name(), join(k.value()->begin(), k.value()->end(), " "),
                            id, k.value());
            }

            void visit(const MetadataCollectionKey<InheritedCollection> & k)
            {
                if (k.type() == type)
                    task->display_metadata_key(k.human_name(), k.raw_name(), join(k.value()->begin(), k.value()->end(), " "));
            }

            void visit(const MetadataCollectionKey<UseFlagNameCollection> & k)
            {
                if (k.type() == type)
                    task->display_metadata_key(k.human_name(), k.raw_name(), join(k.value()->begin(), k.value()->end(), " "));
            }

            void visit(const MetadataCollectionKey<KeywordNameCollection> & k)
            {
                if (k.type() == type)
                    task->display_metadata_key(k.human_name(), k.raw_name(), join(k.value()->begin(), k.value()->end(), " "));
            }

            void visit(const MetadataSpecTreeKey<DependencySpecTree> & k)
            {
                if (k.type() == type)
                    task->display_metadata_dependency(k.human_name(), k.raw_name(), k.value(), false);
            }

            void visit(const MetadataSpecTreeKey<URISpecTree> & k)
            {
                if (k.type() == type)
                    task->display_metadata_uri(k.human_name(), k.raw_name(), k.value(), true);
            }

            void visit(const MetadataSpecTreeKey<LicenseSpecTree> & k)
            {
                if (k.type() == type)
                    task->display_metadata_license(k.human_name(), k.raw_name(), k.value(), id);
            }

            void visit(const MetadataSpecTreeKey<ProvideSpecTree> & k)
            {
                if (k.type() == type)
                    task->display_metadata_provides(k.human_name(), k.raw_name(), k.value(), true);
            }

            void visit(const MetadataSpecTreeKey<RestrictSpecTree> & k)
            {
                if (k.type() == type)
                    task->display_metadata_restrict(k.human_name(), k.raw_name(), k.value(), true);
            }

            void visit(const MetadataPackageIDKey & k)
            {
                if (k.type() == type)
                    task->display_metadata_pde(k.human_name(), k.raw_name(), *k.value());
            }

            void visit(const MetadataStringKey & k)
            {
                if (k.type() == type)
                    task->display_metadata_key(k.human_name(), k.raw_name(), k.value());
            }
    };
}

void
ConsoleQueryTask::display_metadata(const PackageDepSpec &, const tr1::shared_ptr<const PackageID> & id) const
{
    if (! id->eapi()->supported)
    {
        display_metadata_key("EAPI", "EAPI", id->eapi()->name);
        return;
    }

    Displayer ds(this, id, mkt_significant);
    std::for_each(id->begin(), id->end(), accept_visitor(ds));

    Displayer dn(this, id, mkt_normal);
    std::for_each(id->begin(), id->end(), accept_visitor(dn));

    if (want_deps() || want_raw())
    {
        Displayer dd(this, id, mkt_dependencies);
        std::for_each(id->begin(), id->end(), accept_visitor(dd));
    }

    if (want_raw())
    {
        Displayer dr(this, id, mkt_internal);
        std::for_each(id->begin(), id->end(), accept_visitor(dr));
    }
}

namespace
{
    std::string normalise(const std::string & s)
    {
        std::list<std::string> w;
        WhitespaceTokeniser::get_instance()->tokenise(s, std::back_inserter(w));
        return join(w.begin(), w.end(), " ");
    }
}

void
ConsoleQueryTask::display_metadata_key(const std::string & k, const std::string & kk, const std::string & v) const
{
    if (v.empty())
        return;

    output_left_column((want_raw() ? kk : k) + ":");
    output_right_column(normalise(v));
}

void
ConsoleQueryTask::display_metadata_license(const std::string & k, const std::string & kk, tr1::shared_ptr<const LicenseSpecTree::ConstItem> l,
        const tr1::shared_ptr<const PackageID> & display_entry) const
{
    output_left_column((want_raw() ? kk : k) + ":");

    if (want_raw())
    {
        DepSpecPrettyPrinter p(0, false);
        l->accept(p);
        output_right_column(stringify(p));
    }
    else
    {
        LicenceDisplayer d(output_stream(), _imp->env, display_entry);
        l->accept(d);
        output_right_column("");
    }
}

namespace
{
    struct IsEmpty :
        ConstVisitor<GenericSpecTree>,
        ConstVisitor<GenericSpecTree>::VisitConstSequence<IsEmpty, AllDepSpec>,
        ConstVisitor<GenericSpecTree>::VisitConstSequence<IsEmpty, AnyDepSpec>,
        ConstVisitor<GenericSpecTree>::VisitConstSequence<IsEmpty, UseDepSpec>
    {
        using ConstVisitor<GenericSpecTree>::VisitConstSequence<IsEmpty, AnyDepSpec>::visit_sequence;
        using ConstVisitor<GenericSpecTree>::VisitConstSequence<IsEmpty, AllDepSpec>::visit_sequence;
        using ConstVisitor<GenericSpecTree>::VisitConstSequence<IsEmpty, UseDepSpec>::visit_sequence;

        bool empty;

        IsEmpty() :
            empty(true)
        {
        }

        void visit_leaf(const PackageDepSpec &)
        {
            empty = false;
        }

        void visit_leaf(const BlockDepSpec &)
        {
            empty = false;
        }

        void visit_leaf(const URIDepSpec &)
        {
            empty = false;
        }

        void visit_leaf(const PlainTextDepSpec &)
        {
            empty = false;
        }
    };

    template <typename T_>
    bool is_spec_empty(tr1::shared_ptr<const T_> d)
    {
        IsEmpty e;
        d->accept(e);
        return e.empty;
    }
}

namespace
{
    template <typename T_>
    void display_dep(const ConsoleQueryTask * const q, const std::string & k,
            const std::string & kk, tr1::shared_ptr<const T_> d, const bool one_line)
    {
        if (is_spec_empty(d))
            return;

        q->output_left_column((q->want_raw() ? kk : k) + ":");

        if (one_line)
        {
            DepSpecPrettyPrinter p(0, false);
            d->accept(p);
            q->output_stream() << p << std::endl;
        }
        else
        {
            q->output_right_column("");
            DepSpecPrettyPrinter p(q->left_column_width() + 5);
            d->accept(p);
            q->output_stream() << p;
        }
    }
}

void
ConsoleQueryTask::display_metadata_dependency(const std::string & k, const std::string & kk,
        tr1::shared_ptr<const DependencySpecTree::ConstItem> d, const bool one_line) const
{
    display_dep(this, k, kk, d, one_line);
}

void
ConsoleQueryTask::display_metadata_uri(const std::string & k, const std::string & kk,
        tr1::shared_ptr<const URISpecTree::ConstItem> d, const bool one_line) const
{
    display_dep(this, k, kk, d, one_line);
}

void
ConsoleQueryTask::display_metadata_provides(const std::string & k, const std::string & kk,
        tr1::shared_ptr<const ProvideSpecTree::ConstItem> d, const bool one_line) const
{
    display_dep(this, k, kk, d, one_line);
}

void
ConsoleQueryTask::display_metadata_restrict(const std::string & k, const std::string & kk,
        tr1::shared_ptr<const RestrictSpecTree::ConstItem> d, const bool one_line) const
{
    display_dep(this, k, kk, d, one_line);
}

void
ConsoleQueryTask::display_metadata_pde(const std::string & k, const std::string & kk,
        const PackageID & v) const
{
    output_left_column((want_raw() ? kk : k) + ":");
    output_right_column(stringify(v));
}

void
ConsoleQueryTask::display_metadata_time(const std::string & k, const std::string & kk,
        time_t t) const
{
    if (0 == t)
        return;

    if (want_raw())
    {
        output_left_column(kk + ":");
        output_right_column(stringify(t));
    }
    else
    {
        char buf[255];
        if (strftime(buf, 254, "%c", gmtime(&t)))
        {
            output_left_column(k + ":");
            output_right_column(stringify(buf));
        }
    }
}

void
ConsoleQueryTask::display_metadata_iuse(const std::string & k, const std::string & kk,
        const std::string & v, const tr1::shared_ptr<const PackageID> & id,
        const tr1::shared_ptr<const IUseFlagCollection> & e) const
{
    if (v.empty())
        return;

    output_left_column((want_raw() ? kk : k) + ":");

    if (want_raw())
        output_right_column(normalise(v));
    else
    {
        UseFlagPrettyPrinter printer(_imp->env);
        printer.print_package_flags(id, e);
        output_right_column("");
    }
}

const MaskReasons
ConsoleQueryTask::mask_reasons_to_explain() const
{
    return _imp->mask_reasons_to_explain;
}

