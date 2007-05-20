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

#include "do_config.hh"
#include "command_line.hh"
#include <src/output/colour.hh>
#include <paludis/paludis.hh>
#include <iostream>

using namespace paludis;
using std::cout;
using std::cerr;
using std::endl;

namespace
{
    struct AmbiguousConfigTarget :
        public Exception
    {
        tr1::shared_ptr<const PackageDatabaseEntryCollection> matches;

        AmbiguousConfigTarget(tr1::shared_ptr<const PackageDatabaseEntryCollection> & m) throw () :
            Exception("Ambiguous config target"),
            matches(m)
        {
        }

        ~AmbiguousConfigTarget() throw ()
        {
        }
    };

    int
    do_one_config_entry(tr1::shared_ptr<Environment> env, const PackageDatabaseEntry & p)
    {
        int return_code(0);

        tr1::shared_ptr<const Repository> repo(env->package_database()->fetch_repository(p.repository));
        const RepositoryConfigInterface * conf_if(repo->config_interface);

        if (! conf_if)
        {
            std::cerr << "Repository '" << repo->name() <<
                "' does not support post-install configuration" << std::endl;
            return_code |= 1;
        }
        else
            conf_if->config(p.name, p.version);

        return return_code;
    }

    int
    do_one_config(tr1::shared_ptr<Environment> env, const std::string & target)
    {
        Context local_context("When handling query '" + target + "':");

        /* we might have a dep spec, but we might just have a simple package name
         * without a category. either should work. */
        tr1::shared_ptr<PackageDepSpec> spec(std::string::npos == target.find('/') ?
                new PackageDepSpec(tr1::shared_ptr<QualifiedPackageName>(new QualifiedPackageName(
                            env->package_database()->fetch_unique_qualified_package_name(
                                PackageNamePart(target))))) :
                new PackageDepSpec(target, pds_pm_permissive));

        tr1::shared_ptr<const PackageDatabaseEntryCollection>
            entries(env->package_database()->query(query::Matches(*spec) & query::InstalledAtRoot(env->root()), qo_order_by_version));

        if (entries->empty())
            throw NoSuchPackageError(target);

        if (next(entries->begin()) != entries->end())
            throw AmbiguousConfigTarget(entries);

        return do_one_config_entry(env, *entries->begin());
    }
}

int
do_config(tr1::shared_ptr<Environment> env)
{
    int ret_code(0);

    Context context("When performing config action from command line:");

    CommandLine::ParametersIterator q(CommandLine::get_instance()->begin_parameters()),
        q_end(CommandLine::get_instance()->end_parameters());
    for ( ; q != q_end ; ++q)
    {
        try
        {
            ret_code |= do_one_config(env, *q);
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
        catch (const AmbiguousConfigTarget & e)
        {
            cout << endl;
            cerr << "Query error:" << endl;
            cerr << "  * " << e.backtrace("\n  * ");
            cerr << "Ambiguous config target '" << *q << "'. Did you mean:" << endl;
            for (PackageDatabaseEntryCollection::Iterator o(e.matches->begin()),
                    o_end(e.matches->end()) ; o != o_end ; ++o)
                cerr << "    * =" << colour(cl_package_name, *o) << endl;
            cerr << endl;
        }
    }

    return ret_code;
}


