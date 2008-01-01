/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008 Ciaran McCreesh
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

#include <paludis/repositories/e/dependencies_rewriter.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/save.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <list>

using namespace paludis;
using namespace paludis::erepository;

typedef std::list<tr1::shared_ptr<ActiveDependencyLabels> > LabelsStack;

namespace paludis
{
    template <>
    struct Implementation<DependenciesRewriter>
    {
        std::string depend;
        std::string rdepend;
        std::string pdepend;

        tr1::shared_ptr<DependencyLabelSequence> default_labels;
        LabelsStack labels;

        Implementation() :
            default_labels(new DependencyLabelSequence)
        {
            default_labels->push_back(make_shared_ptr(new DependencyBuildLabel("build")));
            default_labels->push_back(make_shared_ptr(new DependencyRunLabel("run")));
            labels.push_front(make_shared_ptr(new ActiveDependencyLabels(*default_labels)));
        }
    };
}

DependenciesRewriter::DependenciesRewriter() :
    PrivateImplementationPattern<DependenciesRewriter>(new Implementation<DependenciesRewriter>)
{
}

DependenciesRewriter::~DependenciesRewriter()
{
}

const std::string
DependenciesRewriter::depend() const
{
    return _imp->depend;
}

const std::string
DependenciesRewriter::rdepend() const
{
    return _imp->rdepend;
}

const std::string
DependenciesRewriter::pdepend() const
{
    return _imp->pdepend;
}

void
DependenciesRewriter::visit_leaf(const PackageDepSpec & spec)
{
    _add_where_necessary(stringify(spec));
}

void
DependenciesRewriter::visit_leaf(const NamedSetDepSpec & spec)
{
    _add_where_necessary(stringify(spec));
}

void
DependenciesRewriter::visit_leaf(const BlockDepSpec & spec)
{
    _add_where_necessary(stringify(spec));
}

void
DependenciesRewriter::visit_leaf(const DependencyLabelsDepSpec & spec)
{
    _imp->depend.append(" " + stringify(spec));
    _imp->rdepend.append(" " + stringify(spec));
    _imp->pdepend.append(" " + stringify(spec));

    _imp->labels.begin()->reset(new ActiveDependencyLabels(**_imp->labels.begin(), spec));
}

void
DependenciesRewriter::visit_sequence(const AllDepSpec &,
        DependencySpecTree::ConstSequenceIterator cur,
        DependencySpecTree::ConstSequenceIterator end)
{
    _imp->labels.push_front(make_shared_ptr(new ActiveDependencyLabels(**_imp->labels.begin())));
    RunOnDestruction restore_labels(tr1::bind(tr1::mem_fn(&LabelsStack::pop_front), &_imp->labels));

    std::string d(_imp->depend), r(_imp->rdepend), p(_imp->pdepend);
    _imp->depend.clear();
    _imp->rdepend.clear();
    _imp->pdepend.clear();

    std::for_each(cur, end, accept_visitor(*this));

    _imp->depend = d + " ( " + _imp->depend + " )";
    _imp->rdepend = r + " ( " + _imp->rdepend + " )";
    _imp->pdepend = p + " ( " + _imp->pdepend + " )";
}

void
DependenciesRewriter::visit_sequence(const AnyDepSpec &,
        DependencySpecTree::ConstSequenceIterator cur,
        DependencySpecTree::ConstSequenceIterator end)
{
    _imp->labels.push_front(make_shared_ptr(new ActiveDependencyLabels(**_imp->labels.begin())));
    RunOnDestruction restore_labels(tr1::bind(tr1::mem_fn(&LabelsStack::pop_front), &_imp->labels));

    std::string d(_imp->depend), r(_imp->rdepend), p(_imp->pdepend);
    _imp->depend.clear();
    _imp->rdepend.clear();
    _imp->pdepend.clear();

    std::for_each(cur, end, accept_visitor(*this));

    _imp->depend = d + " || ( " + _imp->depend + " )";
    _imp->rdepend = r + " || ( " + _imp->rdepend + " )";
    _imp->pdepend = p + " || ( " + _imp->pdepend + " )";
}

void
DependenciesRewriter::visit_sequence(const UseDepSpec & spec,
        DependencySpecTree::ConstSequenceIterator cur,
        DependencySpecTree::ConstSequenceIterator end)
{
    _imp->labels.push_front(make_shared_ptr(new ActiveDependencyLabels(**_imp->labels.begin())));
    RunOnDestruction restore_labels(tr1::bind(tr1::mem_fn(&LabelsStack::pop_front), &_imp->labels));

    std::string d(_imp->depend), r(_imp->rdepend), p(_imp->pdepend);
    _imp->depend.clear();
    _imp->rdepend.clear();
    _imp->pdepend.clear();

    std::for_each(cur, end, accept_visitor(*this));

    _imp->depend = d + stringify(spec) + " ( " + _imp->depend + " )";
    _imp->rdepend = r + stringify(spec) + " ( " + _imp->rdepend + " )";
    _imp->pdepend = p + stringify(spec) + " ( " + _imp->pdepend + " )";
}

namespace
{
    struct AddWhereNecessary :
        ConstVisitor<DependencyTypeLabelVisitorTypes>
    {
        std::string & d, & r, & p;
        const std::string & s;

        AddWhereNecessary(std::string & dd, std::string & rr, std::string & pp, const std::string & ss) :
            d(dd),
            r(rr),
            p(pp),
            s(ss)
        {
        }

        void visit(const DependencyRunLabel &)
        {
            r.append(" " + s);
        }

        void visit(const DependencyPostLabel &)
        {
            p.append(" " + s);
        }

        void visit(const DependencyBuildLabel &)
        {
            d.append(" " + s);
        }

        void visit(const DependencyCompileLabel &)
        {
            r.append(" " + s);
        }

        void visit(const DependencyInstallLabel &)
        {
            d.append(" " + s);
        }
    };
}

void
DependenciesRewriter::_add_where_necessary(const std::string & s)
{
    AddWhereNecessary v(_imp->depend, _imp->rdepend, _imp->pdepend, s);
    std::for_each(
            indirect_iterator((*_imp->labels.begin())->type_labels()->begin()),
            indirect_iterator((*_imp->labels.begin())->type_labels()->end()),
            accept_visitor(v));
}

