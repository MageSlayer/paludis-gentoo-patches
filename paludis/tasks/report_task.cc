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
#include <paludis/metadata_key.hh>
#include <paludis/dep_tag.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/set.hh>
#include <paludis/package_database.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>
#include <libwrapiter/libwrapiter_output_iterator.hh>
#include <set>
#include <map>

using namespace paludis;

namespace
{
    class VulnerableChecker :
        public ConstVisitor<SetSpecTree>,
        public ConstVisitor<SetSpecTree>::VisitConstSequence<VulnerableChecker, AllDepSpec>
    {
        private:
            std::multimap<tr1::shared_ptr<const PackageID>, tr1::shared_ptr<const DepTag>, PackageIDSetComparator> _found;
            const Environment & _env;

        public:
            typedef std::multimap<tr1::shared_ptr<const PackageID>, tr1::shared_ptr<const DepTag>,
                    PackageIDSetComparator>::const_iterator ConstIterator;

            using ConstVisitor<SetSpecTree>::VisitConstSequence<VulnerableChecker, AllDepSpec>::visit;

            /**
             * Constructor.
             */
            VulnerableChecker(const Environment & e) :
                _env(e)
            {
            }

            /// \name Visit functions
            ///{

            void visit_leaf(const PackageDepSpec &);

            ///}

            /**
             * Return whether a PDE is insecure or not
             */
            std::pair<ConstIterator, ConstIterator> insecure_tags(const tr1::shared_ptr<const PackageID> & id) const
            {
                return _found.equal_range(id);
            }
    };

    void
    VulnerableChecker::visit_leaf(const PackageDepSpec & a)
    {
        tr1::shared_ptr<const PackageIDSequence> insecure(
                _env.package_database()->query(query::Matches(a), qo_order_by_version));
        for (PackageIDSequence::Iterator i(insecure->begin()),
                i_end(insecure->end()) ; i != i_end ; ++i)
            if (a.tag())
                _found.insert(std::make_pair(*i, a.tag()));
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
        tr1::shared_ptr<const Repository> rr(e->package_database()->fetch_repository((*r)->name()));
        if (! rr->sets_interface)
            continue;

        try
        {
            tr1::shared_ptr<const SetSpecTree::ConstItem> insecure(rr->sets_interface->package_set(SetName("insecurity")));
            if (! insecure)
                continue;
            insecure->accept(vuln);
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
    std::set<tr1::shared_ptr<const PackageID>, PackageIDSetComparator> unused;
    for (UninstallList::Iterator i(unused_list.begin()), i_end(unused_list.end());
            i != i_end ; ++i)
        if (! i->skip_uninstall)
            unused.insert(i->package_id);

    for (PackageDatabase::RepositoryIterator r(e->package_database()->begin_repositories()),
            r_end(e->package_database()->end_repositories()) ; r != r_end ; ++r)
    {
        tr1::shared_ptr<const Repository> rr(e->package_database()->fetch_repository((*r)->name()));
        if (! rr->installed_interface)
            continue;

        tr1::shared_ptr<const CategoryNamePartSet> cat_names(rr->category_names());
        for (CategoryNamePartSet::Iterator c(cat_names->begin()), c_end(cat_names->end()) ;
                    c != c_end ; ++c)
        {
            tr1::shared_ptr<const QualifiedPackageNameSet> packages(rr->package_names(*c));
            for (QualifiedPackageNameSet::Iterator p(packages->begin()), p_end(packages->end()) ;
                    p != p_end ; ++p)
            {
                on_report_check_package_pre(*p);

                tr1::shared_ptr<const PackageIDSequence> ids(rr->package_ids(*p));
                for (PackageIDSequence::Iterator v(ids->begin()), v_end(ids->end()) ;
                        v != v_end ; ++v)
                {
                    bool is_masked(false);
                    bool is_vulnerable(false);
                    bool is_missing(false);
                    bool is_unused(false);

                    MaskReasons mr;
                    try
                    {
#if 0
                        if ((*v)->source_origin_key())
                        {
                            mr = e->mask_reasons(*((*v)->source_origin_key()->value()));
                            if (mr.any())
                                is_masked = true;
                        }
#endif
                    }
                    catch (const NoSuchPackageError &)
                    {
                        is_missing = true;
                    }
                    catch (const NoSuchRepositoryError &)
                    {
                        is_missing = true;
                    }

                    std::pair<VulnerableChecker::ConstIterator, VulnerableChecker::ConstIterator> pi(vuln.insecure_tags(*v));
                    if (pi.first != pi.second)
                        is_vulnerable = true;

                    if (unused.end() != unused.find(*v))
                        is_unused = true;

                    if (is_masked || is_vulnerable || is_missing || is_unused)
                    {
                        on_report_package_failure_pre(**v);
                        if (is_masked)
                            on_report_package_is_masked(**v, mr);
                        if (is_vulnerable)
                        {
                            on_report_package_is_vulnerable_pre(**v);
                            for (VulnerableChecker::ConstIterator itag(pi.first) ; itag != pi.second ; ++itag)
                                on_report_package_is_vulnerable(**v, itag->second->short_text());
                            on_report_package_is_vulnerable_post(**v);
                        }
                        if (is_missing)
                            on_report_package_is_missing(**v);
                        if (is_unused)
                            on_report_package_is_unused(**v);
                        on_report_package_failure_post(**v);
                    }
                    else
                        on_report_package_success(**v);
                }

                on_report_check_package_post(*p);
            }
        }
    }

    on_report_all_post();
}

