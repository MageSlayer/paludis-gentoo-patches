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

#include <paludis/tee_output_manager.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/tee_output_stream.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/set.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <vector>
#include <algorithm>

using namespace paludis;

namespace paludis
{
    template <>
    struct Imp<TeeOutputManager>
    {
        const std::shared_ptr<const Sequence<std::shared_ptr<OutputManager> > > streams;
        const std::shared_ptr<const Sequence<std::shared_ptr<OutputManager> > > messages_streams;
        const std::shared_ptr<const Sequence<std::shared_ptr<OutputManager> > > stdout_streams;
        const std::shared_ptr<const Sequence<std::shared_ptr<OutputManager> > > stderr_streams;

        TeeOutputStream stdout_stream;
        TeeOutputStream stderr_stream;

        Imp(
                const std::shared_ptr<const Sequence<std::shared_ptr<OutputManager> > > & s,
                const std::shared_ptr<const Sequence<std::shared_ptr<OutputManager> > > & ss,
                const std::shared_ptr<const Sequence<std::shared_ptr<OutputManager> > > & so,
                const std::shared_ptr<const Sequence<std::shared_ptr<OutputManager> > > & se) :
            streams(s),
            messages_streams(ss),
            stdout_streams(so),
            stderr_streams(se)
        {
        }
    };
}

TeeOutputManager::TeeOutputManager(
        const std::shared_ptr<const OutputManagerSequence> & s,
        const std::shared_ptr<const OutputManagerSequence> & ss) :
    _imp(s, ss, make_null_shared_ptr(), make_null_shared_ptr())
{
    for (OutputManagerSequence::ConstIterator i(_imp->streams->begin()), i_end(_imp->streams->end()) ;
            i != i_end ; ++i)
    {
        _imp->stdout_stream.add_stream(&(*i)->stdout_stream());
        _imp->stderr_stream.add_stream(&(*i)->stderr_stream());
    }
}

TeeOutputManager::TeeOutputManager(
        const std::shared_ptr<const OutputManagerSequence> & s,
        const std::shared_ptr<const OutputManagerSequence> & ss,
        const std::shared_ptr<const OutputManagerSequence> & so,
        const std::shared_ptr<const OutputManagerSequence> & se) :
    _imp(s, ss, so, se)
{
    for (OutputManagerSequence::ConstIterator i(_imp->streams->begin()), i_end(_imp->streams->end()) ;
            i != i_end ; ++i)
    {
        _imp->stdout_stream.add_stream(&(*i)->stdout_stream());
        _imp->stderr_stream.add_stream(&(*i)->stderr_stream());
    }

    for (OutputManagerSequence::ConstIterator i(_imp->stdout_streams->begin()), i_end(_imp->stdout_streams->end()) ;
            i != i_end ; ++i)
        _imp->stdout_stream.add_stream(&(*i)->stdout_stream());

    for (OutputManagerSequence::ConstIterator i(_imp->stderr_streams->begin()), i_end(_imp->stderr_streams->end()) ;
            i != i_end ; ++i)
        _imp->stderr_stream.add_stream(&(*i)->stderr_stream());
}

TeeOutputManager::~TeeOutputManager()
{
}

std::ostream &
TeeOutputManager::stdout_stream()
{
    return _imp->stdout_stream;
}

std::ostream &
TeeOutputManager::stderr_stream()
{
    return _imp->stderr_stream;
}

void
TeeOutputManager::message(const MessageType t, const std::string & m)
{
    for (OutputManagerSequence::ConstIterator i(_imp->messages_streams->begin()), i_end(_imp->messages_streams->end()) ;
            i != i_end ; ++i)
        (*i)->message(t, m);
}

void
TeeOutputManager::succeeded()
{
    for (OutputManagerSequence::ConstIterator i(_imp->streams->begin()), i_end(_imp->streams->end()) ;
            i != i_end ; ++i)
        (*i)->succeeded();

    for (OutputManagerSequence::ConstIterator i(_imp->messages_streams->begin()), i_end(_imp->messages_streams->end()) ;
            i != i_end ; ++i)
        (*i)->succeeded();

    for (OutputManagerSequence::ConstIterator i(_imp->stdout_streams->begin()), i_end(_imp->stdout_streams->end()) ;
            i != i_end ; ++i)
        (*i)->succeeded();

    for (OutputManagerSequence::ConstIterator i(_imp->stderr_streams->begin()), i_end(_imp->stderr_streams->end()) ;
            i != i_end ; ++i)
        (*i)->succeeded();
}

void
TeeOutputManager::flush()
{
    for (OutputManagerSequence::ConstIterator i(_imp->streams->begin()), i_end(_imp->streams->end()) ;
            i != i_end ; ++i)
        (*i)->flush();

    for (OutputManagerSequence::ConstIterator i(_imp->messages_streams->begin()), i_end(_imp->messages_streams->end()) ;
            i != i_end ; ++i)
        (*i)->flush();

    for (OutputManagerSequence::ConstIterator i(_imp->stdout_streams->begin()), i_end(_imp->stdout_streams->end()) ;
            i != i_end ; ++i)
        (*i)->flush();

    for (OutputManagerSequence::ConstIterator i(_imp->stderr_streams->begin()), i_end(_imp->stderr_streams->end()) ;
            i != i_end ; ++i)
        (*i)->flush();
}

bool
TeeOutputManager::want_to_flush() const
{
    return indirect_iterator(_imp->streams->end()) != std::find_if(
                indirect_iterator(_imp->streams->begin()), indirect_iterator(_imp->streams->end()),
                std::bind(&OutputManager::want_to_flush, std::placeholders::_1)) ||
        indirect_iterator(_imp->messages_streams->end()) != std::find_if(
                indirect_iterator(_imp->messages_streams->begin()), indirect_iterator(_imp->messages_streams->end()),
                std::bind(&OutputManager::want_to_flush, std::placeholders::_1)) ||
        indirect_iterator(_imp->stdout_streams->end()) != std::find_if(
                indirect_iterator(_imp->stdout_streams->begin()), indirect_iterator(_imp->stdout_streams->end()),
                std::bind(&OutputManager::want_to_flush, std::placeholders::_1)) ||
        indirect_iterator(_imp->stderr_streams->end()) != std::find_if(
                indirect_iterator(_imp->stderr_streams->begin()), indirect_iterator(_imp->stderr_streams->end()),
                std::bind(&OutputManager::want_to_flush, std::placeholders::_1));
}

void
TeeOutputManager::nothing_more_to_come()
{
    for (OutputManagerSequence::ConstIterator i(_imp->streams->begin()), i_end(_imp->streams->end()) ;
            i != i_end ; ++i)
        (*i)->nothing_more_to_come();

    for (OutputManagerSequence::ConstIterator i(_imp->messages_streams->begin()), i_end(_imp->messages_streams->end()) ;
            i != i_end ; ++i)
        (*i)->nothing_more_to_come();

    for (OutputManagerSequence::ConstIterator i(_imp->stdout_streams->begin()), i_end(_imp->stdout_streams->end()) ;
            i != i_end ; ++i)
        (*i)->nothing_more_to_come();

    for (OutputManagerSequence::ConstIterator i(_imp->stderr_streams->begin()), i_end(_imp->stderr_streams->end()) ;
            i != i_end ; ++i)
        (*i)->nothing_more_to_come();
}

const std::shared_ptr<const Set<std::string> >
TeeOutputManager::factory_managers()
{
    std::shared_ptr<Set<std::string> > result(std::make_shared<Set<std::string>>());
    result->insert("tee");
    return result;
}

const std::shared_ptr<OutputManager>
TeeOutputManager::factory_create(
        const OutputManagerFactory::KeyFunction & key_func,
        const OutputManagerFactory::CreateChildFunction & create_child,
        const OutputManagerFactory::ReplaceVarsFunc &)
{
    std::shared_ptr<OutputManagerSequence> children(std::make_shared<OutputManagerSequence>());
    std::shared_ptr<OutputManagerSequence> messages_children(std::make_shared<OutputManagerSequence>());
    std::shared_ptr<OutputManagerSequence> stdout_children(std::make_shared<OutputManagerSequence>());
    std::shared_ptr<OutputManagerSequence> stderr_children(std::make_shared<OutputManagerSequence>());

    std::vector<std::string> children_keys;
    tokenise_whitespace(key_func("children"), std::back_inserter(children_keys));

    std::vector<std::string> messages_children_keys;
    tokenise_whitespace(key_func("messages_children"), std::back_inserter(messages_children_keys));

    std::vector<std::string> stdout_children_keys;
    tokenise_whitespace(key_func("stdout_children"), std::back_inserter(stdout_children_keys));

    std::vector<std::string> stderr_children_keys;
    tokenise_whitespace(key_func("stderr_children"), std::back_inserter(stderr_children_keys));

    for (std::vector<std::string>::const_iterator c(children_keys.begin()), c_end(children_keys.end()) ;
            c != c_end ; ++c)
        children->push_back(create_child(*c));

    for (std::vector<std::string>::const_iterator c(messages_children_keys.begin()), c_end(messages_children_keys.end()) ;
            c != c_end ; ++c)
        messages_children->push_back(create_child(*c));

    for (std::vector<std::string>::const_iterator c(stdout_children_keys.begin()), c_end(stdout_children_keys.end()) ;
            c != c_end ; ++c)
        stdout_children->push_back(create_child(*c));

    for (std::vector<std::string>::const_iterator c(stderr_children_keys.begin()), c_end(stderr_children_keys.end()) ;
            c != c_end ; ++c)
        stderr_children->push_back(create_child(*c));


    return std::make_shared<TeeOutputManager>(children, messages_children, stdout_children, stderr_children);
}

template class Pimp<TeeOutputManager>;

