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

#include "queue_page.hh"
#include "queue_list.hh"
#include "queue_options.hh"
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>

using namespace gtkpaludis;
using namespace paludis;

namespace paludis
{
    template<>
    struct Implementation<QueuePage> :
        InternalCounted<Implementation<QueuePage> >
    {
        Gtk::ScrolledWindow queue_list_scroll;
        QueueList queue_list;
        QueueOptions queue_options;

        Gtk::VButtonBox queue_buttons;
        Gtk::Button recalculate_button;
        Gtk::Button install_button;
        Gtk::Button clear_button;
        Gtk::Button why_button;

        Implementation(QueuePage * const page) :
            queue_list(page),
            recalculate_button("Recalculate"),
            install_button("Install"),
            clear_button("Clear"),
            why_button("Why?")
        {
        }
    };
}

QueuePage::QueuePage() :
    Gtk::Table(2, 2, false),
    PrivateImplementationPattern<QueuePage>(new Implementation<QueuePage>(this))
{
    _imp->queue_list_scroll.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    _imp->queue_list_scroll.add(_imp->queue_list);
    attach(_imp->queue_list_scroll, 0, 2, 0, 1);

    attach(_imp->queue_options, 0, 1, 1, 2, Gtk::FILL | Gtk::EXPAND, Gtk::AttachOptions(0));

    _imp->queue_buttons.set_border_width(5);
    _imp->queue_buttons.set_spacing(5);
    _imp->queue_buttons.set_layout(Gtk::BUTTONBOX_START);
    _imp->queue_buttons.add(_imp->recalculate_button);
    _imp->queue_buttons.add(_imp->install_button);
    _imp->queue_buttons.add(_imp->clear_button);
    _imp->queue_buttons.add(_imp->why_button);
    attach(_imp->queue_buttons, 1, 2, 1, 2, Gtk::AttachOptions(0), Gtk::AttachOptions(0));

    _imp->queue_list.invalidate();

    _imp->recalculate_button.signal_clicked().connect(sigc::mem_fun(this,
                &QueuePage::_recalculate_button_clicked));
    _imp->clear_button.signal_clicked().connect(sigc::mem_fun(this,
                &QueuePage::_clear_button_clicked));
}

QueuePage::~QueuePage()
{
}

void
QueuePage::clear()
{
    _imp->queue_list.clear();
}

void
QueuePage::add_target(const std::string & s)
{
    _imp->queue_list.add_target(s);
}

void
QueuePage::set_queue_list_calculated(bool b)
{
    _imp->install_button.set_sensitive(b);
    _imp->recalculate_button.set_sensitive(! b);
}

void
QueuePage::_recalculate_button_clicked()
{
    _imp->queue_list.calculate();
}

void
QueuePage::recalculate()
{
    _imp->queue_list.calculate();
}

void
QueuePage::_clear_button_clicked()
{
    clear();
}

