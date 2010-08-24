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

#include <paludis/repositories/e/check_fetched_files_visitor.hh>
#include <paludis/repositories/e/memoised_hashes.hh>
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
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/fs_stat.hh>
#include <paludis/util/log.hh>
#include <paludis/util/join.hh>
#include <paludis/util/save.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/rmd160.hh>
#include <paludis/util/sha1.hh>
#include <paludis/util/sha256.hh>
#include <paludis/util/md5.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/accept_visitor.hh>
#include <paludis/util/safe_ifstream.hh>
#include <paludis/output_manager.hh>
#include <algorithm>
#include <list>
#include <set>

using namespace paludis;
using namespace paludis::erepository;

namespace paludis
{
    template <>
    struct Imp<CheckFetchedFilesVisitor>
    {
        const Environment * const env;
        const std::shared_ptr<const PackageID> id;
        const FSPath distdir;
        const bool check_unneeded;
        const bool exclude_unmirrorable;
        const bool ignore_unfetched;
        const bool ignore_not_in_manifest;

        std::set<std::string> done;
        const std::shared_ptr<Sequence<FetchActionFailure> > failures;
        bool need_nofetch;
        bool in_nofetch;

        const std::shared_ptr<Manifest2Reader> m2r;
        const UseManifest use_manifest;
        const std::shared_ptr<OutputManager> output_manager;

        Imp(
                const Environment * const e,
                const std::shared_ptr<const PackageID> & i,
                const FSPath & d,
                const bool c,
                const bool n,
                const FSPath & m2,
                const UseManifest um,
                const std::shared_ptr<OutputManager> & md,
                const bool x,
                const bool u,
                const bool nm) :
            env(e),
            id(i),
            distdir(d),
            check_unneeded(c),
            exclude_unmirrorable(x),
            ignore_unfetched(u),
            ignore_not_in_manifest(nm),
            failures(std::make_shared<Sequence<FetchActionFailure>>()),
            need_nofetch(false),
            in_nofetch(n),
            m2r(std::make_shared<Manifest2Reader>(m2)),
            use_manifest(um),
            output_manager(md)
        {
        }
    };
}

CheckFetchedFilesVisitor::CheckFetchedFilesVisitor(
        const Environment * const e,
        const std::shared_ptr<const PackageID> & i,
        const FSPath & d,
        const bool c,
        const bool n,
        const FSPath & m2,
        const UseManifest um,
        const std::shared_ptr<OutputManager> & md,
        const bool x,
        const bool u,
        const bool nm) :
    Pimp<CheckFetchedFilesVisitor>(e, i, d, c, n, m2, um, md, x, u, nm)
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
CheckFetchedFilesVisitor::check_distfile_manifest(const FSPath & distfile)
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
                        n::failed_automatic_fetching() = false,
                        n::failed_integrity_checks() = "No Manifest available",
                        n::requires_manual_fetching() = false,
                        n::target_file() = stringify(distfile.basename())
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

        FSStat distfile_stat(distfile);

        Log::get_instance()->message("e.manifest.size", ll_debug, lc_context)
            << "Actual size = " << distfile_stat.file_size()
            << "; Manifest file size = " << m->size();
        if (distfile_stat.file_size() != m->size())
        {
            Log::get_instance()->message("e.manifest.no_size", ll_debug, lc_context)
                << "Malformed Manifest: no file size found";
            _imp->output_manager->stdout_stream() << "incorrect size";
            _imp->failures->push_back(make_named_values<FetchActionFailure>(
                    n::failed_automatic_fetching() = false,
                    n::failed_integrity_checks() = "Incorrect file size",
                    n::requires_manual_fetching() = false,
                    n::target_file() = stringify(distfile.basename())
                    ));
            return false;
        }

        try
        {
            SafeIFStream file_stream(distfile);

            MemoisedHashes * hashes = MemoisedHashes::get_instance();

            if (! m->rmd160().empty())
            {
                std::string rmd160hexsum(hashes->get<RMD160>(distfile, file_stream));

                if (rmd160hexsum != m->rmd160())
                {
                    Log::get_instance()->message("e.manifest.rmd160.failure", ll_debug, lc_context)
                        << "Malformed Manifest: failed RMD160 checksum";
                    _imp->output_manager->stdout_stream() << "failed RMD160";
                    _imp->failures->push_back(make_named_values<FetchActionFailure>(
                            n::failed_automatic_fetching() = false,
                            n::failed_integrity_checks() = "Failed RMD160 checksum",
                            n::requires_manual_fetching() = false,
                            n::target_file() = stringify(distfile.basename())
                            ));
                    return false;
                }
                Log::get_instance()->message("e.manifest.rmd160.result", ll_debug, lc_context)
                    << "Actual RMD160 = " << rmd160hexsum;
            }

            if (! m->sha1().empty())
            {
                std::string sha1hexsum(hashes->get<SHA1>(distfile, file_stream));

                if (sha1hexsum != m->sha1())
                {
                    Log::get_instance()->message("e.manifest.sha1.failure", ll_debug, lc_context)
                        << "Malformed Manifest: failed SHA1 checksum";
                    _imp->output_manager->stdout_stream() << "failed SHA1";
                    _imp->failures->push_back(make_named_values<FetchActionFailure>(
                            n::failed_automatic_fetching() = false,
                            n::failed_integrity_checks() = "Failed SHA1 checksum",
                            n::requires_manual_fetching() = false,
                            n::target_file() = stringify(distfile.basename())
                            ));
                    return false;
                }
                Log::get_instance()->message("e.manifest.sha1.result", ll_debug, lc_context)
                    << "Actual SHA1 = " << sha1hexsum;
            }

            if (! m->sha256().empty())
            {
                std::string sha256hexsum(hashes->get<SHA256>(distfile, file_stream));

                if (sha256hexsum != m->sha256())
                {
                    Log::get_instance()->message("e.manifest.sha256.failure", ll_debug, lc_context)
                        << "Malformed Manifest: failed SHA256 checksum";
                    _imp->output_manager->stdout_stream() << "failed SHA256";
                    _imp->failures->push_back(make_named_values<FetchActionFailure>(
                                n::failed_automatic_fetching() = false,
                                n::failed_integrity_checks() = "Failed SHA256 checksum",
                                n::requires_manual_fetching() = false,
                                n::target_file() = stringify(distfile.basename())
                            ));
                    return false;
                }
                Log::get_instance()->message("e.manifest.sha256.result", ll_debug, lc_context)
                    << "Actual SHA256 = " << sha256hexsum;
            }

            if (! m->md5().empty())
            {
                std::string md5hexsum(hashes->get<MD5>(distfile, file_stream));

                if (md5hexsum != m->md5())
                {
                    Log::get_instance()->message("e.manifest.md5.failure", ll_debug, lc_context)
                        << "Malformed Manifest: failed MD5 checksum";
                    _imp->output_manager->stdout_stream() << "failed MD5";
                    _imp->failures->push_back(make_named_values<FetchActionFailure>(
                            n::failed_automatic_fetching() = false,
                            n::failed_integrity_checks() = "Failed MD5 checksum",
                            n::requires_manual_fetching() = false,
                            n::target_file() = stringify(distfile.basename())
                            ));
                    return false;
                }
                Log::get_instance()->message("e.manifest.md5.result", ll_debug, lc_context)
                    << "Actual MD5 = " << md5hexsum;
            }
        }
        catch (const SafeIFStreamError &)
        {
            _imp->output_manager->stdout_stream() << "unreadable file";
            _imp->failures->push_back(make_named_values<FetchActionFailure>(
                        n::failed_automatic_fetching() = true,
                        n::failed_integrity_checks() = "Unreadable file",
                        n::requires_manual_fetching() = false,
                        n::target_file() = stringify(distfile.basename())
                        ));
            return false;
        }
    }

    if ((! found) && (! _imp->ignore_not_in_manifest))
    {
        _imp->output_manager->stdout_stream() << "not in Manifest";
        _imp->failures->push_back(make_named_values<FetchActionFailure>(
                    n::failed_automatic_fetching() = false,
                    n::failed_integrity_checks() = "Not in Manifest",
                    n::requires_manual_fetching() = false,
                    n::target_file() = stringify(distfile.basename())
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

    _imp->output_manager->stdout_stream() << "Checking '" << node.spec()->filename() << "'... " << std::flush;

    FSStat distfile_stat(_imp->distdir / node.spec()->filename());
    if (! distfile_stat.is_regular_file())
    {
        if (_imp->in_nofetch)
        {
            if (! _imp->exclude_unmirrorable)
            {
                Log::get_instance()->message("e.check_fetched_files.requires_manual", ll_debug, lc_context)
                    << "Manual fetch required for '" << node.spec()->filename() << "'";
                _imp->output_manager->stdout_stream() << "requires manual fetch";
                _imp->need_nofetch = true;
                _imp->failures->push_back(make_named_values<FetchActionFailure>(
                        n::failed_automatic_fetching() = false,
                        n::failed_integrity_checks() = "",
                        n::requires_manual_fetching() = true,
                        n::target_file() = node.spec()->filename()
                        ));
            }
        }
        else if (! _imp->ignore_unfetched)
        {
            Log::get_instance()->message("e.check_fetched_files.does_not_exist", ll_debug, lc_context)
                << "Automatic fetch failed for '" << node.spec()->filename() << "'";
            _imp->output_manager->stdout_stream() << "does not exist";
            _imp->failures->push_back(make_named_values<FetchActionFailure>(
                        n::failed_automatic_fetching() = true,
                        n::failed_integrity_checks() = "",
                        n::requires_manual_fetching() = false,
                        n::target_file() = node.spec()->filename()
                        ));
        }
        else
            _imp->output_manager->stdout_stream() << "not fetched yet";
    }
    else if (0 == distfile_stat.file_size())
    {
        Log::get_instance()->message("e.check_fetched_files.empty", ll_debug, lc_context) << "Empty file for '" << node.spec()->filename() << "'";
        _imp->output_manager->stdout_stream() << "empty file";
        _imp->failures->push_back(make_named_values<FetchActionFailure>(
                    n::failed_automatic_fetching() = false,
                    n::failed_integrity_checks() = "SIZE (empty file)",
                    n::requires_manual_fetching() = false,
                    n::target_file() = node.spec()->filename()
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
        _imp->output_manager->stdout_stream() << "ok";
    }

    _imp->output_manager->stdout_stream() << std::endl;
}

const std::shared_ptr<const Sequence<FetchActionFailure> >
CheckFetchedFilesVisitor::failures() const
{
    return _imp->failures;
}

bool
CheckFetchedFilesVisitor::need_nofetch() const
{
    return _imp->need_nofetch;
}

