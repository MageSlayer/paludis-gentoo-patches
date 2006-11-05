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

#include "cellrendererbutton.hh"
#include <gtkmm/enums.h>

using namespace gtkpaludis;

CellRendererButton::CellRendererButton() :
    Glib::ObjectBase(typeid(CellRendererButton)),
    Gtk::CellRendererText()
{
    property_alignment() = Pango::ALIGN_CENTER;
    property_xalign() = 0.5;
}

CellRendererButton::~CellRendererButton()
{
}

void
CellRendererButton::get_size_vfunc(Gtk::Widget & widget, const Gdk::Rectangle * cell_area,
        int * x_offset, int * y_offset, int * width, int * height) const
{
#if 0
    const unsigned int x_pad = property_xpad();
    const unsigned int y_pad = property_ypad();
    const unsigned int x_align = property_xalign();
    const unsigned int y_align = property_yalign();

    const int calculated_width = 2 * x_pad + _button_width;
    const int calculated_height = 2 * y_pad + _button_height;

    if (width)
        *width = calculated_width;
    if (height)
        *height = calculated_height;

    if (cell_area)
    {
        if (x_offset)
            *x_offset = std::max<int>(0, x_align * (cell_area->get_width() - calculated_width));
        if (y_offset)
            *y_offset = std::max<int>(0, y_align * (cell_area->get_height() - calculated_height));
    }
#else
    Gtk::CellRendererText::get_size_vfunc(widget, cell_area, x_offset, y_offset,
            width, height);
    if (width)
        *width += 10;
    if (height)
        *height += 6;
    if (x_offset)
        *x_offset = 0;
    if (y_offset)
        *y_offset = 0;
#endif
}

void
CellRendererButton::render_vfunc(const Glib::RefPtr<Gdk::Drawable> & window,
        Gtk::Widget & widget, const Gdk::Rectangle & r,
        const Gdk::Rectangle & cell_area, const Gdk::Rectangle & rr,
        Gtk::CellRendererState flags)
{
    const unsigned int cell_x_pad = property_xpad();
    const unsigned int cell_y_pad = property_ypad();

    int x_offset = 0, y_offset = 0, width = 0, height = 0;
    get_size(widget, cell_area, x_offset, y_offset, width, height);

    width -= 2 * cell_x_pad;
    height -= 2 * cell_y_pad;

    if (width <= 0 || height <= 0)
        return;

    Gtk::StateType state = Gtk::STATE_INSENSITIVE;
    if (flags & Gtk::CELL_RENDERER_SELECTED)
        state = widget.has_focus() ? Gtk::STATE_SELECTED : Gtk::STATE_ACTIVE;

    const Gtk::ShadowType shadow = Gtk::SHADOW_OUT;

    Glib::RefPtr<Gdk::Window> window_casted = Glib::RefPtr<Gdk::Window>::cast_dynamic<>(window);
    if (window_casted)
    {
        widget.get_style()->paint_box(window_casted, state, shadow, cell_area,
                widget, "button", cell_area.get_x() + x_offset + cell_x_pad,
                cell_area.get_y() + y_offset + cell_y_pad, width - 1, height - 1);

        CellRendererText::render_vfunc(window, widget, r, cell_area, rr, flags);
    }
}

bool
CellRendererButton::activate_vfunc(GdkEvent *, Gtk::Widget &,
        const Glib::ustring &, const Gdk::Rectangle &,
        const Gdk::Rectangle &, Gtk::CellRendererState)
{
    return false;
}

