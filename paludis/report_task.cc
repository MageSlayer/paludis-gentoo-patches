/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Fernando J. Pereda
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

#include <paludis/report_task.hh>
#include <paludis/util/log.hh>
#include <paludis/uninstall_list.hh>
#include <paludis/environment.hh>
#include <paludis/metadata_key.hh>
#include <paludis/dep_tag.hh>
#include <paludis/package_id.hh>
#include <paludis/selection.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/simple_visitor_cast.hh>
#include <paludis/util/set.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/accept_visitor.hh>
#include <paludis/util/sequence.hh>
#include <paludis/package_database.hh>
#include <paludis/version_requirements.hh>
#include <algorithm>
#include <set>
#include <map>

using namespace paludis;

namespace
{
    class VulnerableChecker
    {
        private:
            std::multimap<std::shared_ptr<const PackageID>, std::shared_ptr<const DepTag>, PackageIDSetComparator> _found;
            const Environment & _env;
            std::set<SetName> _recursing_sets;

        public:
            typedef std::multimap<std::shared_ptr<const PackageID>, std::shared_ptr<const DepTag>,
                    PackageIDSetComparator>::const_iterator ConstIterator;

            VulnerableChecker(const Environment & e) :
                _env(e)
            {
            }

            void visit(const SetSpecTree::NodeType<AllDepSpec>::Type & node)
            {
                std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
            }

            void visit(const SetSpecTree::NodeType<PackageDepSpec>::Type & node);

            void visit(const SetSpecTree::NodeType<NamedSetDepSpec>::Type & node)
            {
                Context context("When expanding named set '" + stringify(*node.spec()) + "':");

                std::shared_ptr<const SetSpecTree> set(_env.set(node.spec()->name()));
                if (! set)
                {
                    Log::get_instance()->message("report_task.unknown_set", ll_warning, lc_context)
                        << "Unknown set '" << node.spec()->name() << "'";
                    return;
                }

                if (! _recursing_sets.insert(node.spec()->name()).second)
                {
                    Log::get_instance()->message("report_task.recursive_set", ll_warning, lc_context)
                        << "Recursively defined set '" << node.spec()->name() << "'";
                    return;
                }

                set->root()->accept(*this);

                _recursing_sets.erase(node.spec()->name());
            }

            std::pair<ConstIterator, ConstIterator> insecure_tags(const std::shared_ptr<const PackageID> & id) const
            {
                return _found.equal_range(id);
            }
    };

    void
    VulnerableChecker::visit(const SetSpecTree::NodeType<PackageDepSpec>::Type & node)
    {
        std::shared_ptr<const PackageIDSequence> insecure(_env[selection::AllVersionsSorted(
                    generator::Matches(*node.spec(), MatchPackageOptions()))]);
        for (PackageIDSequence::ConstIterator i(insecure->begin()),
                i_end(insecure->end()) ; i != i_end ; ++i)
            if (node.spec()->tag() && simple_visitor_cast<const GLSADepTag>(*node.spec()->tag()))
                _found.insert(std::make_pair(*i, node.spec()->tag()));
            else
                throw InternalError(PALUDIS_HERE, "didn't get a tag");
    }
}

namespace paludis
{
    template<>
    struct Implementation<ReportTask>
    {
        Environment * const env;

        Implementation(Environment * const e) :
            env(e)
        {
        }
    };
}

ReportTask::ReportTask(Environment * const env) :
    PrivateImplementationPattern<ReportTask>(new Implementation<ReportTask>(env))
{
}

ReportTask::~ReportTask()
{
}

void
ReportTask::execute()
{
    Context context("When executing report task:");

    on_report_all_pre();

    paludis::Environment * const e(_imp->env);
    bool once(true);

    VulnerableChecker vuln(*e);
    try
    {
        std::shared_ptr<const SetSpecTree> insecure(_imp->env->set(SetName("insecurity")));
        if (insecure)
            insecure->root()->accept(vuln);
    }
    catch (const NotAvailableError &)
    {
        if (once)
        {
            Log::get_instance()->message("report_task.skipping_glsas", ll_warning, lc_no_context)
                << "Skipping GLSA checks because Paludis was built without XML support";
            once = false;
        }
    }

    UninstallList unused_list(e, make_named_values<UninstallListOptions>(
                n::with_dependencies_as_errors() = false,
                n::with_dependencies_included() = false,
                n::with_unused_dependencies() = false
                ));
    unused_list.add_unused();
    std::set<std::shared_ptr<const PackageID>, PackageIDSetComparator> unused;
    for (UninstallList::ConstIterator i(unused_list.begin()), i_end(unused_list.end());
            i != i_end ; ++i)
        if (i->kind() != ulk_virtual)
            unused.insert(i->package_id());

    for (PackageDatabase::RepositoryConstIterator r(e->package_database()->begin_repositories()),
            r_end(e->package_database()->end_repositories()) ; r != r_end ; ++r)
    {
        std::shared_ptr<const Repository> rr(e->package_database()->fetch_repository((*r)->name()));
        if (! rr->installed_root_key())
            continue;

        std::shared_ptr<const CategoryNamePartSet> cat_names(rr->category_names());
        for (CategoryNamePartSet::ConstIterator c(cat_names->begin()), c_end(cat_names->end()) ;
                    c != c_end ; ++c)
        {
            std::shared_ptr<const QualifiedPackageNameSet> packages(rr->package_names(*c));
            for (QualifiedPackageNameSet::ConstIterator p(packages->begin()), p_end(packages->end()) ;
                    p != p_end ; ++p)
            {
                on_report_check_package_pre(*p);

                std::shared_ptr<const PackageIDSequence> ids(rr->package_ids(*p));
                for (PackageIDSequence::ConstIterator v(ids->begin()), v_end(ids->end()) ;
                        v != v_end ; ++v)
                {
                    bool is_missing(false);
                    std::shared_ptr<PackageIDSequence> origins;

                    if ((*v)->from_repositories_key())
                    {
                        is_missing = ! ((*v)->behaviours_key() && (*v)->behaviours_key()->value()->end() !=
                                (*v)->behaviours_key()->value()->find("transient"));

                        for (Set<std::string>::ConstIterator o((*v)->from_repositories_key()->value()->begin()),
                                o_end((*v)->from_repositories_key()->value()->end()) ;
                                o != o_end ; ++o)
                        {
                            std::shared_ptr<const PackageIDSequence> installable(
                                    (*e)[selection::BestVersionOnly((
                                            (generator::InRepository(RepositoryName(*o)) &
                                             generator::Matches(make_package_dep_spec(PartiallyMadePackageDepSpecOptions())
                                                 .package((*v)->name())
                                                 .version_requirement(make_named_values<VersionRequirement>(
                                                         n::version_operator() = vo_equal,
                                                         n::version_spec() = (*v)->version())),
                                                 MatchPackageOptions())) |
                                            filter::SupportsAction<InstallAction>()))]);

                            if (! installable->empty())
                            {
                                is_missing = false;
                                if (! origins)
                                    origins.reset(new PackageIDSequence);
                                origins->push_back(*installable->last());
                            }
                        }
                    }

                    bool is_masked(origins && origins->end() != std::find_if(origins->begin(),
                                origins->end(), std::bind(std::mem_fn(&PackageID::masked),
                                    std::placeholders::_1)));
                    bool is_vulnerable(false);
                    bool is_unused(false);

                    std::pair<VulnerableChecker::ConstIterator, VulnerableChecker::ConstIterator> pi(vuln.insecure_tags(*v));
                    if (pi.first != pi.second)
                        is_vulnerable = true;

                    if (unused.end() != unused.find(*v))
                        is_unused = true;

                    if (is_masked || is_vulnerable || is_missing || is_unused)
                    {
                        on_report_package_failure_pre(*v);
                        if (is_masked)
                            on_report_package_is_masked(*v, origins);
                        if (is_vulnerable)
                        {
                            on_report_package_is_vulnerable_pre(*v);
                            for (VulnerableChecker::ConstIterator itag(pi.first) ; itag != pi.second ; ++itag)
                                on_report_package_is_vulnerable(*v, *static_cast<const GLSADepTag *>(itag->second.get()) );
                            on_report_package_is_vulnerable_post(*v);
                        }
                        if (is_missing)
                            on_report_package_is_missing(*v);
                        if (is_unused)
                            on_report_package_is_unused(*v);
                        on_report_package_failure_post(*v);
                    }
                    else
                        on_report_package_success(*v);
                }

                on_report_check_package_post(*p);
            }
        }
    }

    on_report_all_post();
}

