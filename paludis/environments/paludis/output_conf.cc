/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009 Ciaran McCreesh
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

#include <paludis/environments/paludis/output_conf.hh>
#include <paludis/environments/paludis/bashable_conf.hh>
#include <paludis/environments/paludis/paludis_config.hh>
#include <paludis/environments/paludis/paludis_environment.hh>
#include <paludis/environments/paludis/action_to_string.hh>
#include <paludis/util/log.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/named_value.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/config_file.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/destringify.hh>
#include <paludis/util/options.hh>
#include <paludis/util/make_shared_copy.hh>
#include <paludis/util/tribool.hh>
#include <paludis/util/simple_visitor_cast.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/create_output_manager_info.hh>
#include <paludis/package_id.hh>
#include <paludis/match_package.hh>
#include <paludis/action.hh>
#include <list>
#include <vector>

using namespace paludis;
using namespace paludis::paludis_environment;

namespace paludis
{
    namespace n
    {
        struct action_requirement;
        struct ignore_unfetched_requirement;
        struct manager;
        struct matches_requirement;
        struct name_requirement;
        struct output_exclusivity_requirement;
        struct type_requirement;
    }
}

namespace
{
    struct Rule
    {
        NamedValue<n::action_requirement, std::string> action_requirement;
        NamedValue<n::ignore_unfetched_requirement, Tribool> ignore_unfetched_requirement;
        NamedValue<n::manager, std::string> manager;
        NamedValue<n::matches_requirement, std::tr1::shared_ptr<PackageDepSpec> > matches_requirement;
        NamedValue<n::name_requirement, std::string> name_requirement;
        NamedValue<n::output_exclusivity_requirement, OutputExclusivity> output_exclusivity_requirement;
        NamedValue<n::type_requirement, std::string> type_requirement;
    };

    typedef std::list<Rule> RuleList;
}

namespace paludis
{
    template<>
    struct Implementation<OutputConf>
    {
        const PaludisEnvironment * const env;
        RuleList rules;

        Implementation(const PaludisEnvironment * const e) :
            env(e)
        {
        }
    };
}

OutputConf::OutputConf(const PaludisEnvironment * const e) :
    PrivateImplementationPattern<OutputConf>(new Implementation<OutputConf>(e))
{
}

OutputConf::~OutputConf()
{
}

namespace
{
    void set_rule(const Environment * const env, Rule & rule, const std::string & k, const std::string & v)
    {
        if (k == "type")
            rule.type_requirement() = v;
        else if (k == "name")
            rule.name_requirement() = v;
        else if (k == "output_exclusivity")
            rule.output_exclusivity_requirement() = destringify<OutputExclusivity>(v);
        else if (k == "matches")
            rule.matches_requirement() = make_shared_copy(parse_user_package_dep_spec(
                        v, env, UserPackageDepSpecOptions() + updso_allow_wildcards + updso_no_disambiguation));
        else if (k == "action")
            rule.action_requirement() = v;
        else if (k == "ignore_unfetched")
            rule.ignore_unfetched_requirement() = destringify<Tribool>(v);
        else
            throw PaludisConfigError("Unknown rule '" + k + "'");
    }

    struct MatchRuleVisitor
    {
        const Environment * const env;
        const Rule & rule;

        MatchRuleVisitor(const Environment * const e, const Rule & r) :
            env(e),
            rule(r)
        {
        }

        bool visit(const CreateOutputManagerForRepositorySyncInfo & i) const
        {
            if (rule.type_requirement() != "*" && rule.type_requirement() != "repository")
                return false;

            if (rule.name_requirement() != "*" && rule.name_requirement() != stringify(i.repository().name()))
                return false;

            if (rule.action_requirement() != "*" && rule.action_requirement() != "sync")
                return false;

            if (-1 != rule.output_exclusivity_requirement() &&
                    rule.output_exclusivity_requirement() != i.output_exclusivity())
                return false;

            if (rule.matches_requirement())
                return false;

            if (! rule.ignore_unfetched_requirement().is_indeterminate())
                return false;

            return true;
        }

        bool visit(const CreateOutputManagerForPackageIDActionInfo & i) const
        {
            if (rule.type_requirement() != "*" && rule.type_requirement() != "package")
                return false;

            if (rule.name_requirement() != "*" && rule.name_requirement() != stringify(i.package_id()->name()))
                return false;

            if (rule.action_requirement() != "*" && rule.action_requirement() != action_to_string(i.action()))
                return false;

            if (-1 != rule.output_exclusivity_requirement() &&
                    rule.output_exclusivity_requirement() != i.output_exclusivity())
                return false;

            if (rule.matches_requirement() && ! match_package(*env, *rule.matches_requirement(),
                        *i.package_id(), MatchPackageOptions()))
                return false;

            if (! rule.ignore_unfetched_requirement().is_indeterminate())
            {
                const FetchAction * const fetch_action(simple_visitor_cast<const FetchAction>(i.action()));
                if (! fetch_action)
                    return false;
                if (fetch_action->options.ignore_unfetched() != rule.ignore_unfetched_requirement().is_true())
                    return false;
            }

            return true;
        }
    };

    bool match_rule(const Environment * const e, const Rule & rule, const CreateOutputManagerInfo & i)
    {
        MatchRuleVisitor v(e, rule);
        return i.accept_returning<bool>(v);
    }
}

void
OutputConf::add(const FSEntry & filename)
{
    Context context("When adding source '" + stringify(filename) + "' as an output file:");

    std::tr1::shared_ptr<LineConfigFile> f(make_bashable_conf(filename, LineConfigFileOptions()));
    if (! f)
        return;

    for (LineConfigFile::ConstIterator line(f->begin()), line_end(f->end()) ;
            line != line_end ; ++line)
    {
        std::vector<std::string> tokens;
        tokenise_whitespace_quoted(*line, std::back_inserter(tokens));

        if (tokens.empty())
            continue;

        if ("source" == tokens.at(0))
        {
            if (tokens.size() != 2)
                throw PaludisConfigError("Invalid source line '" + *line + "'");

            add(FSEntry(tokens.at(1)));
            continue;
        }

        Rule rule(make_named_values<Rule>(
                    value_for<n::action_requirement>("*"),
                    value_for<n::ignore_unfetched_requirement>(indeterminate),
                    value_for<n::manager>("unset"),
                    value_for<n::matches_requirement>(make_null_shared_ptr()),
                    value_for<n::name_requirement>("*"),
                    value_for<n::output_exclusivity_requirement>(static_cast<OutputExclusivity>(-1)),
                    value_for<n::type_requirement>("*")
                    ));

        std::vector<std::string>::const_iterator t(tokens.begin()), t_end(tokens.end());

        for ( ; t != t_end ; ++t)
        {
            if (*t == ":")
                break;

            std::string::size_type p(t->find("="));
            if (std::string::npos != p)
                set_rule(_imp->env, rule, t->substr(0, p), t->substr(p + 1));
            else
            {
                std::string r(*t);
                if (++t == t_end)
                    throw PaludisConfigError("Expected '=' but found end of line for line '" + *line + "'");

                if (*t != "=")
                    throw PaludisConfigError("Expected '=' but found '" + *t + "' for line '" + *line + "'");

                if (++t == t_end)
                    throw PaludisConfigError("Expected value but found end of for line '" + *line + "'");

                set_rule(_imp->env, rule, r, *t);
            }
        }

        if (t == t_end)
            throw PaludisConfigError("Found no ':' for line '" + *line + "'");

        if (++t == t_end)
            throw PaludisConfigError("Found no manager after ':' for line '" + *line + "'");

        rule.manager() = *t;
        _imp->rules.push_back(rule);

        if (++t != t_end)
            throw PaludisConfigError("Trailing text after manager on line '" + *line + "'");
    }
}

const std::tr1::shared_ptr<OutputManager>
OutputConf::create_output_manager(const CreateOutputManagerInfo & i) const
{
    Context context("When creating output manager:");

    for (RuleList::const_reverse_iterator r(_imp->rules.rbegin()), r_end(_imp->rules.rend()) ;
            r != r_end ; ++r)
        if (match_rule(_imp->env, *r, i))
            return _imp->env->create_named_output_manager(r->manager(), i);

    throw PaludisConfigError("No matching output manager rule specified");
}

template class PrivateImplementationPattern<paludis_environment::OutputConf>;

