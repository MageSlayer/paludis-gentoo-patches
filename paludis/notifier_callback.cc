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

#include <paludis/notifier_callback.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/environment.hh>

using namespace paludis;

NotifierCallbackGeneratingMetadataEvent::NotifierCallbackGeneratingMetadataEvent(const RepositoryName & r) :
    _repo(r)
{
}

const RepositoryName
NotifierCallbackGeneratingMetadataEvent::repository() const
{
    return _repo;
}

NotifierCallbackResolverStageEvent::NotifierCallbackResolverStageEvent(const std::string & r) :
    _stage(r)
{
}

const std::string
NotifierCallbackResolverStageEvent::stage() const
{
    return _stage;
}

namespace paludis
{
    template <>
    struct Implementation<ScopedNotifierCallback>
    {
        Environment * const env;
        const NotifierCallbackID id;
        bool removed;

        Implementation(Environment * const e, const NotifierCallbackFunction & f) :
            env(e),
            id(e->add_notifier_callback(f)),
            removed(false)
        {
        }
    };
}

ScopedNotifierCallback::ScopedNotifierCallback(Environment * const e, const NotifierCallbackFunction & f) :
    PrivateImplementationPattern<ScopedNotifierCallback>(new Implementation<ScopedNotifierCallback>(e, f))
{
}

ScopedNotifierCallback::~ScopedNotifierCallback()
{
    if (! _imp->removed)
        remove_now();
}

void
ScopedNotifierCallback::remove_now()
{
    if (_imp->removed)
        throw InternalError(PALUDIS_HERE, "already removed");

    _imp->env->remove_notifier_callback(_imp->id);
    _imp->removed = true;
}

template class PrivateImplementationPattern<ScopedNotifierCallback>;

