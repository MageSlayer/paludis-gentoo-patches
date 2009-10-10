/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009 Ciaran McCreesh
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
#include <paludis/util/stringify.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/save.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/simple_visitor_cast.hh>
#include <paludis/util/set.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/sequence.hh>
#include <paludis/metadata_key.hh>
#include <list>

using namespace paludis;
using namespace paludis::erepository;

typedef std::list<std::tr1::shared_ptr<DependenciesLabelSequence> > LabelsStack;

namespace
{
    const std::string get_annotations(const DepSpec & a)
    {
        std::stringstream s;
        if (a.annotations_key() && (a.annotations_key()->begin_metadata() != a.annotations_key()->end_metadata()))
        {
            s << " [[ ";
            for (MetadataSectionKey::MetadataConstIterator k(a.annotations_key()->begin_metadata()),
                    k_end(a.annotations_key()->end_metadata()) ;
                    k != k_end ; ++k)
            {
                const MetadataValueKey<std::string> * r(simple_visitor_cast<const MetadataValueKey<std::string> >(**k));
                if (! r)
                    throw InternalError(PALUDIS_HERE, "annotations must be string keys");
                s << (*k)->raw_name() << " = [" << (r->value().empty() ? " " : " " + r->value() + " ") << "] ";
            }
            s << "]]";
        }
        return s.str();
    }
}

namespace paludis
{
    template <>
    struct Implementation<DependenciesRewriter>
    {
        std::string depend;
        std::string rdepend;
        std::string pdepend;

        std::tr1::shared_ptr<DependenciesLabelSequence> default_labels;
        LabelsStack labels;

        Implementation() :
            default_labels(new DependenciesLabelSequence)
        {
            default_labels->push_back(make_shared_ptr(new DependenciesBuildLabel("build")));
            default_labels->push_back(make_shared_ptr(new DependenciesRunLabel("run")));
            labels.push_front(default_labels);
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
DependenciesRewriter::visit(const DependencySpecTree::NodeType<PackageDepSpec>::Type & node)
{
    _add_where_necessary(stringify(*node.spec()), *node.spec());
}

void
DependenciesRewriter::visit(const DependencySpecTree::NodeType<NamedSetDepSpec>::Type & node)
{
    _add_where_necessary(stringify(*node.spec()), *node.spec());
}

void
DependenciesRewriter::visit(const DependencySpecTree::NodeType<BlockDepSpec>::Type & node)
{
    _add_where_necessary(stringify(*node.spec()), *node.spec());
}

void
DependenciesRewriter::visit(const DependencySpecTree::NodeType<DependenciesLabelsDepSpec>::Type & node)
{
    _imp->depend.append(" " + stringify(*node.spec()) + get_annotations(*node.spec()));
    _imp->rdepend.append(" " + stringify(*node.spec()) + get_annotations(*node.spec()));
    _imp->pdepend.append(" " + stringify(*node.spec()) + get_annotations(*node.spec()));

    std::tr1::shared_ptr<DependenciesLabelSequence> labels(new DependenciesLabelSequence);
    std::copy(node.spec()->begin(), node.spec()->end(), labels->back_inserter());
    *_imp->labels.begin() = labels;
}

void
DependenciesRewriter::visit(const DependencySpecTree::NodeType<AllDepSpec>::Type & node)
{
    _imp->labels.push_front(*_imp->labels.begin());
    RunOnDestruction restore_labels(std::tr1::bind(std::tr1::mem_fn(&LabelsStack::pop_front), &_imp->labels));

    std::string d(_imp->depend), r(_imp->rdepend), p(_imp->pdepend);
    _imp->depend.clear();
    _imp->rdepend.clear();
    _imp->pdepend.clear();

    std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));

    _imp->depend = d + " ( " + _imp->depend + " )" + get_annotations(*node.spec());
    _imp->rdepend = r + " ( " + _imp->rdepend + " )" + get_annotations(*node.spec());
    _imp->pdepend = p + " ( " + _imp->pdepend + " )" + get_annotations(*node.spec());
}

void
DependenciesRewriter::visit(const DependencySpecTree::NodeType<AnyDepSpec>::Type & node)
{
    _imp->labels.push_front(*_imp->labels.begin());
    RunOnDestruction restore_labels(std::tr1::bind(std::tr1::mem_fn(&LabelsStack::pop_front), &_imp->labels));

    std::string d(_imp->depend), r(_imp->rdepend), p(_imp->pdepend);
    _imp->depend.clear();
    _imp->rdepend.clear();
    _imp->pdepend.clear();

    std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));

    _imp->depend = d + " || ( " + _imp->depend + " )" + get_annotations(*node.spec());
    _imp->rdepend = r + " || ( " + _imp->rdepend + " )" + get_annotations(*node.spec());
    _imp->pdepend = p + " || ( " + _imp->pdepend + " )" + get_annotations(*node.spec());
}

void
DependenciesRewriter::visit(const DependencySpecTree::NodeType<ConditionalDepSpec>::Type & node)
{
    _imp->labels.push_front(*_imp->labels.begin());
    RunOnDestruction restore_labels(std::tr1::bind(std::tr1::mem_fn(&LabelsStack::pop_front), &_imp->labels));

    std::string d(_imp->depend), r(_imp->rdepend), p(_imp->pdepend);
    _imp->depend.clear();
    _imp->rdepend.clear();
    _imp->pdepend.clear();

    std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));

    _imp->depend = d + " " + stringify(*node.spec()) + " ( " + _imp->depend + " )" + get_annotations(*node.spec());
    _imp->rdepend = r + " " + stringify(*node.spec()) + " ( " + _imp->rdepend + " )" + get_annotations(*node.spec());
    _imp->pdepend = p + " " + stringify(*node.spec()) + " ( " + _imp->pdepend + " )" + get_annotations(*node.spec());
}

namespace
{
    struct AddWhereNecessary
    {
        std::string & d, & r, & p;
        const std::string & s;
        const DepSpec & a;

        AddWhereNecessary(std::string & dd, std::string & rr, std::string & pp, const std::string & ss,
                const DepSpec & aa) :
            d(dd),
            r(rr),
            p(pp),
            s(ss),
            a(aa)
        {
        }

        void visit(const DependenciesRunLabel &)
        {
            r.append(" " + s + get_annotations(a));
        }

        void visit(const DependenciesPostLabel &)
        {
            p.append(" " + s + get_annotations(a));
        }

        void visit(const DependenciesSuggestionLabel &)
        {
            p.append(" " + s + get_annotations(a));
        }

        void visit(const DependenciesRecommendationLabel &)
        {
            p.append(" " + s + get_annotations(a));
        }

        void visit(const DependenciesBuildLabel &)
        {
            d.append(" " + s + get_annotations(a));
        }

        void visit(const DependenciesTestLabel &)
        {
            d.append(" " + s + get_annotations(a));
        }

        void visit(const DependenciesFetchLabel &)
        {
            d.append(" " + s + get_annotations(a));
        }

        void visit(const DependenciesCompileAgainstLabel &)
        {
            r.append(" " + s + get_annotations(a));
        }

        void visit(const DependenciesInstallLabel &)
        {
            d.append(" " + s + get_annotations(a));
        }
    };
}

void
DependenciesRewriter::_add_where_necessary(const std::string & s, const DepSpec & a)
{
    AddWhereNecessary v(_imp->depend, _imp->rdepend, _imp->pdepend, s, a);
    std::for_each(
            indirect_iterator((*_imp->labels.begin())->begin()),
            indirect_iterator((*_imp->labels.begin())->end()),
            accept_visitor(v));
}

