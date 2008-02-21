/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "repository_buttons.hh"
#include "main_window.hh"
#include "gui_sync_task.hh"
#include <gtkmm/button.h>
#include <paludis/environment.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/package_database.hh>
#include <paludis/repository.hh>

using namespace gtkpaludis;
using namespace paludis;

namespace paludis
{
    template<>
    struct Implementation<RepositoryButtons>
    {
        MainWindow * const main_window;

        Gtk::Button sync_button;
        Gtk::Button sync_all_button;

        tr1::shared_ptr<const RepositoryName> repository_name;

        Implementation(MainWindow * const m) :
            main_window(m),
            sync_button("Sync"),
            sync_all_button("Sync All")
        {
        }
    };
}

RepositoryButtons::RepositoryButtons(MainWindow * const m) :
    Gtk::HButtonBox(),
    PrivateImplementationPattern<RepositoryButtons>(new Implementation<RepositoryButtons>(m))
{
    set_layout(Gtk::BUTTONBOX_END);
    set_spacing(10);

    _imp->sync_button.signal_clicked().connect(sigc::mem_fun(this, &RepositoryButtons::handle_sync_button_clicked));
    add(_imp->sync_button);

    _imp->sync_all_button.signal_clicked().connect(sigc::mem_fun(this, &RepositoryButtons::handle_sync_all_button_clicked));
    add(_imp->sync_all_button);
}

RepositoryButtons::~RepositoryButtons()
{
}

void
RepositoryButtons::set_repository(const RepositoryName & name)
{
    _imp->repository_name.reset(new RepositoryName(name));
    _imp->main_window->paludis_thread_action(
            sigc::bind(sigc::mem_fun(this, &RepositoryButtons::set_repository_in_paludis_thread), name),
            "Preparing repository buttons");
}

void
RepositoryButtons::set_repository_in_paludis_thread(const RepositoryName & name)
{
    _imp->main_window->gui_thread_action(
            sigc::bind(sigc::mem_fun(this, &RepositoryButtons::set_repository_in_gui_thread),
                0 != (*_imp->main_window->environment()->package_database()->fetch_repository(name))[k::syncable_interface()]));
}

void
RepositoryButtons::set_repository_in_gui_thread(const bool v)
{
    _imp->sync_button.set_sensitive(v);
}

void
RepositoryButtons::handle_sync_button_clicked()
{
    GuiSyncTask * task(new GuiSyncTask(_imp->main_window));
    if (_imp->repository_name)
        task->add_target(stringify(*_imp->repository_name));
    task->run();
}

void
RepositoryButtons::handle_sync_all_button_clicked()
{
    GuiSyncTask * task(new GuiSyncTask(_imp->main_window));
    task->run();
}

