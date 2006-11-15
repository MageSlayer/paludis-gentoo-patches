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

#ifndef PALUDIS_GUARD_SRC_GTKPALUDIS_TEST_COMMON_HH
#define PALUDIS_GUARD_SRC_GTKPALUDIS_TEST_COMMON_HH 1

#include <sigc++/connection.h>
#include <sigc++/slot.h>
#include <string>

namespace gtkpaludis
{
    struct GtkMainQuitOnDestruction
    {
        ~GtkMainQuitOnDestruction();
    };

    sigc::connection launch_signal_connection(sigc::slot<void>);
    std::string test_dir();
}

#endif
