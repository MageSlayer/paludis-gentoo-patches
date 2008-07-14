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

#include "console_query_task.hh"
#include "mask_displayer.hh"
#include "colour_formatter.hh"
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/set.hh>
#include <paludis/util/map.hh>
#include <paludis/util/map-impl.hh>
#include <paludis/util/join.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/kc.hh>
#include <paludis/mask.hh>
#include <paludis/metadata_key.hh>
#include <paludis/package_database.hh>
#include <paludis/environment.hh>
#include <paludis/package_id.hh>
#include <paludis/selection.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <list>

using namespace paludis;

template class Map<char, std::string>;

namespace paludis
{
    template<>
    struct Implementation<ConsoleQueryTask>
    {
        const Environment * const env;
        mutable std::tr1::shared_ptr<Map<char, std::string> > masks_to_explain;

        Implementation(const Environment * const e) :
            env(e),
            masks_to_explain(new Map<char, std::string>)
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
ConsoleQueryTask::show(const PackageDepSpec & a, std::tr1::shared_ptr<const PackageID> display_entry) const
{
    /* we might be wildcarded. */
    if (! a.package_ptr())
    {
        std::tr1::shared_ptr<const PackageIDSequence> entries(
                (*_imp->env)[selection::BestVersionOnly(generator::Matches(a))]);
        if (entries->empty())
            throw NoSuchPackageError(stringify(a));

        for (PackageIDSequence::ConstIterator i(entries->begin()), i_end(entries->end()) ;
                i != i_end ; ++i)
        {
            PartiallyMadePackageDepSpec p(a);
            p.package((*i)->name());
            show_one(p, display_entry);
        }
    }
    else
        show_one(a, display_entry);
}

void
ConsoleQueryTask::show_one(const PackageDepSpec & a, std::tr1::shared_ptr<const PackageID> display_entry) const
{
    /* prefer the best installed version, then the best visible version, then
     * the best version */
    std::tr1::shared_ptr<const PackageIDSequence>
        entries((*_imp->env)[selection::AllVersionsSorted(generator::Matches(a))]),
        preferred_entries((*_imp->env)[selection::AllVersionsSorted(
                    generator::Matches(a) | filter::InstalledAtRoot(_imp->env->root()))]);
    if (entries->empty())
        throw NoSuchPackageError(stringify(a));
    if (preferred_entries->empty())
        preferred_entries = entries;

    if (! display_entry)
    {
        display_entry = *preferred_entries->last();
        for (PackageIDSequence::ConstIterator i(preferred_entries->begin()),
                i_end(preferred_entries->end()) ; i != i_end ; ++i)
            if (! (*i)->masked())
                display_entry = *i;
    }

    if (want_compact())
    {
        display_compact(a, display_entry);
    }
    else
    {
        display_header(a, display_entry);
        display_versions_by_repository(a, entries, display_entry);
        display_metadata(a, display_entry);
        display_masks(a, display_entry);
        output_endl();
    }
}

void
ConsoleQueryTask::display_header(const PackageDepSpec & a, const std::tr1::shared_ptr<const PackageID> & e) const
{
    if (a.version_requirements_ptr() || a.slot_requirement_ptr() || a.additional_requirements_ptr() ||
            a.in_repository_ptr() || a.from_repository_ptr())
        output_starred_item(render_as_package_name(stringify(a)));
    else
        output_starred_item(render_as_package_name(stringify(e->name())));
}

void
ConsoleQueryTask::display_compact(const PackageDepSpec & a, const std::tr1::shared_ptr<const PackageID> & e) const
{
    if (a.version_requirements_ptr() || a.slot_requirement_ptr() || a.additional_requirements_ptr() ||
            a.in_repository_ptr() || a.from_repository_ptr())
    {
        std::string pad(std::max<long>(1, 30 - stringify(a).length()), ' ');
        output_starred_item_no_endl(render_as_package_name(stringify(a)) + pad);
    }
    else
    {
        std::string pad(std::max<long>(1, 30 - stringify(e->name()).length()), ' ');
        output_starred_item_no_endl(render_as_package_name(stringify(e->name())) + pad);
    }

    if (e->short_description_key())
        output_no_endl(e->short_description_key()->value());
    output_endl();
}

void
ConsoleQueryTask::display_versions_by_repository(const PackageDepSpec &,
        std::tr1::shared_ptr<const PackageIDSequence> entries,
        const std::tr1::shared_ptr<const PackageID> & display_entry) const
{
    /* find all repository names. */
    RepositoryNameSequence repo_names;
    PackageIDSequence::ConstIterator e(entries->begin()), e_end(entries->end());
    for ( ; e != e_end ; ++e)
        if (repo_names.end() == std::find(repo_names.begin(), repo_names.end(), (*e)->repository()->name()))
            repo_names.push_back((*e)->repository()->name());

    /* display versions, by repository. */
    RepositoryNameSequence::ConstIterator r(repo_names.begin()), r_end(repo_names.end());
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

                if (! (*e)->masked())
                    right_column.append(render_as_visible((*e)->canonical_form(idcf_version)));
                else
                {
                    std::string reasons;
                    for (PackageID::MasksConstIterator m((*e)->begin_masks()), m_end((*e)->end_masks()) ;
                            m != m_end ; ++m)
                    {
                        reasons.append(stringify((*m)->key()));
                        _imp->masks_to_explain->insert((*m)->key(), (*m)->description());
                    }
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
    class ComplexLicenseFinder :
        public ConstVisitor<LicenseSpecTree>,
        public ConstVisitor<LicenseSpecTree>::VisitConstSequence<ComplexLicenseFinder, AllDepSpec>
    {
        private:
            bool _is_complex;

        public:
            ComplexLicenseFinder() :
                _is_complex(false)
            {
            }

            using ConstVisitor<LicenseSpecTree>::VisitConstSequence<ComplexLicenseFinder, AllDepSpec>::visit_sequence;

            void visit_leaf(const LicenseDepSpec &)
            {
            }

            void visit_sequence(const AnyDepSpec &,
                    LicenseSpecTree::ConstSequenceIterator,
                    LicenseSpecTree::ConstSequenceIterator)
            {
                _is_complex = true;
            }

            void visit_sequence(const ConditionalDepSpec &,
                    LicenseSpecTree::ConstSequenceIterator,
                    LicenseSpecTree::ConstSequenceIterator)
            {
                _is_complex = true;
            }

            operator bool () const
            {
                return _is_complex;
            }
    };

    class Displayer :
        public ConstVisitor<MetadataKeyVisitorTypes>
    {
        private:
            const ConsoleQueryTask * const task;
            const Environment * const env;
            const std::tr1::shared_ptr<const PackageID> id;
            const MetadataKeyType type;
            const unsigned in;

        public:
            Displayer(const ConsoleQueryTask * const t, const Environment * const e,
                    const std::tr1::shared_ptr<const PackageID> & i, const MetadataKeyType k,
                    const unsigned ind = 0) :
                task(t),
                env(e),
                id(i),
                type(k),
                in(ind)
            {
            }

            void visit(const MetadataCollectionKey<IUseFlagSet> & k)
            {
                if (k.type() == type)
                {
                    ColourFormatter formatter;
                    if (task->want_raw())
                    {
                        task->output_left_column(k.raw_name() + ":", in);
                        task->output_right_column(k.pretty_print_flat(formatter));
                    }
                    else
                    {
                        task->output_left_column(k.human_name() + ":", in);
                        task->output_right_column(k.pretty_print_flat(formatter));
                    }
                }
            }

            void visit(const MetadataCollectionKey<FSEntrySequence> & k)
            {
                if (k.type() == type)
                {
                    ColourFormatter formatter;
                    if (task->want_raw())
                    {
                        task->output_left_column(k.raw_name() + ":", in);
                        task->output_right_column(k.pretty_print_flat(formatter));
                    }
                    else
                    {
                        task->output_left_column(k.human_name() + ":", in);
                        task->output_right_column(k.pretty_print_flat(formatter));
                    }
                }
            }

            void visit(const MetadataCollectionKey<Set<std::string> > & k)
            {
                if (k.type() == type)
                {
                    ColourFormatter formatter;
                    if (task->want_raw())
                    {
                        task->output_left_column(k.raw_name() + ":", in);
                        task->output_right_column(k.pretty_print_flat(formatter));
                    }
                    else
                    {
                        task->output_left_column(k.human_name() + ":", in);
                        task->output_right_column(k.pretty_print_flat(formatter));
                    }
                }
            }

            void visit(const MetadataCollectionKey<UseFlagNameSet> & k)
            {
                if (k.type() == type)
                {
                    ColourFormatter formatter;
                    if (task->want_raw())
                    {
                        task->output_left_column(k.raw_name() + ":", in);
                        task->output_right_column(k.pretty_print_flat(formatter));
                    }
                    else
                    {
                        task->output_left_column(k.human_name() + ":", in);
                        task->output_right_column(k.pretty_print_flat(formatter));
                    }
                }
            }

            void visit(const MetadataCollectionKey<KeywordNameSet> & k)
            {
                if (k.type() == type)
                {
                    ColourFormatter formatter;
                    if (task->want_raw())
                    {
                        task->output_left_column(k.raw_name() + ":", in);
                        task->output_right_column(k.pretty_print_flat(formatter));
                    }
                    else
                    {
                        task->output_left_column(k.human_name() + ":", in);
                        task->output_right_column(k.pretty_print_flat(formatter));
                    }
                }
            }

            void visit(const MetadataSpecTreeKey<DependencySpecTree> & k)
            {
                if (k.type() == type)
                {
                    ColourFormatter formatter;
                    if (task->want_raw())
                    {
                        task->output_left_column(k.raw_name() + ":", in);
                        task->output_right_column(k.pretty_print_flat(formatter));
                    }
                    else
                    {
                        task->output_left_column(k.human_name() + ":", in);
                        task->output_right_column("");
                        task->output_stream() << k.pretty_print(formatter);
                    }
                }
            }

            void visit(const MetadataSpecTreeKey<FetchableURISpecTree> & k)
            {
                if (k.type() == type)
                {
                    ColourFormatter formatter;
                    if (task->want_raw())
                    {
                        task->output_left_column(k.raw_name() + ":", in);
                        task->output_right_column(k.pretty_print_flat(formatter));
                    }
                    else
                    {
                        task->output_left_column(k.human_name() + ":", in);
                        task->output_stream() << k.pretty_print_flat(formatter);
                        task->output_right_column("");
                    }
                }
            }

            void visit(const MetadataSpecTreeKey<SimpleURISpecTree> & k)
            {
                if (k.type() == type)
                {
                    ColourFormatter formatter;
                    if (task->want_raw())
                    {
                        task->output_left_column(k.raw_name() + ":", in);
                        task->output_right_column(k.pretty_print_flat(formatter));
                    }
                    else
                    {
                        task->output_left_column(k.human_name() + ":", in);
                        task->output_stream() << k.pretty_print_flat(formatter);
                        task->output_right_column("");
                    }
                }
            }

            void visit(const MetadataSpecTreeKey<LicenseSpecTree> & k)
            {
                if (k.type() == type)
                {
                    ColourFormatter formatter;
                    if (task->want_raw())
                    {
                        task->output_left_column(k.raw_name() + ":", in);
                        task->output_right_column(k.pretty_print_flat(formatter));
                    }
                    else
                    {
                        task->output_left_column(k.human_name() + ":", in);
                        ComplexLicenseFinder is_complex;
                        k.value()->accept(is_complex);
                        if (is_complex)
                        {
                            task->output_right_column("");
                            task->output_stream() << k.pretty_print(formatter);
                        }
                        else
                            task->output_right_column(k.pretty_print_flat(formatter));
                    }
                }
            }

            void visit(const MetadataSpecTreeKey<ProvideSpecTree> & k)
            {
                if (k.type() == type)
                {
                    ColourFormatter formatter;
                    if (task->want_raw())
                    {
                        task->output_left_column(k.raw_name() + ":", in);
                        task->output_right_column(k.pretty_print_flat(formatter));
                    }
                    else
                    {
                        task->output_left_column(k.human_name() + ":", in);
                        task->output_right_column("");
                        task->output_stream() << k.pretty_print(formatter);
                    }
                }
            }

            void visit(const MetadataSpecTreeKey<RestrictSpecTree> & k)
            {
                if (k.type() == type)
                {
                    ColourFormatter formatter;
                    if (task->want_raw())
                    {
                        task->output_left_column(k.raw_name() + ":", in);
                        task->output_right_column(k.pretty_print_flat(formatter));
                    }
                    else
                    {
                        task->output_left_column(k.human_name() + ":", in);
                        task->output_right_column("");
                        task->output_stream() << k.pretty_print(formatter);
                    }
                }
            }

            void visit(const MetadataCollectionKey<PackageIDSequence> & k)
            {
                if (k.type() == type)
                {
                    ColourFormatter formatter;
                    if (task->want_raw())
                    {
                        task->output_left_column(k.raw_name() + ":", in);
                        task->output_right_column(k.pretty_print_flat(formatter));
                    }
                    else
                    {
                        task->output_left_column(k.human_name() + ":", in);
                        task->output_right_column(k.pretty_print_flat(formatter));
                    }
                }
            }

            void visit(const MetadataValueKey<std::tr1::shared_ptr<const PackageID> > & k)
            {
                if (k.type() == type)
                {
                    if (task->want_raw())
                    {
                        task->output_left_column(k.raw_name() + ":", in);
                        task->output_right_column(stringify(*k.value()));
                    }
                    else
                    {
                        task->output_left_column(k.human_name() + ":", in);
                        task->output_right_column(stringify(*k.value()));
                    }
                }
            }

            void visit(const MetadataValueKey<std::string> & k)
            {
                if (k.type() == type)
                {
                    if (task->want_raw())
                    {
                        task->output_left_column(k.raw_name() + ":", in);
                        task->output_right_column(stringify(k.value()));
                    }
                    else
                    {
                        task->output_left_column(k.human_name() + ":", in);
                        task->output_right_column(stringify(k.value()));
                    }
                }
            }

            void visit(const MetadataValueKey<long> & k)
            {
                if (k.type() == type)
                {
                    if (task->want_raw())
                    {
                        task->output_left_column(k.raw_name() + ":", in);
                        task->output_right_column(stringify(k.value()));
                    }
                    else
                    {
                        task->output_left_column(k.human_name() + ":", in);
                        task->output_right_column(k.pretty_print());
                    }
                }
            }

            void visit(const MetadataValueKey<bool> & k)
            {
                if (k.type() == type)
                {
                    if (task->want_raw())
                    {
                        task->output_left_column(k.raw_name() + ":", in);
                        task->output_right_column(stringify(k.value()));
                    }
                    else
                    {
                        task->output_left_column(k.human_name() + ":", in);
                        task->output_right_column(k.pretty_print());
                    }
                }
            }

            void visit(const MetadataSectionKey & k)
            {
                if (k.type() == type)
                {
                    if (task->want_raw())
                    {
                        task->output_left_column(k.raw_name() + ":", in);
                        task->output_right_column("");
                    }
                    else
                    {
                        task->output_left_column(k.human_name() + ":", in);
                        task->output_right_column("");
                    }

                    Displayer v(task, env, id, type, in + 4);
                    std::for_each(indirect_iterator(k.begin_metadata()), indirect_iterator(k.end_metadata()),
                            accept_visitor(v));
                }
            }

            void visit(const MetadataTimeKey & k)
            {
                if (0 == k.value())
                    return;

                time_t t(k.value());
                char buf[255];
                if (! strftime(buf, 254, "%c", gmtime(&t)))
                    buf[0] = '\0';

                if (k.type() == type)
                {
                    if (task->want_raw())
                    {
                        task->output_left_column(k.raw_name() + ":", in);
                        task->output_right_column(stringify(buf));
                    }
                    else
                    {
                        task->output_left_column(k.human_name() + ":", in);
                        task->output_right_column(stringify(buf));
                    }
                }
            }

            void visit(const MetadataValueKey<std::tr1::shared_ptr<const RepositoryMaskInfo> >  & k)
            {
                if (k.type() == type && k.value())
                {
                    if (task->want_raw())
                    {
                        task->output_left_column(k.raw_name() + ":", in);
                        task->output_right_column(stringify((*k.value())[k::mask_file()]) + ": " +
                                join((*k.value())[k::comment()]->begin(), (*k.value())[k::comment()]->end(), " "));
                    }
                    else
                    {
                        task->output_left_column(k.human_name() + ":", in);
                        task->output_right_column(stringify((*k.value())[k::mask_file()]) + ":");
                        for (Sequence<std::string>::ConstIterator it((*k.value())[k::comment()]->begin()),
                                it_end((*k.value())[k::comment()]->end()); it_end != it; ++it)
                        {
                            task->output_left_column("", in);
                            task->output_right_column(*it);
                        }
                    }
                }
            }

            void visit(const MetadataValueKey<FSEntry> & k)
            {
                if (k.type() == type)
                {
                    if (task->want_raw())
                    {
                        task->output_left_column(k.raw_name() + ":", in);
                        task->output_right_column(stringify(k.value()));
                    }
                    else
                    {
                        task->output_left_column(k.human_name() + ":", in);
                        task->output_right_column(stringify(k.value()));
                    }
                }
            }

            void visit(const MetadataValueKey<std::tr1::shared_ptr<const Contents> > &)
            {
            }
    };
}

void
ConsoleQueryTask::display_metadata(const PackageDepSpec &, const std::tr1::shared_ptr<const PackageID> & id) const
{
    Displayer ds(this, _imp->env, id, mkt_significant);
    std::for_each(indirect_iterator(id->begin_metadata()), indirect_iterator(id->end_metadata()), accept_visitor(ds));

    Displayer dn(this, _imp->env, id, mkt_normal);
    std::for_each(indirect_iterator(id->begin_metadata()), indirect_iterator(id->end_metadata()), accept_visitor(dn));

    if (want_authors() || want_raw())
    {
        Displayer dd(this, _imp->env, id, mkt_author);
        std::for_each(indirect_iterator(id->begin_metadata()), indirect_iterator(id->end_metadata()), accept_visitor(dd));
    }

    if (want_deps() || want_raw())
    {
        Displayer dd(this, _imp->env, id, mkt_dependencies);
        std::for_each(indirect_iterator(id->begin_metadata()), indirect_iterator(id->end_metadata()), accept_visitor(dd));
    }

    if (want_raw())
    {
        Displayer dr(this, _imp->env, id, mkt_internal);
        std::for_each(indirect_iterator(id->begin_metadata()), indirect_iterator(id->end_metadata()), accept_visitor(dr));
    }
}

void
ConsoleQueryTask::display_masks(const PackageDepSpec &, const std::tr1::shared_ptr<const PackageID> & id) const
{
    for (PackageID::MasksConstIterator m(id->begin_masks()), m_end(id->end_masks()) ;
            m != m_end ; ++m)
    {
        MaskDisplayer d(_imp->env, id, false);
        (*m)->accept(d);
        output_left_column("Masked by " + strip_leading_string((*m)->description(), "by ") + ":");
        output_right_column(d.result());
    }
}

namespace
{
    std::string normalise(const std::string & s)
    {
        std::list<std::string> w;
        tokenise_whitespace(s, std::back_inserter(w));
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

namespace
{
    struct IsEmpty :
        ConstVisitor<GenericSpecTree>,
        ConstVisitor<GenericSpecTree>::VisitConstSequence<IsEmpty, AllDepSpec>,
        ConstVisitor<GenericSpecTree>::VisitConstSequence<IsEmpty, AnyDepSpec>,
        ConstVisitor<GenericSpecTree>::VisitConstSequence<IsEmpty, ConditionalDepSpec>
    {
        using ConstVisitor<GenericSpecTree>::VisitConstSequence<IsEmpty, AnyDepSpec>::visit_sequence;
        using ConstVisitor<GenericSpecTree>::VisitConstSequence<IsEmpty, AllDepSpec>::visit_sequence;
        using ConstVisitor<GenericSpecTree>::VisitConstSequence<IsEmpty, ConditionalDepSpec>::visit_sequence;

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

        void visit_leaf(const SimpleURIDepSpec &)
        {
            empty = false;
        }

        void visit_leaf(const FetchableURIDepSpec &)
        {
            empty = false;
        }

        void visit_leaf(const PlainTextDepSpec &)
        {
            empty = false;
        }

        void visit_leaf(const LicenseDepSpec &)
        {
            empty = false;
        }

        void visit_leaf(const NamedSetDepSpec &)
        {
            empty = false;
        }

        void visit_leaf(const URILabelsDepSpec &)
        {
        }

        void visit_leaf(const DependencyLabelsDepSpec &)
        {
        }
    };

    template <typename T_>
    bool is_spec_empty(std::tr1::shared_ptr<const T_> d)
    {
        IsEmpty e;
        d->accept(e);
        return e.empty;
    }
}

const std::tr1::shared_ptr<const Map<char, std::string> >
ConsoleQueryTask::masks_to_explain() const
{
    return _imp->masks_to_explain;
}

