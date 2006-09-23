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

#include <paludis/paludis.hh>
#include <paludis_ruby.hh>
#include <ruby.h>
#include <list>

using namespace paludis;
using namespace paludis::ruby;

namespace paludis
{
    template<>
    struct Implementation<RegisterRubyClass> :
        InternalCounted<Implementation<RegisterRubyClass> >
    {
        std::list<void (*)()> funcs;
    };
}

RegisterRubyClass::RegisterRubyClass() :
    PrivateImplementationPattern<RegisterRubyClass>(new Implementation<RegisterRubyClass>)
{
}

RegisterRubyClass::~RegisterRubyClass()
{
}

void
RegisterRubyClass::execute() const
{
    for (std::list<void (*)()>::const_iterator f(_imp->funcs.begin()), f_end(_imp->funcs.end()) ;
            f != f_end ; ++f)
        (*f)();
}

RegisterRubyClass::Register::Register(void (* f)())
{
    RegisterRubyClass::get_instance()->_imp->funcs.push_back(f);
}

void paludis::ruby::exception_to_ruby_exception(const std::exception & ee)
{
    if (0 != dynamic_cast<const paludis::InternalError *>(&ee))
        rb_raise(rb_eRuntimeError, "Unexpected paludis::InternalError: %s (%s)",
                dynamic_cast<const paludis::InternalError *>(&ee)->message().c_str(), ee.what());
    else if (0 != dynamic_cast<const paludis::Exception *>(&ee))
        rb_raise(rb_eRuntimeError, "Caught paludis::Exception: %s (%s)",
                dynamic_cast<const paludis::Exception *>(&ee)->message().c_str(), ee.what());
    else
        rb_raise(rb_eRuntimeError, "Unexpected std::exception: (%s)", ee.what());
}

extern "C"
{
    void Init_Paludis()
    {
        RegisterRubyClass::get_instance()->execute();
    }
}

