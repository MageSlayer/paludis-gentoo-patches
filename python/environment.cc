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

#include <paludis_python.hh>

#include <paludis/environment.hh>
#include <paludis/environments/environment_maker.hh>
#include <paludis/environments/paludis/paludis_environment.hh>
#include <paludis/environments/paludis/paludis_config.hh>
#include <paludis/environments/no_config/no_config_environment.hh>

using namespace paludis;
using namespace paludis::python;
namespace bp = boost::python;

struct NoConfigEnvironmentWrapper :
    NoConfigEnvironment
{
    NoConfigEnvironmentWrapper(const FSEntry & env_dir, const FSEntry & cache_dir,
            const FSEntry & master_repo_dir) :
        NoConfigEnvironment(NoConfigEnvironmentParams(env_dir, cache_dir, false,
                    ncer_auto, master_repo_dir)
                )
    {
    }
};

void PALUDIS_VISIBLE expose_environment()
{
    static register_exception<NoSuchEnvironmentTypeError>
        NoSuchEnvironmentTypeError("NoSuchEnvironmentTypeError");
    static register_exception<PaludisEnvironmentSoDirNotADirectoryError>
        PaludisEnvironmentSoDirNotADirectoryError("PaludisEnvironmentSoDirNotADirectoryError");
    static register_exception<PaludisEnvironmentSoDirCannotDlopenError>
        PaludisEnvironmentSoDirCannotDlopenError("PaludisEnvironmentSoDirCannotDlopenError");
    static register_exception<PaludisConfigError>
        PaludisConfigError("PaludisConfigError");
    static register_exception<PaludisConfigNoDirectoryError>
        PaludisConfigNoDirectoryError("PaludisConfigNoDirectoryError");

    bp::class_<EnvironmentMaker, boost::noncopyable> em("EnvironmentMaker",
            "Virtual constructor for environments.",
            bp::no_init);
    em.def("make_from_spec", &EnvironmentMaker::make_from_spec,
            "make_from_spec(spec_string) -> Environment\n"
            "Make Environment from specification."
            );
    em.add_static_property("instance", bp::make_function(&EnvironmentMaker::get_instance,
                bp::return_value_policy<bp::reference_existing_object>()),
            "Singleton instance."
          );

    enum_auto("MaskReasonsOption", last_mro);

    class_options<MaskReasonsOptions>
    mrs("MaskReasonsOptions", "MaskReasonsOption",
            "Options for Environment.mask_reasons()."
       );

    bp::class_<Environment, std::tr1::shared_ptr<Environment>, boost::noncopyable>
        e("Environment",
                "Represents a working environment, which contains an available packages database\n"
                "and provides various methods for querying package visibility and options.",
                bp::no_init
         );
    e.def("default_destinations", &Environment::default_destinations,
            "default_destinations() -> DestinationsCollection\n"
            "Default destination candidates for installing packages."
         );
    std::tr1::shared_ptr<PackageDatabase> (Environment::* package_database)() =
        &Environment::package_database;
    e.add_property("package_database", bp::make_function(package_database,
                bp::with_custodian_and_ward_postcall<0, 1>()),
            "[ro] PackageDatabase\n"
            "Our package database."
         );
    e.def("set", &Environment::set,
            "set(SetName) -> DepSpec\n"
            "Fetch a named set."
         );
    e.def("query_use", &Environment::query_use,
            "query_use(UseFlagName, PackageDatabaseEntry) -> bool\n"
            "Is a particular use flag enabled for a particular package?"
         );
    e.def("mask_reasons", &Environment::mask_reasons,
            (bp::arg("PackageDatabaseEntry"), bp::arg("MaskReasonOptions")=MaskReasonsOptions()),
            "mask_reasons(PackageDatabaseEntry, MaskReasonsOptions=MaskReasonsOptions())"
            " -> set of MaskReason\n"
            "Return the reasons for a package being masked."
         );
    e.def("root", &Environment::root,
            "root() -> string\n"
            "Our root location for installs."
         );
    e.def("set_names", &Environment::set_names,
            "set_names() -> SetNamesCollection\n"
            "Return all known named sets."
         );

    bp::class_<PaludisEnvironment, bp::bases<Environment>, boost::noncopyable>
        pe("PaludisEnvironment",
                "The PaludisEnvironment is an Environment that corresponds to the normal operating evironment.",
                bp::init<const std::string &>("__init__(string)")
          );
        pe.add_property("config_dir", &PaludisEnvironment::config_dir,
                "[ro] string\n"
                "The config directory."
              );

    bp::class_<NoConfigEnvironmentWrapper, bp::bases<Environment>, boost::noncopyable>
        nce("NoConfigEnvironment",
                "An environment that uses a single repository, with no user configuration.",
                bp::init<const FSEntry &, const FSEntry &, const FSEntry &>(
                    (bp::arg("environment_dir"), bp::arg("write_cache_dir")="/var/empty",
                     bp::arg("master_repository_dir")="/var/empty"),
                    "__init__(environment_dir, write_cache_dir=\"/var/empty\", "
                        "master_repository_dir=\"/var/empty\")"
                    )
          );
    std::tr1::shared_ptr<Repository> (NoConfigEnvironment::*main_repository)() =
        &NoConfigEnvironment::main_repository;
    nce.add_property("main_repository", main_repository,
            "[ro] Repository\n"
            "Main repository."
            );
    std::tr1::shared_ptr<Repository> (NoConfigEnvironment::*master_repository)()
        = &NoConfigEnvironment::master_repository;
    nce.add_property("master_repository", master_repository,
            "[ro] Repository\n"
            "Master repository."
            );
    nce.add_property("accept_unstable", bp::object(), &NoConfigEnvironment::set_accept_unstable,
            "[wo] bool\n"
            "Should we accept unstable keywords?"
           );
}
