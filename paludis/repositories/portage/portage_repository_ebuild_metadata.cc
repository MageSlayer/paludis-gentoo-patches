/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#include <paludis/repositories/portage/portage_repository_ebuild_metadata.hh>
#include <paludis/repositories/portage/portage_repository.hh>

#include <paludis/ebuild.hh>
#include <paludis/portage_dep_parser.hh>
#include <paludis/version_metadata.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/log.hh>
#include <paludis/util/tokeniser.hh>

#include <fstream>
#include <list>

using namespace paludis;

namespace paludis
{
    template<>
    struct Implementation<PortageRepositoryEbuildMetadata> :
        InternalCounted<Implementation<PortageRepositoryEbuildMetadata> >
    {
        const Environment * const environment;
        PortageRepository * const portage_repository;
        const PortageRepositoryParams params;

        Implementation(const Environment * const e, PortageRepository * const p,
                const PortageRepositoryParams & k) :
            environment(e),
            portage_repository(p),
            params(k)
        {
        }
    };
}

PortageRepositoryEbuildMetadata::PortageRepositoryEbuildMetadata(
        const Environment * const e, PortageRepository * const p, const PortageRepositoryParams & k) :
    PortageRepositoryMetadata(".ebuild"),
    PrivateImplementationPattern<PortageRepositoryEbuildMetadata>(new
            Implementation<PortageRepositoryEbuildMetadata>(e, p, k))
{
}

PortageRepositoryEbuildMetadata::~PortageRepositoryEbuildMetadata()
{
}

VersionMetadata::Pointer
PortageRepositoryEbuildMetadata::generate_version_metadata(const QualifiedPackageName & q,
        const VersionSpec & v) const
{
    VersionMetadata::Pointer result(new VersionMetadata::Ebuild(PortageDepParser::parse_depend));

    FSEntry cache_file(_imp->params.get<prpk_cache>());
    cache_file /= stringify(q.get<qpn_category>());
    cache_file /= stringify(q.get<qpn_package>()) + "-" + stringify(v);

    bool ok(false);
    PortageRepository::OurVirtualsIterator vi(_imp->portage_repository->end_our_virtuals());
    if (cache_file.is_regular_file())
    {
        std::ifstream cache(stringify(cache_file).c_str());
        std::string line;

        if (cache)
        {
            std::getline(cache, line); result->get<vm_deps>().set<vmd_build_depend_string>(line);
            std::getline(cache, line); result->get<vm_deps>().set<vmd_run_depend_string>(line);
            std::getline(cache, line); result->set<vm_slot>(SlotName(line));
            std::getline(cache, line); result->get_ebuild_interface()->set<evm_src_uri>(line);
            std::getline(cache, line); result->get_ebuild_interface()->set<evm_restrict>(line);
            std::getline(cache, line); result->set<vm_homepage>(line);
            std::getline(cache, line); result->set<vm_license>(line);
            std::getline(cache, line); result->set<vm_description>(line);
            std::getline(cache, line); result->get_ebuild_interface()->set<evm_keywords>(line);
            std::getline(cache, line); result->get_ebuild_interface()->set<evm_inherited>(line);
            std::getline(cache, line); result->get_ebuild_interface()->set<evm_iuse>(line);
            std::getline(cache, line);
            std::getline(cache, line); result->get<vm_deps>().set<vmd_post_depend_string>(line);
            std::getline(cache, line); result->get_ebuild_interface()->set<evm_provide>(line);
            std::getline(cache, line); result->set<vm_eapi>(line);
            result->get_ebuild_interface()->set<evm_virtual>("");

            // check mtimes
            time_t cache_time(cache_file.mtime());
            ok = true;

            if ((_imp->params.get<prpk_location>() / stringify(q.get<qpn_category>()) /
                        stringify(q.get<qpn_package>()) /
                        (stringify(q.get<qpn_package>()) + "-" + stringify(v)
                            + ".ebuild")).mtime() > cache_time)
                ok = false;
            else
            {
                FSEntry timestamp(_imp->params.get<prpk_location>() / "metadata" / "timestamp");
                if (timestamp.exists())
                    cache_time = timestamp.mtime();

                std::list<std::string> inherits;
                WhitespaceTokeniser::get_instance()->tokenise(
                        stringify(result->get_ebuild_interface()->get<evm_inherited>()),
                        std::back_inserter(inherits));
                for (FSEntryCollection::Iterator e(_imp->params.get<prpk_eclassdirs>()->begin()),
                        e_end(_imp->params.get<prpk_eclassdirs>()->end()) ; e != e_end ; ++e)
                    for (std::list<std::string>::const_iterator i(inherits.begin()),
                            i_end(inherits.end()) ; i != i_end ; ++i)
                    {
                        if ((*e / (*i + ".eclass")).exists())
                            if (((*e / (*i + ".eclass"))).mtime() > cache_time)
                                ok = false;
                    }
            }

            if (! ok)
                Log::get_instance()->message(ll_warning, lc_no_context, "Stale cache file at '"
                        + stringify(cache_file) + "'");
        }
        else
            Log::get_instance()->message(ll_warning, lc_no_context,
                    "Couldn't read the cache file at '"
                    + stringify(cache_file) + "'");
    }
    else if (_imp->portage_repository->end_our_virtuals() !=
            ((vi = _imp->portage_repository->find_our_virtuals(q))))
    {
        VersionMetadata::ConstPointer m(_imp->portage_repository->version_metadata(
                    vi->second->package(), v));
        result->set<vm_slot>(m->get<vm_slot>());
        result->get_ebuild_interface()->set<evm_keywords>(m->get_ebuild_interface()->get<evm_keywords>());
        result->set<vm_eapi>(m->get<vm_eapi>());
        result->get_ebuild_interface()->set<evm_virtual>(stringify(vi->second->package()));
        result->get<vm_deps>().set<vmd_build_depend_string>(
                "=" + stringify(vi->second->package()) + "-" + stringify(v));
        ok = true;
    }

    if (! ok)
    {
        if (_imp->params.get<prpk_cache>().basename() != "empty")
            Log::get_instance()->message(ll_warning, lc_no_context,
                    "No usable cache entry for '" + stringify(q) +
                    "-" + stringify(v) + "' in '" + stringify(_imp->portage_repository->name()) + "'");

        PackageDatabaseEntry e(q, v, _imp->portage_repository->name());
        EbuildMetadataCommand cmd(EbuildCommandParams::create((
                        param<ecpk_environment>(_imp->environment),
                        param<ecpk_db_entry>(&e),
                        param<ecpk_ebuild_dir>(_imp->params.get<prpk_location>() / stringify(q.get<qpn_category>()) /
                            stringify(q.get<qpn_package>())),
                        param<ecpk_files_dir>(_imp->params.get<prpk_location>() / stringify(q.get<qpn_category>()) /
                            stringify(q.get<qpn_package>()) / "files"),
                        param<ecpk_eclassdirs>(_imp->params.get<prpk_eclassdirs>()),
                        param<ecpk_portdir>(_imp->params.get<prpk_location>()),
                        param<ecpk_distdir>(_imp->params.get<prpk_distdir>()),
                        param<ecpk_buildroot>(_imp->params.get<prpk_buildroot>())
                        )));
        if (! cmd())
            Log::get_instance()->message(ll_warning, lc_no_context,
                    "No usable metadata for '" + stringify(q)
                    + "-" + stringify(v) + "' in '" + stringify(_imp->portage_repository->name()) + "'");

        if (0 == ((result = cmd.metadata())))
            throw InternalError(PALUDIS_HERE, "cmd.metadata() is zero pointer???");
    }

    return result;

}

