/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh
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
#include <paludis/util/log.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/parallel_for_each.hh>
#include <paludis/util/options.hh>
#include <paludis/qa.hh>

#include <libwrapiter/libwrapiter_forward_iterator.hh>

#include <algorithm>
#include <list>
#include <map>

using namespace paludis;
using namespace paludis::erepository;

namespace
{
    struct ThreadSafeQAReporter :
        QAReporter
    {
        QAReporter & base;
        Mutex mutex;

        std::multimap<const FSEntry, const QAMessage> message_buf;
        typedef std::multimap<const FSEntry, const QAMessage>::iterator MessageIterator;

        ThreadSafeQAReporter(QAReporter & b) :
            base(b)
        {
        }

        ~ThreadSafeQAReporter()
        {
            if (! std::uncaught_exception())
            {
                using namespace tr1::placeholders;
                std::for_each(message_buf.begin(), message_buf.end(),
                        tr1::bind(&QAReporter::message, tr1::ref(base),
                            tr1::bind<const QAMessage>(&std::pair<const FSEntry, const QAMessage>::second, _1)));
            }
        }

        void flush(const FSEntry & f)
        {
            Lock lock(mutex);

            std::string root(stringify(f));

            for (MessageIterator i(message_buf.lower_bound(f)), i_end(message_buf.end()) ; i != i_end ; )
            {
                if (0 != stringify(i->first).compare(0, root.length(), root))
                    break;

                base.message(i->second);
                message_buf.erase(i++);
            }
        }

        void message(const QAMessage & msg)
        {
            Lock lock(mutex);
            message_buf.insert(std::make_pair(msg.entry, msg));
        }
    };
}

namespace paludis
{
    template <>
    struct Implementation<QAController>
    {
        const Environment * const env;
        const tr1::shared_ptr<const ERepository> repo;
        const QACheckProperties ignore_if;
        const QACheckProperties ignore_unless;
        const QAMessageLevel minimum_level;
        ThreadSafeQAReporter reporter;
        const FSEntry base_dir;

        Implementation(
                const Environment * const e,
                const tr1::shared_ptr<const ERepository> & r,
                const QACheckProperties & i,
                const QACheckProperties & u,
                const QAMessageLevel m,
                QAReporter & q,
                const FSEntry & d
                ) :
            env(e),
            repo(r),
            ignore_if(i),
            ignore_unless(u),
            minimum_level(m),
            reporter(q),
            base_dir(d.realpath())
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
        QAReporter & reporter,
        const FSEntry & base_dir
        ) :
    PrivateImplementationPattern<QAController>(new Implementation<QAController>(
                env, repo, ignore_if, ignore_unless, minimum_level, reporter, base_dir))
{
}

QAController::~QAController()
{
}

void
QAController::_run_category(const CategoryNamePart & c)
{
    using namespace tr1::placeholders;

    FSEntry c_dir(_imp->repo->layout()->category_directory(c));
    try
    {
        if (_under_base_dir(c_dir))
            std::find_if(
                    QAChecks::get_instance()->category_dir_checks_group()->begin(),
                    QAChecks::get_instance()->category_dir_checks_group()->end(),
                    tr1::bind(std::equal_to<bool>(), false,
                        tr1::bind<bool>(tr1::mem_fn(&CategoryDirCheckFunction::operator() ),
                            _1, _imp->repo->layout()->category_directory(c), tr1::ref(_imp->reporter),
                            _imp->env, _imp->repo, _imp->repo->layout()->category_directory(c))));
        _imp->reporter.flush(c_dir);
    }
    catch (const Exception & e)
    {
        _imp->reporter.message(
                QAMessage(_imp->repo->layout()->category_directory(c), qaml_severe, "category_dir_checks_group",
                    "Caught exception '" + e.message() + "' (" + e.what() + ")"));
        _imp->reporter.flush(c_dir);
    }

    if (_above_base_dir(c_dir) || _under_base_dir(c_dir))
    {
        tr1::shared_ptr<const QualifiedPackageNameSet> packages(_imp->repo->package_names(c));
        parallel_for_each(packages->begin(), packages->end(), tr1::bind(&QAController::_run_package, this, _1));
    }
}

void
QAController::_run_package(const QualifiedPackageName & p)
{
    using namespace tr1::placeholders;

    FSEntry p_dir(_imp->repo->layout()->package_directory(p));

    if (_above_base_dir(p_dir) || _under_base_dir(p_dir))
    {
        tr1::shared_ptr<const PackageIDSequence> ids(_imp->repo->package_ids(p));
        parallel_for_each(ids->begin(), ids->end(), tr1::bind(&QAController::_run_id, this, _1));

        _imp->reporter.flush(p_dir);
    }
}

void
QAController::_run_id(const tr1::shared_ptr<const PackageID> & i)
{
    using namespace tr1::placeholders;

    FSEntry p_dir(_imp->repo->layout()->package_directory(i->name()));

    try
    {
        if (_under_base_dir(p_dir))
            std::find_if(
                    QAChecks::get_instance()->package_id_checks_group()->begin(),
                    QAChecks::get_instance()->package_id_checks_group()->end(),
                    tr1::bind(std::equal_to<bool>(), false,
                        tr1::bind<bool>(tr1::mem_fn(&PackageIDCheckFunction::operator() ),
                            _1, _imp->repo->layout()->package_file(*i), tr1::ref(_imp->reporter),
                            _imp->env, _imp->repo, tr1::static_pointer_cast<const ERepositoryID>(i))));
    }
    catch (const Exception & e)
    {
        _imp->reporter.message(
                QAMessage(_imp->repo->layout()->package_file(*i), qaml_severe, "package_id_checks_group",
                    "Caught exception '" + e.message() + "' (" + e.what() + ")")
                .with_associated_id(i));
    }
}

void
QAController::run()
{
    using namespace tr1::placeholders;

    try
    {
        if (_under_base_dir(_imp->repo->params().location))
            std::find_if(
                    QAChecks::get_instance()->tree_checks_group()->begin(),
                    QAChecks::get_instance()->tree_checks_group()->end(),
                    tr1::bind(std::equal_to<bool>(), false,
                        tr1::bind<bool>(tr1::mem_fn(&CategoryDirCheckFunction::operator() ),
                            _1, _imp->repo->params().location, tr1::ref(_imp->reporter),
                            _imp->env, _imp->repo, _imp->repo->params().location)));
    }
    catch (const Exception & e)
    {
        _imp->reporter.message(
                QAMessage(_imp->repo->params().location, qaml_severe, "tree_checks_group",
                    "Caught exception '" + e.message() + "' (" + e.what() + ")"));
    }

    _imp->reporter.flush(_imp->repo->params().location);

    tr1::shared_ptr<const CategoryNamePartSet> categories(_imp->repo->category_names());
    parallel_for_each(categories->begin(), categories->end(), tr1::bind(&QAController::_run_category, this, _1));
}

bool
QAController::_under_base_dir(const FSEntry & d) const
{
    if (! d.exists())
        return false;

    FSEntry dd(d.realpath()), b("/");
    while (dd != b)
    {
        if (dd == _imp->base_dir)
            return true;
        dd = dd.dirname();
    }

    return false;
}

bool
QAController::_above_base_dir(const FSEntry & d) const
{
    if (! d.exists())
        return false;

    FSEntry dd(_imp->base_dir), b("/");
    while (dd != b)
    {
        if (dd == d)
            return true;
        dd = dd.dirname();
    }

    return false;
}

