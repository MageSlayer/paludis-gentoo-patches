/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009 Ciaran McCreesh
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
#include <iostream>

using namespace paludis;
using namespace cave;

DisplayCallback::DisplayCallback() :
    width(0),
    metadata(0),
    steps(0)
{
    std::cout << "Resolving: " << std::flush;
}

DisplayCallback::~DisplayCallback()
{
    std::cout << std::endl << std::endl;
}

void
DisplayCallback::operator() (const NotifierCallbackEvent & event) const
{
    event.accept(*this);
}

void
DisplayCallback::visit(const NotifierCallbackGeneratingMetadataEvent &) const
{
    Lock lock(mutex);
    ++metadata;
    update();
}

void
DisplayCallback::visit(const NotifierCallbackResolverStepEvent &) const
{
    Lock lock(mutex);
    ++steps;
    update();
}

void
DisplayCallback::update() const
{
    std::string s;
    if (0 != steps)
        s.append("steps: " + stringify(steps));

    if (0 != metadata)
    {
        if (! s.empty())
            s.append(", ");
        s.append("metadata: " + stringify(metadata) + " ");
    }

    std::cout << std::string(width, '\010') << s << std::flush;
    width = s.length();
}

