/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef GTKPALUDIS_GUARD_LIBGTKPALUDIS_REPOSITORY_BUTTONS_HH
#define GTKPALUDIS_GUARD_LIBGTKPALUDIS_REPOSITORY_BUTTONS_HH 1

#include <gtkmm/buttonbox.h>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/name.hh>

namespace gtkpaludis
{
    class MainWindow;

    class RepositoryButtons :
        public Gtk::HButtonBox,
        private paludis::PrivateImplementationPattern<RepositoryButtons>
    {
        private:
            void set_repository_in_paludis_thread(const paludis::RepositoryName &);
            void set_repository_in_gui_thread(const bool);

        protected:
            void handle_sync_button_clicked();
            void handle_sync_all_button_clicked();

        public:
            RepositoryButtons(MainWindow * const m);
            ~RepositoryButtons();

            void set_repository(const paludis::RepositoryName &);
    };
}

#endif
