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

#include <paludis/repositories/unpackaged/unpackaged_id.hh>
#include <paludis/repositories/unpackaged/unpackaged_key.hh>
#include <paludis/repositories/unpackaged/unpackaged_stripper.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/simple_visitor_cast.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/log.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/return_literal_function.hh>
#include <paludis/output_manager.hh>
#include <paludis/name.hh>
#include <paludis/version_spec.hh>
#include <paludis/package_database.hh>
#include <paludis/environment.hh>
#include <paludis/metadata_key.hh>
#include <paludis/action.hh>
#include <paludis/literal_metadata_key.hh>
#include <paludis/choice.hh>
#include <paludis/elike_choices.hh>
#include <paludis/user_dep_spec.hh>

using namespace paludis;
using namespace paludis::unpackaged_repositories;

namespace paludis
{
    template <>
    struct Implementation<UnpackagedID>
    {
        const Environment * const env;
        const QualifiedPackageName name;
        const VersionSpec version;
        const RepositoryName repository_name;

        std::tr1::shared_ptr<DependenciesLabelSequence> build_dependencies_labels;
        std::tr1::shared_ptr<DependenciesLabelSequence> run_dependencies_labels;

        const std::tr1::shared_ptr<LiteralMetadataValueKey<SlotName> > slot_key;
        const std::tr1::shared_ptr<LiteralMetadataValueKey<FSEntry> > fs_location_key;
        const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > build_dependencies_key;
        const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > run_dependencies_key;
        const std::tr1::shared_ptr<const MetadataValueKey<std::string> > description_key;
        const std::tr1::shared_ptr<const UnpackagedChoicesKey> choices_key;

        Implementation(const Environment * const e,
                const QualifiedPackageName & q,
                const VersionSpec & v,
                const SlotName & s,
                const RepositoryName & n,
                const FSEntry & l,
                const std::string & b,
                const std::string & r,
                const std::string & d,
                const UnpackagedID * const id) :
            env(e),
            name(q),
            version(v),
            repository_name(n),
            build_dependencies_labels(new DependenciesLabelSequence),
            run_dependencies_labels(new DependenciesLabelSequence),
            slot_key(new LiteralMetadataValueKey<SlotName> ("slot", "Slot", mkt_internal, s)),
            fs_location_key(new LiteralMetadataValueKey<FSEntry> ("location", "Location", mkt_normal, l)),
            build_dependencies_key(new UnpackagedDependencyKey(env, "build_dependencies", "Build dependencies", mkt_dependencies,
                        build_dependencies_labels, b)),
            run_dependencies_key(new UnpackagedDependencyKey(env, "run_dependencies", "Run dependencies", mkt_dependencies,
                        run_dependencies_labels, r)),
            description_key(new LiteralMetadataValueKey<std::string> ("description", "Description", mkt_significant, d)),
            choices_key(new UnpackagedChoicesKey(env, "choices", "Choices", mkt_normal, id))
        {
            build_dependencies_labels->push_back(make_shared_ptr(new DependenciesBuildLabel("build_dependencies",
                            return_literal_function(true))));
            run_dependencies_labels->push_back(make_shared_ptr(new DependenciesRunLabel("run_dependencies",
                            return_literal_function(true))));
        }
    };
}

UnpackagedID::UnpackagedID(const Environment * const e, const QualifiedPackageName & q,
        const VersionSpec & v, const SlotName & s, const RepositoryName & n, const FSEntry & l,
        const std::string & b, const std::string & r, const std::string & d) :
    PrivateImplementationPattern<UnpackagedID>(new Implementation<UnpackagedID>(e, q, v, s, n, l, b, r, d, this)),
    _imp(PrivateImplementationPattern<UnpackagedID>::_imp)
{
    add_metadata_key(_imp->slot_key);
    add_metadata_key(_imp->fs_location_key);
    add_metadata_key(_imp->build_dependencies_key);
    add_metadata_key(_imp->run_dependencies_key);
    add_metadata_key(_imp->description_key);
    add_metadata_key(_imp->choices_key);
}

UnpackagedID::~UnpackagedID()
{
}

void
UnpackagedID::need_keys_added() const
{
}

void
UnpackagedID::need_masks_added() const
{
}

const std::string
UnpackagedID::canonical_form(const PackageIDCanonicalForm f) const
{
    switch (f)
    {
        case idcf_full:
            return stringify(_imp->name) + "-" + stringify(_imp->version) + ":" +
                stringify(slot_key()->value()) + "::" + stringify(_imp->repository_name);

        case idcf_version:
            return stringify(_imp->version);

        case idcf_no_version:
            return stringify(_imp->name) + ":" + stringify(slot_key()->value()) + "::" +
                stringify(_imp->repository_name);

        case idcf_no_name:
            return stringify(_imp->version) + ":" +
                stringify(slot_key()->value()) + "::" + stringify(_imp->repository_name);

        case last_idcf:
            break;
    }

    throw InternalError(PALUDIS_HERE, "Bad PackageIDCanonicalForm");
}

PackageDepSpec
UnpackagedID::uniquely_identifying_spec() const
{
    return parse_user_package_dep_spec("=" + stringify(name()) + "-" + stringify(version()) +
            (slot_key() ? ":" + stringify(slot_key()->value()) : "") + "::" + stringify(repository()->name()),
            _imp->env, UserPackageDepSpecOptions());
}

const QualifiedPackageName
UnpackagedID::name() const
{
    return _imp->name;
}

const VersionSpec
UnpackagedID::version() const
{
    return _imp->version;
}

const std::tr1::shared_ptr<const Repository>
UnpackagedID::repository() const
{
    return _imp->env->package_database()->fetch_repository(_imp->repository_name);
}

const std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const PackageID> > >
UnpackagedID::virtual_for_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const PackageID> > >();
}

const std::tr1::shared_ptr<const MetadataCollectionKey<KeywordNameSet> >
UnpackagedID::keywords_key() const
{
    return std::tr1::shared_ptr<const MetadataCollectionKey<KeywordNameSet> >();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<ProvideSpecTree> >
UnpackagedID::provide_key() const
{
    return std::tr1::shared_ptr<const MetadataSpecTreeKey<ProvideSpecTree> >();
}

const std::tr1::shared_ptr<const MetadataCollectionKey<PackageIDSequence> >
UnpackagedID::contains_key() const
{
    return std::tr1::shared_ptr<const MetadataCollectionKey<PackageIDSequence> >();
}

const std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const PackageID> > >
UnpackagedID::contained_in_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const PackageID> > >();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
UnpackagedID::dependencies_key() const
{
    return make_null_shared_ptr();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
UnpackagedID::build_dependencies_key() const
{
    return _imp->build_dependencies_key;
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
UnpackagedID::run_dependencies_key() const
{
    return _imp->run_dependencies_key;
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
UnpackagedID::post_dependencies_key() const
{
    return std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
UnpackagedID::suggested_dependencies_key() const
{
    return std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<FetchableURISpecTree> >
UnpackagedID::fetches_key() const
{
    return std::tr1::shared_ptr<const MetadataSpecTreeKey<FetchableURISpecTree> >();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> >
UnpackagedID::homepage_key() const
{
    return std::tr1::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> >();
}

const std::tr1::shared_ptr<const MetadataValueKey<std::string> >
UnpackagedID::short_description_key() const
{
    return _imp->description_key;
}

const std::tr1::shared_ptr<const MetadataValueKey<std::string> >
UnpackagedID::long_description_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<std::string> >();
}

const std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const Contents> > >
UnpackagedID::contents_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const Contents> > >();
}

const std::tr1::shared_ptr<const MetadataTimeKey>
UnpackagedID::installed_time_key() const
{
    return std::tr1::shared_ptr<const MetadataTimeKey>();
}

const std::tr1::shared_ptr<const MetadataCollectionKey<Set<std::string> > >
UnpackagedID::from_repositories_key() const
{
    return std::tr1::shared_ptr<const MetadataCollectionKey<Set<std::string> > >();
}

const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> >
UnpackagedID::fs_location_key() const
{
    return _imp->fs_location_key;
}

const std::tr1::shared_ptr<const MetadataValueKey<SlotName> >
UnpackagedID::slot_key() const
{
    return _imp->slot_key;
}

const std::tr1::shared_ptr<const MetadataCollectionKey<Set<std::string> > >
UnpackagedID::behaviours_key() const
{
    return make_null_shared_ptr();
}

bool
UnpackagedID::supports_action(const SupportsActionTestBase & test) const
{
    return simple_visitor_cast<const SupportsActionTest<InstallAction> >(test);
}

namespace
{
    bool slot_is_same(const std::tr1::shared_ptr<const PackageID> & a,
            const PackageID * const b)
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

    bool ignore_nothing(const FSEntry &)
    {
        return false;
    }
}

void
UnpackagedID::perform_action(Action & action) const
{
    Timestamp build_start_time(Timestamp::now());

    const InstallAction * const install_action(simple_visitor_cast<const InstallAction>(action));
    if (! install_action)
        throw ActionFailedError("Unsupported action: " + action.simple_name());

    if (! (*install_action->options.destination()).destination_interface())
        throw ActionFailedError("Can't install '" + stringify(*this)
                + "' to destination '" + stringify(install_action->options.destination()->name())
                + "' because destination does not provide destination_interface");

    std::tr1::shared_ptr<OutputManager> output_manager(install_action->options.make_output_manager()(*install_action));

    std::string libdir("lib");
    FSEntry root(install_action->options.destination()->installed_root_key() ?
            stringify(install_action->options.destination()->installed_root_key()->value()) : "/");
    if ((root / "usr" / "lib").is_symbolic_link())
    {
        libdir = (root / "usr" / "lib").readlink();
        if (std::string::npos != libdir.find_first_of("./"))
            libdir = "lib";
    }

    Log::get_instance()->message("unpackaged.libdir", ll_debug, lc_context) << "Using '" << libdir << "' for libdir";

    std::tr1::shared_ptr<const ChoiceValue> strip_choice(choices_key()->value()->find_by_name_with_prefix(
                ELikeStripChoiceValue::canonical_name_with_prefix()));
    std::tr1::shared_ptr<const ChoiceValue> split_choice(choices_key()->value()->find_by_name_with_prefix(
                ELikeSplitChoiceValue::canonical_name_with_prefix()));
    std::tr1::shared_ptr<const ChoiceValue> preserve_work_choice(choices_key()->value()->find_by_name_with_prefix(
                ELikePreserveWorkChoiceValue::canonical_name_with_prefix()));

    std::string used_config_protect;

    switch (install_action->options.want_phase()("strip"))
    {
        case wp_yes:
            {
                UnpackagedStripper stripper(make_named_values<UnpackagedStripperOptions>(
                            value_for<n::debug_dir>(fs_location_key()->value() / "usr" / libdir / "debug"),
                            value_for<n::image_dir>(fs_location_key()->value()),
                            value_for<n::output_manager>(output_manager),
                            value_for<n::package_id>(shared_from_this()),
                            value_for<n::split>(split_choice && split_choice->enabled()),
                            value_for<n::strip>(strip_choice && strip_choice->enabled())
                            ));

                stripper.strip();
            }
            break;

        case wp_skip:
            break;

        case wp_abort:
            throw ActionAbortedError("Told to abort install");

        case last_wp:
            throw InternalError(PALUDIS_HERE, "bad WantPhase");
    }

    switch (install_action->options.want_phase()("merge"))
    {
        case wp_yes:
            {
                MergerOptions extra_merger_options;
                if (preserve_work_choice && preserve_work_choice->enabled())
                    extra_merger_options += mo_nondestructive;
                (*install_action->options.destination()).destination_interface()->merge(
                        make_named_values<MergeParams>(
                            value_for<n::build_start_time>(build_start_time),
                            value_for<n::environment_file>(FSEntry("/dev/null")),
                            value_for<n::image_dir>(fs_location_key()->value()),
                            value_for<n::merged_entries>(make_shared_ptr(new FSEntrySet)),
                            value_for<n::options>((MergerOptions() + mo_rewrite_symlinks + mo_allow_empty_dirs)
                                | extra_merger_options),
                            value_for<n::output_manager>(output_manager),
                            value_for<n::package_id>(shared_from_this()),
                            value_for<n::perform_uninstall>(install_action->options.perform_uninstall()),
                            value_for<n::used_this_for_config_protect>(std::tr1::bind(
                                    &used_this_for_config_protect, std::tr1::ref(used_config_protect), std::tr1::placeholders::_1))
                            ));
            }
            break;

        case wp_skip:
            break;

        case wp_abort:
            throw ActionAbortedError("Told to abort install");

        case last_wp:
            throw InternalError(PALUDIS_HERE, "bad WantPhase");
    }

    for (PackageIDSequence::ConstIterator i(install_action->options.replacing()->begin()), i_end(install_action->options.replacing()->end()) ;
            i != i_end ; ++i)
    {
        Context local_context("When cleaning '" + stringify(**i) + "':");
        if ((*i)->name() == name() && (*i)->version() == version() && slot_is_same(*i, this))
            continue;

        UninstallActionOptions uo(make_named_values<UninstallActionOptions>(
                    value_for<n::config_protect>(used_config_protect),
                    value_for<n::if_for_install_id>(shared_from_this()),
                    value_for<n::ignore_for_unmerge>(&ignore_nothing),
                    value_for<n::is_overwrite>(false),
                    value_for<n::make_output_manager>(std::tr1::bind(&this_output_manager, output_manager, std::tr1::placeholders::_1))
                    ));
        install_action->options.perform_uninstall()(*i, uo);
    }

    output_manager->succeeded();
}

void
UnpackagedID::invalidate_masks() const
{
}

std::tr1::shared_ptr<const Set<std::string> >
UnpackagedID::breaks_portage() const
{
    std::tr1::shared_ptr<Set<std::string> > why(new Set<std::string>);
    why->insert("format");
    return why;
}

bool
UnpackagedID::arbitrary_less_than_comparison(const PackageID & other) const
{
    return slot_key()->value().data() < (other.slot_key() ? stringify(other.slot_key()->value()) : "");
}

std::size_t
UnpackagedID::extra_hash_value() const
{
    return Hash<SlotName>()(slot_key()->value());
}

const std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const Choices> > >
UnpackagedID::choices_key() const
{
    return _imp->choices_key;
}

