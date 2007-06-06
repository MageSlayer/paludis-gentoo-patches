/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef GTKPALUDIS_GUARD_LIBGTKPALUDIS_TASK_SEQUENCE_LIST_MODEL_HH
#define GTKPALUDIS_GUARD_LIBGTKPALUDIS_TASK_SEQUENCE_LIST_MODEL_HH 1

#include <gtkmm/liststore.h>
#include <paludis/util/private_implementation_pattern.hh>

namespace gtkpaludis
{
    class TaskSequenceListModel :
        private paludis::PrivateImplementationPattern<TaskSequenceListModel>,
        public Gtk::ListStore
    {
        public:
            TaskSequenceListModel();
            ~TaskSequenceListModel();

            class Columns :
                public Gtk::TreeModelColumnRecord
            {
                public:
                    Columns();
                    ~Columns();

                    Gtk::TreeModelColumn<Glib::ustring> col_id;
                    Gtk::TreeModelColumn<Glib::ustring> col_description;
                    Gtk::TreeModelColumn<Glib::ustring> col_status;
            };

            Columns & columns();

            void append_sequence_item(const std::string & id, const std::string & description,
                    const std::string & status);

            void set_sequence_item_status(const std::string & id, const std::string & status);
    };
}


#endif
