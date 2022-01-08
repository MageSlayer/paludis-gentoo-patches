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

#include <paludis/environments/paludis/paludis_environment.hh>
#include <paludis/environments/paludis/paludis_config.hh>

#include <paludis/environments/test/test_environment.hh>

#include <paludis/hook.hh>
#include <paludis/package_id.hh>
#include <paludis/selection.hh>
#include <paludis/spec_tree.hh>
#include <paludis/filter.hh>
#include <paludis/standard_output_manager.hh>
#include <paludis/environment.hh>
#include <paludis/environment_factory.hh>
#include <paludis/mask.hh>
#include <paludis/repository.hh>

#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/tribool.hh>
#include <paludis/util/make_named_values.hh>

using namespace paludis;
using namespace paludis::python;
namespace bp = boost::python;

class EnvironmentImplementationWrapper :
    public EnvironmentImplementation,
    public bp::wrapper<EnvironmentImplementation>
{
    public:
        EnvironmentImplementationWrapper()
        {
        }

        void populate_sets() const override
        {
            std::unique_lock<std::recursive_mutex> l(get_mutex());

            if (bp::override f = get_override("populate_sets"))
                f();
            else
                throw PythonMethodNotImplemented("EnvironmentImplementation", "populate_sets");
        }

        bool accept_license(const std::string & s, const std::shared_ptr<const PackageID> & p) const
            override PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            std::unique_lock<std::recursive_mutex> l(get_mutex());

            if (bp::override f = get_override("accept_license"))
                return f(s, p);
            else
                throw PythonMethodNotImplemented("EnvironmentImplementation", "accept_license");
        }

        bool accept_keywords(const std::shared_ptr<const KeywordNameSet> & k, const std::shared_ptr<const PackageID> & p) const
            override PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            std::unique_lock<std::recursive_mutex> l(get_mutex());

            if (bp::override f = get_override("accept_keywords"))
                return f(k, p);
            else
                throw PythonMethodNotImplemented("EnvironmentImplementation", "accept_keywords");
        }

        const std::shared_ptr<const Mask> mask_for_user(const std::shared_ptr<const PackageID> & p, const bool b) const
            override PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            std::unique_lock<std::recursive_mutex> l(get_mutex());

            if (bp::override f = get_override("mask_for_user"))
                return f(p, b);
            else
                throw PythonMethodNotImplemented("EnvironmentImplementation", "mask_for_user");
        }

        bool unmasked_by_user(const std::shared_ptr<const PackageID> & p, const std::string & s) const
            override PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            std::unique_lock<std::recursive_mutex> l(get_mutex());

            if (bp::override f = get_override("unmasked_by_user"))
                return f(p, s);
            else
                throw PythonMethodNotImplemented("EnvironmentImplementation", "unmasked_by_user");
        }

        std::shared_ptr<const FSPathSequence> bashrc_files() const
            override PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            std::unique_lock<std::recursive_mutex> l(get_mutex());

            if (bp::override f = get_override("bashrc_files"))
                return f();
            return EnvironmentImplementation::bashrc_files();
        }

        std::shared_ptr<const FSPathSequence> default_bashrc_files() const
            PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            return EnvironmentImplementation::bashrc_files();
        }

        std::shared_ptr<const FSPathSequence> syncers_dirs() const
            override PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            std::unique_lock<std::recursive_mutex> l(get_mutex());

            if (bp::override f = get_override("syncers_dirs"))
                return f();
            return EnvironmentImplementation::syncers_dirs();
        }

        std::shared_ptr<const FSPathSequence> default_syncers_dirs() const
            PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            return EnvironmentImplementation::syncers_dirs();
        }

        std::shared_ptr<const FSPathSequence> fetchers_dirs() const
            override PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            std::unique_lock<std::recursive_mutex> l(get_mutex());

            if (bp::override f = get_override("fetchers_dirs"))
                return f();
            return EnvironmentImplementation::fetchers_dirs();
        }

        std::shared_ptr<const FSPathSequence> default_fetchers_dirs() const
            PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            return EnvironmentImplementation::fetchers_dirs();
        }

        std::shared_ptr<const FSPathSequence> hook_dirs() const
            override PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            std::unique_lock<std::recursive_mutex> l(get_mutex());

            if (bp::override f = get_override("hook_dirs"))
                return f();
            else
                throw PythonMethodNotImplemented("EnvironmentImplementation", "hook_dirs");
        }

        std::string reduced_username() const override
        {
            std::unique_lock<std::recursive_mutex> l(get_mutex());

            if (bp::override f = get_override("reduced_username"))
                return f();
            else
                throw PythonMethodNotImplemented("EnvironmentImplementation", "reduced_username");
        }

        uid_t reduced_uid() const override
        {
            std::unique_lock<std::recursive_mutex> l(get_mutex());

            if (bp::override f = get_override("reduced_uid"))
                return f();
            else
                throw PythonMethodNotImplemented("EnvironmentImplementation", "reduced_uid");
        }

        gid_t reduced_gid() const override
        {
            std::unique_lock<std::recursive_mutex> l(get_mutex());

            if (bp::override f = get_override("reduced_gid"))
                return f();
            else
                throw PythonMethodNotImplemented("EnvironmentImplementation", "reduced_gid");
        }


        std::shared_ptr<const MirrorsSequence> mirrors(const std::string & s) const
            override PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            std::unique_lock<std::recursive_mutex> l(get_mutex());

            if (bp::override f = get_override("mirrors"))
                return f(s);
            else
                throw PythonMethodNotImplemented("EnvironmentImplementation", "mirrors");
        }

        std::shared_ptr<const SetNameSet> set_names() const
            override PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            std::unique_lock<std::recursive_mutex> l(get_mutex());

            if (bp::override f = get_override("set_names"))
                return f();
            return EnvironmentImplementation::set_names();
        }

        std::shared_ptr<const SetNameSet> default_set_names() const
            PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            return EnvironmentImplementation::set_names();
        }

        const std::shared_ptr<const SetSpecTree> set(const SetName & s) const
            override PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            std::unique_lock<std::recursive_mutex> l(get_mutex());

            if (bp::override f = get_override("set"))
                return f(boost::cref(s));
            return EnvironmentImplementation::set(s);
        }

        const std::shared_ptr<const SetSpecTree> default_set(const SetName & s) const
            PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            return EnvironmentImplementation::set(s);
        }

        // FIXME - Hooks are not exposed
        HookResult perform_hook(const Hook &, const std::shared_ptr<OutputManager> &) const
            override PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            return make_named_values<HookResult>(n::max_exit_status() = 0, n::output() = "");
        }

        std::string distribution() const
            override PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            std::unique_lock<std::recursive_mutex> l(get_mutex());

            if (bp::override f = get_override("distribution"))
                return f();
            return EnvironmentImplementation::distribution();
        }

        std::string default_distribution() const
            PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            return EnvironmentImplementation::distribution();
        }

        bool add_to_world(const QualifiedPackageName & s) const override
        {
            std::unique_lock<std::recursive_mutex> l(get_mutex());
            if (bp::override f = get_override("add_to_world"))
                return f(s);
            else
                throw PythonMethodNotImplemented("EnvironmentImplementation", "add_to_world");
        }

        bool add_to_world(const SetName & s) const override
        {
            std::unique_lock<std::recursive_mutex> l(get_mutex());
            if (bp::override f = get_override("add_to_world"))
                return f(s);
            else
                throw PythonMethodNotImplemented("EnvironmentImplementation", "add_to_world");
        }

        bool remove_from_world(const QualifiedPackageName & s) const override
        {
            std::unique_lock<std::recursive_mutex> l(get_mutex());
            if (bp::override f = get_override("remove_from_world"))
                return f(s);
            else
                throw PythonMethodNotImplemented("EnvironmentImplementation", "remove_from_world");
        }

        bool remove_from_world(const SetName & s) const override
        {
            std::unique_lock<std::recursive_mutex> l(get_mutex());
            if (bp::override f = get_override("remove_from_world"))
                return f(s);
            else
                throw PythonMethodNotImplemented("EnvironmentImplementation", "remove_from_world");
        }

        std::shared_ptr<PackageIDSequence> operator[] (const Selection & fg) const
            override PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            std::unique_lock<std::recursive_mutex> l(get_mutex());

            if (bp::override f = get_override("__getitem__"))
                return f(fg);
            return EnvironmentImplementation::operator[] (fg);
        }

        virtual std::shared_ptr<PackageIDSequence> default_operator_square_brackets(const Selection & fg) const
            PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            return EnvironmentImplementation::operator[] (fg);
        }

        void need_keys_added() const override
        {
            std::unique_lock<std::recursive_mutex> l(get_mutex());

            if (bp::override f = get_override("need_keys_added"))
                f();
            else
                throw PythonMethodNotImplemented("EnvironmentImplementation", "need_keys_added");
        }

        const std::shared_ptr<const MetadataValueKey<std::string> > format_key() const override
        {
            std::unique_lock<std::recursive_mutex> l(get_mutex());

            if (bp::override f = get_override("format_key"))
                return f();
            else
                throw PythonMethodNotImplemented("EnvironmentImplementation", "format_key");
        }

        const std::shared_ptr<const MetadataValueKey<FSPath> > config_location_key() const override
        {
            std::unique_lock<std::recursive_mutex> l(get_mutex());

            if (bp::override f = get_override("config_location_key"))
                return f();
            else
                throw PythonMethodNotImplemented("EnvironmentImplementation", "config_location_key");
        }

        const std::shared_ptr<const MetadataValueKey<FSPath> > preferred_root_key() const override
        {
            std::unique_lock<std::recursive_mutex> l(get_mutex());

            if (bp::override f = get_override("preferred_root_key"))
                return f();
            else
                throw PythonMethodNotImplemented("EnvironmentImplementation", "preferred_root_key");
        }

        const std::shared_ptr<const MetadataValueKey<FSPath> > system_root_key() const override
        {
            std::unique_lock<std::recursive_mutex> l(get_mutex());

            if (bp::override f = get_override("system_root_key"))
                return f();
            else
                throw PythonMethodNotImplemented("EnvironmentImplementation", "system_root_key");
        }

        Tribool interest_in_suggestion(
                const std::shared_ptr<const PackageID> &,
                const PackageDepSpec &) const override
        {
            throw PythonMethodNotImplemented("EnvironmentImplementation", "interest_in_suggestion");
        }

        const Tribool want_choice_enabled(
                const std::shared_ptr<const PackageID> &,
                const std::shared_ptr<const Choice> &,
                const UnprefixedChoiceName &
                ) const override
        {
            throw PythonMethodNotImplemented("EnvironmentImplementation", "want_choice_enabled");
        }

        const std::string value_for_choice_parameter(
                const std::shared_ptr<const PackageID> &,
                const std::shared_ptr<const Choice> &,
                const UnprefixedChoiceName &
                ) const override
        {
            throw PythonMethodNotImplemented("EnvironmentImplementation", "value_for_choice_parameter");
        }

        std::shared_ptr<const Set<UnprefixedChoiceName> > known_choice_value_names(
                const std::shared_ptr<const PackageID> &,
                const std::shared_ptr<const Choice> &
                ) const override
        {
            throw PythonMethodNotImplemented("EnvironmentImplementation", "known_choice_value_names");
        }

        const std::shared_ptr<OutputManager> create_output_manager(
                const CreateOutputManagerInfo &) const override
        {
            return std::make_shared<StandardOutputManager>();
        }

        const std::shared_ptr<Repository> repository_from_new_config_file(
                const FSPath &) override
        {
            throw PythonMethodNotImplemented("EnvironmentImplementation", "repository_from_new_config_file");
        }

        void update_config_files_for_package_move(
                const PackageDepSpec &, const QualifiedPackageName &) const override
        {
            throw PythonMethodNotImplemented("EnvironmentImplementation", "update_config_files_for_package_move");
        }

        QualifiedPackageName fetch_unique_qualified_package_name(
                const PackageNamePart &, const Filter & = all_filter(), const bool /*disambiguate*/ = true) const override
        {
            throw PythonMethodNotImplemented("EnvironmentImplementation", "fetch_unique_qualified_package_name");
        }
};

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(fetch_unique_qualified_package_name_overloads, fetch_unique_qualified_package_name, 1, 3)

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

    const std::shared_ptr<Repository> (Environment::* fetch_repository_ptr)(const RepositoryName &) = &Environment::fetch_repository;

    /**
     * Environment
     */
    bp::class_<Environment, std::shared_ptr<Environment>, boost::noncopyable>
        (
         "Environment",
         "Represents a working environment, which contains an available packages database\n"
         "and provides various methods for querying package visibility and options.",
         bp::no_init
        )

        .def("set", &Environment::set,
                "set(SetName) -> DepSpec\n"
                "Fetch a named set."
            )

        .add_property("set_names", &Environment::set_names,
                "[ro] SetNamesIterable\n"
                "All known named sets."
            )

        .def("__getitem__", &Environment::operator[],
                "[selection] -> list of PackageID\n"
                "Return PackageID instances matching a given selection."
            )

        .def("fetch_repository", fetch_repository_ptr, bp::with_custodian_and_ward_postcall<0, 1>(),
                "fetch_repository(RepositoryName) -> Repository\n"
                "Fetch a named repository."
            )

        .def("fetch_unique_qualified_package_name", &Environment::fetch_unique_qualified_package_name,
             fetch_unique_qualified_package_name_overloads(
                "fetch_unique_qualified_package_name(PackageNamePart[, Filter[, bool]]) -> QualifiedPackageName\n"
                "Disambiguate a package name.  If a filter is specified, "
                "limit the potential results to packages that match."
                )
            )

        .def("more_important_than", &Environment::more_important_than,
                "more_important_than(RepositoryName, RepositoryName) -> bool\n"
                "Return true if the first repository is more important than the second."
            )

        .def("reduced_username", &Environment::reduced_username,
                "reduced_username() -> str\n"
                "User name to use when reduced privs are permissible."
            )

        .def("reduced_uid", &Environment::reduced_uid,
                "reduced_uid() -> int\n"
                "User id to use when reduced privs are permissible."
            )

        .def("reduced_gid", &Environment::reduced_gid,
                "reduced_gid() -> int\n"
                "Group id to use when reduced privs are permissible."
            )

        .add_property("repositories",
                bp::range(&Environment::begin_repositories, &Environment::end_repositories),
                "[ro] Iterable of Repository\n"
                "Our repositories"
                )
        ;

    /**
     * EnvironmentImplementation
     */
    typedef EnvironmentImplementation EnvImp;
    typedef EnvironmentImplementationWrapper EnvImpW;

    bp::class_<EnvironmentImplementationWrapper, std::shared_ptr<EnvironmentImplementationWrapper>,
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

        .def("mask_for_user", bp::pure_virtual(&EnvImp::mask_for_user),
                "mask_for_user(PackageID, bool) -> Mask\n"
                "Do we have a 'user' mask for a particular package?\n\n"
                "Returns None if no. The second parameter should be true if the mask will be overridden "
                "and false otherwise."
            )

        .def("unmasked_by_user", bp::pure_virtual(&EnvImp::unmasked_by_user),
                "unmasked_by_user(PackageID, String) -> bool\n"
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

        .def("reduced_username", bp::pure_virtual(&EnvImp::reduced_username),
                "reduced_username() -> str\n"
                "User name to use when reduced privs are permissible."
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

        .def("distribution", &EnvImp::distribution, &EnvImpW::default_distribution,
                "distribution() -> str\n"
                "NEED_DOC"
            )

        .def("__getitem__", &EnvImp::operator[], &EnvImpW::default_operator_square_brackets,
                "[selection] -> list of PackageID\n"
                "Return PackageID instances matching a given selection."
            )

        .def("fetch_repository", fetch_repository_ptr, bp::with_custodian_and_ward_postcall<0, 1>(),
                "fetch_repository(RepositoryName) -> Repository\n"
                "Fetch a named repository."
            )

        .def("fetch_unique_qualified_package_name", &EnvImpW::fetch_unique_qualified_package_name,
             fetch_unique_qualified_package_name_overloads(
                "fetch_unique_qualified_package_name(PackageNamePart[, Filter[, bool]]) -> QualifiedPackageName\n"
                "Disambiguate a package name.  If a filter is specified, "
                "limit the potential results to packages that match."
                )
            )

        .def("more_important_than", &EnvImpW::more_important_than,
                "more_important_than(RepositoryName, RepositoryName) -> bool\n"
                "Return true if the first repository is more important than the second."
            )

        .add_property("repositories",
                bp::range(&EnvImpW::begin_repositories, &EnvImpW::end_repositories),
                "[ro] Iterable of Repository\n"
                "Our repositories"
                )
        ;
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
     * TestEnvironment
     */
    bp::class_<TestEnvironment, bp::bases<Environment>, boost::noncopyable>
        (
         "TestEnvironment",
         "A TestEnvironment is an environment used during testing that lets us "
         "control all the options rather than reading them from configuration files.",
         bp::init<>("__init__()")
        );

    ExceptionRegister::get_instance()->add_exception<DuplicateRepositoryError>
        ("DuplicateRepositoryError", "BaseException",
         "Thrown if a Repository with the same name as an existing member is added to an Environment.");

    ExceptionRegister::get_instance()->add_exception<AmbiguousPackageNameError>
        ("AmbiguousPackageNameError", "BaseException",
         "Thrown if an Environment query results in more than one matching Package.");

    ExceptionRegister::get_instance()->add_exception<NoSuchPackageError>
        ("NoSuchPackageError", "BaseException",
         "Thrown if there is no Package in an Environment with the given name.");

    ExceptionRegister::get_instance()->add_exception<NoSuchRepositoryError>
        ("NoSuchRepositoryError", "BaseException",
         "Thrown if there is no Repository in a RepositoryDatabase with the given name.");
}

