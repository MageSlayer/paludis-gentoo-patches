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
        std::tr1::shared_ptr<const PackageIDSequence> matches;

        AmbiguousConfigTarget(std::tr1::shared_ptr<const PackageIDSequence> & m) throw () :
            Exception("Ambiguous config target"),
            matches(m)
        {
        }

        ~AmbiguousConfigTarget() throw ()
        {
        }
    };

    int
    do_one_config_entry(const std::tr1::shared_ptr<const PackageID> & p)
    {
        int return_code(0);

        ConfigAction a;
        try
        {
            p->perform_action(a);
        }
        catch (const UnsupportedActionError &)
        {
            std::cerr << "Package '" << *p << "' does not support post-install configuration" << std::endl;
            return_code |= 1;
        }

        return return_code;
    }

    int
    do_one_config(std::tr1::shared_ptr<Environment> env, const std::string & target)
    {
        Context local_context("When handling query '" + target + "':");

        /* we might have a dep spec, but we might just have a simple package name
         * without a category. either should work. */
        std::tr1::shared_ptr<PackageDepSpec> spec(std::string::npos == target.find('/') ?
                new PackageDepSpec(make_package_dep_spec().package(
                        env->package_database()->fetch_unique_qualified_package_name(
                            PackageNamePart(target), query::InstalledAtRoot(env->root())))) :
                new PackageDepSpec(parse_user_package_dep_spec(target, UserPackageDepSpecOptions())));

        std::tr1::shared_ptr<const PackageIDSequence>
            entries(env->package_database()->query(query::Matches(*spec) & query::InstalledAtRoot(env->root()), qo_order_by_version));

        if (entries->empty())
            throw NoSuchPackageError(target);

        if (next(entries->begin()) != entries->end())
            throw AmbiguousConfigTarget(entries);

        return do_one_config_entry(*entries->begin());
    }
}

int
do_config(std::tr1::shared_ptr<Environment> env)
{
    int ret_code(0);

    Context context("When performing config action from command line:");

    CommandLine::ParametersConstIterator q(CommandLine::get_instance()->begin_parameters()),
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
            for (AmbiguousPackageNameError::OptionsConstIterator o(e.begin_options()),
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
            for (PackageIDSequence::ConstIterator o(e.matches->begin()),
                    o_end(e.matches->end()) ; o != o_end ; ++o)
                cerr << "    * =" << colour(cl_package_name, **o) << endl;
            cerr << endl;
        }
    }

    return ret_code;
}


