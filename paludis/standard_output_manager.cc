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

#include <paludis/standard_output_manager.hh>
#include <paludis/util/set.hh>
#include <iostream>

using namespace paludis;

StandardOutputManager::StandardOutputManager()
{
}

StandardOutputManager::~StandardOutputManager()
{
}

std::ostream &
StandardOutputManager::stdout_stream()
{
    return std::cout;
}

std::ostream &
StandardOutputManager::stderr_stream()
{
    return std::cerr;
}

void
StandardOutputManager::succeeded()
{
}

void
StandardOutputManager::ignore_succeeded()
{
}

void
StandardOutputManager::flush()
{
}

bool
StandardOutputManager::want_to_flush() const
{
    return false;
}

void
StandardOutputManager::nothing_more_to_come()
{
}

void
StandardOutputManager::message(const MessageType, const std::string &)
{
}

const std::shared_ptr<const Set<std::string> >
StandardOutputManager::factory_managers()
{
    std::shared_ptr<Set<std::string> > result(std::make_shared<Set<std::string>>());
    result->insert("standard");
    return result;
}

const std::shared_ptr<OutputManager>
StandardOutputManager::factory_create(
        const OutputManagerFactory::KeyFunction &,
        const OutputManagerFactory::CreateChildFunction &,
        const OutputManagerFactory::ReplaceVarsFunc &)
{
    return std::make_shared<StandardOutputManager>();
}

