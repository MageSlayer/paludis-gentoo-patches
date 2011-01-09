/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#include <paludis/repositories/e/pretend_fetch_visitor.hh>
#include <paludis/repositories/e/e_repository_id.hh>
#include <paludis/repositories/e/manifest2_reader.hh>

#include <paludis/dep_spec.hh>
#include <paludis/environment.hh>
#include <paludis/package_id.hh>
#include <paludis/action.hh>
#include <paludis/metadata_key.hh>

#include <paludis/util/pimp-impl.hh>
#include <paludis/util/log.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/accept_visitor.hh>
#include <paludis/util/fs_stat.hh>

#include <algorithm>
#include <list>
#include <set>

using namespace paludis;
using namespace paludis::erepository;

namespace paludis
{
    template <>
    struct Imp<PretendFetchVisitor>
    {
        const Environment * const env;
        const std::shared_ptr<const PackageID> id;
        const EAPI & eapi;
        const FSPath distdir;
        const bool fetch_unneeded;
        std::shared_ptr<const URILabel> default_label;
        PretendFetchAction & action;

        std::set<std::string> already_done;
        std::list<const URILabel *> labels;
        Manifest2Reader manifest;

        Imp(
                const Environment * const e,
                const std::shared_ptr<const PackageID> & i,
                const EAPI & p,
                const FSPath & d,
                const bool f,
                const std::shared_ptr<const URILabel> & n,
                PretendFetchAction & a) :
            env(e),
            id(i),
            eapi(p),
            distdir(d),
            fetch_unneeded(f),
            default_label(n),
            action(a),
            manifest(id->fs_location_key()->value().dirname() / "Manifest")
        {
            labels.push_front(default_label.get());
        }
    };
}

PretendFetchVisitor::PretendFetchVisitor(
        const Environment * const e,
        const std::shared_ptr<const PackageID> & i,
        const EAPI & p,
        const FSPath & d,
        const bool f,
        const std::shared_ptr<const URILabel> & n,
        PretendFetchAction & a) :
    _imp(e, i, p, d, f, n, a)
{
}

PretendFetchVisitor::~PretendFetchVisitor()
{
}

void
PretendFetchVisitor::visit(const FetchableURISpecTree::NodeType<ConditionalDepSpec>::Type & node)
{
    if ((_imp->fetch_unneeded) || (node.spec()->condition_met(_imp->env, _imp->id)))
    {
        _imp->labels.push_front(* _imp->labels.begin());
        std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
        _imp->labels.pop_front();
    }
}

void
PretendFetchVisitor::visit(const FetchableURISpecTree::NodeType<AllDepSpec>::Type & node)
{
    _imp->labels.push_front(* _imp->labels.begin());
    std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
    _imp->labels.pop_front();
}

void
PretendFetchVisitor::visit(const FetchableURISpecTree::NodeType<URILabelsDepSpec>::Type & node)
{
    for (URILabelsDepSpec::ConstIterator i(node.spec()->begin()), i_end(node.spec()->end()) ;
            i != i_end ; ++i)
        *_imp->labels.begin() = i->get();
}

void
PretendFetchVisitor::visit(const FetchableURISpecTree::NodeType<FetchableURIDepSpec>::Type & node)
{
    if (! _imp->already_done.insert(node.spec()->filename()).second)
        return;

    FSPath destination(_imp->distdir / node.spec()->filename());
    if (destination.stat().exists())
        return;

    Manifest2Reader::ConstIterator m(_imp->manifest.find("DIST", node.spec()->filename()));
    if (_imp->manifest.end() == m)
        return;

    _imp->action.will_fetch(destination, m->size());
}

