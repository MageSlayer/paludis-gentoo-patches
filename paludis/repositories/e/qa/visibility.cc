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

#include <paludis/repositories/e/qa/visibility.hh>
#include <paludis/repositories/e/dep_spec_pretty_printer.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/log.hh>
#include <paludis/util/set.hh>
#include <paludis/util/iterator.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/query.hh>
#include <paludis/qa.hh>
#include <paludis/dep_spec.hh>
#include <paludis/environment.hh>
#include <paludis/package_database.hh>
#include <paludis/package_id.hh>
#include <paludis/version_requirements.hh>
#include <paludis/metadata_key.hh>
#include <paludis/stringify_formatter.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>
#include <set>
#include <algorithm>

using namespace paludis;

namespace
{
    struct Checker :
        ConstVisitor<DependencySpecTree>
    {
        const FSEntry entry;
        QAReporter * const reporter;
        const Environment * const env;
        const tr1::shared_ptr<const PackageID> & id;
        const tr1::shared_ptr<const ERepository> repo;
        const std::set<KeywordName> & accepted_keywords;
        const ERepository::ProfilesConstIterator profile;
        const std::string name;
        const bool unstable;
        const tr1::shared_ptr<const MetadataKey> & key;

        bool success;
        bool viable;

        Checker(
                const FSEntry & e,
                QAReporter * const r,
                const Environment * const v,
                const tr1::shared_ptr<const PackageID> & i,
                const tr1::shared_ptr<const ERepository> o,
                const std::set<KeywordName> & a,
                const ERepository::ProfilesConstIterator & p,
                const std::string & n,
                const bool u,
                const tr1::shared_ptr<const MetadataKey> & k) :
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
            success = false;
            viable = true;

            const PackageDepSpec * p(&orig_p);
            tr1::shared_ptr<PackageDepSpec> local_p;

            /* rewrite virtuals to avoid problems later on */
            if (p->package_ptr())
            {
                ERepositoryProfile::VirtualsConstIterator v(profile->profile->find_virtual(*p->package_ptr()));
                if (profile->profile->end_virtuals() != v)
                {
                    tr1::shared_ptr<VersionRequirements> reqs;
                    if (v->second->version_requirements_ptr())
                    {
                        reqs.reset(new VersionRequirements);
                        std::copy(v->second->version_requirements_ptr()->begin(), v->second->version_requirements_ptr()->end(),
                                reqs->back_inserter());
                    }
                    if (orig_p.version_requirements_ptr())
                    {
                        if (! reqs)
                            reqs.reset(new VersionRequirements);
                        std::copy(orig_p.version_requirements_ptr()->begin(), orig_p.version_requirements_ptr()->end(),
                                reqs->back_inserter());
                    }
                    local_p.reset(new PackageDepSpec(
                                make_shared_ptr(new QualifiedPackageName(*v->second->package_ptr())),
                                tr1::shared_ptr<CategoryNamePart>(),
                                tr1::shared_ptr<PackageNamePart>(),
                                reqs,
                                vr_and,
                                orig_p.slot_ptr() ?
                                    make_shared_ptr(new SlotName(*orig_p.slot_ptr())) :
                                    tr1::shared_ptr<SlotName>(),
                                orig_p.repository_ptr() ?
                                    make_shared_ptr(new RepositoryName(*orig_p.repository_ptr())) :
                                    tr1::shared_ptr<RepositoryName>(),
                                tr1::shared_ptr<UseRequirements>(),
                                orig_p.tag()
                                ));
                    p = local_p.get();
                }
            }

            const tr1::shared_ptr<const PackageIDSequence> matches(env->package_database()->query(
                        query::Matches(*p) & query::SupportsAction<InstallAction>(), qo_order_by_version));
            if (matches->empty())
            {
                if (reporter)
                    reporter->message(QAMessage(entry, qaml_normal, name, "No packages matching '"
                                + stringify(orig_p) + "' in dependencies key '" + stringify(key->raw_name()) + "' for profile '"
                                + stringify(profile->path) + "' (" + stringify(profile->arch) + "." + stringify(profile->status)
                                + (unstable ? ".unstable" : ".stable") + ")")
                            .with_associated_id(id)
                            .with_associated_key(key));
            }
            else
            {
                for (PackageIDSequence::ConstIterator i(matches->begin()), i_end(matches->end()) ;
                        i != i_end ; ++i)
                {
                    /* can't use the usual masked rules here, so this gets a bit complicated... */
                    if ((*i)->repository() == repo)
                    {
                        if (repo->repository_masked(**i) || profile->profile->profile_masked(**i) || ! (*i)->keywords_key())
                            continue;
                    }
                    else if ((*i)->repository() == repo->params().master_repository)
                    {
                        if (repo->params().master_repository->repository_masked(**i) ||
                                profile->profile->profile_masked(**i) || ! (*i)->keywords_key())
                            continue;
                    }
                    else
                    {
                        Log::get_instance()->message(ll_warning, lc_context) << "Probably a bug: don't know how to get masks for '"
                            << **i << "' from '" << orig_p << "' -> '" << *p << "'";
                        continue;
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
                                    + stringify(profile->path) + "' (" + stringify(profile->arch) + "." + stringify(profile->status)
                                    + (unstable ? ".unstable" : ".stable") + ")")
                                .with_associated_id(id)
                                .with_associated_key(key));
            }
        }

        void visit_sequence(const UseDepSpec & u,
                DependencySpecTree::ConstSequenceIterator cur,
                DependencySpecTree::ConstSequenceIterator end)
        {
            viable =
                ((! u.inverse()) && (! repo->query_use_mask(u.flag(), *id))) ||
                ((u.inverse()) && (! repo->query_use_force(u.flag(), *id)));

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
                    erepository::DepSpecPrettyPrinter printer(0, tr1::shared_ptr<const PackageID>(), ff, 0, false);
                    std::for_each(begin, end, accept_visitor(printer));
                    reporter->message(QAMessage(entry, qaml_normal, name, "No item in block '|| ( "
                                + stringify(printer) + " )' visible for profile '"
                                + stringify(profile->path) + "' (" + stringify(profile->arch) + "." + stringify(profile->status)
                                + (unstable ? ".unstable" : ".stable") + ")")
                            .with_associated_id(id)
                            .with_associated_key(key));
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
        const tr1::shared_ptr<const ERepository> & repo,
        const tr1::shared_ptr<const PackageID> & id,
        const std::string & name)
{
    Context context("When performing check '" + name + "' using visibility_check on ID '" + stringify(*id) + "':");

    Log::get_instance()->message(ll_debug, lc_context) << "visibility_check '"
        << entry << "', '" << *id << "', '" << name << "'";

    if (repo->repository_masked(*id) || ! id->keywords_key())
        return true;

    for (ERepository::ProfilesConstIterator p(repo->begin_profiles()), p_end(repo->end_profiles()) ;
            p != p_end ; ++p)
    {
        if (p->profile->profile_masked(*id))
            continue;

        std::set<KeywordName> accepted_keywords, overlap;
        WhitespaceTokeniser::get_instance()->tokenise(p->profile->environment_variable(
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

