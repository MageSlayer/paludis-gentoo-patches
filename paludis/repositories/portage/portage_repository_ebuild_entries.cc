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

#include <paludis/repositories/portage/portage_repository_ebuild_entries.hh>
#include <paludis/repositories/portage/portage_repository.hh>

#include <paludis/dep_atom_flattener.hh>
#include <paludis/ebuild.hh>
#include <paludis/environment.hh>
#include <paludis/portage_dep_parser.hh>
#include <paludis/version_metadata.hh>
#include <paludis/util/collection_concrete.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/log.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/tokeniser.hh>

#include <fstream>
#include <list>
#include <set>

using namespace paludis;

namespace paludis
{
    template<>
    struct Implementation<PortageRepositoryEbuildEntries> :
        InternalCounted<Implementation<PortageRepositoryEbuildEntries> >
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

PortageRepositoryEbuildEntries::PortageRepositoryEbuildEntries(
        const Environment * const e, PortageRepository * const p, const PortageRepositoryParams & k) :
    PortageRepositoryEntries(".ebuild"),
    PrivateImplementationPattern<PortageRepositoryEbuildEntries>(new
            Implementation<PortageRepositoryEbuildEntries>(e, p, k))
{
}

PortageRepositoryEbuildEntries::~PortageRepositoryEbuildEntries()
{
}

VersionMetadata::Pointer
PortageRepositoryEbuildEntries::generate_version_metadata(const QualifiedPackageName & q,
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

namespace
{
    class AAFinder :
        private InstantiationPolicy<AAFinder, instantiation_method::NonCopyableTag>,
        protected DepAtomVisitorTypes::ConstVisitor
    {
        private:
            mutable std::list<const StringDepAtom *> _atoms;

        protected:
            void visit(const AllDepAtom * a)
            {
                std::for_each(a->begin(), a->end(), accept_visitor(
                            static_cast<DepAtomVisitorTypes::ConstVisitor *>(this)));
            }

            void visit(const AnyDepAtom *) PALUDIS_ATTRIBUTE((noreturn))
            {
                throw InternalError(PALUDIS_HERE, "Found unexpected AnyDepAtom");
            }

            void visit(const UseDepAtom * a)
            {
                std::for_each(a->begin(), a->end(), accept_visitor(
                            static_cast<DepAtomVisitorTypes::ConstVisitor *>(this)));
            }

            void visit(const PlainTextDepAtom * a)
            {
                _atoms.push_back(a);
            }

            void visit(const PackageDepAtom * a)
            {
                _atoms.push_back(a);
            }

            void visit(const BlockDepAtom * a)
            {
                _atoms.push_back(a);
            }

        public:
            AAFinder(const DepAtom::ConstPointer a)
            {
                a->accept(static_cast<DepAtomVisitorTypes::ConstVisitor *>(this));
            }

            typedef std::list<const StringDepAtom *>::const_iterator Iterator;

            Iterator begin()
            {
                return _atoms.begin();
            }

            Iterator end() const
            {
                return _atoms.end();
            }
    };

}

void
PortageRepositoryEbuildEntries::install(const QualifiedPackageName & q, const VersionSpec & v,
        const InstallOptions & o, PortageRepositoryProfile::ConstPointer p) const
{
    VersionMetadata::ConstPointer metadata(0);
    if (! _imp->portage_repository->has_version(q, v))
    {
        if (q.get<qpn_category>() == CategoryNamePart("virtual"))
        {
            VersionMetadata::Ebuild::Pointer m(new VersionMetadata::Ebuild(PortageDepParser::parse_depend));
            m->set<vm_slot>(SlotName("0"));
            m->get_ebuild_interface()->set<evm_virtual>(" ");
            metadata = m;
        }
        else
            throw PackageInstallActionError("Can't install '" + stringify(q) + "-"
                    + stringify(v) + "' since has_version failed");
    }
    else
        metadata = _imp->portage_repository->version_metadata(q, v);

    PackageDatabaseEntry e(q, v, _imp->portage_repository->name());

    bool fetch_restrict(false), no_mirror(false);
    {
        std::list<std::string> restricts;
        WhitespaceTokeniser::get_instance()->tokenise(
                metadata->get_ebuild_interface()->get<evm_restrict>(), std::back_inserter(restricts));
        fetch_restrict = (restricts.end() != std::find(restricts.begin(), restricts.end(), "fetch")) ||
            (restricts.end() != std::find(restricts.begin(), restricts.end(), "nofetch"));
        no_mirror = (restricts.end() != std::find(restricts.begin(), restricts.end(), "mirror")) ||
            (restricts.end() != std::find(restricts.begin(), restricts.end(), "nomirror"));
    }

    std::string archives, all_archives, flat_src_uri;
    {
        std::set<std::string> already_in_archives;

        /* make A and FLAT_SRC_URI */
        DepAtom::ConstPointer f_atom(PortageDepParser::parse(metadata->get_ebuild_interface()->get<evm_src_uri>(),
                    PortageDepParserPolicy<PlainTextDepAtom, false>::get_instance()));
        DepAtomFlattener f(_imp->params.get<prpk_environment>(), &e, f_atom);

        for (DepAtomFlattener::Iterator ff(f.begin()), ff_end(f.end()) ; ff != ff_end ; ++ff)
        {
            std::string::size_type p((*ff)->text().rfind('/'));
            if (std::string::npos == p)
            {
                if (already_in_archives.end() == already_in_archives.find((*ff)->text()))
                {
                    archives.append((*ff)->text());
                    already_in_archives.insert((*ff)->text());
                }
            }
            else
            {
                if (already_in_archives.end() == already_in_archives.find((*ff)->text().substr(p + 1)))
                {
                    archives.append((*ff)->text().substr(p + 1));
                    already_in_archives.insert((*ff)->text().substr(p + 1));
                }
            }
            archives.append(" ");

            /* add * mirror entries */
            for (Environment::MirrorIterator
                    m(_imp->params.get<prpk_environment>()->begin_mirrors("*")),
                    m_end(_imp->params.get<prpk_environment>()->end_mirrors("*")) ;
                    m != m_end ; ++m)
                flat_src_uri.append(m->second + "/" + (*ff)->text().substr(p + 1) + " ");

            if (0 == (*ff)->text().compare(0, 9, "mirror://"))
            {
                std::string mirror((*ff)->text().substr(9));
                std::string::size_type q(mirror.find('/'));

                if (std::string::npos == q)
                    throw PackageInstallActionError("Can't install '" + stringify(q) + "-"
                            + stringify(v) + "' since SRC_URI is broken");

                if (! _imp->portage_repository->is_mirror(mirror.substr(0, q)))
                    throw PackageInstallActionError("Can't install '" + stringify(q) + "-"
                            + stringify(v) + "' since SRC_URI references unknown mirror:// '" +
                            mirror.substr(0, q) + "'");

                for (Environment::MirrorIterator
                        m(_imp->params.get<prpk_environment>()->begin_mirrors(mirror.substr(0, q))),
                        m_end(_imp->params.get<prpk_environment>()->end_mirrors(mirror.substr(0, q))) ;
                        m != m_end ; ++m)
                    flat_src_uri.append(m->second + "/" + mirror.substr(q + 1) + " ");

                for (Repository::MirrorInterface::MirrorsIterator
                        m(_imp->portage_repository->begin_mirrors(mirror.substr(0, q))),
                        m_end(_imp->portage_repository->end_mirrors(mirror.substr(0, q))) ;
                        m != m_end ; ++m)
                    flat_src_uri.append(m->second + "/" + mirror.substr(q + 1) + " ");
            }
            else
                flat_src_uri.append((*ff)->text());
            flat_src_uri.append(" ");

            /* add mirror://gentoo/ entries */
            std::string master_mirror(strip_trailing_string(stringify(_imp->portage_repository->name()), "x-"));
            if (! no_mirror && _imp->portage_repository->is_mirror(master_mirror))
            {
                for (Environment::MirrorIterator
                        m(_imp->params.get<prpk_environment>()->begin_mirrors(master_mirror)),
                        m_end(_imp->params.get<prpk_environment>()->end_mirrors(master_mirror)) ;
                        m != m_end ; ++m)
                    flat_src_uri.append(m->second + "/" + (*ff)->text().substr(p + 1) + " ");

                for (Repository::MirrorInterface::MirrorsIterator
                        m(_imp->portage_repository->begin_mirrors(master_mirror)),
                        m_end(_imp->portage_repository->end_mirrors(master_mirror)) ;
                        m != m_end ; ++m)
                    flat_src_uri.append(m->second + "/" + (*ff)->text().substr(p + 1) + " ");
            }
        }

        /* make AA */
        DepAtom::ConstPointer g_atom(PortageDepParser::parse(
                    metadata->get_ebuild_interface()->get<evm_src_uri>(),
                    PortageDepParserPolicy<PlainTextDepAtom, false>::get_instance()));
        AAFinder g(g_atom);
        std::set<std::string> already_in_all_archives;

        for (AAFinder::Iterator gg(g.begin()), gg_end(g.end()) ; gg != gg_end ; ++gg)
        {
            std::string::size_type p((*gg)->text().rfind('/'));
            if (std::string::npos == p)
            {
                if (already_in_all_archives.end() == already_in_all_archives.find((*gg)->text()))
                {
                    all_archives.append((*gg)->text());
                    already_in_all_archives.insert((*gg)->text());
                }
            }
            else
            {
                if (already_in_all_archives.end() == already_in_all_archives.find((*gg)->text().substr(p + 1)))
                {
                    all_archives.append((*gg)->text().substr(p + 1));
                    already_in_all_archives.insert((*gg)->text().substr(p + 1));
                }
            }
            all_archives.append(" ");
        }
    }

    std::string use;
    {
        std::set<UseFlagName> iuse;
        WhitespaceTokeniser::get_instance()->tokenise(metadata->get_ebuild_interface()->
                get<evm_iuse>(), create_inserter<UseFlagName>(std::inserter(iuse, iuse.begin())));
        for (std::set<UseFlagName>::const_iterator iuse_it(iuse.begin()), iuse_end(iuse.end()) ;
                iuse_it != iuse_end; ++iuse_it)
            if (_imp->params.get<prpk_environment>()->query_use(*iuse_it, &e))
                use += (*iuse_it).data() + " ";
    }

    use += p->environment_variable("ARCH") + " ";
    for (PortageRepositoryProfile::UseExpandIterator x(p->begin_use_expand()),
            x_end(p->end_use_expand()) ; x != x_end ; ++x)
    {
        std::string lower_x;
        std::transform(x->data().begin(), x->data().end(), std::back_inserter(lower_x),
                &::tolower);

        std::list<std::string> uses;
        WhitespaceTokeniser::get_instance()->tokenise(
                p->environment_variable(stringify(*x)),
                std::back_inserter(uses));

        for (std::list<std::string>::const_iterator u(uses.begin()), u_end(uses.end()) ;
                u != u_end ; ++u)
            use += lower_x + "_" + *u + " ";

        UseFlagNameCollection::Pointer u(_imp->params.get<prpk_environment>()->query_enabled_use_matching(
                    lower_x + "_", &e));
        for (UseFlagNameCollection::Iterator uu(u->begin()), uu_end(u->end()) ;
                uu != uu_end ; ++uu)
            use += stringify(*uu) + " ";
    }

    AssociativeCollection<std::string, std::string>::Pointer expand_vars(
            new AssociativeCollection<std::string, std::string>::Concrete);
    for (PortageRepositoryProfile::UseExpandIterator
            u(p->begin_use_expand()), u_end(p->end_use_expand()) ; u != u_end ; ++u)
    {
        std::string prefix;
        std::transform(u->data().begin(), u->data().end(), std::back_inserter(prefix),
                &::tolower);
        prefix.append("_");

        UseFlagNameCollection::Pointer x(_imp->params.get<prpk_environment>()->query_enabled_use_matching(prefix, &e));
        std::string value;
        for (UseFlagNameCollection::Iterator xx(x->begin()), xx_end(x->end()) ;
                xx != xx_end ; ++xx)
            value.append(stringify(*xx).erase(0, stringify(*u).length() + 1) + " ");

        expand_vars->insert(stringify(*u), value);
    }

    /* Strip trailing space. Some ebuilds rely upon this. From kde-meta.eclass:
     *     [[ -n ${A/${TARBALL}/} ]] && unpack ${A/${TARBALL}/}
     * Rather annoying.
     */
    archives = strip_trailing(archives, " ");
    all_archives = strip_trailing(all_archives, " ");

    EbuildFetchCommand fetch_cmd(EbuildCommandParams::create((
                    param<ecpk_environment>(_imp->params.get<prpk_environment>()),
                    param<ecpk_db_entry>(&e),
                    param<ecpk_ebuild_dir>(_imp->params.get<prpk_location>() / stringify(q.get<qpn_category>()) /
                        stringify(q.get<qpn_package>())),
                    param<ecpk_files_dir>(_imp->params.get<prpk_location>() / stringify(q.get<qpn_category>()) /
                        stringify(q.get<qpn_package>()) / "files"),
                    param<ecpk_eclassdirs>(_imp->params.get<prpk_eclassdirs>()),
                    param<ecpk_portdir>(_imp->params.get<prpk_location>()),
                    param<ecpk_distdir>(_imp->params.get<prpk_distdir>()),
                    param<ecpk_buildroot>(_imp->params.get<prpk_buildroot>())
                    )),
            EbuildFetchCommandParams::create((
                    param<ecfpk_a>(archives),
                    param<ecfpk_aa>(all_archives),
                    param<ecfpk_use>(use),
                    param<ecfpk_use_expand>(join(
                            p->begin_use_expand(),
                            p->end_use_expand(), " ")),
                    param<ecfpk_expand_vars>(expand_vars),
                    param<ecfpk_flat_src_uri>(flat_src_uri),
                    param<ecfpk_root>(stringify(_imp->params.get<prpk_root>()) + "/"),
                    param<ecfpk_profiles>(_imp->params.get<prpk_profiles>()),
                    param<ecfpk_no_fetch>(fetch_restrict)
                    )));

    if (metadata->get_ebuild_interface()->get<evm_virtual>().empty())
        fetch_cmd();

    if (o.get<io_fetchonly>())
        return;

    EbuildInstallCommand install_cmd(EbuildCommandParams::create((
                    param<ecpk_environment>(_imp->params.get<prpk_environment>()),
                    param<ecpk_db_entry>(&e),
                    param<ecpk_ebuild_dir>(_imp->params.get<prpk_location>() / stringify(q.get<qpn_category>()) /
                        stringify(q.get<qpn_package>())),
                    param<ecpk_files_dir>(_imp->params.get<prpk_location>() / stringify(q.get<qpn_category>()) /
                        stringify(q.get<qpn_package>()) / "files"),
                    param<ecpk_eclassdirs>(_imp->params.get<prpk_eclassdirs>()),
                    param<ecpk_portdir>(_imp->params.get<prpk_location>()),
                    param<ecpk_distdir>(_imp->params.get<prpk_distdir>()),
                    param<ecpk_buildroot>(_imp->params.get<prpk_buildroot>())
                    )),
            EbuildInstallCommandParams::create((
                    param<ecipk_use>(use),
                    param<ecipk_a>(archives),
                    param<ecipk_aa>(all_archives),
                    param<ecipk_use_expand>(join(
                            p->begin_use_expand(),
                            p->end_use_expand(), " ")),
                    param<ecipk_expand_vars>(expand_vars),
                    param<ecipk_root>(stringify(_imp->params.get<prpk_root>()) + "/"),
                    param<ecipk_profiles>(_imp->params.get<prpk_profiles>()),
                    param<ecipk_disable_cfgpro>(o.get<io_noconfigprotect>()),
                    param<ecipk_merge_only>(! metadata->get_ebuild_interface()->get<evm_virtual>().empty()),
                    param<ecipk_slot>(SlotName(metadata->get<vm_slot>()))
                    )));

    install_cmd();
}

