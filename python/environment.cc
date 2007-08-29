/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Piotr Jaroszyński <peper@gentoo.org>
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
#include <paludis/environments/environment_maker.hh>
#include <paludis/environments/adapted/adapted_environment.hh>
#include <paludis/environments/paludis/paludis_environment.hh>
#include <paludis/environments/paludis/paludis_config.hh>
#include <paludis/environments/no_config/no_config_environment.hh>
#include <paludis/environments/test/test_environment.hh>
#include <paludis/hook.hh>
#include <paludis/package_id.hh>

using namespace paludis;
using namespace paludis::python;
namespace bp = boost::python;

class EnvironmentImplementationWrapper :
    public EnvironmentImplementation,
    public bp::wrapper<EnvironmentImplementation>
{
    private:
        tr1::shared_ptr<PackageDatabase> _db;

    protected:
        virtual tr1::shared_ptr<SetSpecTree::ConstItem> local_set(const SetName & s) const
            PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            return tr1::shared_ptr<SetSpecTree::ConstItem>();
        }

    public:
        EnvironmentImplementationWrapper() :
            _db(new PackageDatabase(this))
        {
        }

        virtual bool query_use(const UseFlagName & u, const PackageID & p) const
            PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            Lock l(get_mutex());

            if (bp::override f = get_override("query_use"))
                return f(boost::cref(u), boost::cref(p));
            return EnvironmentImplementation::query_use(u, p);
        }

        bool default_query_use(const UseFlagName & u, const PackageID & p) const
            PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            return EnvironmentImplementation::query_use(u, p);
        }

        virtual tr1::shared_ptr<const UseFlagNameSet> known_use_expand_names(
                const UseFlagName & u, const PackageID & p) const
            PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            Lock l(get_mutex());

            if (bp::override f = get_override("known_use_expand_names"))
                return f(boost::cref(u), boost::cref(p));
            else
                throw PythonMethodNotImplemented("EnvironmentImplementation", "known_use_expand_names");
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

        virtual bool accept_keywords(tr1::shared_ptr<const KeywordNameSet> k, const PackageID & p) const
            PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            Lock l(get_mutex());

            if (bp::override f = get_override("accept_keywords"))
                return f(k, boost::cref(p));
            else
                throw PythonMethodNotImplemented("EnvironmentImplementation", "accept_keywords");
        }

        virtual const tr1::shared_ptr<const Mask> mask_for_breakage(const PackageID & p) const
            PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            Lock l(get_mutex());

            if (bp::override f = get_override("mask_for_breakage"))
                return f(boost::cref(p));
            else
                throw PythonMethodNotImplemented("EnvironmentImplementation", "mask_for_breakage");
        }

        virtual const tr1::shared_ptr<const Mask> mask_for_user(const PackageID & p) const
            PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            Lock l(get_mutex());

            if (bp::override f = get_override("mask_for_user"))
                return f(boost::cref(p));
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

        virtual tr1::shared_ptr<PackageDatabase> package_database()
            PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            return _db;
        }

        virtual tr1::shared_ptr<const PackageDatabase> package_database() const
            PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            return _db;
        }

        virtual tr1::shared_ptr<const FSEntrySequence> bashrc_files() const
            PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            Lock l(get_mutex());

            if (bp::override f = get_override("bashrc_files"))
                return f();
            return EnvironmentImplementation::bashrc_files();
        }

        tr1::shared_ptr<const FSEntrySequence> default_bashrc_files() const
            PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            return EnvironmentImplementation::bashrc_files();
        }

        virtual tr1::shared_ptr<const FSEntrySequence> syncers_dirs() const
            PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            Lock l(get_mutex());

            if (bp::override f = get_override("syncers_dirs"))
                return f();
            return EnvironmentImplementation::syncers_dirs();
        }

        tr1::shared_ptr<const FSEntrySequence> default_syncers_dirs() const
            PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            return EnvironmentImplementation::syncers_dirs();
        }

        virtual tr1::shared_ptr<const FSEntrySequence> fetchers_dirs() const
            PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            Lock l(get_mutex());

            if (bp::override f = get_override("fetchers_dirs"))
                return f();
            return EnvironmentImplementation::fetchers_dirs();
        }

        tr1::shared_ptr<const FSEntrySequence> default_fetchers_dirs() const
            PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            return EnvironmentImplementation::fetchers_dirs();
        }

        virtual tr1::shared_ptr<const FSEntrySequence> hook_dirs() const
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


        virtual tr1::shared_ptr<const MirrorsSequence> mirrors(const std::string & s) const
            PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            Lock l(get_mutex());

            if (bp::override f = get_override("mirrors"))
                return f(s);
            else
                throw PythonMethodNotImplemented("EnvironmentImplementation", "mirrors");
        }

        virtual tr1::shared_ptr<const SetNameSet> set_names() const
            PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            Lock l(get_mutex());

            if (bp::override f = get_override("set_names"))
                return f();
            return EnvironmentImplementation::set_names();
        }

        tr1::shared_ptr<const SetNameSet> default_set_names() const
            PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            return EnvironmentImplementation::set_names();
        }

        virtual tr1::shared_ptr<SetSpecTree::ConstItem> set(const SetName & s) const
            PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            Lock l(get_mutex());

            if (bp::override f = get_override("set"))
                return f(boost::cref(s));
            return EnvironmentImplementation::set(s);
        }

        tr1::shared_ptr<SetSpecTree::ConstItem> default_set(const SetName & s) const
            PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            return EnvironmentImplementation::set(s);
        }

        virtual tr1::shared_ptr<const DestinationsSet> default_destinations() const
            PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            Lock l(get_mutex());

            if (bp::override f = get_override("default_destinations"))
                return f();
            return EnvironmentImplementation::default_destinations();
        }

        tr1::shared_ptr<const DestinationsSet> default_default_destinations() const
            PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            return EnvironmentImplementation::default_destinations();
        }

        // FIXME - Hooks are not exposed
        virtual HookResult perform_hook(const Hook & h) const
            PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            return HookResult(0, "");
        }

        virtual std::string default_distribution() const
            PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            Lock l(get_mutex());

            if (bp::override f = get_override("default_distribution"))
                return f();
            return EnvironmentImplementation::default_distribution();
        }

        std::string default_default_distribution() const
            PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            return EnvironmentImplementation::default_distribution();
        }

};

struct NoConfigEnvironmentWrapper :
    NoConfigEnvironment
{
    NoConfigEnvironmentWrapper(const FSEntry & env_dir, const FSEntry & cache_dir,
            const FSEntry & master_repo_dir) :
        NoConfigEnvironment(no_config_environment::Params(env_dir, cache_dir, false, false,
                    no_config_environment::ncer_auto, master_repo_dir, tr1::shared_ptr<Map<std::string, std::string> >())
                )
    {
    }
};

void expose_environment()
{
    /**
     * Exceptions
     */
    ExceptionRegister::get_instance()->add_exception<NoSuchEnvironmentTypeError>
        ("NoSuchEnvironmentTypeError", "ConfigurationError",
         "Thrown if an environment of the specified type does not exist.");
    ExceptionRegister::get_instance()->add_exception<PaludisEnvironmentSoDirNotADirectoryError>
        ("PaludisEnvironmentSoDirNotADirectoryError", "BaseException",
         "Thrown if PALUDIS_ENVIRONMENT_SO_DIR is not a directory.");
    ExceptionRegister::get_instance()->add_exception<PaludisEnvironmentSoDirCannotDlopenError>
        ("PaludisEnvironmentSoDirCannotDlopenError", "BaseException",
         "Thrown if a repository .so cannot be used.");
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
     * EnvironmentMaker
     */
    bp::class_<EnvironmentMaker, boost::noncopyable> 
        (
         "EnvironmentMaker",
         "Virtual constructor for environments.",
         bp::no_init
        )
        .add_static_property("instance", bp::make_function(&EnvironmentMaker::get_instance,
                    bp::return_value_policy<bp::reference_existing_object>()),
                "Singleton instance."
                )

        .def("make_from_spec", &EnvironmentMaker::make_from_spec,
                "make_from_spec(spec_string) -> Environment\n"
                "Make Environment from specification."
            )
        ;

    /**
     * Environment
     */
    tr1::shared_ptr<PackageDatabase> (Environment::* package_database)() =
        &Environment::package_database;
    bp::class_<Environment, tr1::shared_ptr<Environment>, boost::noncopyable>
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

        .def("query_use", &Environment::query_use,
                "query_use(UseFlagName, PackageID) -> bool\n"
                "Is a particular use flag enabled for a particular package?"
            )

        .add_property("root", &Environment::root,
                "[ro] string\n"
                "Our root location for installs."
            )

        .add_property("set_names", &Environment::set_names,
                "[ro] SetNamesIterable\n"
                "All known named sets."
            )
        ;

    /**
     * EnvironmentImplementation
     */
    typedef EnvironmentImplementation EnvImp;
    typedef EnvironmentImplementationWrapper EnvImpW;
    bp::class_<EnvironmentImplementationWrapper, tr1::shared_ptr<EnvironmentImplementationWrapper>,
            bp::bases<Environment>, boost::noncopyable>
        (
         "EnvironmentImplementation",
         "Represents a working environment, which contains an available packages database\n"
         "and provides various methods for querying package visibility and options.\n"
         "This class can be subclassed in Python.",
         bp::init<>()
        )
        //FIXME - local_set is protected
        //.def("local_set", bp::pure_virtual(&EnvImp::local_set))

        .def("query_use", &EnvImp::query_use, &EnvImpW::default_query_use,
                "query_use(UseFlagName, PackageID) -> bool\n"
                "Is a particular use flag enabled for a particular package?"
            )

        .def("known_use_expand_names", bp::pure_virtual(&EnvImp::known_use_expand_names),
                "known_use_expand_names(UseFlagName, PackageID) -> UseFlagNameIterable\n"
                "Return a collection of known use flag names for a particular package that start\n"
                "with a particular use expand prefix.\n\n"
                "It is up to subclasses to decide whether to return all known use flags with\n"
                "the specified prefix or merely all enabled use flags. It is not safe to assume\n"
                "that all flags in the returned value will be enabled for the specified package."
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
                "mask_for_user(PackageID) -> Mask\n"
                "Do we have a 'user' mask for a particular package?\n\n"
                "Returns None if no."
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

        .def("default_distribution", &EnvImp::default_distribution, &EnvImpW::default_default_distribution,
                "default_distribution() -> str\n"
                "NEED_DOC"
            )
        ;

    /**
     * AdaptedEnvironment
     */
    bp::class_<AdaptedEnvironment, bp::bases<Environment>, boost::noncopyable>
        (
         "AdaptedEnvironment",
         "An Environment that allows you to change aspects of an existing Environment,"
         " e.g. the state of a USE flag for a package.",
         bp::init<tr1::shared_ptr<Environment> >("__init__(Environment)")
        )
        .def("adapt_use", &AdaptedEnvironment::adapt_use,
                "adapt_use(PackageDepSpeec, UseFlagName, UseFlagState)\n"
                "Set the state of a USE flag for the given PackageDepSpec."
            )

        .def("clear_adaptions", &AdaptedEnvironment::clear_adaptions,
                "clear_adaptions()\n"
                "Clear all adaptions from this Environemnt."
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
    tr1::shared_ptr<Repository> (NoConfigEnvironment::*main_repository)()
        = &NoConfigEnvironment::main_repository;
    tr1::shared_ptr<Repository> (NoConfigEnvironment::*master_repository)()
        = &NoConfigEnvironment::master_repository;
    bp::class_<NoConfigEnvironmentWrapper, bp::bases<Environment>, boost::noncopyable>
        (
         "NoConfigEnvironment",
         "An environment that uses a single repository, with no user configuration.",
         bp::init<const FSEntry &, const FSEntry &, const FSEntry &>(
             (bp::arg("environment_dir"), bp::arg("write_cache_dir")="/var/empty",
              bp::arg("master_repository_dir")="/var/empty"),
             "__init__(environment_dir, write_cache_dir=\"/var/empty\", "
             "master_repository_dir=\"/var/empty\")"
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
