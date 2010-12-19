/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010 Ciaran McCreesh
 * Copyright (c) 2009 Ingmar Vanhassel
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
#include <paludis/util/simple_visitor_cast.hh>
#include <paludis/util/set.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/accept_visitor.hh>
#include <paludis/util/save.hh>
#include <paludis/output_manager.hh>
#include <paludis/package_id.hh>
#include <paludis/environment.hh>
#include <paludis/package_database.hh>
#include <paludis/metadata_key.hh>
#include <paludis/stringify_formatter.hh>
#include <paludis/selection.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/choice.hh>
#include <paludis/dep_spec_annotations.hh>
#include <vector>
#include <limits>
#include <sstream>
#include <algorithm>

using namespace paludis;

namespace
{
    std::string name_and_version(const PackageID & id)
    {
        return stringify(id.name()) + "-" + stringify(id.version());
    }

    struct MyOptionsRewriter
    {
        StringifyFormatter f;
        std::stringstream str;

        std::string prefix;

        const std::shared_ptr<const PackageID> id;
        const std::string description_annotation;
        const std::string joiner;

        MyOptionsRewriter(
                const std::shared_ptr<const PackageID> & i,
                const std::string & d,
                const std::string & j) :
            id(i),
            description_annotation(d),
            joiner(j)
        {
        }

        void visit(const PlainTextSpecTree::NodeType<AllDepSpec>::Type & node)
        {
            Save<std::string> save_prefix(&prefix);
            str << "( ";
            std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
            str << " ) ";
            do_annotations(*node.spec(), "");
        }

        void visit(const PlainTextSpecTree::NodeType<ConditionalDepSpec>::Type & node)
        {
            Save<std::string> save_prefix(&prefix);
            str << f.format(*node.spec(), format::Plain()) << " ( ";
            std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
            str << " ) ";
            do_annotations(*node.spec(), "");
        }

        void visit(const PlainTextSpecTree::NodeType<PlainTextDepSpec>::Type & node)
        {
            str << f.format(*node.spec(), format::Plain()) << " ";
            do_annotations(*node.spec(), (prefix.empty() ? "" : prefix + joiner) + stringify(*node.spec()));
        }

        void visit(const PlainTextSpecTree::NodeType<PlainTextLabelDepSpec>::Type & node)
        {
            prefix = node.spec()->label();
            str << f.format(*node.spec(), format::Plain()) << " ";
            do_annotations(*node.spec(), "");
        }

        void do_annotations(const DepSpec & p, const std::string & desc_from)
        {
            bool seen_description(false), done_brackets(false);

            if (p.maybe_annotations())
                for (auto m(p.maybe_annotations()->begin()), m_end(p.maybe_annotations()->end()) ;
                        m != m_end ; ++m)
                {
                    if (! done_brackets)
                    {
                        str << " [[ ";
                        done_brackets = true;
                    }

                    str << m->key() << " = [" << (m->value().empty() ? " " : " " + m->value() + " ") << "] ";

                    if (m->key() == description_annotation)
                        seen_description = true;
                }

            if ((! seen_description) && (id->choices_key()) && (! desc_from.empty()))
            {
                const std::shared_ptr<const ChoiceValue> choice(
                        id->choices_key()->value()->find_by_name_with_prefix(ChoiceNameWithPrefix(desc_from)));
                if (choice && (! choice->description().empty()) && (! description_annotation.empty()))
                {
                    if (! done_brackets)
                    {
                        str << " [[ ";
                        done_brackets = true;
                    }

                    str << description_annotation << " = [ " << choice->description() << " ] ";
                }
            }

            if (done_brackets)
                str << "]] ";
        }
    };
}

std::string
paludis::erepository::pipe_command_handler(const Environment * const environment,
        const std::shared_ptr<const PackageID> & package_id, bool in_metadata_generation,
        const std::string & s, const std::shared_ptr<OutputManager> & maybe_output_manager)
{
    Context context("In ebuild pipe command handler:");

    try
    {
        std::vector<std::string> tokens;
        if (std::string::npos == s.find('\2'))
            tokenise_whitespace(s, std::back_inserter(tokens));
        else
        {
            std::string t(s);
            std::string::size_type p(t.find('\2'));
            while (std::string::npos != p)
            {
                tokens.push_back(t.substr(0, p));
                t.erase(0, p + 1);
                p = t.find('\2');
            }
        }

        if (tokens.empty())
        {
            Log::get_instance()->message("e.pipe_commands.empty", ll_warning, lc_context) << "Got empty pipe command";
            return "Eempty pipe command";
        }

        if (tokens[0] == "PING")
        {
            if (tokens.size() != 3)
            {
                Log::get_instance()->message("e.pipe_commands.ping.bad", ll_warning, lc_context) << "Got bad PING command, tokens are { '"
                    << join(tokens.begin(), tokens.end(), "', '") << "' }";
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
                if (tokens[2] == "status")
                {
                    if (maybe_output_manager)
                        maybe_output_manager->message(mt_status, join(next(next(next(tokens.begin()))), tokens.end(), " "));
                    return "O0;";
                }
                else
                {
                    Log::get_instance()->message("e.child.message", destringify<LogLevel>(tokens[2]), lc_context)
                        << join(next(next(next(tokens.begin()))), tokens.end(), " ");
                    return "O0;";
                }
            }
        }
        else if (tokens[0] == "MESSAGE")
        {
            if (tokens.size() == 3)
            {
                /* don't barf on empty messages */
                tokens.push_back(" ");
            }

            if (tokens.size() < 4)
            {
                Log::get_instance()->message("e.pipe_commands.message.bad", ll_warning, lc_context) << "Got bad MESSAGE pipe command";
                return "Ebad MESSAGE command";
            }
            else
            {
                MessageType m;
                if (tokens[2] == "einfo" || tokens[2] == "einfon" || tokens[2] == "ebegin")
                    m = mt_info;
                else if (tokens[2] == "ewarn")
                    m = mt_warn;
                else if (tokens[2] == "eerror")
                    m = mt_error;
                else if (tokens[2] == "elog")
                    m = mt_log;
                else
                    return "EUnknown message type " + tokens[2] + "";

                if (maybe_output_manager)
                    maybe_output_manager->message(m, join(next(next(next(tokens.begin()))), tokens.end(), " "));
                return "O0;";
            }
        }
        else if (tokens[0] == "BEST_VERSION")
        {
            if (tokens.size() != 4)
            {
                Log::get_instance()->message("e.pipe_commands.best_version.bad", ll_warning, lc_context) << "Got bad BEST_VERSION pipe command";
                return "Ebad BEST_VERSION command";
            }
            else
            {
                std::shared_ptr<const EAPI> eapi(EAPIData::get_instance()->eapi_from_string(tokens[1]));
                if (! eapi->supported())
                    return "EBEST_VERSION EAPI " + tokens[1] + " unsupported";

                Filter root((filter::All()));
                if (tokens[2] == "--slash")
                    root = filter::InstalledAtRoot(environment->system_root_key()->value());
                else if (tokens[2] == "--root")
                    root = filter::InstalledAtRoot(environment->preferred_root_key()->value());
                else
                    return "Ebad BEST_VERSION " + tokens[2] + " argument";

                PackageDepSpec spec(parse_elike_package_dep_spec(tokens[3],
                            eapi->supported()->package_dep_spec_parse_options(),
                            eapi->supported()->version_spec_options(), package_id));
                std::shared_ptr<const PackageIDSequence> entries((*environment)[selection::AllVersionsSorted(
                            generator::Matches(spec, { }) | root)]);
                if (eapi->supported()->pipe_commands()->rewrite_virtuals() && (! entries->empty()) &&
                        (*entries->last())->virtual_for_key())
                {
                    Log::get_instance()->message("e.pipe_commands.best_version.is_virtual", ll_qa, lc_context) << "best-version of '" << spec <<
                        "' resolves to '" << **entries->last() << "', which is a virtual for '"
                        << *(*entries->last())->virtual_for_key()->value() << "'. This will break with "
                        "new style virtuals.";
                    std::shared_ptr<PackageIDSequence> new_entries(std::make_shared<PackageIDSequence>());
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
            if (tokens.size() != 4)
            {
                Log::get_instance()->message("e.pipe_commands.has_version.bad", ll_warning, lc_context) << "Got bad HAS_VERSION pipe command";
                return "Ebad HAS_VERSION command";
            }
            else
            {
                std::shared_ptr<const EAPI> eapi(EAPIData::get_instance()->eapi_from_string(tokens[1]));
                if (! eapi->supported())
                    return "EHAS_VERSION EAPI " + tokens[1] + " unsupported";

                Filter root((filter::All()));
                if (tokens[2] == "--slash")
                    root = filter::InstalledAtRoot(environment->system_root_key()->value());
                else if (tokens[2] == "--root")
                    root = filter::InstalledAtRoot(environment->preferred_root_key()->value());
                else
                    return "Ebad HAS_VERSION " + tokens[2] + " argument";

                PackageDepSpec spec(parse_elike_package_dep_spec(tokens[3],
                            eapi->supported()->package_dep_spec_parse_options(),
                            eapi->supported()->version_spec_options(), package_id));
                std::shared_ptr<const PackageIDSequence> entries((*environment)[selection::SomeArbitraryVersion(
                            generator::Matches(spec, { }) | root)]);
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
                std::shared_ptr<const EAPI> eapi(EAPIData::get_instance()->eapi_from_string(tokens[1]));
                if (! eapi->supported())
                    return "EMATCH EAPI " + tokens[1] + " unsupported";

                PackageDepSpec spec(parse_elike_package_dep_spec(tokens[2],
                            eapi->supported()->package_dep_spec_parse_options(),
                            eapi->supported()->version_spec_options(), package_id));
                std::shared_ptr<const PackageIDSequence> entries((*environment)[selection::AllVersionsSorted(
                            generator::Matches(spec, { }) | filter::InstalledAtRoot(environment->preferred_root_key()->value()))]);
                if (eapi->supported()->pipe_commands()->rewrite_virtuals() && (! entries->empty()))
                {
                    std::shared_ptr<PackageIDSequence> new_entries(std::make_shared<PackageIDSequence>());
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
                std::shared_ptr<const Repository> repo(environment->package_database()->fetch_repository(RepositoryName("installed")));
                Repository::MetadataConstIterator key(repo->find_metadata("location"));
                if (repo->end_metadata() == key)
                    return "Einstalled repository has no location key";
                if (! simple_visitor_cast<const MetadataValueKey<FSPath> >(**key))
                    return "Einstalled repository location key is not a MetadataValueKey<FSPath> ";
                return "O0;" + stringify(simple_visitor_cast<const MetadataValueKey<FSPath> >(**key)->value());
            }
        }
        else if (tokens[0] == "OPTIONQ")
        {
            if (tokens.size() != 3)
            {
                Log::get_instance()->message("e.pipe_commands.has_version.bad", ll_warning, lc_context) << "Got bad OPTIONQ pipe command";
                return "Ebad OPTIONQ command";
            }
            else
            {
                std::shared_ptr<const EAPI> eapi(EAPIData::get_instance()->eapi_from_string(tokens[1]));
                if (! eapi->supported())
                    return "EOPTIONQ EAPI " + tokens[1] + " unsupported";

                if (in_metadata_generation)
                    return "Ecannot query options during metadata generation";

                if (! package_id->choices_key())
                    return "EOPTIONQ ID " + stringify(*package_id) + " has no choices";

                ChoiceNameWithPrefix name(tokens[2]);
                std::shared_ptr<const ChoiceValue> value(package_id->choices_key()->value()->find_by_name_with_prefix(name));
                if (! value)
                {
                    if (package_id->choices_key()->value()->has_matching_contains_every_value_prefix(name))
                        return "O1;";

                    return "EOPTIONQ ID " + stringify(*package_id) + " has no choice named '" + stringify(name) + "'";
                }

                if (value->enabled())
                    return "O0;";
                else
                    return "O1;";
            }
        }
        else if (tokens[0] == "REWRITE_VAR")
        {
            if (tokens.size() < 4)
            {
                Log::get_instance()->message("e.pipe_commands.rewrite_var.bad", ll_warning, lc_context) << "Got bad REWRITE_VAR pipe command";
                return "Ebad REWRITE_VAR command";
            }

            std::shared_ptr<const EAPI> eapi(EAPIData::get_instance()->eapi_from_string(tokens[1]));
            if (! eapi->supported())
                return "EREWRITE_VAR EAPI " + tokens[1] + " unsupported";

            std::string var(tokens[3]);

            if ((var == eapi->supported()->ebuild_metadata_variables()->build_depend()->name()) ||
                    (var == eapi->supported()->ebuild_metadata_variables()->run_depend()->name()) ||
                    (var == eapi->supported()->ebuild_metadata_variables()->pdepend()->name()) ||
                    (var == eapi->supported()->ebuild_metadata_variables()->dependencies()->name()))
            {
                /* use the value in the metadata key, since RDEPEND in the
                 * ebuild env isn't modified for DEPEND=RDEPEND */
                PackageID::MetadataConstIterator m(package_id->find_metadata(var));
                if (m == package_id->end_metadata())
                    throw InternalError(PALUDIS_HERE, "oops. can't find key '" + var + "'");

                const MetadataSpecTreeKey<DependencySpecTree> * mm(
                        simple_visitor_cast<const MetadataSpecTreeKey<DependencySpecTree> >(**m));
                if (! mm)
                    throw InternalError(PALUDIS_HERE, "oops. key '" + var + "' isn't a DependencySpecTree key");

                std::shared_ptr<const DependencySpecTree> before(mm->value());
                std::shared_ptr<const DependencySpecTree> after(fix_locked_dependencies(
                            environment, *eapi, package_id, before));
                StringifyFormatter ff;
                DepSpecPrettyPrinter p(0, std::shared_ptr<const PackageID>(), ff, 0, false, false);
                after->top()->accept(p);
                return "O0;" + stringify(p);
            }
            else if (var == eapi->supported()->ebuild_metadata_variables()->myoptions()->name())
            {
                /* if options have a global description and no local description, rewrite it as
                 * a local description so that descriptions carry on working for installed stuff. */
                PackageID::MetadataConstIterator m(package_id->find_metadata(var));
                if (m == package_id->end_metadata())
                    throw InternalError(PALUDIS_HERE, "oops. can't find key '" + var + "'");

                const MetadataSpecTreeKey<PlainTextSpecTree> * mm(
                        simple_visitor_cast<const MetadataSpecTreeKey<PlainTextSpecTree> >(**m));
                if (! mm)
                    throw InternalError(PALUDIS_HERE, "oops. key '" + var + "' isn't a PlainTextSpecTree key");

                StringifyFormatter ff;
                MyOptionsRewriter p(package_id,
                        eapi->supported()->annotations()->myoptions_description(),
                        std::string(1, eapi->supported()->choices_options()->use_expand_separator()));
                mm->value()->top()->accept(p);
                return "O0;" + p.str.str();
            }

            return "O0;" + join(tokens.begin() + 4, tokens.end(), " ");
        }
        else if (tokens[0] == "EVER")
        {
            std::shared_ptr<const EAPI> eapi(EAPIData::get_instance()->eapi_from_string(tokens[1]));
            if (! eapi->supported())
                return "EVER EAPI " + tokens[1] + " unsupported";
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

                VersionSpec v1(tokens[3], eapi->supported()->version_spec_options()),
                            v2(tokens[4], eapi->supported()->version_spec_options());
                return v2 >= v1 ? "O0;" : "O1;";
            }
            else if (tokens[2] == "IS_SCM")
            {
                if (tokens.size() != 4)
                {
                    Log::get_instance()->message("e.pipe_commands.ever.is_scm.bad", ll_warning, lc_context) << "Got bad EVER IS_SCM pipe command";
                    return "Ebad EVER " + tokens[2] + " command {'" + join(tokens.begin(), tokens.end(), "', '") + "'}";
                }

                VersionSpec v(tokens[3], eapi->supported()->version_spec_options());

                return v.is_scm() ? "O0;" : "O1;";
            }
            else if (tokens[2] == "SPLIT" || tokens[2] == "SPLIT_ALL")
            {
                bool all(tokens[2] == "SPLIT_ALL");

                if (tokens.size() != 4)
                {
                    Log::get_instance()->message("e.pipe_commands.ever.split.bad", ll_warning, lc_context) << "Got bad EVER " + tokens[2] + " pipe command";
                    return "Ebad EVER " + tokens[2] + " command {'" + join(tokens.begin(), tokens.end(), "', '") + "'}";
                }

                VersionSpec v(tokens[3], eapi->supported()->version_spec_options());
                std::string result;
                for (VersionSpec::ConstIterator c(v.begin()), c_end(v.end()) ;
                        c != c_end ; ++c)
                {
                    if (c->text().empty())
                        continue;

                    if (! result.empty())
                        result.append(" ");

                    switch (c->text().at(0))
                    {
                        case '.':
                        case '-':
                        case '_':
                            {
                                if (all)
                                    result.append(c->text().substr(0, 1));
                                if (c->text().length() > 1)
                                {
                                    if (all)
                                        result.append(" ");
                                    result.append(c->text().substr(1));
                                }

                            }
                            break;

                        default:
                            result.append(c->text());
                            break;
                    }
                }
                return "O0;" + result;
            }
            else if (tokens[2] == "RANGE")
            {
                if (tokens.size() != 5)
                {
                    Log::get_instance()->message("e.pipe_commands.ever.major.bad", ll_warning, lc_context) << "Got bad EVER " + tokens[2] + " pipe command";
                    return "Ebad EVER " + tokens[2] + " command {'" + join(tokens.begin(), tokens.end(), "', '") + "'}";
                }

                std::string range_s(tokens[3]);
                std::string::size_type hyphen_pos(range_s.find('-'));
                int range_start, range_end;

                if (std::string::npos == hyphen_pos)
                    range_start = range_end = destringify<int>(range_s);
                else
                {
                    std::string range_start_s(range_s.substr(0, hyphen_pos));
                    std::string range_end_s(range_s.substr(hyphen_pos + 1));

                    if (range_start_s.empty())
                        range_start = 0;
                    else
                        range_start = destringify<int>(range_start_s);

                    if (range_end_s.empty())
                        range_end = std::numeric_limits<int>::max();
                    else
                        range_end = destringify<int>(range_end_s);

                }

                VersionSpec v(tokens[4], eapi->supported()->version_spec_options());

                int current_pos(0);
                std::string result;
                for (VersionSpec::ConstIterator c(v.begin()), c_end(v.end()) ;
                        c != c_end ; ++c)
                {
                    ++current_pos;
                    if (current_pos > range_end)
                        break;
                    if (current_pos < range_start)
                        continue;

                    if (c->text().empty())
                        continue;

                    switch (c->text().at(0))
                    {
                        case '.':
                        case '-':
                        case '_':
                            {
                                if (! result.empty())
                                    result.append(c->text().substr(0, 1));
                                if (c->text().length() > 1)
                                    result.append(c->text().substr(1));

                            }
                            break;

                        default:
                            result.append(c->text());
                            break;
                    }
                }

                return "O0;" + result;
            }
            else if (tokens[2] == "REPLACE")
            {
                if (tokens.size() != 6)
                {
                    Log::get_instance()->message("e.pipe_commands.ever.replace.bad", ll_warning, lc_context) << "Got bad EVER " + tokens[2] + " pipe command";
                    return "Ebad EVER " + tokens[2] + " command {'" + join(tokens.begin(), tokens.end(), "', '") + "'}";
                }

                VersionSpec v(tokens[5], eapi->supported()->version_spec_options());
                int current_pos(0);
                std::string replacement(tokens[4]);
                std::string result;

                switch (tokens[3].at(0))
                {
                    case '.':
                    case '-':
                    case '_':
                        {
                            bool replacing_done(false);
                            char replace_separator(tokens[3].at(0));

                            for (VersionSpec::ConstIterator c(v.begin()), c_end(v.end()) ;
                                    c != c_end ; ++c)
                            {
                                if (c->text().empty())
                                    continue;

                                if (! replacing_done && replace_separator == c->text().at(0))
                                {
                                    result.append(replacement + c->text().substr(1));
                                    replacing_done = true;
                                }
                                else
                                    result.append(c->text());
                            }
                        }
                        break;

                    default:
                        int replace_pos(destringify<int>(tokens[3]));

                        for (VersionSpec::ConstIterator c(v.begin()), c_end(v.end()) ;
                                c != c_end ; ++c)
                        {
                            if (c->text().empty())
                                continue;

                            switch (c->text().at(0))
                            {
                                case '.':
                                case '-':
                                case '_':
                                    {
                                        if (current_pos == replace_pos)
                                            result.append(replacement + c->text().substr(1));
                                        else
                                            result.append(c->text());
                                    }
                                    break;

                                default:
                                    result.append(c->text());
                                    break;
                            }

                            ++current_pos;
                        }
                        break;
                }

                return "O0;" + result;
            }
            else if (tokens[2] == "REPLACE_ALL")
            {
                if (tokens.size() != 5)
                {
                    Log::get_instance()->message("e.pipe_commands.ever.replace_all.bad", ll_warning, lc_context) << "Got bad EVER " + tokens[2] + " pipe command";
                    return "Ebad EVER " + tokens[2] + " command {'" + join(tokens.begin(), tokens.end(), "', '") + "'}";
                }

                VersionSpec v(tokens[4], eapi->supported()->version_spec_options());
                std::string replacement(tokens[3]);
                std::string result;
                for (VersionSpec::ConstIterator c(v.begin()), c_end(v.end()) ;
                        c != c_end ; ++c)
                {
                    if (c->text().empty())
                        continue;

                    switch (c->text().at(0))
                    {
                        case '.':
                        case '-':
                        case '_':
                            {
                                result.append(replacement);
                                if (c->text().length() > 1)
                                    result.append(c->text().substr(1));
                            }
                            break;

                        default:
                            result.append(c->text());
                            break;
                    }
                }

                return "O0;" + result;
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

