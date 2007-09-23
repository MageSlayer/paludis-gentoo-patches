/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/sequence-impl.hh>

using namespace paludis;

#include <paludis/action-se.cc>
#include <paludis/action-sr.cc>

template class MutableAcceptInterfaceVisitsThis<ActionVisitorTypes, InstallAction>;
template class MutableAcceptInterfaceVisitsThis<ActionVisitorTypes, ConfigAction>;
template class MutableAcceptInterfaceVisitsThis<ActionVisitorTypes, InfoAction>;
template class MutableAcceptInterfaceVisitsThis<ActionVisitorTypes, PretendAction>;
template class MutableAcceptInterfaceVisitsThis<ActionVisitorTypes, InstalledAction>;
template class MutableAcceptInterfaceVisitsThis<ActionVisitorTypes, UninstallAction>;
template class MutableAcceptInterfaceVisitsThis<ActionVisitorTypes, FetchAction>;

template class MutableAcceptInterfaceVisitsThis<SupportsActionTestVisitorTypes, SupportsActionTest<InstallAction> >;
template class MutableAcceptInterfaceVisitsThis<SupportsActionTestVisitorTypes, SupportsActionTest<ConfigAction> >;
template class MutableAcceptInterfaceVisitsThis<SupportsActionTestVisitorTypes, SupportsActionTest<InfoAction> >;
template class MutableAcceptInterfaceVisitsThis<SupportsActionTestVisitorTypes, SupportsActionTest<PretendAction> >;
template class MutableAcceptInterfaceVisitsThis<SupportsActionTestVisitorTypes, SupportsActionTest<InstalledAction> >;
template class MutableAcceptInterfaceVisitsThis<SupportsActionTestVisitorTypes, SupportsActionTest<UninstallAction> >;
template class MutableAcceptInterfaceVisitsThis<SupportsActionTestVisitorTypes, SupportsActionTest<FetchAction> >;

template class Sequence<FetchActionFailure>;

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

const bool
PretendAction::failed() const
{
    return _imp->failed;
}

void
PretendAction::set_failed()
{
    _imp->failed = true;
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
            if (a.options.destination)
                s << &a.options.destination;
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
        const tr1::shared_ptr<const Sequence<FetchActionFailure> > & e) throw () :
    ActionError("Fetch error: " + msg),
    _failures(e)
{
}

FetchActionError::FetchActionError(const std::string & msg) throw () :
    ActionError("Fetch error: " + msg)
{
}

const tr1::shared_ptr<const Sequence<FetchActionFailure> >
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

