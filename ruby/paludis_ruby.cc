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
#include <paludis/config_file.hh>
#include <ruby.h>
#include <list>
#include <ctype.h>

#ifdef ENABLE_RUBY_QA
#include <paludis/qa/qa_environment.hh>
#endif

using namespace paludis;
using namespace paludis::ruby;

#define RUBY_FUNC_CAST(x) reinterpret_cast<VALUE (*)(...)>(x)

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
    static VALUE c_paludis_module;
    static VALUE c_name_error;
    static VALUE c_set_name_error;
    static VALUE c_category_name_part_error;
    static VALUE c_package_name_part_error;
    static VALUE c_bad_version_spec_error;
    static VALUE c_package_dep_atom_error;
    static VALUE c_package_database_error;
    static VALUE c_package_database_lookup_error;
    static VALUE c_ambiguous_package_name_error;
    static VALUE c_no_such_package_error;
    static VALUE c_no_such_repository_error;
    static VALUE c_dep_string_error;
    static VALUE c_dep_string_parse_error;
    static VALUE c_dep_string_nesting_error;
    static VALUE c_configuration_error;
    static VALUE c_config_file_error;

    static VALUE c_environment;

#ifdef ENABLE_RUBY_QA
    static VALUE c_paludis_qa_module;
    static VALUE c_profiles_desc_error;
    static VALUE c_no_such_file_check_type_error;
    static VALUE c_no_such_package_dir_check_type_error;
    static VALUE c_no_such_ebuild_check_type_error;
#endif

    VALUE paludis_match_package(VALUE, VALUE en, VALUE a, VALUE t)
    {
        try
        {
            Environment * env = value_to_environment_data(en)->env_ptr;
            PackageDepAtom::ConstPointer atom = value_to_package_dep_atom(a);
            PackageDatabaseEntry target = value_to_package_database_entry(t);
            return match_package(env, atom, target) ? Qtrue : Qfalse;
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }

    }
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
    else if (0 != dynamic_cast<const paludis::BadVersionSpecError *>(&ee))
        rb_raise(c_bad_version_spec_error, dynamic_cast<const paludis::BadVersionSpecError *>(&ee)->message().c_str());
    else if (0 != dynamic_cast<const paludis::SetNameError *>(&ee))
        rb_raise(c_set_name_error, dynamic_cast<const paludis::SetNameError *>(&ee)->message().c_str());
    else if (0 != dynamic_cast<const paludis::PackageNamePartError *>(&ee))
        rb_raise(c_package_name_part_error, dynamic_cast<const paludis::PackageNamePartError *>(&ee)->message().c_str());
    else if (0 != dynamic_cast<const paludis::CategoryNamePartError *>(&ee))
        rb_raise(c_category_name_part_error, dynamic_cast<const paludis::CategoryNamePartError *>(&ee)->message().c_str());
    else if (0 != dynamic_cast<const paludis::NameError *>(&ee))
        rb_raise(c_name_error, dynamic_cast<const paludis::NameError *>(&ee)->message().c_str());
    else if (0 != dynamic_cast<const paludis::PackageDepAtomError *>(&ee))
        rb_raise(c_package_dep_atom_error, dynamic_cast<const paludis::PackageDepAtomError *>(&ee)->message().c_str());
    else if (0 != dynamic_cast<const paludis::NoSuchRepositoryError *>(&ee))
        rb_raise(c_no_such_repository_error, dynamic_cast<const paludis::NoSuchRepositoryError *>(&ee)->message().c_str());
    else if (0 != dynamic_cast<const paludis::AmbiguousPackageNameError *>(&ee))
        rb_raise(c_ambiguous_package_name_error, dynamic_cast<const paludis::AmbiguousPackageNameError *>(&ee)->message().c_str());
    else if (0 != dynamic_cast<const paludis::NoSuchPackageError *>(&ee))
        rb_raise(c_no_such_package_error, dynamic_cast<const paludis::NoSuchPackageError *>(&ee)->message().c_str());
    else if (0 != dynamic_cast<const paludis::PackageDatabaseLookupError *>(&ee))
        rb_raise(c_package_database_lookup_error, dynamic_cast<const paludis::PackageDatabaseLookupError *>(&ee)->message().c_str());
    else if (0 != dynamic_cast<const paludis::PackageDatabaseError *>(&ee))
        rb_raise(c_package_database_error, dynamic_cast<const paludis::PackageDatabaseError *>(&ee)->message().c_str());
    else if (0 != dynamic_cast<const paludis::DepStringNestingError *>(&ee))
        rb_raise(c_dep_string_nesting_error, dynamic_cast<const paludis::DepStringNestingError *>(&ee)->message().c_str());
    else if (0 != dynamic_cast<const paludis::DepStringParseError *>(&ee))
        rb_raise(c_dep_string_parse_error, dynamic_cast<const paludis::DepStringParseError *>(&ee)->message().c_str());
    else if (0 != dynamic_cast<const paludis::DepStringError *>(&ee))
        rb_raise(c_dep_string_error, dynamic_cast<const paludis::DepStringError *>(&ee)->message().c_str());
#ifdef ENABLE_RUBY_QA
    else if (0 != dynamic_cast<const paludis::qa::ProfilesDescError *>(&ee))
        rb_raise(c_profiles_desc_error, dynamic_cast<const paludis::qa::ProfilesDescError *>(&ee)->message().c_str());
    else if (0 != dynamic_cast<const paludis::qa::NoSuchFileCheckTypeError *>(&ee))
        rb_raise(c_no_such_file_check_type_error, dynamic_cast<const paludis::qa::NoSuchFileCheckTypeError *>(&ee)->message().c_str());
    else if (0 != dynamic_cast<const paludis::qa::NoSuchPackageDirCheckTypeError *>(&ee))
        rb_raise(c_no_such_package_dir_check_type_error, dynamic_cast<const paludis::qa::NoSuchPackageDirCheckTypeError *>(&ee)->message().c_str());
    else if (0 != dynamic_cast<const paludis::qa::NoSuchEbuildCheckTypeError *>(&ee))
        rb_raise(c_no_such_ebuild_check_type_error, dynamic_cast<const paludis::qa::NoSuchEbuildCheckTypeError *>(&ee)->message().c_str());
#endif
    else if (0 != dynamic_cast<const paludis::ConfigFileError *>(&ee))
        rb_raise(c_config_file_error, dynamic_cast<const paludis::ConfigFileError *>(&ee)->message().c_str());
    else if (0 != dynamic_cast<const paludis::ConfigurationError *>(&ee))
        rb_raise(c_configuration_error, dynamic_cast<const paludis::ConfigurationError *>(&ee)->message().c_str());

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
paludis::ruby::paludis_module()
{
    return c_paludis_module;
}

VALUE
paludis::ruby::environment_class()
{
    return c_environment;
}

#ifdef ENABLE_RUBY_QA
VALUE
paludis::ruby::paludis_qa_module()
{
    return c_paludis_qa_module;
}
#endif

extern "C"
{
    void Init_Paludis()
    {
        c_paludis_module = rb_define_module("Paludis");
        c_environment = rb_define_class_under(paludis_module(), "Environment", rb_cObject);
        c_name_error = rb_define_class_under(c_paludis_module, "NameError", rb_eRuntimeError);
        c_set_name_error = rb_define_class_under(c_paludis_module, "SetNameError", c_name_error);
        c_category_name_part_error = rb_define_class_under(c_paludis_module, "CategoryNamePartError", c_name_error);
        c_package_name_part_error = rb_define_class_under(c_paludis_module, "PackageNamePartError", c_name_error);
        c_bad_version_spec_error = rb_define_class_under(c_paludis_module, "BadVersionSpecError", c_name_error);
        c_package_dep_atom_error = rb_define_class_under(c_paludis_module, "PackageDepAtomError", rb_eRuntimeError);
        c_package_database_error = rb_define_class_under(c_paludis_module, "PackageDatabaseError", rb_eRuntimeError);
        c_package_database_lookup_error = rb_define_class_under(c_paludis_module, "PackageDatabaseLookupError", c_package_database_error);
        c_ambiguous_package_name_error = rb_define_class_under(c_paludis_module, "AmbiguousPackageNameError", c_package_database_lookup_error);
        c_no_such_package_error = rb_define_class_under(c_paludis_module, "NoSuchPackageError", c_package_database_lookup_error);
        c_no_such_repository_error = rb_define_class_under(c_paludis_module, "NoSuchRepositoryError", c_package_database_lookup_error);
        c_dep_string_error = rb_define_class_under(c_paludis_module, "DepStringError", rb_eRuntimeError);
        c_dep_string_parse_error = rb_define_class_under(c_paludis_module, "DepStringParseError", c_dep_string_error);
        c_dep_string_nesting_error = rb_define_class_under(c_paludis_module, "DepStringNestingError", c_dep_string_parse_error);
        c_configuration_error = rb_define_class_under(c_paludis_module, "ConfigurationError", rb_eRuntimeError);
        c_config_file_error = rb_define_class_under(c_paludis_module, "ConfigFileError", c_configuration_error);

        rb_define_module_function(c_paludis_module, "match_package", RUBY_FUNC_CAST(&paludis_match_package), 3);

        rb_define_const(c_paludis_module, "Version", rb_str_new2((stringify(PALUDIS_VERSION_MAJOR) + "."
                        + stringify(PALUDIS_VERSION_MINOR) + "." + stringify(PALUDIS_VERSION_MICRO)).c_str()));
#ifdef ENABLE_RUBY_QA
        c_paludis_qa_module = rb_define_module_under(c_paludis_module,"QA");
        c_profiles_desc_error = rb_define_class_under(c_paludis_qa_module, "ProfilesDescError", c_configuration_error);
        c_no_such_file_check_type_error = rb_define_class_under(c_paludis_qa_module, "NoSuchFileCheckTypeError", rb_eTypeError);
        c_no_such_package_dir_check_type_error = rb_define_class_under(c_paludis_qa_module, "NoSuchPackageDirCheckTypeError", rb_eTypeError);
        c_no_such_ebuild_check_type_error = rb_define_class_under(c_paludis_qa_module, "NoSuchEbuildCheckTypeError", rb_eTypeError);
#endif
        RegisterRubyClass::get_instance()->execute();
    }
}

