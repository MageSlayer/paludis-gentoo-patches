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

#include <paludis/repositories/unpackaged/unpackaged_key.hh>
#include <paludis/repositories/unpackaged/unpackaged_id.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/set.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/choice.hh>
#include <paludis/elike_choices.hh>
#include <paludis/comma_separated_dep_printer.hh>
#include <paludis/comma_separated_dep_parser.hh>
#include <paludis/comma_separated_dep_pretty_printer.hh>
#include <memory>

using namespace paludis;
using namespace paludis::unpackaged_repositories;

namespace paludis
{
    template <>
    struct Imp<UnpackagedDependencyKey>
    {
        const Environment * const env;
        const std::shared_ptr<const DependencySpecTree> value;
        const std::shared_ptr<const DependenciesLabelSequence> labels;

        const std::string raw_name;
        const std::string human_name;
        const MetadataKeyType type;

        Imp(const Environment * const e, const std::string & v,
                const std::shared_ptr<const DependenciesLabelSequence> & l,
                const std::string & r, const std::string & h, const MetadataKeyType t) :
            env(e),
            value(CommaSeparatedDepParser::parse(env, v)),
            labels(l),
            raw_name(r),
            human_name(h),
            type(t)
        {
        }
    };
}

UnpackagedDependencyKey::UnpackagedDependencyKey(const Environment * const env,
        const std::string & r, const std::string & h, const MetadataKeyType t,
        const std::shared_ptr<const DependenciesLabelSequence> & l,
        const std::string & v) :
    Pimp<UnpackagedDependencyKey>(env, v, l, r, h, t)
{
}

UnpackagedDependencyKey::~UnpackagedDependencyKey()
{
}

const std::shared_ptr<const DependencySpecTree>
UnpackagedDependencyKey::value() const
{
    return _imp->value;
}

const std::string
UnpackagedDependencyKey::raw_name() const
{
    return _imp->raw_name;
}

const std::string
UnpackagedDependencyKey::human_name() const
{
    return _imp->human_name;
}

MetadataKeyType
UnpackagedDependencyKey::type() const
{
    return _imp->type;
}

const std::string
UnpackagedDependencyKey::pretty_print_value(
        const PrettyPrinter & printer, const PrettyPrintOptions & options) const
{
    CommaSeparatedDepPrettyPrinter p(printer, options);
    _imp->value->top()->accept(p);
    return p.result();
}

std::string
UnpackagedDependencyKey::pretty_print(const DependencySpecTree::ItemFormatter & f) const
{
    CommaSeparatedDepPrinter p(_imp->env, f, false);
    _imp->value->top()->accept(p);
    return p.result();
}

std::string
UnpackagedDependencyKey::pretty_print_flat(const DependencySpecTree::ItemFormatter & f) const
{
    CommaSeparatedDepPrinter p(_imp->env, f, true);
    _imp->value->top()->accept(p);
    return p.result();
}

const std::shared_ptr<const DependenciesLabelSequence>
UnpackagedDependencyKey::initial_labels() const
{
    return _imp->labels;
}

namespace paludis
{
    template <>
    struct Imp<UnpackagedChoicesKey>
    {
        const Environment * const env;
        const UnpackagedID * const id;

        mutable Mutex mutex;
        mutable std::shared_ptr<Choices> value;

        const std::string raw_name;
        const std::string human_name;
        const MetadataKeyType type;

        Imp(const Environment * const e, const UnpackagedID * const i,
                const std::string & r, const std::string & h, const MetadataKeyType t) :
            env(e),
            id(i),
            raw_name(r),
            human_name(h),
            type(t)
        {
        }
    };
}

UnpackagedChoicesKey::UnpackagedChoicesKey(const Environment * const env, const std::string & r, const std::string & h,
        const MetadataKeyType t, const UnpackagedID * const id) :
    Pimp<UnpackagedChoicesKey>(env, id, r, h, t)
{
}

UnpackagedChoicesKey::~UnpackagedChoicesKey()
{
}

const std::shared_ptr<const Choices>
UnpackagedChoicesKey::value() const
{
    Lock lock(_imp->mutex);
    if (! _imp->value)
    {
        _imp->value = std::make_shared<Choices>();
        std::shared_ptr<Choice> build_options(std::make_shared<Choice>(make_named_values<ChoiceParams>(
                        n::consider_added_or_changed() = false,
                        n::contains_every_value() = false,
                        n::hidden() = false,
                        n::human_name() = canonical_build_options_human_name(),
                        n::prefix() = canonical_build_options_prefix(),
                        n::raw_name() = canonical_build_options_raw_name(),
                        n::show_with_no_prefix() = false
                        )));

        Tribool strip(indeterminate);
        if (_imp->id->strip_key())
            strip = _imp->id->strip_key()->value();

        build_options->add(std::make_shared<ELikeSplitChoiceValue>(_imp->id->shared_from_this(), _imp->env, build_options, strip));
        build_options->add(std::make_shared<ELikeStripChoiceValue>(_imp->id->shared_from_this(), _imp->env, build_options, strip));

        Tribool preserve_work(indeterminate);
        if (_imp->id->preserve_work_key())
            preserve_work = _imp->id->preserve_work_key()->value();

        build_options->add(std::make_shared<ELikePreserveWorkChoiceValue>(_imp->id->shared_from_this(), _imp->env, build_options, preserve_work));

        _imp->value->add(build_options);
    }

    return _imp->value;
}

const std::string
UnpackagedChoicesKey::raw_name() const
{
    return _imp->raw_name;
}

const std::string
UnpackagedChoicesKey::human_name() const
{
    return _imp->human_name;
}

MetadataKeyType
UnpackagedChoicesKey::type() const
{
    return _imp->type;
}

