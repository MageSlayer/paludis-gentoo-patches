/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef GTKPALUDIS_GUARD_LIBGTKPALUDIS_GUI_TASK_HH
#define GTKPALUDIS_GUARD_LIBGTKPALUDIS_GUI_TASK_HH 1

namespace gtkpaludis
{
    class TaskWindow;

    class GuiTask
    {
        public:
            virtual ~GuiTask() = 0;
            virtual void paludis_thread_execute() = 0;
    };
}

#endif
