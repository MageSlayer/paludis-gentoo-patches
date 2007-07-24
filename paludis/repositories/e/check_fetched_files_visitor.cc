/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <paludis/repositories/e/check_fetched_files_visitor.hh>
#include <paludis/repositories/e/source_uri_finder.hh>
#include <paludis/repositories/e/e_repository_id.hh>
#include <paludis/dep_spec.hh>
#include <paludis/environment.hh>
#include <paludis/package_id.hh>
#include <paludis/repository.hh>
#include <paludis/about.hh>
#include <paludis/action.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/system.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/log.hh>
#include <paludis/util/join.hh>
#include <paludis/util/save.hh>
#include <paludis/util/stringify.hh>
#include <iostream>
#include <list>
#include <set>

using namespace paludis;
using namespace paludis::erepository;

namespace paludis
{
    template <>
    struct Implementation<CheckFetchedFilesVisitor>
    {
        const Environment * const env;
        const tr1::shared_ptr<const PackageID> id;
        const FSEntry distdir;
        const bool check_unneeded;

        std::set<std::string> done;
        const tr1::shared_ptr<Sequence<FetchActionFailure> > failures;
        bool need_nofetch;
        bool in_nofetch;

        Implementation(
                const Environment * const e,
                const tr1::shared_ptr<const PackageID> & i,
                const FSEntry & d,
                const bool c,
                const bool n) :
            env(e),
            id(i),
            distdir(d),
            check_unneeded(c),
            failures(new Sequence<FetchActionFailure>),
            need_nofetch(false),
            in_nofetch(n)
        {
        }
    };
}

CheckFetchedFilesVisitor::CheckFetchedFilesVisitor(
        const Environment * const e,
        const tr1::shared_ptr<const PackageID> & i,
        const FSEntry & d,
        const bool c,
        const bool n) :
    PrivateImplementationPattern<CheckFetchedFilesVisitor>(new Implementation<CheckFetchedFilesVisitor>(e, i, d, c, n))
{
}

CheckFetchedFilesVisitor::~CheckFetchedFilesVisitor()
{
}

void
CheckFetchedFilesVisitor::visit_sequence(const UseDepSpec & u,
        URISpecTree::ConstSequenceIterator cur,
        URISpecTree::ConstSequenceIterator end)
{
    Save<bool> save_in_nofetch(&_imp->in_nofetch, _imp->in_nofetch);
    if ((_imp->check_unneeded) || (_imp->env->query_use(u.flag(), *_imp->id) ^ u.inverse()))
        std::for_each(cur, end, accept_visitor(*this));
}

void
CheckFetchedFilesVisitor::visit_sequence(const AllDepSpec &,
        URISpecTree::ConstSequenceIterator cur,
        URISpecTree::ConstSequenceIterator end)
{
    Save<bool> save_in_nofetch(&_imp->in_nofetch, _imp->in_nofetch);
    std::for_each(cur, end, accept_visitor(*this));
}

namespace
{
    struct InNoFetchVisitor :
        ConstVisitor<URILabelVisitorTypes>
    {
        bool result;

        InNoFetchVisitor() :
            result(false)
        {
        }

        void visit(const URIMirrorsOnlyLabel &)
        {
        }

        void visit(const URIManualOnlyLabel &)
        {
            result = true;
        }

        void visit(const URIMirrorsThenListedLabel &)
        {
        }

        void visit(const URILocalMirrorsOnlyLabel &)
        {
            result = true;
        }

        void visit(const URIListedOnlyLabel &)
        {
        }

        void visit(const URIListedThenMirrorsLabel &)
        {
        }
    };
}

void
CheckFetchedFilesVisitor::visit_leaf(const LabelsDepSpec<URILabelVisitorTypes> & l)
{
    InNoFetchVisitor v;
    std::for_each(l.begin(), l.end(), accept_visitor(v));
    _imp->in_nofetch = v.result;
}

void
CheckFetchedFilesVisitor::visit_leaf(const URIDepSpec & u)
{
    Context context("When visiting URI dep spec '" + stringify(u.text()) + "':");

    if (_imp->done.end() != _imp->done.find(u.filename()))
    {
        Log::get_instance()->message(ll_debug, lc_context) << "Already checked '" << u.filename() << "'";
        return;
    }
    _imp->done.insert(u.filename());

    std::cout << "Checking '" << u.filename() << "'... ";

    if (! (_imp->distdir / u.filename()).is_regular_file())
    {
        if (_imp->in_nofetch)
        {
            Log::get_instance()->message(ll_debug, lc_context) << "Manual fetch required for '" << u.filename() << "'";
            std::cout << "requires manual fetch";
            _imp->need_nofetch = true;
            _imp->failures->push_back(FetchActionFailure::create()
                    .target_file(u.filename())
                    .requires_manual_fetching(true)
                    .failed_automatic_fetching(false)
                    .failed_integrity_checks("")
                    );
        }
        else
        {
            Log::get_instance()->message(ll_debug, lc_context) << "Automatic fetch failed for '" << u.filename() << "'";
            std::cout << "does not exist";
            _imp->failures->push_back(FetchActionFailure::create()
                    .target_file(u.filename())
                    .requires_manual_fetching(false)
                    .failed_automatic_fetching(true)
                    .failed_integrity_checks("")
                    );
        }
    }
    else if (0 == (_imp->distdir / u.filename()).file_size())
    {
        Log::get_instance()->message(ll_debug, lc_context) << "Empty file for '" << u.filename() << "'";
        std::cout << "empty file";
        _imp->failures->push_back(FetchActionFailure::create()
                .target_file(u.filename())
                .requires_manual_fetching(false)
                .failed_integrity_checks("SIZE (empty file)")
                .failed_automatic_fetching(false)
                );
    }
    else
    {
        Log::get_instance()->message(ll_debug, lc_context) << "Success for '" << u.filename() << "'";
        std::cout << "ok";
    }

    std::cout << std::endl;
}

const tr1::shared_ptr<const Sequence<FetchActionFailure> >
CheckFetchedFilesVisitor::failures() const
{
    return _imp->failures;
}

bool
CheckFetchedFilesVisitor::need_nofetch() const
{
    return _imp->need_nofetch;
}

