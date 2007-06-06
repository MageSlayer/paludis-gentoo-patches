/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "task_sequence_list.hh"
#include "task_sequence_list_model.hh"
#include <paludis/util/private_implementation_pattern-impl.hh>

using namespace gtkpaludis;
using namespace paludis;

namespace paludis
{
    template<>
    struct Implementation<TaskSequenceList>
    {
        Glib::RefPtr<TaskSequenceListModel> model;

        Implementation() :
            model(new TaskSequenceListModel)
        {
        }
    };
}

TaskSequenceList::TaskSequenceList() :
    Gtk::TreeView(),
    PrivateImplementationPattern<TaskSequenceList>(new Implementation<TaskSequenceList>())
{
    set_model(_imp->model);

    append_column("Action", _imp->model->columns().col_description);
    append_column("Status", _imp->model->columns().col_status);
}

TaskSequenceList::~TaskSequenceList()
{
}

void
TaskSequenceList::append_sequence_item(const std::string & id, const std::string & description,
        const std::string & status)
{
    _imp->model->append_sequence_item(id, description, status);
}

void
TaskSequenceList::set_sequence_item_status(const std::string & id, const std::string & status)
{
    _imp->model->set_sequence_item_status(id, status);
}

