/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Fernando J. Pereda <ferdy@gentoo.org>
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

#include "report_task.hh"
#include <paludis/util/log.hh>
#include <paludis/dep_list/uninstall_list.hh>
#include <paludis/environment.hh>
#include <paludis/query.hh>
#include <paludis/package_database.hh>
#include <set>

using namespace paludis;

namespace
{
    class VulnerableChecker :
        public DepSpecVisitorTypes::ConstVisitor,
        public DepSpecVisitorTypes::ConstVisitor::VisitChildren<VulnerableChecker, AllDepSpec>,
        public DepSpecVisitorTypes::ConstVisitor::VisitChildren<VulnerableChecker, AnyDepSpec>,
        public DepSpecVisitorTypes::ConstVisitor::VisitChildren<VulnerableChecker, UseDepSpec>
    {
        private:
            std::multimap<PackageDatabaseEntry, std::tr1::shared_ptr<const DepTag>,
                ArbitrarilyOrderedPackageDatabaseEntryCollectionComparator> _found;
            const Environment & _env;

        public:
            typedef std::multimap<PackageDatabaseEntry, std::tr1::shared_ptr<const DepTag> >::const_iterator ConstIterator;

            using DepSpecVisitorTypes::ConstVisitor::VisitChildren<VulnerableChecker, AllDepSpec>::visit;
            using DepSpecVisitorTypes::ConstVisitor::VisitChildren<VulnerableChecker, UseDepSpec>::visit;
            using DepSpecVisitorTypes::ConstVisitor::VisitChildren<VulnerableChecker, AnyDepSpec>::visit;

            /**
             * Constructor.
             */
            VulnerableChecker(const Environment & e) :
                _env(e)
            {
            }

            /// \name Visit functions
            ///{

            void visit(const PackageDepSpec * const);

            void visit(const PlainTextDepSpec * const)
            {
            }

            void visit(const BlockDepSpec * const)
            {
            }
            ///}

            /**
             * Return whether a PDE is insecure or not
             */
            std::pair<ConstIterator, ConstIterator> insecure_tags(const PackageDatabaseEntry & pde) const
            {
                return _found.equal_range(pde);
            }
    };

    void
    VulnerableChecker::visit(const PackageDepSpec * const a)
    {
        std::tr1::shared_ptr<const PackageDatabaseEntryCollection> insecure(
                _env.package_database()->query(query::Matches(*a), qo_order_by_version));
        for (PackageDatabaseEntryCollection::Iterator i(insecure->begin()),
                i_end(insecure->end()) ; i != i_end ; ++i)
            if (a->tag())
                _found.insert(std::make_pair(*i, a->tag()));
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
    for (PackageDatabase::RepositoryIterator r(e->package_database()->begin_repositories()),
            r_end(e->package_database()->end_repositories()) ; r != r_end ; ++r)
    {
        std::tr1::shared_ptr<const Repository> rr(e->package_database()->fetch_repository((*r)->name()));
        if (! rr->sets_interface)
            continue;

        try
        {
            std::tr1::shared_ptr<const DepSpec> insecure(rr->sets_interface->package_set(SetName("insecurity")));
            if (! insecure)
                continue;
            insecure->accept(&vuln);
        }
        catch (const NotAvailableError &)
        {
            if (once)
            {
                Log::get_instance()->message(ll_warning, lc_no_context,
                        "Skipping GLSA checks because Paludis was built without XML support");
                once = false;
            }
        }
    }

    UninstallList unused_list(e, UninstallListOptions());
    unused_list.add_unused();
    std::set<PackageDatabaseEntry, ArbitrarilyOrderedPackageDatabaseEntryCollectionComparator> unused;
    for (UninstallList::Iterator i(unused_list.begin()), i_end(unused_list.end());
            i != i_end ; ++i)
        if (! i->skip_uninstall)
            unused.insert(i->package);

    for (PackageDatabase::RepositoryIterator r(e->package_database()->begin_repositories()),
            r_end(e->package_database()->end_repositories()) ; r != r_end ; ++r)
    {
        std::tr1::shared_ptr<const Repository> rr(e->package_database()->fetch_repository((*r)->name()));
        if (! rr->installed_interface)
            continue;

        std::tr1::shared_ptr<const CategoryNamePartCollection> cat_names(rr->category_names());
        for (CategoryNamePartCollection::Iterator c(cat_names->begin()), c_end(cat_names->end()) ;
                    c != c_end ; ++c)
        {
            std::tr1::shared_ptr<const QualifiedPackageNameCollection> packages(rr->package_names(*c));
            for (QualifiedPackageNameCollection::Iterator p(packages->begin()), p_end(packages->end()) ;
                    p != p_end ; ++p)
            {
                on_report_check_package_pre(*p);

                std::tr1::shared_ptr<const VersionSpecCollection> vers(rr->version_specs(*p));
                for (VersionSpecCollection::Iterator v(vers->begin()), v_end(vers->end()) ;
                        v != v_end ; ++v)
                {
                    PackageDatabaseEntry pde(*p, *v, rr->name());
                    bool is_masked(false);
                    bool is_vulnerable(false);
                    bool is_missing(false);
                    bool is_unused(false);

                    MaskReasons mr;
                    try
                    {
                        std::tr1::shared_ptr<const VersionMetadata> m(rr->version_metadata(pde.name, pde.version));

                        if (m->origins_interface && m->origins_interface->source)
                        {
                            mr = e->mask_reasons(*(m->origins_interface->source));
                            if (mr.any())
                                is_masked = true;
                        }
                    }
                    catch (const NoSuchPackageError &)
                    {
                        is_missing = true;
                    }
                    catch (const NoSuchRepositoryError &)
                    {
                        is_missing = true;
                    }

                    std::pair<VulnerableChecker::ConstIterator, VulnerableChecker::ConstIterator> pi(
                            vuln.insecure_tags(pde));
                    if (pi.first != pi.second)
                        is_vulnerable = true;

                    if (unused.end() != unused.find(pde))
                        is_unused = true;

                    if (is_masked || is_vulnerable || is_missing || is_unused)
                    {
                        on_report_package_failure_pre(pde);
                        if (is_masked)
                            on_report_package_is_masked(pde, mr);
                        if (is_vulnerable)
                        {
                            on_report_package_is_vulnerable_pre(pde);
                            for (VulnerableChecker::ConstIterator itag(pi.first) ; itag != pi.second ; ++itag)
                                on_report_package_is_vulnerable(pde, itag->second->short_text());
                            on_report_package_is_vulnerable_post(pde);
                        }
                        if (is_missing)
                            on_report_package_is_missing(pde);
                        if (is_unused)
                            on_report_package_is_unused(pde);
                        on_report_package_failure_post(pde);
                    }
                    else
                        on_report_package_success(pde);

                }

                on_report_check_package_post(*p);
            }
        }
    }

    on_report_all_post();
}
