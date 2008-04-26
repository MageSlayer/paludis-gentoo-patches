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

#include <paludis/dep_label.hh>
#include <paludis/dep_spec.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/sequence-impl.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/stringify.hh>
#include <ostream>
#include <algorithm>

using namespace paludis;

template class Sequence<std::tr1::shared_ptr<const DependencyLabel> >;
template class Sequence<std::tr1::shared_ptr<const DependencySystemLabel> >;
template class Sequence<std::tr1::shared_ptr<const DependencyTypeLabel> >;
template class Sequence<std::tr1::shared_ptr<const DependencySuggestLabel> >;
template class Sequence<std::tr1::shared_ptr<const DependencyABIsLabel> >;

std::ostream &
paludis::operator<< (std::ostream & s, const URILabel & l)
{
    s << l.text();
    return s;
}

std::ostream &
paludis::operator<< (std::ostream & s, const DependencyLabel & l)
{
    s << l.text();
    return s;
}

URILabel::~URILabel()
{
}

namespace paludis
{
#ifndef PALUDIS_NO_DOUBLE_TEMPLATE
    template <>
#endif
    template <typename T_>
    struct Implementation<ConcreteURILabel<T_> >
    {
        const std::string text;

        Implementation(const std::string & t) :
            text(t)
        {
        }
    };
}

template <typename T_>
ConcreteURILabel<T_>::ConcreteURILabel(const std::string & t) :
    PrivateImplementationPattern<ConcreteURILabel<T_> >(new Implementation<ConcreteURILabel<T_> >(t))
{
}

template <typename T_>
ConcreteURILabel<T_>::~ConcreteURILabel()
{
}

template <typename T_>
const std::string
ConcreteURILabel<T_>::text() const
{
    return _imp->text;
}

template class ConcreteURILabel<URIMirrorsThenListedLabel::Tag>;
template class ConcreteURILabel<URIMirrorsOnlyLabel::Tag>;
template class ConcreteURILabel<URIListedOnlyLabel::Tag>;
template class ConcreteURILabel<URIListedThenMirrorsLabel::Tag>;
template class ConcreteURILabel<URILocalMirrorsOnlyLabel::Tag>;
template class ConcreteURILabel<URIManualOnlyLabel::Tag>;

std::ostream & operator<<(std::ostream & s, const DependencyLabel & l)
{
    s << l.text();
    return s;
}

DependencyLabel::~DependencyLabel()
{
}

namespace paludis
{
#ifndef PALUDIS_NO_DOUBLE_TEMPLATE
    template <>
#endif
    template <typename T_, typename C_>
    struct Implementation<ConcreteDependencyLabel<T_, C_> >
    {
        const std::string text;

        Implementation(const std::string & t) :
            text(t)
        {
        }
    };
}

template <typename T_, typename C_>
ConcreteDependencyLabel<T_, C_>::ConcreteDependencyLabel(const std::string & t) :
    PrivateImplementationPattern<ConcreteDependencyLabel<T_, C_> >(new Implementation<ConcreteDependencyLabel<T_, C_> >(t))
{
}

template <typename T_, typename C_>
ConcreteDependencyLabel<T_, C_>::~ConcreteDependencyLabel()
{
}

template <typename T_, typename C_>
const std::string
ConcreteDependencyLabel<T_, C_>::text() const
{
    return _imp->text;
}

namespace paludis
{
    template <>
    struct Implementation<ActiveDependencyLabels>
    {
        std::tr1::shared_ptr<DependencySystemLabelSequence> system_labels;
        std::tr1::shared_ptr<DependencyTypeLabelSequence> type_labels;
        std::tr1::shared_ptr<DependencyABIsLabelSequence> abi_labels;
        std::tr1::shared_ptr<DependencySuggestLabelSequence> suggest_labels;
    };
}

namespace
{
    struct LabelsPopulator :
        ConstVisitor<DependencyLabelVisitorTypes>
    {
        Implementation<ActiveDependencyLabels> & _imp;
        const std::tr1::shared_ptr<const DependencyLabel> _l;

        LabelsPopulator(Implementation<ActiveDependencyLabels> & i,
                const std::tr1::shared_ptr<const DependencyLabel> & l) :
            _imp(i),
            _l(l)
        {
        }

        void visit(const DependencySuggestLabel &)
        {
            if (! _imp.suggest_labels)
                _imp.suggest_labels.reset(new DependencySuggestLabelSequence);
            _imp.suggest_labels->push_back(std::tr1::static_pointer_cast<const DependencySuggestLabel>(_l));
        }

        void visit(const DependencySystemLabel &)
        {
            if (! _imp.system_labels)
                _imp.system_labels.reset(new DependencySystemLabelSequence);
            _imp.system_labels->push_back(std::tr1::static_pointer_cast<const DependencySystemLabel>(_l));
        }

        void visit(const DependencyABIsLabel &)
        {
            if (! _imp.abi_labels)
                _imp.abi_labels.reset(new DependencyABIsLabelSequence);
            _imp.abi_labels->push_back(std::tr1::static_pointer_cast<const DependencyABIsLabel>(_l));
        }

        void visit(const DependencyTypeLabel &)
        {
            if (! _imp.type_labels)
                _imp.type_labels.reset(new DependencyTypeLabelSequence);
            _imp.type_labels->push_back(std::tr1::static_pointer_cast<const DependencyTypeLabel>(_l));
        }
    };
}

ActiveDependencyLabels::ActiveDependencyLabels(const DependencyLabelsDepSpec & spec) :
    PrivateImplementationPattern<ActiveDependencyLabels>(new Implementation<ActiveDependencyLabels>)
{
    for (DependencyLabelsDepSpec::ConstIterator i(spec.begin()), i_end(spec.end()) ;
            i != i_end ; ++i)
    {
        LabelsPopulator p(*_imp.get(), *i);
        (*i)->accept(p);
    }

    if (! _imp->system_labels)
        _imp->system_labels.reset(new DependencySystemLabelSequence);
    if (! _imp->type_labels)
        _imp->type_labels.reset(new DependencyTypeLabelSequence);
    if (! _imp->suggest_labels)
        _imp->suggest_labels.reset(new DependencySuggestLabelSequence);
    if (! _imp->abi_labels)
        _imp->abi_labels.reset(new DependencyABIsLabelSequence);
}

ActiveDependencyLabels::ActiveDependencyLabels(const ActiveDependencyLabels & other) :
    PrivateImplementationPattern<ActiveDependencyLabels>(new Implementation<ActiveDependencyLabels>)
{
    _imp->system_labels.reset(new DependencySystemLabelSequence);
    std::copy(other._imp->system_labels->begin(), other._imp->system_labels->end(), _imp->system_labels->back_inserter());

    _imp->type_labels.reset(new DependencyTypeLabelSequence);
    std::copy(other._imp->type_labels->begin(), other._imp->type_labels->end(), _imp->type_labels->back_inserter());

    _imp->suggest_labels.reset(new DependencySuggestLabelSequence);
    std::copy(other._imp->suggest_labels->begin(), other._imp->suggest_labels->end(), _imp->suggest_labels->back_inserter());

    _imp->abi_labels.reset(new DependencyABIsLabelSequence);
    std::copy(other._imp->abi_labels->begin(), other._imp->abi_labels->end(), _imp->abi_labels->back_inserter());
}

ActiveDependencyLabels::ActiveDependencyLabels(const ActiveDependencyLabels & other, const DependencyLabelsDepSpec & spec) :
    PrivateImplementationPattern<ActiveDependencyLabels>(new Implementation<ActiveDependencyLabels>)
{
    for (DependencyLabelsDepSpec::ConstIterator i(spec.begin()), i_end(spec.end()) ;
            i != i_end ; ++i)
    {
        LabelsPopulator p(*_imp.get(), *i);
        (*i)->accept(p);
    }

    if (! _imp->system_labels)
    {
        _imp->system_labels.reset(new DependencySystemLabelSequence);
        std::copy(other._imp->system_labels->begin(), other._imp->system_labels->end(), _imp->system_labels->back_inserter());
    }
    if (! _imp->type_labels)
    {
        _imp->type_labels.reset(new DependencyTypeLabelSequence);
        std::copy(other._imp->type_labels->begin(), other._imp->type_labels->end(), _imp->type_labels->back_inserter());
    }
    if (! _imp->suggest_labels)
    {
        _imp->suggest_labels.reset(new DependencySuggestLabelSequence);
        std::copy(other._imp->suggest_labels->begin(), other._imp->suggest_labels->end(), _imp->suggest_labels->back_inserter());
    }
    if (! _imp->abi_labels)
    {
        _imp->abi_labels.reset(new DependencyABIsLabelSequence);
        std::copy(other._imp->abi_labels->begin(), other._imp->abi_labels->end(), _imp->abi_labels->back_inserter());
    }
}

ActiveDependencyLabels::ActiveDependencyLabels(const DependencyLabelSequence & spec) :
    PrivateImplementationPattern<ActiveDependencyLabels>(new Implementation<ActiveDependencyLabels>)
{
    for (DependencyLabelSequence::ConstIterator i(spec.begin()), i_end(spec.end()) ;
            i != i_end ; ++i)
    {
        LabelsPopulator p(*_imp.get(), *i);
        (*i)->accept(p);
    }

    if (! _imp->system_labels)
        _imp->system_labels.reset(new DependencySystemLabelSequence);
    if (! _imp->type_labels)
        _imp->type_labels.reset(new DependencyTypeLabelSequence);
    if (! _imp->suggest_labels)
        _imp->suggest_labels.reset(new DependencySuggestLabelSequence);
    if (! _imp->abi_labels)
        _imp->abi_labels.reset(new DependencyABIsLabelSequence);
}

ActiveDependencyLabels::~ActiveDependencyLabels()
{
}

const std::tr1::shared_ptr<const DependencySystemLabelSequence>
ActiveDependencyLabels::system_labels() const
{
    return _imp->system_labels;
}

const std::tr1::shared_ptr<const DependencyTypeLabelSequence>
ActiveDependencyLabels::type_labels() const
{
    return _imp->type_labels;
}

const std::tr1::shared_ptr<const DependencyABIsLabelSequence>
ActiveDependencyLabels::abi_labels() const
{
    return _imp->abi_labels;
}

const std::tr1::shared_ptr<const DependencySuggestLabelSequence>
ActiveDependencyLabels::suggest_labels() const
{
    return _imp->suggest_labels;
}

template class ConcreteDependencyLabel<DependencyHostLabel::Tag, DependencySystemLabel>;
template class ConcreteDependencyLabel<DependencyTargetLabel::Tag, DependencySystemLabel>;
template class ConcreteDependencyLabel<DependencyBuildLabel::Tag, DependencyTypeLabel>;
template class ConcreteDependencyLabel<DependencyRunLabel::Tag, DependencyTypeLabel>;
template class ConcreteDependencyLabel<DependencyPostLabel::Tag, DependencyTypeLabel>;
template class ConcreteDependencyLabel<DependencyInstallLabel::Tag, DependencyTypeLabel>;
template class ConcreteDependencyLabel<DependencyCompileLabel::Tag, DependencyTypeLabel>;
template class ConcreteDependencyLabel<DependencySuggestedLabel::Tag, DependencySuggestLabel>;
template class ConcreteDependencyLabel<DependencyRecommendedLabel::Tag, DependencySuggestLabel>;
template class ConcreteDependencyLabel<DependencyRequiredLabel::Tag, DependencySuggestLabel>;
template class ConcreteDependencyLabel<DependencyAnyLabel::Tag, DependencyABIsLabel>;
template class ConcreteDependencyLabel<DependencyMineLabel::Tag, DependencyABIsLabel>;
template class ConcreteDependencyLabel<DependencyPrimaryLabel::Tag, DependencyABIsLabel>;
template class ConcreteDependencyLabel<DependencyABILabel::Tag, DependencyABIsLabel>;

