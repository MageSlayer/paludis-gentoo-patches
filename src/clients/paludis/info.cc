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

#include "info.hh"
#include "command_line.hh"
#include "src/output/colour.hh"
#include "src/output/colour_formatter.hh"
#include <paludis/about.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/simple_visitor_cast.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/system.hh>
#include <paludis/util/pretty_print.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/timestamp.hh>
#include <paludis/package_database.hh>
#include <paludis/environment.hh>
#include <paludis/package_id.hh>
#include <paludis/action.hh>
#include <paludis/metadata_key.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/selection.hh>
#include <paludis/create_output_manager_info.hh>
#include <paludis/output_manager_from_environment.hh>
#include <paludis/output_manager.hh>
#include <iostream>
#include <iomanip>
#include <set>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>

using namespace paludis;
using std::endl;
using std::flush;
using std::cout;

namespace
{
    struct MetadataKeyComparator
    {
        bool operator() (const std::tr1::shared_ptr<const MetadataKey> & a, const std::tr1::shared_ptr<const MetadataKey> & b) const
        {
            bool a_is_section(simple_visitor_cast<const MetadataSectionKey>(*a));
            bool b_is_section(simple_visitor_cast<const MetadataSectionKey>(*b));
            if (a_is_section != b_is_section)
                return b_is_section;
            if (a->type() != b->type())
                return a->type() < b->type();
            return a->human_name() < b->human_name();
        }
    };

    struct InfoDisplayer
    {
        std::string indent;

        InfoDisplayer(const std::string & i) :
            indent(i)
        {
        }

        void visit(const MetadataSectionKey & k)
        {
            cout << endl;
            cout << indent << colour(cl_heading, k.human_name() + ":") << endl;
            std::set<std::tr1::shared_ptr<const MetadataKey>, MetadataKeyComparator > keys(
                    k.begin_metadata(), k.end_metadata());
            InfoDisplayer i(indent + "    ");
            for (std::set<std::tr1::shared_ptr<const MetadataKey>, MetadataKeyComparator>::const_iterator
                    e(keys.begin()), e_end(keys.end()) ; e != e_end ; ++e)
                if ((*e)->type() != mkt_internal)
                    accept_visitor(i)(**e);
        }

        void visit(const MetadataValueKey<std::string> & k)
        {
            cout << std::setw(30) << (indent + k.human_name() + ":") << " " << k.value() << endl;
        }

        void visit(const MetadataValueKey<SlotName> & k)
        {
            cout << std::setw(30) << (indent + k.human_name() + ":") << " " << k.value() << endl;
        }

        void visit(const MetadataValueKey<long> & k)
        {
            cout << std::setw(30) << (indent + k.human_name() + ":") << " " << k.value() << endl;
        }

        void visit(const MetadataValueKey<bool> & k)
        {
            cout << std::setw(30) << (indent + k.human_name() + ":") << " " << k.value() << endl;
        }

        void visit(const MetadataValueKey<FSEntry> & k)
        {
            cout << std::setw(30) << (indent + k.human_name() + ":") << " " << k.value() << endl;
        }

        void visit(const MetadataValueKey<std::tr1::shared_ptr<const PackageID> > & k)
        {
            cout << std::setw(30) << (indent + k.human_name() + ":") << " " << *k.value() << endl;
        }

        void visit(const MetadataValueKey<std::tr1::shared_ptr<const RepositoryMaskInfo> >  & k)
        {
            cout << std::setw(30) << (indent + k.human_name() + ":") << " " << endl;
        }

        void visit(const MetadataValueKey<std::tr1::shared_ptr<const Contents> > & k)
        {
            cout << std::setw(30) << (indent + k.human_name() + ":") << " " << endl;
        }

        void visit(const MetadataTimeKey & k)
        {
            cout << std::setw(30) << (indent + k.human_name() + ":") << " "
                << pretty_print_time(k.value().seconds()) << endl;
        }

        void visit(const MetadataSpecTreeKey<PlainTextSpecTree> & k)
        {
            ColourFormatter f;
            cout << std::setw(30) << (indent + k.human_name() + ":") << " " << k.pretty_print_flat(f) << endl;
        }

        void visit(const MetadataSpecTreeKey<LicenseSpecTree> & k)
        {
            ColourFormatter f;
            cout << std::setw(30) << (indent + k.human_name() + ":") << " " << k.pretty_print_flat(f) << endl;
        }

        void visit(const MetadataSpecTreeKey<FetchableURISpecTree> & k)
        {
            ColourFormatter f;
            cout << std::setw(30) << (indent + k.human_name() + ":") << " " << k.pretty_print_flat(f) << endl;
        }

        void visit(const MetadataSpecTreeKey<SimpleURISpecTree> & k)
        {
            ColourFormatter f;
            cout << std::setw(30) << (indent + k.human_name() + ":") << " " << k.pretty_print_flat(f) << endl;
        }

        void visit(const MetadataSpecTreeKey<DependencySpecTree> & k)
        {
            ColourFormatter f;
            cout << std::setw(30) << (indent + k.human_name() + ":") << " " << k.pretty_print_flat(f) << endl;
        }

        void visit(const MetadataSpecTreeKey<ProvideSpecTree> & k)
        {
            ColourFormatter f;
            cout << std::setw(30) << (indent + k.human_name() + ":") << " " << k.pretty_print_flat(f) << endl;
        }

        void visit(const MetadataCollectionKey<FSEntrySequence> & k)
        {
            ColourFormatter f;
            cout << std::setw(30) << (indent + k.human_name() + ":") << " " << k.pretty_print_flat(f) << endl;
        }

        void visit(const MetadataCollectionKey<PackageIDSequence> & k)
        {
            ColourFormatter f;
            cout << std::setw(30) << (indent + k.human_name() + ":") << " " << k.pretty_print_flat(f) << endl;
        }

        void visit(const MetadataCollectionKey<KeywordNameSet> & k)
        {
            ColourFormatter f;
            cout << std::setw(30) << (indent + k.human_name() + ":") << " " << k.pretty_print_flat(f) << endl;
        }

        void visit(const MetadataCollectionKey<Set<std::string> > & k)
        {
            ColourFormatter f;
            cout << std::setw(30) << (indent + k.human_name() + ":") << " " << k.pretty_print_flat(f) << endl;
        }

        void visit(const MetadataCollectionKey<Sequence<std::string> > & k)
        {
            ColourFormatter f;
            cout << std::setw(30) << (indent + k.human_name() + ":") << " " << k.pretty_print_flat(f) << endl;
        }

        void visit(const MetadataValueKey<std::tr1::shared_ptr<const Choices> > & k)
        {
            cout << std::setw(30) << (indent + k.human_name() + ":") << " " << endl;
        }
    };
}

int do_one_info(
        const std::tr1::shared_ptr<const Environment> & env,
        const std::string & q)
{
    Context local_context("When handling query '" + q + "':");

    std::tr1::shared_ptr<PackageDepSpec> spec(
            new PackageDepSpec(parse_user_package_dep_spec(q, env.get(), UserPackageDepSpecOptions())));

    std::tr1::shared_ptr<const PackageIDSequence>
        entries((*env)[selection::AllVersionsSorted(generator::Matches(*spec, MatchPackageOptions()))]),
        installed_entries((*env)[selection::AllVersionsSorted(
                    generator::Matches(*spec, MatchPackageOptions()) | filter::InstalledAtRoot(env->root()))]),
        installable_entries((*env)[selection::AllVersionsSorted(
                    generator::Matches(*spec, MatchPackageOptions()) | filter::SupportsAction<InstallAction>() | filter::NotMasked())]);

    std::tr1::shared_ptr<PackageIDSequence> to_show_entries(new PackageIDSequence);

    if (entries->empty())
        throw NoSuchPackageError(q);

    if (! installed_entries->empty())
        std::copy(installed_entries->begin(), installed_entries->end(), to_show_entries->back_inserter());

    if (! installable_entries->empty())
        to_show_entries->push_back(*installable_entries->last());

    if (to_show_entries->empty())
        to_show_entries->push_back(*entries->last());

    for (PackageIDSequence::ConstIterator p(to_show_entries->begin()), p_end(to_show_entries->end()) ;
            p != p_end ; ++p)
    {
        OutputManagerFromEnvironment output_manager_holder(env.get(), *p, oe_exclusive, ClientOutputFeatures());
        InfoActionOptions options(make_named_values<InfoActionOptions>(
                    value_for<n::make_output_manager>(std::tr1::ref(output_manager_holder))
                    ));
        InfoAction a(options);

        try
        {
            cout << "Package " << colour(cl_package_name, **p) << ":" << endl;
            cout << endl;
            (*p)->perform_action(a);
            cout << endl;

            if (output_manager_holder.output_manager_if_constructed())
                output_manager_holder.output_manager_if_constructed()->succeeded();
        }
        catch (const ActionFailedError &)
        {
            cout << "        No extra information available for '" << **p << "'" << endl;
            cout << endl;
        }
    }

    return 0;
}

int
do_info(const std::tr1::shared_ptr<const Environment> & env)
{
    int return_code(0);

    cout << "Paludis build information:" << endl;

    cout << "    " << colour(cl_heading, "Compiler:") << endl;
    cout << "        " << std::setw(22) << std::left << ("CXX:") << std::setw(0) << " " << PALUDIS_BUILD_CXX
#if defined(__ICC)
        << " " << __ICC
#elif defined(__VERSION__)
        << " " << __VERSION__
#endif
        << endl;

    cout << "        " << std::setw(22) << std::left << ("CXXFLAGS:") << std::setw(0) << " " << PALUDIS_BUILD_CXXFLAGS << endl;
    cout << "        " << std::setw(22) << std::left << ("LDFLAGS:") << std::setw(0) << " " << PALUDIS_BUILD_LDFLAGS << endl;
    cout << "        " << std::setw(22) << std::left << ("DATE:") << std::setw(0) << " " << PALUDIS_BUILD_DATE << endl;

    cout << endl;

    cout << "    " << colour(cl_heading, "Libraries:") << endl;
    cout << "        " << std::setw(22) << std::left << ("C++ Library:") << std::setw(0) << " "
#if defined(__GLIBCXX__)
#  define XSTRINGIFY(x) #x
#  define STRINGIFY(x) XSTRINGIFY(x)
        << "GNU libstdc++ " << STRINGIFY(__GLIBCXX__)
#endif
        << endl;
    cout << endl;

    cout << "    " << colour(cl_heading, "Paths:") << endl;

    cout << "        " << std::setw(22) << std::left << ("DATADIR:") << std::setw(0) << " " << DATADIR << endl;
    cout << "        " << std::setw(22) << std::left << ("LIBDIR:") << std::setw(0) << " " << LIBDIR << endl;
    cout << "        " << std::setw(22) << std::left << ("LIBEXECDIR:") << std::setw(0) << " " << LIBEXECDIR << endl;
    cout << "        " << std::setw(22) << std::left << ("SYSCONFDIR:") << std::setw(0) << " " << SYSCONFDIR << endl;
    cout << "        " << std::setw(22) << std::left << ("PYTHONINSTALLDIR:") << std::setw(0) << " " << PYTHONINSTALLDIR << endl;
    cout << "        " << std::setw(22) << std::left << ("RUBYINSTALLDIR:") << std::setw(0) << " " << RUBYINSTALLDIR << endl;

    cout << endl;

    cout << colour(cl_heading, "System:") << endl;
    cout << "    " << flush;
    int status(run_command(Command("uname -a")));
    if (0 != status)
        Log::get_instance()->message("info.uname.failure", ll_warning, lc_context)
            << "uname -a failed with status " << status;

    cout << endl;

    cout << colour(cl_heading, "Reduced Privs:") << endl;
    cout << "    " << std::setw(26) << std::left << "reduced_uid:" << std::setw(0) << " "
        << env->reduced_uid() << endl;
    const struct passwd * const p(getpwuid(env->reduced_uid()));
    cout << "    " << std::setw(26) << std::left << "reduced_uid->name:" << std::setw(0) << " "
        << (p ? p->pw_name : "???") << endl;
    cout << "    " << std::setw(26) << std::left << "reduced_uid->dir:" << std::setw(0) << " "
        << (p ? p->pw_dir : "???") << endl;
    cout << "    " << std::setw(26) << std::left << "reduced_gid:" << std::setw(0) << " "
        << env->reduced_gid() << endl;
    const struct group * const g(getgrgid(env->reduced_gid()));
    cout << "    " << std::setw(26) << std::left << "reduced_gid->name:" << std::setw(0) << " "
        << (g ? g->gr_name : "???") << endl;

    cout << endl;

    {
        cout << colour(cl_heading, "Environment:") << endl;
        std::set<std::tr1::shared_ptr<const MetadataKey>, MetadataKeyComparator> keys(
                env->begin_metadata(), env->end_metadata());
        InfoDisplayer i("    ");
        for (std::set<std::tr1::shared_ptr<const MetadataKey>, MetadataKeyComparator>::const_iterator
                k(keys.begin()), k_end(keys.end()) ; k != k_end ; ++k)
            if ((*k)->type() != mkt_internal)
                accept_visitor(i)(**k);
        cout << endl;
    }

    for (IndirectIterator<PackageDatabase::RepositoryConstIterator, const Repository>
            r(env->package_database()->begin_repositories()), r_end(env->package_database()->end_repositories()) ;
            r != r_end ; ++r)
    {
        cout << "Repository " << colour(cl_repository_name, r->name()) << ":" << endl;
        std::set<std::tr1::shared_ptr<const MetadataKey>, MetadataKeyComparator> keys(r->begin_metadata(), r->end_metadata());
        InfoDisplayer i("    ");
        for (std::set<std::tr1::shared_ptr<const MetadataKey>, MetadataKeyComparator>::const_iterator
                k(keys.begin()), k_end(keys.end()) ; k != k_end ; ++k)
            if ((*k)->type() != mkt_internal)
                accept_visitor(i)(**k);
        cout << endl;
    }

    if (CommandLine::get_instance()->empty())
    {
        cout << "No packages were specified on the command line, so detailed information is not" << endl;
        cout << "available (Paludis can display detailed information for both installed and" << endl;
        cout << "installable packages)." << endl;
        cout << endl;
        cout << colour(cl_bold_pink, "So if you're reporting a bug in cat/pkg, use '") <<
            "paludis --info cat/pkg" << colour(cl_bold_pink, "' instead.") << endl;
        cout << endl;
    }
    else
    {
        for (CommandLine::ParametersConstIterator q(CommandLine::get_instance()->begin_parameters()),
                q_end(CommandLine::get_instance()->end_parameters()) ;
                q != q_end ; ++q)
        {
            try
            {
                return_code |= do_one_info(env, *q);
            }
            catch (const AmbiguousPackageNameError & e)
            {
                cout << endl;
                cout << "Query error:" << endl;
                cout << "  * " << e.backtrace("\n  * ");
                cout << "Ambiguous package name '" << e.name() << "'. Did you mean:" << endl;
                for (AmbiguousPackageNameError::OptionsConstIterator o(e.begin_options()),
                        o_end(e.end_options()) ; o != o_end ; ++o)
                    cout << "    * " << colour(cl_package_name, *o) << endl;
                cout << endl;
                return_code |= 1;
            }
        }
    }

    return return_code;
}

