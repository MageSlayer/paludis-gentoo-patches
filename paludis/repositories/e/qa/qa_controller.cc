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

#include <paludis/repositories/e/qa/qa_controller.hh>
#include <paludis/repositories/e/qa/qa_checks.hh>
#include <paludis/repositories/e/e_repository.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/set.hh>
#include <paludis/util/log.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/options.hh>
#include <paludis/util/thread_pool.hh>
#include <paludis/util/action_queue.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/qa.hh>
#include <paludis/metadata_key.hh>
#include <tr1/functional>
#include <fstream>
#include <unistd.h>
#include <algorithm>
#include <list>
#include <set>
#include <map>

using namespace paludis;
using namespace paludis::erepository;

namespace
{
    struct ThreadSafeQAReporter :
        QAReporter
    {
        QAReporter & base;
        Mutex mutex, flush_mutex;
        ActionQueue message_queue;

        std::multimap<const FSEntry, const QAMessage> message_buf;
        typedef std::multimap<const FSEntry, const QAMessage>::iterator MessageIterator;

        ThreadSafeQAReporter(QAReporter & b) :
            base(b),
            message_queue(1, false, false)
        {
        }

        ~ThreadSafeQAReporter()
        {
            if (! std::uncaught_exception())
            {
                using namespace std::tr1::placeholders;
                std::for_each(message_buf.begin(), message_buf.end(),
                        std::tr1::bind(&QAReporter::message, std::tr1::ref(base),
                            std::tr1::bind<const QAMessage>(&std::pair<const FSEntry, const QAMessage>::second, _1)));
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

                message_queue.enqueue(std::tr1::bind(&QAReporter::message, &base, QAMessage(i->second)));
                message_buf.erase(i++);
            }
        }

        void message(const QAMessage & msg)
        {
            Lock lock(mutex);
            message_buf.insert(std::make_pair(msg.entry, msg));
        }

        void status(const std::string & s)
        {
            message_queue.enqueue(std::tr1::bind(&QAReporter::status, &base, std::string(s)));
        }
    };
}

namespace paludis
{
    template <>
    struct Implementation<QAController>
    {
        const Environment * const env;
        const std::tr1::shared_ptr<const ERepository> repo;
        const QACheckProperties ignore_if;
        const QACheckProperties ignore_unless;
        const QAMessageLevel minimum_level;
        ThreadSafeQAReporter reporter;
        const FSEntry base_dir;

        QAChecks qa_checks;

        std::set<CategoryNamePart> cats_pool;
        std::set<QualifiedPackageName> pkgs_pool;
        Mutex pools_mutex;

        Implementation(
                const Environment * const e,
                const std::tr1::shared_ptr<const ERepository> & r,
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
            base_dir(d.realpath()),
            qa_checks(e)
        {
        }
    };
}

QAController::QAController(
        const Environment * const env,
        const std::tr1::shared_ptr<const ERepository> & repo,
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
QAController::_worker()
{
    bool done(false);

    while (! done)
    {
        std::tr1::function<void ()> work_item;

        {
            Lock l(_imp->pools_mutex);
            if (! _imp->cats_pool.empty())
            {
                CategoryNamePart cat(*_imp->cats_pool.begin());
                FSEntry c_dir(_imp->repo->layout()->category_directory(cat));
                _imp->cats_pool.erase(_imp->cats_pool.begin());
                if (_above_base_dir(c_dir) || _under_base_dir(c_dir))
                {
                    std::tr1::shared_ptr<const QualifiedPackageNameSet> qpns(_imp->repo->package_names(cat));
                    std::copy(qpns->begin(), qpns->end(), std::inserter(_imp->pkgs_pool, _imp->pkgs_pool.begin()));
                    work_item = std::tr1::bind(&QAController::_check_category, this, cat, qpns);
                }
            }
            else if (! _imp->pkgs_pool.empty())
            {
                QualifiedPackageName qpn(*_imp->pkgs_pool.begin());
                _imp->pkgs_pool.erase(_imp->pkgs_pool.begin());
                work_item = std::tr1::bind(&QAController::_check_package, this, qpn);
            }
            else
                done = true;
        }

        if (work_item)
            work_item();
    }
}

void
QAController::_status_worker()
{
    while (true)
    {
        {
            Lock l(_imp->pools_mutex);
            _imp->reporter.status("Pending: " + stringify(_imp->cats_pool.size()) + " full categories, "
                    + stringify(_imp->pkgs_pool.size()) + " packages in '" + stringify(_imp->repo->name()) + "'");

            if (_imp->cats_pool.empty() && _imp->pkgs_pool.empty())
                break;
        }

        ::sleep(1);
    }
}

void
QAController::_check_eclasses(const FSEntry & dir, const std::string & type)
{
    using namespace std::tr1::placeholders;

    if (! _under_base_dir(dir) || ! dir.exists())
        return;

    try
    {
        for (DirIterator it(dir), it_end; it_end != it; ++it)
            if (is_file_with_extension(*it, type, IsFileWithOptions()))
            {
                std::ifstream f(stringify(*it).c_str());
                std::string content((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());

                if (! f)
                    _imp->reporter.message(
                            QAMessage(*it, qaml_severe, "check_" + type.substr(1),
                                "Couldn't get file contents for " + type.substr(1) + " '" + stringify(*it) + ")"));
                else
                    std::find_if(
                            _imp->qa_checks.eclass_file_contents_checks_group()->begin(),
                            _imp->qa_checks.eclass_file_contents_checks_group()->end(),
                            std::tr1::bind(std::equal_to<bool>(), false,
                                std::tr1::bind<bool>(std::tr1::mem_fn(&EclassFileContentsCheckFunction::operator() ),
                                    _1, *it, std::tr1::ref(_imp->reporter),
                                    _imp->env, _imp->repo, content)));
            }
    }
    catch (const InternalError &)
    {
        throw;
    }
    catch (const Exception & e)
    {
        _imp->reporter.message(
                QAMessage(dir, qaml_severe, "check_" + type.substr(1),
                    "Caught exception '" + e.message() + "' (" + e.what() + ")"));
    }

    _imp->reporter.flush(dir);
}

void
QAController::_check_category(const CategoryNamePart c, const std::tr1::shared_ptr<const QualifiedPackageNameSet> qpns)
{
    using namespace std::tr1::placeholders;

    FSEntry c_dir(_imp->repo->layout()->category_directory(c));

    if (_under_base_dir(c_dir))
    {
        using namespace std::tr1::placeholders;
        std::find_if(
                _imp->qa_checks.category_dir_checks_group()->begin(),
                _imp->qa_checks.category_dir_checks_group()->end(),
                std::tr1::bind(std::equal_to<bool>(), false,
                    std::tr1::bind<bool>(std::tr1::mem_fn(&CategoryDirCheckFunction::operator() ),
                        _1, _imp->repo->layout()->category_directory(c), std::tr1::ref(_imp->reporter),
                        _imp->env, _imp->repo, c)));
    }

    std::tr1::shared_ptr<const FSEntrySequence> exlibs(_imp->repo->layout()->exlibsdirs_category(c));
    std::for_each(exlibs->begin(), exlibs->end(), std::tr1::bind(&QAController::_check_eclasses, this, _1, ".exlib"));

    bool done(false);
    while (! done)
    {
        std::tr1::function<void ()> work_item;
        {
            Lock l(_imp->pools_mutex);
            for (QualifiedPackageNameSet::ConstIterator q(qpns->begin()), q_end(qpns->end()) ;
                    q != q_end ; ++q)
            {
                std::set<QualifiedPackageName>::iterator i(_imp->pkgs_pool.find(*q));
                if (i != _imp->pkgs_pool.end())
                {
                    _imp->pkgs_pool.erase(i);
                    work_item = std::tr1::bind(&QAController::_check_package, this, *q);
                    break;
                }
            }
        }

        if (work_item)
        {
            work_item();
        }
        else
            done = true;
    }
}

void
QAController::_check_package(const QualifiedPackageName p)
{
    using namespace std::tr1::placeholders;

    FSEntry p_dir(_imp->repo->layout()->package_directory(p));

    if (_above_base_dir(p_dir) || _under_base_dir(p_dir))
    {
        std::find_if(
                _imp->qa_checks.package_dir_checks_group()->begin(),
                _imp->qa_checks.package_dir_checks_group()->end(),
                std::tr1::bind(std::equal_to<bool>(), false,
                    std::tr1::bind<bool>(std::tr1::mem_fn(&PackageDirCheckFunction::operator()),
                        _1, _imp->repo->layout()->package_directory(p),
                        std::tr1::ref(_imp->reporter), _imp->env, _imp->repo, p)));

        std::tr1::shared_ptr<const PackageIDSequence> ids(_imp->repo->package_ids(p));
        std::for_each(ids->begin(), ids->end(), std::tr1::bind(&QAController::_check_id, this, _1));
        _imp->reporter.flush(p_dir);
    }

    std::tr1::shared_ptr<const FSEntrySequence> exlibs(_imp->repo->layout()->exlibsdirs_package(p));
    std::for_each(exlibs->begin(), exlibs->end(), std::tr1::bind(&QAController::_check_eclasses, this, _1, ".exlib"));
}

void
QAController::_check_id(const std::tr1::shared_ptr<const PackageID> & i)
{
    using namespace std::tr1::placeholders;

    FSEntry p_dir(_imp->repo->layout()->package_directory(i->name()));

    try
    {
        if (_under_base_dir(p_dir))
        {
            std::find_if(
                    _imp->qa_checks.package_id_checks_group()->begin(),
                    _imp->qa_checks.package_id_checks_group()->end(),
                    std::tr1::bind(std::equal_to<bool>(), false,
                        std::tr1::bind<bool>(std::tr1::mem_fn(&PackageIDCheckFunction::operator() ),
                            _1, i->fs_location_key()->value(), std::tr1::ref(_imp->reporter),
                            _imp->env, _imp->repo, std::tr1::static_pointer_cast<const ERepositoryID>(i))));

            std::ifstream f(stringify(i->fs_location_key()->value()).c_str());
            std::string content((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
            if (! f)
                _imp->reporter.message(
                        QAMessage(i->fs_location_key()->value(), qaml_severe, "check_id",
                            "Couldn't get file contents for ID '" + stringify(*i) + ")")
                        .with_associated_id(i)
                        .with_associated_key(i, i->fs_location_key()));
            else
                std::find_if(
                        _imp->qa_checks.package_id_file_contents_checks_group()->begin(),
                        _imp->qa_checks.package_id_file_contents_checks_group()->end(),
                        std::tr1::bind(std::equal_to<bool>(), false,
                            std::tr1::bind<bool>(std::tr1::mem_fn(&PackageIDFileContentsCheckFunction::operator() ),
                                _1, i->fs_location_key()->value(), std::tr1::ref(_imp->reporter),
                                _imp->env, _imp->repo, std::tr1::static_pointer_cast<const ERepositoryID>(i), content)));
        }
    }
    catch (const InternalError &)
    {
        throw;
    }
    catch (const Exception & e)
    {
        _imp->reporter.message(
                QAMessage(i->fs_location_key()->value(), qaml_severe, "check_id",
                    "Caught exception '" + e.message() + "' (" + e.what() + ")")
                .with_associated_id(i));
    }
}

void
QAController::run()
{
    using namespace std::tr1::placeholders;

    try
    {
        if (_under_base_dir(_imp->repo->params().location()))
            if (_imp->qa_checks.tree_checks_group()->end() !=
                    std::find_if(
                        _imp->qa_checks.tree_checks_group()->begin(),
                        _imp->qa_checks.tree_checks_group()->end(),
                        std::tr1::bind(std::equal_to<bool>(), false,
                            std::tr1::bind<bool>(std::tr1::mem_fn(&TreeCheckFunction::operator() ),
                                _1, _imp->repo->params().location(), std::tr1::ref(_imp->reporter),
                                _imp->env, _imp->repo))))
            {
                QAMessage(_imp->repo->params().location(), qaml_severe, "tree_checks_group",
                        "Tree checks failed. Not continuing.");
                return;
            }
        _imp->reporter.flush(_imp->repo->params().location());

        std::for_each(_imp->repo->params().eclassdirs()->begin(),
                      _imp->repo->params().eclassdirs()->end(),
                      std::tr1::bind(&QAController::_check_eclasses, this, _1, ".eclass"));

        std::tr1::shared_ptr<const FSEntrySequence> exlibs(_imp->repo->layout()->exlibsdirs_global());
        std::for_each(exlibs->begin(), exlibs->end(), std::tr1::bind(&QAController::_check_eclasses, this, _1, ".exlib"));

        /* Create our workers and pools. Each worker starts by working on a
         * separate category. If there aren't any unclaimed categories, workers
         * start taking packages from another worker's category. */
        std::tr1::shared_ptr<const CategoryNamePartSet> cats(_imp->repo->category_names());
        std::copy(cats->begin(), cats->end(), std::inserter(_imp->cats_pool, _imp->cats_pool.begin()));
        ThreadPool workers;
        for (int x(0) ; x < 5 ; ++x)
            workers.create_thread(std::tr1::bind(&QAController::_worker, this));
        workers.create_thread(std::tr1::bind(&QAController::_status_worker, this));
    }
    catch (const InternalError &)
    {
        throw;
    }
    catch (const Exception & e)
    {
        _imp->reporter.message(
                QAMessage(_imp->repo->params().location(), qaml_severe, "run",
                    "Caught exception '" + e.message() + "' (" + e.what() + ")"));
    }
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

