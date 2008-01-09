/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Mike Kelly
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

#include <paludis/repositories/e/distfiles_size_visitor.hh>
#include <paludis/repositories/e/source_uri_finder.hh>
#include <paludis/repositories/e/e_repository_id.hh>
#include <paludis/repositories/e/dep_parser.hh>
#include <paludis/repositories/e/manifest2_reader.hh>
#include <paludis/dep_spec.hh>
#include <paludis/environment.hh>
#include <paludis/package_id.hh>
#include <paludis/action.hh>
#include <paludis/repository.hh>
#include <paludis/about.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/system.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/log.hh>
#include <paludis/util/join.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <iostream>
#include <list>

using namespace paludis;
using namespace paludis::erepository;

namespace paludis
{
    template <>
    struct Implementation<DistfilesSizeVisitor>
    {
        const Environment * const env;
        const tr1::shared_ptr<const PackageID> id;
        const FSEntry distdir;
        tr1::shared_ptr<const URILabel> default_label;
        bool everything;
        const tr1::shared_ptr<Manifest2Reader> m2r;

        std::list<const URILabel *> labels;
        size_t size;

        Implementation(
                const Environment * const e,
                const tr1::shared_ptr<const PackageID> & i,
                const FSEntry & d,
                const tr1::shared_ptr<const URILabel> & n,
                const bool ev,
                const tr1::shared_ptr<Manifest2Reader> mr) :
            env(e),
            id(i),
            distdir(d),
            default_label(n),
            everything(ev),
            m2r(mr),
            size(0)
        {
            labels.push_front(default_label.get());
        }
    };
}

DistfilesSizeVisitor::DistfilesSizeVisitor(
        const Environment * const e,
        const tr1::shared_ptr<const PackageID> & i,
        const FSEntry & d,
        const tr1::shared_ptr<const URILabel> & n,
        const bool ev,
        const tr1::shared_ptr<Manifest2Reader> mr) :
    PrivateImplementationPattern<DistfilesSizeVisitor>(new Implementation<DistfilesSizeVisitor>(e, i, d, n, ev, mr))
{
}

DistfilesSizeVisitor::~DistfilesSizeVisitor()
{
}

void
DistfilesSizeVisitor::visit_sequence(const UseDepSpec & u,
        FetchableURISpecTree::ConstSequenceIterator cur,
        FetchableURISpecTree::ConstSequenceIterator end)
{
    if (_imp->env->query_use(u.flag(), *_imp->id) ^ u.inverse())
    {
        _imp->labels.push_front(* _imp->labels.begin());
        std::for_each(cur, end, accept_visitor(*this));
        _imp->labels.pop_front();
    }
}

void
DistfilesSizeVisitor::visit_sequence(const AllDepSpec &,
        FetchableURISpecTree::ConstSequenceIterator cur,
        FetchableURISpecTree::ConstSequenceIterator end)
{
    _imp->labels.push_front(* _imp->labels.begin());
    std::for_each(cur, end, accept_visitor(*this));
    _imp->labels.pop_front();
}

void
DistfilesSizeVisitor::visit_leaf(const URILabelsDepSpec & l)
{
    for (URILabelsDepSpec::ConstIterator i(l.begin()), i_end(l.end()) ;
            i != i_end ; ++i)
        *_imp->labels.begin() = i->get();
}

void
DistfilesSizeVisitor::visit_leaf(const FetchableURIDepSpec & u)
{
    Context context("When visiting URI dep spec '" + stringify(u.text()) + "':");

    FSEntry destination(_imp->distdir / u.filename());

    if (destination.exists() && ! _imp->everything)
        return;
    Manifest2Reader::ConstIterator m(_imp->m2r->find("DIST", u.filename()));
    if (_imp->m2r->end() == m)
        return;
    long s(m->size);
    Log::get_instance()->message(ll_debug, lc_context) << "Adding " << s << " to size. Was "
        << _imp->size << ", is now " << (_imp->size + s);
    _imp->size += s;
}

long
DistfilesSizeVisitor::size()
{
    return _imp->size;
}
