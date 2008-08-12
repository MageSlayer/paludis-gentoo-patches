/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include <paludis/environment_factory.hh>
#include <paludis/util/log.hh>
#include <libgtkpaludis/main_window.hh>
#include <gtkmm.h>
#include <cstdlib>

using namespace gtkpaludis;

int main(int argc, char *argv[])
{
    Glib::thread_init();
    Gtk::Main kit(argc, argv);

    paludis::Log::get_instance()->set_log_level(paludis::ll_qa);
    paludis::Log::get_instance()->set_program_name(argv[0]);
    std::tr1::shared_ptr<paludis::Environment> env(
            paludis::EnvironmentFactory::get_instance()->create(""));

    MainWindow main_window(env.get());
    Gtk::Main::run(main_window);

    return EXIT_SUCCESS;
}

