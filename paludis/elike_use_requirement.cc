/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010 Ciaran McCreesh
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
#include <paludis/util/make_shared_ptr.hh>
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
#include <vector>
#include <functional>
#include <algorithm>

using namespace paludis;

namespace
{
    bool icky_use_query(const ChoiceNameWithPrefix & f, const PackageID & id, Tribool default_value = Tribool(indeterminate))
    {
        if (! id.choices_key())
        {
            Log::get_instance()->message("elike_use_requirement.query", ll_warning, lc_context) <<
                "ID '" << id << "' has no choices, so couldn't get the state of flag '" << f << "'";
            return false;
        }

        const std::tr1::shared_ptr<const ChoiceValue> v(id.choices_key()->value()->find_by_name_with_prefix(f));
        if (v)
            return v->enabled();

        if (default_value.is_indeterminate() && ! id.choices_key()->value()->has_matching_contains_every_value_prefix(f))
            Log::get_instance()->message("elike_use_requirement.query", ll_warning, lc_context) <<
                "ID '" << id << "' has no flag named '" << f << "'";
        return default_value.is_true();
    }

    class UseRequirement
    {
        private:
            const std::string _flags;
            const std::tr1::shared_ptr<const PackageID> _id;
            const ELikeUseRequirementOptions _options;
            const Tribool _default_value;

        public:
            UseRequirement(const std::string & n, const std::tr1::shared_ptr<const PackageID> & i,
                    const ELikeUseRequirementOptions & o, Tribool d) :
                _flags(n),
                _id(i),
                _options(o),
                _default_value(d)
            {
            }

            virtual ~UseRequirement()
            {
            }

            virtual bool one_requirement_met(
                    const Environment * const,
                    const ChoiceNameWithPrefix &,
                    const PackageID &) const PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            virtual const std::string as_human_string() const PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            const std::string flags() const PALUDIS_ATTRIBUTE((warn_unused_result))
            {
                return _flags;
            }

            const std::tr1::shared_ptr<const PackageID> package_id() const PALUDIS_ATTRIBUTE((warn_unused_result))
            {
                return _id;
            }

            bool requirement_met(const Environment * const env, const PackageID & i) const
            {
                if (_flags.length() >= 2 && ":*" == _flags.substr(_flags.length() - 2))
                {
                    if ((! _options[euro_allow_self_deps]) || (! _id) || (! _id->choices_key()))
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
                    Choices::ConstIterator cc(_id->choices_key()->value()->find(prefix));
                    if (cc == _id->choices_key()->value()->end())
                        Log::get_instance()->message("elike_use_requirement.prefix_star_unmatching", ll_warning, lc_context) <<
                            "ID '" << *_id << "' uses requirement '" << _flags << "' but has no choice prefix '" << prefix << "'";
                    else
                    {
                        for (Choice::ConstIterator v((*cc)->begin()), v_end((*cc)->end()) ;
                                v != v_end ; ++v)
                            if (! one_requirement_met(env, (*v)->name_with_prefix(), i))
                                return false;
                    }
                }
                else
                    if (! one_requirement_met(env, ChoiceNameWithPrefix(_flags), i))
                        return false;

                return true;
            }

            const Tribool default_value() const PALUDIS_ATTRIBUTE((warn_unused_result))
            {
                return _default_value;
            }

            const std::string default_value_human_string_fragment() const PALUDIS_ATTRIBUTE((warn_unused_result))
            {
                if (_default_value.is_true())
                    return ", assuming enabled if missing";
                else if (_default_value.is_false())
                    return ", assuming disabled if missing";
                else
                    return "";
            }
    };

    class PALUDIS_VISIBLE EnabledUseRequirement :
        public UseRequirement
    {
        public:
            EnabledUseRequirement(const std::string & n,
                    const std::tr1::shared_ptr<const PackageID> & i,
                    const ELikeUseRequirementOptions & o,
                    Tribool d) :
                UseRequirement(n, i, o, d)
            {
            }

            virtual bool one_requirement_met(const Environment * const, const ChoiceNameWithPrefix & flag,
                    const PackageID & pkg) const
            {
                return icky_use_query(flag, pkg, default_value());
            }

            virtual const std::string as_human_string() const
            {
                return "Flag '" + stringify(flags()) + "' enabled" + default_value_human_string_fragment();
            }
    };

    class PALUDIS_VISIBLE DisabledUseRequirement :
        public UseRequirement
    {
        public:
            DisabledUseRequirement(const std::string & n,
                    const std::tr1::shared_ptr<const PackageID> & i,
                    const ELikeUseRequirementOptions & o,
                    Tribool d) :
                UseRequirement(n, i, o, d)
            {
            }

            virtual bool one_requirement_met(const Environment * const, const ChoiceNameWithPrefix & flag,
                    const PackageID & pkg) const
            {
                return ! icky_use_query(flag, pkg, default_value());
            }

            virtual const std::string as_human_string() const
            {
                return "Flag '" + stringify(flags()) + "' disabled" + default_value_human_string_fragment();
            }
    };

    class PALUDIS_VISIBLE ConditionalUseRequirement :
        public UseRequirement
    {
        public:
            ConditionalUseRequirement(const std::string & n,
                    const std::tr1::shared_ptr<const PackageID> & i,
                    const ELikeUseRequirementOptions & o,
                    Tribool d) :
                UseRequirement(n, i, o, d)
            {
            }
    };

    class PALUDIS_VISIBLE IfMineThenUseRequirement :
        public ConditionalUseRequirement
    {
        public:
            IfMineThenUseRequirement(
                    const std::string & n,
                    const std::tr1::shared_ptr<const PackageID> & i,
                    const ELikeUseRequirementOptions & o,
                    Tribool d) :
                ConditionalUseRequirement(n, i, o, d)
            {
            }

            virtual bool one_requirement_met(const Environment * const, const ChoiceNameWithPrefix & flag,
                    const PackageID & pkg) const
            {
                return ! icky_use_query(flag, *package_id()) || icky_use_query(flag, pkg, default_value());
            }

            virtual const std::string as_human_string() const
            {
                return "Flag '" + stringify(flags()) + "' enabled if it is enabled for '"
                    + stringify(*package_id()) + "'" + default_value_human_string_fragment();
            }
    };

    class PALUDIS_VISIBLE IfNotMineThenUseRequirement :
        public ConditionalUseRequirement
    {
        public:
            IfNotMineThenUseRequirement(const std::string & n,
                    const std::tr1::shared_ptr<const PackageID> & i,
                    const ELikeUseRequirementOptions & o,
                    Tribool d) :
                ConditionalUseRequirement(n, i, o, d)
            {
            }

            virtual bool one_requirement_met(const Environment * const, const ChoiceNameWithPrefix & flag,
                    const PackageID & pkg) const
            {
                return icky_use_query(flag, *package_id()) || icky_use_query(flag, pkg, default_value());
            }

            virtual const std::string as_human_string() const
            {
                return "Flag '" + stringify(flags()) + "' enabled if it is disabled for '" +
                    stringify(*package_id()) + "'" + default_value_human_string_fragment();
            }
    };

    class PALUDIS_VISIBLE IfMineThenNotUseRequirement :
        public ConditionalUseRequirement
    {
        public:
            IfMineThenNotUseRequirement(const std::string & n,
                    const std::tr1::shared_ptr<const PackageID> & i,
                    const ELikeUseRequirementOptions & o,
                    Tribool d) :
                ConditionalUseRequirement(n, i, o, d)
            {
            }

            virtual bool one_requirement_met(const Environment * const, const ChoiceNameWithPrefix & flag,
                    const PackageID & pkg) const
            {
                return ! icky_use_query(flag, *package_id()) || ! icky_use_query(flag, pkg, default_value());
            }

            virtual const std::string as_human_string() const
            {
                return "Flag '" + stringify(flags()) + "' disabled if it is enabled for '" +
                    stringify(*package_id()) + "'" + default_value_human_string_fragment();
            }
    };

    class PALUDIS_VISIBLE IfNotMineThenNotUseRequirement :
        public ConditionalUseRequirement
    {
        public:
            IfNotMineThenNotUseRequirement(const std::string & n,
                    const std::tr1::shared_ptr<const PackageID> & i,
                    const ELikeUseRequirementOptions & o,
                    Tribool d) :
                ConditionalUseRequirement(n, i, o, d)
            {
            }

            virtual bool one_requirement_met(const Environment * const, const ChoiceNameWithPrefix & flag,
                    const PackageID & pkg) const
            {
                return icky_use_query(flag, *package_id()) || ! icky_use_query(flag, pkg, default_value());
            }

            virtual const std::string as_human_string() const
            {
                return "Flag '" + stringify(flags()) + "' disabled if it is disabled for '" +
                    stringify(*package_id()) + "'" + default_value_human_string_fragment();
            }
    };

    class PALUDIS_VISIBLE EqualUseRequirement :
        public ConditionalUseRequirement
    {
        public:
            EqualUseRequirement(const std::string & n,
                    const std::tr1::shared_ptr<const PackageID> & i,
                    const ELikeUseRequirementOptions & o,
                    Tribool d) :
                ConditionalUseRequirement(n, i, o, d)
            {
            }

            virtual bool one_requirement_met(const Environment * const, const ChoiceNameWithPrefix & flag,
                    const PackageID & pkg) const
            {
                return icky_use_query(flag, pkg, default_value()) == icky_use_query(flag, *package_id());
            }

            virtual const std::string as_human_string() const
            {
                return "Flag '" + stringify(flags()) + "' enabled or disabled like it is for '"
                    + stringify(*package_id()) + "'" + default_value_human_string_fragment();
            }
    };

    class PALUDIS_VISIBLE NotEqualUseRequirement :
        public ConditionalUseRequirement
    {
        public:
            NotEqualUseRequirement(const std::string & n,
                    const std::tr1::shared_ptr<const PackageID> & i,
                    const ELikeUseRequirementOptions & o,
                    Tribool d) :
                ConditionalUseRequirement(n, i, o, d)
            {
            }

            virtual bool one_requirement_met(const Environment * const, const ChoiceNameWithPrefix & flag,
                    const PackageID & pkg) const
            {
                return icky_use_query(flag, pkg, default_value()) != icky_use_query(flag, *package_id());
            }

            virtual const std::string as_human_string() const
            {
                return "Flag '" + stringify(flags()) + "' enabled or disabled opposite to how it is for '"
                    + stringify(*package_id()) + "'" + default_value_human_string_fragment();
            }
    };

    class UseRequirements :
        public AdditionalPackageDepSpecRequirement
    {
        private:
            std::string _raw;
            std::vector<std::tr1::shared_ptr<const UseRequirement> > _reqs;

        public:
            UseRequirements(const std::string & r) :
                _raw(r)
            {
            }

            virtual const std::pair<bool, std::string> requirement_met(const Environment * const env, const PackageID & id) const
            {
                using namespace std::tr1::placeholders;
                return std::make_pair(
                        _reqs.end() == std::find_if(_reqs.begin(), _reqs.end(), std::tr1::bind(
                                std::logical_not<bool>(), std::tr1::bind(
                                    &UseRequirement::requirement_met, _1, env, std::tr1::cref(id)))),
                        as_human_string());
            }

            virtual const std::string as_human_string() const
            {
                return join(_reqs.begin(), _reqs.end(), "; ", std::tr1::mem_fn(&UseRequirement::as_human_string));
            }

            virtual const std::string as_raw_string() const
            {
                return _raw;
            }

            void add_requirement(const std::tr1::shared_ptr<const UseRequirement> & req)
            {
                _reqs.push_back(req);
            }
    };

    template <typename T_>
    std::tr1::shared_ptr<const UseRequirement>
    make_requirement(const std::string & n, const std::tr1::shared_ptr<const PackageID > & i,
            const ELikeUseRequirementOptions & o, Tribool d)
    {
        return make_shared_ptr(new T_(n, i, o, d));
    }

    typedef std::tr1::shared_ptr<const UseRequirement> (* Factory)(
            const std::string &, const std::tr1::shared_ptr<const PackageID> &,
            const ELikeUseRequirementOptions &, Tribool);

    void
    parse_flag(
            const std::tr1::shared_ptr<UseRequirements> & result,
            const Factory & factory,
            const std::string & c,
            const std::tr1::shared_ptr<const PackageID> & id,
            Tribool d,
            const ELikeUseRequirementOptions & options)
    {
        result->add_requirement(factory(c, id, options, d));
    }

    void
    parse_one_use_requirement(
            const std::tr1::shared_ptr<UseRequirements> & result,
            const std::string & s, std::string & flag,
            const std::tr1::shared_ptr<const PackageID> & id, const ELikeUseRequirementOptions & options)
    {
        Factory factory;

        if (flag.empty())
            throw ELikeUseRequirementError(s, "Invalid [] contents");

        if ('=' == flag.at(flag.length() - 1))
        {
            if ((! options[euro_allow_self_deps]) || (! id))
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
            if ((! options[euro_allow_self_deps]) || (! id))
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
                parse_flag(result, factory, flag, id, Tribool(true), options);
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
                parse_flag(result, factory, flag, id, Tribool(false), options);
            }
            else
                throw ELikeUseRequirementError(s, "Invalid [] contents");
        }
        else
            parse_flag(result, factory, flag, id, Tribool(indeterminate), options);
    }
}

ELikeUseRequirementError::ELikeUseRequirementError(const std::string & s, const std::string & m) throw () :
    Exception("Error parsing use requirement '" + s + "': " + m)
{
}

std::tr1::shared_ptr<const AdditionalPackageDepSpecRequirement>
paludis::parse_elike_use_requirement(const std::string & s,
        const std::tr1::shared_ptr<const PackageID> & id, const ELikeUseRequirementOptions & options)
{
    Context context("When parsing use requirement '" + s + "':");

    std::tr1::shared_ptr<UseRequirements> result(new UseRequirements("[" + s + "]"));
    std::string::size_type pos(0);
    for (;;)
    {
        std::string::size_type comma(s.find(',', pos));
        std::string flag(s.substr(pos, std::string::npos == comma ? comma : comma - pos));
        parse_one_use_requirement(result, s, flag, id, options);
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

