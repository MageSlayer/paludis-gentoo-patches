/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010, 2011 Ciaran McCreesh
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

#include "cmd_resolve_display_callback.hh"
#include <paludis/notifier_callback.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/fs_stat.hh>
#include <iostream>
#include <map>
#include <unistd.h>

using namespace paludis;
using namespace cave;

namespace paludis
{
    template <>
    struct Imp<DisplayCallback>
    {
        mutable Mutex mutex;
        mutable std::map<std::string, int> metadata, steps;
        mutable std::string stage;
        mutable unsigned width;

        bool output;

        Imp(const std::string & s) :
            stage(s),
            width(stage.length()),
            output(::isatty(1))
        {
        }
    };
}


DisplayCallback::DisplayCallback(const std::string & s) :
    _imp(s)
{
    if (_imp->output)
        std::cout << _imp->stage << std::flush;
}

DisplayCallback::~DisplayCallback()
{
    if (_imp->output)
        std::cout << std::endl << std::endl;
}

void
DisplayCallback::operator() (const NotifierCallbackEvent & event) const
{
    if (! _imp->output)
        return;

    event.accept(*this);
}

void
DisplayCallback::operator() (const ResolverRestart &) const
{
    if (! _imp->output)
        return;

    Lock lock(_imp->mutex);
    ++_imp->steps.insert(std::make_pair("restarts", 0)).first->second;
    update();
}

void
DisplayCallback::visit(const NotifierCallbackGeneratingMetadataEvent & e) const
{
    if (! _imp->output)
        return;

    Lock lock(_imp->mutex);
    ++_imp->metadata.insert(std::make_pair(stringify(e.repository()), 0)).first->second;
    update();
}

void
DisplayCallback::visit(const NotifierCallbackResolverStageEvent & e) const
{
    if (! _imp->output)
        return;

    Lock lock(_imp->mutex);
    _imp->stage = e.stage() + ": ";
    update();
}

void
DisplayCallback::visit(const NotifierCallbackResolverStepEvent &) const
{
    if (! _imp->output)
        return;

    Lock lock(_imp->mutex);
    ++_imp->steps.insert(std::make_pair("steps", 0)).first->second;
    update();
}

void
DisplayCallback::visit(const NotifierCallbackLinkageStepEvent & e) const
{
    if (! _imp->output)
        return;

    Lock lock(_imp->mutex);
    if (e.location().stat().is_directory_or_symlink_to_directory())
        ++_imp->steps.insert(std::make_pair("directories", 0)).first->second;
    else
        ++_imp->steps.insert(std::make_pair("files", 0)).first->second;
    update();
}

void
DisplayCallback::update() const
{
    if (! _imp->output)
        return;

    std::string s(_imp->stage);
    bool first(true);

    if (! _imp->steps.empty())
    {
        for (std::map<std::string, int>::const_iterator i(_imp->steps.begin()), i_end(_imp->steps.end()) ;
                i != i_end ; ++i)
        {
            if (! first)
                s.append(", ");
            first = false;

            s.append(stringify(i->second) + " " + i->first);
        }
    }

    if (! _imp->metadata.empty())
    {
        std::multimap<int, std::string> biggest;
        for (std::map<std::string, int>::const_iterator i(_imp->metadata.begin()), i_end(_imp->metadata.end()) ;
                i != i_end ; ++i)
            biggest.insert(std::make_pair(i->second, i->first));

        int t(0), n(0);
        std::string ss;
        for (std::multimap<int, std::string>::const_reverse_iterator i(biggest.rbegin()), i_end(biggest.rend()) ;
                i != i_end ; ++i)
        {
            ++n;

            if (n == 4)
                ss.append(", ...");

            if (n < 4)
            {
                if (! ss.empty())
                    ss.append(", ");

                ss.append(stringify(i->first) + " " + i->second);
            }

            t += i->first;
        }

        if (! s.empty())
            s.append(", ");
        s.append(stringify(t) + " metadata (" + ss + ") ");
    }

    std::cout << std::string(_imp->width, '\010') << s;

    if (_imp->width > s.length())
        std::cout
            << std::string(_imp->width - s.length(), ' ')
            << std::string(_imp->width - s.length(), '\010');

    _imp->width = s.length();
    std::cout << std::flush;
}

