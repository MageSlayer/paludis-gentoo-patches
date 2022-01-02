/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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
#include <paludis/output_manager.hh>

#include <paludis/util/system.hh>
#include <paludis/util/process.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/log.hh>
#include <paludis/util/join.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/fs_path.hh>
#include <paludis/util/fs_stat.hh>
#include <paludis/util/env_var_names.hh>
#include <paludis/util/upper_lower.hh>

#include <algorithm>
#include <list>

using namespace paludis;
using namespace paludis::erepository;

namespace paludis
{
    template <>
    struct Imp<FetchVisitor>
    {
        const Environment * const env;
        const std::shared_ptr<const PackageID> id;
        const EAPI & eapi;
        const FSPath distdir;
        const bool fetch_unneeded;
        const bool userpriv;
        const std::string mirrors_name;
        std::shared_ptr<const URILabel> default_label;
        const bool safe_resume;
        const std::shared_ptr<OutputManager> output_manager;
        const GetMirrorsFunction get_mirrors_fn;

        std::list<const URILabel *> labels;

        Imp(
                const Environment * const e,
                const std::shared_ptr<const PackageID> & i,
                const EAPI & p,
                const FSPath & d,
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
        const FSPath & d,
        const bool f,
        const bool u,
        const std::string & m,
        const std::shared_ptr<const URILabel> & n,
        const bool sr,
        const std::shared_ptr<OutputManager> & md,
        const GetMirrorsFunction & g) :
    _imp(e, i, p, d, f, u, m, n, sr, md, g)
{
}

FetchVisitor::~FetchVisitor() = default;

void
FetchVisitor::visit(const FetchableURISpecTree::NodeType<ConditionalDepSpec>::Type & node)
{
    if ((_imp->fetch_unneeded) || (node.spec()->condition_met(_imp->env, _imp->id)))
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
    for (const auto & i : *node.spec())
        *_imp->labels.begin() = i.get();
}

namespace
{
    FSPath make_fetcher(const FSPath & d, const std::string & x)
    {
        return d / ("do" + tolower(x));
    }
}

void
FetchVisitor::visit(const FetchableURISpecTree::NodeType<FetchableURIDepSpec>::Type & node)
{
    Context context("When visiting URI dep spec '" + stringify(node.spec()->text()) + "':");

    if (! *_imp->labels.begin())
        throw ActionFailedError("No fetch action label available");

    auto repo(_imp->env->fetch_repository(_imp->id->repository_name()));
    SourceURIFinder source_uri_finder(_imp->env, repo.get(), _imp->eapi,
            node.spec()->original_url(), node.spec()->filename(), _imp->mirrors_name, _imp->get_mirrors_fn);
    (*_imp->labels.begin())->accept(source_uri_finder);
    for (const auto & uri_to_filename : source_uri_finder)
    {
        Context local_context("When fetching URI '" + stringify(uri_to_filename.first) + "' to '" + stringify(uri_to_filename.second) + ":");

        FSPath destination(_imp->distdir / node.spec()->filename());

        FSStat destination_stat(destination);
        if (destination_stat.exists())
        {
            if (0 == destination_stat.file_size())
                destination.unlink();
            else
                return;
        }

        std::string::size_type protocol_pos(uri_to_filename.first.find("://"));
        if (std::string::npos == protocol_pos)
            continue;

        std::string protocol(uri_to_filename.first.substr(0, protocol_pos));
        if (protocol.empty())
        {
            Log::get_instance()->message("e.fetch_visitor.no_protocol", ll_warning, lc_context)
                << "URI part '" << uri_to_filename.first << "' has empty protocol";
            continue;
        }

        const std::shared_ptr<const FSPathSequence> fetch_dirs(_imp->env->fetchers_dirs());
        bool found(false);
        for (const auto & dir : *fetch_dirs)
            if (make_fetcher(dir, protocol).stat().exists())
            {
                found = true;

                Process fetch_process(ProcessCommand({ stringify(make_fetcher(dir, protocol)),
                            uri_to_filename.first, stringify(_imp->distdir / uri_to_filename.second) }));
                if (_imp->userpriv)
                    fetch_process.setuid_setgid(_imp->env->reduced_uid(), _imp->env->reduced_gid());

                std::shared_ptr<const FSPathSequence> syncers_dirs(_imp->env->syncers_dirs());
                std::shared_ptr<const FSPathSequence> bashrc_files(_imp->env->bashrc_files());
                std::shared_ptr<const FSPathSequence> fetchers_dirs(_imp->env->fetchers_dirs());
                std::shared_ptr<const FSPathSequence> hook_dirs(_imp->env->hook_dirs());

                fetch_process
                    .setenv("P", stringify(_imp->id->name().package()) + "-" +
                            stringify(_imp->id->version().remove_revision()))
                    .setenv("PNV", stringify(_imp->id->name().package()) + "-" +
                            stringify(_imp->id->version().remove_revision()))
                    .setenv("PV", stringify(_imp->id->version().remove_revision()))
                    .setenv("PR", stringify(_imp->id->version().revision_only()))
                    .setenv("PN", stringify(_imp->id->name().package()))
                    .setenv("PVR", stringify(_imp->id->version()))
                    .setenv("PF", stringify(_imp->id->name().package()) + "-" +
                            stringify(_imp->id->version()))
                    .setenv("PNVR", stringify(_imp->id->name().package()) + "-" +
                            stringify(_imp->id->version()))
                    .setenv("CATEGORY", stringify(_imp->id->name().category()))
                    .setenv("REPOSITORY", stringify(_imp->id->repository_name()))
                    .setenv("EAPI", stringify(_imp->eapi.name()))
                    .setenv("SLOT", "")
                    .setenv("PKGMANAGER", PALUDIS_PACKAGE "-" + stringify(PALUDIS_VERSION_MAJOR) + "." +
                            stringify(PALUDIS_VERSION_MINOR) + "." +
                            stringify(PALUDIS_VERSION_MICRO) +
                            (std::string(PALUDIS_GIT_HEAD).empty() ?
                             std::string("") : "-git-" + std::string(PALUDIS_GIT_HEAD)))
                    .setenv("PALUDIS_CONFIG_DIR", SYSCONFDIR "/paludis/")
                    .setenv("PALUDIS_BASHRC_FILES", join(bashrc_files->begin(), bashrc_files->end(), " "))
                    .setenv("PALUDIS_HOOK_DIRS", join(hook_dirs->begin(), hook_dirs->end(), " "))
                    .setenv("PALUDIS_FETCHERS_DIRS", join(fetchers_dirs->begin(), fetchers_dirs->end(), " "))
                    .setenv("PALUDIS_SYNCERS_DIRS", join(syncers_dirs->begin(), syncers_dirs->end(), " "))
                    .setenv("PALUDIS_REDUCED_GID", stringify(_imp->env->reduced_gid()))
                    .setenv("PALUDIS_REDUCED_UID", stringify(_imp->env->reduced_uid()))
                    .setenv("PALUDIS_EBUILD_LOG_LEVEL", stringify(Log::get_instance()->log_level()))
                    .setenv("PALUDIS_EBUILD_DIR", getenv_with_default(env_vars::ebuild_dir, LIBEXECDIR "/paludis"));

                if (_imp->safe_resume)
                    fetch_process
                        .setenv("PALUDIS_USE_SAFE_RESUME", "yesplease");

                fetch_process
                    .capture_stdout(_imp->output_manager->stderr_stream())
                    .capture_stderr(_imp->output_manager->stdout_stream())
                    .use_ptys();

                _imp->output_manager->stdout_stream() << "Trying to fetch '" << uri_to_filename.first << "' to '" <<
                    uri_to_filename.second << "'..." << std::endl;

                if (0 != fetch_process.run().wait())
                    destination.unlink();
                break;
            }

        if (! found)
            Log::get_instance()->message("e.fetch_visitor.unknown_protocol", ll_warning, lc_context)
                << "URI part '" << uri_to_filename.first << "' uses unknown protocol '"
                << protocol << "'";
    }
}
