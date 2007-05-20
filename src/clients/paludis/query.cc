/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <src/output/colour.hh>
#include "query.hh"
#include <src/output/licence.hh>
#include <src/output/use_flag_pretty_printer.hh>
#include <src/output/console_query_task.hh>
#include <functional>
#include <iomanip>
#include <iostream>
#include <paludis/paludis.hh>
#include <paludis/util/collection_concrete.hh>
#include <string>

/** \file
 * Handle the --query action for the main paludis program.
 */

using namespace paludis;
using std::cout;
using std::cerr;
using std::endl;

namespace
{
    class QueryTask :
        public ConsoleQueryTask
    {
        public:
            QueryTask(const tr1::shared_ptr<Environment> e) :
                ConsoleQueryTask(e.get())
            {
            }

            bool want_deps() const
            {
                return CommandLine::get_instance()->a_show_deps.specified() || want_raw();
            }

            bool want_raw() const
            {
                return CommandLine::get_instance()->a_show_metadata.specified();
            }
    };
}

void do_one_package_query(
        const tr1::shared_ptr<Environment> env,
        MaskReasons & mask_reasons_to_explain,
        tr1::shared_ptr<PackageDepSpec> spec)
{
    QueryTask query(env);
    query.show(*spec);
    mask_reasons_to_explain |= query.mask_reasons_to_explain();
    cout << endl;
}

void do_one_set_query(
        const tr1::shared_ptr<Environment>,
        const std::string & q,
        MaskReasons &,
        tr1::shared_ptr<DepSpec> set)
{
    cout << "* " << colour(cl_package_name, q) << endl;
    DepSpecPrettyPrinter packages(12);
    set->accept(&packages);
    cout << "    " << std::setw(22) << std::left << "Packages:" << std::setw(0)
        << endl << packages << endl;
}

void do_one_query(
        const tr1::shared_ptr<Environment> env,
        const std::string & q,
        MaskReasons & mask_reasons_to_explain)
{
    Context local_context("When handling query '" + q + "':");

    /* we might have a dep spec, but we might just have a simple package name
     * without a category. or it might be a set... all should work. */
    tr1::shared_ptr<PackageDepSpec> spec;
    tr1::shared_ptr<DepSpec> set;
    if (std::string::npos == q.find('/'))
    {
        try
        {
            set = env->set(SetName(q));
        }
        catch (const SetNameError &)
        {
        }
        if (0 == set)
            spec.reset(new PackageDepSpec(tr1::shared_ptr<QualifiedPackageName>(new QualifiedPackageName(
                                env->package_database()->fetch_unique_qualified_package_name(PackageNamePart(q))))));
    }
    else
        spec.reset(new PackageDepSpec(q, pds_pm_permissive));

    if (spec)
        do_one_package_query(env, mask_reasons_to_explain, spec);
    else
        do_one_set_query(env, q, mask_reasons_to_explain, set);
}

int do_query(tr1::shared_ptr<Environment> env)
{
    int return_code(0);

    Context context("When performing query action from command line:");

    MaskReasons mask_reasons_to_explain;

    CommandLine::ParametersIterator q(CommandLine::get_instance()->begin_parameters()),
        q_end(CommandLine::get_instance()->end_parameters());
    for ( ; q != q_end ; ++q)
    {
        try
        {
            do_one_query(env, *q, mask_reasons_to_explain);
        }
        catch (const AmbiguousPackageNameError & e)
        {
            cout << endl;
            cerr << "Query error:" << endl;
            cerr << "  * " << e.backtrace("\n  * ");
            cerr << "Ambiguous package name '" << e.name() << "'. Did you mean:" << endl;
            for (AmbiguousPackageNameError::OptionsIterator o(e.begin_options()),
                    o_end(e.end_options()) ; o != o_end ; ++o)
                cerr << "    * " << colour(cl_package_name, *o) << endl;
            cerr << endl;
        }
        catch (const NameError & e)
        {
            return_code |= 1;
            cout << endl;
            cerr << "Query error:" << endl;
            cerr << "  * " << e.backtrace("\n  * ") << e.message() << endl;
            cerr << endl;
        }
        catch (const PackageDatabaseLookupError & e)
        {
            return_code |= 1;
            cout << endl;
            cerr << "Query error:" << endl;
            cerr << "  * " << e.backtrace("\n  * ") << e.message() << endl;
            cerr << endl;
        }
    }

    if (mask_reasons_to_explain.any())
    {
        cout << colour(cl_heading, "Key to mask reasons:") << endl << endl;

        /* use for/case to get compiler warnings when new mr_ are added */
        for (MaskReason m(MaskReason(0)) ; m < last_mr ;
                m = MaskReason(static_cast<int>(m) + 1))
        {
            if (! mask_reasons_to_explain[m])
                continue;

            switch (m)
            {
                case mr_keyword:
                    cout << "* " << colour(cl_masked, "K") << ": keyword";
                    break;
                case mr_user_mask:
                    cout << "* " << colour(cl_masked, "U") << ": user mask";
                    break;
                case mr_profile_mask:
                    cout << "* " << colour(cl_masked, "P") << ": profile mask";
                    break;
                case mr_repository_mask:
                    cout << "* " << colour(cl_masked, "R") << ": repository mask";
                    break;
                case mr_eapi:
                    cout << "* " << colour(cl_masked, "E") << ": EAPI";
                    break;
                case mr_license:
                    cout << "* " << colour(cl_masked, "L") << ": licence";
                    break;
                case mr_by_association:
                    cout << "* " << colour(cl_masked, "A") << ": by association";
                    break;
                case mr_chost:
                    cout << "* " << colour(cl_masked, "C") << ": wrong CHOST";
                    break;
                case mr_breaks_portage:
                    cout << "* " << colour(cl_masked, "B") << ": breaks Portage";
                    break;
                case mr_interactive:
                    cout << "* " << colour(cl_masked, "I") << ": interactive";

                case last_mr:
                    break;
            }

            cout << endl;
        }

        cout << endl;
    }

    return return_code;
}

