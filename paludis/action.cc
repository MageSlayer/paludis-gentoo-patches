/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009 Ciaran McCreesh
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

#include <paludis/action.hh>
#include <paludis/repository.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/sequence-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <ostream>

using namespace paludis;

#include <paludis/action-se.cc>

Action::~Action()
{
}

namespace paludis
{
    template <>
    struct Implementation<InstallAction>
    {
        const InstallActionOptions options;

        Implementation(const InstallActionOptions & o) :
            options(o)
        {
        }
    };
}

InstallAction::InstallAction(const InstallActionOptions & o) :
    PrivateImplementationPattern<InstallAction>(new Implementation<InstallAction>(o)),
    options(_imp->options)
{
}

InstallAction::~InstallAction()
{
}

namespace paludis
{
    template <>
    struct Implementation<FetchAction>
    {
        const FetchActionOptions options;

        Implementation(const FetchActionOptions & o) :
            options(o)
        {
        }
    };
}

FetchAction::FetchAction(const FetchActionOptions & o) :
    PrivateImplementationPattern<FetchAction>(new Implementation<FetchAction>(o)),
    options(_imp->options)
{
}

FetchAction::~FetchAction()
{
}

namespace paludis
{
    template <>
    struct Implementation<UninstallAction>
    {
        const UninstallActionOptions options;

        Implementation(const UninstallActionOptions & o) :
            options(o)
        {
        }
    };
}

UninstallAction::UninstallAction(const UninstallActionOptions & o) :
    PrivateImplementationPattern<UninstallAction>(new Implementation<UninstallAction>(o)),
    options(_imp->options)
{
}

UninstallAction::~UninstallAction()
{
}

namespace paludis
{
    template <>
    struct Implementation<PretendAction>
    {
        bool failed;
        const PretendActionOptions options;

        Implementation(const PretendActionOptions & o) :
            failed(false),
            options(o)
        {
        }
    };
}

PretendAction::PretendAction(const PretendActionOptions & o) :
    PrivateImplementationPattern<PretendAction>(new Implementation<PretendAction>(o)),
    options(_imp->options)
{
}

PretendAction::~PretendAction()
{
}

bool
PretendAction::failed() const
{
    return _imp->failed;
}

void
PretendAction::set_failed()
{
    _imp->failed = true;
}

namespace paludis
{
    template <>
    struct Implementation<PretendFetchAction>
    {
        const FetchActionOptions options;

        Implementation(const FetchActionOptions & o) :
            options(o)
        {
        }
    };
}

PretendFetchAction::PretendFetchAction(const FetchActionOptions & o) :
    PrivateImplementationPattern<PretendFetchAction>(new Implementation<PretendFetchAction>(o)),
    options(_imp->options)
{
}

PretendFetchAction::~PretendFetchAction()
{
}

namespace paludis
{
    template <>
    struct Implementation<InfoAction>
    {
        const InfoActionOptions options;

        Implementation(const InfoActionOptions & o) :
            options(o)
        {
        }
    };
}

InfoAction::InfoAction(const InfoActionOptions & o) :
    PrivateImplementationPattern<InfoAction>(new Implementation<InfoAction>(o)),
    options(_imp->options)
{
}

InfoAction::~InfoAction()
{
}

namespace paludis
{
    template <>
    struct Implementation<ConfigAction>
    {
        const ConfigActionOptions options;

        Implementation(const ConfigActionOptions & o) :
            options(o)
        {
        }
    };
}

ConfigAction::ConfigAction(const ConfigActionOptions & o) :
    PrivateImplementationPattern<ConfigAction>(new Implementation<ConfigAction>(o)),
    options(_imp->options)
{
}

ConfigAction::~ConfigAction()
{
}

SupportsActionTestBase::~SupportsActionTestBase()
{
}

namespace
{
    struct ActionStringifier
    {
        std::string visit(const InstallAction & a)
        {
            std::string s("install to ");
            if (a.options.destination())
                s.append(stringify(a.options.destination()->name()));
            else
                s.append("nowhere");
            return s;
        }

        std::string visit(const UninstallAction &)
        {
            return "uninstall";
        }

        std::string visit(const PretendAction &)
        {
            return "pretend";
        }

        std::string visit(const ConfigAction &)
        {
            return "config";
        }

        std::string visit(const InfoAction &)
        {
            return "info";
        }

        std::string visit(const FetchAction &)
        {
            return "fetch";
        }

        std::string visit(const PretendFetchAction &)
        {
            return "pretend fetch";
        }
    };
}

std::ostream &
paludis::operator<< (std::ostream & s, const Action & a)
{
    ActionStringifier t;
    return s << a.accept_returning<std::string>(t);
}

ActionFailedError::ActionFailedError(const std::string & msg) throw () :
    Exception(msg)
{
}

ActionAbortedError::ActionAbortedError(const std::string & msg) throw () :
    Exception(msg)
{
}

template class PrivateImplementationPattern<FetchAction>;
template class PrivateImplementationPattern<InstallAction>;
template class PrivateImplementationPattern<PretendAction>;
template class PrivateImplementationPattern<PretendFetchAction>;
template class PrivateImplementationPattern<UninstallAction>;
template class PrivateImplementationPattern<InfoAction>;
template class PrivateImplementationPattern<ConfigAction>;

template class Sequence<FetchActionFailure>;
template class WrappedForwardIterator<Sequence<FetchActionFailure>::ConstIteratorTag, const FetchActionFailure>;

