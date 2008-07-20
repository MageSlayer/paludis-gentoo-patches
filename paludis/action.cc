/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008 Ciaran McCreesh
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
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/sequence-impl.hh>
#include <paludis/util/kc.hh>

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

UninstallAction::UninstallAction()
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

        Implementation() :
            failed(false)
        {
        }
    };
}

PretendAction::PretendAction() :
    PrivateImplementationPattern<PretendAction>(new Implementation<PretendAction>)
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

SupportsActionTestBase::~SupportsActionTestBase()
{
}

UnsupportedActionError::UnsupportedActionError(const PackageID & id, const Action & a) throw () :
    ActionError("Unsupported action '" + stringify(a) + "' on '" + stringify(id) + "'")
{
}

namespace
{
    struct ActionStringifier :
        ConstVisitor<ActionVisitorTypes>
    {
        std::ostream & s;

        ActionStringifier(std::ostream & ss) :
            s(ss)
        {
        }

        void visit(const InstallAction & a)
        {
            s << "install to ";
            if (a.options[k::destination()])
                s << a.options[k::destination()]->name();
            else
                s << "nowhere";
        }

        void visit(const UninstallAction &)
        {
            s << "uninstall";
        }

        void visit(const PretendAction &)
        {
            s << "pretend";
        }

        void visit(const InstalledAction &)
        {
            s << "installed";
        }

        void visit(const ConfigAction &)
        {
            s << "config";
        }

        void visit(const InfoAction &)
        {
            s << "info";
        }

        void visit(const FetchAction &)
        {
            s << "fetch";
        }

        void visit(const PretendFetchAction &)
        {
            s << "pretend fetch";
        }
    };
}

std::ostream &
paludis::operator<< (std::ostream & s, const Action & a)
{
    ActionStringifier t(s);
    a.accept(t);
    return s;
}

ActionError::ActionError(const std::string & msg) throw () :
    Exception(msg)
{
}

InstallActionError::InstallActionError(const std::string & msg) throw () :
    ActionError("Install error: " + msg)
{
}

FetchActionError::FetchActionError(const std::string & msg,
        const std::tr1::shared_ptr<const Sequence<FetchActionFailure> > & e) throw () :
    ActionError("Fetch error: " + msg),
    _failures(e)
{
}

FetchActionError::FetchActionError(const std::string & msg) throw () :
    ActionError("Fetch error: " + msg)
{
}

FetchActionError::~FetchActionError() throw ()
{
}

const std::tr1::shared_ptr<const Sequence<FetchActionFailure> >
FetchActionError::failures() const
{
    return _failures;
}

UninstallActionError::UninstallActionError(const std::string & msg) throw () :
    ActionError("Uninstall error: " + msg)
{
}

ConfigActionError::ConfigActionError(const std::string & msg) throw () :
    ActionError("Configuration error: " + msg)
{
}

InfoActionError::InfoActionError(const std::string & msg) throw () :
    ActionError("Info error: " + msg)
{
}

template class AcceptInterface<ActionVisitorTypes>;
template class AcceptInterface<SupportsActionTestVisitorTypes>;

template class AcceptInterfaceVisitsThis<ActionVisitorTypes, ConfigAction>;
template class AcceptInterfaceVisitsThis<ActionVisitorTypes, FetchAction>;
template class AcceptInterfaceVisitsThis<ActionVisitorTypes, InfoAction>;
template class AcceptInterfaceVisitsThis<ActionVisitorTypes, InstallAction>;
template class AcceptInterfaceVisitsThis<ActionVisitorTypes, InstalledAction>;
template class AcceptInterfaceVisitsThis<ActionVisitorTypes, PretendAction>;
template class AcceptInterfaceVisitsThis<ActionVisitorTypes, PretendFetchAction>;
template class AcceptInterfaceVisitsThis<ActionVisitorTypes, UninstallAction>;

template class Visits<ConfigAction>;
template class Visits<FetchAction>;
template class Visits<InfoAction>;
template class Visits<InstallAction>;
template class Visits<InstalledAction>;
template class Visits<PretendAction>;
template class Visits<PretendFetchAction>;
template class Visits<UninstallAction>;

template class AcceptInterfaceVisitsThis<SupportsActionTestVisitorTypes, SupportsActionTest<ConfigAction> >;
template class AcceptInterfaceVisitsThis<SupportsActionTestVisitorTypes, SupportsActionTest<FetchAction> >;
template class AcceptInterfaceVisitsThis<SupportsActionTestVisitorTypes, SupportsActionTest<InfoAction> >;
template class AcceptInterfaceVisitsThis<SupportsActionTestVisitorTypes, SupportsActionTest<InstallAction> >;
template class AcceptInterfaceVisitsThis<SupportsActionTestVisitorTypes, SupportsActionTest<InstalledAction> >;
template class AcceptInterfaceVisitsThis<SupportsActionTestVisitorTypes, SupportsActionTest<PretendAction> >;
template class AcceptInterfaceVisitsThis<SupportsActionTestVisitorTypes, SupportsActionTest<PretendFetchAction> >;
template class AcceptInterfaceVisitsThis<SupportsActionTestVisitorTypes, SupportsActionTest<UninstallAction> >;

template class Visits<SupportsActionTest<ConfigAction> >;
template class Visits<SupportsActionTest<FetchAction> >;
template class Visits<SupportsActionTest<InfoAction> >;
template class Visits<SupportsActionTest<InstallAction> >;
template class Visits<SupportsActionTest<InstalledAction> >;
template class Visits<SupportsActionTest<PretendAction> >;
template class Visits<SupportsActionTest<PretendFetchAction> >;
template class Visits<SupportsActionTest<UninstallAction> >;

template class PrivateImplementationPattern<FetchAction>;
template class PrivateImplementationPattern<InstallAction>;
template class PrivateImplementationPattern<PretendAction>;
template class PrivateImplementationPattern<PretendFetchAction>;

template class Sequence<FetchActionFailure>;

