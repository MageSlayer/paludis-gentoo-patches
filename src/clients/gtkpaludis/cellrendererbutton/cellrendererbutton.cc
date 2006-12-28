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
#include <iomanip>
#include <sstream>

using namespace gtkpaludis;

namespace
{
    std::string
    gdk_color_to_colour_string(const Gdk::Color & c)
    {
        std::stringstream rs, gs, bs;
        rs << std::hex << std::setw(4) << std::setfill('0') << c.get_red();
        gs << std::hex << std::setw(4) << std::setfill('0') << c.get_green();
        bs << std::hex << std::setw(4) << std::setfill('0') << c.get_blue();

        return "#" + rs.str() + gs.str() + bs.str();
    }
}

CellRendererButton::CellRendererButton(Gtk::TreeView & owner) :
    Glib::ObjectBase(typeid(CellRendererButton)),
    Gtk::CellRendererText(),
    _owner(&owner),
    _column(0),
    _temporary_highlight_hack(false),
    _property_text_x_pad(*this, "text-x-pad", 4),
    _property_text_y_pad(*this, "text-y-pad", 4)
{
    property_alignment() = Pango::ALIGN_CENTER;
    property_xalign() = 0.5;

    _owner->add_events(Gdk::POINTER_MOTION_MASK | Gdk::BUTTON_MOTION_MASK |
            Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK | Gdk::ENTER_NOTIFY_MASK |
            Gdk::LEAVE_NOTIFY_MASK);

    _owner->signal_button_press_event().connect(sigc::mem_fun(
                this, &CellRendererButton::_button_pressed), false);
    _owner->signal_motion_notify_event().connect(sigc::mem_fun(
                this, &CellRendererButton::_motion_notify), false);
}

CellRendererButton::~CellRendererButton()
{
}

void
CellRendererButton::get_size_vfunc(Gtk::Widget & widget, const Gdk::Rectangle * cell_area,
        int * x_offset, int * y_offset, int * width, int * height) const
{
    Gtk::CellRendererText::get_size_vfunc(widget, cell_area, x_offset, y_offset,
            width, height);

    const unsigned int x_pad = _property_text_x_pad.get_value();
    const unsigned int y_pad = _property_text_y_pad.get_value();
    const float x_align = property_xalign();
    const float y_align = property_yalign();

    const int calculated_width = 2 * x_pad + (width ? *width : 0);
    const int calculated_height = 2 * y_pad + (height ? *height : 0);

    if (width)
        *width = calculated_width;
    if (height)
        *height = calculated_height;

    if (cell_area)
    {
        if (x_offset)
            *x_offset = std::max(0, static_cast<int>(x_align * (cell_area->get_width() - calculated_width)));
        if (y_offset)
            *y_offset = std::max(0, static_cast<int>(y_align * (cell_area->get_height() - calculated_height)));
    }
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

#if 0
    Gtk::StateType state = Gtk::STATE_INSENSITIVE;
    if (flags & Gtk::CELL_RENDERER_SELECTED)
        state = widget.has_focus() ? Gtk::STATE_SELECTED : Gtk::STATE_ACTIVE;
#else
    Gtk::StateType state(_temporary_highlight_hack ? Gtk::STATE_SELECTED : Gtk::STATE_ACTIVE);
#endif

    Glib::RefPtr<Gdk::Window> window_casted = Glib::RefPtr<Gdk::Window>::cast_dynamic<>(window);
    if (window_casted)
    {
        widget.get_style()->paint_box(window_casted, state, Gtk::SHADOW_OUT, cell_area,
                widget, "button", cell_area.get_x() + x_offset + cell_x_pad,
                cell_area.get_y() + y_offset + cell_y_pad, width - 1, height - 1);

        /* using property_attributes() sometimes breaks on redraws */
        widget.ensure_style();
        std::string colour_string(gdk_color_to_colour_string(widget.get_style()->get_fg(state)));
        property_markup() = "<markup><span foreground=\"" + colour_string +
            "\">" + property_text() + "</span></markup>";
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

bool
CellRendererButton::_button_pressed(GdkEventButton * event)
{
    if (! (event->type & GDK_BUTTON_PRESS && event->button == 1))
        return false;

    return true;
}

bool
CellRendererButton::_motion_notify(GdkEventMotion * event)
{
    Gtk::TreeModel::Path path, old_focused_path(_focused_path);
    Gtk::TreeViewColumn * column;
    int cell_x, cell_y;
    if (_owner->get_path_at_pos(static_cast<int>(event->x), static_cast<int>(event->y),
                path, column, cell_x, cell_y)
            && (_column && column == _column))
        _focused_path = path;
    else
        _focused_path = Gtk::TreePath();

    _temporary_highlight_hack = ! _focused_path.empty();

    /* path comparisons are a bit weird */
    if (old_focused_path.empty() && ! _focused_path.empty())
        _owner->queue_draw();
    else if (_focused_path.empty() && ! old_focused_path.empty())
        _owner->queue_draw();
    else if (_focused_path.empty() && old_focused_path.empty())
    {
        /* nothing */
    }
    else if (old_focused_path != _focused_path)
        _owner->queue_draw();
    return false;
}

void
CellRendererButton::set_column(Gtk::TreeViewColumn * const column)
{
    _column = column;
}

