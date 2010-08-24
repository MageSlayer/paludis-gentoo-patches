/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010 Ciaran McCreesh
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

#include <paludis/util/fs_path.hh>
#include <paludis/filter.hh>

using namespace paludis;
using namespace paludis::python;
namespace bp = boost::python;

namespace
{
    template <typename C_>
    class class_supports_action_filter :
        public bp::class_<filter::SupportsAction<C_>, bp::bases<Filter> >
    {
        public:
            class_supports_action_filter(const std::string & a) :
                bp::class_<filter::SupportsAction<C_>, bp::bases<Filter> >(
                        ("Supports" + a + "Action").c_str(),
                        ("Accept packages that support " + a + "Action").c_str(),
                        bp::init<>("__init__()")
                        )
        {
        }
    };
}

void expose_filter()
{
    bp::class_<Filter> filter(
            "Filter",
            "Filter for an Environment selection.",
            bp::no_init
            );

    filter
        .def(bp::self_ns::str(bp::self))
        ;

    bp::scope filter_scope = filter;

    bp::class_<filter::All, bp::bases<Filter> > filter_all(
            "All",
            "Accept all packages.",
            bp::init<>("__init__()")
            );

    bp::class_<filter::NotMasked, bp::bases<Filter> > filter_not_masked(
            "NotMasked",
            "Accept unmasked packages.",
            bp::init<>("__init__()")
            );

    bp::class_<filter::InstalledAtRoot, bp::bases<Filter> > filter_installed_at_root(
            "InstalledAtRoot",
            "Accept packages installed at a particular root.",
            bp::init<const FSPath &>("__init__(path)")
            );

    bp::class_<filter::And, bp::bases<Filter> > filter_and(
            "And",
            "Accept packages that match both filters.",
            bp::init<const Filter &, const Filter &>("__init__(filter, filter)")
            );

    class_supports_action_filter<InstallAction>("Install");
    class_supports_action_filter<UninstallAction>("Uninstall");
    class_supports_action_filter<PretendAction>("Pretend");
    class_supports_action_filter<ConfigAction>("Config");
    class_supports_action_filter<FetchAction>("Fetch");
    class_supports_action_filter<InfoAction>("Info");
    class_supports_action_filter<PretendFetchAction>("PretendFetch");
}

