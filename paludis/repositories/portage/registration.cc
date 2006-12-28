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

#include <paludis/repository_maker.hh>
#include <paludis/repositories/portage/make_ebuild_repository.hh>
#include "config.h"

using namespace paludis;

#ifndef MONOLITHIC

extern "C"
{
    void register_repositories(RepositoryMaker * maker);
}

void register_repositories(RepositoryMaker * maker)
{
    maker->register_maker("ebuild", &make_ebuild_repository_wrapped);
    maker->register_maker("portage", &make_ebuild_repository_wrapped);
}

#endif


