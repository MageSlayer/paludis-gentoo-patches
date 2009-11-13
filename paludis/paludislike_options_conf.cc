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

#include <paludis/paludislike_options_conf.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/validated.hh>
#include <paludis/util/named_value.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/config_file.hh>
#include <paludis/util/options.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/log.hh>
#include <paludis/util/iterator_funcs.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/set.hh>
#include <paludis/util/active_object_ptr.hh>
#include <paludis/util/deferred_construction_ptr.hh>
#include <paludis/choice.hh>
#include <paludis/dep_spec.hh>
#include <paludis/name.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/match_package.hh>
#include <paludis/package_id.hh>
#include <paludis/environment.hh>
#include <paludis/spec_tree.hh>
#include <tr1/unordered_map>
#include <tr1/unordered_set>
#include <list>
#include <vector>
#include <algorithm>

using namespace paludis;

/*
 * Syntax:
 *
 *     spec|set        foo -bar -* baz=value
 *     spec|set        blah: foo -bar -* baz=value
 *
 * Lines in the form 'spec foo blah: baz' are treated as two lines, 'spec foo'
 * and 'spec blah: baz'.
 *
 *  Specific cat/pkg specs have highest priority, followed by named sets,
 *  followed by wildcards.
 */

namespace paludis
{
    namespace n
    {
        struct equals_value;
        struct minus;
        struct minus_star;
        struct prefix;
        struct set_name;
        struct set_value;
        struct spec;
        struct unprefixed_name;
        struct values;
        struct values_groups;
    }
}

namespace
{
    struct Value
    {
        NamedValue<n::equals_value, std::string> equals_value;
        NamedValue<n::minus, bool> minus;
        NamedValue<n::unprefixed_name, UnprefixedChoiceName> unprefixed_name;

        bool operator== (const Value & other) const
        {
            return unprefixed_name() == other.unprefixed_name();
        }

        std::size_t hash() const
        {
            return Hash<UnprefixedChoiceName>()(unprefixed_name());
        }
    };

    typedef std::tr1::unordered_multiset<Value, Hash<Value> > Values;

    struct ValuesGroup
    {
        NamedValue<n::minus_star, bool> minus_star;
        NamedValue<n::prefix, ChoicePrefixName> prefix;
        NamedValue<n::values, Values> values;
    };

    typedef std::list<ValuesGroup> ValuesGroups;

    struct SpecWithValuesGroups
    {
        NamedValue<n::spec, PackageDepSpec> spec;
        NamedValue<n::values_groups, ValuesGroups> values_groups;
    };

    typedef std::list<SpecWithValuesGroups> SpecsWithValuesGroups;

    struct SetNameWithValuesGroups
    {
        NamedValue<n::set_name, SetName> set_name;
        NamedValue<n::set_value, ActiveObjectPtr<DeferredConstructionPtr<std::tr1::shared_ptr<const SetSpecTree> > > > set_value;
        NamedValue<n::values_groups, ValuesGroups> values_groups;
    };

    typedef std::list<SetNameWithValuesGroups> SetNamesWithValuesGroups;

    typedef std::tr1::unordered_map<QualifiedPackageName, SpecsWithValuesGroups, Hash<QualifiedPackageName> > SpecificSpecs;

    const std::tr1::shared_ptr<const SetSpecTree> make_set_value(
            const Environment * const env,
            const FSEntry from,
            const SetName name)
    {
        std::tr1::shared_ptr<const SetSpecTree> result(env->set(name));
        if (! result)
        {
            Log::get_instance()->message("paludislike_options_conf.bad_set", ll_warning, lc_context)
                << "Set '" << name << "' in '" << from << "' does not exist";
            result.reset(new SetSpecTree(make_shared_ptr(new AllDepSpec)));
        }

        return result;
    }
}

namespace paludis
{
    template <>
    struct Implementation<PaludisLikeOptionsConf>
    {
        const PaludisLikeOptionsConfParams params;

        SpecificSpecs specific_specs;
        SetNamesWithValuesGroups set_specs;
        SpecsWithValuesGroups wildcard_specs;

        Implementation(const PaludisLikeOptionsConfParams & p) :
            params(p)
        {
        }
    };
}

PaludisLikeOptionsConf::PaludisLikeOptionsConf(const PaludisLikeOptionsConfParams & params) :
    PrivateImplementationPattern<PaludisLikeOptionsConf>(new Implementation<PaludisLikeOptionsConf>(params))
{
}

PaludisLikeOptionsConf::~PaludisLikeOptionsConf()
{
}

void
PaludisLikeOptionsConf::add_file(const FSEntry & f)
{
    Context context("When adding '" + stringify(f) + "':");

    const std::tr1::shared_ptr<const LineConfigFile> file(_imp->params.make_config_file()(f, LineConfigFileOptions()));
    if (! file)
        return;

    for (LineConfigFile::ConstIterator line(file->begin()), line_end(file->end()) ;
            line != line_end ; ++line)
    {
        std::vector<std::string> tokens;
        tokenise_whitespace_quoted(*line, std::back_inserter(tokens));

        if (tokens.size() < 2)
            continue;

        ValuesGroups * values_groups(0);
        try
        {
            std::tr1::shared_ptr<PackageDepSpec> d(new PackageDepSpec(parse_user_package_dep_spec(
                            tokens.at(0), _imp->params.environment(),
                            UserPackageDepSpecOptions() + updso_allow_wildcards + updso_no_disambiguation + updso_throw_if_set)));

            if (d->additional_requirements_ptr())
            {
                Log::get_instance()->message("paludislike_options_conf.bad_spec", ll_warning, lc_context)
                    << "Dependency specification '" << stringify(*d)
                    << "' includes use requirements, which cannot be used in '" << f << "'";
                continue;
            }

            if (d->package_ptr())
            {
                SpecificSpecs::iterator i(_imp->specific_specs.insert(std::make_pair(
                                *d->package_ptr(),
                                SpecsWithValuesGroups())).first);
                values_groups = &i->second.insert(i->second.end(),
                        make_named_values<SpecWithValuesGroups>(
                            value_for<n::spec>(*d),
                            value_for<n::values_groups>(ValuesGroups())
                            ))->values_groups();
            }
            else
            {
                values_groups = &_imp->wildcard_specs.insert(_imp->wildcard_specs.end(),
                        make_named_values<SpecWithValuesGroups>(
                            value_for<n::spec>(*d),
                            value_for<n::values_groups>(ValuesGroups())
                            ))->values_groups();
            }
        }
        catch (const GotASetNotAPackageDepSpec &)
        {
            SetName n(tokens.at(0));

            values_groups = &_imp->set_specs.insert(_imp->set_specs.end(),
                    make_named_values<SetNameWithValuesGroups>(
                        value_for<n::set_name>(n),
                        value_for<n::set_value>(DeferredConstructionPtr<std::tr1::shared_ptr<const SetSpecTree> >(
                                std::tr1::bind(&make_set_value, _imp->params.environment(), f, n))),
                        value_for<n::values_groups>(ValuesGroups())
                        ))->values_groups();
        }

        if (! values_groups)
            throw InternalError(PALUDIS_HERE, "huh?");

        ValuesGroup * values_group(0);

        for (std::vector<std::string>::const_iterator t(next(tokens.begin())), t_end(tokens.end()) ;
                t != t_end ; ++t)
        {
            if (t->empty())
                continue;

            if (':' == t->at(t->length() - 1))
            {
                /* foo: */
                std::string p;
                std::transform(t->begin(), previous(t->end()), std::back_inserter(p), &::tolower);

                values_group = &*values_groups->insert(values_groups->end(), make_named_values<ValuesGroup>(
                            value_for<n::minus_star>(false),
                            value_for<n::prefix>(ChoicePrefixName(p)),
                            value_for<n::values>(Values())
                            ));
            }
            else
            {
                /* if we don't have a foo: active, make one */
                if (! values_group)
                    values_group = &*values_groups->insert(values_groups->end(), make_named_values<ValuesGroup>(
                                value_for<n::minus_star>(false),
                                value_for<n::prefix>(ChoicePrefixName("")),
                                value_for<n::values>(Values())
                                ));

                if ("-*" == *t)
                {
                    /* -* */
                    values_group->minus_star() = true;
                }
                else if ('-' == t->at(0))
                {
                    /* -bar */
                    values_group->values().insert(make_named_values<Value>(
                                value_for<n::equals_value>(""),
                                value_for<n::minus>(true),
                                value_for<n::unprefixed_name>(UnprefixedChoiceName(t->substr(1)))
                                ));
                }
                else
                {
                    std::string::size_type equals_p(t->find('='));
                    if (std::string::npos == equals_p)
                    {
                        /* foo */
                        values_group->values().insert(make_named_values<Value>(
                                    value_for<n::equals_value>(""),
                                    value_for<n::minus>(false),
                                    value_for<n::unprefixed_name>(UnprefixedChoiceName(*t))
                                    ));
                    }
                    else
                    {
                        /* foo=blah */
                        values_group->values().insert(make_named_values<Value>(
                                    value_for<n::equals_value>(t->substr(equals_p + 1)),
                                    value_for<n::minus>(false),
                                    value_for<n::unprefixed_name>(UnprefixedChoiceName(t->substr(0, equals_p)))
                                    ));
                    }
                }
            }
        }
    }
}

namespace
{
    void check_values_groups(
            const Environment * const,
            const std::tr1::shared_ptr<const PackageID> &,
            const std::tr1::shared_ptr<const Choice> & choice,
            const UnprefixedChoiceName & unprefixed_name,
            const ValuesGroups & values_groups,
            bool & seen_minus_star,
            Tribool & result_state,
            std::string & result_value)
    {

        for (ValuesGroups::const_iterator i(values_groups.begin()), i_end(values_groups.end()) ;
                i != i_end ; ++i)
        {
            if (i->prefix() != choice->prefix())
                continue;

            seen_minus_star = seen_minus_star || i->minus_star();

            std::pair<Values::const_iterator, Values::const_iterator> range(i->values().equal_range(
                        make_named_values<Value>(
                            value_for<n::equals_value>(""),
                            value_for<n::minus>(false),
                            value_for<n::unprefixed_name>(unprefixed_name)
                            )));
            for ( ; range.first != range.second ; ++range.first)
            {
                if (range.first->minus())
                    result_state = false;
                else
                    result_state = true;

                if (! (range.first->equals_value().empty()))
                    result_value = range.first->equals_value();
            }
        }
    }

    void collect_known_from_values_groups(
            const Environment * const,
            const std::tr1::shared_ptr<const PackageID> &,
            const std::tr1::shared_ptr<const Choice> & choice,
            const ValuesGroups & values_groups,
            const std::tr1::shared_ptr<Set<UnprefixedChoiceName> > & known)
    {

        for (ValuesGroups::const_iterator i(values_groups.begin()), i_end(values_groups.end()) ;
                i != i_end ; ++i)
        {
            if (i->prefix() != choice->prefix())
                continue;

            for (Values::const_iterator v(i->values().begin()), v_end(i->values().end()) ;
                    v != v_end ; ++v)
                known->insert(v->unprefixed_name());
        }
    }

    void check_specs_with_values_groups(
            const Environment * const env,
            const std::tr1::shared_ptr<const PackageID> & id,
            const std::tr1::shared_ptr<const Choice> & choice,
            const UnprefixedChoiceName & unprefixed_name,
            const SpecsWithValuesGroups & specs_with_values_groups,
            bool & seen_minus_star,
            Tribool & result_state,
            std::string & result_value)
    {
        for (SpecsWithValuesGroups::const_iterator i(specs_with_values_groups.begin()),
                i_end(specs_with_values_groups.end()) ;
                i != i_end ; ++i)
        {
            if (! match_package(*env, i->spec(), *id, MatchPackageOptions()))
                continue;

            check_values_groups(env, id, choice, unprefixed_name, i->values_groups(),
                    seen_minus_star, result_state, result_value);
        }
    }

    void collect_known_from_specs_with_values_groups(
            const Environment * const env,
            const std::tr1::shared_ptr<const PackageID> & id,
            const std::tr1::shared_ptr<const Choice> & choice,
            const SpecsWithValuesGroups & specs_with_values_groups,
            const std::tr1::shared_ptr<Set<UnprefixedChoiceName> > & known)
    {
        for (SpecsWithValuesGroups::const_iterator i(specs_with_values_groups.begin()),
                i_end(specs_with_values_groups.end()) ;
                i != i_end ; ++i)
        {
            if (! match_package(*env, i->spec(), *id, MatchPackageOptions()))
                continue;

            collect_known_from_values_groups(env, id, choice, i->values_groups(), known);
        }
    }
}

const Tribool
PaludisLikeOptionsConf::want_choice_enabled(
        const std::tr1::shared_ptr<const PackageID> & id,
        const std::tr1::shared_ptr<const Choice> & choice,
        const UnprefixedChoiceName & unprefixed_name
        ) const
{
    Context context("When checking state of flag prefix '" + stringify(choice->prefix()) +
            "' name '" + stringify(unprefixed_name) + "' for '" + stringify(*id) + "':");

    bool seen_minus_star(false);
    Tribool result(indeterminate);
    std::string dummy;

    /* Any specific matches? */
    {
        SpecificSpecs::const_iterator i(_imp->specific_specs.find(id->name()));
        if (i != _imp->specific_specs.end())
        {
            check_specs_with_values_groups(_imp->params.environment(), id, choice, unprefixed_name, i->second,
                    seen_minus_star, result, dummy);
            if (result.is_true())
                return true;
            else if (result.is_false())
                return false;
        }
    }

    /* Any set matches? */
    {
        for (SetNamesWithValuesGroups::const_iterator r(_imp->set_specs.begin()), r_end(_imp->set_specs.end()) ;
                r != r_end ; ++r)
        {
            if (! match_package_in_set(*_imp->params.environment(), *r->set_value().value().value(), *id, MatchPackageOptions()))
                continue;

            check_values_groups(_imp->params.environment(), id, choice, unprefixed_name, r->values_groups(),
                    seen_minus_star, result, dummy);
        }

        if (result.is_true())
            return true;
        else if (result.is_false())
            return false;
    }

    /* Wildcards? */
    {
        check_specs_with_values_groups(_imp->params.environment(), id, choice, unprefixed_name, _imp->wildcard_specs,
                seen_minus_star, result, dummy);

        if (result.is_true())
            return true;
        else if (result.is_false())
            return false;
    }

    if (seen_minus_star)
        return Tribool(false);
    else
        return indeterminate;
}

const std::string
PaludisLikeOptionsConf::value_for_choice_parameter(
        const std::tr1::shared_ptr<const PackageID> & id,
        const std::tr1::shared_ptr<const Choice> & choice,
        const UnprefixedChoiceName & unprefixed_name
        ) const
{
    Context context("When checking value for flag prefix '" + stringify(choice->prefix()) +
            "' name '" + stringify(unprefixed_name) + "' for '" + stringify(*id) + "':");

    bool dummy_seen_minus_star(false);
    Tribool dummy_result;
    std::string equals_value;

    /* Any specific matches? */
    {
        SpecificSpecs::const_iterator i(_imp->specific_specs.find(id->name()));
        if (i != _imp->specific_specs.end())
        {
            check_specs_with_values_groups(_imp->params.environment(), id, choice, unprefixed_name, i->second,
                    dummy_seen_minus_star, dummy_result, equals_value);

            if (! equals_value.empty())
                return equals_value;
        }
    }

    /* Any set matches? */
    {
        for (SetNamesWithValuesGroups::const_iterator r(_imp->set_specs.begin()), r_end(_imp->set_specs.end()) ;
                r != r_end ; ++r)
        {
            if (! match_package_in_set(*_imp->params.environment(), *r->set_value().value().value(), *id, MatchPackageOptions()))
                continue;

            check_values_groups(_imp->params.environment(), id, choice, unprefixed_name, r->values_groups(),
                    dummy_seen_minus_star, dummy_result, equals_value);
        }

        if (! equals_value.empty())
            return equals_value;
    }

    /* Wildcards? */
    {
        check_specs_with_values_groups(_imp->params.environment(), id, choice, unprefixed_name, _imp->wildcard_specs,
                dummy_seen_minus_star, dummy_result, equals_value);

        if (! equals_value.empty())
            return equals_value;
    }

    return "";
}

const std::tr1::shared_ptr<const Set<UnprefixedChoiceName> >
PaludisLikeOptionsConf::known_choice_value_names(
        const std::tr1::shared_ptr<const PackageID> & id,
        const std::tr1::shared_ptr<const Choice> & choice
        ) const
{
    const std::tr1::shared_ptr<Set<UnprefixedChoiceName> > result(new Set<UnprefixedChoiceName>);

    /* Any specific matches? */
    {
        SpecificSpecs::const_iterator i(_imp->specific_specs.find(id->name()));
        if (i != _imp->specific_specs.end())
            collect_known_from_specs_with_values_groups(_imp->params.environment(), id, choice, i->second, result);
    }

    /* Any set matches? */
    {
        for (SetNamesWithValuesGroups::const_iterator r(_imp->set_specs.begin()), r_end(_imp->set_specs.end()) ;
                r != r_end ; ++r)
        {
            if (! match_package_in_set(*_imp->params.environment(), *r->set_value().value().value(), *id, MatchPackageOptions()))
                continue;

            collect_known_from_values_groups(_imp->params.environment(), id, choice, r->values_groups(), result);
        }
    }

    /* Wildcards? */
    {
        collect_known_from_specs_with_values_groups(_imp->params.environment(), id, choice, _imp->wildcard_specs, result);
    }

    return result;
}

template class PrivateImplementationPattern<PaludisLikeOptionsConf>;

