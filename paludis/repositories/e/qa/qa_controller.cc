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

#include <paludis/repositories/e/qa/qa_controller.hh>
#include <paludis/repositories/e/qa/qa_checks.hh>
#include <paludis/repositories/e/e_repository.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/tr1_functional.hh>
#include <paludis/util/set.hh>

#include <libwrapiter/libwrapiter_forward_iterator.hh>

#include <algorithm>

using namespace paludis;
using namespace paludis::erepository;

namespace paludis
{
    template <>
    struct Implementation<QAController>
    {
        const Environment * const env;
        const tr1::shared_ptr<const ERepository> & repo;
        const QACheckProperties & ignore_if;
        const QACheckProperties & ignore_unless;
        const QAMessageLevel minimum_level;
        QAReporter & reporter;

        Implementation(
                const Environment * const e,
                const tr1::shared_ptr<const ERepository> & r,
                const QACheckProperties & i,
                const QACheckProperties & u,
                const QAMessageLevel m,
                QAReporter & q
                ) :
            env(e),
            repo(r),
            ignore_if(i),
            ignore_unless(u),
            minimum_level(m),
            reporter(q)
        {
        }
    };
}

QAController::QAController(
        const Environment * const env,
        const tr1::shared_ptr<const ERepository> & repo,
        const QACheckProperties & ignore_if,
        const QACheckProperties & ignore_unless,
        const QAMessageLevel minimum_level,
        QAReporter & reporter
        ) :
    PrivateImplementationPattern<QAController>(new Implementation<QAController>(
                env, repo, ignore_if, ignore_unless, minimum_level, reporter))
{
}

QAController::~QAController()
{
}

void
QAController::run()
{
    using namespace tr1::placeholders;

    std::find_if(
            QAChecks::get_instance()->tree_checks_group()->begin(),
            QAChecks::get_instance()->tree_checks_group()->end(),
            tr1::bind(std::equal_to<bool>(), false,
                tr1::bind<bool>(tr1::mem_fn(&TreeCheckFunction::operator() ),
                    _1, tr1::ref(_imp->reporter), _imp->env, _imp->repo, _imp->repo->params().location)));

    tr1::shared_ptr<const CategoryNamePartSet> categories(_imp->repo->category_names());
    for (CategoryNamePartSet::Iterator c(categories->begin()), c_end(categories->end()) ;
            c != c_end ; ++c)
        std::find_if(
                QAChecks::get_instance()->category_dir_checks_group()->begin(),
                QAChecks::get_instance()->category_dir_checks_group()->end(),
                tr1::bind(std::equal_to<bool>(), false,
                    tr1::bind<bool>(tr1::mem_fn(&CategoryDirCheckFunction::operator() ),
                        _1, tr1::ref(_imp->reporter), _imp->env, _imp->repo, _imp->repo->layout()->category_directory(*c))));

}

