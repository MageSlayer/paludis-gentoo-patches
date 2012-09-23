/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010, 2011, 2012 Ciaran McCreesh
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

#include <paludis/elike_use_requirement.hh>
#include <paludis/util/options.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/join.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/log.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/tribool.hh>
#include <paludis/dep_spec.hh>
#include <paludis/name.hh>
#include <paludis/environment.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/choice.hh>
#include <paludis/changed_choices.hh>
#include <paludis/additional_package_dep_spec_requirement.hh>
#include <vector>
#include <functional>
#include <algorithm>

using namespace paludis;

namespace
{
    bool icky_use_query(
            const ELikeUseRequirementOptions & options,
            const ChoiceNameWithPrefix & f,
            const std::shared_ptr<const PackageID> & id,
            const ChangedChoices * const changed_choices,
            Tribool default_value = Tribool(indeterminate))
    {
        if (! id->choices_key())
        {
            Log::get_instance()->message("elike_use_requirement.query", ll_warning, lc_context) <<
                "ID '" << *id << "' has no choices, so couldn't get the state of flag '" << f << "'";
            return false;
        }

        if (changed_choices)
        {
            auto c(changed_choices->overridden_value(f));
            if (! c.is_indeterminate())
                return c.is_true();
        }

        auto choices(id->choices_key()->parse_value());
        auto v(choices->find_by_name_with_prefix(f));
        if (v)
        {
            if (co_special == v->origin())
                Log::get_instance()->message("elike_use_requirement.query", ll_warning, lc_context) <<
                    "ID '" << *id << "' flag '" << f << "' should not be used as a conditional";
            return v->enabled();
        }

        if (default_value.is_indeterminate() && ! choices->has_matching_contains_every_value_prefix(f) && options[euro_missing_is_qa])
            Log::get_instance()->message("elike_use_requirement.query", ll_qa, lc_context) <<
                "ID '" << *id << "' has no flag named '" << f << "'";
        return default_value.is_true();
    }

    class UseRequirement
    {
        private:
            const std::string _flags;
            const Tribool _default_value;
            const bool _ignore_if_no_such_group;

        protected:
            const ELikeUseRequirementOptions _options;

        public:
            UseRequirement(const std::string & n,
                    const ELikeUseRequirementOptions & o, Tribool d, const bool g) :
                _flags(n),
                _default_value(d),
                _ignore_if_no_such_group(g),
                _options(o)
            {
            }

            virtual ~UseRequirement()
            {
            }

            virtual bool one_requirement_met_base(
                    const Environment * const,
                    const ChoiceNameWithPrefix &,
                    const ChangedChoices * const,
                    const std::shared_ptr<const PackageID> &,
                    const std::shared_ptr<const PackageID> &,
                    const ChangedChoices * const) const PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            bool one_requirement_met(
                    const Environment * const env,
                    const ChoiceNameWithPrefix & c,
                    const ChangedChoices * const maybe_changes_to_owner,
                    const std::shared_ptr<const PackageID> & id,
                    const std::shared_ptr<const PackageID> & from_id,
                    const ChangedChoices * const maybe_changes_to_target) const PALUDIS_ATTRIBUTE((warn_unused_result))
            {
                if (_ignore_if_no_such_group)
                {
                    std::string::size_type p(_flags.find(':'));
                    ChoicePrefixName prefix((std::string::npos == p) ? "" : _flags.substr(0, p));
                    if (! id->choices_key())
                        return true;
                    auto choices(id->choices_key()->parse_value());
                    Choices::ConstIterator k(choices->find(prefix));
                    if (choices->end() == k)
                        return true;
                    if ((*k)->begin() == (*k)->end())
                        return true;
                }

                return one_requirement_met_base(env, c, maybe_changes_to_owner, id, from_id, maybe_changes_to_target);
            }

            virtual const std::string as_human_string(
                    const std::shared_ptr<const PackageID> & from_id) const PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            const std::string flags() const PALUDIS_ATTRIBUTE((warn_unused_result))
            {
                return _flags;
            }

            const std::pair<bool, std::string> requirement_met(
                    const Environment * const env,
                    const ChangedChoices * const maybe_changes_to_owner,
                    const std::shared_ptr<const PackageID> & id,
                    const std::shared_ptr<const PackageID> & from_id,
                    const ChangedChoices * const maybe_changes_to_target) const
            {
                if (_flags.length() >= 2 && ":*" == _flags.substr(_flags.length() - 2))
                {
                    if ((! _options[euro_allow_self_deps]) || (! from_id) || (! from_id->choices_key()))
                        throw ELikeUseRequirementError(_flags, "[prefix:*] not allowed here");
                    if (_options[euro_portage_syntax] && ! _options[euro_both_syntaxes])
                    {
                        if (_options[euro_strict_parsing])
                            throw ELikeUseRequirementError(_flags, "[prefix:*] not allowed here");
                        else
                            Log::get_instance()->message("elike_use_requirement.prefix_star_not_allowed", ll_warning, lc_context)
                                << "[prefix:*] not safe for use here";
                    }

                    ChoicePrefixName prefix(_flags.substr(0, _flags.length() - 2));
                    auto choices(from_id->choices_key()->parse_value());
                    Choices::ConstIterator cc(choices->find(prefix));
                    if (cc == choices->end())
                        Log::get_instance()->message("elike_use_requirement.prefix_star_unmatching", ll_warning, lc_context) <<
                            "ID '" << *from_id << "' uses requirement '" << _flags << "' but has no choice prefix '" << prefix << "'";
                    else
                    {
                        std::pair<bool, std::string> result(true, "");
                        for (Choice::ConstIterator v((*cc)->begin()), v_end((*cc)->end()) ;
                                v != v_end ; ++v)
                            if (! one_requirement_met(env, (*v)->name_with_prefix(), maybe_changes_to_owner, id, from_id, maybe_changes_to_target))
                            {
                                if (! result.first)
                                    result.second.append(", ");
                                result.second.append(stringify((*v)->name_with_prefix()));
                                result.first = false;
                            }

                        if (! result.first)
                        {
                            result.second = as_human_string(from_id) + " (unmet: " + result.second + ")";
                            return result;
                        }
                    }
                }
                else
                    if (! one_requirement_met(env, ChoiceNameWithPrefix(_flags), maybe_changes_to_owner, id, from_id, maybe_changes_to_target))
                        return std::make_pair(false, as_human_string(from_id));

                return std::make_pair(true, as_human_string(from_id));
            }

            const Tribool default_value() const PALUDIS_ATTRIBUTE((warn_unused_result))
            {
                return _default_value;
            }

            const std::string default_value_human_string_fragment() const PALUDIS_ATTRIBUTE((warn_unused_result))
            {
                std::string result;
                if (_default_value.is_true())
                    result = ", assuming enabled if missing";
                else if (_default_value.is_false())
                    result = ", assuming disabled if missing";

                if (_ignore_if_no_such_group)
                    result.append(", only if the target has any flags with this prefix");

                return result;
            }

            Tribool accumulate_changes_to_make_met(
                    const Environment * const env,
                    const ChangedChoices * const maybe_changes_to_owner,
                    const std::shared_ptr<const PackageID> & id,
                    const std::shared_ptr<const PackageID> & from_id,
                    ChangedChoices & changed_choices) const
            {
                if (requirement_met(env, maybe_changes_to_owner, id, from_id, &changed_choices).first)
                    return indeterminate;

                if (_flags.length() >= 2 && ":*" == _flags.substr(_flags.length() - 2))
                {
                    if ((! _options[euro_allow_self_deps]) || (! from_id) || (! from_id->choices_key()))
                        throw ELikeUseRequirementError(_flags, "[prefix:*] not allowed here");
                    if (_options[euro_portage_syntax] && ! _options[euro_both_syntaxes])
                    {
                        if (_options[euro_strict_parsing])
                            throw ELikeUseRequirementError(_flags, "[prefix:*] not allowed here");
                        else
                            Log::get_instance()->message("elike_use_requirement.prefix_star_not_allowed", ll_warning, lc_context)
                                << "[prefix:*] not safe for use here";
                    }

                    ChoicePrefixName prefix(_flags.substr(0, _flags.length() - 2));
                    auto choices(from_id->choices_key()->parse_value());
                    Choices::ConstIterator cc(choices->find(prefix));
                    if (cc == choices->end())
                        Log::get_instance()->message("elike_use_requirement.prefix_star_unmatching", ll_warning, lc_context) <<
                            "ID '" << *from_id << "' uses requirement '" << _flags << "' but has no choice prefix '" << prefix << "'";
                    else
                    {
                        for (Choice::ConstIterator v((*cc)->begin()), v_end((*cc)->end()) ;
                                v != v_end ; ++v)
                            if (! one_accumulate_changes_to_make_met(env, maybe_changes_to_owner, id, changed_choices, (*v)->name_with_prefix()))
                                return false;
                    }
                }
                else
                    if (! one_accumulate_changes_to_make_met(env, maybe_changes_to_owner, id, changed_choices, ChoiceNameWithPrefix(_flags)))
                        return false;

                return true;
            }

            bool one_accumulate_changes_to_make_met(
                    const Environment * const,
                    const ChangedChoices * const,
                    const std::shared_ptr<const PackageID> & id,
                    ChangedChoices & changed_choices,
                    const ChoiceNameWithPrefix & flag) const
            {
                const std::shared_ptr<const ChoiceValue> v(id->choices_key()->parse_value()->find_by_name_with_prefix(flag));

                if (! v)
                {
                    /* warning messages get done elsewhere, no need here */
                    return false;
                }

                if (v->locked() || co_explicit != v->origin())
                    return false;

                return changed_choices.add_override_if_possible(flag, ! v->enabled());;
            }
    };

    class PALUDIS_VISIBLE EnabledUseRequirement :
        public UseRequirement
    {
        public:
            EnabledUseRequirement(const std::string & n,
                    const ELikeUseRequirementOptions & o,
                    Tribool d, const bool b) :
                UseRequirement(n, o, d, b)
            {
            }

            virtual bool one_requirement_met_base(const Environment * const, const ChoiceNameWithPrefix & flag, const ChangedChoices * const,
                    const std::shared_ptr<const PackageID> & pkg, const std::shared_ptr<const PackageID> &, const ChangedChoices * const changed_choices) const
            {
                return icky_use_query(_options, flag, pkg, changed_choices, default_value());
            }

            virtual const std::string as_human_string(
                    const std::shared_ptr<const PackageID> &) const
            {
                return "Flag '" + stringify(flags()) + "' enabled" + default_value_human_string_fragment();
            }
    };

    class PALUDIS_VISIBLE DisabledUseRequirement :
        public UseRequirement
    {
        public:
            DisabledUseRequirement(const std::string & n,
                    const ELikeUseRequirementOptions & o,
                    Tribool d, const bool b) :
                UseRequirement(n, o, d, b)
            {
            }

            virtual bool one_requirement_met_base(const Environment * const, const ChoiceNameWithPrefix & flag, const ChangedChoices * const,
                    const std::shared_ptr<const PackageID> & pkg, const std::shared_ptr<const PackageID> &, const ChangedChoices * const changed_choices) const
            {
                return ! icky_use_query(_options, flag, pkg, changed_choices, default_value());
            }

            virtual const std::string as_human_string(
                    const std::shared_ptr<const PackageID> &) const
            {
                return "Flag '" + stringify(flags()) + "' disabled" + default_value_human_string_fragment();
            }
    };

    class PALUDIS_VISIBLE EnabledOrDisabledUseRequirement :
        public UseRequirement
    {
        public:
            EnabledOrDisabledUseRequirement(const std::string & n,
                    const ELikeUseRequirementOptions & o,
                    Tribool d, const bool b) :
                UseRequirement(n, o, d, b)
            {
            }

            virtual bool one_requirement_met_base(const Environment * const, const ChoiceNameWithPrefix & flag, const ChangedChoices * const,
                    const std::shared_ptr<const PackageID> & pkg, const std::shared_ptr<const PackageID> &, const ChangedChoices * const changed_choices) const
            {
                return icky_use_query(_options, flag, pkg, changed_choices, default_value()) ||
                    ! icky_use_query(_options, flag, pkg, changed_choices, default_value());
            }

            virtual const std::string as_human_string(
                    const std::shared_ptr<const PackageID> &) const
            {
                return "Flag '" + stringify(flags()) + "' either enabled or disabled" + default_value_human_string_fragment();
            }
    };

    class PALUDIS_VISIBLE ConditionalUseRequirement :
        public UseRequirement
    {
        public:
            ConditionalUseRequirement(const std::string & n,
                    const ELikeUseRequirementOptions & o,
                    Tribool d, const bool b) :
                UseRequirement(n, o, d, b)
            {
            }
    };

    class PALUDIS_VISIBLE IfMineThenUseRequirement :
        public ConditionalUseRequirement
    {
        public:
            IfMineThenUseRequirement(
                    const std::string & n,
                    const ELikeUseRequirementOptions & o,
                    Tribool d, const bool b) :
                ConditionalUseRequirement(n, o, d, b)
            {
            }

            virtual bool one_requirement_met_base(const Environment * const, const ChoiceNameWithPrefix & flag,
                    const ChangedChoices * const maybe_changes_to_owner, const std::shared_ptr<const PackageID> & pkg,
                    const std::shared_ptr<const PackageID> & from_id, const ChangedChoices * const changed_choices) const
            {
                return ! icky_use_query(_options, flag, from_id, maybe_changes_to_owner) ||
                    icky_use_query(_options, flag, pkg, changed_choices, default_value());
            }

            virtual const std::string as_human_string(
                    const std::shared_ptr<const PackageID> & from_id) const
            {
                return "Flag '" + stringify(flags()) + "' enabled if it is enabled for '"
                    + stringify(*from_id) + "'" + default_value_human_string_fragment();
            }
    };

    class PALUDIS_VISIBLE IfNotMineThenUseRequirement :
        public ConditionalUseRequirement
    {
        public:
            IfNotMineThenUseRequirement(const std::string & n,
                    const ELikeUseRequirementOptions & o,
                    Tribool d, const bool b) :
                ConditionalUseRequirement(n, o, d, b)
            {
            }

            virtual bool one_requirement_met_base(const Environment * const, const ChoiceNameWithPrefix & flag,
                    const ChangedChoices * const maybe_changes_to_owner,
                    const std::shared_ptr<const PackageID> & pkg, const std::shared_ptr<const PackageID> & from_id,
                    const ChangedChoices * const changed_choices) const
            {
                return icky_use_query(_options, flag, from_id, maybe_changes_to_owner) ||
                    icky_use_query(_options, flag, pkg, changed_choices, default_value());
            }

            virtual const std::string as_human_string(
                    const std::shared_ptr<const PackageID> & from_id) const
            {
                return "Flag '" + stringify(flags()) + "' enabled if it is disabled for '" +
                    stringify(*from_id) + "'" + default_value_human_string_fragment();
            }
    };

    class PALUDIS_VISIBLE IfMineThenNotUseRequirement :
        public ConditionalUseRequirement
    {
        public:
            IfMineThenNotUseRequirement(const std::string & n,
                    const ELikeUseRequirementOptions & o,
                    Tribool d, const bool b) :
                ConditionalUseRequirement(n, o, d, b)
            {
            }

            virtual bool one_requirement_met_base(const Environment * const, const ChoiceNameWithPrefix & flag,
                    const ChangedChoices * const maybe_changes_to_owner, const std::shared_ptr<const PackageID> & pkg,
                    const std::shared_ptr<const PackageID> & from_id, const ChangedChoices * const changed_choices) const
            {
                return ! icky_use_query(_options, flag, from_id, maybe_changes_to_owner) ||
                    ! icky_use_query(_options, flag, pkg, changed_choices, default_value());
            }

            virtual const std::string as_human_string(
                    const std::shared_ptr<const PackageID> & from_id) const
            {
                return "Flag '" + stringify(flags()) + "' disabled if it is enabled for '" +
                    stringify(*from_id) + "'" + default_value_human_string_fragment();
            }
    };

    class PALUDIS_VISIBLE IfNotMineThenNotUseRequirement :
        public ConditionalUseRequirement
    {
        public:
            IfNotMineThenNotUseRequirement(const std::string & n,
                    const ELikeUseRequirementOptions & o,
                    Tribool d, const bool b) :
                ConditionalUseRequirement(n, o, d, b)
            {
            }

            virtual bool one_requirement_met_base(const Environment * const, const ChoiceNameWithPrefix & flag,
                    const ChangedChoices * const maybe_changes_to_owner, const std::shared_ptr<const PackageID> & pkg,
                    const std::shared_ptr<const PackageID> & from_id, const ChangedChoices * const changed_choices) const
            {
                return icky_use_query(_options, flag, from_id, maybe_changes_to_owner) ||
                    ! icky_use_query(_options, flag, pkg, changed_choices, default_value());
            }

            virtual const std::string as_human_string(
                    const std::shared_ptr<const PackageID> & from_id) const
            {
                return "Flag '" + stringify(flags()) + "' disabled if it is disabled for '" +
                    stringify(*from_id) + "'" + default_value_human_string_fragment();
            }
    };

    class PALUDIS_VISIBLE EqualUseRequirement :
        public ConditionalUseRequirement
    {
        public:
            EqualUseRequirement(const std::string & n,
                    const ELikeUseRequirementOptions & o,
                    Tribool d, const bool b) :
                ConditionalUseRequirement(n, o, d, b)
            {
            }

            virtual bool one_requirement_met_base(const Environment * const, const ChoiceNameWithPrefix & flag,
                    const ChangedChoices * const maybe_changes_to_owner, const std::shared_ptr<const PackageID> & pkg,
                    const std::shared_ptr<const PackageID> & from_id, const ChangedChoices * const changed_choices) const
            {
                return icky_use_query(_options, flag, pkg, changed_choices, default_value()) ==
                    icky_use_query(_options, flag, from_id, maybe_changes_to_owner);
            }

            virtual const std::string as_human_string(
                    const std::shared_ptr<const PackageID> & from_id) const
            {
                return "Flag '" + stringify(flags()) + "' enabled or disabled like it is for '"
                    + stringify(*from_id) + "'" + default_value_human_string_fragment();
            }
    };

    class PALUDIS_VISIBLE NotEqualUseRequirement :
        public ConditionalUseRequirement
    {
        public:
            NotEqualUseRequirement(const std::string & n,
                    const ELikeUseRequirementOptions & o,
                    Tribool d, const bool b) :
                ConditionalUseRequirement(n, o, d, b)
            {
            }

            virtual bool one_requirement_met_base(const Environment * const, const ChoiceNameWithPrefix & flag,
                    const ChangedChoices * const maybe_changes_to_owner, const std::shared_ptr<const PackageID> & pkg,
                    const std::shared_ptr<const PackageID> & from_id, const ChangedChoices * const changed_choices) const
            {
                return icky_use_query(_options, flag, pkg, changed_choices, default_value()) !=
                    icky_use_query(_options, flag, from_id, maybe_changes_to_owner);
            }

            virtual const std::string as_human_string(
                    const std::shared_ptr<const PackageID> & from_id) const
            {
                return "Flag '" + stringify(flags()) + "' enabled or disabled opposite to how it is for '"
                    + stringify(*from_id) + "'" + default_value_human_string_fragment();
            }
    };

    class UseRequirements :
        public AdditionalPackageDepSpecRequirement
    {
        private:
            typedef std::vector<std::shared_ptr<const UseRequirement> > Reqs;

            std::string _raw;
            Reqs _reqs;

        public:
            UseRequirements(const std::string & r) :
                _raw(r)
            {
            }

            virtual const std::pair<bool, std::string> requirement_met(
                    const Environment * const env,
                    const ChangedChoices * const maybe_changes_to_owner,
                    const std::shared_ptr<const PackageID> & id,
                    const std::shared_ptr<const PackageID> & from_id,
                    const ChangedChoices * const maybe_changes_to_target) const
            {
                using namespace std::placeholders;

                std::pair<bool, std::string> result(true, "");
                for (Reqs::const_iterator r(_reqs.begin()), r_end(_reqs.end()) ;
                        r != r_end ; ++r)
                {
                    std::pair<bool, std::string> r_result((*r)->requirement_met(env, maybe_changes_to_owner, id, from_id, maybe_changes_to_target));
                    if (! r_result.first)
                    {
                        if (! result.first)
                            result.second.append("; ");
                        result.second.append(r_result.second);
                        result.first = false;
                    }
                }

                return result;
            }

            virtual const std::string as_human_string(
                    const std::shared_ptr<const PackageID> & from_id) const
            {
                return join(_reqs.begin(), _reqs.end(), "; ", std::bind(std::mem_fn(&UseRequirement::as_human_string),
                            std::placeholders::_1, from_id));
            }

            virtual const std::string as_raw_string() const
            {
                return _raw;
            }

            void add_requirement(const std::shared_ptr<const UseRequirement> & req)
            {
                _reqs.push_back(req);
            }

            virtual Tribool accumulate_changes_to_make_met(
                    const Environment * const env,
                    const ChangedChoices * const maybe_changes_to_owner,
                    const std::shared_ptr<const PackageID> & id,
                    const std::shared_ptr<const PackageID> & from_id,
                    ChangedChoices & changed_choices) const
            {
                Tribool result(indeterminate);
                for (auto r(_reqs.begin()), r_end(_reqs.end()) ;
                        r != r_end ; ++r)
                {
                    auto b((*r)->accumulate_changes_to_make_met(env, maybe_changes_to_owner, id, from_id, changed_choices));
                    if (b.is_false())
                        return false;
                    else if (b.is_true())
                        result = true;
                }

                return result;
            }
    };

    template <typename T_>
    std::shared_ptr<const UseRequirement>
    make_requirement(const std::string & n, const ELikeUseRequirementOptions & o, Tribool d, const bool b)
    {
        return std::make_shared<T_>(n, o, d, b);
    }

    typedef std::shared_ptr<const UseRequirement> (* Factory)(
            const std::string &, const ELikeUseRequirementOptions &, Tribool, bool);

    void
    parse_flag(
            const std::shared_ptr<UseRequirements> & result,
            const Factory & factory,
            const std::string & c,
            Tribool d,
            const bool i,
            const ELikeUseRequirementOptions & options,
            const std::shared_ptr<Set<std::string> > & maybe_accumulate_mentioned)
    {
        result->add_requirement(factory(c, options, d, i));
        if (maybe_accumulate_mentioned)
            maybe_accumulate_mentioned->insert(c);
    }

    void
    parse_one_use_requirement(
            const std::shared_ptr<UseRequirements> & result,
            const std::string & s, std::string & flag,
            const ELikeUseRequirementOptions & options,
            const std::shared_ptr<Set<std::string> > & maybe_accumulate_mentioned)
    {
        Factory factory;

        if (flag.empty())
            throw ELikeUseRequirementError(s, "Invalid [] contents");

        if ('=' == flag.at(flag.length() - 1))
        {
            if ((! options[euro_allow_self_deps]))
                throw ELikeUseRequirementError(s, "Cannot use [use=] here");

            flag.erase(flag.length() - 1);
            if (flag.empty())
                throw ELikeUseRequirementError(s, "Invalid [] contents");

            if ('!' == flag.at(flag.length() - 1))
            {
                if (options[euro_portage_syntax] && ! options[euro_both_syntaxes])
                {
                    if (options[euro_strict_parsing])
                        throw ELikeUseRequirementError(s, "[use!=] not safe for use here");
                    else
                        Log::get_instance()->message("e.use_requirement.flag_not_equal_not_allowed", ll_warning, lc_context)
                            << "[use!=] not safe for use here";
                }

                flag.erase(flag.length() - 1, 1);
                if (flag.empty())
                    throw ELikeUseRequirementError(s, "Invalid [] contents");
                factory = make_requirement<NotEqualUseRequirement>;
            }
            else if ('!' == flag.at(0))
            {
                if (! options[euro_portage_syntax] && ! options[euro_both_syntaxes])
                {
                    if (options[euro_strict_parsing])
                        throw ELikeUseRequirementError(s, "[!use=] not safe for use here");
                    else
                        Log::get_instance()->message("e.use_requirement.not_flag_equal_not_allowed", ll_warning, lc_context)
                            << "[!use=] not safe for use here";
                }

                flag.erase(0, 1);
                if (flag.empty())
                    throw ELikeUseRequirementError(s, "Invalid [] contents");
                factory = make_requirement<NotEqualUseRequirement>;
            }
            else
                factory = make_requirement<EqualUseRequirement>;
        }
        else if ('?' == flag.at(flag.length() - 1))
        {
            if ((! options[euro_allow_self_deps]))
                throw ELikeUseRequirementError(s, "Cannot use [use?] here");

            flag.erase(flag.length() - 1);
            if (flag.empty())
                throw ELikeUseRequirementError(s, "Invalid [] contents");

            if ('!' == flag.at(flag.length() - 1))
            {
                flag.erase(flag.length() - 1, 1);
                if (flag.empty())
                    throw ELikeUseRequirementError(s, "Invalid [] contents");

                if ('-' == flag.at(0))
                {
                    if (options[euro_portage_syntax] && ! options[euro_both_syntaxes])
                    {
                        if (options[euro_strict_parsing])
                            throw ELikeUseRequirementError(s, "[-use!?] not safe for use here");
                        else
                            Log::get_instance()->message("e.use_requirement.minus_flag_not_question_not_allowed", ll_warning, lc_context)
                                << "[-use!?] not safe for use here";
                    }

                    flag.erase(0, 1);
                    if (flag.empty())
                        throw ELikeUseRequirementError(s, "Invalid [] contents");

                    factory = make_requirement<IfNotMineThenNotUseRequirement>;
                }
                else
                {
                    if (options[euro_portage_syntax] && ! options[euro_both_syntaxes])
                    {
                        if (options[euro_strict_parsing])
                            throw ELikeUseRequirementError(s, "[use!?] not safe for use here");
                        else
                            Log::get_instance()->message("e.use_requirement.flag_not_question_not_allowed", ll_warning, lc_context)
                                << "[use!?] not safe for use here";
                    }

                    factory = make_requirement<IfNotMineThenUseRequirement>;
                }
            }
            else if ('!' == flag.at(0))
            {
                if (! options[euro_portage_syntax] && ! options[euro_both_syntaxes])
                {
                    if (options[euro_strict_parsing])
                        throw ELikeUseRequirementError(s, "[!use?] not safe for use here");
                    else
                        Log::get_instance()->message("e.use_requirement.not_flag_question_not_allowed", ll_warning, lc_context)
                            << "[!use?] not safe for use here";
                }

                flag.erase(0, 1);
                if (flag.empty())
                    throw ELikeUseRequirementError(s, "Invalid [] contents");
                factory = make_requirement<IfNotMineThenNotUseRequirement>;
            }
            else
            {
                if ('-' == flag.at(0))
                {
                    if (options[euro_portage_syntax] && ! options[euro_both_syntaxes])
                    {
                        if (options[euro_strict_parsing])
                            throw ELikeUseRequirementError(s, "[-use?] not safe for use here");
                        else
                            Log::get_instance()->message("e.use_requirement.minus_flag_question_not_allowed", ll_warning, lc_context)
                                << "[-use?] not safe for use here";
                    }

                    flag.erase(0, 1);
                    if (flag.empty())
                        throw ELikeUseRequirementError(s, "Invalid [] contents");

                    factory = make_requirement<IfMineThenNotUseRequirement>;
                }
                else
                    factory = make_requirement<IfMineThenUseRequirement>;
            }
        }
        else if ('-' == flag.at(0))
        {
            flag.erase(0, 1);
            if (flag.empty())
                throw ELikeUseRequirementError(s, "Invalid [] contents");
            factory = make_requirement<DisabledUseRequirement>;
        }
        else if ('?' == flag.at(0))
        {
            if (options[euro_portage_syntax] && ! options[euro_both_syntaxes])
            {
                if (options[euro_strict_parsing])
                    throw ELikeUseRequirementError(s, "[?use] not safe for use here");
                else
                    Log::get_instance()->message("e.use_requirement.question_flag_not_allowed", ll_warning, lc_context)
                        << "[?use] not safe for use here";
            }

            flag.erase(0, 1);
            if (flag.empty())
                throw ELikeUseRequirementError(s, "Invalid [] contents");
            factory = make_requirement<EnabledOrDisabledUseRequirement>;
        }
        else
            factory = make_requirement<EnabledUseRequirement>;

        if (')' == flag.at(flag.length() - 1))
        {
            if (flag.length() < 4 || flag.at(flag.length() - 3) != '(')
                throw ELikeUseRequirementError(s, "Invalid [] contents");

            if ('+' == flag.at(flag.length() - 2))
            {
                if (! options[euro_allow_default_values])
                {
                    if (options[euro_strict_parsing])
                        throw ELikeUseRequirementError(s, "[use(+)] not safe for use here");
                    else
                        Log::get_instance()->message("e.use_requirement.flag_bracket_plus_not_allowed", ll_warning, lc_context)
                            << "[use(+)] not safe for use here";
                }

                flag.erase(flag.length() - 3, 3);
                parse_flag(result, factory, flag, Tribool(true), false, options, maybe_accumulate_mentioned);
            }
            else if ('-' == flag.at(flag.length() - 2))
            {
                if (! options[euro_allow_default_values])
                {
                    if (options[euro_strict_parsing])
                        throw ELikeUseRequirementError(s, "[use(-)] not safe for use here");
                    else
                        Log::get_instance()->message("e.use_requirement.flag_bracket_minus_not_allowed", ll_warning, lc_context)
                            << "[use(-)] not safe for use here";
                }

                flag.erase(flag.length() - 3, 3);
                parse_flag(result, factory, flag, Tribool(false), false, options, maybe_accumulate_mentioned);
            }
            else if ('?' == flag.at(flag.length() - 2))
            {
                if (! options[euro_allow_default_question_values])
                {
                    if (options[euro_strict_parsing])
                        throw ELikeUseRequirementError(s, "[use(?)] not safe for use here");
                    else
                        Log::get_instance()->message("e.use_requirement.flag_bracket_question_not_allowed", ll_warning, lc_context)
                            << "[use(?)] not safe for use here";
                }

                flag.erase(flag.length() - 3, 3);
                parse_flag(result, factory, flag, Tribool(false), true, options, maybe_accumulate_mentioned);
            }
            else
                throw ELikeUseRequirementError(s, "Invalid [] contents");
        }
        else
            parse_flag(result, factory, flag, Tribool(indeterminate), false, options, maybe_accumulate_mentioned);
    }
}

ELikeUseRequirementError::ELikeUseRequirementError(const std::string & s, const std::string & m) throw () :
    Exception("Error parsing use requirement '" + s + "': " + m)
{
}

std::shared_ptr<const AdditionalPackageDepSpecRequirement>
paludis::parse_elike_use_requirement(const std::string & s,
        const ELikeUseRequirementOptions & options,
        const std::shared_ptr<Set<std::string> > & maybe_accumulate_mentioned)
{
    Context context("When parsing use requirement '" + s + "':");

    std::shared_ptr<UseRequirements> result(std::make_shared<UseRequirements>("[" + s + "]"));
    std::string::size_type pos(0);
    for (;;)
    {
        std::string::size_type comma(s.find(',', pos));
        std::string flag(s.substr(pos, std::string::npos == comma ? comma : comma - pos));
        parse_one_use_requirement(result, s, flag, options, maybe_accumulate_mentioned);
        if (std::string::npos == comma)
            break;

        if (! options[euro_portage_syntax] && ! options[euro_both_syntaxes])
        {
            if (options[euro_strict_parsing])
                throw ELikeUseRequirementError(s, "[use,use] not safe for use here");
            else
                Log::get_instance()->message("e.use_requirement.comma_separated_not_allowed", ll_warning, lc_context)
                    << "[use,use] not safe for use here";
        }

        pos = comma + 1;
    }

    return result;
}

namespace
{
    class ELikePresumedChoicesRequirement :
        public AdditionalPackageDepSpecRequirement
    {
        private:
            const std::shared_ptr<const Set<std::string> > _mentioned;

        public:
            ELikePresumedChoicesRequirement(
                    const std::shared_ptr<const Set<std::string> > m) :
                _mentioned(m)
            {
            }

            virtual const std::pair<bool, std::string> requirement_met(
                    const Environment * const,
                    const ChangedChoices * const,
                    const std::shared_ptr<const PackageID> & id,
                    const std::shared_ptr<const PackageID> &,
                    const ChangedChoices * const) const PALUDIS_ATTRIBUTE((warn_unused_result))
            {
                if (! id->choices_key())
                    return std::make_pair(true, "");

                auto choices(id->choices_key()->parse_value());
                std::string p;
                int n(0);
                for (auto c(choices->begin()), c_end(choices->end()) ; c != c_end ; ++c)
                {
                    if (_mentioned->end() != _mentioned->find(stringify((*c)->prefix()) + ":*"))
                        continue;

                    for (auto v((*c)->begin()), v_end((*c)->end()) ; v != v_end ; ++v)
                        if ((*v)->presumed() && ! (*v)->enabled())
                        {
                            if (_mentioned->end() != _mentioned->find(stringify((*v)->name_with_prefix())))
                                continue;

                            ++n;
                            if (! p.empty())
                                p.append("', '");
                            p.append(stringify((*v)->name_with_prefix()));
                        }
                }

                if (n > 1)
                    return std::make_pair(false, "Flags '" + p + "' enabled (presumed)");
                else if (n == 1)
                    return std::make_pair(false, "Flag '" + p + "' enabled (presumed)");
                else
                    return std::make_pair(true, as_human_string(id));
            }

            virtual Tribool accumulate_changes_to_make_met(
                    const Environment * const env,
                    const ChangedChoices * const maybe_changes_to_owner,
                    const std::shared_ptr<const PackageID> & id,
                    const std::shared_ptr<const PackageID> & spec_id,
                    ChangedChoices &) const PALUDIS_ATTRIBUTE((warn_unused_result))
            {
                if (requirement_met(env, maybe_changes_to_owner, id, spec_id, 0).first)
                    return indeterminate;
                else
                    return false;
            }

            virtual const std::string as_human_string(
                    const std::shared_ptr<const PackageID> &) const PALUDIS_ATTRIBUTE((warn_unused_result))
            {
                return "Remaining presumed flags enabled";
            }

            virtual const std::string as_raw_string() const PALUDIS_ATTRIBUTE((warn_unused_result))
            {
                return "";
            }
    };
}

std::shared_ptr<const AdditionalPackageDepSpecRequirement>
paludis::make_elike_presumed_choices_requirement(
        const std::shared_ptr<const Set<std::string> > mentioned)
{
    return std::make_shared<ELikePresumedChoicesRequirement>(mentioned);
}

