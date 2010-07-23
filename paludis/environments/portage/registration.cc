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

#include <paludis/environment_factory.hh>
#include <paludis/environments/portage/portage_environment.hh>
#include <paludis/util/set.hh>
#include "config.h"

using namespace paludis;

namespace
{
    std::shared_ptr<Environment>
    make_portage_environment(const std::string & s)
    {
        return std::shared_ptr<Environment>(std::make_shared<PortageEnvironment>(s));
    }
}

namespace paludis
{
    namespace environment_groups
    {
        ENVIRONMENT_GROUPS_DECLS;
    }

    template <>
    void register_environment<environment_groups::portage>(const environment_groups::portage * const,
            EnvironmentFactory * const factory)
    {
        std::shared_ptr<Set<std::string> > portage_formats(std::make_shared<Set<std::string>>());
        portage_formats->insert("portage");
        factory->add_environment_format(portage_formats, &make_portage_environment);
    }
}

