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

#include <paludis/paludislike_options_conf.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/hashes.hh>
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
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/choice.hh>
#include <paludis/dep_spec.hh>
#include <paludis/name.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/match_package.hh>
#include <paludis/package_id.hh>
#include <paludis/environment.hh>
#include <paludis/spec_tree.hh>
#include <paludis/package_dep_spec_properties.hh>
#include <unordered_map>
#include <unordered_set>
#include <list>
#include <vector>
#include <algorithm>

using namespace paludis;

/*
 * Syntax:
 *
 *     spec|set        foo -bar -* baz=value (forced) (-masked)
 *     spec|set        blah: foo -bar -* baz=value (forced) (-masked)
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
        typedef Name<struct name_equals_value> equals_value;
        typedef Name<struct name_locked> locked;
        typedef Name<struct name_minus> minus;
        typedef Name<struct name_minus_star> minus_star;
        typedef Name<struct name_prefix> prefix;
        typedef Name<struct name_set_name> set_name;
        typedef Name<struct name_set_value> set_value;
        typedef Name<struct name_spec> spec;
        typedef Name<struct name_unprefixed_name> unprefixed_name;
        typedef Name<struct name_values> values;
        typedef Name<struct name_values_groups> values_groups;
    }
}

namespace
{
    struct Value
    {
        NamedValue<n::equals_value, std::string> equals_value;
        NamedValue<n::locked, bool> locked;
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

    typedef std::unordered_multiset<Value, Hash<Value> > Values;

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
        NamedValue<n::set_value, ActiveObjectPtr<DeferredConstructionPtr<std::shared_ptr<const SetSpecTree> > > > set_value;
        NamedValue<n::values_groups, ValuesGroups> values_groups;
    };

    typedef std::list<SetNameWithValuesGroups> SetNamesWithValuesGroups;

    typedef std::unordered_map<QualifiedPackageName, SpecsWithValuesGroups, Hash<QualifiedPackageName> > SpecificSpecs;

    const std::shared_ptr<const SetSpecTree> make_set_value(
            const Environment * const env,
            const FSPath & from,
            const SetName name)
    {
        std::shared_ptr<const SetSpecTree> result(env->set(name));
        if (! result)
        {
            Log::get_instance()->message("paludislike_options_conf.bad_set", ll_warning, lc_context)
                << "Set '" << name << "' in '" << from << "' does not exist";
            result = std::make_shared<SetSpecTree>(std::make_shared<AllDepSpec>());
        }

        return result;
    }
}

namespace paludis
{
    template <>
    struct Imp<PaludisLikeOptionsConf>
    {
        const PaludisLikeOptionsConfParams params;

        SpecificSpecs specific_specs;
        SetNamesWithValuesGroups set_specs;
        SpecsWithValuesGroups wildcard_specs;

        Imp(const PaludisLikeOptionsConfParams & p) :
            params(p)
        {
        }
    };
}

PaludisLikeOptionsConf::PaludisLikeOptionsConf(const PaludisLikeOptionsConfParams & params) :
    Pimp<PaludisLikeOptionsConf>(params)
{
}

PaludisLikeOptionsConf::~PaludisLikeOptionsConf()
{
}

void
PaludisLikeOptionsConf::add_file(const FSPath & f)
{
    Context context("When adding '" + stringify(f) + "':");

    const std::shared_ptr<const LineConfigFile> file(_imp->params.make_config_file()(f, { }));
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
            std::shared_ptr<PackageDepSpec> d(std::make_shared<PackageDepSpec>(parse_user_package_dep_spec(
                            tokens.at(0), _imp->params.environment(),
                            { updso_allow_wildcards, updso_no_disambiguation, updso_throw_if_set })));

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
                            n::spec() = *d,
                            n::values_groups() = ValuesGroups()
                            ))->values_groups();
            }
            else
            {
                values_groups = &_imp->wildcard_specs.insert(_imp->wildcard_specs.end(),
                        make_named_values<SpecWithValuesGroups>(
                            n::spec() = *d,
                            n::values_groups() = ValuesGroups()
                            ))->values_groups();
            }
        }
        catch (const GotASetNotAPackageDepSpec &)
        {
            SetName n(tokens.at(0));

            values_groups = &_imp->set_specs.insert(_imp->set_specs.end(),
                    make_named_values<SetNameWithValuesGroups>(
                        n::set_name() = n,
                        n::set_value() = DeferredConstructionPtr<std::shared_ptr<const SetSpecTree> >(
                                std::bind(&make_set_value, _imp->params.environment(), f, n)),
                        n::values_groups() = ValuesGroups()
                        ))->values_groups();
        }

        if (! values_groups)
            throw InternalError(PALUDIS_HERE, "huh?");

        ValuesGroup * values_group(0);

        for (std::vector<std::string>::iterator t(next(tokens.begin())), t_end(tokens.end()) ;
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
                            n::minus_star() = false,
                            n::prefix() = ChoicePrefixName(p),
                            n::values() = Values()
                            ));
            }
            else
            {
                /* if we don't have a foo: active, make one */
                if (! values_group)
                    values_group = &*values_groups->insert(values_groups->end(), make_named_values<ValuesGroup>(
                                n::minus_star() = false,
                                n::prefix() = ChoicePrefixName(""),
                                n::values() = Values()
                                ));

                if ("-*" == *t)
                {
                    /* -* */
                    values_group->minus_star() = true;
                }
                else
                {
                    /* (something) is legal */
                    bool locked(false);
                    if ('(' == t->at(0) && ')' == t->at(t->length() - 1))
                    {
                        if (! _imp->params.allow_locking())
                            throw ConfigurationError("Locking on '" + *t + "' not allowed in '" + stringify(f) + "'");

                        locked = true;
                        *t = t->substr(1, t->length() - 2);
                    }

                    if ('-' == t->at(0))
                    {
                        /* -bar */
                        values_group->values().insert(make_named_values<Value>(
                                    n::equals_value() = "",
                                    n::locked() = locked,
                                    n::minus() = true,
                                    n::unprefixed_name() = UnprefixedChoiceName(t->substr(1))
                                    ));
                    }
                    else
                    {
                        std::string::size_type equals_p(t->find('='));
                        if (std::string::npos == equals_p)
                        {
                            /* foo */
                            values_group->values().insert(make_named_values<Value>(
                                        n::equals_value() = "",
                                        n::locked() = locked,
                                        n::minus() = false,
                                        n::unprefixed_name() = UnprefixedChoiceName(*t)
                                        ));
                        }
                        else
                        {
                            /* foo=blah */
                            values_group->values().insert(make_named_values<Value>(
                                        n::equals_value() = t->substr(equals_p + 1),
                                        n::locked() = locked,
                                        n::minus() = false,
                                        n::unprefixed_name() = UnprefixedChoiceName(t->substr(0, equals_p))
                                        ));
                        }
                    }
                }
            }
        }
    }
}

namespace
{
    bool match_anything(const PackageDepSpec & spec)
    {
        return package_dep_spec_has_properties(spec, make_named_values<PackageDepSpecProperties>(
                    n::has_additional_requirements() = false,
                    n::has_category_name_part() = false,
                    n::has_from_repository() = false,
                    n::has_in_repository() = false,
                    n::has_installable_to_path() = false,
                    n::has_installable_to_repository() = false,
                    n::has_installed_at_path() = false,
                    n::has_package() = false,
                    n::has_package_name_part() = false,
                    n::has_slot_requirement() = false,
                    n::has_tag() = false,
                    n::has_version_requirements() = false
                    ));
    }

    void check_values_groups(
            const Environment * const,
            const std::shared_ptr<const PackageID> &,
            const ChoicePrefixName & prefix,
            const UnprefixedChoiceName & unprefixed_name,
            const ValuesGroups & values_groups,
            bool & seen_minus_star,
            std::pair<Tribool, bool> & result_state,
            std::string & result_value)
    {

        for (ValuesGroups::const_iterator i(values_groups.begin()), i_end(values_groups.end()) ;
                i != i_end ; ++i)
        {
            if (i->prefix() != prefix)
                continue;

            seen_minus_star = seen_minus_star || i->minus_star();

            std::pair<Values::const_iterator, Values::const_iterator> range(i->values().equal_range(
                        make_named_values<Value>(
                            n::equals_value() = "",
                            n::locked() = false,
                            n::minus() = false,
                            n::unprefixed_name() = unprefixed_name
                            )));
            for ( ; range.first != range.second ; ++range.first)
            {
                if (range.first->minus())
                    result_state = std::make_pair(Tribool(false), range.first->locked());
                else
                    result_state = std::make_pair(Tribool(true), range.first->locked());

                if (! (range.first->equals_value().empty()))
                    result_value = range.first->equals_value();
            }
        }
    }

    void collect_known_from_values_groups(
            const Environment * const,
            const std::shared_ptr<const PackageID> &,
            const ChoicePrefixName & prefix,
            const ValuesGroups & values_groups,
            const std::shared_ptr<Set<UnprefixedChoiceName> > & known)
    {

        for (ValuesGroups::const_iterator i(values_groups.begin()), i_end(values_groups.end()) ;
                i != i_end ; ++i)
        {
            if (i->prefix() != prefix)
                continue;

            for (Values::const_iterator v(i->values().begin()), v_end(i->values().end()) ;
                    v != v_end ; ++v)
                known->insert(v->unprefixed_name());
        }
    }

    void check_specs_with_values_groups(
            const Environment * const env,
            const std::shared_ptr<const PackageID> & maybe_id,
            const ChoicePrefixName & prefix,
            const UnprefixedChoiceName & unprefixed_name,
            const SpecsWithValuesGroups & specs_with_values_groups,
            bool & seen_minus_star,
            std::pair<Tribool, bool> & result_state,
            std::string & result_value)
    {
        for (SpecsWithValuesGroups::const_iterator i(specs_with_values_groups.begin()),
                i_end(specs_with_values_groups.end()) ;
                i != i_end ; ++i)
        {
            if (maybe_id)
            {
                if (! match_package(*env, i->spec(), maybe_id, make_null_shared_ptr(), { }))
                    continue;
            }
            else
            {
                if (! match_anything(i->spec()))
                    continue;
            }

            check_values_groups(env, maybe_id, prefix, unprefixed_name, i->values_groups(),
                    seen_minus_star, result_state, result_value);
        }
    }

    void collect_known_from_specs_with_values_groups(
            const Environment * const env,
            const std::shared_ptr<const PackageID> & maybe_id,
            const ChoicePrefixName & prefix,
            const SpecsWithValuesGroups & specs_with_values_groups,
            const std::shared_ptr<Set<UnprefixedChoiceName> > & known)
    {
        for (SpecsWithValuesGroups::const_iterator i(specs_with_values_groups.begin()),
                i_end(specs_with_values_groups.end()) ;
                i != i_end ; ++i)
        {
            if (maybe_id)
            {
                if (! match_package(*env, i->spec(), maybe_id, make_null_shared_ptr(), { }))
                    continue;
            }
            else
            {
                if (! match_anything(i->spec()))
                    continue;
            }

            collect_known_from_values_groups(env, maybe_id, prefix, i->values_groups(), known);
        }
    }
}

const std::pair<Tribool, bool>
PaludisLikeOptionsConf::want_choice_enabled_locked(
        const std::shared_ptr<const PackageID> & maybe_id,
        const ChoicePrefixName & prefix,
        const UnprefixedChoiceName & unprefixed_name
        ) const
{
    Context context("When checking state of flag prefix '" + stringify(prefix) +
            "' name '" + stringify(unprefixed_name) + "' for '" +
            (maybe_id ? stringify(*maybe_id) : "*/*") + "':");

    bool seen_minus_star(false);
    std::pair<Tribool, bool> result(indeterminate, false);
    std::string dummy;

    /* Any specific matches? */
    if (maybe_id)
    {
        SpecificSpecs::const_iterator i(_imp->specific_specs.find(maybe_id->name()));
        if (i != _imp->specific_specs.end())
        {
            check_specs_with_values_groups(_imp->params.environment(), maybe_id, prefix, unprefixed_name, i->second,
                    seen_minus_star, result, dummy);
            if (! result.first.is_indeterminate())
                return result;
        }
    }

    /* Any set matches? */
    if (maybe_id && ! seen_minus_star)
    {
        for (SetNamesWithValuesGroups::const_iterator r(_imp->set_specs.begin()), r_end(_imp->set_specs.end()) ;
                r != r_end ; ++r)
        {
            if (! match_package_in_set(*_imp->params.environment(), *r->set_value().value().value(),
                        maybe_id, { }))
                continue;

            check_values_groups(_imp->params.environment(), maybe_id, prefix, unprefixed_name, r->values_groups(),
                    seen_minus_star, result, dummy);
        }

        if (! result.first.is_indeterminate())
            return result;
    }

    /* Wildcards? */
    if (! seen_minus_star)
    {
        check_specs_with_values_groups(_imp->params.environment(), maybe_id, prefix, unprefixed_name,
                _imp->wildcard_specs, seen_minus_star, result, dummy);

        if (! result.first.is_indeterminate())
            return result;
    }

    if (seen_minus_star)
        return std::make_pair(Tribool(false), false);
    else
        return std::make_pair(Tribool(indeterminate), false);
}

const std::string
PaludisLikeOptionsConf::value_for_choice_parameter(
        const std::shared_ptr<const PackageID> & id,
        const ChoicePrefixName & prefix,
        const UnprefixedChoiceName & unprefixed_name
        ) const
{
    Context context("When checking value for flag prefix '" + stringify(prefix) +
            "' name '" + stringify(unprefixed_name) + "' for '" + stringify(*id) + "':");

    bool dummy_seen_minus_star(false);
    std::pair<Tribool, bool> dummy_result;
    std::string equals_value;

    /* Any specific matches? */
    {
        SpecificSpecs::const_iterator i(_imp->specific_specs.find(id->name()));
        if (i != _imp->specific_specs.end())
        {
            check_specs_with_values_groups(_imp->params.environment(), id, prefix, unprefixed_name, i->second,
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
            if (! match_package_in_set(*_imp->params.environment(), *r->set_value().value().value(), id, { }))
                continue;

            check_values_groups(_imp->params.environment(), id, prefix, unprefixed_name, r->values_groups(),
                    dummy_seen_minus_star, dummy_result, equals_value);
        }

        if (! equals_value.empty())
            return equals_value;
    }

    /* Wildcards? */
    {
        check_specs_with_values_groups(_imp->params.environment(), id, prefix, unprefixed_name, _imp->wildcard_specs,
                dummy_seen_minus_star, dummy_result, equals_value);

        if (! equals_value.empty())
            return equals_value;
    }

    return "";
}

const std::shared_ptr<const Set<UnprefixedChoiceName> >
PaludisLikeOptionsConf::known_choice_value_names(
        const std::shared_ptr<const PackageID> & maybe_id,
        const ChoicePrefixName & prefix
        ) const
{
    const std::shared_ptr<Set<UnprefixedChoiceName> > result(std::make_shared<Set<UnprefixedChoiceName>>());

    /* Any specific matches? */
    if (maybe_id)
    {
        SpecificSpecs::const_iterator i(_imp->specific_specs.find(maybe_id->name()));
        if (i != _imp->specific_specs.end())
            collect_known_from_specs_with_values_groups(_imp->params.environment(), maybe_id, prefix, i->second, result);
    }

    /* Any set matches? */
    if (maybe_id)
    {
        for (SetNamesWithValuesGroups::const_iterator r(_imp->set_specs.begin()), r_end(_imp->set_specs.end()) ;
                r != r_end ; ++r)
        {
            if (! match_package_in_set(*_imp->params.environment(), *r->set_value().value().value(),
                        maybe_id, { }))
                continue;

            collect_known_from_values_groups(_imp->params.environment(), maybe_id, prefix, r->values_groups(), result);
        }
    }

    /* Wildcards? */
    {
        collect_known_from_specs_with_values_groups(_imp->params.environment(), maybe_id, prefix,
                _imp->wildcard_specs, result);
    }

    return result;
}

template class Pimp<PaludisLikeOptionsConf>;

