/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009 Ciaran McCreesh
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

#include <paludis/repositories/e/qa/qa_controller.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/fs_entry-fwd.hh>
#include <paludis/environment-fwd.hh>
#include <paludis/qa-fwd.hh>

using namespace paludis;

namespace paludis
{
    class RepositoryFactory;
    class ERepository;
}

void check_qa(
        const Environment * const env,
        const std::tr1::shared_ptr<const ERepository> & repo,
        const QACheckProperties & ignore_if,
        const QACheckProperties & ignore_unless,
        const QAMessageLevel minimum_level,
        QAReporter & reporter,
        const FSEntry & if_dir)
{
    erepository::QAController controller(env, repo, ignore_if, ignore_unless, minimum_level, reporter, if_dir);
    controller.run();
}

