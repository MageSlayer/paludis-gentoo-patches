/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008 Ciaran McCreesh
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

#include <paludis/repositories/e/ebuild_entries.hh>
#include <paludis/repositories/e/ebuild_flat_metadata_cache.hh>
#include <paludis/repositories/e/e_repository.hh>
#include <paludis/repositories/e/ebuild.hh>
#include <paludis/repositories/e/eapi_phase.hh>
#include <paludis/repositories/e/ebuild_id.hh>
#include <paludis/repositories/e/eapi.hh>
#include <paludis/repositories/e/dep_parser.hh>
#include <paludis/repositories/e/fetch_visitor.hh>
#include <paludis/repositories/e/pretend_fetch_visitor.hh>
#include <paludis/repositories/e/check_fetched_files_visitor.hh>
#include <paludis/repositories/e/aa_visitor.hh>
#include <paludis/repositories/e/e_stripper.hh>

#include <paludis/action.hh>
#include <paludis/dep_spec_flattener.hh>
#include <paludis/environment.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/log.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/map.hh>
#include <paludis/util/system.hh>
#include <paludis/util/create_iterator-impl.hh>
#include <paludis/util/set.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/kc.hh>
#include <paludis/util/config_file.hh>
#include <paludis/util/instantiation_policy-impl.hh>
#include <tr1/functional>
#include <fstream>
#include <list>
#include <set>
#include <sys/types.h>
#include <grp.h>
#include <functional>

using namespace paludis;
using namespace paludis::erepository;

namespace
{
    struct Suffixes :
        InstantiationPolicy<Suffixes, instantiation_method::SingletonTag>
    {
        KeyValueConfigFile file;

        Suffixes() :
            file(FSEntry(getenv_with_default("PALUDIS_SUFFIXES_FILE", DATADIR "/paludis/ebuild_entries_suffixes.conf")),
                    KeyValueConfigFileOptions(), &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation)
        {
        }

        bool is_known_suffix(const std::string & s) const
        {
            return ! file.get("suffix_" + s + "_known").empty();
        }

        std::string guess_eapi(const std::string & s) const
        {
            return file.get("guess_eapi_" + s);
        }

        std::string manifest_key(const std::string & s) const
        {
            std::string result(file.get("manifest_key_" + s));
            if (result.empty())
            {
                Log::get_instance()->message("e.ebuild.unknown_manifest_key", ll_warning, lc_context)
                    << "Don't know what the manifest key for files with suffix '" << s << "' is, guessing 'MISC'";
                return "MISC";
            }
            else
                return result;
        }
    };
}

namespace paludis
{
    /**
     * Implementation data for EbuildEntries.
     *
     * \ingroup grperepository
     */
    template<>
    struct Implementation<EbuildEntries>
    {
        const Environment * const environment;
        ERepository * const e_repository;
        const ERepositoryParams params;

        std::tr1::shared_ptr<EclassMtimes> eclass_mtimes;
        time_t master_mtime;

        Implementation(const Environment * const e, ERepository * const p,
                const ERepositoryParams & k) :
            environment(e),
            e_repository(p),
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
        const Environment * const e, ERepository * const p, const ERepositoryParams & k) :
    PrivateImplementationPattern<EbuildEntries>(new Implementation<EbuildEntries>(e, p, k))
{
}

EbuildEntries::~EbuildEntries()
{
}

const std::tr1::shared_ptr<const ERepositoryID>
EbuildEntries::make_id(const QualifiedPackageName & q, const FSEntry & f) const
{
    Context context("When creating ID for '" + stringify(q) + "' from '" + stringify(f) + "':");

    std::tr1::shared_ptr<EbuildID> result(new EbuildID(q, extract_package_file_version(q, f), _imp->params.environment,
                _imp->e_repository->shared_from_this(), f, _guess_eapi(q, f),
                _imp->master_mtime, _imp->eclass_mtimes));
    return result;
}

namespace
{
    class AFinder :
        private InstantiationPolicy<AFinder, instantiation_method::NonCopyableTag>,
        public ConstVisitor<FetchableURISpecTree>
    {
        private:
            std::list<std::pair<const FetchableURIDepSpec *, const URILabelsDepSpec *> > _specs;
            std::list<const URILabelsDepSpec *> _labels;

            const Environment * const env;
            const std::tr1::shared_ptr<const PackageID> id;

        public:
            AFinder(const Environment * const e, const std::tr1::shared_ptr<const PackageID> & i) :
                env(e),
                id(i)
            {
                _labels.push_back(0);
            }

            void visit_leaf(const FetchableURIDepSpec & a)
            {
                _specs.push_back(std::make_pair(&a, *_labels.begin()));
            }

            void visit_leaf(const URILabelsDepSpec & l)
            {
                *_labels.begin() = &l;
            }

            void visit_sequence(const AllDepSpec &,
                    FetchableURISpecTree::ConstSequenceIterator cur,
                    FetchableURISpecTree::ConstSequenceIterator e)
            {
                _labels.push_front(*_labels.begin());
                std::for_each(cur, e, accept_visitor(*this));
                _labels.pop_front();
            }

            void visit_sequence(const ConditionalDepSpec & u,
                    FetchableURISpecTree::ConstSequenceIterator cur,
                    FetchableURISpecTree::ConstSequenceIterator e)
            {
                if (u.condition_met())
                {
                    _labels.push_front(*_labels.begin());
                    std::for_each(cur, e, accept_visitor(*this));
                    _labels.pop_front();
                }
            }

            typedef std::list<std::pair<const FetchableURIDepSpec *,
                    const URILabelsDepSpec *> >::const_iterator ConstIterator;

            ConstIterator begin()
            {
                return _specs.begin();
            }

            ConstIterator end() const
            {
                return _specs.end();
            }
    };
}

namespace
{
    std::string make_use(const Environment * const env,
            const ERepositoryID & id,
            std::tr1::shared_ptr<const ERepositoryProfile> profile)
    {
        if (! (*id.eapi())[k::supported()])
        {
            Log::get_instance()->message("e.ebuild.unknown_eapi", ll_warning, lc_context)
                << "Can't make the USE string for '" << id << "' because its EAPI is unsupported";
            return "";
        }

        std::string use;

        if (id.iuse_key())
            for (IUseFlagSet::ConstIterator i(id.iuse_key()->value()->begin()),
                    i_end(id.iuse_key()->value()->end()) ; i != i_end ; ++i)
                if (env->query_use(i->flag, id))
                    use += stringify(i->flag) + " ";

        if (! (*(*id.eapi())[k::supported()])[k::ebuild_environment_variables()][k::env_arch()].empty())
            use += profile->environment_variable((*(*id.eapi())[k::supported()])[k::ebuild_environment_variables()][k::env_arch()]) + " ";

        return use;
    }

    std::tr1::shared_ptr<Map<std::string, std::string> >
    make_expand(const Environment * const env,
            const ERepositoryID & e,
            std::tr1::shared_ptr<const ERepositoryProfile> profile,
            std::string & use,
            const std::string & expand_sep)
    {
        std::tr1::shared_ptr<Map<std::string, std::string> > expand_vars(
            new Map<std::string, std::string>);

        if (! (*e.eapi())[k::supported()])
        {
            Log::get_instance()->message("e.ebuild.unknown_eapi", ll_warning, lc_context)
                << "Can't make the USE_EXPAND strings for '" << e << "' because its EAPI is unsupported";
            return expand_vars;
        }

        for (ERepositoryProfile::UseExpandConstIterator x(profile->begin_use_expand()),
                x_end(profile->end_use_expand()) ; x != x_end ; ++x)
        {
            std::string lower_x;
            std::transform(x->data().begin(), x->data().end(), std::back_inserter(lower_x), &::tolower);

            expand_vars->insert(stringify(*x), "");

            /* possible values from profile */
            std::set<UseFlagName> possible_values;
            tokenise_whitespace(profile->environment_variable(stringify(*x)),
                    create_inserter<UseFlagName>(std::inserter(possible_values, possible_values.end())));

            /* possible values from environment */
            std::tr1::shared_ptr<const UseFlagNameSet> possible_values_from_env(
                    env->known_use_expand_names(*x, e));
            for (UseFlagNameSet::ConstIterator i(possible_values_from_env->begin()),
                    i_end(possible_values_from_env->end()) ; i != i_end ; ++i)
                possible_values.insert(UseFlagName(stringify(*i).substr(lower_x.length() + 1)));

            for (std::set<UseFlagName>::const_iterator u(possible_values.begin()), u_end(possible_values.end()) ;
                    u != u_end ; ++u)
            {
                if ((*(*e.eapi())[k::supported()])[k::ebuild_options()].require_use_expand_in_iuse)
                    if (e.iuse_key() && e.iuse_key()->value()->end() == e.iuse_key()->value()->find(
                                IUseFlag(*u, use_unspecified, std::string::npos)))
                        continue;

                if (! env->query_use(UseFlagName(lower_x + expand_sep + stringify(*u)), e))
                    continue;

                if (! (*(*e.eapi())[k::supported()])[k::ebuild_options()].require_use_expand_in_iuse)
                    use.append(lower_x + expand_sep + stringify(*u) + " ");

                std::string value;
                Map<std::string, std::string>::ConstIterator i(expand_vars->find(stringify(*x)));
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

        return expand_vars;
    }
}

namespace
{
    bool
    check_userpriv(const FSEntry & f, const Environment * env)
    {
        Context c("When checking permissions on '" + stringify(f) + "' for userpriv:");

        if (f.exists())
        {
            if (f.group() != env->reduced_gid())
            {
                Log::get_instance()->message("e.ebuild.userpriv_disabled", ll_warning, lc_context) << "Directory '" <<
                        f << "' owned by group '" << get_group_name(f.group()) << "', not '"
                        << get_group_name(env->reduced_gid()) << "', so cannot enable userpriv";
                return false;
            }
            else if (! f.has_permission(fs_ug_group, fs_perm_write))
            {
                Log::get_instance()->message("e.ebuild.userpriv_disabled", ll_warning, lc_context) << "Directory '" <<
                        f << "' does not have group write permission, cannot enable userpriv";
                return false;
            }
        }

        return true;
    }
}

void
EbuildEntries::fetch(const std::tr1::shared_ptr<const ERepositoryID> & id,
        const FetchActionOptions & o, std::tr1::shared_ptr<const ERepositoryProfile> p) const
{
    using namespace std::tr1::placeholders;

    Context context("When fetching '" + stringify(*id) + "':");

    bool fetch_restrict(false), userpriv_restrict(false);
    {
        DepSpecFlattener<RestrictSpecTree, PlainTextDepSpec> restricts(_imp->params.environment);
        if (id->restrict_key())
            id->restrict_key()->value()->accept(restricts);

        for (DepSpecFlattener<RestrictSpecTree, PlainTextDepSpec>::ConstIterator i(restricts.begin()), i_end(restricts.end()) ;
                i != i_end ; ++i)
        {
            if ((*(*id->eapi())[k::supported()])[k::ebuild_options()].restrict_fetch->end() !=
                    std::find((*(*id->eapi())[k::supported()])[k::ebuild_options()].restrict_fetch->begin(),
                        (*(*id->eapi())[k::supported()])[k::ebuild_options()].restrict_fetch->end(), (*i)->text()))
                fetch_restrict = true;
            if ("userpriv" == (*i)->text() || "nouserpriv" == (*i)->text())
                userpriv_restrict = true;
        }
    }

    bool fetch_userpriv_ok(_imp->environment->reduced_gid() != getgid() &&
            check_userpriv(FSEntry(_imp->params.distdir), _imp->environment));

    std::string archives, all_archives;
    {
        std::set<std::string> already_in_archives;

        /* make A */
        AFinder f(_imp->params.environment, id);
        if (id->fetches_key())
            id->fetches_key()->value()->accept(f);

        for (AFinder::ConstIterator i(f.begin()), i_end(f.end()) ; i != i_end ; ++i)
        {
            const FetchableURIDepSpec * const spec(static_cast<const FetchableURIDepSpec *>(i->first));

            if (already_in_archives.end() == already_in_archives.find(spec->filename()))
            {
                archives.append(spec->filename());
                already_in_archives.insert(spec->filename());
            }
            archives.append(" ");
        }

        /* make AA */
        if (! (*(*id->eapi())[k::supported()])[k::ebuild_environment_variables()][k::env_aa()].empty())
        {
            AAVisitor g;
            if (id->fetches_key())
                id->fetches_key()->value()->accept(g);
            std::set<std::string> already_in_all_archives;

            for (AAVisitor::ConstIterator gg(g.begin()), gg_end(g.end()) ; gg != gg_end ; ++gg)
            {
                if (already_in_all_archives.end() == already_in_all_archives.find(*gg))
                {
                    all_archives.append(*gg);
                    already_in_all_archives.insert(*gg);
                }
                all_archives.append(" ");
            }
        }
        else
            all_archives = "AA-not-set-for-this-EAPI";
    }

    /* Strip trailing space. Some ebuilds rely upon this. From kde-meta.eclass:
     *     [[ -n ${A/${TARBALL}/} ]] && unpack ${A/${TARBALL}/}
     * Rather annoying.
     */
    archives = strip_trailing(archives, " ");
    all_archives = strip_trailing(all_archives, " ");

    if (id->fetches_key())
    {
        std::string mirrors_name(_imp->e_repository->params().master_repository ?
                stringify(_imp->e_repository->params().master_repository->name()) :
                stringify(_imp->e_repository->name()));
        FetchVisitor f(_imp->params.environment, id, *id->eapi(),
                _imp->e_repository->params().distdir, o[k::fetch_unneeded()], fetch_userpriv_ok, mirrors_name,
                id->fetches_key()->initial_label(), o[k::safe_resume()]);
        id->fetches_key()->value()->accept(f);
        CheckFetchedFilesVisitor c(_imp->environment, id, _imp->e_repository->params().distdir, o[k::fetch_unneeded()], fetch_restrict,
                ((_imp->e_repository->layout()->package_directory(id->name())) / "Manifest"),
                _imp->e_repository->params().use_manifest);
        id->fetches_key()->value()->accept(c);

        if (c.need_nofetch())
        {
            bool userpriv_ok((! userpriv_restrict) && (_imp->environment->reduced_gid() != getgid()) &&
                    check_userpriv(FSEntry(_imp->params.builddir), _imp->environment));
            std::string use(make_use(_imp->params.environment, *id, p));
            std::string expand_sep(stringify((*(*id->eapi())[k::supported()])[k::ebuild_options()].use_expand_separator));
            std::tr1::shared_ptr<Map<std::string, std::string> > expand_vars(make_expand(
                        _imp->params.environment, *id, p, use, expand_sep));

            std::tr1::shared_ptr<const FSEntrySequence> exlibsdirs(_imp->e_repository->layout()->exlibsdirs(id->name()));

            EAPIPhases phases((*(*id->eapi())[k::supported()])[k::ebuild_phases()].ebuild_nofetch);
            for (EAPIPhases::ConstIterator phase(phases.begin_phases()), phase_end(phases.end_phases()) ;
                    phase != phase_end ; ++phase)
            {
                EbuildCommandParams command_params(EbuildCommandParams::named_create()
                        (k::environment(), _imp->params.environment)
                        (k::package_id(), id)
                        (k::ebuild_dir(), _imp->e_repository->layout()->package_directory(id->name()))
                        (k::ebuild_file(), id->fs_location_key()->value())
                        (k::files_dir(), _imp->e_repository->layout()->package_directory(id->name()) / "files")
                        (k::eclassdirs(), _imp->params.eclassdirs)
                        (k::exlibsdirs(), exlibsdirs)
                        (k::portdir(), _imp->params.master_repository ? _imp->params.master_repository->params().location :
                         _imp->params.location)
                        (k::distdir(), _imp->params.distdir)
                        (k::userpriv(), phase->option("userpriv") && userpriv_ok)
                        (k::sandbox(), phase->option("sandbox"))
                        (k::commands(), join(phase->begin_commands(), phase->end_commands(), " "))
                        (k::builddir(), _imp->params.builddir));

                EbuildNoFetchCommand nofetch_cmd(command_params,
                        EbuildNoFetchCommandParams::named_create()
                        (k::a(), archives)
                        (k::aa(), all_archives)
                        (k::use(), use)
                        (k::use_expand(), join(p->begin_use_expand(), p->end_use_expand(), " "))
                        (k::expand_vars(), expand_vars)
                        (k::root(), "/")
                        (k::profiles(), _imp->params.profiles));

                if (! nofetch_cmd())
                    throw FetchActionError("Fetch of '" + stringify(*id) + "' failed", c.failures());
            }
        }

        if (! c.failures()->empty())
            throw FetchActionError("Fetch of '" + stringify(*id) + "' failed", c.failures());
    }
}

void
EbuildEntries::pretend_fetch(const std::tr1::shared_ptr<const ERepositoryID> & id,
        PretendFetchAction & a, std::tr1::shared_ptr<const ERepositoryProfile>) const
{
    using namespace std::tr1::placeholders;

    Context context("When pretending to fetch ID '" + stringify(*id) + "':");

    if (id->fetches_key())
    {
        PretendFetchVisitor f(_imp->params.environment, id, *id->eapi(),
                _imp->e_repository->params().distdir, a.options[k::fetch_unneeded()],
                id->fetches_key()->initial_label(), a);
        id->fetches_key()->value()->accept(f);
    }
}

void
EbuildEntries::install(const std::tr1::shared_ptr<const ERepositoryID> & id,
        const InstallActionOptions & o, std::tr1::shared_ptr<const ERepositoryProfile> p) const
{
    using namespace std::tr1::placeholders;

    Context context("When installing '" + stringify(*id) + "':");

    bool userpriv_restrict, test_restrict, strip_restrict;
    {
        DepSpecFlattener<RestrictSpecTree, PlainTextDepSpec> restricts(_imp->params.environment);
        if (id->restrict_key())
            id->restrict_key()->value()->accept(restricts);

        userpriv_restrict =
            indirect_iterator(restricts.end()) != std::find_if(indirect_iterator(restricts.begin()), indirect_iterator(restricts.end()),
                    std::tr1::bind(std::equal_to<std::string>(), std::tr1::bind(std::tr1::mem_fn(&StringDepSpec::text), _1), "userpriv")) ||
            indirect_iterator(restricts.end()) != std::find_if(indirect_iterator(restricts.begin()), indirect_iterator(restricts.end()),
                    std::tr1::bind(std::equal_to<std::string>(), std::tr1::bind(std::tr1::mem_fn(&StringDepSpec::text), _1), "nouserpriv"));

        test_restrict =
            indirect_iterator(restricts.end()) != std::find_if(indirect_iterator(restricts.begin()), indirect_iterator(restricts.end()),
                    std::tr1::bind(std::equal_to<std::string>(), std::tr1::bind(std::tr1::mem_fn(&StringDepSpec::text), _1), "test"));

        strip_restrict =
            indirect_iterator(restricts.end()) != std::find_if(indirect_iterator(restricts.begin()), indirect_iterator(restricts.end()),
                    std::tr1::bind(std::equal_to<std::string>(), std::tr1::bind(std::tr1::mem_fn(&StringDepSpec::text), _1), "strip")) ||
            indirect_iterator(restricts.end()) != std::find_if(indirect_iterator(restricts.begin()), indirect_iterator(restricts.end()),
                    std::tr1::bind(std::equal_to<std::string>(), std::tr1::bind(std::tr1::mem_fn(&StringDepSpec::text), _1), "nostrip"));
    }

    std::string archives, all_archives;
    {
        std::set<std::string> already_in_archives;

        /* make A */
        AFinder f(_imp->params.environment, id);
        if (id->fetches_key())
            id->fetches_key()->value()->accept(f);

        for (AFinder::ConstIterator i(f.begin()), i_end(f.end()) ; i != i_end ; ++i)
        {
            const FetchableURIDepSpec * const spec(static_cast<const FetchableURIDepSpec *>(i->first));

            if (already_in_archives.end() == already_in_archives.find(spec->filename()))
            {
                archives.append(spec->filename());
                already_in_archives.insert(spec->filename());
            }
            archives.append(" ");
        }

        /* make AA */
        if (! (*(*id->eapi())[k::supported()])[k::ebuild_environment_variables()][k::env_aa()].empty())
        {
            AAVisitor g;
            if (id->fetches_key())
                id->fetches_key()->value()->accept(g);
            std::set<std::string> already_in_all_archives;

            for (AAVisitor::ConstIterator gg(g.begin()), gg_end(g.end()) ; gg != gg_end ; ++gg)
            {
                if (already_in_all_archives.end() == already_in_all_archives.find(*gg))
                {
                    all_archives.append(*gg);
                    already_in_all_archives.insert(*gg);
                }
                all_archives.append(" ");
            }
        }
        else
            all_archives = "AA-not-set-for-this-EAPI";
    }

    /* Strip trailing space. Some ebuilds rely upon this. From kde-meta.eclass:
     *     [[ -n ${A/${TARBALL}/} ]] && unpack ${A/${TARBALL}/}
     * Rather annoying.
     */
    archives = strip_trailing(archives, " ");
    all_archives = strip_trailing(all_archives, " ");

    /* make use */
    std::string use(make_use(_imp->params.environment, *id, p));

    /* add expand to use (iuse isn't reliable for use_expand things), and make the expand
     * environment variables */
    std::string expand_sep(stringify((*(*id->eapi())[k::supported()])[k::ebuild_options()].use_expand_separator));
    std::tr1::shared_ptr<Map<std::string, std::string> > expand_vars(make_expand(
                _imp->params.environment, *id, p, use, expand_sep));

    std::tr1::shared_ptr<const FSEntrySequence> exlibsdirs(_imp->e_repository->layout()->exlibsdirs(id->name()));

    bool userpriv_ok((! userpriv_restrict) && (_imp->environment->reduced_gid() != getgid()) &&
            check_userpriv(FSEntry(_imp->params.distdir),  _imp->environment) &&
            check_userpriv(FSEntry(_imp->params.builddir), _imp->environment));

    EAPIPhases phases((*(*id->eapi())[k::supported()])[k::ebuild_phases()].ebuild_install);
    for (EAPIPhases::ConstIterator phase(phases.begin_phases()), phase_end(phases.end_phases()) ;
            phase != phase_end ; ++phase)
    {
        if (phase->option("merge"))
        {
            if (! (*o[k::destination()])[k::destination_interface()])
                throw InstallActionError("Can't install '" + stringify(*id)
                        + "' to destination '" + stringify(o[k::destination()]->name())
                        + "' because destination does not provide destination_interface");

                (*o[k::destination()])[k::destination_interface()]->merge(
                        MergeParams::named_create()
                        (k::package_id(), id)
                        (k::image_dir(), _imp->params.builddir / (stringify(id->name().category) + "-" + stringify(id->name().package) + "-"
                                + stringify(id->version())) / "image")
                        (k::environment_file(), _imp->params.builddir / (stringify(id->name().category) + "-" + stringify(id->name().package) + "-"
                                + stringify(id->version())) / "temp" / "loadsaveenv")
                        (k::options(), (*(*id->eapi())[k::supported()])[k::merger_options()])
                        );
        }
        else if (phase->option("strip"))
        {
            if (! strip_restrict)
            {
                std::string libdir("lib");
                FSEntry root(o[k::destination()]->installed_root_key() ?
                        stringify(o[k::destination()]->installed_root_key()->value()) : "/");
                if ((root / "usr" / "lib").is_symbolic_link())
                {
                    libdir = (root / "usr" / "lib").readlink();
                    if (std::string::npos != libdir.find_first_of("./"))
                        libdir = "lib";
                }

                Log::get_instance()->message("e.ebuild.libdir", ll_debug, lc_context) << "Using '" << libdir << "' for libdir";

                EStripper stripper(EStripperOptions::named_create()
                        (k::package_id(), id)
                        (k::image_dir(), _imp->params.builddir / (stringify(id->name().category) + "-" + stringify(id->name().package) + "-"
                                + stringify(id->version())) / "image")
                        (k::debug_dir(), _imp->params.builddir / (stringify(id->name().category) + "-" + stringify(id->name().package) + "-"
                                + stringify(id->version())) / "image" / "usr" / libdir / "debug")
                        (k::debug_build(), o[k::debug_build()])
                        );
                stripper.strip();
            }
        }
        else if ((! phase->option("prepost")) ||
                ((*o[k::destination()])[k::destination_interface()] &&
                 (*o[k::destination()])[k::destination_interface()]->want_pre_post_phases()))
        {
            if (phase->option("checkphase"))
            {
                if (test_restrict)
                    continue;

                switch (o[k::checks()])
                {
                    case iaco_none:
                        if (! phase->option("checks=none"))
                            continue;
                        break;

                    case iaco_default:
                        if (! phase->option("checks=default"))
                            continue;
                        break;

                    case iaco_always:
                        if (! phase->option("checks=always"))
                            continue;
                        break;

                    case last_iaco:
                        break;
                }
            }

            EbuildCommandParams command_params(EbuildCommandParams::named_create()
                    (k::environment(), _imp->params.environment)
                    (k::package_id(), id)
                    (k::ebuild_dir(), _imp->e_repository->layout()->package_directory(id->name()))
                    (k::ebuild_file(), id->fs_location_key()->value())
                    (k::files_dir(), _imp->e_repository->layout()->package_directory(id->name()) / "files")
                    (k::eclassdirs(), _imp->params.eclassdirs)
                    (k::exlibsdirs(), exlibsdirs)
                    (k::portdir(), _imp->params.master_repository ? _imp->params.master_repository->params().location :
                     _imp->params.location)
                    (k::distdir(), _imp->params.distdir)
                    (k::commands(), join(phase->begin_commands(), phase->end_commands(), " "))
                    (k::sandbox(), phase->option("sandbox"))
                    (k::userpriv(), phase->option("userpriv") && userpriv_ok)
                    (k::builddir(), _imp->params.builddir));

            EbuildInstallCommandParams install_params(
                    EbuildInstallCommandParams::named_create()
                            (k::use(), use)
                            (k::a(), archives)
                            (k::aa(), all_archives)
                            (k::use_expand(), join(p->begin_use_expand(), p->end_use_expand(), " "))
                            (k::expand_vars(), expand_vars)
                            (k::root(), o[k::destination()]->installed_root_key() ?
                             stringify(o[k::destination()]->installed_root_key()->value()) : "/")
                            (k::profiles(), _imp->params.profiles)
                            (k::disable_cfgpro(), o[k::no_config_protect()])
                            (k::config_protect(), _imp->e_repository->profile_variable("CONFIG_PROTECT"))
                            (k::config_protect_mask(), _imp->e_repository->profile_variable("CONFIG_PROTECT_MASK"))
                            (k::loadsaveenv_dir(), _imp->params.builddir / (stringify(id->name().category) + "-" +
                                   stringify(id->name().package) + "-" + stringify(id->version())) / "temp")
                            (k::slot(), SlotName(id->slot())));

            EbuildInstallCommand cmd(command_params, install_params);
            cmd();
        }
    }
}

void
EbuildEntries::info(const std::tr1::shared_ptr<const ERepositoryID> & id,
        std::tr1::shared_ptr<const ERepositoryProfile> p) const
{
    using namespace std::tr1::placeholders;

    Context context("When infoing '" + stringify(*id) + "':");

    bool userpriv_restrict;
    {
        DepSpecFlattener<RestrictSpecTree, PlainTextDepSpec> restricts(_imp->params.environment);
        if (id->restrict_key())
            id->restrict_key()->value()->accept(restricts);

        userpriv_restrict =
            indirect_iterator(restricts.end()) != std::find_if(indirect_iterator(restricts.begin()), indirect_iterator(restricts.end()),
                    std::tr1::bind(std::equal_to<std::string>(), std::tr1::bind(std::tr1::mem_fn(&StringDepSpec::text), _1), "userpriv")) ||
            indirect_iterator(restricts.end()) != std::find_if(indirect_iterator(restricts.begin()), indirect_iterator(restricts.end()),
                    std::tr1::bind(std::equal_to<std::string>(), std::tr1::bind(std::tr1::mem_fn(&StringDepSpec::text), _1), "nouserpriv"));
    }
    bool userpriv_ok((! userpriv_restrict) && (_imp->environment->reduced_gid() != getgid()) &&
            check_userpriv(FSEntry(_imp->params.builddir), _imp->environment));

    /* make use */
    std::string use(make_use(_imp->params.environment, *id, p));

    /* add expand to use (iuse isn't reliable for use_expand things), and make the expand
     * environment variables */
    std::string expand_sep(stringify((*(*id->eapi())[k::supported()])[k::ebuild_options()].use_expand_separator));
    std::tr1::shared_ptr<Map<std::string, std::string> > expand_vars(make_expand(
                _imp->params.environment, *id, p, use, expand_sep));

    std::tr1::shared_ptr<const FSEntrySequence> exlibsdirs(_imp->e_repository->layout()->exlibsdirs(id->name()));

    EAPIPhases phases((*(*id->eapi())[k::supported()])[k::ebuild_phases()].ebuild_info);
    for (EAPIPhases::ConstIterator phase(phases.begin_phases()), phase_end(phases.end_phases()) ;
            phase != phase_end ; ++phase)
    {
        if (phase->option("installed=true"))
            continue;

        EbuildCommandParams command_params(EbuildCommandParams::named_create()
                (k::environment(), _imp->params.environment)
                (k::package_id(), id)
                (k::ebuild_dir(), _imp->e_repository->layout()->package_directory(id->name()))
                (k::ebuild_file(), id->fs_location_key()->value())
                (k::files_dir(), _imp->e_repository->layout()->package_directory(id->name()) / "files")
                (k::eclassdirs(), _imp->params.eclassdirs)
                (k::exlibsdirs(), exlibsdirs)
                (k::portdir(), _imp->params.master_repository ? _imp->params.master_repository->params().location :
                 _imp->params.location)
                (k::distdir(), _imp->params.distdir)
                (k::commands(), join(phase->begin_commands(), phase->end_commands(), " "))
                (k::sandbox(), phase->option("sandbox"))
                (k::userpriv(), phase->option("userpriv") && userpriv_ok)
                (k::builddir(), _imp->params.builddir));

        FSEntry i(_imp->e_repository->layout()->info_variables_file(
                    _imp->e_repository->params().location / "profiles"));

        if (_imp->e_repository->params().master_repository && ! i.exists())
            i = _imp->e_repository->params().master_repository->layout()->info_variables_file(
                    _imp->e_repository->params().master_repository->params().location / "profiles");

        EbuildInfoCommandParams info_params(
                EbuildInfoCommandParams::named_create()
                (k::use(), use)
                (k::use_expand(), join(p->begin_use_expand(), p->end_use_expand(), " "))
                (k::expand_vars(), expand_vars)
                (k::root(), stringify(_imp->params.environment->root()))
                (k::profiles(), _imp->params.profiles)
                (k::load_environment(), static_cast<const FSEntry *>(0))
                (k::info_vars(), i)
                (k::use_ebuild_file(), true)
                );

        EbuildInfoCommand cmd(command_params, info_params);
        cmd();
    }
}

std::string
EbuildEntries::get_environment_variable(const std::tr1::shared_ptr<const ERepositoryID> & id,
        const std::string & var, std::tr1::shared_ptr<const ERepositoryProfile>) const
{
    EAPIPhases phases((*(*id->eapi())[k::supported()])[k::ebuild_phases()].ebuild_variable);

    int c(std::distance(phases.begin_phases(), phases.end_phases()));
    if (1 != c)
        throw EAPIConfigurationError("EAPI '" + (*id->eapi())[k::name()] + "' defines "
                + (c == 0 ? "no" : stringify(c)) + " ebuild variable phases but expected exactly one");

    bool userpriv_restrict;
    {
        using namespace std::tr1::placeholders;

        DepSpecFlattener<RestrictSpecTree, PlainTextDepSpec> restricts(_imp->params.environment);
        if (id->restrict_key())
            id->restrict_key()->value()->accept(restricts);

        userpriv_restrict =
            indirect_iterator(restricts.end()) != std::find_if(indirect_iterator(restricts.begin()), indirect_iterator(restricts.end()),
                    std::tr1::bind(std::equal_to<std::string>(), std::tr1::bind(std::tr1::mem_fn(&StringDepSpec::text), _1), "userpriv")) ||
            indirect_iterator(restricts.end()) != std::find_if(indirect_iterator(restricts.begin()), indirect_iterator(restricts.end()),
                    std::tr1::bind(std::equal_to<std::string>(), std::tr1::bind(std::tr1::mem_fn(&StringDepSpec::text), _1), "nouserpriv"));
    }
    bool userpriv_ok((! userpriv_restrict) && (_imp->environment->reduced_gid() != getgid()) &&
            check_userpriv(FSEntry(_imp->params.builddir), _imp->environment));

    std::tr1::shared_ptr<const FSEntrySequence> exlibsdirs(_imp->e_repository->layout()->exlibsdirs(id->name()));

    EbuildVariableCommand cmd(EbuildCommandParams::named_create()
            (k::environment(), _imp->params.environment)
            (k::package_id(), id)
            (k::ebuild_dir(), _imp->e_repository->layout()->package_directory(id->name()))
            (k::ebuild_file(), id->fs_location_key()->value())
            (k::files_dir(), _imp->e_repository->layout()->package_directory(id->name()) / "files")
            (k::eclassdirs(), _imp->params.eclassdirs)
            (k::exlibsdirs(), exlibsdirs)
            (k::portdir(), _imp->params.master_repository ? _imp->params.master_repository->params().location :
             _imp->params.location)
            (k::distdir(), _imp->params.distdir)
            (k::sandbox(), phases.begin_phases()->option("sandbox"))
            (k::userpriv(), phases.begin_phases()->option("userpriv") && userpriv_ok)
            (k::commands(), join(phases.begin_phases()->begin_commands(), phases.begin_phases()->end_commands(), " "))
            (k::builddir(), _imp->params.builddir),

            var);

    if (! cmd())
        throw ActionError("Couldn't get environment variable '" + stringify(var) + "' for package '" + stringify(*id) + "'");

    return cmd.result();
}

std::tr1::shared_ptr<ERepositoryEntries>
EbuildEntries::make_ebuild_entries(
        const Environment * const e, ERepository * const r, const ERepositoryParams & p)
{
    return std::tr1::shared_ptr<ERepositoryEntries>(new EbuildEntries(e, r, p));
}

void
EbuildEntries::merge(const MergeParams & m)
{
    Context context("When merging '" + stringify(*m[k::package_id()]) + "' at '" + stringify(m[k::image_dir()])
            + "' to E repository '" + stringify(_imp->e_repository->name()) + "':");

    if (! _imp->e_repository->is_suitable_destination_for(*m[k::package_id()]))
        throw InstallActionError("Not a suitable destination for '" + stringify(*m[k::package_id()]) + "'");

    FSEntry binary_ebuild_location(_imp->e_repository->layout()->binary_ebuild_location(
                m[k::package_id()]->name(), m[k::package_id()]->version(),
                "pbin-1+" + (*std::tr1::static_pointer_cast<const ERepositoryID>(m[k::package_id()])->eapi())[k::name()]));

    binary_ebuild_location.dirname().dirname().mkdir();
    binary_ebuild_location.dirname().mkdir();

    WriteBinaryEbuildCommand write_binary_ebuild_command(
            WriteBinaryEbuildCommandParams::named_create()
            (k::environment(), _imp->params.environment)
            (k::package_id(), std::tr1::static_pointer_cast<const ERepositoryID>(m[k::package_id()]))
            (k::binary_ebuild_location(), binary_ebuild_location)
            (k::binary_distdir(), _imp->params.binary_distdir)
            (k::environment_file(), m[k::environment_file()])
            (k::image(), m[k::image_dir()])
            (k::destination_repository(), _imp->e_repository)
            (k::builddir(), _imp->params.builddir)
            (k::merger_options(), (*(*std::tr1::static_pointer_cast<const ERepositoryID>(m[k::package_id()])->eapi())[k::supported()])
             [k::merger_options()])
            );

    write_binary_ebuild_command();
}

bool
EbuildEntries::is_package_file(const QualifiedPackageName & n, const FSEntry & e) const
{
    Context context("When working out whether '" + stringify(e) + "' is a package file for '" + stringify(n) + "':");

    if (0 != e.basename().compare(0, stringify(n.package).length() + 1, stringify(n.package) + "-"))
        return false;

    std::string::size_type p(e.basename().rfind('.'));
    if (std::string::npos == p)
        return false;

    std::string suffix(e.basename().substr(p + 1));
    return Suffixes::get_instance()->is_known_suffix(suffix);
}

VersionSpec
EbuildEntries::extract_package_file_version(const QualifiedPackageName & n, const FSEntry & e) const
{
    Context context("When extracting version from '" + stringify(e) + "':");
    std::string::size_type p(e.basename().rfind('.'));
    if (std::string::npos == p)
        throw InternalError(PALUDIS_HERE, "got npos");
    return VersionSpec(strip_leading_string(e.basename().substr(0, p), stringify(n.package) + "-"));
}

bool
EbuildEntries::pretend(const std::tr1::shared_ptr<const ERepositoryID> & id,
        std::tr1::shared_ptr<const ERepositoryProfile> p) const
{
    using namespace std::tr1::placeholders;

    Context context("When running pretend for '" + stringify(*id) + "':");

    if (! (*id->eapi())[k::supported()])
        return true;
    if ((*(*id->eapi())[k::supported()])[k::ebuild_phases()].ebuild_pretend.empty())
        return true;

    bool userpriv_restrict;
    {
        DepSpecFlattener<RestrictSpecTree, PlainTextDepSpec> restricts(_imp->params.environment);
        if (id->restrict_key())
            id->restrict_key()->value()->accept(restricts);

        userpriv_restrict =
            indirect_iterator(restricts.end()) != std::find_if(indirect_iterator(restricts.begin()), indirect_iterator(restricts.end()),
                    std::tr1::bind(std::equal_to<std::string>(), std::tr1::bind(std::tr1::mem_fn(&StringDepSpec::text), _1), "userpriv")) ||
            indirect_iterator(restricts.end()) != std::find_if(indirect_iterator(restricts.begin()), indirect_iterator(restricts.end()),
                    std::tr1::bind(std::equal_to<std::string>(), std::tr1::bind(std::tr1::mem_fn(&StringDepSpec::text), _1), "nouserpriv"));
    }
    bool userpriv_ok((! userpriv_restrict) && (_imp->environment->reduced_gid() != getgid()) &&
            check_userpriv(FSEntry(_imp->params.builddir), _imp->environment));

    std::string use(make_use(_imp->params.environment, *id, p));
    std::string expand_sep(stringify((*(*id->eapi())[k::supported()])[k::ebuild_options()].use_expand_separator));
    std::tr1::shared_ptr<Map<std::string, std::string> > expand_vars(make_expand(
                _imp->params.environment, *id, p, use, expand_sep));

    std::tr1::shared_ptr<const FSEntrySequence> exlibsdirs(_imp->e_repository->layout()->exlibsdirs(id->name()));

    EAPIPhases phases((*(*id->eapi())[k::supported()])[k::ebuild_phases()].ebuild_pretend);
    for (EAPIPhases::ConstIterator phase(phases.begin_phases()), phase_end(phases.end_phases()) ;
            phase != phase_end ; ++phase)
    {
        EbuildCommandParams command_params(EbuildCommandParams::named_create()
                (k::environment(), _imp->params.environment)
                (k::package_id(), id)
                (k::ebuild_dir(), _imp->e_repository->layout()->package_directory(id->name()))
                (k::ebuild_file(), id->fs_location_key()->value())
                (k::files_dir(), _imp->e_repository->layout()->package_directory(id->name()) / "files")
                (k::eclassdirs(), _imp->params.eclassdirs)
                (k::exlibsdirs(), exlibsdirs)
                (k::portdir(), _imp->params.master_repository ? _imp->params.master_repository->params().location :
                   _imp->params.location)
                (k::distdir(), _imp->params.distdir)
                (k::userpriv(), phase->option("userpriv") && userpriv_ok)
                (k::sandbox(), phase->option("sandbox"))
                (k::commands(), join(phase->begin_commands(), phase->end_commands(), " "))
                (k::builddir(), _imp->params.builddir));

        EbuildPretendCommand pretend_cmd(command_params,
                EbuildPretendCommandParams::named_create()
                (k::use(), use)
                (k::use_expand(), join(p->begin_use_expand(), p->end_use_expand(), " "))
                (k::expand_vars(), expand_vars)
                (k::root(), stringify(_imp->params.environment->root()))
                (k::profiles(), _imp->params.profiles));

        if (! pretend_cmd())
            return false;
    }

    return true;
}

std::string
EbuildEntries::get_package_file_manifest_key(const FSEntry & e, const QualifiedPackageName & q) const
{
    if (! is_package_file(q, e))
        return "";

    std::string::size_type p(e.basename().rfind('.'));
    if (std::string::npos == p)
        return "EBUILD";

    std::string suffix(e.basename().substr(p + 1));
    return Suffixes::get_instance()->manifest_key(suffix);
}

std::string
EbuildEntries::binary_ebuild_name(const QualifiedPackageName & q, const VersionSpec & v, const std::string & e) const
{
    return stringify(q.package) + "-" + stringify(v) + "." + e;
}

std::string
EbuildEntries::_guess_eapi(const QualifiedPackageName &, const FSEntry & e) const
{
    std::string::size_type p(e.basename().rfind('.'));
    if (std::string::npos == p)
        return "";

    std::string suffix(e.basename().substr(p + 1));
    return Suffixes::get_instance()->guess_eapi(suffix);
}

