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

#include <paludis/repositories/e/check_fetched_files_visitor.hh>
#include <paludis/repositories/e/source_uri_finder.hh>
#include <paludis/repositories/e/e_repository_id.hh>
#include <paludis/repositories/e/e_repository_params.hh>
#include <paludis/repositories/e/manifest2_reader.hh>
#include <paludis/dep_spec.hh>
#include <paludis/environment.hh>
#include <paludis/package_id.hh>
#include <paludis/repository.hh>
#include <paludis/about.hh>
#include <paludis/action.hh>
#include <paludis/util/system.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/log.hh>
#include <paludis/util/join.hh>
#include <paludis/util/save.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/rmd160.hh>
#include <paludis/util/sha1.hh>
#include <paludis/util/sha256.hh>
#include <paludis/util/md5.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/output_deviator.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/indirect_iterator.hh>
#include <paludis/util/accept_visitor.hh>
#include <algorithm>
#include <iostream>
#include <fstream>
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
        const std::tr1::shared_ptr<const PackageID> id;
        const FSEntry distdir;
        const bool check_unneeded;
        const bool exclude_unmirrorable;

        std::set<std::string> done;
        const std::tr1::shared_ptr<Sequence<FetchActionFailure> > failures;
        bool need_nofetch;
        bool in_nofetch;

        const std::tr1::shared_ptr<Manifest2Reader> m2r;
        const UseManifest use_manifest;
        const std::tr1::shared_ptr<OutputDeviant> maybe_output_deviant;
        std::ostream * out;

        Implementation(
                const Environment * const e,
                const std::tr1::shared_ptr<const PackageID> & i,
                const FSEntry & d,
                const bool c,
                const bool n,
                const FSEntry & m2,
                const UseManifest um,
                const std::tr1::shared_ptr<OutputDeviant> & md,
                const bool x) :
            env(e),
            id(i),
            distdir(d),
            check_unneeded(c),
            exclude_unmirrorable(x),
            failures(new Sequence<FetchActionFailure>),
            need_nofetch(false),
            in_nofetch(n),
            m2r(new Manifest2Reader(m2)),
            use_manifest(um),
            maybe_output_deviant(md),
            out(md ? maybe_output_deviant->stdout_stream() : &std::cout)
        {
        }
    };
}

CheckFetchedFilesVisitor::CheckFetchedFilesVisitor(
        const Environment * const e,
        const std::tr1::shared_ptr<const PackageID> & i,
        const FSEntry & d,
        const bool c,
        const bool n,
        const FSEntry & m2,
        const UseManifest um,
        const std::tr1::shared_ptr<OutputDeviant> & md,
        const bool x) :
    PrivateImplementationPattern<CheckFetchedFilesVisitor>(new Implementation<CheckFetchedFilesVisitor>(e, i, d, c, n, m2, um, md, x))
{
}

CheckFetchedFilesVisitor::~CheckFetchedFilesVisitor()
{
}

void
CheckFetchedFilesVisitor::visit(const FetchableURISpecTree::NodeType<ConditionalDepSpec>::Type & node)
{
    Save<bool> save_in_nofetch(&_imp->in_nofetch, _imp->in_nofetch);
    if ((_imp->check_unneeded) || (node.spec()->condition_met()))
        std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
}

void
CheckFetchedFilesVisitor::visit(const FetchableURISpecTree::NodeType<AllDepSpec>::Type & node)
{
    Save<bool> save_in_nofetch(&_imp->in_nofetch, _imp->in_nofetch);
    std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
}

namespace
{
    struct InNoFetchVisitor
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
CheckFetchedFilesVisitor::visit(const FetchableURISpecTree::NodeType<URILabelsDepSpec>::Type & node)
{
    InNoFetchVisitor v;
    std::for_each(indirect_iterator(node.spec()->begin()), indirect_iterator(node.spec()->end()), accept_visitor(v));
    _imp->in_nofetch = v.result;
}

bool
CheckFetchedFilesVisitor::check_distfile_manifest(const FSEntry & distfile)
{
    if (_imp->m2r->begin() == _imp->m2r->end())
    {
        switch (_imp->use_manifest)
        {
            case manifest_use:
            case manifest_ignore:
                Log::get_instance()->message("e.manifest.empty", ll_debug, lc_context) << "Empty or non-existent Manifest file";
                return true;

            case manifest_require:
            case last_manifest:
                _imp->failures->push_back(make_named_values<FetchActionFailure>(
                        value_for<n::failed_automatic_fetching>(false),
                        value_for<n::failed_integrity_checks>("No Manifest available"),
                        value_for<n::requires_manual_fetching>(false),
                        value_for<n::target_file>(stringify(distfile.basename()))
                        ));
                return false;

        }
    }

    if (manifest_ignore == _imp->use_manifest)
        return true;

    bool found(false);

    for (Manifest2Reader::ConstIterator m(_imp->m2r->begin()), m_end(_imp->m2r->end()) ;
        m != m_end ; ++m)
    {
        if (distfile.basename() != m->name())
            continue;
        found = true;

        Log::get_instance()->message("e.manifest.size", ll_debug, lc_context)
            << "Actual size = " << distfile.file_size()
            << "; Manifest file size = " << m->size();
        if (distfile.file_size() != m->size())
        {
            Log::get_instance()->message("e.manifest.no_size", ll_debug, lc_context)
                << "Malformed Manifest: no file size found";
            *_imp->out << "incorrect size";
            _imp->failures->push_back(make_named_values<FetchActionFailure>(
                    value_for<n::failed_automatic_fetching>(false),
                    value_for<n::failed_integrity_checks>("Incorrect file size"),
                    value_for<n::requires_manual_fetching>(false),
                    value_for<n::target_file>(stringify(distfile.basename()))
                    ));
            return false;
        }

        std::ifstream file_stream(stringify(distfile).c_str());
        if (! file_stream)
        {
            *_imp->out << "unreadable file";
            _imp->failures->push_back(make_named_values<FetchActionFailure>(
                    value_for<n::failed_automatic_fetching>(false),
                    value_for<n::failed_integrity_checks>("Unreadable file"),
                    value_for<n::requires_manual_fetching>(false),
                    value_for<n::target_file>(stringify(distfile.basename()))
                    ));
            return false;
        }

        if (! m->rmd160().empty())
        {
            RMD160 rmd160sum(file_stream);
            if (rmd160sum.hexsum() != m->rmd160())
            {
                Log::get_instance()->message("e.manifest.rmd160.failure", ll_debug, lc_context)
                    << "Malformed Manifest: failed RMD160 checksum";
                *_imp->out << "failed RMD160";
                _imp->failures->push_back(make_named_values<FetchActionFailure>(
                        value_for<n::failed_automatic_fetching>(false),
                        value_for<n::failed_integrity_checks>("Failed RMD160 checksum"),
                        value_for<n::requires_manual_fetching>(false),
                        value_for<n::target_file>(stringify(distfile.basename()))
                        ));
                return false;
            }
            Log::get_instance()->message("e.manifest.rmd160.result", ll_debug, lc_context)
                << "Actual RMD160 = " << rmd160sum.hexsum();
            file_stream.clear();
            file_stream.seekg(0, std::ios::beg);
        }

        if (! m->sha1().empty())
        {
            SHA1 sha1sum(file_stream);
            if (sha1sum.hexsum() != m->sha1())
            {
                Log::get_instance()->message("e.manifest.sha1.failure", ll_debug, lc_context)
                    << "Malformed Manifest: failed SHA1 checksum";
                *_imp->out << "failed SHA1";
                _imp->failures->push_back(make_named_values<FetchActionFailure>(
                        value_for<n::failed_automatic_fetching>(false),
                        value_for<n::failed_integrity_checks>("Failed SHA1 checksum"),
                        value_for<n::requires_manual_fetching>(false),
                        value_for<n::target_file>(stringify(distfile.basename()))
                        ));
                return false;
            }
            Log::get_instance()->message("e.manifest.sha1.result", ll_debug, lc_context)
                << "Actual SHA1 = " << sha1sum.hexsum();
            file_stream.clear();
            file_stream.seekg(0, std::ios::beg);
        }

        if (! m->sha256().empty())
        {
            SHA256 sha256sum(file_stream);
            if (sha256sum.hexsum() != m->sha256())
            {
                Log::get_instance()->message("e.manifest.sha256.failure", ll_debug, lc_context)
                    << "Malformed Manifest: failed SHA256 checksum";
                *_imp->out << "failed SHA256";
                _imp->failures->push_back(make_named_values<FetchActionFailure>(
                        value_for<n::failed_automatic_fetching>(false),
                        value_for<n::failed_integrity_checks>("Failed SHA256 checksum"),
                        value_for<n::requires_manual_fetching>(false),
                        value_for<n::target_file>(stringify(distfile.basename()))
                        ));
                return false;
            }
            Log::get_instance()->message("e.manifest.sha256.result", ll_debug, lc_context)
                << "Actual SHA256 = " << sha256sum.hexsum();
            file_stream.clear();
            file_stream.seekg(0, std::ios::beg);
        }

        if (! m->md5().empty())
        {
            MD5 md5sum(file_stream);
            if (md5sum.hexsum() != m->md5())
            {
                Log::get_instance()->message("e.manifest.md5.failure", ll_debug, lc_context)
                    << "Malformed Manifest: failed MD5 checksum";
                *_imp->out << "failed MD5";
                _imp->failures->push_back(make_named_values<FetchActionFailure>(
                        value_for<n::failed_automatic_fetching>(false),
                        value_for<n::failed_integrity_checks>("Failed MD5 checksum"),
                        value_for<n::requires_manual_fetching>(false),
                        value_for<n::target_file>(stringify(distfile.basename()))
                        ));
                return false;
            }
            Log::get_instance()->message("e.manifest.md5.result", ll_debug, lc_context)
                << "Actual MD5 = " << md5sum.hexsum();
        }
    }

    if (! found)
    {
        *_imp->out << "not in Manifest";
        _imp->failures->push_back(make_named_values<FetchActionFailure>(
                    value_for<n::failed_automatic_fetching>(false),
                    value_for<n::failed_integrity_checks>("Not in Manifest"),
                    value_for<n::requires_manual_fetching>(false),
                    value_for<n::target_file>(stringify(distfile.basename()))
                ));
        return false;
    }

    return true;
}

void
CheckFetchedFilesVisitor::visit(const FetchableURISpecTree::NodeType<FetchableURIDepSpec>::Type & node)
{
    Context context("When visiting URI dep spec '" + stringify(node.spec()->text()) + "':");

    if (_imp->done.end() != _imp->done.find(node.spec()->filename()))
    {
        Log::get_instance()->message("e.check_fetched_files.already_checked", ll_debug, lc_context)
            << "Already checked '" << node.spec()->filename() << "'";
        return;
    }
    _imp->done.insert(node.spec()->filename());

    *_imp->out << "Checking '" << node.spec()->filename() << "'... " << std::flush;

    if (! (_imp->distdir / node.spec()->filename()).is_regular_file())
    {
        if (_imp->in_nofetch)
        {
            if (! _imp->exclude_unmirrorable)
            {
                Log::get_instance()->message("e.check_fetched_files.requires_manual", ll_debug, lc_context)
                    << "Manual fetch required for '" << node.spec()->filename() << "'";
                *_imp->out << "requires manual fetch";
                _imp->need_nofetch = true;
                _imp->failures->push_back(make_named_values<FetchActionFailure>(
                        value_for<n::failed_automatic_fetching>(false),
                        value_for<n::failed_integrity_checks>(""),
                        value_for<n::requires_manual_fetching>(true),
                        value_for<n::target_file>(node.spec()->filename())
                        ));
            }
        }
        else
        {
            Log::get_instance()->message("e.check_fetched_files.does_not_exist", ll_debug, lc_context)
                << "Automatic fetch failed for '" << node.spec()->filename() << "'";
            *_imp->out << "does not exist";
            _imp->failures->push_back(make_named_values<FetchActionFailure>(
                        value_for<n::failed_automatic_fetching>(true),
                        value_for<n::failed_integrity_checks>(""),
                        value_for<n::requires_manual_fetching>(false),
                        value_for<n::target_file>(node.spec()->filename())
                        ));
        }
    }
    else if (0 == (_imp->distdir / node.spec()->filename()).file_size())
    {
        Log::get_instance()->message("e.check_fetched_files.empty", ll_debug, lc_context) << "Empty file for '" << node.spec()->filename() << "'";
        *_imp->out << "empty file";
        _imp->failures->push_back(make_named_values<FetchActionFailure>(
                value_for<n::failed_automatic_fetching>(false),
                value_for<n::failed_integrity_checks>("SIZE (empty file)"),
                value_for<n::requires_manual_fetching>(false),
                value_for<n::target_file>(node.spec()->filename())
                ));
    }
    else if (! check_distfile_manifest(_imp->distdir / node.spec()->filename()))
    {
        Log::get_instance()->message("e.check_fetched_files.failure", ll_debug, lc_context)
            << "Manifest check failed for '" << node.spec()->filename() << "'";
    }
    else
    {
        Log::get_instance()->message("e.check_fetched_files.success", ll_debug, lc_context) << "Success for '" << node.spec()->filename() << "'";
        *_imp->out << "ok";
    }

    *_imp->out << std::endl;
}

const std::tr1::shared_ptr<const Sequence<FetchActionFailure> >
CheckFetchedFilesVisitor::failures() const
{
    return _imp->failures;
}

bool
CheckFetchedFilesVisitor::need_nofetch() const
{
    return _imp->need_nofetch;
}

