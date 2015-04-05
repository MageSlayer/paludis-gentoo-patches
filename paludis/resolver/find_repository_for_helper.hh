/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010, 2011, 2014 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_RESOLVER_FIND_REPOSITORY_FOR_HELPER_HH
#define PALUDIS_GUARD_PALUDIS_RESOLVER_FIND_REPOSITORY_FOR_HELPER_HH 1

#include <paludis/resolver/find_repository_for_helper-fwd.hh>
#include <paludis/resolver/resolution-fwd.hh>
#include <paludis/resolver/decision-fwd.hh>
#include <paludis/util/pimp.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/fs_path-fwd.hh>
#include <paludis/environment-fwd.hh>
#include <paludis/repository-fwd.hh>
#include <memory>

namespace paludis
{
    namespace resolver
    {
        class PALUDIS_VISIBLE FindRepositoryForHelper
        {
            private:
                Pimp<FindRepositoryForHelper> _imp;

            public:
                explicit FindRepositoryForHelper(const Environment * const);
                ~FindRepositoryForHelper();

                void set_chroot_path(const FSPath &);

                void set_cross_compile_host(const std::string &);

                const std::shared_ptr<const Repository> operator() (
                        const std::shared_ptr<const Resolution> &,
                        const ChangesToMakeDecision &) const;
        };
    }

    extern template class Pimp<resolver::FindRepositoryForHelper>;
}

#endif
