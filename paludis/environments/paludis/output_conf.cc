/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010, 2011 Ciaran McCreesh
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
#include <paludis/environments/paludis/extra_distribution_data.hh>

#include <paludis/util/log.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/named_value.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/config_file.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/destringify.hh>
#include <paludis/util/options.hh>
#include <paludis/util/make_shared_copy.hh>
#include <paludis/util/tribool.hh>
#include <paludis/util/visitor_cast.hh>
#include <paludis/util/map.hh>
#include <paludis/util/simple_parser.hh>

#include <paludis/user_dep_spec.hh>
#include <paludis/create_output_manager_info.hh>
#include <paludis/package_id.hh>
#include <paludis/match_package.hh>
#include <paludis/action.hh>
#include <paludis/output_manager_factory.hh>
#include <paludis/metadata_key.hh>
#include <paludis/distribution.hh>
#include <paludis/version_spec.hh>
#include <paludis/slot.hh>

#include <list>
#include <vector>
#include <map>
#include <algorithm>
#include <unistd.h>

using namespace paludis;
using namespace paludis::paludis_environment;

namespace paludis
{
    namespace n
    {
        typedef Name<struct name_action_requirement> action_requirement;
        typedef Name<struct name_ignore_unfetched_requirement> ignore_unfetched_requirement;
        typedef Name<struct name_manager> manager;
        typedef Name<struct name_matches_requirement> matches_requirement;
        typedef Name<struct name_name_requirement> name_requirement;
        typedef Name<struct name_output_exclusivity_requirement> output_exclusivity_requirement;
        typedef Name<struct name_type_requirement> type_requirement;
    }
}

namespace
{
    struct Rule
    {
        NamedValue<n::action_requirement, std::string> action_requirement;
        NamedValue<n::ignore_unfetched_requirement, Tribool> ignore_unfetched_requirement;
        NamedValue<n::manager, std::string> manager;
        NamedValue<n::matches_requirement, std::shared_ptr<PackageDepSpec> > matches_requirement;
        NamedValue<n::name_requirement, std::string> name_requirement;
        NamedValue<n::output_exclusivity_requirement, OutputExclusivity> output_exclusivity_requirement;
        NamedValue<n::type_requirement, std::string> type_requirement;
    };

    typedef std::list<Rule> RuleList;
    typedef std::map<std::string, std::shared_ptr<Map<std::string, std::string> > > Managers;

    std::string from_keys(
            const std::shared_ptr<const Map<std::string, std::string> > & m,
            const std::string & k)
    {
        if (m->end() == m->find(k))
            return "";
        else
            return m->find(k)->second;
    }
}

namespace paludis
{
    template<>
    struct Imp<OutputConf>
    {
        const PaludisEnvironment * const env;
        RuleList rules;
        Managers managers;
        std::map<std::string, std::string> misc_vars;
        std::shared_ptr<Map<std::string, std::string> > predefined_variables;

        Imp(const PaludisEnvironment * const e) :
            env(e),
            predefined_variables(std::make_shared<Map<std::string, std::string> >())
        {
        }
    };
}

OutputConf::OutputConf(const PaludisEnvironment * const e) :
    _imp(e)
{
}

OutputConf::~OutputConf() = default;

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
                        v, env, { updso_allow_wildcards, updso_no_disambiguation }));
        else if (k == "action")
            rule.action_requirement() = v;
        else if (k == "ignore_unfetched")
            rule.ignore_unfetched_requirement() = destringify<Tribool>(v);
        else if (k == "manager")
            rule.manager() = v;
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

            if (rule.name_requirement() != "*" && rule.name_requirement() != stringify(i.repository_name()))
                return false;

            if (rule.action_requirement() != "*" && rule.action_requirement() != "sync")
                return false;

            if (static_cast<OutputExclusivity>(-1) != rule.output_exclusivity_requirement() &&
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

            if (rule.action_requirement() != "*" && rule.action_requirement() != i.action_name())
                return false;

            if (static_cast<OutputExclusivity>(-1) != rule.output_exclusivity_requirement() &&
                    rule.output_exclusivity_requirement() != i.output_exclusivity())
                return false;

            if (rule.matches_requirement() && ! match_package(*env, *rule.matches_requirement(),
                        i.package_id(), nullptr, { }))
                return false;

            if (! rule.ignore_unfetched_requirement().is_indeterminate())
            {
                if (i.action_name() != FetchAction::class_simple_name())
                    return false;
                if ((i.action_flags()->end() != i.action_flags()->find(FetchAction::ignore_unfetched_flag_name())) !=
                        rule.ignore_unfetched_requirement().is_true())
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

    std::string escape(const std::string & s)
    {
        std::string result(s);
        std::replace(result.begin(), result.end(), ' ', '_');
        std::replace(result.begin(), result.end(), '/', '_');
        return result;
    }

    struct CreateVarsFromInfo
    {
        const Environment * const env;
        std::shared_ptr<Map<std::string, std::string> > m;

        CreateVarsFromInfo(
                const Environment * const e,
                std::shared_ptr<Map<std::string, std::string> > & mm) :
            env(e),
            m(mm)
        {
            /* convenience, for everyone */
            m->insert("newline", "\n");
            m->insert("red", "\033[1;31m");
            m->insert("yellow", "\033[1;33m");
            m->insert("green", "\033[1;32m");
            m->insert("blue", "\033[1;34m");
            m->insert("normal", "\033[0;0m");
            m->insert("time", stringify(time(nullptr)));
            m->insert("pid", stringify(getpid()));

            const std::shared_ptr<const PaludisDistribution> dist(
                    PaludisExtraDistributionData::get_instance()->data_from_distribution(
                        *DistributionData::get_instance()->distribution_from_string(env->distribution())));

            m->insert("info_messages_are_spam", stringify(dist->info_messages_are_spam()));
        }

        void visit(const CreateOutputManagerForRepositorySyncInfo & i)
        {
            m->insert("type", "repository");
            m->insert("action", "sync");
            m->insert("name", stringify(i.repository_name()));
            m->insert("full_name", stringify(i.repository_name()));
            m->insert("summaries_supported", stringify(i.client_output_features()[cof_summary_at_end]));
        }

        void visit(const CreateOutputManagerForPackageIDActionInfo & i)
        {
            m->insert("type", "package");
            m->insert("action", i.action_name());
            m->insert("name", stringify(i.package_id()->name()));
            m->insert("id", escape(stringify(*i.package_id())));
            m->insert("full_name", escape(stringify(*i.package_id())));
            if (i.package_id()->slot_key())
                m->insert("slot", stringify(i.package_id()->slot_key()->parse_value().raw_value()));
            m->insert("version", stringify(i.package_id()->version()));
            m->insert("repository", stringify(i.package_id()->repository_name()));
            m->insert("category", stringify(i.package_id()->name().category()));
            m->insert("package", stringify(i.package_id()->name().package()));
            m->insert("summaries_supported", stringify(i.client_output_features()[cof_summary_at_end]));
        }
    };

    const std::shared_ptr<Map<std::string, std::string> >
    vars_from_create_output_manager_info(
            const Environment * const env,
            const CreateOutputManagerInfo & i)
    {
        std::shared_ptr<Map<std::string, std::string> > result(std::make_shared<Map<std::string, std::string>>());
        CreateVarsFromInfo v(env, result);
        i.accept(v);
        return result;
    }

    const std::string replace_percent_vars(
        const std::string & s,
        const std::shared_ptr<const Map<std::string, std::string> > & vars,
        const std::shared_ptr<const Map<std::string, std::string> > & override_vars,
        const std::shared_ptr<const Map<std::string, std::string> > & file_vars)
    {
        std::string result;
        std::string token;
        SimpleParser parser(s);
        while (! parser.eof())
        {
            if (parser.consume((+simple_parser::any_except("%")) >> token))
                result.append(token);
            else if (parser.consume(simple_parser::exact("%%")))
                result.append("%");
            else if (parser.consume(simple_parser::exact("%{") &
                        ((+simple_parser::any_except("} \t\r\n%")) >> token) &
                        simple_parser::exact("}")))
            {
                Map<std::string, std::string>::ConstIterator v(override_vars->find(token));
                if (v == override_vars->end())
                    v = vars->find(token);
                if (v == vars->end())
                    v = file_vars->find(token);
                if (v == file_vars->end())
                    throw PaludisConfigError("No variable named '" + token + "' in var string '" + s + "'");

                result.append(v->second);
            }
            else
                throw PaludisConfigError("Invalid var string '" + s + "'");
        }

        return result;
    }
}

void
OutputConf::add(const FSPath & filename, const FSPath & root)
{
    Context context("When adding source '" + stringify(filename) + "' as an output file:");

    _imp->predefined_variables->erase(std::string("root"));
    _imp->predefined_variables->insert("root", stringify(root));
    _imp->predefined_variables->erase(std::string("ROOT"));
    _imp->predefined_variables->insert("ROOT", stringify(root));

    std::shared_ptr<KeyValueConfigFile> f(make_bashable_kv_conf(filename,
                _imp->predefined_variables, { kvcfo_allow_sections, kvcfo_allow_fancy_assigns, kvcfo_allow_env }));
    if (! f)
        return;

    Managers local_managers;
    Managers local_rules;

    for (const auto & kv : *f)
    {
        std::string remainder(kv.first);
        std::string::size_type p(remainder.find('/'));
        if (std::string::npos == p)
        {
            _imp->misc_vars[kv.first] = kv.second;
            continue;
        }

        std::string section_kind(remainder.substr(0, p));
        remainder.erase(0, p + 1);

        p = remainder.find('/');
        if (std::string::npos == p)
            throw PaludisConfigError("Section '" + section_kind + "' has no name");
        std::string section_name(remainder.substr(0, p));
        remainder.erase(0, p + 1);

        if (section_kind == "rule")
        {
            local_rules.insert(
                    std::make_pair(section_name,
                        std::make_shared<Map<std::string, std::string>>())).first->second->insert(
                    remainder, kv.second);
        }
        else if (section_kind == "manager")
        {
            local_managers.insert(
                    std::make_pair(section_name,
                        std::make_shared<Map<std::string, std::string>>())).first->second->insert(
                    remainder, kv.second);
        }
        else
            throw PaludisConfigError("Section kind '" + section_kind + "' unknown");
    }

    for (const auto & local_rule : local_rules)
    {
        Rule rule(make_named_values<Rule>(
                    n::action_requirement() = "*",
                    n::ignore_unfetched_requirement() = Tribool(indeterminate),
                    n::manager() = "unset",
                    n::matches_requirement() = nullptr,
                    n::name_requirement() = "*",
                    n::output_exclusivity_requirement() = static_cast<OutputExclusivity>(-1),
                    n::type_requirement() = "*"
                    ));
        for (const auto & kv : *local_rule.second)
            set_rule(_imp->env, rule, kv.first, kv.second);

        _imp->rules.push_back(rule);
    }

    for (const auto & local_manager : local_managers)
        _imp->managers.insert(std::make_pair(local_manager.first, local_manager.second));

    for (const auto & kv : *f)
    {
        _imp->predefined_variables->erase(kv.first);
        _imp->predefined_variables->insert(kv.first, kv.second);
    }
}

const std::shared_ptr<OutputManager>
OutputConf::create_output_manager(const CreateOutputManagerInfo & i) const
{
    Context context("When creating output manager:");

    for (RuleList::const_reverse_iterator r(_imp->rules.rbegin()), r_end(_imp->rules.rend()) ;
            r != r_end ; ++r)
        if (match_rule(_imp->env, *r, i))
            return create_named_output_manager(r->manager(), i);

    throw PaludisConfigError("No matching output manager rule specified");
}

const std::shared_ptr<OutputManager>
OutputConf::create_named_output_manager(const std::string & s, const CreateOutputManagerInfo & n) const
{
    Context context("When creating output manager named '" + s + "':");

    Managers::const_iterator i(_imp->managers.find(s));
    if (i == _imp->managers.end())
        throw PaludisConfigError("No output manager named '" + s + "' exists");

    std::shared_ptr<Map<std::string, std::string> > vars(vars_from_create_output_manager_info(
                _imp->env, n));

    std::string handler;
    if (i->second->end() != i->second->find("handler"))
        handler = i->second->find("handler")->second;

    if (handler == "conditional_alias")
    {
        /* easier to handle this specially here */
        std::string condition_variable;
        if (i->second->end() != i->second->find("condition_variable"))
            condition_variable = i->second->find("condition_variable")->second;

        if (condition_variable.empty())
            throw PaludisConfigError("No condition_variable specified for manager '" + s + "'");

        std::string value;
        if (_imp->misc_vars.end() != _imp->misc_vars.find(condition_variable))
            value = _imp->misc_vars.find(condition_variable)->second;
        else if (vars->end() != vars->find(condition_variable))
            value = vars->find(condition_variable)->second;

        std::string alias_var;
        if (value.empty())
            alias_var = "if_unset";
        else if (value == "true")
            alias_var = "if_true";
        else if (value == "false")
            alias_var = "if_false";
        else
            throw PaludisConfigError("For manager '" + s + "', condition_variable '" + condition_variable
                    + "' should be either 'true', 'false' or unset, but is instead '" + alias_var + "'");

        std::string alias;
        if (i->second->end() != i->second->find(alias_var))
            alias = i->second->find(alias_var)->second;

        if (alias.empty())
            throw PaludisConfigError("For manager '" + s + "', no alias is defined");

        return create_named_output_manager(alias, n);
    }
    else
        return OutputManagerFactory::get_instance()->create(
                std::bind(&from_keys, i->second, std::placeholders::_1),
                std::bind(&OutputConf::create_named_output_manager, this,
                    std::placeholders::_1, std::cref(n)),
                std::bind(replace_percent_vars, std::placeholders::_1, vars, std::placeholders::_2,
                    i->second)
                );
}

namespace paludis
{
    template class Pimp<paludis_environment::OutputConf>;
}

