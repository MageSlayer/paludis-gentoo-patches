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

#include <paludis/repositories/e/pipe_command_handler.hh>
#include <paludis/repositories/e/eapi.hh>
#include <paludis/repositories/e/fix_locked_dependencies.hh>
#include <paludis/repositories/e/dep_parser.hh>
#include <paludis/repositories/e/dep_spec_pretty_printer.hh>
#include <paludis/util/log.hh>
#include <paludis/util/join.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/destringify.hh>
#include <paludis/util/iterator_funcs.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/visitor_cast.hh>
#include <paludis/util/set.hh>
#include <paludis/package_id.hh>
#include <paludis/environment.hh>
#include <paludis/package_database.hh>
#include <paludis/metadata_key.hh>
#include <paludis/stringify_formatter.hh>
#include <paludis/selection.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <vector>

using namespace paludis;

namespace
{
    std::string name_and_version(const PackageID & id)
    {
        return stringify(id.name()) + "-" + stringify(id.version());
    }
}

std::string
paludis::erepository::pipe_command_handler(const Environment * const environment,
        const std::tr1::shared_ptr<const PackageID> & package_id, const std::string & s)
{
    Context context("In ebuild pipe command handler for '" + s + "':");

    try
    {
        std::vector<std::string> tokens;
        tokenise_whitespace(s, std::back_inserter(tokens));
        if (tokens.empty())
        {
            Log::get_instance()->message("e.pipe_commands.empty", ll_warning, lc_context) << "Got empty pipe command";
            return "Eempty pipe command";
        }

        if (tokens[0] == "PING")
        {
            if (tokens.size() != 3)
            {
                Log::get_instance()->message("e.pipe_commands.ping.bad", ll_warning, lc_context) << "Got bad PING command";
                return "Ebad PING command";
            }
            else
                return "OPONG " + tokens[2];
        }
        else if (tokens[0] == "LOG")
        {
            if (tokens.size() < 4)
            {
                Log::get_instance()->message("e.pipe_commands.log.bad", ll_warning, lc_context) << "Got too short LOG pipe command";
                return "Ebad LOG command";
            }
            else
            {
                Log::get_instance()->message("e.child.message", destringify<LogLevel>(tokens[2]), lc_context)
                    << join(next(next(next(tokens.begin()))), tokens.end(), " ");
                return "O";
            }
        }
        else if (tokens[0] == "BEST_VERSION")
        {
            if (tokens.size() != 3)
            {
                Log::get_instance()->message("e.pipe_commands.best_version.bad", ll_warning, lc_context) << "Got bad BEST_VERSION pipe command";
                return "Ebad BEST_VERSION command";
            }
            else
            {
                std::tr1::shared_ptr<const EAPI> eapi(EAPIData::get_instance()->eapi_from_string(tokens[1]));
                if (! eapi->supported())
                    return "EBEST_VERSION EAPI " + tokens[1] + " unsupported";

                PackageDepSpec spec(parse_elike_package_dep_spec(tokens[2], eapi->supported()->package_dep_spec_parse_options(), package_id));
                std::tr1::shared_ptr<const PackageIDSequence> entries((*environment)[selection::AllVersionsSorted(
                            generator::Matches(spec) | filter::InstalledAtRoot(environment->root()))]);
                if (eapi->supported()->pipe_commands()->rewrite_virtuals() && (! entries->empty()) &&
                        (*entries->last())->virtual_for_key())
                {
                    Log::get_instance()->message("e.pipe_commands.best_version.is_virtual", ll_qa, lc_context) << "best-version of '" << spec <<
                        "' resolves to '" << **entries->last() << "', which is a virtual for '"
                        << *(*entries->last())->virtual_for_key()->value() << "'. This will break with "
                        "new style virtuals.";
                    std::tr1::shared_ptr<PackageIDSequence> new_entries(new PackageIDSequence);
                    new_entries->push_back((*entries->last())->virtual_for_key()->value());
                    entries = new_entries;
                }

                if (entries->empty())
                    return "O1;";
                else
                {
                    if (eapi->supported()->pipe_commands()->no_slot_or_repo())
                        return "O0;" + name_and_version(**entries->last());
                    else
                        return "O0;" + stringify(**entries->last());
                }
            }
        }
        else if (tokens[0] == "HAS_VERSION")
        {
            if (tokens.size() != 3)
            {
                Log::get_instance()->message("e.pipe_commands.has_version.bad", ll_warning, lc_context) << "Got bad HAS_VERSION pipe command";
                return "Ebad HAS_VERSION command";
            }
            else
            {
                std::tr1::shared_ptr<const EAPI> eapi(EAPIData::get_instance()->eapi_from_string(tokens[1]));
                if (! eapi->supported())
                    return "EHAS_VERSION EAPI " + tokens[1] + " unsupported";

                PackageDepSpec spec(parse_elike_package_dep_spec(tokens[2], eapi->supported()->package_dep_spec_parse_options(), package_id));
                std::tr1::shared_ptr<const PackageIDSequence> entries((*environment)[selection::SomeArbitraryVersion(
                            generator::Matches(spec) | filter::InstalledAtRoot(environment->root()))]);
                if (entries->empty())
                    return "O1;";
                else
                    return "O0;";
            }
        }
        else if (tokens[0] == "MATCH")
        {
            if (tokens.size() != 3)
            {
                Log::get_instance()->message("e.pipe_commands.match.bad", ll_warning, lc_context) << "Got bad MATCH pipe command";
                return "Ebad MATCH command";
            }
            else
            {
                std::tr1::shared_ptr<const EAPI> eapi(EAPIData::get_instance()->eapi_from_string(tokens[1]));
                if (! eapi->supported())
                    return "EMATCH EAPI " + tokens[1] + " unsupported";

                PackageDepSpec spec(parse_elike_package_dep_spec(tokens[2], eapi->supported()->package_dep_spec_parse_options(), package_id));
                std::tr1::shared_ptr<const PackageIDSequence> entries((*environment)[selection::AllVersionsSorted(
                            generator::Matches(spec) | filter::InstalledAtRoot(environment->root()))]);
                if (eapi->supported()->pipe_commands()->rewrite_virtuals() && (! entries->empty()))
                {
                    std::tr1::shared_ptr<PackageIDSequence> new_entries(new PackageIDSequence);
                    for (PackageIDSequence::ConstIterator i(entries->begin()), i_end(entries->end()) ;
                            i != i_end ; ++i)
                    {
                        if ((*i)->virtual_for_key())
                        {
                            Log::get_instance()->message("e.pipe_commands.match.is_virtual", ll_qa, lc_context) << "match of '" << spec <<
                                "' resolves to '" << **i << "', which is a virtual for '"
                                << *(*i)->virtual_for_key()->value() << "'. This will break with "
                                "new style virtuals.";
                            new_entries->push_back((*i)->virtual_for_key()->value());
                        }
                        else
                            new_entries->push_back(*i);
                    }
                    entries = new_entries;
                }

                if (entries->empty())
                    return "O1;";
                else
                {
                    if (eapi->supported()->pipe_commands()->no_slot_or_repo())
                        return "O0;" + join(indirect_iterator(entries->begin()), indirect_iterator(entries->end()), "\n", &name_and_version);
                    else
                        return "O0;" + join(indirect_iterator(entries->begin()), indirect_iterator(entries->end()), "\n");
                }
            }
        }
        else if (tokens[0] == "VDB_PATH")
        {
            if (tokens.size() != 2)
            {
                Log::get_instance()->message("e.pipe_commands.vdb_path.bad", ll_warning, lc_context) << "Got bad VDB_PATH pipe command";
                return "Ebad VDB_PATH command";
            }
            else
            {
                if (! environment->package_database()->has_repository_named(RepositoryName("installed")))
                    return "Eno installed repository available";
                std::tr1::shared_ptr<const Repository> repo(environment->package_database()->fetch_repository(RepositoryName("installed")));
                Repository::MetadataConstIterator key(repo->find_metadata("location"));
                if (repo->end_metadata() == key)
                    return "Einstalled repository has no location key";
                if (! visitor_cast<const MetadataValueKey<FSEntry> >(**key))
                    return "Einstalled repository location key is not a MetadataValueKey<FSEntry> ";
                return "O0;" + stringify(visitor_cast<const MetadataValueKey<FSEntry> >(**key)->value());
            }
        }
        else if (tokens[0] == "REWRITE_VAR")
        {
            if (tokens.size() < 4)
            {
                Log::get_instance()->message("e.pipe_commands.rewrite_var.bad", ll_warning, lc_context) << "Got bad REWRITE_VAR pipe command";
                return "Ebad REWRITE_VAR command";
            }

            std::tr1::shared_ptr<const EAPI> eapi(EAPIData::get_instance()->eapi_from_string(tokens[1]));
            if (! eapi->supported())
                return "EREWRITE_VAR EAPI " + tokens[1] + " unsupported";

            std::string var(tokens[3]);

            if ((var == eapi->supported()->ebuild_metadata_variables()->build_depend().name()) ||
                    (var == eapi->supported()->ebuild_metadata_variables()->run_depend().name()) ||
                    (var == eapi->supported()->ebuild_metadata_variables()->pdepend().name()) ||
                    (var == eapi->supported()->ebuild_metadata_variables()->dependencies().name()))
            {
                std::tr1::shared_ptr<const DependencySpecTree::ConstItem> before(parse_depend(join(tokens.begin() + 4, tokens.end(), " "),
                            environment, package_id, *eapi));
                std::tr1::shared_ptr<const DependencySpecTree::ConstItem> after(fix_locked_dependencies(
                            environment, *eapi, package_id, before));
                StringifyFormatter ff;
                DepSpecPrettyPrinter p(0, std::tr1::shared_ptr<const PackageID>(), ff, 0, false);
                after->accept(p);
                return "O0;" + stringify(p);
            }

            return "O0;" + join(tokens.begin() + 4, tokens.end(), " ");
        }
        else if (tokens[0] == "EVER")
        {
            if (tokens.size() < 3)
            {
                Log::get_instance()->message("e.pipe_commands.ever.bad", ll_warning, lc_context) << "Got bad EVER pipe command";
                return "Ebad EVER command";
            }

            if (tokens[2] == "AT_LEAST")
            {
                if (tokens.size() != 5)
                {
                    Log::get_instance()->message("e.pipe_commands.ever.at_least.bad", ll_warning, lc_context) << "Got bad EVER AT_LEAST pipe command";
                    return "Ebad EVER AT_LEAST command {'" + join(tokens.begin(), tokens.end(), "', '") + "'}";
                }

                VersionSpec v1(tokens[3]), v2(tokens[4]);
                return v2 >= v1 ? "O0;" : "O1;";
            }
            else
            {
                Log::get_instance()->message("e.pipe_commands.ever.unknown", ll_warning, lc_context) << "Got unknown EVER pipe subcommand";
                return "Ebad EVER subcommand";
            }
        }
        else
        {
            Log::get_instance()->message("e.pipe_commands.unknown", ll_warning, lc_context) << "Got unknown ebuild pipe command '" + s + "'";
            return "Eunknown pipe command";
        }
    }
    catch (const Exception & e)
    {
        return "Eexception '" + e.message() + "' (" + e.what() + ")";
    }
    catch (const std::exception & e)
    {
        return "Eexception " + stringify(e.what());
    }
    catch (...)
    {
        return "Eexception ???";
    }
}


