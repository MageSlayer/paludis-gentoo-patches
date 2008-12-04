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

#include <paludis/repositories/e/qa/visibility.hh>
#include <paludis/repositories/e/dep_spec_pretty_printer.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/log.hh>
#include <paludis/util/set.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/create_iterator-impl.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/qa.hh>
#include <paludis/dep_spec.hh>
#include <paludis/environment.hh>
#include <paludis/package_database.hh>
#include <paludis/package_id.hh>
#include <paludis/version_requirements.hh>
#include <paludis/metadata_key.hh>
#include <paludis/stringify_formatter.hh>
#include <paludis/elike_conditional_dep_spec.hh>
#include <paludis/selection.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/choice.hh>
#include <set>
#include <algorithm>

using namespace paludis;
using namespace paludis::erepository;

namespace
{
    struct Checker :
        ConstVisitor<DependencySpecTree>
    {
        const FSEntry entry;
        QAReporter * const reporter;
        const Environment * const env;
        const std::tr1::shared_ptr<const PackageID> & id;
        const std::tr1::shared_ptr<const ERepository> repo;
        const std::set<KeywordName> & accepted_keywords;
        const ERepository::ProfilesConstIterator profile;
        const std::string name;
        const bool unstable;
        const std::tr1::shared_ptr<const MetadataKey> & key;

        bool success;
        bool viable;

        Checker(
                const FSEntry & e,
                QAReporter * const r,
                const Environment * const v,
                const std::tr1::shared_ptr<const PackageID> & i,
                const std::tr1::shared_ptr<const ERepository> o,
                const std::set<KeywordName> & a,
                const ERepository::ProfilesConstIterator & p,
                const std::string & n,
                const bool u,
                const std::tr1::shared_ptr<const MetadataKey> & k) :
            entry(e),
            reporter(r),
            env(v),
            id(i),
            repo(o),
            accepted_keywords(a),
            profile(p),
            name(n),
            unstable(u),
            key(k),
            success(true),
            viable(false)
        {
        }

        void visit_leaf(const BlockDepSpec &)
        {
            viable = true;
        }

        void visit_leaf(const DependencyLabelsDepSpec &)
        {
        }

        void visit_leaf(const NamedSetDepSpec &)
        {
        }

        void visit_leaf(const PackageDepSpec & orig_p)
        {
            using namespace std::tr1::placeholders;

            success = false;
            viable = true;

            const PackageDepSpec * p(&orig_p);
            std::tr1::shared_ptr<PackageDepSpec> local_p;

            /* rewrite virtuals to avoid problems later on */
            if (p->package_ptr())
            {
                ERepositoryProfile::VirtualsConstIterator v((*profile).profile()->find_virtual(*p->package_ptr()));
                if ((*profile).profile()->end_virtuals() != v)
                {
                    PartiallyMadePackageDepSpec pp;

                    if (v->second->version_requirements_ptr())
                        std::for_each(v->second->version_requirements_ptr()->begin(), v->second->version_requirements_ptr()->end(),
                                std::tr1::bind(&PartiallyMadePackageDepSpec::version_requirement, &pp, _1));
                    if (orig_p.version_requirements_ptr())
                        std::for_each(orig_p.version_requirements_ptr()->begin(), orig_p.version_requirements_ptr()->end(),
                                std::tr1::bind(&PartiallyMadePackageDepSpec::version_requirement, &pp, _1));

                    pp.package(*v->second->package_ptr());
                    if (orig_p.slot_requirement_ptr())
                        pp.slot_requirement(orig_p.slot_requirement_ptr());
                    if (orig_p.in_repository_ptr())
                        pp.in_repository(*orig_p.in_repository_ptr());

                    local_p.reset(new PackageDepSpec(pp));
                    local_p->set_tag(orig_p.tag());
                    p = local_p.get();
                }
            }

            const std::tr1::shared_ptr<const PackageIDSequence> matches((*env)[selection::AllVersionsSorted(
                        generator::Matches(*p, MatchPackageOptions() + mpo_ignore_additional_requirements)
                        | filter::SupportsAction<InstallAction>())]);
            if (matches->empty())
            {
                if (reporter)
                    reporter->message(QAMessage(entry, qaml_normal, name, "No packages matching '"
                                + stringify(orig_p) + "' in dependencies key '" + stringify(key->raw_name()) + "' for profile '"
                                + stringify((*profile).path()) + "' (" + stringify((*profile).arch()) + "."
                                + stringify((*profile).status())
                                + (unstable ? ".unstable" : ".stable") + ")")
                            .with_associated_id(id)
                            .with_associated_key(id, key));
            }
            else
            {
                for (PackageIDSequence::ConstIterator i(matches->begin()), i_end(matches->end()) ;
                        i != i_end ; ++i)
                {
                    /* can't use the usual masked rules here, so this gets a bit complicated... */
                    if ((*i)->repository() == repo)
                    {
                        if (repo->repository_masked(**i) || (*profile).profile()->profile_masked(**i) || ! (*i)->keywords_key())
                            continue;
                    }
                    else
                    {
                        bool found_repo(false), repo_masked(false);
                        if (repo->params().master_repositories())
                        {
                            for (ERepositorySequence::ConstIterator e(repo->params().master_repositories()->begin()),
                                    e_end(repo->params().master_repositories()->end()) ; e != e_end ; ++e)
                            {
                                if ((*i)->repository()->name() == (*e)->name())
                                {
                                    if ((*e)->repository_masked(**i))
                                        repo_masked = true;
                                    found_repo = true;
                                    break;
                                }
                            }
                        }

                        if (repo_masked)
                            continue;

                        if (! found_repo)
                        {
                            Log::get_instance()->message("e.qa.visibility_check.no_masks", ll_warning, lc_context)
                                << "Probably a bug: don't know how to get masks for '"
                                << **i << "' from '" << orig_p << "' -> '" << *p << "'";
                            continue;
                        }
                    }

                    std::set<KeywordName> overlap;
                    if ((*i)->keywords_key())
                        std::set_intersection(accepted_keywords.begin(), accepted_keywords.end(),
                                (*i)->keywords_key()->value()->begin(), (*i)->keywords_key()->value()->end(),
                                std::inserter(overlap, overlap.begin()));

                    if (overlap.empty())
                        continue;

                    success = true;
                    break;
                }

                if (! success)
                    if (reporter)
                        reporter->message(QAMessage(entry, qaml_normal, name, "No visible packages matching '"
                                    + stringify(orig_p) + "' in dependencies key '" + stringify(key->raw_name()) + "' for profile '"
                                    + stringify((*profile).path()) + "' (" + stringify((*profile).arch()) + "." + stringify((*profile).status())
                                    + (unstable ? ".unstable" : ".stable") + ")")
                                .with_associated_id(id)
                                .with_associated_key(id, key));
            }
        }

        void visit_sequence(const ConditionalDepSpec & u,
                DependencySpecTree::ConstSequenceIterator cur,
                DependencySpecTree::ConstSequenceIterator end)
        {
            ChoiceNameWithPrefix prefixed(elike_conditional_dep_spec_flag(u));
            UnprefixedChoiceName value("x");
            std::tr1::shared_ptr<const Choice> choice;
            if (id->choices_key())
                for (Choices::ConstIterator c(id->choices_key()->value()->begin()),
                        c_end(id->choices_key()->value()->end()) ;
                        c != c_end ; ++c)
                {
                    if (0 != prefixed.data().compare(0, (*c)->prefix().data().length(), (*c)->prefix().data(),
                                0, (*c)->prefix().data().length()))
                        continue;

                    for (Choice::ConstIterator d((*c)->begin()), d_end((*c)->end()) ;
                            d != d_end ; ++d)
                        if ((*d)->name_with_prefix() == prefixed)
                        {
                            choice = *c;
                            value = (*d)->unprefixed_name();
                            break;
                        }

                    if (choice)
                        break;
                }

            if (! choice)
            {
                viable = false;
                if (reporter)
                    reporter->message(QAMessage(entry, qaml_normal, name, "No flag matching '"
                                + stringify(prefixed) + "' in dependencies key '" + stringify(key->raw_name()) + "' for profile '"
                                + stringify((*profile).path()) + "' (" + stringify((*profile).arch()) + "." + stringify((*profile).status())
                                + (unstable ? ".unstable" : ".stable") + ")")
                            .with_associated_id(id)
                            .with_associated_key(id, key));
            }
            else
                viable =
                    ((! elike_conditional_dep_spec_is_inverse(u)) && (! (*profile).profile()->use_masked(
                            id, choice, value, prefixed))) ||
                    ((elike_conditional_dep_spec_is_inverse(u)) && (! (*profile).profile()->use_forced(
                            id, choice, value, prefixed)));

            if (viable)
                std::for_each(cur, end, accept_visitor(*this));
        }

        void visit_sequence(const AnyDepSpec &,
                DependencySpecTree::ConstSequenceIterator begin,
                DependencySpecTree::ConstSequenceIterator end)
        {
            success = true;
            viable = true;
            for (DependencySpecTree::ConstSequenceIterator cur(begin) ; cur != end ; ++cur)
            {
                Checker c(entry, 0, env, id, repo, accepted_keywords, profile, name, unstable, key);
                accept_visitor(c)(*cur);
                if (c.success)
                {
                    success = true;
                    break;
                }
                else if (c.viable)
                    success = false;
            }

            if (! success)
            {
                if (reporter)
                {
                    StringifyFormatter ff;
                    DepSpecPrettyPrinter printer(0, std::tr1::shared_ptr<const PackageID>(), ff, 0, false, false);
                    std::for_each(begin, end, accept_visitor(printer));
                    reporter->message(QAMessage(entry, qaml_normal, name, "No item in block '|| ( "
                                + stringify(printer) + " )' visible for profile '"
                                + stringify((*profile).path()) + "' (" + stringify((*profile).arch()) + "." + stringify((*profile).status())
                                + (unstable ? ".unstable" : ".stable") + ")")
                            .with_associated_id(id)
                            .with_associated_key(id, key));
                }
            }
        }

        void visit_sequence(const AllDepSpec &,
                DependencySpecTree::ConstSequenceIterator cur,
                DependencySpecTree::ConstSequenceIterator end)
        {
            viable = true;
            std::for_each(cur, end, accept_visitor(*this));
        }
    };
}

bool
paludis::erepository::visibility_check(
        const FSEntry & entry,
        QAReporter & reporter,
        const Environment * const env,
        const std::tr1::shared_ptr<const ERepository> & repo,
        const std::tr1::shared_ptr<const PackageID> & id,
        const std::string & name)
{
    Context context("When performing check '" + name + "' using visibility_check on ID '" + stringify(*id) + "':");

    Log::get_instance()->message("e.qa.visibility_check", ll_debug, lc_context) << "visibility_check '"
        << entry << "', '" << *id << "', '" << name << "'";

    if (repo->repository_masked(*id) || ! id->keywords_key())
        return true;

    for (ERepository::ProfilesConstIterator p(repo->begin_profiles()), p_end(repo->end_profiles()) ;
            p != p_end ; ++p)
    {
        if ((*p).profile()->profile_masked(*id))
            continue;

        std::set<KeywordName> accepted_keywords, overlap;
        tokenise_whitespace((*p).profile()->environment_variable(
                    repo->accept_keywords_variable()), create_inserter<KeywordName>(std::inserter(accepted_keywords, accepted_keywords.begin())));

        std::set_intersection(accepted_keywords.begin(), accepted_keywords.end(),
                id->keywords_key()->value()->begin(), id->keywords_key()->value()->end(),
                std::inserter(overlap, overlap.begin()));

        if (! overlap.empty())
        {
            if (id->build_dependencies_key())
            {
                Checker c(entry, &reporter, env, id, repo, accepted_keywords, p, name, false, id->build_dependencies_key());
                id->build_dependencies_key()->value()->accept(c);
            }

            if (id->run_dependencies_key())
            {
                Checker c(entry, &reporter, env, id, repo, accepted_keywords, p, name, false, id->run_dependencies_key());
                id->run_dependencies_key()->value()->accept(c);
            }

            if (id->post_dependencies_key())
            {
                Checker c(entry, &reporter, env, id, repo, accepted_keywords, p, name, false, id->post_dependencies_key());
                id->post_dependencies_key()->value()->accept(c);
            }

            if (id->suggested_dependencies_key())
            {
                Checker c(entry, &reporter, env, id, repo, accepted_keywords, p, name, false, id->suggested_dependencies_key());
                id->post_dependencies_key()->value()->accept(c);
            }
        }
        else
        {
            for (std::set<KeywordName>::iterator i(accepted_keywords.begin()), i_end(accepted_keywords.end()) ;
                    i != i_end ; ++i)
                if ('~' != stringify(*i).at(0))
                    accepted_keywords.insert(KeywordName("~" + stringify(*i)));

            std::set_intersection(accepted_keywords.begin(), accepted_keywords.end(),
                    id->keywords_key()->value()->begin(), id->keywords_key()->value()->end(),
                    std::inserter(overlap, overlap.begin()));

            if (! overlap.empty())
            {
                if (id->build_dependencies_key())
                {
                    Checker c(entry, &reporter, env, id, repo, accepted_keywords, p, name, true, id->build_dependencies_key());
                    id->build_dependencies_key()->value()->accept(c);
                }

                if (id->run_dependencies_key())
                {
                    Checker c(entry, &reporter, env, id, repo, accepted_keywords, p, name, true, id->run_dependencies_key());
                    id->run_dependencies_key()->value()->accept(c);
                }

                if (id->post_dependencies_key())
                {
                    Checker c(entry, &reporter, env, id, repo, accepted_keywords, p, name, true, id->post_dependencies_key());
                    id->post_dependencies_key()->value()->accept(c);
                }

                if (id->suggested_dependencies_key())
                {
                    Checker c(entry, &reporter, env, id, repo, accepted_keywords, p, name, true, id->suggested_dependencies_key());
                    id->post_dependencies_key()->value()->accept(c);
                }
            }
        }
    }

    return true;
}

