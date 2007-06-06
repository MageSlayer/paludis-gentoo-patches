/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef GTKPALUDIS_GUARD_LIBGTKPALUDIS_TASK_SEQUENCE_LIST_HH
#define GTKPALUDIS_GUARD_LIBGTKPALUDIS_TASK_SEQUENCE_LIST_HH 1

#include <paludis/util/private_implementation_pattern.hh>
#include <gtkmm/treeview.h>

namespace gtkpaludis
{
    class TaskSequenceList :
        public Gtk::TreeView,
        private paludis::PrivateImplementationPattern<TaskSequenceList>
    {
        public:
            TaskSequenceList();
            ~TaskSequenceList();

            void append_sequence_item(const std::string & id, const std::string & description,
                    const std::string & status);

            void set_sequence_item_status(const std::string & id, const std::string & status);
    };
}


#endif
