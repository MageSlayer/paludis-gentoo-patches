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

#include <paludis/dep_list.hh>
#include <paludis/dep_list_exceptions.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>


using namespace paludis;
using namespace paludis::python;
namespace bp = boost::python;

struct DepListWrapper
{
    static void
    add(DepList & self, tr1::shared_ptr<SetSpecTree::ConstItem> st,
            tr1::shared_ptr<const DestinationsSet> & d)
    {
        self.add(*st, d);
    }

    static bool
    already_installed(const DepList & self, tr1::shared_ptr<DependencySpecTree::ConstItem> st,
            tr1::shared_ptr<const DestinationsSet> & d)
    {
        return self.already_installed(*st, d);
    }
};

void expose_dep_list()
{
    /**
     * Exceptions
     */
    ExceptionRegister::get_instance()->add_exception<DepListError>
        ("DepListError", "BaseException",
         "Thrown if an error occurs whilst building a DepList.");
    ExceptionRegister::get_instance()->add_exception<AllMaskedError>
        ("AllMaskedError", "DepListError",
         "Thrown if all versions of a particular spec are masked.");
    ExceptionRegister::get_instance()->add_exception<UseRequirementsNotMetError>
        ("UseRequirementsNotMetError", "DepListError",
         "Thrown if all versions of a particular spec are masked, "
         "but would not be if use requirements were not in effect.");
    ExceptionRegister::get_instance()->add_exception<DowngradeNotAllowedError>
        ("DowngradeNotAllowedError", "DepListError",
         "Thrown if a downgrade is forced and we're not allowed to downgrade.");
    ExceptionRegister::get_instance()->add_exception<BlockError>
        ("BlockError", "DepListError",
         "Thrown if a block is encountered.");
    ExceptionRegister::get_instance()->add_exception<CircularDependencyError>
        ("CircularDependencyError", "DepListError",
         "Thrown if a circular dependency is encountered.");
    ExceptionRegister::get_instance()->add_exception<NoDestinationError>
        ("NoDestinationError", "DepListError",
         "Thrown if no destination can be found.");

    /**
     * Enums
     */
    enum_auto("DepListTargetType", last_dl_target,
            "Type of target being handled at the top level.");
    enum_auto("DepListReinstallOption", last_dl_reinstall,
            "When we should reinstall.");
    enum_auto("DepListFallBackOption", last_dl_fall_back,
            "When we should fall back to an installed package.");
    enum_auto("DepListReinstallScmOption", last_dl_reinstall_scm,
            "When we should reinstall SCM packages.");
    enum_auto("DepListUpgradeOption", last_dl_upgrade,
            "When we should upgrade.");
    enum_auto("DepListDowngradeOption", last_dl_downgrade,
            "What to do when we downgrade.");
    enum_auto("DepListNewSlotsOption", last_dl_new_slots,
            "When we should pull in a new slot.");
    enum_auto("DepListDepsOption", last_dl_deps,
            "How we should handle a dep class.");
    enum_auto("DepListSuggestedOption", last_dl_suggested,
            "How we should handle suggested deps.");
    enum_auto("DepListCircularOption", last_dl_circular,
            "How we should handle circular deps.");
    enum_auto("DepListBlocksOption", last_dl_blocks,
            "How we handle blocks.");
    enum_auto("DepListUseOption", last_dl_use_deps,
            "How we handle use deps.");
    enum_auto("DepListEntryState", last_dle,
            "State of a DepListEntry.");
    enum_auto("DepListEntryKind", last_dlk,
            "Kind of a DepListEntry.");

    /**
     * DepListOptions
     */
    register_shared_ptrs_to_python<DepListOptions>(rsp_non_const);
    bp::class_<DepListOptions, boost::noncopyable>
        (
         "DepListOptions",
         "Parameters for a DepList.",
         bp::init<>("__init__()")
        )
        .def_readwrite("reinstall", &DepListOptions::reinstall,
                "[rw] DepListReinstallOption"
                )

        .def_readwrite("reinstall_scm", &DepListOptions::reinstall_scm,
                "[rw] DepListReinstallScmOption"
                )

        .def_readwrite("target_type", &DepListOptions::target_type,
                "[rw] DepListTargetType"
                )

        .def_readwrite("upgrade", &DepListOptions::upgrade,
                "[rw] DepListUpgradeOption"
                )

        .def_readwrite("downgrade", &DepListOptions::downgrade,
                "[rw] DepListDowngradeOption"
                )

        .def_readwrite("new_slots", &DepListOptions::new_slots,
                "[rw] DepListNewSlotsOption"
                )

        .def_readwrite("fall_back", &DepListOptions::fall_back,
                "[rw] DepListFallBackOption"
                )

        .def_readwrite("installed_deps_pre", &DepListOptions::installed_deps_pre,
                "[rw] DepListDepsOption"
                )

        .def_readwrite("installed_deps_runtime", &DepListOptions::installed_deps_runtime,
                "[rw] DepListDepsOption"
                )

        .def_readwrite("installed_deps_post", &DepListOptions::installed_deps_post,
                "[rw] DepListDepsOption"
                )

        .def_readwrite("uninstalled_deps_pre", &DepListOptions::uninstalled_deps_pre,
                "[rw] DepListDepsOption"
                )

        .def_readwrite("uninstalled_deps_runtime", &DepListOptions::uninstalled_deps_runtime,
                "[rw] DepListDepsOption"
                )

        .def_readwrite("uninstalled_deps_post", &DepListOptions::uninstalled_deps_post,
                "[rw] DepListDepsOption"
                )

        .def_readwrite("uninstalled_deps_suggested", &DepListOptions::uninstalled_deps_suggested,
                "[rw] DepListDepsOption"
                )

        .def_readwrite("suggested", &DepListOptions::suggested,
                "[rw] DepListSuggestedOption"
                )

        .def_readwrite("circular", &DepListOptions::circular,
                "[rw] DepListCircularOption"
                )

        .def_readwrite("use", &DepListOptions::use,
                "[rw] DepListUseOption"
                )

        .def_readwrite("blocks", &DepListOptions::blocks,
                "[rw] DepListBlocksOption"
                )

        .def_readwrite("dependency_tags", &DepListOptions::dependency_tags,
                "[rw] bool"
                )
        ;

    /**
     * DepListEntry
     */
    bp::register_ptr_to_python<DepListEntry *>();
    bp::class_<DepListEntry, boost::noncopyable>
        (
         "DepListEntry",
         "An entry in a DepList.",
         bp::no_init
        )
        .def_readonly("kind", &DepListEntry::kind,
                "[ro] DepListEntryKind"
                )

        .add_property("package_id", bp::make_getter(&DepListEntry::package_id,
                    bp::return_value_policy<bp::return_by_value>()),
                "[ro] PackageID"
                )

        .add_property("associated_entry", bp::make_getter(&DepListEntry::associated_entry,
                    bp::return_value_policy<bp::reference_existing_object>()),
                "[ro] DepListEntry"
                )

        .add_property("tags", bp::make_getter(&DepListEntry::tags,
                    bp::return_value_policy<bp::return_by_value>()),
                "[ro] DepListEntryTags"
                )

        .add_property("destination", bp::make_getter(&DepListEntry::destination,
                    bp::return_value_policy<bp::return_by_value>()),
                "[ro] Repository"
                )

        .def_readonly("state", &DepListEntry::state,
                "[ro] DepListEntryState"
                )
        ;

    /**
     * DepList
     */
    tr1::shared_ptr<DepListOptions> (DepList::* options_ptr)() = &DepList::options;
    DepList::Iterator (DepList::*dl_begin_ptr)() = &DepList::begin;
    DepList::Iterator (DepList::*dl_end_ptr)() = &DepList::end;
    bp::class_<DepList, boost::noncopyable>
        (
         "DepList",
         "Holds a list of dependencies in merge order.",
         bp::init<const Environment * const, const DepListOptions &>("__init__(Environment, DepListOptions)")
        )
        .add_property("options", options_ptr,
                "[ro] DepListOptions\n"
                "Our options."
                )

        .def("add", &DepListWrapper::add,
                "add(DepSpec, DestinationsIterable)\n"
                "Add the packages required to resolve an additional dependency spec."
            )

        .def("clear", &DepList::clear,
                "clear()\n"
                "Clear the list."
            )

        .def("already_installed", &DepListWrapper::already_installed,
                "already_installed(DepSpec, DestinationsIterable) -> bool\n"
                "Return whether a spec structure is already installed."
            )

        .def("match_on_list", &DepList::match_on_list,
                "match_on_list(PackageDepSpec) -> bool\n"
                "Return whether a spec matches an item in the list."
            )

        .def("__iter__", bp::range<bp::return_internal_reference<> >(dl_begin_ptr, dl_end_ptr))
        ;
}
