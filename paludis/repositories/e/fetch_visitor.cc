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

#include <paludis/repositories/e/fetch_visitor.hh>
#include <paludis/repositories/e/source_uri_finder.hh>
#include <paludis/repositories/e/e_repository_id.hh>
#include <paludis/repositories/e/eapi.hh>
#include <paludis/repositories/e/dep_parser.hh>
#include <paludis/dep_spec.hh>
#include <paludis/environment.hh>
#include <paludis/package_id.hh>
#include <paludis/action.hh>
#include <paludis/repository.hh>
#include <paludis/about.hh>
#include <paludis/util/system.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/log.hh>
#include <paludis/util/join.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/output_manager.hh>
#include <list>

using namespace paludis;
using namespace paludis::erepository;

namespace paludis
{
    template <>
    struct Implementation<FetchVisitor>
    {
        const Environment * const env;
        const std::shared_ptr<const PackageID> id;
        const EAPI & eapi;
        const FSEntry distdir;
        const bool fetch_unneeded;
        const bool userpriv;
        const std::string mirrors_name;
        std::shared_ptr<const URILabel> default_label;
        const bool safe_resume;
        const std::shared_ptr<OutputManager> output_manager;
        const GetMirrorsFunction get_mirrors_fn;

        std::list<const URILabel *> labels;

        Implementation(
                const Environment * const e,
                const std::shared_ptr<const PackageID> & i,
                const EAPI & p,
                const FSEntry & d,
                const bool f,
                const bool u,
                const std::string & m,
                const std::shared_ptr<const URILabel> & n,
                const bool sr,
                const std::shared_ptr<OutputManager> & md,
                const GetMirrorsFunction & g) :
            env(e),
            id(i),
            eapi(p),
            distdir(d),
            fetch_unneeded(f),
            userpriv(u),
            mirrors_name(m),
            default_label(n),
            safe_resume(sr),
            output_manager(md),
            get_mirrors_fn(g)
        {
            labels.push_front(default_label.get());
        }
    };
}

FetchVisitor::FetchVisitor(
        const Environment * const e,
        const std::shared_ptr<const PackageID> & i,
        const EAPI & p,
        const FSEntry & d,
        const bool f,
        const bool u,
        const std::string & m,
        const std::shared_ptr<const URILabel> & n,
        const bool sr,
        const std::shared_ptr<OutputManager> & md,
        const GetMirrorsFunction & g) :
    PrivateImplementationPattern<FetchVisitor>(e, i, p, d, f, u, m, n, sr, md, g)
{
}

FetchVisitor::~FetchVisitor()
{
}

void
FetchVisitor::visit(const FetchableURISpecTree::NodeType<ConditionalDepSpec>::Type & node)
{
    if ((_imp->fetch_unneeded) || (node.spec()->condition_met()))
    {
        _imp->labels.push_front(* _imp->labels.begin());
        std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
        _imp->labels.pop_front();
    }
}

void
FetchVisitor::visit(const FetchableURISpecTree::NodeType<AllDepSpec>::Type & node)
{
    _imp->labels.push_front(* _imp->labels.begin());
    std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
    _imp->labels.pop_front();
}

void
FetchVisitor::visit(const FetchableURISpecTree::NodeType<URILabelsDepSpec>::Type & node)
{
    for (URILabelsDepSpec::ConstIterator i(node.spec()->begin()), i_end(node.spec()->end()) ;
            i != i_end ; ++i)
        *_imp->labels.begin() = i->get();
}

namespace
{
    FSEntry make_fetcher(const FSEntry & d, const std::string & x)
    {
        std::string lower_x;
        std::transform(x.begin(), x.end(), std::back_inserter(lower_x), &::tolower);
        return d / ("do" + lower_x);
    }
}

void
FetchVisitor::visit(const FetchableURISpecTree::NodeType<FetchableURIDepSpec>::Type & node)
{
    Context context("When visiting URI dep spec '" + stringify(node.spec()->text()) + "':");

    if (! *_imp->labels.begin())
        throw ActionFailedError("No fetch action label available");

    SourceURIFinder source_uri_finder(_imp->env, _imp->id->repository().get(),
            node.spec()->original_url(), node.spec()->filename(), _imp->mirrors_name, _imp->get_mirrors_fn);
    (*_imp->labels.begin())->accept(source_uri_finder);
    for (SourceURIFinder::ConstIterator i(source_uri_finder.begin()), i_end(source_uri_finder.end()) ;
            i != i_end ; ++i)
    {
        Context local_context("When fetching URI '" + stringify(i->first) + "' to '" + stringify(i->second) + ":");

        FSEntry destination(_imp->distdir / node.spec()->filename());

        if (destination.exists())
        {
            if (0 == destination.file_size())
                destination.unlink();
            else
                return;
        }

        std::string::size_type protocol_pos(i->first.find("://"));
        if (std::string::npos == protocol_pos)
            continue;

        std::string protocol(i->first.substr(0, protocol_pos));
        if (protocol.empty())
        {
            Log::get_instance()->message("e.fetch_visitor.no_protocol", ll_warning, lc_context)
                << "URI part '" << i->first << "' has empty protocol";
            continue;
        }

        const std::shared_ptr<const FSEntrySequence> fetch_dirs(_imp->env->fetchers_dirs());
        bool found(false);
        for (FSEntrySequence::ConstIterator d(fetch_dirs->begin()), d_end(fetch_dirs->end()) ;
                d != d_end ; ++d)
            if (make_fetcher(*d, protocol).exists())
            {
                found = true;

                Command cmd(stringify(make_fetcher(*d, protocol)) + " '" + i->first + "' '" +
                        stringify(_imp->distdir / i->second) + "'");
                if (_imp->userpriv)
                    cmd.with_uid_gid(_imp->env->reduced_uid(), _imp->env->reduced_gid());

                std::shared_ptr<const FSEntrySequence> syncers_dirs(_imp->env->syncers_dirs());
                std::shared_ptr<const FSEntrySequence> bashrc_files(_imp->env->bashrc_files());
                std::shared_ptr<const FSEntrySequence> fetchers_dirs(_imp->env->fetchers_dirs());
                std::shared_ptr<const FSEntrySequence> hook_dirs(_imp->env->hook_dirs());

                cmd
                    .with_setenv("P", stringify(_imp->id->name().package()) + "-" +
                            stringify(_imp->id->version().remove_revision()))
                    .with_setenv("PNV", stringify(_imp->id->name().package()) + "-" +
                            stringify(_imp->id->version().remove_revision()))
                    .with_setenv("PV", stringify(_imp->id->version().remove_revision()))
                    .with_setenv("PR", stringify(_imp->id->version().revision_only()))
                    .with_setenv("PN", stringify(_imp->id->name().package()))
                    .with_setenv("PVR", stringify(_imp->id->version()))
                    .with_setenv("PF", stringify(_imp->id->name().package()) + "-" +
                            stringify(_imp->id->version()))
                    .with_setenv("PNVR", stringify(_imp->id->name().package()) + "-" +
                            stringify(_imp->id->version()))
                    .with_setenv("CATEGORY", stringify(_imp->id->name().category()))
                    .with_setenv("REPOSITORY", stringify(_imp->id->repository()->name()))
                    .with_setenv("EAPI", stringify(_imp->eapi.name()))
                    .with_setenv("SLOT", "")
                    .with_setenv("PKGMANAGER", PALUDIS_PACKAGE "-" + stringify(PALUDIS_VERSION_MAJOR) + "." +
                            stringify(PALUDIS_VERSION_MINOR) + "." +
                            stringify(PALUDIS_VERSION_MICRO) +
                            (std::string(PALUDIS_GIT_HEAD).empty() ?
                             std::string("") : "-git-" + std::string(PALUDIS_GIT_HEAD)))
                    .with_setenv("PALUDIS_CONFIG_DIR", SYSCONFDIR "/paludis/")
                    .with_setenv("PALUDIS_BASHRC_FILES", join(bashrc_files->begin(), bashrc_files->end(), " "))
                    .with_setenv("PALUDIS_HOOK_DIRS", join(hook_dirs->begin(), hook_dirs->end(), " "))
                    .with_setenv("PALUDIS_FETCHERS_DIRS", join(fetchers_dirs->begin(), fetchers_dirs->end(), " "))
                    .with_setenv("PALUDIS_SYNCERS_DIRS", join(syncers_dirs->begin(), syncers_dirs->end(), " "))
                    .with_setenv("PALUDIS_COMMAND", _imp->env->paludis_command())
                    .with_setenv("PALUDIS_REDUCED_GID", stringify(_imp->env->reduced_gid()))
                    .with_setenv("PALUDIS_REDUCED_UID", stringify(_imp->env->reduced_uid()))
                    .with_setenv("PALUDIS_EBUILD_LOG_LEVEL", stringify(Log::get_instance()->log_level()))
                    .with_setenv("PALUDIS_EBUILD_DIR", getenv_with_default("PALUDIS_EBUILD_DIR", LIBEXECDIR "/paludis"));

                if (_imp->safe_resume)
                    cmd
                        .with_setenv("PALUDIS_USE_SAFE_RESUME", "yesplease");

                cmd
                    .with_captured_stderr_stream(&_imp->output_manager->stderr_stream())
                    .with_captured_stdout_stream(&_imp->output_manager->stdout_stream())
                    .with_ptys();

                _imp->output_manager->stdout_stream() << "Trying to fetch '" << i->first << "' to '" <<
                    i->second << "'..." << std::endl;

                if (0 != run_command(cmd))
                    destination.unlink();
                break;
            }

        if (! found)
            Log::get_instance()->message("e.fetch_visitor.unknown_protocol", ll_warning, lc_context)
                << "URI part '" << i->first << "' uses unknown protocol '"
                << protocol << "'";
    }
}

