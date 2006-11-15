/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <gtkmm/main.h>
#include <paludis/environment/default/default_config.hh>
#include <paludis/util/fs_entry.hh>
#include <test/test_framework.hh>

#include "main_window.hh"
#include "sets_page.hh"
#include "sets_list.hh"
#include "test_common.hh"

using namespace paludis;
using namespace gtkpaludis;
using namespace test;

namespace
{
    struct SetsListTest :
        TestCase
    {
        SetsListTest() : TestCase("sets list") { }

        void run()
        {
            MainWindow::get_instance()->show_sets_page();
            TEST_CHECK(MainWindow::get_instance()->sets_page()->sets_list()->number_of_sets() > 1);
            TEST_CHECK(MainWindow::get_instance()->sets_page()->sets_list()->has_set_named(SetName("system")));
            TEST_CHECK(! MainWindow::get_instance()->sets_page()->sets_list()->has_set_named(SetName("monkey")));
        }

        bool repeatable() const
        {
            return false;
        }
    } test_sets_list;
}

sigc::connection
gtkpaludis::launch_signal_connection(sigc::slot<void> slot)
{
    return MainWindow::get_instance()->sets_page()->sets_list()->populated().connect(slot);
}

std::string
gtkpaludis::test_dir()
{
    return "sets_list_TEST_dir";
}

