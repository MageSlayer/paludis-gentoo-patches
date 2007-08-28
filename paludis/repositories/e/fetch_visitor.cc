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
    struct Implementation<FetchVisitor>
    {
        const Environment * const env;
        const tr1::shared_ptr<const PackageID> id;
        const EAPI & eapi;
        const FSEntry distdir;
        const bool fetch_unneeded;
        const bool userpriv;
        const std::string mirrors_name;
        const bool fetch_restrict;
        const bool no_mirror;

        tr1::shared_ptr<LabelsDepSpec<URILabelVisitorTypes> > default_label;
        std::list<const LabelsDepSpec<URILabelVisitorTypes> *> labels;

        Implementation(
                const Environment * const e,
                const tr1::shared_ptr<const PackageID> & i,
                const EAPI & p,
                const FSEntry & d,
                const bool f,
                const bool u,
                const std::string & m,
                const bool n,
                const bool nm) :
            env(e),
            id(i),
            eapi(p),
            distdir(d),
            fetch_unneeded(f),
            userpriv(u),
            mirrors_name(m),
            fetch_restrict(n),
            no_mirror(nm)
        {
            if (fetch_restrict)
            {
                default_label.reset(new LabelsDepSpec<URILabelVisitorTypes>);
                default_label->add_label(make_shared_ptr(new URIManualOnlyLabel("fetch-restrict")));
            }
            else if (no_mirror)
            {
                default_label.reset(new LabelsDepSpec<URILabelVisitorTypes>);
                default_label->add_label(make_shared_ptr(new URIListedOnlyLabel("mirror-restrict")));
            }
            else
                default_label = parse_uri_label("default:", eapi);

            labels.push_front(default_label.get());
        }
    };
}

FetchVisitor::FetchVisitor(
        const Environment * const e,
        const tr1::shared_ptr<const PackageID> & i,
        const EAPI & p,
        const FSEntry & d,
        const bool f,
        const bool u,
        const std::string & m,
        const bool n,
        const bool nm) :
    PrivateImplementationPattern<FetchVisitor>(new Implementation<FetchVisitor>(e, i, p, d, f, u, m, n, nm))
{
}

FetchVisitor::~FetchVisitor()
{
}

void
FetchVisitor::visit_sequence(const UseDepSpec & u,
        URISpecTree::ConstSequenceIterator cur,
        URISpecTree::ConstSequenceIterator end)
{
    if ((_imp->fetch_unneeded) || (_imp->env->query_use(u.flag(), *_imp->id) ^ u.inverse()))
    {
        _imp->labels.push_front(* _imp->labels.begin());
        std::for_each(cur, end, accept_visitor(*this));
        _imp->labels.pop_front();
    }
}

void
FetchVisitor::visit_sequence(const AllDepSpec &,
        URISpecTree::ConstSequenceIterator cur,
        URISpecTree::ConstSequenceIterator end)
{
    _imp->labels.push_front(* _imp->labels.begin());
    std::for_each(cur, end, accept_visitor(*this));
    _imp->labels.pop_front();
}

void
FetchVisitor::visit_leaf(const LabelsDepSpec<URILabelVisitorTypes> & l)
{
    *_imp->labels.begin() = &l;
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
FetchVisitor::visit_leaf(const URIDepSpec & u)
{
    Context context("When visiting URI dep spec '" + stringify(u.text()) + "':");

    if (! *_imp->labels.begin())
        throw FetchActionError("No fetch action label available");
    if (1 != std::distance((*_imp->labels.begin())->begin(), (*_imp->labels.begin())->end()))
        throw FetchActionError("Fetch action label does not define exactly one behaviour");

    SourceURIFinder source_uri_finder(_imp->env, _imp->id->repository().get(),
            u.original_url(), u.filename(), _imp->mirrors_name);
    (*_imp->labels.begin())->begin()->accept(source_uri_finder);
    for (SourceURIFinder::Iterator i(source_uri_finder.begin()), i_end(source_uri_finder.end()) ;
            i != i_end ; ++i)
    {
        Context local_context("When fetching URI '" + stringify(i->first) + "' to '" + stringify(i->second) + ":");

        FSEntry destination(_imp->distdir / u.filename());

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
            Log::get_instance()->message(ll_warning, lc_context) << "URI part '" << i->first << "' has empty protocol";
            continue;
        }

        const tr1::shared_ptr<const FSEntrySequence> fetch_dirs(_imp->env->fetchers_dirs());
        bool found(false);
        for (FSEntrySequence::Iterator d(fetch_dirs->begin()), d_end(fetch_dirs->end()) ;
                d != d_end ; ++d)
            if (make_fetcher(*d, protocol).exists())
            {
                found = true;

                Command cmd(stringify(make_fetcher(*d, protocol)) + " '" + i->first + "' '" +
                        stringify(_imp->distdir / i->second) + "'");
                if (_imp->userpriv)
                    cmd.with_uid_gid(_imp->env->reduced_uid(), _imp->env->reduced_gid());

                tr1::shared_ptr<const FSEntrySequence> syncers_dirs(_imp->env->syncers_dirs());
                tr1::shared_ptr<const FSEntrySequence> bashrc_files(_imp->env->bashrc_files());
                tr1::shared_ptr<const FSEntrySequence> fetchers_dirs(_imp->env->fetchers_dirs());
                tr1::shared_ptr<const FSEntrySequence> hook_dirs(_imp->env->hook_dirs());

                cmd
                    .with_setenv("P", stringify(_imp->id->name().package) + "-" +
                            stringify(_imp->id->version().remove_revision()))
                    .with_setenv("PV", stringify(_imp->id->version().remove_revision()))
                    .with_setenv("PR", stringify(_imp->id->version().revision_only()))
                    .with_setenv("PN", stringify(_imp->id->name().package))
                    .with_setenv("PVR", stringify(_imp->id->version()))
                    .with_setenv("PF", stringify(_imp->id->name().package) + "-" +
                            stringify(_imp->id->version()))
                    .with_setenv("CATEGORY", stringify(_imp->id->name().category))
                    .with_setenv("REPOSITORY", stringify(_imp->id->repository()->name()))
                    .with_setenv("EAPI", stringify(_imp->eapi.name))
                    .with_setenv("SLOT", "")
                    .with_setenv("PKGMANAGER", PALUDIS_PACKAGE "-" + stringify(PALUDIS_VERSION_MAJOR) + "." +
                            stringify(PALUDIS_VERSION_MINOR) + "." +
                            stringify(PALUDIS_VERSION_MICRO) +
                            (std::string(PALUDIS_SUBVERSION_REVISION).empty() ?
                             std::string("") : "-r" + std::string(PALUDIS_SUBVERSION_REVISION)))
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

                std::cout << "Trying to fetch '" << i->first << "' to '" << i->second << "'..." << std::endl;
                if (0 != run_command(cmd))
                    destination.unlink();
                break;
            }

        if (! found)
            Log::get_instance()->message(ll_warning, lc_context) << "URI part '" << i->first << "' uses unknown protocol '"
                << protocol << "'";
    }
}

