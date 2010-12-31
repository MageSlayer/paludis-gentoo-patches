/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010 Ciaran McCreesh
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

#include "applets.hh"
#include <src/output/colour.hh>
#include <functional>
#include <iomanip>
#include <iostream>
#include <paludis/paludis.hh>
#include <paludis/util/log.hh>
#include <paludis/util/system.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/timestamp.hh>
#include <paludis/util/process.hh>
#include <paludis/repository_factory.hh>
#include <string>
#include <set>
#include <map>

/** \file
 * Handle the --has-version, --best-version and various --list actions for the
 * main paludis program.
 */

using namespace paludis;

namespace
{
    struct ValuePrinter
    {
        int return_code;

        ValuePrinter() :
            return_code(0)
        {
        }

        void visit(const MetadataValueKey<std::string> & k)
        {
            std::cout << k.value() << std::endl;
        }

        void visit(const MetadataValueKey<SlotName> & k)
        {
            std::cout << k.value() << std::endl;
        }

        void visit(const MetadataValueKey<long> & k)
        {
            std::cout << k.value() << std::endl;
        }

        void visit(const MetadataValueKey<bool> & k)
        {
            std::cout << k.value() << std::endl;
        }

        void visit(const MetadataValueKey<FSPath> & k)
        {
            std::cout << k.value() << std::endl;
        }

        void visit(const MetadataValueKey<std::shared_ptr<const RepositoryMaskInfo> >  &)
        {
            std::cout << "(unprintable)" << std::endl;
            return_code |= 1;
        }

        void visit(const MetadataSectionKey &)
        {
            std::cout << "(unprintable)" << std::endl;
            return_code |= 1;
        }

        void visit(const MetadataValueKey<std::shared_ptr<const Contents> > &)
        {
            std::cout << "(unprintable)" << std::endl;
            return_code |= 1;
        }

        void visit(const MetadataValueKey<std::shared_ptr<const Choices> > &)
        {
            std::cout << "(unprintable)" << std::endl;
            return_code |= 1;
        }

        void visit(const MetadataSpecTreeKey<PlainTextSpecTree> & k)
        {
            std::cout << k.pretty_print_value(UnformattedPrettyPrinter(), { }) << std::endl;
        }

        void visit(const MetadataSpecTreeKey<RequiredUseSpecTree> & k)
        {
            std::cout << k.pretty_print_value(UnformattedPrettyPrinter(), { }) << std::endl;
        }

        void visit(const MetadataSpecTreeKey<ProvideSpecTree> & k)
        {
            std::cout << k.pretty_print_value(UnformattedPrettyPrinter(), { }) << std::endl;
        }

        void visit(const MetadataSpecTreeKey<FetchableURISpecTree> & k)
        {
            std::cout << k.pretty_print_value(UnformattedPrettyPrinter(), { }) << std::endl;
        }

        void visit(const MetadataSpecTreeKey<DependencySpecTree> & k)
        {
            std::cout << k.pretty_print_value(UnformattedPrettyPrinter(), { }) << std::endl;
        }

        void visit(const MetadataSpecTreeKey<SimpleURISpecTree> & k)
        {
            std::cout << k.pretty_print_value(UnformattedPrettyPrinter(), { }) << std::endl;
        }

        void visit(const MetadataSpecTreeKey<LicenseSpecTree> & k)
        {
            std::cout << k.pretty_print_value(UnformattedPrettyPrinter(), { }) << std::endl;
        }

        void visit(const MetadataCollectionKey<FSPathSequence> & k)
        {
            std::cout << k.pretty_print_value(UnformattedPrettyPrinter(), { }) << std::endl;
        }

        void visit(const MetadataCollectionKey<PackageIDSequence> & k)
        {
            std::cout << k.pretty_print_value(UnformattedPrettyPrinter(), { }) << std::endl;
        }

        void visit(const MetadataCollectionKey<KeywordNameSet> & k)
        {
            std::cout << k.pretty_print_value(UnformattedPrettyPrinter(), { }) << std::endl;
        }

        void visit(const MetadataCollectionKey<Set<std::string> > & k)
        {
            std::cout << k.pretty_print_value(UnformattedPrettyPrinter(), { }) << std::endl;
        }

        void visit(const MetadataCollectionKey<Map<std::string, std::string> > & k)
        {
            std::cout << k.pretty_print_value(UnformattedPrettyPrinter(), { }) << std::endl;
        }

        void visit(const MetadataCollectionKey<Sequence<std::string> > & k)
        {
            std::cout << k.pretty_print_value(UnformattedPrettyPrinter(), { }) << std::endl;
        }

        void visit(const MetadataValueKey<std::shared_ptr<const PackageID> > & k)
        {
            std::cout << *k.value() << std::endl;
        }

        void visit(const MetadataTimeKey & k)
        {
            std::cout << k.value().seconds() << std::endl;
        }
    };
}

int do_has_version(const std::shared_ptr<Environment> & env)
{
    int return_code(0);

    Context context("When performing has-version action from command line:");

    std::string query(*CommandLine::get_instance()->begin_parameters());
    std::shared_ptr<PackageDepSpec> spec(std::make_shared<PackageDepSpec>(
                parse_user_package_dep_spec(query, env.get(), { })));
    std::shared_ptr<const PackageIDSequence> entries((*env)[selection::SomeArbitraryVersion(
                generator::Matches(*spec, { }) | filter::InstalledAtRoot(env->preferred_root_key()->value()))]);

    if (entries->empty())
        return_code = 1;

    return return_code;
}

int do_best_version(const std::shared_ptr<Environment> & env)
{
    int return_code(0);

    Context context("When performing best-version action from command line:");

    std::string query(*CommandLine::get_instance()->begin_parameters());
    std::shared_ptr<PackageDepSpec> spec(std::make_shared<PackageDepSpec>(
                parse_user_package_dep_spec(query, env.get(), { })));
    std::shared_ptr<const PackageIDSequence> entries((*env)[selection::AllVersionsSorted(
                generator::Matches(*spec, { }) | filter::InstalledAtRoot(env->preferred_root_key()->value()))]);

    /* make built_with_use work for virtuals... icky... */
    while (! entries->empty())
    {
        if (! (*entries->last())->virtual_for_key())
            break;

        Log::get_instance()->message("paludis.best_version.is_virtual", ll_qa, lc_context) << "best-version of '" << query <<
            "' resolves to '" << **entries->last() << "', which is a virtual for '"
            << *(*entries->last())->virtual_for_key()->value() << "'. This will break with "
            "new style virtuals.";
        std::shared_ptr<PackageIDSequence> new_entries(std::make_shared<PackageIDSequence>());
        new_entries->push_back((*entries->last())->virtual_for_key()->value());
        entries = new_entries;
    }

    if (entries->empty())
        return_code = 1;
    else
    {
        // don't include repo, it breaks built_with_use and the like.
        std::string entry(
                stringify((*entries->last())->name()) + "-" +
                stringify((*entries->last())->version()));
        std::cout << entry << std::endl;
    }

    return return_code;
}

int do_match(const std::shared_ptr<Environment> & env)
{
    int return_code(0);

    Context context("When performing match action from command line:");

    std::string query(*CommandLine::get_instance()->begin_parameters());
    std::shared_ptr<PackageDepSpec> spec(std::make_shared<PackageDepSpec>(
                parse_user_package_dep_spec(query, env.get(), { })));
    std::shared_ptr<const PackageIDSequence> entries((*env)[selection::AllVersionsSorted(
                generator::Matches(*spec, { }) | filter::InstalledAtRoot(env->preferred_root_key()->value()))]);

    while (! entries->empty())
    {
        if (! (*entries->last())->virtual_for_key())
            break;

        Log::get_instance()->message("paludis.match.is_virtual", ll_qa, lc_context) << "match of '" << query <<
                "' resolves to '" << **entries->last() << "', which is a virtual for '"
                << *(*entries->last())->virtual_for_key()->value() << "'. This will break with "
                "new style virtuals.";
        std::shared_ptr<PackageIDSequence> new_entries(std::make_shared<PackageIDSequence>());
        new_entries->push_back((*entries->last())->virtual_for_key()->value());
        entries = new_entries;
    }

    if (entries->empty())
        return_code = 1;
    else
    {
        for (PackageIDSequence::ConstIterator i(entries->begin()), i_end(entries->end()) ; i != i_end ; ++i)
        {
            // don't include repo, it breaks built_with_use and the like.
            std::string entry(
                    stringify((*i)->name()) + "-" +
                    stringify((*i)->version()));
            std::cout << entry << std::endl;
        }
    }

    return return_code;
}

int do_environment_variable(const std::shared_ptr<Environment> & env)
{
    int return_code(0);

    Context context("When performing environment-variable action from command line:");

    std::string spec_str(*CommandLine::get_instance()->begin_parameters());
    std::string var_str(* next(CommandLine::get_instance()->begin_parameters()));
    std::shared_ptr<PackageDepSpec> spec(std::make_shared<PackageDepSpec>(
                parse_user_package_dep_spec(spec_str, env.get(), { })));

    std::shared_ptr<const PackageIDSequence> entries((*env)[selection::AllVersionsSorted(
                generator::Matches(*spec, { }) | filter::InstalledAtRoot(env->preferred_root_key()->value()))]);

    if (entries->empty())
        entries = (*env)[selection::AllVersionsSorted(generator::Matches(*spec, { }))];

    if (entries->empty())
        throw NoSuchPackageError(spec_str);

    auto repo(env->package_database()->fetch_repository((*entries->last())->repository_name()));
    RepositoryEnvironmentVariableInterface * env_if(repo->environment_variable_interface());

    if (! env_if)
    {
        std::cerr << "Repository '" << (*entries->last())->repository_name() <<
            "' cannot be queried for environment variables" << std::endl;
        return_code |= 1;
    }
    else
        std::cout << env_if->get_environment_variable(*entries->last(), var_str) << std::endl;

    return return_code;
}

int do_configuration_variable(const std::shared_ptr<Environment> & env)
{
    Context context("When performing configuration-variable action from command line:");

    std::string repo_str(*CommandLine::get_instance()->begin_parameters());
    std::string var_str(* next(CommandLine::get_instance()->begin_parameters()));

    const std::shared_ptr<const Repository> repo(env->package_database()->fetch_repository(RepositoryName(repo_str)));
    Repository::MetadataConstIterator i(repo->find_metadata(var_str));
    if (i == repo->end_metadata())
        return 1;
    else
    {
        ValuePrinter v;
        (*i)->accept(v);
        return v.return_code;
    }
}

int do_list_repository_formats()
{
    int return_code(1);

    std::set<std::string> keys(RepositoryFactory::get_instance()->begin_keys(), RepositoryFactory::get_instance()->end_keys());

    if (! keys.empty())
    {
        return_code = 0;
        for (std::set<std::string>::const_iterator k(keys.begin()), k_end(keys.end()) ;
                k != k_end ; ++k)
            std::cout << "* " << colour(cl_key_name, *k) << std::endl;
    }

    return return_code;
}

int do_list_sync_protocols(const std::shared_ptr<Environment> & env)
{
    std::map<std::string, std::string> syncers;

    std::shared_ptr<const FSPathSequence> sd(env->syncers_dirs());
    for (FSPathSequence::ConstIterator d(sd->begin()),
            d_end(sd->end()) ; d != d_end ; ++d)
    {
        FSPath dir(*d);
        if (! dir.stat().is_directory())
            continue;

        for (FSIterator f(dir, { }), f_end; f != f_end; ++f)
        {
            std::string name(f->basename());
            if ((0 != (f->stat().permissions() & S_IXUSR)) &&
                    0 == name.compare(0, 2, "do", 0, 2))
            {
                name.erase(0, 2);
                if (syncers.find(name) == syncers.end())
                    syncers[name] = stringify(*f);
            }
        }
    }

    int return_code(1);

    if (! syncers.empty())
    {
        return_code = 0;
        for (std::map<std::string, std::string>::const_iterator s(syncers.begin()), s_end(syncers.end()) ;
                s != s_end ; ++s)
        {
            std::cout << "* " << colour(cl_key_name, s->first) << std::endl;
            Process process((ProcessCommand(s->second + " --help")));
            process
                .setenv("PALUDIS_FETCHERS_DIRS", join(sd->begin(), sd->end(), " "))
                .setenv("PALUDIS_EBUILD_DIR", getenv_with_default("PALUDIS_EBUILD_DIR", LIBEXECDIR "/paludis"));
            if (0 != process.run().wait())
                Log::get_instance()->message("paludis.syncer_help.failure", ll_warning, lc_context)
                    << "Syncer help command '" << s->second << " --help' failed";
            std::cout << std::endl;
        }
    }

    return return_code;
}

int do_regenerate_cache(const std::shared_ptr<Environment> & env, bool installed)
{
    Context context("When performing cache regeneration action from command line:");

    if (! CommandLine::get_instance()->empty())
    {
        CommandLine::ParametersConstIterator q(CommandLine::get_instance()->begin_parameters()),
            q_end(CommandLine::get_instance()->end_parameters());
        for ( ; q != q_end ; ++q)
        {
            if (! env->package_database()->has_repository_named(RepositoryName(*q)))
                throw NoSuchRepositoryError(RepositoryName(*q));

            std::cout << "Regenerating cache for " << (*q) << "..." << std::endl;
            env->package_database()->fetch_repository(RepositoryName(*q))->regenerate_cache();
        }
    }
    else
    {
        for (PackageDatabase::RepositoryConstIterator r(env->package_database()->begin_repositories()),
                r_end(env->package_database()->end_repositories()) ; r != r_end ; ++r)
        {
            if (installed)
            {
                if (! (*r)->installed_root_key())
                    continue;
            }
            else
            {
                SupportsActionTest<InstallAction> action_test;
                if (! (*r)->some_ids_might_support_action(action_test))
                    continue;
            }

            std::cout << "Regenerating cache for " << (*r)->name() << "..." << std::endl;
            (*r)->regenerate_cache();
        }
    }

    return 0;
}

