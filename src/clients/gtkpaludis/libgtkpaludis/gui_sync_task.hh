/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef GTKPALUDIS_GUARD_LIBGTKPALUDIS_GUI_SYNC_TASK_HH
#define GTKPALUDIS_GUARD_LIBGTKPALUDIS_GUI_SYNC_TASK_HH 1

#include <paludis/sync_task.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <libgtkpaludis/gui_task.hh>

namespace gtkpaludis
{
    class MainWindow;

    class GuiSyncTask :
        public GuiTask,
        protected paludis::SyncTask,
        private paludis::PrivateImplementationPattern<GuiSyncTask>
    {
        private:
            paludis::PrivateImplementationPattern<GuiSyncTask>::ImpPtr & _imp;

        protected:
            virtual void on_sync_all_pre();
            virtual void on_sync_pre(const paludis::RepositoryName &);
            virtual void on_sync_post(const paludis::RepositoryName &);
            virtual void on_sync_skip(const paludis::RepositoryName &);
            virtual void on_sync_fail(const paludis::RepositoryName &, const paludis::SyncFailedError &);
            virtual void on_sync_succeed(const paludis::RepositoryName &);
            virtual void on_sync_all_post();

        public:
            GuiSyncTask(MainWindow * const);
            ~GuiSyncTask();

            using paludis::SyncTask::add_target;

            virtual void run();
            virtual void paludis_thread_execute();
    };
}

#endif
