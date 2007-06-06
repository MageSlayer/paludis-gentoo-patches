/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "task_sequence_list_model.hh"
#include <paludis/util/private_implementation_pattern-impl.hh>

using namespace paludis;
using namespace gtkpaludis;

namespace paludis
{
    template<>
    struct Implementation<TaskSequenceListModel>
    {
        TaskSequenceListModel::Columns columns;
    };
}

TaskSequenceListModel::TaskSequenceListModel() :
    PrivateImplementationPattern<TaskSequenceListModel>(new Implementation<TaskSequenceListModel>()),
    Gtk::ListStore(_imp->columns)
{
}

TaskSequenceListModel::~TaskSequenceListModel()
{
}

TaskSequenceListModel::Columns::Columns()
{
    add(col_id);
    add(col_description);
    add(col_status);
}

TaskSequenceListModel::Columns::~Columns()
{
}

TaskSequenceListModel::Columns &
TaskSequenceListModel::columns()
{
    return _imp->columns;
}

void
TaskSequenceListModel::append_sequence_item(const std::string & id, const std::string & description,
        const std::string & status)
{
    iterator r(append());
    (*r)[_imp->columns.col_id] = id;
    (*r)[_imp->columns.col_description] = description;
    (*r)[_imp->columns.col_status] = status;
}

void
TaskSequenceListModel::set_sequence_item_status(const std::string & id, const std::string & status)
{
    Children c(children());
    for (iterator r(c.begin()), r_end(c.end()) ; r != r_end ; ++r)
        if ((*r)[_imp->columns.col_id] == id)
            (*r)[_imp->columns.col_status] = status;
}

