/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "gui_sync_task.hh"
#include "main_window.hh"
#include "main_notebook.hh"
#include "task_window.hh"
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/wrapped_forward_iterator.hh>

using namespace gtkpaludis;
using namespace paludis;

namespace paludis
{
    template <>
    struct Implementation<GuiSyncTask>
    {
        MainWindow * const main_window;
        TaskWindow * task_window;

        Implementation(MainWindow * const m, GuiSyncTask * const g) :
            main_window(m),
            task_window(new TaskWindow(m, g))
        {
        }
    };
}

GuiSyncTask::GuiSyncTask(MainWindow * const m) :
    SyncTask(m->environment(), false),
    PrivateImplementationPattern<GuiSyncTask>(new Implementation<GuiSyncTask>(m, this)),
    _imp(PrivateImplementationPattern<GuiSyncTask>::_imp)
{
}

GuiSyncTask::~GuiSyncTask()
{
}

void
GuiSyncTask::on_sync_all_pre()
{
    for (TargetsConstIterator t(begin_targets()), t_end(end_targets()) ;
            t != t_end ; ++t)
        _imp->task_window->gui_thread_action(
                sigc::bind(sigc::mem_fun(_imp->task_window, &TaskWindow::append_sequence_item),
                    stringify(*t), stringify(*t), "Pending..."));
}

void
GuiSyncTask::on_sync_pre(const RepositoryName & r)
{
    _imp->task_window->gui_thread_action(
            sigc::bind(sigc::mem_fun(_imp->task_window, &TaskWindow::set_sequence_item_status),
                stringify(r), "Syncing..."));
}

void
GuiSyncTask::on_sync_post(const RepositoryName &)
{
}

void
GuiSyncTask::on_sync_skip(const RepositoryName & r)
{
    _imp->task_window->gui_thread_action(
            sigc::bind(sigc::mem_fun(_imp->task_window, &TaskWindow::set_sequence_item_status),
                stringify(r), "Skipped"));
}

void
GuiSyncTask::on_sync_fail(const RepositoryName & r, const SyncFailedError &)
{
    _imp->task_window->gui_thread_action(
            sigc::bind(sigc::mem_fun(_imp->task_window, &TaskWindow::set_sequence_item_status),
                stringify(r), "Failed"));
}

void
GuiSyncTask::on_sync_succeed(const RepositoryName & r)
{
    _imp->task_window->gui_thread_action(
            sigc::bind(sigc::mem_fun(_imp->task_window, &TaskWindow::set_sequence_item_status),
                stringify(r), "Success"));
}

void
GuiSyncTask::on_sync_all_post()
{
}

void
GuiSyncTask::run()
{
    _imp->task_window->run();
}

void
GuiSyncTask::paludis_thread_execute()
{
    execute();
}

void
GuiSyncTask::on_sync_status(const int, const int, const int)
{
}

