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

#include <paludis/repositories/unpackaged/unpackaged_key.hh>
#include <paludis/repositories/unpackaged/dep_printer.hh>
#include <paludis/repositories/unpackaged/dep_parser.hh>
#include <paludis/repositories/unpackaged/unpackaged_id.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/validated.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/set.hh>
#include <paludis/choice.hh>
#include <paludis/elike_choices.hh>
#include <tr1/memory>

using namespace paludis;
using namespace paludis::unpackaged_repositories;

namespace paludis
{
    template <>
    struct Implementation<UnpackagedDependencyKey>
    {
        const Environment * const env;
        const std::tr1::shared_ptr<const DependencySpecTree::ConstItem> value;
        const std::tr1::shared_ptr<const DependencyLabelSequence> labels;

        Implementation(const Environment * const e, const std::string & v,
                const std::tr1::shared_ptr<const DependencyLabelSequence> & l) :
            env(e),
            value(DepParser::parse(env, v)),
            labels(l)
        {
        }
    };
}

UnpackagedDependencyKey::UnpackagedDependencyKey(const Environment * const env,
        const std::string & r, const std::string & h, const MetadataKeyType t,
        const std::tr1::shared_ptr<const DependencyLabelSequence> & l,
        const std::string & v) :
    MetadataSpecTreeKey<DependencySpecTree>(r, h, t),
    PrivateImplementationPattern<UnpackagedDependencyKey>(new Implementation<UnpackagedDependencyKey>(env, v, l)),
    _imp(PrivateImplementationPattern<UnpackagedDependencyKey>::_imp)
{
}

UnpackagedDependencyKey::~UnpackagedDependencyKey()
{
}

const std::tr1::shared_ptr<const DependencySpecTree::ConstItem>
UnpackagedDependencyKey::value() const
{
    return _imp->value;
}

std::string
UnpackagedDependencyKey::pretty_print(const DependencySpecTree::ItemFormatter & f) const
{
    DepPrinter p(_imp->env, f, false);
    _imp->value->accept(p);
    return p.result();
}

std::string
UnpackagedDependencyKey::pretty_print_flat(const DependencySpecTree::ItemFormatter & f) const
{
    DepPrinter p(_imp->env, f, true);
    _imp->value->accept(p);
    return p.result();
}

const std::tr1::shared_ptr<const DependencyLabelSequence>
UnpackagedDependencyKey::initial_labels() const
{
    return _imp->labels;
}

namespace paludis
{
    template <>
    struct Implementation<UnpackagedChoicesKey>
    {
        const Environment * const env;
        const UnpackagedID * const id;

        mutable Mutex mutex;
        mutable std::tr1::shared_ptr<Choices> value;

        Implementation(const Environment * const e, const UnpackagedID * const i) :
            env(e),
            id(i)
        {
        }
    };
}

UnpackagedChoicesKey::UnpackagedChoicesKey(const Environment * const env, const std::string & r, const std::string & h,
        const MetadataKeyType t, const UnpackagedID * const id) :
    MetadataValueKey<std::tr1::shared_ptr<const Choices> >(r, h, t),
    PrivateImplementationPattern<UnpackagedChoicesKey>(new Implementation<UnpackagedChoicesKey>(env, id)),
    _imp(PrivateImplementationPattern<UnpackagedChoicesKey>::_imp)
{
}

UnpackagedChoicesKey::~UnpackagedChoicesKey()
{
}

const std::tr1::shared_ptr<const Choices>
UnpackagedChoicesKey::value() const
{
    Lock lock(_imp->mutex);
    if (! _imp->value)
    {
        _imp->value.reset(new Choices);
        std::tr1::shared_ptr<Choice> build_options(new Choice(canonical_build_options_raw_name(), canonical_build_options_human_name(),
                    canonical_build_options_prefix(), false, false, false, false));
        build_options->add(make_shared_ptr(new ELikeSplitChoiceValue(_imp->id->shared_from_this(), _imp->env, build_options)));
        build_options->add(make_shared_ptr(new ELikeStripChoiceValue(_imp->id->shared_from_this(), _imp->env, build_options)));
        _imp->value->add(build_options);
    }

    return _imp->value;
}

