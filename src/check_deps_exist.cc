/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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

#include <paludis/paludis.hh>
#include <paludis/args/args.hh>
#include <cstdlib>
#include <iostream>
#include <algorithm>

using namespace paludis;
using std::cout;
using std::cerr;
using std::endl;

#ifndef DOXYGEN
struct CheckDepsExistCommandline :
    args::ArgsHandler,
    InstantiationPolicy<CheckDepsExistCommandline, instantiation_method::SingletonAsNeededTag>
{
    friend class InstantiationPolicy<CheckDepsExistCommandline, instantiation_method::SingletonAsNeededTag>;

    args::ArgsGroup a_general_args;
    args::SwitchArg a_help;
    args::SwitchArg a_check_blockers;

    CheckDepsExistCommandline() :
        a_general_args(this, "General options"),
        a_help(&a_general_args, "help", 'h', "Display a help message"),
        a_check_blockers(&a_general_args, "check-blockers", 'b', "Include blockers in checks")
    {
    }
};

struct DepExistsChecker :
    DepAtomVisitorTypes::ConstVisitor
{
    Environment * const env;
    int status;

    DepExistsChecker(Environment * const e) :
        env(e),
        status(0)
    {
    }

    void visit(const AllDepAtom * a)
    {
        std::for_each(a->begin(), a->end(), accept_visitor(this));
    }

    void visit(const AnyDepAtom * a)
    {
        std::for_each(a->begin(), a->end(), accept_visitor(this));
    }

    void visit(const UseDepAtom * a)
    {
        std::for_each(a->begin(), a->end(), accept_visitor(this));
    }

    void visit(const BlockDepAtom * a)
    {
        if (CheckDepsExistCommandline::get_instance()->a_check_blockers.specified())
            a->blocked_atom()->accept(this);
    }

    void visit(const PackageDepAtom * a)
    {
        if (env->package_database()->query(a)->empty())
        {
            status |= 1;
            cout << *a << " ";
        }
    }
};

int
check_one(const std::string & a)
{
    Context context("When checking '" + a + "':");

    Environment * const env(DefaultEnvironment::get_instance());
    PackageDepAtom::Pointer atom(new PackageDepAtom(a));

    PackageDatabaseEntryCollection::ConstPointer entries(env->package_database()->query(atom));
    if (entries->empty())
        throw NoSuchPackageError(a);

    int retcode(0);
    for (PackageDatabaseEntryCollection::Iterator entry(entries->begin()),
            entry_end(entries->end()) ; entry != entry_end ; ++entry)
    {
        Context local_context("When checking '" + stringify(*entry) + "':");
        cout << *entry << ": ";

        DepExistsChecker checker(env);
        VersionMetadata::ConstPointer metadata(env->package_database()->fetch_metadata(*entry));
        DepParser::parse(metadata->get(vmk_depend))->accept(&checker);
        DepParser::parse(metadata->get(vmk_rdepend))->accept(&checker);
        DepParser::parse(metadata->get(vmk_pdepend))->accept(&checker);

        cout << (checker.status ? "BAD" : "OK") << endl;
        retcode |= checker.status;
    }

    return retcode;
}

struct DoHelp
{
    const std::string message;

    DoHelp(const std::string & m = "") :
        message(m)
    {
    }
};
#endif

int
main(int argc, char *argv[])
{
    Context context("In main program:");

    int retcode = EXIT_SUCCESS;

    try
    {
        CheckDepsExistCommandline::get_instance()->run(argc, argv);

        if (CheckDepsExistCommandline::get_instance()->a_help.specified())
            throw DoHelp();

        if (CheckDepsExistCommandline::get_instance()->empty())
            throw DoHelp("Expected at least one parameter");

        for (CheckDepsExistCommandline::ParametersIterator
                a(CheckDepsExistCommandline::get_instance()->begin_parameters()),
                a_end(CheckDepsExistCommandline::get_instance()->end_parameters()) ;
                a != a_end ; ++a)
            retcode |= check_one(*a);
    }
    catch (const args::ArgsError & e)
    {
        cerr << "Usage error: " << e.message() << endl;
        cerr << "Try " << argv[0] << " --help" << endl;
        return EXIT_FAILURE;
    }
    catch (const Exception & e)
    {
        cout << endl;
        cerr << "Unhandled exception:" << endl
            << "  * " << e.backtrace("\n  * ")
            << e.message() << " (" << e.what() << ")" << endl;
        return EXIT_FAILURE;
    }
    catch (const std::exception & e)
    {
        cout << endl;
        cerr << "Unhandled exception:" << endl
            << "  * " << e.what() << endl;
        return EXIT_FAILURE;
    }
    catch (const DoHelp & h)
    {
        if (h.message.empty())
        {
            cout << "Usage: " << argv[0] << " [options]" << endl;
            cout << endl;
            cout << *CheckDepsExistCommandline::get_instance();
            return EXIT_SUCCESS;
        }
        else
        {
            cerr << "Usage error: " << h.message << endl;
            cerr << "Try " << argv[0] << " --help" << endl;
            return EXIT_FAILURE;
        }
    }
    catch (...)
    {
        cout << endl;
        cerr << "Unhandled exception:" << endl
            << "  * Unknown exception type. Ouch..." << endl;
        return EXIT_FAILURE;
    }

    return retcode;
}

