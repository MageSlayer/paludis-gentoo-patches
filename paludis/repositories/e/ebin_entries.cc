/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh
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

#include "ebin_entries.hh"
#include <paludis/repositories/e/ebin.hh>
#include <paludis/repositories/e/e_repository.hh>
#include <paludis/repositories/e/eapi.hh>
#include <paludis/repositories/e/dep_parser.hh>
#include <paludis/repositories/e/dep_spec_pretty_printer.hh>
#include <paludis/util/config_file.hh>
#include <paludis/dep_spec_flattener.hh>
#include <paludis/environment.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/log.hh>
#include <paludis/util/system.hh>
#include <paludis/util/join.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/visitor-impl.hh>
#include <set>
#include <fstream>

using namespace paludis;

#if 0

namespace paludis
{
    template<>
    struct Implementation<EbinEntries>
    {
        const Environment * const environment;
        ERepository * const portage_repository;
        const ERepositoryParams params;

        Implementation(const Environment * const e, ERepository * const p,
                const ERepositoryParams & k) :
            environment(e),
            portage_repository(p),
            params(k)
        {
        }
    };
}

EbinEntries::EbinEntries(
        const Environment * const e, ERepository * const p, const ERepositoryParams & k) :
    PrivateImplementationPattern<EbinEntries>(new Implementation<EbinEntries>(e, p, k))
{
}

EbinEntries::~EbinEntries()
{
}

tr1::shared_ptr<VersionMetadata>
EbinEntries::generate_version_metadata(const QualifiedPackageName & q,
        const VersionSpec & v) const
{
    Context context("When generating version metadata for '" + stringify(q) + "-" + stringify(v) + "':");

    tr1::shared_ptr<EbinVersionMetadata> result(new EbinVersionMetadata(SlotName("unset")));

    KeyValueConfigFile f(_imp->portage_repository->layout()->package_directory(q) /
            (stringify(q.package) + "-" + stringify(v) + ".ebin"),
            KeyValueConfigFileOptions() + kvcfo_disallow_continuations + kvcfo_disallow_comments +
            kvcfo_disallow_space_around_equals + kvcfo_disallow_source);

    result->set_run_depend(f.get("RDEPEND"));
    result->set_post_depend(f.get("PDEPEND"));
    result->set_suggested_depend(f.get("SDEPEND"));

    result->set_license(f.get("LICENSE"));

    result->source.reset();
    result->binary.reset();

    result->slot = SlotName(f.get("SLOT"));
    result->set_homepage(f.get("HOMEPAGE"));
    result->description = f.get("DESCRIPTION");
    result->eapi = EAPIData::get_instance()->eapi_from_string(f.get("EAPI"));

    result->set_provide(f.get("PROVIDE"));
    result->set_src_uri(f.get("SRC_URI"));
    result->set_restrictions(f.get("RESTRICT"));
    result->set_keywords(f.get("KEYWORDS"));
    result->set_iuse(f.get("IUSE"));
    result->set_inherited(f.get("INHERITED"));

    result->set_bin_uri(f.get("BIN_URI"));

    return result;
}

namespace
{
    FSEntry
    get_root(tr1::shared_ptr<const DestinationsCollection> destinations)
    {
        if (destinations)
            for (DestinationsCollection::Iterator d(destinations->begin()), d_end(destinations->end()) ;
                    d != d_end ; ++d)
                if ((*d)->installed_interface)
                    return (*d)->installed_interface->root();

        return FSEntry("/");
    }
}

void
EbinEntries::install(const tr1::shared_ptr<const PackageID> & id,
        const InstallOptions & o, tr1::shared_ptr<const ERepositoryProfile>) const
{
    std::string binaries, flat_bin_uri;
    {
        std::set<std::string> already_in_binaries;

        /* make B and FLAT_BIN_URI */
        DepSpecFlattener f(_imp->params.environment, id);
        if (id->bin_uri_key())
            id->bin_uri_key()->value()->accept(f);

        for (DepSpecFlattener::Iterator ff(f.begin()), ff_end(f.end()) ; ff != ff_end ; ++ff)
        {
            std::string::size_type pos((*ff)->text().rfind('/'));
            if (std::string::npos == pos)
            {
                if (already_in_binaries.end() == already_in_binaries.find((*ff)->text()))
                {
                    binaries.append((*ff)->text());
                    already_in_binaries.insert((*ff)->text());
                }
            }
            else
            {
                if (already_in_binaries.end() == already_in_binaries.find((*ff)->text().substr(pos + 1)))
                {
                    binaries.append((*ff)->text().substr(pos + 1));
                    already_in_binaries.insert((*ff)->text().substr(pos + 1));
                }
            }
            binaries.append(" ");

            /* add * mirror entries */
            tr1::shared_ptr<const MirrorsCollection> star_mirrors(_imp->params.environment->mirrors("*"));
            for (MirrorsCollection::Iterator m(star_mirrors->begin()), m_end(star_mirrors->end()) ; m != m_end ; ++m)
                flat_bin_uri.append(*m + "/" + (*ff)->text().substr(pos + 1) + " ");

            if (0 == (*ff)->text().compare(0, 9, "mirror://"))
            {
                std::string mirror((*ff)->text().substr(9));
                std::string::size_type spos(mirror.find('/'));

                if (std::string::npos == spos)
                    throw PackageInstallActionError("Can't install '" + stringify(*id) + "' since BIN_URI is broken");

                tr1::shared_ptr<const MirrorsCollection> mirrors(_imp->params.environment->mirrors(mirror.substr(0, spos)));
                if (! _imp->portage_repository->is_mirror(mirror.substr(0, spos)) &&
                        mirrors->empty())
                    throw PackageInstallActionError("Can't install '" + stringify(*id)
                            + "' since BIN_URI references unknown mirror:// '" +
                            mirror.substr(0, spos) + "'");

                for (MirrorsCollection::Iterator m(mirrors->begin()), m_end(mirrors->end()) ; m != m_end ; ++m)
                    flat_bin_uri.append(*m + "/" + mirror.substr(spos + 1) + " ");

                for (RepositoryMirrorsInterface::MirrorsIterator
                        m(_imp->portage_repository->begin_mirrors(mirror.substr(0, spos))),
                        m_end(_imp->portage_repository->end_mirrors(mirror.substr(0, spos))) ;
                        m != m_end ; ++m)
                    flat_bin_uri.append(m->second + "/" + mirror.substr(spos + 1) + " ");
            }
            else
                flat_bin_uri.append((*ff)->text());
            flat_bin_uri.append(" ");

            /* add mirror://gentoo/ entries */
            std::string master_mirror(strip_trailing_string(stringify(_imp->portage_repository->name()), "x-"));
            if (_imp->portage_repository->is_mirror(master_mirror))
            {
                tr1::shared_ptr<const MirrorsCollection> repo_mirrors(_imp->params.environment->mirrors(master_mirror));
                for (MirrorsCollection::Iterator m(repo_mirrors->begin()), m_end(repo_mirrors->end()) ; m != m_end ; ++m)
                    flat_bin_uri.append(*m + "/" + (*ff)->text().substr(pos + 1) + " ");

                for (RepositoryMirrorsInterface::MirrorsIterator
                        m(_imp->portage_repository->begin_mirrors(master_mirror)),
                        m_end(_imp->portage_repository->end_mirrors(master_mirror)) ;
                        m != m_end ; ++m)
                    flat_bin_uri.append(m->second + "/" + (*ff)->text().substr(pos + 1) + " ");
            }
        }
    }

    EbinCommandParams command_params(EbinCommandParams::create()
            .environment(_imp->params.environment)
            .package_id(id)
            .portdir(_imp->params.master_repository ? _imp->params.master_repository->params().location :
                _imp->params.location)
            .distdir(_imp->params.distdir)
            .buildroot(_imp->params.buildroot));

    bool fetch_userpriv_ok(_imp->environment->reduced_gid() != getgid());
    if (fetch_userpriv_ok)
    {
        FSEntry f(_imp->params.distdir);
        Context c("When checking permissions on '" + stringify(f) + "' for userpriv:");

        if (f.exists())
        {
            if (f.group() != _imp->environment->reduced_gid())
            {
                Log::get_instance()->message(ll_warning, lc_context, "Directory '" +
                        stringify(f) + "' owned by group '" +
                        stringify(get_group_name(f.group())) + "', not '" +
                        stringify(get_group_name(_imp->environment->reduced_gid())) +
                        "', so cannot enable userpriv");
                fetch_userpriv_ok = false;
            }
            else if (! f.has_permission(fs_ug_group, fs_perm_write))
            {
                Log::get_instance()->message(ll_warning, lc_context, "Directory '" +
                        stringify(f) + "' does not group write permission," +
                        "cannot enable userpriv");
                fetch_userpriv_ok = false;
            }
        }
    }

    if (! o.destination)
        throw PackageInstallActionError("Can't install '" + stringify(*id)
                + "' because no destination was provided");

    EbinFetchCommand fetch_cmd(command_params,
            EbinFetchCommandParams::create()
            .b(binaries)
            .flat_bin_uri(flat_bin_uri)
            .root(o.destination->installed_interface ? stringify(o.destination->installed_interface->root()) : "/")
            .safe_resume(o.safe_resume)
            .userpriv(false));

    fetch_cmd();

    if (o.fetch_only)
        return;

    EbinInstallCommandParams install_params(
            EbinInstallCommandParams::create()
            .b(binaries)
            .root(o.destination->installed_interface ? stringify(o.destination->installed_interface->root()) : "/")
            .debug_build(o.debug_build)
            .phase(ebin_ip_prepare)
            .disable_cfgpro(o.no_config_protect)
            .config_protect(_imp->portage_repository->profile_variable("CONFIG_PROTECT"))
            .config_protect_mask(_imp->portage_repository->profile_variable("CONFIG_PROTECT_MASK"))
            .loadsaveenv_dir(_imp->params.buildroot / stringify(id->name().category) / (
                    stringify(id->name().package) + "-" + stringify(id->version())) / "temp")
            .slot(SlotName(id->slot())));

    EbinInstallCommand prepare_cmd(command_params, install_params);
    prepare_cmd();

    install_params.phase = ebin_ip_initbinenv;
    EbinInstallCommand initbinenv_cmd(command_params, install_params);
    initbinenv_cmd();

    install_params.phase = ebin_ip_setup;
    EbinInstallCommand setup_cmd(command_params, install_params);
    setup_cmd();

    install_params.phase = ebin_ip_unpackbin;
    EbinInstallCommand unpackbin_cmd(command_params, install_params);
    unpackbin_cmd();

    if (! o.destination->destination_interface)
        throw PackageInstallActionError("Can't install '" + stringify(*id)
                + "' to destination '" + stringify(o.destination->name())
                + "' because destination does not provide destination_interface");

    if (o.destination->destination_interface->want_pre_post_phases())
    {
        install_params.phase = ebin_ip_preinstall;
        install_params.root = o.destination->installed_interface ?
            stringify(o.destination->installed_interface->root()) : "/";
        EbinInstallCommand preinst_cmd(command_params, install_params);
        preinst_cmd();
    }

    o.destination->destination_interface->merge(
            MergeOptions::create()
            .package_id(id)
            .image_dir(_imp->params.buildroot / stringify(id->name().category) / (stringify(id->name().package) + "-"
                    + stringify(id->version())) / "image")
            .environment_file(_imp->params.buildroot / stringify(id->name().category) / (stringify(id->name().package) + "-"
                    + stringify(id->version())) / "temp" / "loadsaveenv")
            );

    if (o.destination->destination_interface->want_pre_post_phases())
    {
        install_params.phase = ebin_ip_postinstall;
        install_params.root = o.destination->installed_interface ?
            stringify(o.destination->installed_interface->root()) : "/";
        EbinInstallCommand postinst_cmd(command_params, install_params);
        postinst_cmd();
    }

    install_params.phase = ebin_ip_tidyup;
    EbinInstallCommand tidyup_cmd(command_params, install_params);
    tidyup_cmd();
}

tr1::shared_ptr<ERepositoryEntries>
EbinEntries::make_ebin_entries(
        const Environment * const e, ERepository * const r, const ERepositoryParams & p)
{
    return tr1::shared_ptr<ERepositoryEntries>(new EbinEntries(e, r, p));
}

std::string
EbinEntries::get_environment_variable(const tr1::shared_ptr<const PackageID> & id,
        const std::string & var, tr1::shared_ptr<const ERepositoryProfile>) const
{
    PackageID::Iterator i(id->find(var));
    if (id->end() != i)
    {
        MetadataKeyRawPrinter p;
        i->accept(p);
        return stringify(p);
    }

    throw EnvironmentVariableActionError("Couldn't get environment variable '" +
            stringify(var) + "' for package '" + stringify(*id) + "'");
}

void
EbinEntries::merge(const MergeOptions & m)
{
    Context context("When merging '" + stringify(*m.package_id) + "' at '" + stringify(m.image_dir)
            + "' to ebin repository '" + stringify(_imp->portage_repository->name()) + "':");

    if (! _imp->portage_repository->destination_interface->is_suitable_destination_for(m.package()))
        throw PackageInstallActionError("Not a suitable destination for '" + stringify(*m.package()) + "'");

    FSEntry ebin_dir(_imp->params.location);
    ebin_dir /= stringify(m.package_id->name().category);
    ebin_dir.mkdir();
    ebin_dir /= stringify(m.package_id->name().package);
    ebin_dir.mkdir();

    FSEntry ebin_file_name(ebin_dir / (stringify(m.package_id->name.package()) + "-" +
                stringify(m.package_id->version()) + ".ebin.incomplete"));
    std::ofstream ebin_file(stringify(ebin_file_name).c_str());
    if (! ebin_file)
        throw PackageInstallActionError("Cannot write to '" + stringify(ebin_file_name) + "'");

    if (metadata->deps_interface)
    {
        DepSpecPrettyPrinter r(0, false), p(0, false), s(0, false);
        metadata->deps_interface->run_depend()->accept(r);
        metadata->deps_interface->post_depend()->accept(p);
        metadata->deps_interface->suggested_depend()->accept(s);
        ebin_file << "RDEPEND=" << r << std::endl;
        ebin_file << "PDEPEND=" << p << std::endl;
        ebin_file << "SDEPEND=" << s << std::endl;
    }

    if (metadata->license_interface)
    {
        DepSpecPrettyPrinter l(0, false);
        metadata->license_interface->license()->accept(l);
        ebin_file << "LICENSE=" << l << std::endl;
    }

    ebin_file << "SLOT=" << metadata->slot << std::endl;
    DepSpecPrettyPrinter h(0, false);
    metadata->homepage()->accept(h);
    ebin_file << "HOMEPAGE=" << h << std::endl;
    ebin_file << "DESCRIPTION=" << metadata->description << std::endl;
    ebin_file << "EAPI=" << metadata->eapi->name << std::endl;

    if (metadata->ebuild_interface)
    {
        DepSpecPrettyPrinter p(0, false), s(0, false), r(0, false);
        metadata->ebuild_interface->provide()->accept(p);
        metadata->ebuild_interface->src_uri()->accept(s);
        metadata->ebuild_interface->restrictions()->accept(r);
        ebin_file << "PROVIDE=" << p << std::endl;
        ebin_file << "SRC_URI=" << s << std::endl;
        ebin_file << "RESTRICT=" << r << std::endl;
        ebin_file << "KEYWORDS=" << join(metadata->ebuild_interface->keywords()->begin(),
                metadata->ebuild_interface->keywords()->end(), " ") << std::endl;
        ebin_file << "IUSE=" << join(metadata->ebuild_interface->iuse()->begin(),
                metadata->ebuild_interface->iuse()->end(), " ") << std::endl;
        ebin_file << "INHERITED=" << join(metadata->ebuild_interface->inherited()->begin(),
                metadata->ebuild_interface->inherited()->end(), " ") << std::endl;
    }

    FSEntry pkg_file_name(_imp->params.distdir / (
                stringify(_imp->portage_repository->name()) + "--" +
                stringify(m.package_id->name.category) + "--" +
                stringify(m.package_id->name.package) + "-" +
                stringify(m.package_id->version) + ".tar.bz2"));

    ebin_file << "BIN_URI=" << _imp->params.write_bin_uri_prefix << pkg_file_name.basename() << std::endl;

    if (pkg_file_name.exists())
        pkg_file_name.unlink();

    EbinCommandParams command_params(EbinCommandParams::create()
            .environment(_imp->params.environment)
            .db_entry(&m.package)
            .portdir(_imp->params.master_repository ? _imp->params.master_repository->params().location :
                _imp->params.location)
            .distdir(_imp->params.distdir)
            .buildroot(_imp->params.buildroot));

    EbinMergeCommand merge_cmd(
            command_params,
            EbinMergeCommandParams::create()
            .pkg_file_name(pkg_file_name)
            .image(m.image_dir)
            .environment_file(m.environment_file));

    merge_cmd();

    FSEntry real_ebin_file_name(ebin_dir / (stringify(m.package_id->name.package) + "-" + stringify(m.package_id->version) + ".ebin"));
    if (real_ebin_file_name.exists())
        real_ebin_file_name.unlink();
    ebin_file_name.rename(real_ebin_file_name);
}

bool
EbinEntries::is_package_file(const QualifiedPackageName & n, const FSEntry & e) const
{
    return is_file_with_prefix_extension(e, stringify(n.package) + "-", ".ebin", IsFileWithOptions());
}

VersionSpec
EbinEntries::extract_package_file_version(const QualifiedPackageName & n, const FSEntry & e) const
{
    Context context("When extracting version from '" + stringify(e) + "':");
    return VersionSpec(strip_leading_string(strip_trailing_string(e.basename(), ".ebin"), stringify(n.package) + "-"));
}

bool
EbinEntries::pretend(const QualifiedPackageName &, const VersionSpec &,
        tr1::shared_ptr<const ERepositoryProfile>) const
{
    return true;
}

std::string
EbinEntries::get_package_file_manifest_key(const FSEntry & f, const QualifiedPackageName & q) const
{
    if (is_package_file(q, f)
        return "EBIN";
    return "";
}

#endif

