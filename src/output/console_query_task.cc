/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010 Ciaran McCreesh
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
#include <paludis/util/tokeniser.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/set.hh>
#include <paludis/util/map.hh>
#include <paludis/util/map-impl.hh>
#include <paludis/util/join.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/sequence-impl.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/pretty_print.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/timestamp.hh>
#include <paludis/package_dep_spec_properties.hh>
#include <paludis/mask.hh>
#include <paludis/metadata_key.hh>
#include <paludis/package_database.hh>
#include <paludis/environment.hh>
#include <paludis/package_id.hh>
#include <paludis/selection.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/choice.hh>
#include <list>
#include <algorithm>

using namespace paludis;

namespace paludis
{
    template<>
    struct Implementation<ConsoleQueryTask>
    {
        const Environment * const env;
        mutable std::shared_ptr<Map<char, std::string> > masks_to_explain;

        Implementation(const Environment * const e) :
            env(e),
            masks_to_explain(new Map<char, std::string>)
        {
        }
    };
}

namespace
{
    std::string slot_as_string(const std::shared_ptr<const PackageID> & id)
    {
        if (id->slot_key())
            return stringify(id->slot_key()->value());
        else
            return "(none)";
    }
}

ConsoleQueryTask::ConsoleQueryTask(const Environment * const e) :
    PrivateImplementationPattern<ConsoleQueryTask>(e)
{
}

ConsoleQueryTask::~ConsoleQueryTask()
{
}

void
ConsoleQueryTask::show(const PackageDepSpec & a, const std::shared_ptr<const PackageID> & display_entry) const
{
    /* we might be wildcarded. */
    if (! a.package_ptr())
    {
        std::shared_ptr<const PackageIDSequence> entries(
                (*_imp->env)[selection::BestVersionOnly(generator::Matches(a, MatchPackageOptions()))]);
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
ConsoleQueryTask::show_one(const PackageDepSpec & a, const std::shared_ptr<const PackageID> & display_entry) const
{
    std::shared_ptr<const PackageID> our_display_entry(display_entry);
    /* prefer the best installed version, then the best visible version, then
     * the best version */
    std::shared_ptr<const PackageIDSequence>
        entries((*_imp->env)[selection::AllVersionsSorted(generator::Matches(a, MatchPackageOptions()))]),
        preferred_entries((*_imp->env)[selection::AllVersionsSorted(
                    generator::Matches(a, MatchPackageOptions()) | filter::InstalledAtRoot(_imp->env->root()))]);
    if (entries->empty())
        throw NoSuchPackageError(stringify(a));
    if (preferred_entries->empty())
        preferred_entries = entries;

    if (! our_display_entry)
    {
        our_display_entry = *preferred_entries->last();
        for (PackageIDSequence::ConstIterator i(preferred_entries->begin()),
                i_end(preferred_entries->end()) ; i != i_end ; ++i)
            if (! (*i)->masked())
                our_display_entry = *i;
    }

    if (want_compact())
    {
        display_compact(a, our_display_entry);
    }
    else
    {
        display_header(a, our_display_entry);
        display_versions_by_repository(a, entries, our_display_entry);
        display_metadata(a, our_display_entry);
        display_masks(a, our_display_entry);
        output_endl();
    }
}

void
ConsoleQueryTask::display_header(const PackageDepSpec & a, const std::shared_ptr<const PackageID> & e) const
{
    if (package_dep_spec_has_properties(a, make_named_values<PackageDepSpecProperties>(
                    n::has_additional_requirements() = false,
                    n::has_category_name_part() = false,
                    n::has_from_repository() = false,
                    n::has_in_repository() = false,
                    n::has_installable_to_path() = false,
                    n::has_installable_to_repository() = false,
                    n::has_installed_at_path() = false,
                    n::has_package() = true,
                    n::has_package_name_part() = false,
                    n::has_slot_requirement() = false,
                    n::has_tag() = indeterminate,
                    n::has_version_requirements() = false
                    )))
        output_starred_item(render_as_package_name(stringify(e->name())));
    else
        output_starred_item(render_as_package_name(stringify(a)));
}

void
ConsoleQueryTask::display_compact(const PackageDepSpec & a, const std::shared_ptr<const PackageID> & e) const
{
    if (package_dep_spec_has_properties(a, make_named_values<PackageDepSpecProperties>(
                    n::has_additional_requirements() = false,
                    n::has_category_name_part() = false,
                    n::has_from_repository() = false,
                    n::has_in_repository() = false,
                    n::has_installable_to_path() = false,
                    n::has_installable_to_repository() = false,
                    n::has_installed_at_path() = false,
                    n::has_package() = true,
                    n::has_package_name_part() = false,
                    n::has_slot_requirement() = false,
                    n::has_tag() = indeterminate,
                    n::has_version_requirements() = false
                    )))
    {
        std::string pad(std::max<long>(1, 30 - stringify(e->name()).length()), ' ');
        output_starred_item_no_endl(render_as_package_name(stringify(e->name())) + pad);
    }
    else
    {
        std::string pad(std::max<long>(1, 30 - stringify(a).length()), ' ');
        output_starred_item_no_endl(render_as_package_name(stringify(a)) + pad);
    }

    if (e->short_description_key())
        output_no_endl(e->short_description_key()->value());
    output_endl();
}

void
ConsoleQueryTask::display_versions_by_repository(const PackageDepSpec &,
        const std::shared_ptr<const PackageIDSequence> & entries,
        const std::shared_ptr<const PackageID> & display_entry) const
{
    /* find all repository names. */
    std::list<RepositoryName> repo_names;
    PackageIDSequence::ConstIterator e(entries->begin()), e_end(entries->end());
    for ( ; e != e_end ; ++e)
        if (repo_names.end() == std::find(repo_names.begin(), repo_names.end(), (*e)->repository()->name()))
            repo_names.push_back((*e)->repository()->name());

    /* display versions, by repository. */
    std::list<RepositoryName>::const_iterator r(repo_names.begin()), r_end(repo_names.end());
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
                std::string slot_name(slot_as_string(*e));
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

                {
                    std::string reasons;
                    for (PackageID::OverriddenMasksConstIterator m((*e)->begin_overridden_masks()), m_end((*e)->end_overridden_masks()) ;
                            m != m_end ; ++m)
                    {
                        reasons.append(stringify((*m)->mask()->key()));
                        _imp->masks_to_explain->insert((*m)->mask()->key(), (*m)->mask()->description());
                    }

                    if (! reasons.empty())
                        right_column.append(render_as_visible("(" + reasons + ")"));
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
    class ComplexLicenseFinder
    {
        private:
            bool _is_complex;

        public:
            ComplexLicenseFinder() :
                _is_complex(false)
            {
            }

            void visit(const LicenseSpecTree::BasicNode &)
            {
            }

            void visit(const LicenseSpecTree::NodeType<AnyDepSpec>::Type &)
            {
                _is_complex = true;
            }

            void visit(const LicenseSpecTree::NodeType<ConditionalDepSpec>::Type &)
            {
                _is_complex = true;
            }

            operator bool () const
            {
                return _is_complex;
            }
    };

    class Displayer
    {
        private:
            const ConsoleQueryTask * const task;
            const Environment * const env;
            const std::shared_ptr<const PackageID> id;
            const MetadataKeyType type;
            const unsigned in;

        public:
            Displayer(const ConsoleQueryTask * const t, const Environment * const e,
                    const std::shared_ptr<const PackageID> & i, const MetadataKeyType k,
                    const unsigned ind = 0) :
                task(t),
                env(e),
                id(i),
                type(k),
                in(ind)
            {
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

            void visit(const MetadataCollectionKey<Sequence<std::string> > & k)
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
                        k.value()->root()->accept(is_complex);
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

            void visit(const MetadataSpecTreeKey<PlainTextSpecTree> & k)
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

            void visit(const MetadataValueKey<std::shared_ptr<const PackageID> > & k)
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

            void visit(const MetadataValueKey<SlotName> & k)
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
                if (0 == k.value().seconds())
                    return;

                std::string pretty_time(pretty_print_time(k.value().seconds()));

                if (k.type() == type)
                {
                    if (task->want_raw())
                    {
                        task->output_left_column(k.raw_name() + ":", in);
                        task->output_right_column(pretty_time);
                    }
                    else
                    {
                        task->output_left_column(k.human_name() + ":", in);
                        task->output_right_column(pretty_time);
                    }
                }
            }

            void visit(const MetadataValueKey<std::shared_ptr<const RepositoryMaskInfo> >  & k)
            {
                if (k.type() == type && k.value())
                {
                    if (task->want_raw())
                    {
                        task->output_left_column(k.raw_name() + ":", in);
                        task->output_right_column(stringify((*k.value()).mask_file()) + ": " +
                                join((*k.value()).comment()->begin(), (*k.value()).comment()->end(), " "));
                    }
                    else
                    {
                        task->output_left_column(k.human_name() + ":", in);
                        task->output_right_column(stringify((*k.value()).mask_file()) + ":");
                        for (Sequence<std::string>::ConstIterator it((*k.value()).comment()->begin()),
                                it_end((*k.value()).comment()->end()); it_end != it; ++it)
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

            void visit(const MetadataValueKey<std::shared_ptr<const Choices> > & k)
            {
                ColourFormatter formatter;
                if (k.type() == type)
                {
                    if (task->want_raw())
                    {
                        task->output_left_column(k.raw_name() + ":", in);
                        task->output_right_column("");
                        for (Choices::ConstIterator c(k.value()->begin()), c_end(k.value()->end()) ;
                                c != c_end ; ++c)
                        {
                            task->output_left_column((*c)->raw_name() + ":", in + 4);

                            std::string v;
                            for (Choice::ConstIterator i((*c)->begin()), i_end((*c)->end()) ;
                                    i != i_end ; ++i)
                            {
                                if (! v.empty())
                                    v.append(" ");

                                std::string t;
                                if ((*i)->enabled())
                                {
                                    if ((*i)->locked())
                                        t = formatter.format(**i, format::Forced());
                                    else
                                        t = formatter.format(**i, format::Enabled());
                                }
                                else
                                {
                                    if ((*i)->locked())
                                        t = formatter.format(**i, format::Masked());
                                    else
                                        t = formatter.format(**i, format::Disabled());
                                }

                                v.append(t);
                            }
                            task->output_right_column(v);
                        }
                    }
                    else
                    {
                        std::string s;
                        bool shown_prefix(false);

                        for (Choices::ConstIterator c(k.value()->begin()), c_end(k.value()->end()) ;
                                c != c_end ; ++c)
                        {
                            bool done_leader(false);
                            for (Choice::ConstIterator i((*c)->begin()), i_end((*c)->end()) ;
                                    i != i_end ; ++i)
                            {
                                if ((*c)->hidden())
                                    continue;
                                if (! (*i)->explicitly_listed())
                                    continue;

                                if (! done_leader)
                                {
                                    if (shown_prefix || ! (*c)->show_with_no_prefix())
                                    {
                                        if (! s.empty())
                                            s.append(" ");

                                        s.append((*c)->human_name() + ":");
                                        done_leader = true;
                                        shown_prefix = true;
                                    }
                                }

                                if (! s.empty())
                                    s.append(" ");

                                std::string t;
                                if ((*i)->enabled())
                                {
                                    if ((*i)->locked())
                                        t = formatter.format(**i, format::Forced());
                                    else
                                        t = formatter.format(**i, format::Enabled());
                                }
                                else
                                {
                                    if ((*i)->locked())
                                        t = formatter.format(**i, format::Masked());
                                    else
                                        t = formatter.format(**i, format::Disabled());
                                }

                                s.append(t);
                            }
                        }

                        task->output_left_column(k.human_name() + ":", in);
                        task->output_right_column(s);
                    }
                }
            }

            void visit(const MetadataValueKey<std::shared_ptr<const Contents> > &)
            {
            }
    };
}

void
ConsoleQueryTask::display_metadata(const PackageDepSpec &, const std::shared_ptr<const PackageID> & id) const
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
ConsoleQueryTask::display_masks(const PackageDepSpec &, const std::shared_ptr<const PackageID> & id) const
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
    struct IsEmpty
    {
        bool empty;

        IsEmpty() :
            empty(true)
        {
        }

        void visit(const GenericSpecTree::BasicInnerNode & node)
        {
            std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
        }

        void visit(const GenericSpecTree::NodeType<PackageDepSpec>::Type &)
        {
            empty = false;
        }

        void visit(const GenericSpecTree::NodeType<BlockDepSpec>::Type &)
        {
            empty = false;
        }

        void visit(const GenericSpecTree::NodeType<SimpleURIDepSpec>::Type &)
        {
            empty = false;
        }

        void visit(const GenericSpecTree::NodeType<FetchableURIDepSpec>::Type &)
        {
            empty = false;
        }

        void visit(const GenericSpecTree::NodeType<PlainTextDepSpec>::Type &)
        {
            empty = false;
        }

        void visit(const GenericSpecTree::NodeType<LicenseDepSpec>::Type &)
        {
            empty = false;
        }

        void visit(const GenericSpecTree::NodeType<NamedSetDepSpec>::Type &)
        {
            empty = false;
        }

        void visit(const GenericSpecTree::NodeType<URILabelsDepSpec>::Type &)
        {
        }

        void visit(const GenericSpecTree::NodeType<PlainTextLabelDepSpec>::Type &)
        {
        }

        void visit(const GenericSpecTree::NodeType<DependenciesLabelsDepSpec>::Type &)
        {
        }
    };

    template <typename T_>
    bool is_spec_empty(std::shared_ptr<const T_> d)
    {
        IsEmpty e;
        d->accept(e);
        return e.empty;
    }
}

const std::shared_ptr<const Map<char, std::string> >
ConsoleQueryTask::masks_to_explain() const
{
    return _imp->masks_to_explain;
}

template class Map<char, std::string>;
template class WrappedForwardIterator<Map<char, std::string>::ConstIteratorTag, const std::pair<const char, std::string> >;

