/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <paludis/repositories/gentoo/ebuild_entries.hh>
#include <paludis/repositories/gentoo/ebuild_flat_metadata_cache.hh>
#include <paludis/repositories/gentoo/portage_repository.hh>
#include <paludis/repositories/gentoo/ebuild.hh>

#include <paludis/dep_spec_flattener.hh>
#include <paludis/environment.hh>
#include <paludis/portage_dep_parser.hh>
#include <paludis/version_metadata.hh>
#include <paludis/util/collection_concrete.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/log.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/system.hh>

#include <fstream>
#include <list>
#include <set>
#include <sys/types.h>
#include <grp.h>
#include <tr1/functional>
#include <functional>

using namespace paludis;

namespace paludis
{
    /**
     * Implementation data for EbuildEntries.
     *
     * \ingroup grpportagerepository
     */
    template<>
    struct Implementation<EbuildEntries>
    {
        const Environment * const environment;
        PortageRepository * const portage_repository;
        const PortageRepositoryParams params;

        std::tr1::shared_ptr<EclassMtimes> eclass_mtimes;
        time_t master_mtime;

        Implementation(const Environment * const e, PortageRepository * const p,
                const PortageRepositoryParams & k) :
            environment(e),
            portage_repository(p),
            params(k),
            eclass_mtimes(new EclassMtimes(k.eclassdirs)),
            master_mtime(0)
        {
            FSEntry m(k.location / "metadata" / "timestamp");
            if (m.exists())
                master_mtime = m.mtime();
        }
    };
}

EbuildEntries::EbuildEntries(
        const Environment * const e, PortageRepository * const p, const PortageRepositoryParams & k) :
    PortageRepositoryEntries(".ebuild"),
    PrivateImplementationPattern<EbuildEntries>(new
            Implementation<EbuildEntries>(e, p, k))
{
}

EbuildEntries::~EbuildEntries()
{
}

std::tr1::shared_ptr<VersionMetadata>
EbuildEntries::generate_version_metadata(const QualifiedPackageName & q,
        const VersionSpec & v) const
{
    Context context("When generating version metadata for '" + stringify(q) + "-" + stringify(v) + "':");

    std::tr1::shared_ptr<EbuildVersionMetadata> result(new EbuildVersionMetadata);

    FSEntry ebuild_file(_imp->params.location / stringify(q.category) /
            stringify(q.package) / (stringify(q.package) + "-" + stringify(v) + ".ebuild"));

    FSEntry cache_file(_imp->params.cache);
    cache_file /= stringify(q.category);
    cache_file /= stringify(q.package) + "-" + stringify(v);

    FSEntry write_cache_file(_imp->params.write_cache);
    write_cache_file /= stringify(_imp->portage_repository->name());
    write_cache_file /= stringify(q.category);
    write_cache_file /= stringify(q.package) + "-" + stringify(v);

    bool ok(false);
    if (_imp->params.cache.basename() != "empty")
    {

        EbuildFlatMetadataCache metadata_cache(cache_file, ebuild_file, _imp->master_mtime,
                _imp->eclass_mtimes, false);
        if (metadata_cache.load(result))
            ok = true;
    }

    if ((! ok) && _imp->params.write_cache.basename() != "empty")
    {
        EbuildFlatMetadataCache write_metadata_cache(write_cache_file, ebuild_file, _imp->master_mtime,
                _imp->eclass_mtimes, true);
        if (write_metadata_cache.load(result))
            ok = true;
        else if (write_cache_file.exists())
            write_cache_file.unlink();
    }

    if (! ok)
    {
        if (_imp->params.cache.basename() != "empty")
            Log::get_instance()->message(ll_warning, lc_no_context,
                    "No usable cache entry for '" + stringify(q) +
                    "-" + stringify(v) + "' in '" + stringify(_imp->portage_repository->name()) + "'");

        PackageDatabaseEntry e(q, v, _imp->portage_repository->name());
        EbuildMetadataCommand cmd(EbuildCommandParams::create()
                .environment(_imp->environment)
                .db_entry(&e)
                .ebuild_dir(_imp->params.location / stringify(q.category) /
                            stringify(q.package))
                .files_dir(_imp->params.location / stringify(q.category) /
                            stringify(q.package) / "files")
                .eclassdirs(_imp->params.eclassdirs)
                .portdir(_imp->params.master_repository ? _imp->params.master_repository->params().location :
                    _imp->params.location)
                .distdir(_imp->params.distdir)
                .buildroot(_imp->params.buildroot));

        if (! cmd())
            Log::get_instance()->message(ll_warning, lc_no_context,
                    "No usable metadata for '" + stringify(q)
                    + "-" + stringify(v) + "' in '" + stringify(_imp->portage_repository->name()) + "'");

        if (0 == ((result = cmd.metadata())))
            throw InternalError(PALUDIS_HERE, "cmd.metadata() is zero pointer???");

        if (_imp->params.write_cache.basename() != "empty" && result->eapi != "UNKNOWN")
        {
            EbuildFlatMetadataCache metadata_cache(write_cache_file, ebuild_file, _imp->master_mtime,
                    _imp->eclass_mtimes, false);
            metadata_cache.save(result);
        }
    }

    return result;
}

namespace
{
    class AAFinder :
        private InstantiationPolicy<AAFinder, instantiation_method::NonCopyableTag>,
        protected DepSpecVisitorTypes::ConstVisitor
    {
        private:
            mutable std::list<const StringDepSpec *> _specs;

        protected:
            void visit(const AllDepSpec * a)
            {
                std::for_each(a->begin(), a->end(), accept_visitor(
                            static_cast<DepSpecVisitorTypes::ConstVisitor *>(this)));
            }

            void visit(const AnyDepSpec *) PALUDIS_ATTRIBUTE((noreturn))
            {
                throw InternalError(PALUDIS_HERE, "Found unexpected AnyDepSpec");
            }

            void visit(const UseDepSpec * a)
            {
                std::for_each(a->begin(), a->end(), accept_visitor(
                            static_cast<DepSpecVisitorTypes::ConstVisitor *>(this)));
            }

            void visit(const PlainTextDepSpec * a)
            {
                _specs.push_back(a);
            }

            void visit(const PackageDepSpec * a)
            {
                _specs.push_back(a);
            }

            void visit(const BlockDepSpec * a)
            {
                _specs.push_back(a);
            }

        public:
            AAFinder(const std::tr1::shared_ptr<const DepSpec> a)
            {
                a->accept(static_cast<DepSpecVisitorTypes::ConstVisitor *>(this));
            }

            typedef std::list<const StringDepSpec *>::const_iterator Iterator;

            Iterator begin()
            {
                return _specs.begin();
            }

            Iterator end() const
            {
                return _specs.end();
            }
    };

}

namespace
{
    FSEntry
    get_root(std::tr1::shared_ptr<const DestinationsCollection> destinations)
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
EbuildEntries::install(const QualifiedPackageName & q, const VersionSpec & v,
        const InstallOptions & o, std::tr1::shared_ptr<const PortageRepositoryProfile> p) const
{
    using namespace std::tr1::placeholders;

    Context context("When installing '" + stringify(q) + "-" + stringify(v) + "':");

    if (! _imp->portage_repository->has_version(q, v))
    {
        throw PackageInstallActionError("Can't install '" + stringify(q) + "-"
                + stringify(v) + "' since has_version failed");
    }

    std::tr1::shared_ptr<const VersionMetadata> metadata(_imp->portage_repository->version_metadata(q, v));

    PackageDatabaseEntry e(q, v, _imp->portage_repository->name());

    bool fetch_restrict(false), no_mirror(false), userpriv_restrict;
    {
        DepSpecFlattener restricts(_imp->params.environment, &e, metadata->ebuild_interface->restrictions());
        fetch_restrict =
            restricts.end() != std::find_if(restricts.begin(), restricts.end(),
                    std::tr1::bind(std::equal_to<std::string>(), std::tr1::bind(std::tr1::mem_fn(&StringDepSpec::text), _1), "fetch")) ||
            restricts.end() != std::find_if(restricts.begin(), restricts.end(),
                    std::tr1::bind(std::equal_to<std::string>(), std::tr1::bind(std::tr1::mem_fn(&StringDepSpec::text), _1), "nofetch"));

        userpriv_restrict =
            restricts.end() != std::find_if(restricts.begin(), restricts.end(),
                    std::tr1::bind(std::equal_to<std::string>(), std::tr1::bind(std::tr1::mem_fn(&StringDepSpec::text), _1), "userpriv")) ||
            restricts.end() != std::find_if(restricts.begin(), restricts.end(),
                    std::tr1::bind(std::equal_to<std::string>(), std::tr1::bind(std::tr1::mem_fn(&StringDepSpec::text), _1), "nouserpriv"));

        no_mirror =
            restricts.end() != std::find_if(restricts.begin(), restricts.end(),
                    std::tr1::bind(std::equal_to<std::string>(), std::tr1::bind(std::tr1::mem_fn(&StringDepSpec::text), _1), "mirror")) ||
            restricts.end() != std::find_if(restricts.begin(), restricts.end(),
                    std::tr1::bind(std::equal_to<std::string>(), std::tr1::bind(std::tr1::mem_fn(&StringDepSpec::text), _1), "nomirror"));
    }

    std::string archives, all_archives, flat_src_uri;
    {
        std::set<std::string> already_in_archives;

        /* make A and FLAT_SRC_URI */
        std::tr1::shared_ptr<const DepSpec> f_spec(metadata->ebuild_interface->src_uri());
        DepSpecFlattener f(_imp->params.environment, &e, f_spec);

        for (DepSpecFlattener::Iterator ff(f.begin()), ff_end(f.end()) ; ff != ff_end ; ++ff)
        {
            std::string::size_type pos((*ff)->text().rfind('/'));
            if (std::string::npos == pos)
            {
                if (already_in_archives.end() == already_in_archives.find((*ff)->text()))
                {
                    archives.append((*ff)->text());
                    already_in_archives.insert((*ff)->text());
                }
            }
            else
            {
                if (already_in_archives.end() == already_in_archives.find((*ff)->text().substr(pos + 1)))
                {
                    archives.append((*ff)->text().substr(pos + 1));
                    already_in_archives.insert((*ff)->text().substr(pos + 1));
                }
            }
            archives.append(" ");

            /* add * mirror entries */
            std::tr1::shared_ptr<const MirrorsCollection> star_mirrors(_imp->params.environment->mirrors("*"));
            for (MirrorsCollection::Iterator m(star_mirrors->begin()), m_end(star_mirrors->end()) ; m != m_end ; ++m)
                flat_src_uri.append(*m + "/" + (*ff)->text().substr(pos + 1) + " ");

            if (0 == (*ff)->text().compare(0, 9, "mirror://"))
            {
                std::string mirror((*ff)->text().substr(9));
                std::string::size_type spos(mirror.find('/'));

                if (std::string::npos == spos)
                    throw PackageInstallActionError("Can't install '" + stringify(q) + "-"
                            + stringify(v) + "' since SRC_URI is broken");

                std::tr1::shared_ptr<const MirrorsCollection> mirrors(_imp->params.environment->mirrors(mirror.substr(0, spos)));
                if (! _imp->portage_repository->is_mirror(mirror.substr(0, spos)) &&
                        mirrors->empty())
                    throw PackageInstallActionError("Can't install '" + stringify(q) + "-"
                            + stringify(v) + "' since SRC_URI references unknown mirror:// '" +
                            mirror.substr(0, spos) + "'");

                for (MirrorsCollection::Iterator m(mirrors->begin()), m_end(mirrors->end()) ; m != m_end ; ++m)
                    flat_src_uri.append(*m + "/" + mirror.substr(spos + 1) + " ");

                for (RepositoryMirrorsInterface::MirrorsIterator
                        m(_imp->portage_repository->begin_mirrors(mirror.substr(0, spos))),
                        m_end(_imp->portage_repository->end_mirrors(mirror.substr(0, spos))) ;
                        m != m_end ; ++m)
                    flat_src_uri.append(m->second + "/" + mirror.substr(spos + 1) + " ");
            }
            else
                flat_src_uri.append((*ff)->text());
            flat_src_uri.append(" ");

            /* add mirror://gentoo/ entries */
            std::string master_mirror(strip_trailing_string(stringify(_imp->portage_repository->name()), "x-"));
            if (! no_mirror && _imp->portage_repository->is_mirror(master_mirror))
            {
                std::tr1::shared_ptr<const MirrorsCollection> repo_mirrors(_imp->params.environment->mirrors(master_mirror));

                for (MirrorsCollection::Iterator m(repo_mirrors->begin()), m_end(repo_mirrors->end()) ; m != m_end ; ++m)
                    flat_src_uri.append(*m + "/" + (*ff)->text().substr(pos + 1) + " ");

                for (RepositoryMirrorsInterface::MirrorsIterator
                        m(_imp->portage_repository->begin_mirrors(master_mirror)),
                        m_end(_imp->portage_repository->end_mirrors(master_mirror)) ;
                        m != m_end ; ++m)
                    flat_src_uri.append(m->second + "/" + (*ff)->text().substr(pos + 1) + " ");
            }
        }

        /* make AA */
        std::tr1::shared_ptr<const DepSpec> g_spec(metadata->ebuild_interface->src_uri());
        AAFinder g(g_spec);
        std::set<std::string> already_in_all_archives;

        for (AAFinder::Iterator gg(g.begin()), gg_end(g.end()) ; gg != gg_end ; ++gg)
        {
            std::string::size_type pos((*gg)->text().rfind('/'));
            if (std::string::npos == pos)
            {
                if (already_in_all_archives.end() == already_in_all_archives.find((*gg)->text()))
                {
                    all_archives.append((*gg)->text());
                    already_in_all_archives.insert((*gg)->text());
                }
            }
            else
            {
                if (already_in_all_archives.end() == already_in_all_archives.find((*gg)->text().substr(pos + 1)))
                {
                    all_archives.append((*gg)->text().substr(pos + 1));
                    already_in_all_archives.insert((*gg)->text().substr(pos + 1));
                }
            }
            all_archives.append(" ");
        }
    }

    /* Strip trailing space. Some ebuilds rely upon this. From kde-meta.eclass:
     *     [[ -n ${A/${TARBALL}/} ]] && unpack ${A/${TARBALL}/}
     * Rather annoying.
     */
    archives = strip_trailing(archives, " ");
    all_archives = strip_trailing(all_archives, " ");

    /* make use */
    std::string use;
    for (IUseFlagCollection::Iterator i(metadata->ebuild_interface->iuse()->begin()),
            i_end(metadata->ebuild_interface->iuse()->end()) ; i != i_end ; ++i)
        if (_imp->params.environment->query_use(i->flag, e))
            use += stringify(i->flag) + " ";

    use += p->environment_variable("ARCH") + " ";

    /* add expand to use (iuse isn't reliable for use_expand things), and make the expand
     * environment variables */
    std::tr1::shared_ptr<AssociativeCollection<std::string, std::string> > expand_vars(
            new AssociativeCollection<std::string, std::string>::Concrete);
    for (PortageRepositoryProfile::UseExpandIterator x(p->begin_use_expand()),
            x_end(p->end_use_expand()) ; x != x_end ; ++x)
    {
        std::string lower_x;
        std::transform(x->data().begin(), x->data().end(), std::back_inserter(lower_x), &::tolower);

        expand_vars->insert(stringify(*x), "");

        /* possible values from profile */
        std::set<UseFlagName> possible_values;
        WhitespaceTokeniser::get_instance()->tokenise(p->environment_variable(stringify(*x)),
                create_inserter<UseFlagName>(std::inserter(possible_values, possible_values.end())));

        /* possible values from environment */
        std::tr1::shared_ptr<const UseFlagNameCollection> possible_values_from_env(_imp->params.environment->
                known_use_expand_names(*x, e));
        for (UseFlagNameCollection::Iterator i(possible_values_from_env->begin()),
                i_end(possible_values_from_env->end()) ; i != i_end ; ++i)
            possible_values.insert(UseFlagName(stringify(*i).substr(lower_x.length() + 1)));

        for (std::set<UseFlagName>::const_iterator u(possible_values.begin()), u_end(possible_values.end()) ;
                u != u_end ; ++u)
        {
            if (! _imp->params.environment->query_use(UseFlagName(lower_x + "_" + stringify(*u)), e))
                continue;

            use.append(lower_x + "_" + stringify(*u) + " ");

            std::string value;
            AssociativeCollection<std::string, std::string>::Iterator i(expand_vars->find(stringify(*x)));
            if (expand_vars->end() != i)
            {
                value = i->second;
                if (! value.empty())
                    value.append(" ");
                expand_vars->erase(i);
            }
            value.append(stringify(*u));
            expand_vars->insert(stringify(*x), value);
        }
    }

    EbuildCommandParams command_params(EbuildCommandParams::create()
            .environment(_imp->params.environment)
            .db_entry(&e)
            .ebuild_dir(_imp->params.location / stringify(q.category) /
                        stringify(q.package))
            .files_dir(_imp->params.location / stringify(q.category) /
                        stringify(q.package) / "files")
            .eclassdirs(_imp->params.eclassdirs)
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

    EbuildFetchCommand fetch_cmd(command_params,
            EbuildFetchCommandParams::create()
            .a(archives)
            .aa(all_archives)
            .use(use)
            .use_expand(join(p->begin_use_expand(), p->end_use_expand(), " "))
            .expand_vars(expand_vars)
            .flat_src_uri(flat_src_uri)
            .root(stringify(get_root(o.destinations)))
            .profiles(_imp->params.profiles)
            .no_fetch(fetch_restrict)
            .userpriv(fetch_userpriv_ok)
            .safe_resume(o.safe_resume));

    fetch_cmd();

    if (o.fetch_only)
        return;

    if (! o.destinations)
        throw PackageInstallActionError("Can't install '" + stringify(q) + "-"
                + stringify(v) + "' because no destinations were provided");

    bool userpriv_ok((! userpriv_restrict) && (_imp->environment->reduced_gid() != getgid()));
    if (userpriv_ok)
    {
        FSEntry f(_imp->params.buildroot);
        Context c("When checking permissions on '" + stringify(f) + "' for userpriv:");

        if (f.exists())
        {
            if (f.group() != _imp->environment->reduced_gid())
            {
                Log::get_instance()->message(ll_warning, lc_context, "Directory '" +
                        stringify(f) + "' owned by group '" +
                        stringify(get_group_name(f.group())) + "', not '" +
                        stringify(get_group_name(_imp->environment->reduced_gid())) + "', cannot enable userpriv");
                userpriv_ok = false;
            }
            else if (! f.has_permission(fs_ug_group, fs_perm_write))
            {
                Log::get_instance()->message(ll_warning, lc_context, "Directory '" +
                        stringify(f) + "' does not group write permission," +
                        "cannot enable userpriv");
                userpriv_ok = false;
            }
        }
    }

    EbuildInstallCommandParams install_params(
            EbuildInstallCommandParams::create()
                    .phase(ebuild_ip_prepare)
                    .use(use)
                    .a(archives)
                    .aa(all_archives)
                    .use_expand(join(p->begin_use_expand(), p->end_use_expand(), " "))
                    .expand_vars(expand_vars)
                    .root(stringify(get_root(o.destinations)))
                    .profiles(_imp->params.profiles)
                    .disable_cfgpro(o.no_config_protect)
                    .debug_build(o.debug_build)
                    .config_protect(_imp->portage_repository->profile_variable("CONFIG_PROTECT"))
                    .config_protect_mask(_imp->portage_repository->profile_variable("CONFIG_PROTECT_MASK"))
                    .loadsaveenv_dir(_imp->params.buildroot / stringify(q.category) / (
                            stringify(q.package) + "-" + stringify(v)) / "temp")
                    .userpriv(userpriv_ok)
                    .slot(SlotName(metadata->slot)));

    EbuildInstallCommand prepare_cmd(command_params, install_params);
    prepare_cmd();

    install_params.phase = ebuild_ip_init;
    EbuildInstallCommand init_cmd(command_params, install_params);
    init_cmd();

    install_params.phase = ebuild_ip_setup;
    EbuildInstallCommand setup_cmd(command_params, install_params);
    setup_cmd();

    install_params.phase = ebuild_ip_build;
    EbuildInstallCommand build_cmd(command_params, install_params);
    build_cmd();

    install_params.phase = ebuild_ip_install;
    EbuildInstallCommand install_cmd(command_params, install_params);
    install_cmd();

    for (DestinationsCollection::Iterator d(o.destinations->begin()),
            d_end(o.destinations->end()) ; d != d_end ; ++d)
    {
        if (! (*d)->destination_interface)
            throw PackageInstallActionError("Can't install '" + stringify(q) + "-"
                    + stringify(v) + "' to destination '" + stringify((*d)->name())
                    + "' because destination does not provide destination_interface");

        install_params.root = (*d)->installed_interface ?
            stringify((*d)->installed_interface->root()) : "/";

        if ((*d)->destination_interface->want_pre_post_phases())
        {
            install_params.phase = ebuild_ip_preinstall;
            EbuildInstallCommand preinst_cmd(command_params, install_params);
            preinst_cmd();
        }

        (*d)->destination_interface->merge(
                MergeOptions::create()
                .package(PackageDatabaseEntry(q, v, _imp->portage_repository->name()))
                .image_dir(command_params.buildroot / stringify(q.category) / (stringify(q.package) + "-"
                        + stringify(v)) / "image")
                .environment_file(command_params.buildroot / stringify(q.category) / (stringify(q.package) + "-"
                        + stringify(v)) / "temp" / "loadsaveenv")
                );

        if ((*d)->destination_interface->want_pre_post_phases())
        {
            install_params.phase = ebuild_ip_postinstall;
            EbuildInstallCommand postinst_cmd(command_params, install_params);
            postinst_cmd();
        }
    }

    install_params.phase = ebuild_ip_tidyup;
    install_params.root = stringify(get_root(o.destinations));
    EbuildInstallCommand tidyup_cmd(command_params, install_params);
    tidyup_cmd();
}

std::string
EbuildEntries::get_environment_variable(const QualifiedPackageName & q,
        const VersionSpec & v, const std::string & var,
        std::tr1::shared_ptr<const PortageRepositoryProfile>) const
{
    PackageDatabaseEntry for_package(q, v, _imp->portage_repository->name());

    EbuildVariableCommand cmd(EbuildCommandParams::create()
            .environment(_imp->params.environment)
            .db_entry(&for_package)
            .ebuild_dir(_imp->params.location / stringify(q.category) /
                        stringify(q.package))
            .files_dir(_imp->params.location / stringify(q.category) /
                        stringify(q.package) / "files")
            .eclassdirs(_imp->params.eclassdirs)
            .portdir(_imp->params.master_repository ? _imp->params.master_repository->params().location :
                _imp->params.location)
            .distdir(_imp->params.distdir)
            .buildroot(_imp->params.buildroot),

            var);

    if (! cmd())
        throw EnvironmentVariableActionError("Couldn't get environment variable '" +
                stringify(var) + "' for package '" + stringify(for_package) + "'");

    return cmd.result();
}

std::tr1::shared_ptr<PortageRepositoryEntries>
EbuildEntries::make_ebuild_entries(
        const Environment * const e, PortageRepository * const r, const PortageRepositoryParams & p)
{
    return std::tr1::shared_ptr<PortageRepositoryEntries>(new EbuildEntries(e, r, p));
}

void
EbuildEntries::merge(const MergeOptions &)
{
    throw InternalError(PALUDIS_HERE, "Cannot merge to PortageRepository with ebuild entries");
}

