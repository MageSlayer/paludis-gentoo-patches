/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_QA_STRAY_FILES_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_QA_STRAY_FILES_HH 1

#include <paludis/repositories/e/qa/qa_controller.hh>

namespace paludis
{
    namespace erepository
    {
        bool stray_files_check(
                QAReporter &,
                const std::tr1::shared_ptr<const ERepository> &,
                const FSEntry & dir,
                const std::tr1::function<bool (const std::tr1::shared_ptr<const ERepository> &, const FSEntry &)> &,
                const std::string & s
                );

        bool is_stray_at_tree_dir(
                const std::tr1::shared_ptr<const ERepository> &,
                const FSEntry &);

        bool is_stray_at_category_dir(
                const std::tr1::shared_ptr<const ERepository> &,
                const FSEntry &);
    }
}

#endif
