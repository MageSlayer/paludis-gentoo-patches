/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <paludis/repositories/repository_maker.hh>
#include <paludis/repositories/e/make_ebuild_repository.hh>
#include <paludis/repositories/e/make_ebin_repository.hh>
#include <paludis/repositories/e/vdb_repository.hh>
#include <paludis/util/log.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>
#include <libwrapiter/libwrapiter_output_iterator.hh>
#include "config.h"

using namespace paludis;

#ifndef MONOLITHIC

extern "C"
{
    void PALUDIS_VISIBLE register_repositories(RepositoryMaker * maker);
}

namespace
{
    tr1::shared_ptr<Repository>
    make_portage_repository(
            Environment * const env,
            tr1::shared_ptr<const AssociativeCollection<std::string, std::string> > m)
    {
        std::string repo_file = "?";
        if (m->end() != m->find("repo_file"))
            repo_file = m->find("repo_file")->second;

        Context context("When creating repository using '" + repo_file + "':");

        Log::get_instance()->message(ll_warning, lc_context, "Format 'portage' is "
                "deprecated, use 'ebuild' instead");

        return make_ebuild_repository_wrapped(env, m);
    }
}

void register_repositories(RepositoryMaker * maker)
{
    maker->register_maker("ebuild", &make_ebuild_repository_wrapped);
    maker->register_maker("ebin", &make_ebin_repository_wrapped);
    maker->register_maker("portage", &make_portage_repository);
    maker->register_maker("vdb", &VDBRepository::make_vdb_repository);
}

#endif


