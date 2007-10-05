/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <paludis/environment_maker.hh>
#include <paludis/environments/portage/portage_environment.hh>

using namespace paludis;

namespace
{
    tr1::shared_ptr<Environment>
    make_portage_environment(const std::string & s)
    {
        return tr1::shared_ptr<Environment>(new PortageEnvironment(s));
    }
}

extern "C"
{
    void PALUDIS_VISIBLE register_environments(EnvironmentMaker * maker);
}

void register_environments(EnvironmentMaker * maker)
{
    maker->register_maker("portage", &make_portage_environment);
}

