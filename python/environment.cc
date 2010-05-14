/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Piotr Jaroszyński
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

#include <python/paludis_python.hh>
#include <python/exception.hh>
#include <python/iterable.hh>

#include <paludis/environment.hh>
#include <paludis/environment_factory.hh>
#include <paludis/environments/paludis/paludis_environment.hh>
#include <paludis/environments/paludis/paludis_config.hh>
#include <paludis/environments/no_config/no_config_environment.hh>
#include <paludis/environments/test/test_environment.hh>
#include <paludis/hook.hh>
#include <paludis/package_id.hh>
#include <paludis/selection.hh>
#include <paludis/spec_tree.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/tribool.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/standard_output_manager.hh>

using namespace paludis;
using namespace paludis::python;
namespace bp = boost::python;

class EnvironmentImplementationWrapper :
    public EnvironmentImplementation,
    public bp::wrapper<EnvironmentImplementation>
{
    private:
        std::tr1::shared_ptr<PackageDatabase> _db;

    public:
        EnvironmentImplementationWrapper() :
            _db(new PackageDatabase(this))
        {
        }

        virtual void populate_sets() const
        {
            Lock l(get_mutex());

            if (bp::override f = get_override("populate_sets"))
                f();
            else
                throw PythonMethodNotImplemented("EnvironmentImplementation", "populate_sets");
        }

        virtual bool accept_license(const std::string & s, const PackageID & p) const
            PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            Lock l(get_mutex());

            if (bp::override f = get_override("accept_license"))
                return f(s, boost::cref(p));
            else
                throw PythonMethodNotImplemented("EnvironmentImplementation", "accept_license");
        }

        virtual bool accept_keywords(const std::tr1::shared_ptr<const KeywordNameSet> & k, const PackageID & p) const
            PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            Lock l(get_mutex());

            if (bp::override f = get_override("accept_keywords"))
                return f(k, boost::cref(p));
            else
                throw PythonMethodNotImplemented("EnvironmentImplementation", "accept_keywords");
        }

        virtual const std::tr1::shared_ptr<const Mask> mask_for_breakage(const PackageID & p) const
            PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            Lock l(get_mutex());

            if (bp::override f = get_override("mask_for_breakage"))
                return f(boost::cref(p));
            else
                throw PythonMethodNotImplemented("EnvironmentImplementation", "mask_for_breakage");
        }

        virtual const std::tr1::shared_ptr<const Mask> mask_for_user(const PackageID & p, const bool b) const
            PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            Lock l(get_mutex());

            if (bp::override f = get_override("mask_for_user"))
                return f(boost::cref(p), b);
            else
                throw PythonMethodNotImplemented("EnvironmentImplementation", "mask_for_user");
        }

        virtual bool unmasked_by_user(const PackageID & p) const
            PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            Lock l(get_mutex());

            if (bp::override f = get_override("unmasked_by_user"))
                return f(boost::cref(p));
            else
                throw PythonMethodNotImplemented("EnvironmentImplementation", "unmasked_by_user");
        }

        virtual std::tr1::shared_ptr<PackageDatabase> package_database()
            PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            return _db;
        }

        virtual std::tr1::shared_ptr<const PackageDatabase> package_database() const
            PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            return _db;
        }

        virtual std::tr1::shared_ptr<const FSEntrySequence> bashrc_files() const
            PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            Lock l(get_mutex());

            if (bp::override f = get_override("bashrc_files"))
                return f();
            return EnvironmentImplementation::bashrc_files();
        }

        std::tr1::shared_ptr<const FSEntrySequence> default_bashrc_files() const
            PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            return EnvironmentImplementation::bashrc_files();
        }

        virtual std::tr1::shared_ptr<const FSEntrySequence> syncers_dirs() const
            PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            Lock l(get_mutex());

            if (bp::override f = get_override("syncers_dirs"))
                return f();
            return EnvironmentImplementation::syncers_dirs();
        }

        std::tr1::shared_ptr<const FSEntrySequence> default_syncers_dirs() const
            PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            return EnvironmentImplementation::syncers_dirs();
        }

        virtual std::tr1::shared_ptr<const FSEntrySequence> fetchers_dirs() const
            PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            Lock l(get_mutex());

            if (bp::override f = get_override("fetchers_dirs"))
                return f();
            return EnvironmentImplementation::fetchers_dirs();
        }

        std::tr1::shared_ptr<const FSEntrySequence> default_fetchers_dirs() const
            PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            return EnvironmentImplementation::fetchers_dirs();
        }

        virtual std::tr1::shared_ptr<const FSEntrySequence> hook_dirs() const
            PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            Lock l(get_mutex());

            if (bp::override f = get_override("hook_dirs"))
                return f();
            else
                throw PythonMethodNotImplemented("EnvironmentImplementation", "hook_dirs");
        }

        virtual std::string paludis_command() const
            PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            Lock l(get_mutex());

            if (bp::override f = get_override("paludis_command"))
                return f();
            else
                throw PythonMethodNotImplemented("EnvironmentImplementation", "paludis_command");
        }

        virtual void set_paludis_command(const std::string & s)
        {
            Lock l(get_mutex());

            if (bp::override f = get_override("set_paludis_command"))
                f(s);
            else
                throw PythonMethodNotImplemented("EnvironmentImplementation", "set_paludis_command");
        }

        virtual const FSEntry root() const
        {
            Lock l(get_mutex());

            if (bp::override f = get_override("root"))
                return f();
            else
                throw PythonMethodNotImplemented("EnvironmentImplementation", "root");
        }

        virtual uid_t reduced_uid() const
        {
            Lock l(get_mutex());

            if (bp::override f = get_override("reduced_uid"))
                return f();
            else
                throw PythonMethodNotImplemented("EnvironmentImplementation", "reduced_uid");
        }

        virtual gid_t reduced_gid() const
        {
            Lock l(get_mutex());

            if (bp::override f = get_override("reduced_gid"))
                return f();
            else
                throw PythonMethodNotImplemented("EnvironmentImplementation", "reduced_gid");
        }


        virtual std::tr1::shared_ptr<const MirrorsSequence> mirrors(const std::string & s) const
            PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            Lock l(get_mutex());

            if (bp::override f = get_override("mirrors"))
                return f(s);
            else
                throw PythonMethodNotImplemented("EnvironmentImplementation", "mirrors");
        }

        virtual std::tr1::shared_ptr<const SetNameSet> set_names() const
            PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            Lock l(get_mutex());

            if (bp::override f = get_override("set_names"))
                return f();
            return EnvironmentImplementation::set_names();
        }

        std::tr1::shared_ptr<const SetNameSet> default_set_names() const
            PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            return EnvironmentImplementation::set_names();
        }

        virtual const std::tr1::shared_ptr<const SetSpecTree> set(const SetName & s) const
            PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            Lock l(get_mutex());

            if (bp::override f = get_override("set"))
                return f(boost::cref(s));
            return EnvironmentImplementation::set(s);
        }

        const std::tr1::shared_ptr<const SetSpecTree> default_set(const SetName & s) const
            PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            return EnvironmentImplementation::set(s);
        }

        virtual std::tr1::shared_ptr<const DestinationsSet> default_destinations() const
            PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            Lock l(get_mutex());

            if (bp::override f = get_override("default_destinations"))
                return f();
            return EnvironmentImplementation::default_destinations();
        }

        std::tr1::shared_ptr<const DestinationsSet> default_default_destinations() const
            PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            return EnvironmentImplementation::default_destinations();
        }

        // FIXME - Hooks are not exposed
        virtual HookResult perform_hook(const Hook & h) const
            PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            return make_named_values<HookResult>(value_for<n::max_exit_status>(0), value_for<n::output>(""));
        }

        virtual std::string distribution() const
            PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            Lock l(get_mutex());

            if (bp::override f = get_override("distribution"))
                return f();
            return EnvironmentImplementation::distribution();
        }

        std::string default_distribution() const
            PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            return EnvironmentImplementation::distribution();
        }

        virtual void add_to_world(const QualifiedPackageName & s) const
        {
            Lock l(get_mutex());
            if (bp::override f = get_override("add_to_world"))
                f(s);
            else
                throw PythonMethodNotImplemented("EnvironmentImplementation", "add_to_world");
        }

        virtual void add_to_world(const SetName & s) const
        {
            Lock l(get_mutex());
            if (bp::override f = get_override("add_to_world"))
                f(s);
            else
                throw PythonMethodNotImplemented("EnvironmentImplementation", "add_to_world");
        }

        virtual void remove_from_world(const QualifiedPackageName & s) const
        {
            Lock l(get_mutex());
            if (bp::override f = get_override("remove_from_world"))
                f(s);
            else
                throw PythonMethodNotImplemented("EnvironmentImplementation", "remove_from_world");
        }

        virtual void remove_from_world(const SetName & s) const
        {
            Lock l(get_mutex());
            if (bp::override f = get_override("remove_from_world"))
                f(s);
            else
                throw PythonMethodNotImplemented("EnvironmentImplementation", "remove_from_world");
        }

        virtual std::tr1::shared_ptr<PackageIDSequence> operator[] (const Selection & fg) const
            PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            Lock l(get_mutex());

            if (bp::override f = get_override("__getitem__"))
                return f(fg);
            return EnvironmentImplementation::operator[] (fg);
        }

        virtual std::tr1::shared_ptr<PackageIDSequence> default_operator_square_brackets(const Selection & fg) const
            PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            return EnvironmentImplementation::operator[] (fg);
        }

        virtual void need_keys_added() const
        {
            Lock l(get_mutex());

            if (bp::override f = get_override("need_keys_added"))
                f();
            else
                throw PythonMethodNotImplemented("EnvironmentImplementation", "need_keys_added");
        }

        virtual const std::tr1::shared_ptr<const MetadataValueKey<std::string> > format_key() const
        {
            Lock l(get_mutex());

            if (bp::override f = get_override("format_key"))
                return f();
            else
                throw PythonMethodNotImplemented("EnvironmentImplementation", "format_key");
        }

        virtual const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> > config_location_key() const
        {
            Lock l(get_mutex());

            if (bp::override f = get_override("config_location_key"))
                return f();
            else
                throw PythonMethodNotImplemented("EnvironmentImplementation", "config_location_key");
        }

        virtual const Tribool want_choice_enabled(
                const std::tr1::shared_ptr<const PackageID> &,
                const std::tr1::shared_ptr<const Choice> &,
                const UnprefixedChoiceName &
                ) const
        {
            throw PythonMethodNotImplemented("EnvironmentImplementation", "want_choice_enabled");
        }

        virtual const std::string value_for_choice_parameter(
                const std::tr1::shared_ptr<const PackageID> &,
                const std::tr1::shared_ptr<const Choice> &,
                const UnprefixedChoiceName &
                ) const
        {
            throw PythonMethodNotImplemented("EnvironmentImplementation", "value_for_choice_parameter");
        }

        virtual std::tr1::shared_ptr<const Set<UnprefixedChoiceName> > known_choice_value_names(
                const std::tr1::shared_ptr<const PackageID> &,
                const std::tr1::shared_ptr<const Choice> &
                ) const
        {
            throw PythonMethodNotImplemented("EnvironmentImplementation", "known_choice_value_names");
        }

        virtual const std::tr1::shared_ptr<OutputManager> create_output_manager(
                const CreateOutputManagerInfo &) const
        {
            return make_shared_ptr(new StandardOutputManager);
        }

        virtual const std::tr1::shared_ptr<Repository> repository_from_new_config_file(
                const FSEntry &)
        {
            throw PythonMethodNotImplemented("EnvironmentImplementation", "repository_from_new_config_file");
        }
};

struct NoConfigEnvironmentWrapper :
    NoConfigEnvironment
{
    NoConfigEnvironmentWrapper(const FSEntry & env_dir, const FSEntry & cache_dir,
            const std::string & master_repo_name, const std::tr1::shared_ptr<const FSEntrySequence> & extra_repository_dirs
            ) :
        NoConfigEnvironment(make_named_values<no_config_environment::Params>(
                    value_for<n::accept_unstable>(false),
                    value_for<n::disable_metadata_cache>(false),
                    value_for<n::extra_accept_keywords>(""),
                    value_for<n::extra_params>(make_null_shared_ptr()),
                    value_for<n::extra_repository_dirs>(extra_repository_dirs),
                    value_for<n::master_repository_name>(master_repo_name),
                    value_for<n::profiles_if_not_auto>(""),
                    value_for<n::repository_dir>(env_dir),
                    value_for<n::repository_type>(no_config_environment::ncer_auto),
                    value_for<n::write_cache>(cache_dir)
                ))
    {
    }
};

void expose_environment()
{
    /**
     * Exceptions
     */
    ExceptionRegister::get_instance()->add_exception<paludis_environment::PaludisConfigError>
        ("PaludisConfigError", "ConfigurationError",
         "Thrown if a configuration error is encountered by PaludisConfig.");
    ExceptionRegister::get_instance()->add_exception<paludis_environment::PaludisConfigNoDirectoryError>
        ("PaludisConfigNoDirectoryError", "PaludisConfigError",
         "Thrown if the config directory cannot be found by PaludisConfig.");

    /**
     * StringIterable
     */
    class_iterable<Sequence<std::string> >
        (
         "StringIterable",
         "Iterable of string",
         true
        );

    /**
     * EnvironmentFactory
     */
    bp::class_<EnvironmentFactory, boost::noncopyable>
        (
         "EnvironmentFactory",
         "Virtual constructor for environments.",
         bp::no_init
        )
        .add_static_property("instance", bp::make_function(&EnvironmentFactory::get_instance,
                    bp::return_value_policy<bp::reference_existing_object>()),
                "Singleton instance."
                )

        .def("create", &EnvironmentFactory::create,
                "create(spec_string) -> Environment\n"
                "Make Environment from specification."
            )
        ;

    /**
     * Environment
     */
    std::tr1::shared_ptr<PackageDatabase> (Environment::* package_database)() =
        &Environment::package_database;
    bp::class_<Environment, std::tr1::shared_ptr<Environment>, boost::noncopyable>
        (
         "Environment",
         "Represents a working environment, which contains an available packages database\n"
         "and provides various methods for querying package visibility and options.",
         bp::no_init
        )
        .add_property("default_destinations", &Environment::default_destinations,
                "[ro] DestinationsIterable\n"
                "Default destination candidates for installing packages."
            )

        .add_property("package_database", bp::make_function(package_database,
                    bp::with_custodian_and_ward_postcall<0, 1>()),
                "[ro] PackageDatabase\n"
                "Our package databas."
                )

        .def("set", &Environment::set,
                "set(SetName) -> DepSpec\n"
                "Fetch a named set."
            )

        .add_property("root", &Environment::root,
                "[ro] string\n"
                "Our root location for installs."
            )

        .add_property("set_names", &Environment::set_names,
                "[ro] SetNamesIterable\n"
                "All known named sets."
            )

        .def("__getitem__", &Environment::operator[],
                "[selection] -> list of PackageID\n"
                "Return PackageID instances matching a given selection."
            )
        ;

    /**
     * EnvironmentImplementation
     */
    typedef EnvironmentImplementation EnvImp;
    typedef EnvironmentImplementationWrapper EnvImpW;
    bp::class_<EnvironmentImplementationWrapper, std::tr1::shared_ptr<EnvironmentImplementationWrapper>,
            bp::bases<Environment>, boost::noncopyable>
        (
         "EnvironmentImplementation",
         "Represents a working environment, which contains an available packages database\n"
         "and provides various methods for querying package visibility and options.\n"
         "This class can be subclassed in Python.",
         bp::init<>()
        )

        .def("accept_license", bp::pure_virtual(&EnvImp::accept_license),
                "accept_license(str, PackageID) -> bool\n"
                "Do we accept a particular license for a particular package?"
            )

        .def("accept_keywords", bp::pure_virtual(&EnvImp::accept_keywords),
                "accept_keywords(KeywordsNameIterable, PackageID)\n"
                "Do we accept any of the specified keywords for a particular package?\n\n"
                "If the collection includes \"*\", should return true."
            )

        .def("mask_for_breakage", bp::pure_virtual(&EnvImp::mask_for_breakage),
                "mask_for_breakage(PackageID) -> Mask\n"
                "Do we have a 'breaks' mask for a particular package?\n\n"
                "Returns None if no."
            )

        .def("mask_for_user", bp::pure_virtual(&EnvImp::mask_for_user),
                "mask_for_user(PackageID, bool) -> Mask\n"
                "Do we have a 'user' mask for a particular package?\n\n"
                "Returns None if no. The second parameter should be true if the mask will be overridden "
                "and false otherwise."
            )

        .def("unmasked_by_user", bp::pure_virtual(&EnvImp::unmasked_by_user),
                "unmasked_by_user(PackageID) -> bool\n"
                "Do we have a user unmask for a particular package?\n\n"
                "This is only applied to repository and profile style masks, not\n"
                "keywords, licences etc. If true, user_mask shouldn't be used."
            )

        .def("bashrc_files", &EnvImp::bashrc_files, &EnvImpW::default_bashrc_files,
                "bashrc_files() -> list of paths\n"
                "Return a collection of bashrc files to be used by the various components\n"
                "that are implemented in bash."
            )

        .def("syncers_dirs", &EnvImp::syncers_dirs, &EnvImpW::default_syncers_dirs,
                "syncers_dirs() -> list of paths\n"
                "Return directories to search for syncer scripts."
            )

        .def("fetchers_dirs", &EnvImp::fetchers_dirs, &EnvImpW::default_fetchers_dirs,
                "fetchers_dirs() -> list of paths\n"
                "Return directories to search for fetcher scripts."
            )

        .def("hook_dirs", bp::pure_virtual(&EnvImp::hook_dirs),
                "hook_dirs() -> list of paths\n"
                "Return directories to search for hooks."
            )

        .def("paludis_command", bp::pure_virtual(&EnvImp::paludis_command),
                "paludis_command() -> str\n"
                "Return the command used to launch paludis (the client)."
            )

        .def("set_paludis_command", bp::pure_virtual(&EnvImp::set_paludis_command),
                "set_paludis_command(str)\n"
                "Change the command used to launch paludis (the client)."
            )

        .def("root", bp::pure_virtual(&EnvImp::root),
                "root() -> path\n"
                "Our root location for installs."
            )

        .def("reduced_uid", bp::pure_virtual(&EnvImp::reduced_uid),
                "reduced_uid() -> int\n"
                "User id to use when reduced privs are permissible."
            )

        .def("reduced_gid", bp::pure_virtual(&EnvImp::reduced_gid),
                "reduced_gid() -> int\n"
                "Group id to use when reduced privs are permissible."
            )

        .def("mirrors", bp::pure_virtual(&EnvImp::mirrors),
                "mirrors(str) -> list of str\n"
                "Return the mirror URI prefixes for a named mirror."
            )

        .def("set_names", &EnvImp::set_names, &EnvImpW::default_set_names,
                "set_names() -> list of SetName\n"
                "Return all known named sets."
            )

        .def("set", &EnvImp::set, &EnvImpW::default_set,
                "set(SetName) -> CompositeDepSpec\n"
                "Return a named set.\n\n"
                "If the named set is not known, returns None."
            )

        .def("default_destinations", &EnvImp::default_destinations, &EnvImpW::default_default_destinations,
                "default_destinations() -> list of Repository\n"
                "Default destination candidates for installing packages."
            )

        .def("distribution", &EnvImp::distribution, &EnvImpW::default_distribution,
                "distribution() -> str\n"
                "NEED_DOC"
            )

        .def("__getitem__", &EnvImp::operator[], &EnvImpW::default_operator_square_brackets,
                "[selection] -> list of PackageID\n"
                "Return PackageID instances matching a given selection."
            )
        ;

    /**
     * PaludisEnvironment
     */
    bp::class_<PaludisEnvironment, bp::bases<Environment>, boost::noncopyable>
        (
         "PaludisEnvironment",
         "The PaludisEnvironment is an Environment that corresponds to the normal operating evironment.",
         bp::init<const std::string &>("__init__(string)")
        )
        .add_property("config_dir", &PaludisEnvironment::config_dir,
                "[ro] string\n"
                "The config directory."
                )
        ;

    /**
     * NoConfigEnvironment
     */
    std::tr1::shared_ptr<Repository> (NoConfigEnvironment::*main_repository)()
        = &NoConfigEnvironment::main_repository;
    std::tr1::shared_ptr<Repository> (NoConfigEnvironment::*master_repository)()
        = &NoConfigEnvironment::master_repository;
    bp::class_<NoConfigEnvironmentWrapper, bp::bases<Environment>, boost::noncopyable>
        (
         "NoConfigEnvironment",
         "An environment that uses a single repository, with no user configuration.",
         bp::init<const FSEntry &, const FSEntry &, const std::string &, const std::tr1::shared_ptr<const FSEntrySequence> &>(
             (bp::arg("environment_dir"), bp::arg("write_cache_dir")="/var/empty",
              bp::arg("master_repository_name")="",
              bp::arg("extra_repository_dirs") = make_shared_ptr(new FSEntrySequence)),
             "__init__(environment_dir, write_cache_dir=\"/var/empty\", "
             "master_repository_name=\"\", extra_repository_dirs=[])"
             )
        )
        .add_property("main_repository", main_repository,
                "[ro] Repository\n"
                "Main repository."
                )

        .add_property("master_repository", master_repository,
                "[ro] Repository\n"
                "Master repository."
                )

        .add_property("accept_unstable", bp::object(), &NoConfigEnvironment::set_accept_unstable,
                "[wo] bool\n"
                "Should we accept unstable keywords?"
                )
        ;

    /**
     * TestEnvironment
     */
    bp::class_<TestEnvironment, bp::bases<Environment>, boost::noncopyable>
        (
         "TestEnvironment",
         "A TestEnvironment is an environment used during testing that lets us "
         "control all the options rather than reading them from configuration files.",
         bp::init<>("__init__()")
        );
}

