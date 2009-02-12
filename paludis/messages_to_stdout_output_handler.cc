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

#include <paludis/messages_to_stdout_output_handler.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/discard_output_stream.hh>
#include <paludis/util/sequence-impl.hh>
#include <paludis/util/set.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/exception.hh>
#include <sstream>

using namespace paludis;

namespace paludis
{
    template <>
    struct Implementation<MessagesToStdoutOutputManager>
    {
        DiscardOutputStream output_stream;
        const std::tr1::shared_ptr<OutputManager> child;

        Implementation(const std::tr1::shared_ptr<OutputManager> & c) :
            child(c)
        {
        }
    };
}

MessagesToStdoutOutputManager::MessagesToStdoutOutputManager(
        const std::tr1::shared_ptr<OutputManager> & s) :
    PrivateImplementationPattern<MessagesToStdoutOutputManager>(new Implementation<MessagesToStdoutOutputManager>(s))
{
}

MessagesToStdoutOutputManager::~MessagesToStdoutOutputManager()
{
}

std::ostream &
MessagesToStdoutOutputManager::stdout_stream()
{
    return _imp->output_stream;
}

std::ostream &
MessagesToStdoutOutputManager::stderr_stream()
{
    return _imp->output_stream;
}

void
MessagesToStdoutOutputManager::succeeded()
{
    _imp->child->succeeded();
}

void
MessagesToStdoutOutputManager::message(const MessageType t, const std::string & s)
{
    std::ostringstream m;
    m << t << " " << s << std::endl;
    _imp->child->stdout_stream() << m.str();
}

const std::tr1::shared_ptr<const Set<std::string> >
MessagesToStdoutOutputManager::factory_managers()
{
    std::tr1::shared_ptr<Set<std::string> > result(new Set<std::string>);
    result->insert("messages_to_stdout");
    return result;
}

const std::tr1::shared_ptr<OutputManager>
MessagesToStdoutOutputManager::factory_create(
        const OutputManagerFactory::KeyFunction & key_func,
        const OutputManagerFactory::CreateChildFunction & create_child,
        const OutputManagerFactory::ReplaceVarsFunc &)
{
    std::string child(key_func("child"));
    if (child.empty())
        throw ConfigurationError("No child specified for MessagesToStdoutOutputManager");

    return make_shared_ptr(new MessagesToStdoutOutputManager(create_child(child)));
}

template class PrivateImplementationPattern<MessagesToStdoutOutputManager>;

