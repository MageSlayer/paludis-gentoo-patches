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
#include <ctype.h>

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

namespace
{
    static VALUE c_master_class;
    static VALUE c_name_error;
    static VALUE c_package_dep_atom_error;
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
    else if (0 != dynamic_cast<const paludis::NameError *>(&ee))
        rb_raise(c_name_error, dynamic_cast<const paludis::NameError *>(&ee)->message().c_str());
    else if (0 != dynamic_cast<const paludis::PackageDepAtomError *>(&ee))
        rb_raise(c_package_dep_atom_error, dynamic_cast<const paludis::PackageDepAtomError *>(&ee)->message().c_str());
    else if (0 != dynamic_cast<const paludis::Exception *>(&ee))
        rb_raise(rb_eRuntimeError, "Caught paludis::Exception: %s (%s)",
                dynamic_cast<const paludis::Exception *>(&ee)->message().c_str(), ee.what());
    else
        rb_raise(rb_eRuntimeError, "Unexpected std::exception: (%s)", ee.what());
}

std::string
paludis::ruby::value_case_to_RubyCase(const std::string & s)
{
    if (s.empty())
        return s;

    bool upper_next(true);
    std::string result;
    for (std::string::size_type p(0), p_end(s.length()) ; p != p_end ; ++p)
    {
        if ('_' == s[p])
            upper_next = true;
        else if (upper_next)
        {
            result.append(std::string(1, toupper(s[p])));
            upper_next = false;
        }
        else
            result.append(std::string(1, s[p]));
    }

    return result;
}

VALUE
paludis::ruby::master_class()
{
    return c_master_class;
}

extern "C"
{
    void Init_Paludis()
    {
        c_master_class = rb_define_class("Paludis", rb_cObject);
        c_name_error = rb_define_class_under(c_master_class, "NameError", rb_eRuntimeError);
        c_package_dep_atom_error = rb_define_class_under(c_master_class, "PackageDepAtomError", rb_eRuntimeError);

        rb_define_const(c_master_class, "Version", rb_str_new2((stringify(PALUDIS_VERSION_MAJOR) + "."
                        + stringify(PALUDIS_VERSION_MINOR) + "." + stringify(PALUDIS_VERSION_MICRO)).c_str()));
        RegisterRubyClass::get_instance()->execute();
    }
}

