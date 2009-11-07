/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009 Ciaran McCreesh
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
#include <paludis/repositories/e/myoptions_requirements_verifier.hh>
#include <paludis/repositories/e/can_skip_phase.hh>

#include <paludis/action.hh>
#include <paludis/dep_spec_flattener.hh>
#include <paludis/environment.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/choice.hh>
#include <paludis/elike_choices.hh>
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
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/config_file.hh>
#include <paludis/util/instantiation_policy-impl.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/options.hh>
#include <paludis/output_manager.hh>
#include <tr1/functional>
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
            eclass_mtimes(new EclassMtimes(p, k.eclassdirs())),
            master_mtime(0)
        {
            FSEntry m(k.location() / "metadata" / "timestamp");
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

    std::tr1::shared_ptr<EbuildID> result(new EbuildID(q, extract_package_file_version(q, f), _imp->params.environment(),
                _imp->e_repository->shared_from_this(), f, _guess_eapi(q, f),
                _imp->master_mtime, _imp->eclass_mtimes));
    return result;
}

namespace
{
    class AFinder :
        private InstantiationPolicy<AFinder, instantiation_method::NonCopyableTag>
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

            void visit(const FetchableURISpecTree::NodeType<FetchableURIDepSpec>::Type & node)
            {
                _specs.push_back(std::make_pair(node.spec().get(), *_labels.begin()));
            }

            void visit(const FetchableURISpecTree::NodeType<URILabelsDepSpec>::Type & node)
            {
                *_labels.begin() = node.spec().get();
            }

            void visit(const FetchableURISpecTree::NodeType<AllDepSpec>::Type & node)
            {
                _labels.push_front(*_labels.begin());
                std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
                _labels.pop_front();
            }

            void visit(const FetchableURISpecTree::NodeType<ConditionalDepSpec>::Type & node)
            {
                if (node.spec()->condition_met())
                {
                    _labels.push_front(*_labels.begin());
                    std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
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
    std::string make_use(const Environment * const,
            const ERepositoryID & id,
            std::tr1::shared_ptr<const Profile> profile)
    {
        if (! id.eapi()->supported())
        {
            Log::get_instance()->message("e.ebuild.unknown_eapi", ll_warning, lc_context)
                << "Can't make the USE string for '" << id << "' because its EAPI is unsupported";
            return "";
        }

        std::string use;

        if (id.choices_key())
        {
            for (Choices::ConstIterator k(id.choices_key()->value()->begin()),
                    k_end(id.choices_key()->value()->end()) ;
                    k != k_end ; ++k)
            {
                if ((*k)->prefix() == canonical_build_options_prefix())
                    continue;

                for (Choice::ConstIterator i((*k)->begin()), i_end((*k)->end()) ;
                        i != i_end ; ++i)
                    if ((*i)->enabled())
                        use += stringify((*i)->name_with_prefix()) + " ";
            }
        }

        if (! id.eapi()->supported()->ebuild_environment_variables()->env_arch().empty())
            use += profile->environment_variable(id.eapi()->supported()->ebuild_environment_variables()->env_arch()) + " ";

        return use;
    }

    std::tr1::shared_ptr<Map<std::string, std::string> >
    make_expand(const Environment * const,
            const ERepositoryID & e,
            std::tr1::shared_ptr<const Profile> profile)
    {
        std::tr1::shared_ptr<Map<std::string, std::string> > expand_vars(
            new Map<std::string, std::string>);

        if (! e.eapi()->supported())
        {
            Log::get_instance()->message("e.ebuild.unknown_eapi", ll_warning, lc_context)
                << "Can't make the USE_EXPAND strings for '" << e << "' because its EAPI is unsupported";
            return expand_vars;
        }

        if (! e.choices_key())
            return expand_vars;

        for (Set<std::string>::ConstIterator x(profile->use_expand()->begin()), x_end(profile->use_expand()->end()) ;
                x != x_end ; ++x)
        {
            expand_vars->insert(stringify(*x), "");

            Choices::ConstIterator k(std::find_if(e.choices_key()->value()->begin(), e.choices_key()->value()->end(),
                        std::tr1::bind(std::equal_to<std::string>(), *x,
                            std::tr1::bind(std::tr1::mem_fn(&Choice::raw_name), std::tr1::placeholders::_1))));
            if (k == e.choices_key()->value()->end())
                continue;

            for (Choice::ConstIterator i((*k)->begin()), i_end((*k)->end()) ;
                    i != i_end ; ++i)
                if ((*i)->enabled())
                {
                    std::string value;
                    Map<std::string, std::string>::ConstIterator v(expand_vars->find(stringify(*x)));
                    if (expand_vars->end() != v)
                    {
                        value = v->second;
                        if (! value.empty())
                            value.append(" ");
                        expand_vars->erase(v);
                    }
                    value.append(stringify((*i)->unprefixed_name()));
                    expand_vars->insert(stringify(*x), value);
                }
        }

        return expand_vars;
    }
}

namespace
{
    bool
    check_userpriv(const FSEntry & f, const Environment * env, bool mandatory)
    {
        Context c("When checking permissions on '" + stringify(f) + "' for userpriv:");

        if (! getenv_with_default("PALUDIS_BYPASS_USERPRIV_CHECKS", "").empty())
            return false;

        if (f.exists())
        {
            if (f.group() != env->reduced_gid())
            {
                if (mandatory)
                    throw ConfigurationError("Directory '" + stringify(f) + "' owned by group '" + get_group_name(f.group())
                            + "', not '" + get_group_name(env->reduced_gid()) + "'");
                else
                    Log::get_instance()->message("e.ebuild.userpriv_disabled", ll_warning, lc_context) << "Directory '" <<
                        f << "' owned by group '" << get_group_name(f.group()) << "', not '"
                        << get_group_name(env->reduced_gid()) << "', so cannot enable userpriv";
                return false;
            }
            else if (! f.has_permission(fs_ug_group, fs_perm_write))
            {
                if (mandatory)
                    throw ConfigurationError("Directory '" + stringify(f) + "' does not have group write permission");
                else
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
        const FetchAction & fetch_action, const std::tr1::shared_ptr<const Profile> & p) const
{
    using namespace std::tr1::placeholders;

    Context context("When fetching '" + stringify(*id) + "':");

    bool fetch_restrict(false), userpriv_restrict(false);
    {
        DepSpecFlattener<PlainTextSpecTree, PlainTextDepSpec> restricts(_imp->params.environment());
        if (id->restrict_key())
            id->restrict_key()->value()->root()->accept(restricts);

        for (DepSpecFlattener<PlainTextSpecTree, PlainTextDepSpec>::ConstIterator i(restricts.begin()), i_end(restricts.end()) ;
                i != i_end ; ++i)
        {
            if (id->eapi()->supported()->ebuild_options()->restrict_fetch()->end() !=
                    std::find(id->eapi()->supported()->ebuild_options()->restrict_fetch()->begin(),
                        id->eapi()->supported()->ebuild_options()->restrict_fetch()->end(), (*i)->text()))
                fetch_restrict = true;
            if ("userpriv" == (*i)->text() || "nouserpriv" == (*i)->text())
                userpriv_restrict = true;
        }
    }

    bool fetch_userpriv_ok(_imp->environment->reduced_gid() != getgid() &&
            check_userpriv(FSEntry(_imp->params.distdir()), _imp->environment, id->eapi()->supported()->userpriv_cannot_use_root()));

    std::string archives, all_archives;
    {
        std::set<std::string> already_in_archives;

        /* make A */
        AFinder f(_imp->params.environment(), id);
        if (id->fetches_key())
            id->fetches_key()->value()->root()->accept(f);

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
        if (! id->eapi()->supported()->ebuild_environment_variables()->env_aa().empty())
        {
            AAVisitor g;
            if (id->fetches_key())
                id->fetches_key()->value()->root()->accept(g);
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

    std::tr1::shared_ptr<OutputManager> output_manager(fetch_action.options.make_output_manager()(fetch_action));

    CheckFetchedFilesVisitor c(_imp->environment, id, _imp->e_repository->params().distdir(),
            fetch_action.options.fetch_unneeded(), fetch_restrict,
            ((_imp->e_repository->layout()->package_directory(id->name())) / "Manifest"),
            _imp->e_repository->params().use_manifest(),
            output_manager, fetch_action.options.exclude_unmirrorable(),
            fetch_action.options.ignore_unfetched());

    if (id->fetches_key())
    {
        /* always use mirror://gentoo/, where gentoo is the name of our first master repository,
         * or our name if there's no master. */
        std::string mirrors_name(
                (_imp->e_repository->params().master_repositories() && ! _imp->e_repository->params().master_repositories()->empty()) ?
                stringify((*_imp->e_repository->params().master_repositories()->begin())->name()) :
                stringify(_imp->e_repository->name()));

        if (! fetch_action.options.ignore_unfetched())
        {
            FetchVisitor f(_imp->params.environment(), id, *id->eapi(),
                    _imp->e_repository->params().distdir(), fetch_action.options.fetch_unneeded(),
                    fetch_userpriv_ok, mirrors_name,
                    id->fetches_key()->initial_label(), fetch_action.options.safe_resume(),
                    output_manager);
            id->fetches_key()->value()->root()->accept(f);
        }

        id->fetches_key()->value()->root()->accept(c);
    }

    if ( (! fetch_action.options.fetch_regulars_only()) && ((c.need_nofetch()) ||
            ((! fetch_action.options.ignore_unfetched()) && (! id->eapi()->supported()->ebuild_phases()->ebuild_fetch_extra().empty()))))
    {
        bool userpriv_ok((! userpriv_restrict) && (_imp->environment->reduced_gid() != getgid()) &&
                check_userpriv(FSEntry(_imp->params.builddir()), _imp->environment,
                    id->eapi()->supported()->userpriv_cannot_use_root()));
        std::string use(make_use(_imp->params.environment(), *id, p));
        std::tr1::shared_ptr<Map<std::string, std::string> > expand_vars(make_expand(
                    _imp->params.environment(), *id, p));

        std::tr1::shared_ptr<const FSEntrySequence> exlibsdirs(_imp->e_repository->layout()->exlibsdirs(id->name()));

        EAPIPhases fetch_extra_phases(id->eapi()->supported()->ebuild_phases()->ebuild_fetch_extra());
        if ((! fetch_action.options.ignore_unfetched()) && (fetch_extra_phases.begin_phases() != fetch_extra_phases.end_phases()))
        {
            FSEntry package_builddir(_imp->params.builddir() / (stringify(id->name().category()) + "-" +
                    stringify(id->name().package()) + "-" + stringify(id->version()) + "-fetch_extra"));

            for (EAPIPhases::ConstIterator phase(fetch_extra_phases.begin_phases()), phase_end(fetch_extra_phases.end_phases()) ;
                    phase != phase_end ; ++phase)
            {
                if (can_skip_phase(id, *phase))
                    continue;

                EbuildCommandParams command_params(make_named_values<EbuildCommandParams>(
                        value_for<n::builddir>(_imp->params.builddir()),
                        value_for<n::clearenv>(phase->option("clearenv")),
                        value_for<n::commands>(join(phase->begin_commands(), phase->end_commands(), " ")),
                        value_for<n::distdir>(_imp->params.distdir()),
                        value_for<n::ebuild_dir>(_imp->e_repository->layout()->package_directory(id->name())),
                        value_for<n::ebuild_file>(id->fs_location_key()->value()),
                        value_for<n::eclassdirs>(_imp->params.eclassdirs()),
                        value_for<n::environment>(_imp->params.environment()),
                        value_for<n::exlibsdirs>(exlibsdirs),
                        value_for<n::files_dir>(_imp->e_repository->layout()->package_directory(id->name()) / "files"),
                        value_for<n::maybe_output_manager>(output_manager),
                        value_for<n::package_builddir>(package_builddir),
                        value_for<n::package_id>(id),
                        value_for<n::portdir>(
                            (_imp->params.master_repositories() && ! _imp->params.master_repositories()->empty()) ?
                            (*_imp->params.master_repositories()->begin())->params().location() : _imp->params.location()),
                        value_for<n::sandbox>(phase->option("sandbox")),
                        value_for<n::sydbox>(phase->option("sydbox")),
                        value_for<n::userpriv>(phase->option("userpriv") && userpriv_ok)
                        ));

                EbuildFetchExtraCommand fetch_extra_cmd(command_params,
                        make_named_values<EbuildFetchExtraCommandParams>(
                        value_for<n::a>(archives),
                        value_for<n::aa>(all_archives),
                        value_for<n::expand_vars>(expand_vars),
                        value_for<n::loadsaveenv_dir>(package_builddir / "temp"),
                        value_for<n::profiles>(_imp->params.profiles()),
                        value_for<n::root>("/"),
                        value_for<n::slot>(id->slot_key() ? stringify(id->slot_key()->value()) : ""),
                        value_for<n::use>(use),
                        value_for<n::use_expand>(join(p->use_expand()->begin(), p->use_expand()->end(), " ")),
                        value_for<n::use_expand_hidden>(join(p->use_expand_hidden()->begin(), p->use_expand_hidden()->end(), " "))
                        ));

                if (! fetch_extra_cmd())
                    throw ActionFailedError("Fetch of '" + stringify(*id) + "' failed");
            }
        }

        if (c.need_nofetch())
        {
            EAPIPhases phases(id->eapi()->supported()->ebuild_phases()->ebuild_nofetch());
            for (EAPIPhases::ConstIterator phase(phases.begin_phases()), phase_end(phases.end_phases()) ;
                    phase != phase_end ; ++phase)
            {
                EbuildCommandParams command_params(make_named_values<EbuildCommandParams>(
                        value_for<n::builddir>(_imp->params.builddir()),
                        value_for<n::clearenv>(phase->option("clearenv")),
                        value_for<n::commands>(join(phase->begin_commands(), phase->end_commands(), " ")),
                        value_for<n::distdir>(_imp->params.distdir()),
                        value_for<n::ebuild_dir>(_imp->e_repository->layout()->package_directory(id->name())),
                        value_for<n::ebuild_file>(id->fs_location_key()->value()),
                        value_for<n::eclassdirs>(_imp->params.eclassdirs()),
                        value_for<n::environment>(_imp->params.environment()),
                        value_for<n::exlibsdirs>(exlibsdirs),
                        value_for<n::files_dir>(_imp->e_repository->layout()->package_directory(id->name()) / "files"),
                        value_for<n::maybe_output_manager>(output_manager),
                        value_for<n::package_builddir>(_imp->params.builddir() / (stringify(id->name().category()) + "-" + stringify(id->name().package()) + "-" + stringify(id->version()) + "-nofetch")),
                        value_for<n::package_id>(id),
                        value_for<n::portdir>(
                            (_imp->params.master_repositories() && ! _imp->params.master_repositories()->empty()) ?
                            (*_imp->params.master_repositories()->begin())->params().location() : _imp->params.location()),
                        value_for<n::sandbox>(phase->option("sandbox")),
                        value_for<n::sydbox>(phase->option("sydbox")),
                        value_for<n::userpriv>(phase->option("userpriv") && userpriv_ok)
                        ));

                EbuildNoFetchCommand nofetch_cmd(command_params,
                        make_named_values<EbuildNoFetchCommandParams>(
                        value_for<n::a>(archives),
                        value_for<n::aa>(all_archives),
                        value_for<n::expand_vars>(expand_vars),
                        value_for<n::profiles>(_imp->params.profiles()),
                        value_for<n::root>("/"),
                        value_for<n::use>(use),
                        value_for<n::use_expand>(join(p->use_expand()->begin(), p->use_expand()->end(), " ")),
                        value_for<n::use_expand_hidden>(join(p->use_expand_hidden()->begin(), p->use_expand_hidden()->end(), " "))
                        ));

                if (! nofetch_cmd())
                {
                    std::copy(c.failures()->begin(), c.failures()->end(),
                            fetch_action.options.errors()->back_inserter());
                    throw ActionFailedError("Fetch of '" + stringify(*id) + "' failed");
                }
            }
        }
    }

    if (! c.failures()->empty())
    {
        std::copy(c.failures()->begin(), c.failures()->end(),
                fetch_action.options.errors()->back_inserter());
        throw ActionFailedError("Fetch of '" + stringify(*id) + "' failed");
    }

    output_manager->succeeded();
}

void
EbuildEntries::pretend_fetch(const std::tr1::shared_ptr<const ERepositoryID> & id,
        PretendFetchAction & a, const std::tr1::shared_ptr<const Profile> &) const
{
    using namespace std::tr1::placeholders;

    Context context("When pretending to fetch ID '" + stringify(*id) + "':");

    if (id->fetches_key())
    {
        PretendFetchVisitor f(_imp->params.environment(), id, *id->eapi(),
                _imp->e_repository->params().distdir(), a.options.fetch_unneeded(),
                id->fetches_key()->initial_label(), a);
        id->fetches_key()->value()->root()->accept(f);
    }
}

namespace
{
    bool slot_is_same(const std::tr1::shared_ptr<const PackageID> & a,
            const std::tr1::shared_ptr<const PackageID> & b)
    {
        if (a->slot_key())
            return b->slot_key() && a->slot_key()->value() == b->slot_key()->value();
        else
            return ! b->slot_key();
    }

    void used_this_for_config_protect(std::string & s, const std::string & v)
    {
        s = v;
    }

    std::tr1::shared_ptr<OutputManager> this_output_manager(const std::tr1::shared_ptr<OutputManager> & o, const Action &)
    {
        return o;
    }

    void installed_this(const FSEntry &)
    {
    }

    bool ignore_merged(const std::tr1::shared_ptr<const FSEntrySet> & s,
            const FSEntry & f)
    {
        return s->end() != s->find(f);
    }
}

void
EbuildEntries::install(const std::tr1::shared_ptr<const ERepositoryID> & id,
        const InstallAction & install_action, const std::tr1::shared_ptr<const Profile> & p) const
{
    using namespace std::tr1::placeholders;

    Context context("When installing '" + stringify(*id) + "'" +
            (install_action.options.replacing()->empty() ? "" : " replacing { '"
             + join(indirect_iterator(install_action.options.replacing()->begin()),
                 indirect_iterator(install_action.options.replacing()->end()), "', '") + "' }") + ":");

    std::tr1::shared_ptr<OutputManager> output_manager(install_action.options.make_output_manager()(install_action));

    bool userpriv_restrict, test_restrict, strip_restrict;
    {
        DepSpecFlattener<PlainTextSpecTree, PlainTextDepSpec> restricts(_imp->params.environment());
        if (id->restrict_key())
            id->restrict_key()->value()->root()->accept(restricts);

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
        AFinder f(_imp->params.environment(), id);
        if (id->fetches_key())
            id->fetches_key()->value()->root()->accept(f);

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
        if (! id->eapi()->supported()->ebuild_environment_variables()->env_aa().empty())
        {
            AAVisitor g;
            if (id->fetches_key())
                id->fetches_key()->value()->root()->accept(g);
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
    std::string use(make_use(_imp->params.environment(), *id, p));

    /* add expand to use (iuse isn't reliable for use_expand things), and make the expand
     * environment variables */
    std::tr1::shared_ptr<Map<std::string, std::string> > expand_vars(make_expand(
                _imp->params.environment(), *id, p));

    std::tr1::shared_ptr<const FSEntrySequence> exlibsdirs(_imp->e_repository->layout()->exlibsdirs(id->name()));

    bool userpriv_ok((! userpriv_restrict) && (_imp->environment->reduced_gid() != getgid()) &&
            check_userpriv(FSEntry(_imp->params.distdir()),  _imp->environment, id->eapi()->supported()->userpriv_cannot_use_root()) &&
            check_userpriv(FSEntry(_imp->params.builddir()), _imp->environment, id->eapi()->supported()->userpriv_cannot_use_root()));

    FSEntry package_builddir(_imp->params.builddir() / (stringify(id->name().category()) + "-" +
            stringify(id->name().package()) + "-" + stringify(id->version())));

    std::string used_config_protect;
    std::tr1::shared_ptr<FSEntrySet> merged_entries(new FSEntrySet);

    EAPIPhases phases(id->eapi()->supported()->ebuild_phases()->ebuild_install());
    for (EAPIPhases::ConstIterator phase(phases.begin_phases()), phase_end(phases.end_phases()) ;
            phase != phase_end ; ++phase)
    {
        bool skip(false);
        do
        {
            switch (install_action.options.want_phase()(phase->equal_option("skipname")))
            {
                case wp_yes:
                    continue;

                case wp_skip:
                    skip = true;
                    continue;

                case wp_abort:
                    throw ActionAbortedError("Told to abort install");

                case last_wp:
                    break;
            }

            throw InternalError(PALUDIS_HERE, "bad want_phase");
        } while (false);

        if (skip)
            continue;

        if (can_skip_phase(id, *phase))
        {
            output_manager->stdout_stream() << "--- No need to do anything for " << phase->equal_option("skipname") << " phase" << std::endl;
            continue;
        }

        if (phase->option("merge"))
        {
            if (! (*install_action.options.destination()).destination_interface())
                throw ActionFailedError("Can't install '" + stringify(*id)
                        + "' to destination '" + stringify(install_action.options.destination()->name())
                        + "' because destination does not provide destination_interface");

                (*install_action.options.destination()).destination_interface()->merge(
                        make_named_values<MergeParams>(
                            value_for<n::environment_file>(package_builddir / "temp" / "loadsaveenv"),
                            value_for<n::image_dir>(package_builddir / "image"),
                            value_for<n::merged_entries>(merged_entries),
                            value_for<n::options>(id->eapi()->supported()->merger_options()),
                            value_for<n::output_manager>(output_manager),
                            value_for<n::package_id>(id),
                            value_for<n::perform_uninstall>(install_action.options.perform_uninstall()),
                            value_for<n::used_this_for_config_protect>(std::tr1::bind(
                                    &used_this_for_config_protect, std::tr1::ref(used_config_protect), std::tr1::placeholders::_1))
                            ));
        }
        else if (phase->option("strip"))
        {
            if (! strip_restrict)
            {
                std::string libdir("lib");
                FSEntry root(install_action.options.destination()->installed_root_key() ?
                        stringify(install_action.options.destination()->installed_root_key()->value()) : "/");
                if ((root / "usr" / "lib").is_symbolic_link())
                {
                    libdir = (root / "usr" / "lib").readlink();
                    if (std::string::npos != libdir.find_first_of("./"))
                        libdir = "lib";
                }

                Log::get_instance()->message("e.ebuild.libdir", ll_debug, lc_context) << "Using '" << libdir << "' for libdir";

                std::tr1::shared_ptr<const ChoiceValue> strip_choice(id->choices_key()->value()->find_by_name_with_prefix(
                            ELikeStripChoiceValue::canonical_name_with_prefix()));
                std::tr1::shared_ptr<const ChoiceValue> split_choice(id->choices_key()->value()->find_by_name_with_prefix(
                            ELikeSplitChoiceValue::canonical_name_with_prefix()));

                EStripper stripper(make_named_values<EStripperOptions>(
                        value_for<n::debug_dir>(package_builddir / "image" / "usr" / libdir / "debug"),
                        value_for<n::image_dir>(package_builddir / "image"),
                        value_for<n::output_manager>(output_manager),
                        value_for<n::package_id>(id),
                        value_for<n::split>(split_choice && split_choice->enabled()),
                        value_for<n::strip>(strip_choice && strip_choice->enabled())
                        ));
                stripper.strip();
            }
        }
        else if ((! phase->option("prepost")) ||
                ((*install_action.options.destination()).destination_interface() &&
                 (*install_action.options.destination()).destination_interface()->want_pre_post_phases()))
        {
            if (phase->option("optional_tests"))
            {
                if (test_restrict)
                    continue;

                std::tr1::shared_ptr<const ChoiceValue> choice(id->choices_key()->value()->find_by_name_with_prefix(
                            ELikeOptionalTestsChoiceValue::canonical_name_with_prefix()));
                if (choice && ! choice->enabled())
                    continue;
            }
            else if (phase->option("recommended_tests"))
            {
                if (test_restrict)
                    continue;

                std::tr1::shared_ptr<const ChoiceValue> choice(id->choices_key()->value()->find_by_name_with_prefix(
                            ELikeRecommendedTestsChoiceValue::canonical_name_with_prefix()));
                if (choice && ! choice->enabled())
                    continue;
            }
            else if (phase->option("expensive_tests"))
            {
                std::tr1::shared_ptr<const ChoiceValue> choice(id->choices_key()->value()->find_by_name_with_prefix(
                            ELikeExpensiveTestsChoiceValue::canonical_name_with_prefix()));
                if (choice && ! choice->enabled())
                    continue;
            }

            EbuildCommandParams command_params(make_named_values<EbuildCommandParams>(
                    value_for<n::builddir>(_imp->params.builddir()),
                    value_for<n::clearenv>(phase->option("clearenv")),
                    value_for<n::commands>(join(phase->begin_commands(), phase->end_commands(), " ")),
                    value_for<n::distdir>(_imp->params.distdir()),
                    value_for<n::ebuild_dir>(_imp->e_repository->layout()->package_directory(id->name())),
                    value_for<n::ebuild_file>(id->fs_location_key()->value()),
                    value_for<n::eclassdirs>(_imp->params.eclassdirs()),
                    value_for<n::environment>(_imp->params.environment()),
                    value_for<n::exlibsdirs>(exlibsdirs),
                    value_for<n::files_dir>(_imp->e_repository->layout()->package_directory(id->name()) / "files"),
                    value_for<n::maybe_output_manager>(output_manager),
                    value_for<n::package_builddir>(package_builddir),
                    value_for<n::package_id>(id),
                    value_for<n::portdir>(
                        (_imp->params.master_repositories() && ! _imp->params.master_repositories()->empty()) ?
                        (*_imp->params.master_repositories()->begin())->params().location() : _imp->params.location()),
                    value_for<n::sandbox>(phase->option("sandbox")),
                    value_for<n::sydbox>(phase->option("sydbox")),
                    value_for<n::userpriv>(phase->option("userpriv") && userpriv_ok)
                    ));

            EbuildInstallCommandParams install_params(
                    make_named_values<EbuildInstallCommandParams>(
                            value_for<n::a>(archives),
                            value_for<n::aa>(all_archives),
                            value_for<n::config_protect>(_imp->e_repository->profile_variable("CONFIG_PROTECT")),
                            value_for<n::config_protect_mask>(_imp->e_repository->profile_variable("CONFIG_PROTECT_MASK")),
                            value_for<n::expand_vars>(expand_vars),
                            value_for<n::loadsaveenv_dir>(package_builddir / "temp"),
                            value_for<n::profiles>(_imp->params.profiles()),
                            value_for<n::replacing_ids>(install_action.options.replacing()),
                            value_for<n::root>(install_action.options.destination()->installed_root_key() ?
                                stringify(install_action.options.destination()->installed_root_key()->value()) :
                                "/"),
                            value_for<n::slot>(id->slot_key() ? stringify(id->slot_key()->value()) : ""),
                            value_for<n::use>(use),
                            value_for<n::use_expand>(join(p->use_expand()->begin(), p->use_expand()->end(), " ")),
                            value_for<n::use_expand_hidden>(join(p->use_expand_hidden()->begin(), p->use_expand_hidden()->end(), " "))
                            ));

            EbuildInstallCommand cmd(command_params, install_params);
            cmd();
        }
    }

    for (PackageIDSequence::ConstIterator i(install_action.options.replacing()->begin()), i_end(install_action.options.replacing()->end()) ;
            i != i_end ; ++i)
    {
        Context local_context("When cleaning '" + stringify(**i) + "':");
        if ((*i)->name() == id->name() && (*i)->version() == id->version())
            continue;

        if (id->eapi()->supported()->ebuild_phases()->ebuild_new_upgrade_phase_order())
            if ((*i)->name() == id->name() && slot_is_same(*i, id))
                continue;

        UninstallActionOptions uo(make_named_values<UninstallActionOptions>(
                    value_for<n::config_protect>(used_config_protect),
                    value_for<n::if_for_install_id>(id),
                    value_for<n::ignore_for_unmerge>(std::tr1::bind(&ignore_merged, merged_entries,
                            std::tr1::placeholders::_1)),
                    value_for<n::is_overwrite>(false),
                    value_for<n::make_output_manager>(std::tr1::bind(&this_output_manager, output_manager, std::tr1::placeholders::_1))
                    ));
        install_action.options.perform_uninstall()(*i, uo);
    }

    output_manager->succeeded();
}

void
EbuildEntries::info(const std::tr1::shared_ptr<const ERepositoryID> & id,
        const InfoAction & a,
        const std::tr1::shared_ptr<const Profile> & p) const
{
    using namespace std::tr1::placeholders;

    Context context("When infoing '" + stringify(*id) + "':");

    std::tr1::shared_ptr<OutputManager> output_manager(a.options.make_output_manager()(a));

    bool userpriv_restrict;
    {
        DepSpecFlattener<PlainTextSpecTree, PlainTextDepSpec> restricts(_imp->params.environment());
        if (id->restrict_key())
            id->restrict_key()->value()->root()->accept(restricts);

        userpriv_restrict =
            indirect_iterator(restricts.end()) != std::find_if(indirect_iterator(restricts.begin()), indirect_iterator(restricts.end()),
                    std::tr1::bind(std::equal_to<std::string>(), std::tr1::bind(std::tr1::mem_fn(&StringDepSpec::text), _1), "userpriv")) ||
            indirect_iterator(restricts.end()) != std::find_if(indirect_iterator(restricts.begin()), indirect_iterator(restricts.end()),
                    std::tr1::bind(std::equal_to<std::string>(), std::tr1::bind(std::tr1::mem_fn(&StringDepSpec::text), _1), "nouserpriv"));
    }
    bool userpriv_ok((! userpriv_restrict) && (_imp->environment->reduced_gid() != getgid()) &&
            check_userpriv(FSEntry(_imp->params.builddir()), _imp->environment, id->eapi()->supported()->userpriv_cannot_use_root()));

    /* make use */
    std::string use(make_use(_imp->params.environment(), *id, p));

    /* add expand to use (iuse isn't reliable for use_expand things), and make the expand
     * environment variables */
    std::tr1::shared_ptr<Map<std::string, std::string> > expand_vars(make_expand(
                _imp->params.environment(), *id, p));

    std::tr1::shared_ptr<const FSEntrySequence> exlibsdirs(_imp->e_repository->layout()->exlibsdirs(id->name()));

    EAPIPhases phases(id->eapi()->supported()->ebuild_phases()->ebuild_info());
    for (EAPIPhases::ConstIterator phase(phases.begin_phases()), phase_end(phases.end_phases()) ;
            phase != phase_end ; ++phase)
    {
        if (phase->option("installed=true"))
            continue;

        EbuildCommandParams command_params(make_named_values<EbuildCommandParams>(
                value_for<n::builddir>(_imp->params.builddir()),
                value_for<n::clearenv>(phase->option("clearenv")),
                value_for<n::commands>(join(phase->begin_commands(), phase->end_commands(), " ")),
                value_for<n::distdir>(_imp->params.distdir()),
                value_for<n::ebuild_dir>(_imp->e_repository->layout()->package_directory(id->name())),
                value_for<n::ebuild_file>(id->fs_location_key()->value()),
                value_for<n::eclassdirs>(_imp->params.eclassdirs()),
                value_for<n::environment>(_imp->params.environment()),
                value_for<n::exlibsdirs>(exlibsdirs),
                value_for<n::files_dir>(_imp->e_repository->layout()->package_directory(id->name()) / "files"),
                value_for<n::maybe_output_manager>(output_manager),
                value_for<n::package_builddir>(_imp->params.builddir() / (stringify(id->name().category()) + "-" + stringify(id->name().package()) + "-" + stringify(id->version()) + "-info")),
                value_for<n::package_id>(id),
                value_for<n::portdir>(
                    (_imp->params.master_repositories() && ! _imp->params.master_repositories()->empty()) ?
                    (*_imp->params.master_repositories()->begin())->params().location() : _imp->params.location()),
                value_for<n::sandbox>(phase->option("sandbox")),
                value_for<n::sydbox>(phase->option("sydbox")),
                value_for<n::userpriv>(phase->option("userpriv") && userpriv_ok)
                ));

        EbuildInfoCommandParams info_params(
                make_named_values<EbuildInfoCommandParams>(
                value_for<n::expand_vars>(expand_vars),
                value_for<n::info_vars>(_imp->e_repository->info_vars_key() ?
                    _imp->e_repository->info_vars_key()->value() : make_shared_ptr(new const Set<std::string>)),
                value_for<n::load_environment>(static_cast<const FSEntry *>(0)),
                value_for<n::profiles>(_imp->params.profiles()),
                value_for<n::root>(stringify(_imp->params.environment()->root())),
                value_for<n::use>(use),
                value_for<n::use_ebuild_file>(true),
                value_for<n::use_expand>(join(p->use_expand()->begin(), p->use_expand()->end(), " ")),
                value_for<n::use_expand_hidden>(join(p->use_expand_hidden()->begin(), p->use_expand_hidden()->end(), " "))
                ));

        EbuildInfoCommand cmd(command_params, info_params);
        cmd();
    }
}

std::string
EbuildEntries::get_environment_variable(
        const std::tr1::shared_ptr<const ERepositoryID> & id,
        const std::string & var, const std::tr1::shared_ptr<const Profile> &) const
{
    EAPIPhases phases(id->eapi()->supported()->ebuild_phases()->ebuild_variable());

    int c(std::distance(phases.begin_phases(), phases.end_phases()));
    if (1 != c)
        throw EAPIConfigurationError("EAPI '" + id->eapi()->name() + "' defines "
                + (c == 0 ? "no" : stringify(c)) + " ebuild variable phases but expected exactly one");

    bool userpriv_restrict;
    {
        using namespace std::tr1::placeholders;

        DepSpecFlattener<PlainTextSpecTree, PlainTextDepSpec> restricts(_imp->params.environment());
        if (id->restrict_key())
            id->restrict_key()->value()->root()->accept(restricts);

        userpriv_restrict =
            indirect_iterator(restricts.end()) != std::find_if(indirect_iterator(restricts.begin()), indirect_iterator(restricts.end()),
                    std::tr1::bind(std::equal_to<std::string>(), std::tr1::bind(std::tr1::mem_fn(&StringDepSpec::text), _1), "userpriv")) ||
            indirect_iterator(restricts.end()) != std::find_if(indirect_iterator(restricts.begin()), indirect_iterator(restricts.end()),
                    std::tr1::bind(std::equal_to<std::string>(), std::tr1::bind(std::tr1::mem_fn(&StringDepSpec::text), _1), "nouserpriv"));
    }
    bool userpriv_ok((! userpriv_restrict) && (_imp->environment->reduced_gid() != getgid()) &&
            check_userpriv(FSEntry(_imp->params.builddir()), _imp->environment, id->eapi()->supported()->userpriv_cannot_use_root()));

    std::tr1::shared_ptr<const FSEntrySequence> exlibsdirs(_imp->e_repository->layout()->exlibsdirs(id->name()));

    EbuildVariableCommand cmd(make_named_values<EbuildCommandParams>(
            value_for<n::builddir>(_imp->params.builddir()),
            value_for<n::clearenv>(phases.begin_phases()->option("clearenv")),
            value_for<n::commands>(join(phases.begin_phases()->begin_commands(), phases.begin_phases()->end_commands(), " ")),
            value_for<n::distdir>(_imp->params.distdir()),
            value_for<n::ebuild_dir>(_imp->e_repository->layout()->package_directory(id->name())),
            value_for<n::ebuild_file>(id->fs_location_key()->value()),
            value_for<n::eclassdirs>(_imp->params.eclassdirs()),
            value_for<n::environment>(_imp->params.environment()),
            value_for<n::exlibsdirs>(exlibsdirs),
            value_for<n::files_dir>(_imp->e_repository->layout()->package_directory(id->name()) / "files"),
            value_for<n::maybe_output_manager>(make_null_shared_ptr()),
            value_for<n::package_builddir>(_imp->params.builddir() / (stringify(id->name().category()) + "-" + stringify(id->name().package()) + "-" + stringify(id->version()) + "-variable")),
            value_for<n::package_id>(id),
            value_for<n::portdir>(
                (_imp->params.master_repositories() && ! _imp->params.master_repositories()->empty()) ?
                (*_imp->params.master_repositories()->begin())->params().location() : _imp->params.location()),
            value_for<n::sandbox>(phases.begin_phases()->option("sandbox")),
            value_for<n::sydbox>(phases.begin_phases()->option("sydbox")),
            value_for<n::userpriv>(phases.begin_phases()->option("userpriv") && userpriv_ok)
            ),

            var);

    if (! cmd())
        throw ActionFailedError("Couldn't get environment variable '" + stringify(var) +
                "' for package '" + stringify(*id) + "'");

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
    Context context("When merging '" + stringify(*m.package_id()) + "' at '" + stringify(m.image_dir())
            + "' to E repository '" + stringify(_imp->e_repository->name()) + "':");

    if (! _imp->e_repository->is_suitable_destination_for(*m.package_id()))
        throw ActionFailedError("Not a suitable destination for '" + stringify(*m.package_id()) + "'");

    FSEntry binary_ebuild_location(_imp->e_repository->layout()->binary_ebuild_location(
                m.package_id()->name(), m.package_id()->version(),
                "pbin-1+" + std::tr1::static_pointer_cast<const ERepositoryID>(m.package_id())->eapi()->name()));

    binary_ebuild_location.dirname().dirname().mkdir();
    binary_ebuild_location.dirname().mkdir();

    WriteBinaryEbuildCommand write_binary_ebuild_command(
            make_named_values<WriteBinaryEbuildCommandParams>(
            value_for<n::binary_distdir>(_imp->params.binary_distdir()),
            value_for<n::binary_ebuild_location>(binary_ebuild_location),
            value_for<n::builddir>(_imp->params.builddir()),
            value_for<n::destination_repository>(_imp->e_repository),
            value_for<n::environment>(_imp->params.environment()),
            value_for<n::environment_file>(m.environment_file()),
            value_for<n::image>(m.image_dir()),
            value_for<n::maybe_output_manager>(m.output_manager()),
            value_for<n::merger_options>(std::tr1::static_pointer_cast<const ERepositoryID>(m.package_id())->eapi()->supported()->merger_options()),
            value_for<n::package_id>(std::tr1::static_pointer_cast<const ERepositoryID>(m.package_id()))
            ));

    write_binary_ebuild_command();
}

bool
EbuildEntries::is_package_file(const QualifiedPackageName & n, const FSEntry & e) const
{
    Context context("When working out whether '" + stringify(e) + "' is a package file for '" + stringify(n) + "':");

    if (0 != e.basename().compare(0, stringify(n.package()).length() + 1, stringify(n.package()) + "-"))
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
    return VersionSpec(strip_leading_string(e.basename().substr(0, p), stringify(n.package()) + "-"),
            EAPIData::get_instance()->eapi_from_string(
                _imp->e_repository->params().eapi_when_unknown())->supported()->version_spec_options());
}

bool
EbuildEntries::pretend(
        const std::tr1::shared_ptr<const ERepositoryID> & id,
        const PretendAction & a,
        const std::tr1::shared_ptr<const Profile> & p) const
{
    using namespace std::tr1::placeholders;

    Context context("When running pretend for '" + stringify(*id) + "':");

    if (! id->eapi()->supported())
        return false;

    bool result(true);

    if (! id->raw_myoptions_key())
        if (id->eapi()->supported()->ebuild_phases()->ebuild_pretend().empty())
            return result;

    bool userpriv_restrict;
    {
        DepSpecFlattener<PlainTextSpecTree, PlainTextDepSpec> restricts(_imp->params.environment());
        if (id->restrict_key())
            id->restrict_key()->value()->root()->accept(restricts);

        userpriv_restrict =
            indirect_iterator(restricts.end()) != std::find_if(indirect_iterator(restricts.begin()), indirect_iterator(restricts.end()),
                    std::tr1::bind(std::equal_to<std::string>(), std::tr1::bind(std::tr1::mem_fn(&StringDepSpec::text), _1), "userpriv")) ||
            indirect_iterator(restricts.end()) != std::find_if(indirect_iterator(restricts.begin()), indirect_iterator(restricts.end()),
                    std::tr1::bind(std::equal_to<std::string>(), std::tr1::bind(std::tr1::mem_fn(&StringDepSpec::text), _1), "nouserpriv"));
    }
    bool userpriv_ok((! userpriv_restrict) && (_imp->environment->reduced_gid() != getgid()) &&
            check_userpriv(FSEntry(_imp->params.builddir()), _imp->environment, id->eapi()->supported()->userpriv_cannot_use_root()));

    std::string use(make_use(_imp->params.environment(), *id, p));
    std::tr1::shared_ptr<Map<std::string, std::string> > expand_vars(make_expand(
                _imp->params.environment(), *id, p));

    std::tr1::shared_ptr<const FSEntrySequence> exlibsdirs(_imp->e_repository->layout()->exlibsdirs(id->name()));

    std::tr1::shared_ptr<OutputManager> output_manager;

    if (id->raw_myoptions_key())
    {
        MyOptionsRequirementsVerifier verifier(id);
        id->raw_myoptions_key()->value()->root()->accept(verifier);

        if (verifier.unmet_requirements() && ! verifier.unmet_requirements()->empty())
        {
            EAPIPhases phases(id->eapi()->supported()->ebuild_phases()->ebuild_bad_options());
            if (phases.begin_phases() == phases.end_phases())
                throw InternalError(PALUDIS_HERE, "using myoptions but no ebuild_bad_options phase");

            for (EAPIPhases::ConstIterator phase(phases.begin_phases()), phase_end(phases.end_phases()) ;
                    phase != phase_end ; ++phase)
            {
                if (! output_manager)
                    output_manager = a.options.make_output_manager()(a);

                EbuildCommandParams command_params(make_named_values<EbuildCommandParams>(
                            value_for<n::builddir>(_imp->params.builddir()),
                            value_for<n::clearenv>(phase->option("clearenv")),
                            value_for<n::commands>(join(phase->begin_commands(), phase->end_commands(), " ")),
                            value_for<n::distdir>(_imp->params.distdir()),
                            value_for<n::ebuild_dir>(_imp->e_repository->layout()->package_directory(id->name())),
                            value_for<n::ebuild_file>(id->fs_location_key()->value()),
                            value_for<n::eclassdirs>(_imp->params.eclassdirs()),
                            value_for<n::environment>(_imp->params.environment()),
                            value_for<n::exlibsdirs>(exlibsdirs),
                            value_for<n::files_dir>(_imp->e_repository->layout()->package_directory(id->name()) / "files"),
                            value_for<n::maybe_output_manager>(output_manager),
                            value_for<n::package_builddir>(_imp->params.builddir() / (stringify(id->name().category()) + "-" + stringify(id->name().package()) + "-" + stringify(id->version()) + "-bad_options")),
                            value_for<n::package_id>(id),
                            value_for<n::portdir>(
                                (_imp->params.master_repositories() && ! _imp->params.master_repositories()->empty()) ?
                                (*_imp->params.master_repositories()->begin())->params().location() : _imp->params.location()),
                            value_for<n::sandbox>(phase->option("sandbox")),
                            value_for<n::sydbox>(phase->option("sydbox")),
                            value_for<n::userpriv>(phase->option("userpriv") && userpriv_ok)
                            ));

                EbuildBadOptionsCommand bad_options_cmd(command_params,
                        make_named_values<EbuildBadOptionsCommandParams>(
                            value_for<n::expand_vars>(expand_vars),
                            value_for<n::profiles>(_imp->params.profiles()),
                            value_for<n::root>(stringify(_imp->params.environment()->root())),
                            value_for<n::unmet_requirements>(verifier.unmet_requirements()),
                            value_for<n::use>(use),
                            value_for<n::use_expand>(join(p->use_expand()->begin(), p->use_expand()->end(), " ")),
                            value_for<n::use_expand_hidden>(join(p->use_expand_hidden()->begin(), p->use_expand_hidden()->end(), " "))
                            ));

                if (! bad_options_cmd())
                    throw ActionFailedError("Bad options phase died");
            }

            result = false;
        }
    }

    if (id->eapi()->supported()->ebuild_phases()->ebuild_pretend().empty())
        return result;

    EAPIPhases phases(id->eapi()->supported()->ebuild_phases()->ebuild_pretend());
    for (EAPIPhases::ConstIterator phase(phases.begin_phases()), phase_end(phases.end_phases()) ;
            phase != phase_end ; ++phase)
    {
        if (can_skip_phase(id, *phase))
            continue;

        if (! output_manager)
            output_manager = a.options.make_output_manager()(a);

        EbuildCommandParams command_params(make_named_values<EbuildCommandParams>(
                value_for<n::builddir>(_imp->params.builddir()),
                value_for<n::clearenv>(phase->option("clearenv")),
                value_for<n::commands>(join(phase->begin_commands(), phase->end_commands(), " ")),
                value_for<n::distdir>(_imp->params.distdir()),
                value_for<n::ebuild_dir>(_imp->e_repository->layout()->package_directory(id->name())),
                value_for<n::ebuild_file>(id->fs_location_key()->value()),
                value_for<n::eclassdirs>(_imp->params.eclassdirs()),
                value_for<n::environment>(_imp->params.environment()),
                value_for<n::exlibsdirs>(exlibsdirs),
                value_for<n::files_dir>(_imp->e_repository->layout()->package_directory(id->name()) / "files"),
                value_for<n::maybe_output_manager>(output_manager),
                value_for<n::package_builddir>(_imp->params.builddir() / (stringify(id->name().category()) + "-" + stringify(id->name().package()) + "-" + stringify(id->version()) + "-pretend")),
                value_for<n::package_id>(id),
                value_for<n::portdir>(
                    (_imp->params.master_repositories() && ! _imp->params.master_repositories()->empty()) ?
                    (*_imp->params.master_repositories()->begin())->params().location() : _imp->params.location()),
                value_for<n::sandbox>(phase->option("sandbox")),
                value_for<n::sydbox>(phase->option("sydbox")),
                value_for<n::userpriv>(phase->option("userpriv") && userpriv_ok)
                ));

        EbuildPretendCommand pretend_cmd(command_params,
                make_named_values<EbuildPretendCommandParams>(
                value_for<n::expand_vars>(expand_vars),
                value_for<n::profiles>(_imp->params.profiles()),
                value_for<n::root>(stringify(_imp->params.environment()->root())),
                value_for<n::use>(use),
                value_for<n::use_expand>(join(p->use_expand()->begin(), p->use_expand()->end(), " ")),
                value_for<n::use_expand_hidden>(join(p->use_expand_hidden()->begin(), p->use_expand_hidden()->end(), " "))
                ));

        if (! pretend_cmd())
            return false;
    }

    return result;
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
    return stringify(q.package()) + "-" + stringify(v) + "." + e;
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

