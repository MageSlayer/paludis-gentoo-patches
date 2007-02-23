/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Richard Brown <rbrown@gentoo.org>
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

#include <paludis_ruby.hh>
#include <paludis/qa/check.hh>
#include <paludis/qa/package_dir_check.hh>
#include <paludis/util/stringify.hh>
#include <paludis/qa/qa.hh>
#include <ruby.h>

using namespace paludis;
using namespace paludis::qa;
using namespace paludis::ruby;

#define RUBY_FUNC_CAST(x) reinterpret_cast<VALUE (*)(...)>(x)

namespace
{
    static VALUE c_ebuild_check_data;
    static VALUE c_per_profile_ebuild_check_data;
    static VALUE c_profile_check_data;
    static VALUE c_package_dir_check;
    static VALUE c_package_dir_check_maker;
    static VALUE c_file_check;
    static VALUE c_file_check_maker;
    static VALUE c_ebuild_check;
    static VALUE c_ebuild_check_maker;
    static VALUE c_per_profile_ebuild_check;
    static VALUE c_per_profile_ebuild_check_maker;
    static VALUE c_profiles_check;
    static VALUE c_profiles_check_maker;
    static VALUE c_profile_check;
    static VALUE c_profile_check_maker;

    VALUE
    ebuild_check_data_init(int, VALUE *, VALUE self)
    {
        return self;
    }

    /*
     * call-seq:
     *     EbuildCheckData.new(qualified_package_name, version_spec, environment_data)
     *
     * Creates a new EbuildCheckData for EbuildCheck.
     */
    VALUE
    ebuild_check_data_new(int argc, VALUE *argv, VALUE self)
    {
        EbuildCheckData * ptr(0);
        try
        {
            if (3 == argc)
            {
                ptr = new EbuildCheckData(
                    value_to_qualified_package_name(argv[0]),
                    value_to_version_spec(argv[1]),
                    value_to_qa_environment(argv[2]));
            }
            else
            {
                rb_raise(rb_eArgError, "EbuildCheckData expects three arguments, but got %d",argc);
            }
            VALUE tdata(Data_Wrap_Struct(self, 0, &Common<EbuildCheckData>::free, ptr));
            rb_obj_call_init(tdata, argc, argv);
            return tdata;
        }
        catch (const std::exception & e)
        {
            delete ptr;
            exception_to_ruby_exception(e);
        }
    }

    VALUE
    per_profile_ebuild_check_data_init(int, VALUE *, VALUE self)
    {
        return self;
    }

    /*
     * call-seq:
     *     PerProfileEbuildCheckData.new(qualified_package_name, version_spec, environment_data, profile)
     *
     * Creates a new PerProfileEbuildCheckData for PerProfileEbuildCheck.2
     */
    VALUE
    per_profile_ebuild_check_data_new(int argc, VALUE *argv, VALUE self)
    {
        PerProfileEbuildCheckData * ptr(0);
        try
        {
            if (4 == argc)
            {
                ptr = new PerProfileEbuildCheckData(
                    value_to_qualified_package_name(argv[0]),
                    value_to_version_spec(argv[1]),
                    value_to_qa_environment(argv[2]),
                    FSEntry(StringValuePtr(argv[3]))
                );
            }
            else
            {
                rb_raise(rb_eArgError, "PerProfileEbuildCheckData expects three arguments, but got %d",argc);
            }
            VALUE tdata(Data_Wrap_Struct(self, 0, &Common<PerProfileEbuildCheckData>::free, ptr));
            rb_obj_call_init(tdata, argc, argv);
            return tdata;
        }
        catch (const std::exception & e)
        {
            delete ptr;
            exception_to_ruby_exception(e);
        }
    }

    VALUE
    profile_check_data_init(int, VALUE *, VALUE self)
    {
        return self;
    }

    /*
     * call-seq:
     *     ProfileCheckData.new(profiles_dir, profiles_desc_line)
     *
     * Creates a new ProfileCheckData for ProfileCheck.
     */
    VALUE
    profile_check_data_new(int argc, VALUE *argv, VALUE self)
    {
        ProfileCheckData * ptr(0);
        try
        {
            if (2 == argc)
            {
                ptr = new ProfileCheckData(
                    FSEntry(StringValuePtr(argv[0])),
                    value_to_portage_repository_profiles_desc_line(argv[1])
                );
            }
            else
            {
                rb_raise(rb_eArgError, "ProfileCheckData expects two arguments, but got %d",argc);
            }
            VALUE tdata(Data_Wrap_Struct(self, 0, &Common<ProfileCheckData>::free, ptr));
            rb_obj_call_init(tdata, argc, argv);
            return tdata;
        }
        catch (const std::exception & e)
        {
            delete ptr;
            exception_to_ruby_exception(e);
        }
    }

    /*
     * Document-method: describe
     *
     * call-seq:
     *     describe -> String
     *
     * Describe what check checks.
     */
    /*
     * Document-method: is_important?
     *
     * call-seq:
     *     is_important? -> true or false
     *
     * Is the check important?
     */
    template <typename T_>
    struct CheckStruct
    {
        static
        VALUE describe(VALUE self)
        {
            T_ * ptr;
            Data_Get_Struct(self, T_, ptr);
            return rb_str_new2((*ptr)->describe().c_str());
        }
        static
        VALUE
        is_important(VALUE self)
        {
            T_ * ptr;
            Data_Get_Struct(self, T_, ptr);
            return (*ptr)->is_important() ? Qtrue : Qfalse;
        }
    };

    template <typename T_, typename S_>
    VALUE
    check_check(VALUE check, S_ arg)
    {
        try
        {
            T_ * ptr;
            Data_Get_Struct(check, T_, ptr);
            return check_result_to_value((**ptr)(arg));
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     check(directory) -> CheckResult
     *
     * Runs check on directory.
     */
    VALUE
    package_dir_check_check(VALUE self, VALUE f)
    {
        try
        {
            return (check_check <std::tr1::shared_ptr<PackageDirCheck>, FSEntry> (self, FSEntry(StringValuePtr(f))));
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     check(file) -> CheckResult
     *
     * Runs check on file.
     */
    VALUE
    file_check_check(VALUE self, VALUE f)
    {
        try
        {
            return (check_check <std::tr1::shared_ptr<FileCheck>, FSEntry> (self, FSEntry(StringValuePtr(f))));
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     check(ebuild_check_data) -> CheckResult
     *
     * Runs check on EbuildCheckData.
     */
    VALUE
    ebuild_check_check(VALUE self, VALUE f)
    {
        try
        {
            return (check_check <std::tr1::shared_ptr<EbuildCheck>, EbuildCheckData> (self, value_to_ebuild_check_data(f)));
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     check(per_profile_ebuild_check_data) -> CheckResult
     *
     * Runs check on PerProfileEbuildCheckData.
     */
    VALUE
    per_profile_ebuild_check_check(VALUE self, VALUE f)
    {
        try
        {
            return (check_check <std::tr1::shared_ptr<PerProfileEbuildCheck>, PerProfileEbuildCheckData> (self, value_to_per_profile_ebuild_check_data(f)));
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     check(dir) -> CheckResult
     *
     * Runs check on directory.
     */
    VALUE
    profiles_check_check(VALUE self, VALUE f)
    {
        try
        {
            return (check_check <std::tr1::shared_ptr<ProfilesCheck>, FSEntry> (self, FSEntry(StringValuePtr(f))));
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     check(profile_check_data) -> CheckResult
     *
     * Runs check on ProfileCheckData.
     */
    VALUE
    profile_check_check(VALUE self, VALUE f)
    {
        try
        {
            return (check_check <std::tr1::shared_ptr<ProfileCheck>, ProfileCheckData> (self, value_to_profile_check_data(f)));
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * Document-method: keys
     *
     * call-seq:
     *     keys -> Array
     *     keys {|check_name| block } -> Qnil
     *
     * Returns the names of all checks in this maker, either as an Array or as the parameters to a block.
     */
    template <typename T_>
    struct CheckMakerStruct
    {
        static
        VALUE keys(VALUE)
        {
            std::list<std::string> checks;
            T_::get_instance()->copy_keys(std::back_inserter(checks));
            if (rb_block_given_p())
            {
                for (std::list<std::string>::const_iterator i(checks.begin()),
                    i_end(checks.end()) ; i != i_end ; ++i)
                        rb_yield(rb_str_new2((*i).c_str()));
                return Qnil;
            }
            VALUE result(rb_ary_new());
            for (std::list<std::string>::const_iterator i(checks.begin()),
                i_end(checks.end()) ; i != i_end ; ++i)
                    rb_ary_push(result, rb_str_new2((*i).c_str()));
            return result;
        }
    };

    /*
     * call-seq:
     *     find_maker(name)
     *
     * Fetch the named check.
     */
    VALUE package_dir_check_maker_find_maker(VALUE, VALUE maker)
    {
        try
        {
            std::tr1::shared_ptr<PackageDirCheck> p = (PackageDirCheckMaker::get_instance()->find_maker(StringValuePtr(maker)))();
            return package_dir_check_to_value(p);
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     find_maker(name)
     *
     * Fetch the named check.
     */
    VALUE file_check_maker_find_maker(VALUE, VALUE maker)
    {
        try
        {
            std::tr1::shared_ptr<FileCheck> p = (FileCheckMaker::get_instance()->find_maker(StringValuePtr(maker)))();
            return file_check_to_value(p);
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     find_maker(name)
     *
     * Fetch the named check.
     */
    VALUE ebuild_check_maker_find_maker(VALUE, VALUE maker)
    {
        try
        {
            std::tr1::shared_ptr<EbuildCheck> p = (EbuildCheckMaker::get_instance()->find_maker(StringValuePtr(maker)))();
            return ebuild_check_to_value(p);
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     find_maker(name)
     *
     * Fetch the named check.
     */
    VALUE per_profile_ebuild_check_maker_find_maker(VALUE, VALUE maker)
    {
        try
        {
            std::tr1::shared_ptr<PerProfileEbuildCheck> p = (PerProfileEbuildCheckMaker::get_instance()->find_maker(StringValuePtr(maker)))();
            return per_profile_ebuild_check_to_value(p);
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     find_maker(name)
     *
     * Fetch the named check.
     */
    VALUE profiles_check_maker_find_maker(VALUE, VALUE maker)
    {
        try
        {
            std::tr1::shared_ptr<ProfilesCheck> p = (ProfilesCheckMaker::get_instance()->find_maker(StringValuePtr(maker)))();
            return profiles_check_to_value(p);
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     find_maker(name)
     *
     * Fetch the named check.
     */
    VALUE profile_check_maker_find_maker(VALUE, VALUE maker)
    {
        try
        {
            std::tr1::shared_ptr<ProfileCheck> p = (ProfileCheckMaker::get_instance()->find_maker(StringValuePtr(maker)))();
            return profile_check_to_value(p);
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    void do_register_check()
    {
        rb_require("singleton");

        /*
         * Document-class: Paludis::QA::EbuildCheckData
         *
         * A collection class for EbuildCheck.
         */
        c_ebuild_check_data = rb_define_class_under(paludis_qa_module(), "EbuildCheckData", rb_cObject);
        rb_define_singleton_method(c_ebuild_check_data, "new", RUBY_FUNC_CAST(&ebuild_check_data_new),-1);
        rb_define_method(c_ebuild_check_data, "initialize", RUBY_FUNC_CAST(&ebuild_check_data_init),-1);

        /*
         * Document-class: Paludis::QA::PerProfileEbuildCheckData
         *
         * A collection class for PerProfileEbuildCheck.
         */
        c_per_profile_ebuild_check_data = rb_define_class_under(paludis_qa_module(), "PerProfileEbuildCheckData", rb_cObject);
        rb_define_singleton_method(c_per_profile_ebuild_check_data, "new", RUBY_FUNC_CAST(&per_profile_ebuild_check_data_new),-1);
        rb_define_method(c_per_profile_ebuild_check_data, "initialize", RUBY_FUNC_CAST(&per_profile_ebuild_check_data_init),-1);

        /*
         * Document-class: Paludis::QA::ProfileCheckData
         *
         * A collection class for ProfileCheck.
         */
        c_profile_check_data = rb_define_class_under(paludis_qa_module(), "ProfileCheckData", rb_cObject);
        rb_define_singleton_method(c_profile_check_data, "new", RUBY_FUNC_CAST(&profile_check_data_new),-1);
        rb_define_method(c_profile_check_data, "initialize", RUBY_FUNC_CAST(&profile_check_data_init),-1);

        /*
         * Document-class: Paludis::QA::PackageDirCheck
         *
         * A QA check that operates upon a package directory.
         */
        c_package_dir_check = rb_define_class_under(paludis_qa_module(), "PackageDirCheck", rb_cObject);
        rb_funcall(c_package_dir_check, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_method(c_package_dir_check, "check", RUBY_FUNC_CAST(&package_dir_check_check),1);
        rb_define_method(c_package_dir_check, "describe", RUBY_FUNC_CAST(&CheckStruct<std::tr1::shared_ptr<PackageDirCheck> >::describe),0);
        rb_define_method(c_package_dir_check, "is_important?", RUBY_FUNC_CAST(&CheckStruct<std::tr1::shared_ptr<PackageDirCheck> >::is_important),0);

        /*
         * Document-class: Paludis::QA::FileCheck
         *
         * A QA check that operates upon a file.
         */
        c_file_check = rb_define_class_under(paludis_qa_module(), "FileCheck", rb_cObject);
        rb_funcall(c_file_check, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_method(c_file_check, "check", RUBY_FUNC_CAST(&file_check_check),1);
        rb_define_method(c_file_check, "describe", RUBY_FUNC_CAST(&CheckStruct<std::tr1::shared_ptr<FileCheck> >::describe),0);
        rb_define_method(c_file_check, "is_important?", RUBY_FUNC_CAST(&CheckStruct<std::tr1::shared_ptr<FileCheck> >::is_important),0);

        /*
         * Document-class: Paludis::QA::EbuildCheck
         *
         * Base class for QA checks that operate upon ebuilds.
         */
        c_ebuild_check = rb_define_class_under(paludis_qa_module(), "EbuildCheck", rb_cObject);
        rb_funcall(c_ebuild_check, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_method(c_ebuild_check, "check", RUBY_FUNC_CAST(&ebuild_check_check),1);
        rb_define_method(c_ebuild_check, "describe", RUBY_FUNC_CAST(&CheckStruct<std::tr1::shared_ptr<EbuildCheck> >::describe),0);
        rb_define_method(c_ebuild_check, "is_important?", RUBY_FUNC_CAST(&CheckStruct<std::tr1::shared_ptr<EbuildCheck> >::is_important),0);

        /*
         * Document-class: Paludis::QA::PerProfileEbuildCheck
         *
         * Base class for QA checks that operate upon ebuilds.
         */
        c_per_profile_ebuild_check = rb_define_class_under(paludis_qa_module(), "PerProfileEbuildCheck", rb_cObject);
        rb_funcall(c_per_profile_ebuild_check, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_method(c_per_profile_ebuild_check, "check", RUBY_FUNC_CAST(&per_profile_ebuild_check_check),1);
        rb_define_method(c_per_profile_ebuild_check, "describe", RUBY_FUNC_CAST(&CheckStruct<std::tr1::shared_ptr<PerProfileEbuildCheck> >::describe),0);
        rb_define_method(c_per_profile_ebuild_check, "is_important?", RUBY_FUNC_CAST(&CheckStruct<std::tr1::shared_ptr<PerProfileEbuildCheck> >::is_important),0);

        /*
         * Document-class: Paludis::QA::ProfilesCheck
         *
         * Base class for QA checks that operate upon the top level /profiles directory.
         */
        c_profiles_check = rb_define_class_under(paludis_qa_module(), "ProfilesCheck", rb_cObject);
        rb_funcall(c_profiles_check, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_method(c_profiles_check, "check", RUBY_FUNC_CAST(&profiles_check_check),1);
        rb_define_method(c_profiles_check, "describe", RUBY_FUNC_CAST(&CheckStruct<std::tr1::shared_ptr<ProfilesCheck> >::describe),0);
        rb_define_method(c_profiles_check, "is_important?", RUBY_FUNC_CAST(&CheckStruct<std::tr1::shared_ptr<ProfilesCheck> >::is_important),0);

        /*
         * Document-class: Paludis::QA::ProfileCheck
         *
         * Base class for QA checks that operate upon a profiles.desc entry directory.
         */
        c_profile_check = rb_define_class_under(paludis_qa_module(), "ProfileCheck", rb_cObject);
        rb_funcall(c_profile_check, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_method(c_profile_check, "check", RUBY_FUNC_CAST(&profile_check_check),1);
        rb_define_method(c_profile_check, "describe", RUBY_FUNC_CAST(&CheckStruct<std::tr1::shared_ptr<ProfileCheck> >::describe),0);
        rb_define_method(c_profile_check, "is_important?", RUBY_FUNC_CAST(&CheckStruct<std::tr1::shared_ptr<ProfileCheck> >::is_important),0);

        /*
         * Document-class: Paludis::QA::PackageDirCheckMaker
         *
         * Class to access PackageDirChecks
         *
         */
        c_package_dir_check_maker = rb_define_class_under(paludis_qa_module(), "PackageDirCheckMaker", rb_cObject);
        rb_funcall(c_package_dir_check_maker, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_funcall(rb_const_get(rb_cObject, rb_intern("Singleton")), rb_intern("included"), 1, c_package_dir_check_maker);
        rb_define_method(c_package_dir_check_maker, "keys", RUBY_FUNC_CAST(CheckMakerStruct<PackageDirCheckMaker>::keys),0);
        rb_define_method(c_package_dir_check_maker, "find_maker", RUBY_FUNC_CAST(&package_dir_check_maker_find_maker),1);
        rb_define_alias(c_package_dir_check_maker, "check_names", "keys");
        rb_define_alias(c_package_dir_check_maker, "find_check", "find_maker");

        /*
         * Document-class: Paludis::QA::FileCheckMaker
         *
         * Class to access FileChecks
         *
         */
        c_file_check_maker = rb_define_class_under(paludis_qa_module(), "FileCheckMaker", rb_cObject);
        rb_funcall(c_file_check_maker, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_funcall(rb_const_get(rb_cObject, rb_intern("Singleton")), rb_intern("included"), 1, c_file_check_maker);
        rb_define_method(c_file_check_maker, "keys", RUBY_FUNC_CAST(CheckMakerStruct<FileCheckMaker>::keys),0);
        rb_define_method(c_file_check_maker, "find_maker", RUBY_FUNC_CAST(&file_check_maker_find_maker),1);
        rb_define_alias(c_file_check_maker, "check_names", "keys");
        rb_define_alias(c_file_check_maker, "find_check", "find_maker");

        /*
         * Document-class: Paludis::QA::EbuildCheckMaker
         *
         * Class to access EbuildChecks
         *
         */
        c_ebuild_check_maker = rb_define_class_under(paludis_qa_module(), "EbuildCheckMaker", rb_cObject);
        rb_funcall(c_ebuild_check_maker, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_funcall(rb_const_get(rb_cObject, rb_intern("Singleton")), rb_intern("included"), 1, c_ebuild_check_maker);
        rb_define_method(c_ebuild_check_maker, "keys", RUBY_FUNC_CAST(&CheckMakerStruct<EbuildCheckMaker>::keys),0);
        rb_define_method(c_ebuild_check_maker, "find_maker", RUBY_FUNC_CAST(&ebuild_check_maker_find_maker),1);
        rb_define_alias(c_ebuild_check_maker, "check_names", "keys");
        rb_define_alias(c_ebuild_check_maker, "find_check", "find_maker");

        /*
         * Document-class: Paludis::QA::PerProfileEbuildCheckMaker
         *
         * Class to access PerProfileEbuildChecks
         *
         */
        c_per_profile_ebuild_check_maker = rb_define_class_under(paludis_qa_module(), "PerProfileEbuildCheckMaker", rb_cObject);
        rb_funcall(c_per_profile_ebuild_check_maker, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_funcall(rb_const_get(rb_cObject, rb_intern("Singleton")), rb_intern("included"), 1, c_per_profile_ebuild_check_maker);
        rb_define_method(c_per_profile_ebuild_check_maker, "keys", RUBY_FUNC_CAST(&CheckMakerStruct<PerProfileEbuildCheckMaker>::keys),0);
        rb_define_method(c_per_profile_ebuild_check_maker, "find_maker", RUBY_FUNC_CAST(&per_profile_ebuild_check_maker_find_maker),1);
        rb_define_alias(c_per_profile_ebuild_check_maker, "check_names", "keys");
        rb_define_alias(c_per_profile_ebuild_check_maker, "find_check", "find_maker");

        /*
         * Document-class: Paludis::QA::ProfilesCheckMaker
         *
         * Class to access ProfilesChecks
         *
         */
        c_profiles_check_maker = rb_define_class_under(paludis_qa_module(), "ProfilesCheckMaker", rb_cObject);
        rb_funcall(c_profiles_check_maker, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_funcall(rb_const_get(rb_cObject, rb_intern("Singleton")), rb_intern("included"), 1, c_profiles_check_maker);
        rb_define_method(c_profiles_check_maker, "keys", RUBY_FUNC_CAST(&CheckMakerStruct<ProfilesCheckMaker>::keys),0);
        rb_define_method(c_profiles_check_maker, "find_maker", RUBY_FUNC_CAST(&profiles_check_maker_find_maker),1);
        rb_define_alias(c_profiles_check_maker, "check_names", "keys");
        rb_define_alias(c_profiles_check_maker, "find_check", "find_maker");

        /*
         * Document-class: Paludis::QA::ProfileCheckMaker
         *
         * Class to access ProfileChecks
         *
         */
        c_profile_check_maker = rb_define_class_under(paludis_qa_module(), "ProfileCheckMaker", rb_cObject);
        rb_funcall(c_profile_check_maker, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_funcall(rb_const_get(rb_cObject, rb_intern("Singleton")), rb_intern("included"), 1, c_profile_check_maker);
        rb_define_method(c_profile_check_maker, "keys", RUBY_FUNC_CAST(&CheckMakerStruct<ProfileCheckMaker>::keys),0);
        rb_define_method(c_profile_check_maker, "find_maker", RUBY_FUNC_CAST(&profile_check_maker_find_maker),1);
        rb_define_alias(c_profile_check_maker, "check_names", "keys");
        rb_define_alias(c_profile_check_maker, "find_check", "find_maker");
    }
}

VALUE
paludis::ruby::file_check_to_value(std::tr1::shared_ptr<FileCheck> m)
{
    std::tr1::shared_ptr<FileCheck> * m_ptr(0);
    try
    {
        m_ptr = new std::tr1::shared_ptr<FileCheck>(m);
        return Data_Wrap_Struct(c_file_check, 0, &Common<std::tr1::shared_ptr<FileCheck> >::free, m_ptr);
    }
    catch (const std::exception & e)
    {
        delete m_ptr;
        exception_to_ruby_exception(e);
    }
}

VALUE
paludis::ruby::package_dir_check_to_value(std::tr1::shared_ptr<PackageDirCheck> m)
{
    std::tr1::shared_ptr<PackageDirCheck> * m_ptr(0);
    try
    {
        m_ptr = new std::tr1::shared_ptr<PackageDirCheck>(m);
        return Data_Wrap_Struct(c_package_dir_check, 0, &Common<std::tr1::shared_ptr<PackageDirCheck> >::free, m_ptr);
    }
    catch (const std::exception & e)
    {
        delete m_ptr;
        exception_to_ruby_exception(e);
    }
}

VALUE
paludis::ruby::ebuild_check_to_value(std::tr1::shared_ptr<EbuildCheck> m)
{
    std::tr1::shared_ptr<EbuildCheck> * m_ptr(0);
    try
    {
        m_ptr = new std::tr1::shared_ptr<EbuildCheck>(m);
        return Data_Wrap_Struct(c_ebuild_check, 0, &Common<std::tr1::shared_ptr<EbuildCheck> >::free, m_ptr);
    }
    catch (const std::exception & e)
    {
        delete m_ptr;
        exception_to_ruby_exception(e);
    }
}

VALUE
paludis::ruby::profiles_check_to_value(std::tr1::shared_ptr<ProfilesCheck> m)
{
    std::tr1::shared_ptr<ProfilesCheck> * m_ptr(0);
    try
    {
        m_ptr = new std::tr1::shared_ptr<ProfilesCheck>(m);
        return Data_Wrap_Struct(c_profiles_check, 0, &Common<std::tr1::shared_ptr<ProfilesCheck> >::free, m_ptr);
    }
    catch (const std::exception & e)
    {
        delete m_ptr;
        exception_to_ruby_exception(e);
    }
}

VALUE
paludis::ruby::profile_check_to_value(std::tr1::shared_ptr<ProfileCheck> m)
{
    std::tr1::shared_ptr<ProfileCheck> * m_ptr(0);
    try
    {
        m_ptr = new std::tr1::shared_ptr<ProfileCheck>(m);
        return Data_Wrap_Struct(c_profile_check, 0, &Common<std::tr1::shared_ptr<ProfileCheck> >::free, m_ptr);
    }
    catch (const std::exception & e)
    {
        delete m_ptr;
        exception_to_ruby_exception(e);
    }
}

VALUE
paludis::ruby::per_profile_ebuild_check_to_value(std::tr1::shared_ptr<PerProfileEbuildCheck> m)
{
    std::tr1::shared_ptr<PerProfileEbuildCheck> * m_ptr(0);
    try
    {
        m_ptr = new std::tr1::shared_ptr<PerProfileEbuildCheck>(m);
        return Data_Wrap_Struct(c_per_profile_ebuild_check, 0, &Common<std::tr1::shared_ptr<PerProfileEbuildCheck> >::free, m_ptr);
    }
    catch (const std::exception & e)
    {
        delete m_ptr;
        exception_to_ruby_exception(e);
    }
}

EbuildCheckData
paludis::ruby::value_to_ebuild_check_data(VALUE v)
{
    if (rb_obj_is_kind_of(v, c_ebuild_check_data))
    {
        EbuildCheckData * v_ptr;
        Data_Get_Struct(v, EbuildCheckData, v_ptr);
        return *v_ptr;
    }
    else
    {
        rb_raise(rb_eTypeError, "Can't convert %s into EbuildCheckData", rb_obj_classname(v));
    }
}

VALUE
paludis::ruby::ebuild_check_data_to_value(const EbuildCheckData & v)
{
    EbuildCheckData * vv(new EbuildCheckData(v));
    return Data_Wrap_Struct(c_ebuild_check_data, 0, &Common<EbuildCheckData>::free, vv);
}

PerProfileEbuildCheckData
paludis::ruby::value_to_per_profile_ebuild_check_data(VALUE v)
{
    if (rb_obj_is_kind_of(v, c_per_profile_ebuild_check_data))
    {
        PerProfileEbuildCheckData * v_ptr;
        Data_Get_Struct(v, PerProfileEbuildCheckData, v_ptr);
        return *v_ptr;
    }
    else
    {
        rb_raise(rb_eTypeError, "Can't convert %s into PerProfileEbuildCheckData", rb_obj_classname(v));
    }
}

VALUE
paludis::ruby::per_profile_ebuild_check_data_to_value(const PerProfileEbuildCheckData & v)
{
    PerProfileEbuildCheckData * vv(new PerProfileEbuildCheckData(v));
    return Data_Wrap_Struct(c_per_profile_ebuild_check_data, 0, &Common<PerProfileEbuildCheckData>::free, vv);
}

ProfileCheckData
paludis::ruby::value_to_profile_check_data(VALUE v)
{
    if (rb_obj_is_kind_of(v, c_profile_check_data))
    {
        ProfileCheckData * v_ptr;
        Data_Get_Struct(v, ProfileCheckData, v_ptr);
        return *v_ptr;
    }
    else
    {
        rb_raise(rb_eTypeError, "Can't convert %s into ProfileCheckData", rb_obj_classname(v));
    }
}

VALUE
paludis::ruby::profile_check_data_to_value(const ProfileCheckData & v)
{
    ProfileCheckData * vv(new ProfileCheckData(v));
    return Data_Wrap_Struct(c_profile_check_data, 0, &Common<ProfileCheckData>::free, vv);
}

RegisterRubyClass::Register paludis_ruby_register_check PALUDIS_ATTRIBUTE((used))
    (&do_register_check);
