/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
 *
 * This file is part of the Paludis package manager. Paludis is free software;
 * you can redistribute it and/or modify it under the terms of the GNU General
 * Public License version 2, as published by the Free Software Foundation.
 *
 * Paludis is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef PALUDIS_GUARD_SRC_GTKPALUDIS_CELLRENDERERBUTTON_HH
#define PALUDIS_GUARD_SRC_GTKPALUDIS_CELLRENDERERBUTTON_HH 1

#include <gtkmm/cellrenderertext.h>

namespace gtkpaludis
{
    /**
     * A cell renderer for a Gtk::TreeView that displays a button.
     *
     * Based upon gtkmm/examples/cellrenderercustom/cellrenderertoggle.cc
     */
    class CellRendererButton :
        public Gtk::CellRendererText
    {
        protected:
            virtual void get_size_vfunc(Gtk::Widget & widget, const Gdk::Rectangle * cell_area,
                    int * x_offset, int * y_offset, int * width, int * height) const;

            virtual void render_vfunc(const Glib::RefPtr<Gdk::Drawable> & window,
                    Gtk::Widget & widget, const Gdk::Rectangle & background_area,
                    const Gdk::Rectangle & cell_area, const Gdk::Rectangle & expose_area,
                    Gtk::CellRendererState flags);

            virtual bool activate_vfunc(GdkEvent * event, Gtk::Widget & widget,
                    const Glib::ustring & path, const Gdk::Rectangle & background_area,
                    const Gdk::Rectangle & cell_area, Gtk::CellRendererState flags);

        public:
            CellRendererButton();
            virtual ~CellRendererButton();
    };
}

#endif
