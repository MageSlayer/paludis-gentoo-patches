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

#include <paludis/repositories/portage/portage_repository_ebin_entries.hh>
#include <paludis/repositories/portage/portage_repository.hh>

#include <paludis/config_file.hh>
#include <paludis/dep_atom_flattener.hh>
#include <paludis/ebin.hh>
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
    /**
     * Implementation data for PortageRepositoryEbinEntries.
     *
     * \ingroup grpportagerepository
     */
    template<>
    struct Implementation<PortageRepositoryEbinEntries> :
        InternalCounted<Implementation<PortageRepositoryEbinEntries> >
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

PortageRepositoryEbinEntries::PortageRepositoryEbinEntries(
        const Environment * const e, PortageRepository * const p, const PortageRepositoryParams & k) :
    PortageRepositoryEntries(".ebin"),
    PrivateImplementationPattern<PortageRepositoryEbinEntries>(new
            Implementation<PortageRepositoryEbinEntries>(e, p, k))
{
}

PortageRepositoryEbinEntries::~PortageRepositoryEbinEntries()
{
}

VersionMetadata::Pointer
PortageRepositoryEbinEntries::generate_version_metadata(const QualifiedPackageName & q,
        const VersionSpec & v) const
{
    VersionMetadata::Pointer result(new VersionMetadata::Ebin(PortageDepParser::parse_depend));

    FSEntry ebin_file(_imp->params.get<prpk_location>());
    ebin_file /= stringify(q.get<qpn_category>());
    ebin_file /= stringify(q.get<qpn_package>());
    ebin_file /= (stringify(q.get<qpn_package>()) + "-" + stringify(v) + ".ebin");

    PortageRepository::OurVirtualsIterator vi(_imp->portage_repository->end_our_virtuals());
    if (ebin_file.is_regular_file())
    {
        KeyValueConfigFile f(ebin_file);
        result->get<vm_deps>().set<vmd_build_depend_string>(f.get("depend"));
        result->get<vm_deps>().set<vmd_run_depend_string>(f.get("rdepend"));
        result->get<vm_deps>().set<vmd_post_depend_string>(f.get("pdepend"));
        result->set<vm_slot>(SlotName(f.get("slot")));
        result->set<vm_license>(f.get("license"));
        result->set<vm_eapi>(f.get("eapi"));
        result->set<vm_homepage>(f.get("homepage"));
        result->set<vm_description>(f.get("description"));
        result->get_ebuild_interface()->set<evm_provide>(f.get("provide"));
        result->get_ebuild_interface()->set<evm_restrict>(f.get("restrict"));
        result->get_ebuild_interface()->set<evm_keywords>(f.get("keywords"));
        result->get_ebuild_interface()->set<evm_iuse>(f.get("iuse"));
        result->get_ebuild_interface()->set<evm_virtual>(f.get("virtual"));
        result->get_ebuild_interface()->set<evm_inherited>(f.get("inherited"));
        result->get_ebin_interface()->set<ebvm_bin_uri>(f.get("bin_uri"));
        result->get_ebin_interface()->set<ebvm_src_repository>(RepositoryName(f.get("src_repository")));
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
    }
    else
        throw NoSuchPackageError(stringify(PackageDatabaseEntry(q, v, _imp->portage_repository->name())));

    return result;

}

void
PortageRepositoryEbinEntries::install(const QualifiedPackageName & q, const VersionSpec & v,
        const InstallOptions & o, PortageRepositoryProfile::ConstPointer p) const
{
    VersionMetadata::ConstPointer metadata(0);
    if (! _imp->portage_repository->has_version(q, v))
    {
        if (q.get<qpn_category>() == CategoryNamePart("virtual"))
        {
            VersionMetadata::Ebin::Pointer m(new VersionMetadata::Ebin(PortageDepParser::parse_depend));
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

    std::string binaries, flat_bin_uri;
    {
        std::set<std::string> already_in_binaries;

        /* make B */
        DepAtom::ConstPointer f_atom(PortageDepParser::parse(metadata->get_ebin_interface()->get<ebvm_bin_uri>(),
                    PortageDepParserPolicy<PlainTextDepAtom, false>::get_instance()));
        DepAtomFlattener f(_imp->params.get<prpk_environment>(), &e, f_atom);

        for (DepAtomFlattener::Iterator ff(f.begin()), ff_end(f.end()) ; ff != ff_end ; ++ff)
        {
            std::string::size_type p((*ff)->text().rfind('/'));
            if (std::string::npos == p)
            {
                if (already_in_binaries.end() == already_in_binaries.find((*ff)->text()))
                {
                    binaries.append((*ff)->text());
                    already_in_binaries.insert((*ff)->text());
                }
            }
            else
            {
                if (already_in_binaries.end() == already_in_binaries.find((*ff)->text().substr(p + 1)))
                {
                    binaries.append((*ff)->text().substr(p + 1));
                    already_in_binaries.insert((*ff)->text().substr(p + 1));
                }
            }
            binaries.append(" ");

            /* add * mirror entries */
            for (Environment::MirrorIterator
                    m(_imp->params.get<prpk_environment>()->begin_mirrors("*")),
                    m_end(_imp->params.get<prpk_environment>()->end_mirrors("*")) ;
                    m != m_end ; ++m)
                flat_bin_uri.append(m->second + "/" + (*ff)->text().substr(p + 1) + " ");

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
                    flat_bin_uri.append(m->second + "/" + mirror.substr(q + 1) + " ");

                for (Repository::MirrorInterface::MirrorsIterator
                        m(_imp->portage_repository->begin_mirrors(mirror.substr(0, q))),
                        m_end(_imp->portage_repository->end_mirrors(mirror.substr(0, q))) ;
                        m != m_end ; ++m)
                    flat_bin_uri.append(m->second + "/" + mirror.substr(q + 1) + " ");
            }
            else
                flat_bin_uri.append((*ff)->text());
            flat_bin_uri.append(" ");

            /* add mirror://gentoo/ entries */
            std::string master_mirror(strip_trailing_string(stringify(_imp->portage_repository->name()), "x-"));
            if (_imp->portage_repository->is_mirror(master_mirror))
            {
                for (Environment::MirrorIterator
                        m(_imp->params.get<prpk_environment>()->begin_mirrors(master_mirror)),
                        m_end(_imp->params.get<prpk_environment>()->end_mirrors(master_mirror)) ;
                        m != m_end ; ++m)
                    flat_bin_uri.append(m->second + "/" + (*ff)->text().substr(p + 1) + " ");

                for (Repository::MirrorInterface::MirrorsIterator
                        m(_imp->portage_repository->begin_mirrors(master_mirror)),
                        m_end(_imp->portage_repository->end_mirrors(master_mirror)) ;
                        m != m_end ; ++m)
                    flat_bin_uri.append(m->second + "/" + (*ff)->text().substr(p + 1) + " ");
            }
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

    binaries = strip_trailing(binaries, " ");

    EbinFetchCommand fetch_cmd(EbinCommandParams::create((
                    param<ebcpk_environment>(_imp->params.get<prpk_environment>()),
                    param<ebcpk_db_entry>(&e),
                    param<ebcpk_src_repository>(metadata->get_ebin_interface()->get<ebvm_src_repository>()),
                    param<ebcpk_ebin_dir>(_imp->params.get<prpk_location>() / stringify(q.get<qpn_category>()) /
                        stringify(q.get<qpn_package>())),
                    param<ebcpk_pkgdir>(_imp->params.get<prpk_pkgdir>()),
                    param<ebcpk_buildroot>(_imp->params.get<prpk_buildroot>())
                    )),
            EbinFetchCommandParams::create((
                    param<ebcfpk_b>(binaries),
                    param<ebcfpk_flat_bin_uri>(flat_bin_uri),
                    param<ebcfpk_root>(stringify(_imp->params.get<prpk_root>()) + "/"),
                    param<ebcfpk_profiles>(_imp->params.get<prpk_profiles>())
                    )));

    if (metadata->get_ebuild_interface()->get<evm_virtual>().empty())
        fetch_cmd();

    if (o.get<io_fetchonly>())
        return;

    EbinInstallCommand install_cmd(EbinCommandParams::create((
                    param<ebcpk_environment>(_imp->params.get<prpk_environment>()),
                    param<ebcpk_db_entry>(&e),
                    param<ebcpk_src_repository>(metadata->get_ebin_interface()->get<ebvm_src_repository>()),
                    param<ebcpk_ebin_dir>(_imp->params.get<prpk_location>() / stringify(q.get<qpn_category>()) /
                        stringify(q.get<qpn_package>())),
                    param<ebcpk_pkgdir>(_imp->params.get<prpk_pkgdir>()),
                    param<ebcpk_buildroot>(_imp->params.get<prpk_buildroot>())
                    )),
            EbinInstallCommandParams::create((
                    param<ebcipk_b>(binaries),
                    param<ebcipk_use>(use),
                    param<ebcipk_use_expand>(join(
                            p->begin_use_expand(),
                            p->end_use_expand(), " ")),
                    param<ebcipk_expand_vars>(expand_vars),
                    param<ebcipk_root>(stringify(_imp->params.get<prpk_root>()) + "/"),
                    param<ebcipk_profiles>(_imp->params.get<prpk_profiles>()),
                    param<ebcipk_disable_cfgpro>(o.get<io_noconfigprotect>()),
                    param<ebcipk_merge_only>(! metadata->get_ebuild_interface()->get<evm_virtual>().empty()),
                    param<ebcipk_slot>(SlotName(metadata->get<vm_slot>()))
                    )));

    install_cmd();
}

std::string
PortageRepositoryEbinEntries::get_environment_variable(const QualifiedPackageName &,
        const VersionSpec &, const std::string &,
        PortageRepositoryProfile::ConstPointer) const
{
    return "todo";
}

PortageRepositoryEbinEntries::Pointer
PortageRepositoryEbinEntries::make_portage_repository_ebin_entries(
        const Environment * const e, PortageRepository * const r, const PortageRepositoryParams & p)
{
    return Pointer(new PortageRepositoryEbinEntries(e, r, p));
}


