/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh
 * Copyright (c) 2007 Richard Brown
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
#include <paludis/action.hh>
#include <paludis/util/visitor-impl.hh>
#include <ruby.h>

using namespace paludis;
using namespace paludis::ruby;

#define RUBY_FUNC_CAST(x) reinterpret_cast<VALUE (*)(...)>(x)

namespace
{
    static VALUE c_supports_action_test_base;
    static VALUE c_supports_fetch_action_test;
    static VALUE c_supports_info_action_test;
    static VALUE c_supports_config_action_test;
    static VALUE c_supports_install_action_test;
    static VALUE c_supports_uninstall_action_test;

    static VALUE c_action;
    static VALUE c_fetch_action;
    static VALUE c_fetch_action_options;
    static VALUE c_fetch_action_failure;

    static VALUE c_info_action;
    static VALUE c_config_action;

    static VALUE c_install_action_options;
    static VALUE c_install_action_debug_option;
    static VALUE c_install_action_checks_option;
    static VALUE c_install_action;

    static VALUE c_uninstall_action_options;
    static VALUE c_uninstall_action;

    const bool
    value_to_bool(VALUE v)
    {
        if (Qfalse == v || Qnil == v)
            return false;
        return true;
    }

    VALUE
    bool_to_value(bool b)
    {
        return b ? Qtrue : Qfalse;
    }

    const FetchActionOptions
    value_to_fetch_action_options(VALUE v)
    {
        if (rb_obj_is_kind_of(v, c_fetch_action_options))
        {
            FetchActionOptions * v_ptr;
            Data_Get_Struct(v, FetchActionOptions, v_ptr);
            return *v_ptr;
        }
        else
        {
            rb_raise(rb_eTypeError, "Can't convert %s into FetchActionOptions", rb_obj_classname(v));
        }
    }

    VALUE
    fetch_action_options_to_value(const FetchActionOptions & m)
    {
        FetchActionOptions * m_ptr(new FetchActionOptions(m));
        try
        {
            return Data_Wrap_Struct(c_fetch_action_options, 0, &Common<FetchActionOptions>::free, m_ptr);
        }
        catch (const std::exception & e)
        {
            delete m_ptr;
            exception_to_ruby_exception(e);
        }
    }

    const InstallActionOptions
    value_to_install_action_options(VALUE v)
    {
        if (rb_obj_is_kind_of(v, c_install_action_options))
        {
            InstallActionOptions * v_ptr;
            Data_Get_Struct(v, InstallActionOptions, v_ptr);
            return *v_ptr;
        }
        else
        {
            rb_raise(rb_eTypeError, "Can't convert %s into InstallActionOptions", rb_obj_classname(v));
        }
    }

    VALUE
    install_action_options_to_value(const InstallActionOptions & m)
    {
        InstallActionOptions * m_ptr(new InstallActionOptions(m));
        try
        {
            return Data_Wrap_Struct(c_install_action_options, 0, &Common<InstallActionOptions>::free, m_ptr);
        }
        catch (const std::exception & e)
        {
            delete m_ptr;
            exception_to_ruby_exception(e);
        }
    }

    const UninstallActionOptions
    value_to_uninstall_action_options(VALUE v)
    {
        if (rb_obj_is_kind_of(v, c_uninstall_action_options))
        {
            UninstallActionOptions * v_ptr;
            Data_Get_Struct(v, UninstallActionOptions, v_ptr);
            return *v_ptr;
        }
        else
        {
            rb_raise(rb_eTypeError, "Can't convert %s into UninstallActionOptions", rb_obj_classname(v));
        }
    }

    VALUE
    uninstall_action_options_to_value(const UninstallActionOptions & m)
    {
        UninstallActionOptions * m_ptr(new UninstallActionOptions(m));
        try
        {
            return Data_Wrap_Struct(c_uninstall_action_options, 0, &Common<UninstallActionOptions>::free, m_ptr);
        }
        catch (const std::exception & e)
        {
            delete m_ptr;
            exception_to_ruby_exception(e);
        }
    }

    const FetchActionFailure
    value_to_fetch_action_failure(VALUE v)
    {
        if (rb_obj_is_kind_of(v, c_fetch_action_failure))
        {
            FetchActionFailure * v_ptr;
            Data_Get_Struct(v, FetchActionFailure, v_ptr);
            return *v_ptr;
        }
        else
        {
            rb_raise(rb_eTypeError, "Can't convert %s into FetchActionFailure", rb_obj_classname(v));
        }
    }

    VALUE
    empty_init(int, VALUE *, VALUE self)
    {
        return self;
    }

    /*
     * Document-method: SupportsFetchActionTest.new
     *
     * call-seq:
     *     SupportsFetchActionTest.new -> SupportsFetchActionTest
     *
     * Create new SupportsFetchActionTest object.
     */
    /*
     * Document-method: SupportsInfoActionTest.new
     *
     * call-seq:
     *     SupportsInfoActionTest.new -> SupportsInfoActionTest
     *
     * Create new SupportsInfoActionTest object.
     */
    /*
     * Document-method: SupportsConfigActionTest.new
     *
     * call-seq:
     *     SupportsConfigActionTest.new -> SupportsInfoActionTest
     *
     * Create new SupportsConfigActionTest object.
     */
    /*
     * Document-method: SupportsInstallActionTest.new
     *
     * call-seq:
     *     SupportsInstallActionTest.new -> SupportsInstallActionTest
     *
     * Create new SupportsInstallActionTest object.
     */
    /*
     * Document-method: SupportsUninstallActionTest.new
     *
     * call-seq:
     *     SupportsUninstallActionTest.new -> SupportsUninstallActionTest
     *
     * Create new SupportsUninstallActionTest object.
     */
    template <typename A_>
    struct SupportsActionTestNew
    {
        static VALUE
        supports_action_test_new(VALUE self)
        {
            tr1::shared_ptr<const SupportsActionTestBase> * a(
                    new tr1::shared_ptr<const SupportsActionTestBase>(new SupportsActionTest<A_>));
            VALUE tdata(Data_Wrap_Struct(self, 0, &Common<tr1::shared_ptr<const SupportsActionTestBase> >::free, a));
            rb_obj_call_init(tdata, 0, &self);
            return tdata;
        }
    };

    /*
     * call-seq:
     *     FetchActionOptions.new(fetch_unneeded, safe_resume) -> FetchActionOptions
     *     FetchActionOptions.new(Hash) -> FetchActionOptions
     *
     * FetchActionOptions.new can either be called with all parameters in order, or with one hash
     * parameter, where the hash keys are symbols with the names above.
     */
    VALUE
    fetch_action_options_new(int argc, VALUE *argv, VALUE self)
    {
        FetchActionOptions * ptr(0);
        try
        {
            bool v_fetch_unneeded;
            bool v_safe_resume;

            if (1 == argc && rb_obj_is_kind_of(argv[0], rb_cHash))
            {
                if (Qnil == rb_hash_aref(argv[0], ID2SYM(rb_intern("fetch_unneeded"))))
                    rb_raise(rb_eArgError, "Missing Parameter: fetch_unneeded");
                if (Qnil == rb_hash_aref(argv[0], ID2SYM(rb_intern("safe_resume"))))
                    rb_raise(rb_eArgError, "Missing Parameter: safe_resume");
                v_fetch_unneeded =
                    rb_hash_aref(argv[0], ID2SYM(rb_intern("fetch_unneeded"))) != Qfalse;
                v_safe_resume =
                    rb_hash_aref(argv[0], ID2SYM(rb_intern("safe_resume"))) != Qfalse;
            }
            else if (2 == argc)
            {
                v_fetch_unneeded          = argv[0] != Qfalse;
                v_safe_resume             = argv[1] != Qfalse;
            }
            else
            {
                rb_raise(rb_eArgError, "FetchActionOptions expects one or two arguments, but got %d",argc);
            }

            ptr = new FetchActionOptions(FetchActionOptions::create()
                    .fetch_unneeded(v_fetch_unneeded)
                    .safe_resume(v_safe_resume)
                    );

            VALUE tdata(Data_Wrap_Struct(self, 0, &Common<FetchActionOptions>::free, ptr));
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
     * call-seq:
     *     FetchAction.new -> FetchAction
     */
    VALUE
    fetch_action_new(VALUE self, VALUE opts)
    {
        const FetchActionOptions opts_ptr(value_to_fetch_action_options(opts));
        tr1::shared_ptr<Action> * a(
                new tr1::shared_ptr<Action>(new FetchAction(opts_ptr)));
        VALUE tdata(Data_Wrap_Struct(self, 0, &Common<tr1::shared_ptr<Action> >::free, a));
        rb_obj_call_init(tdata, 1, &self);
        return tdata;
    }

    /*
     * call-seq:
     *     options -> FetchActionOptions
     *
     * Our FetchActionOptions.
     */
    VALUE
    fetch_action_options(VALUE self)
    {
        tr1::shared_ptr<Action> * p;
        Data_Get_Struct(self, tr1::shared_ptr<Action>, p);
        return fetch_action_options_to_value(tr1::static_pointer_cast<FetchAction>(*p)->options);
    }

    /*
     * call-seq:
     *     FetchActionFailure.new(target_file, requires_manual_fetching, failed_automatic_fetching, failed_integrity_checks) -> FetchActionFailure
     *     FetchActionFailure.new(Hash) -> FetchActionFailure
     *
     * FetchActionFailure.new can either be called with all parameters in order, or with one hash
     * parameter, where the hash keys are symbols with the names above.
     */
    VALUE
    fetch_action_failure_new(int argc, VALUE *argv, VALUE self)
    {
        FetchActionFailure * ptr(0);
        try
        {
            std::string v_target_file;
            bool v_requires_manual_fetching;
            bool v_failed_automatic_fetching;
            std::string v_failed_integrity_checks;

            if (1 == argc && rb_obj_is_kind_of(argv[0], rb_cHash))
            {
                if (Qnil == rb_hash_aref(argv[0], ID2SYM(rb_intern("target_file"))))
                    rb_raise(rb_eArgError, "Missing Parameter: target_file");
                if (Qnil == rb_hash_aref(argv[0], ID2SYM(rb_intern("requires_manual_fetching"))))
                    rb_raise(rb_eArgError, "Missing Parameter: requires_manual_fetching");
                if (Qnil == rb_hash_aref(argv[0], ID2SYM(rb_intern("failed_automatic_fetching"))))
                    rb_raise(rb_eArgError, "Missing Parameter: failed_automatic_fetching");
                if (Qnil == rb_hash_aref(argv[0], ID2SYM(rb_intern("failed_integrity_checks"))))
                    rb_raise(rb_eArgError, "Missing Parameter: failed_integrity_checks");
                VALUE v0 = (rb_hash_aref(argv[0], ID2SYM(rb_intern("target_file"))));
                v_target_file = StringValuePtr(v0);
                v_requires_manual_fetching =
                    value_to_bool(rb_hash_aref(argv[0], ID2SYM(rb_intern("requires_manual_fetching"))));
                v_failed_automatic_fetching =
                    value_to_bool(rb_hash_aref(argv[0], ID2SYM(rb_intern("failed_automatic_fetching"))));
                VALUE v1 = rb_hash_aref(argv[0], ID2SYM(rb_intern("failed_integrity_checks")));
                v_failed_integrity_checks = StringValuePtr(v1);
            }
            else if (4 == argc)
            {
                v_target_file                       = StringValuePtr(argv[0]);
                v_requires_manual_fetching          = value_to_bool(argv[1]);
                v_failed_automatic_fetching         = value_to_bool(argv[2]);
                v_failed_integrity_checks           = StringValuePtr(argv[3]);
            }
            else
            {
                rb_raise(rb_eArgError, "FetchActionOptions expects one or four arguments, but got %d",argc);
            }

            ptr = new FetchActionFailure(FetchActionFailure::create()
                    .target_file(v_target_file)
                    .requires_manual_fetching(v_requires_manual_fetching)
                    .failed_automatic_fetching(v_failed_automatic_fetching)
                    .failed_integrity_checks(v_failed_integrity_checks)
                    );

            VALUE tdata(Data_Wrap_Struct(self, 0, &Common<FetchActionFailure>::free, ptr));
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
     * Document-method: target_file
     *
     *     call-seq: target_file -> String
     *
     * Our target file.
     */
    /*
     * Document-method: failed_integrity_checks
     *
     *     call-seq: failed_integrity_checks -> String
     *
     * Our failed integrity checks.
     */
    template <std::string FetchActionFailure::* m_>
    struct FailureStringFetch
    {
        static VALUE
        fetch(VALUE self)
        {
            FetchActionFailure * ptr;
            Data_Get_Struct(self, FetchActionFailure, ptr);
            return rb_str_new2(((*ptr).*m_).c_str());
        }
    };

    /*
     * Document-method: fetch_unneeded?
     *
     * call-seq:
     *     fetch_unneeded -> true or false
     */
    /*
     * Document-method: safe_resume?
     *
     * call-seq:
     *     safe_resume -> true or false
     */
    /*
     * Document-method: requires_manual_fetching?
     *
     * call-seq:
     *     requires_manual_fetching? -> true or false
     *
     * Do we require manual fetching?
     */
    /*
     * Document-method: failed_automatic_fetching?
     *
     * call-seq:
     *     failed_automatic_fetching? -> true or false
     *
     * Did we fail automatic fetching?
     */
    /*
     * Document-method: no_config_protect?
     *
     * call-seq:
     *     no_config_protect? -> true or false
     *
     * Do we ignore config protection.
     */
    template <typename T_, bool T_::* m_>
    struct BoolFetch
    {
        static VALUE
        fetch(VALUE self)
        {
            T_ * p;
            Data_Get_Struct(self, T_, p);
            return bool_to_value((*p).*m_);
        }
    };

    /*
     * Document-method InfoAction.new
     *
     * call-seq:
     *     InfoAction.new -> InfoAction
     *
     * Create new InfoAction
     */
    /*
     * Document-method ConfigAction.new
     *
     * call-seq:
     *     ConfigAction.new -> ConfigAction
     *
     * Create new ConfigAction
     */
    template <typename A_>
    struct EasyActionNew
    {
        static VALUE
        easy_action_new(VALUE self)
        {
            tr1::shared_ptr<Action> * a(new tr1::shared_ptr<Action>(new A_()));
            VALUE tdata(Data_Wrap_Struct(self, 0, &Common<tr1::shared_ptr<Action> >::free, a));
            rb_obj_call_init(tdata, 1, &self);
            return tdata;
        }
    };

    /*
     * call-seq:
     *     InstallActionOptions.new(no_config_protect, debug_build, checks, destination) -> InstallActionOptions
     *     InstallActionOptions.new(Hash) -> InstallActionOptions
     *
     * InstallActionOptions.new can either be called with all parameters in order, or with one hash
     * parameter, where the hash keys are symbols with the names above.
     */
    VALUE
    install_action_options_new(int argc, VALUE *argv, VALUE self)
    {
        InstallActionOptions * ptr(0);
        try
        {
            bool v_no_config_protect;
            InstallActionDebugOption v_debug_build;
            InstallActionChecksOption v_checks;
            tr1::shared_ptr<Repository> v_destination;

            if (1 == argc && rb_obj_is_kind_of(argv[0], rb_cHash))
            {
                if (Qnil == rb_hash_aref(argv[0], ID2SYM(rb_intern("no_config_protect"))))
                    rb_raise(rb_eArgError, "Missing Parameter: no_config_protect");
                if (Qnil == rb_hash_aref(argv[0], ID2SYM(rb_intern("debug_build"))))
                    rb_raise(rb_eArgError, "Missing Parameter: debug_build");
                if (Qnil == rb_hash_aref(argv[0], ID2SYM(rb_intern("checks"))))
                    rb_raise(rb_eArgError, "Missing Parameter: checks");
                if (Qnil == rb_hash_aref(argv[0], ID2SYM(rb_intern("destination"))))
                    rb_raise(rb_eArgError, "Missing Parameter: destination");
                v_no_config_protect =
                    value_to_bool(rb_hash_aref(argv[0], ID2SYM(rb_intern("no_config_protect"))));
                v_debug_build = static_cast<InstallActionDebugOption>(NUM2INT(
                            rb_hash_aref(argv[0], ID2SYM(rb_intern("debug_build")))
                ));
                v_checks = static_cast<InstallActionChecksOption>(NUM2INT(
                            rb_hash_aref(argv[0], ID2SYM(rb_intern("checks")))
                ));
                v_destination = value_to_repository(rb_hash_aref(argv[0], ID2SYM(rb_intern("destination"))));
            }
            else if (4 == argc)
            {
                v_no_config_protect = value_to_bool(argv[0]);
                v_debug_build = static_cast<InstallActionDebugOption>(NUM2INT(argv[1]));
                v_checks = static_cast<InstallActionChecksOption>(NUM2INT(argv[2]));
                v_destination = value_to_repository(argv[3]);
            }
            else
            {
                rb_raise(rb_eArgError, "InstallActionOptions expects one or four arguments, but got %d",argc);
            }

            ptr = new InstallActionOptions(InstallActionOptions::create()
                    .no_config_protect(v_no_config_protect)
                    .debug_build(v_debug_build)
                    .checks(v_checks)
                    .destination(v_destination)
                    );

            VALUE tdata(Data_Wrap_Struct(self, 0, &Common<InstallActionOptions>::free, ptr));
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
     * Document-method: debug_build
     *
     * call-seq:
     *     debug_build -> Fixnum
     *
     * Our InstallActionDebugOption
     */
    /*
     * Document-method: checks
     *
     * call-seq:
     *     checks -> FixNum
     *
     * Our InstallActionChecksOption
     *
     */
    template <typename T_, T_ InstallActionOptions::* m_>
    struct IAOMember
    {
        static VALUE
        fetch(VALUE self)
        {
            InstallActionOptions * p;
            Data_Get_Struct(self, InstallActionOptions, p);
            return INT2FIX((*p).*m_);
        }
    };

    /*
     * Document-method: destination
     *
     * call-seq:
     *     destination -> Repository
     *
     * Our destination
     */
    VALUE
    install_action_options_destination(VALUE self)
    {
        InstallActionOptions * p;
        Data_Get_Struct(self, InstallActionOptions, p);
        return repository_to_value(p->destination);
    }

    /*
     * call-seq:
     *     InstallAction.new -> InstallAction
     */
    VALUE
    install_action_new(VALUE self, VALUE opts)
    {
        const InstallActionOptions opts_ptr(value_to_install_action_options(opts));
        tr1::shared_ptr<Action> * a(
                new tr1::shared_ptr<Action>(new InstallAction(opts_ptr)));
        VALUE tdata(Data_Wrap_Struct(self, 0, &Common<tr1::shared_ptr<Action> >::free, a));
        rb_obj_call_init(tdata, 1, &self);
        return tdata;
    }

    /*
     * call-seq:
     *     options -> InstallActionOptions
     *
     * Our InstallActionOptions.
     */
    VALUE
    install_action_options(VALUE self)
    {
        tr1::shared_ptr<Action> * p;
        Data_Get_Struct(self, tr1::shared_ptr<Action>, p);
        return install_action_options_to_value(tr1::static_pointer_cast<InstallAction>(*p)->options);
    }

    /*
     * call-seq:
     *     UninstallActionOptions.new(no_config_protect) -> UninstallActionOptions
     *     UninstallActionOptions.new(Hash) -> UninstallActionOptions
     *
     * UninstallActionOptions.new can either be called with all parameters in order, or with one hash
     * parameter, where the hash keys are symbols with the names above.
     */
    VALUE
    uninstall_action_options_new(int argc, VALUE *argv, VALUE self)
    {
        UninstallActionOptions * ptr(0);
        try
        {
            bool v_no_config_protect;

            if (1 == argc && rb_obj_is_kind_of(argv[0], rb_cHash))
            {
                if (Qnil == rb_hash_aref(argv[0], ID2SYM(rb_intern("no_config_protect"))))
                    rb_raise(rb_eArgError, "Missing Parameter: no_config_protect");
                v_no_config_protect =
                    value_to_bool(rb_hash_aref(argv[0], ID2SYM(rb_intern("no_config_protect"))));
            }
            else if (1 == argc)
            {
                v_no_config_protect = value_to_bool(argv[0]);
            }
            else
            {
                rb_raise(rb_eArgError, "UninstallActionOptions expects one argument, but got %d",argc);
            }

            ptr = new UninstallActionOptions(UninstallActionOptions::create()
                    .no_config_protect(v_no_config_protect)
                    );

            VALUE tdata(Data_Wrap_Struct(self, 0, &Common<UninstallActionOptions>::free, ptr));
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
     * call-seq:
     *     UninstallAction.new -> UninstallAction
     */
    VALUE
    uninstall_action_new(VALUE self, VALUE opts)
    {
        const UninstallActionOptions opts_ptr(value_to_uninstall_action_options(opts));
        tr1::shared_ptr<Action> * a(
                new tr1::shared_ptr<Action>(new UninstallAction(opts_ptr)));
        VALUE tdata(Data_Wrap_Struct(self, 0, &Common<tr1::shared_ptr<Action> >::free, a));
        rb_obj_call_init(tdata, 1, &self);
        return tdata;
    }

    /*
     * call-seq:
     *     options -> UninstallActionOptions
     *
     * Our UninstallActionOptions.
     */
    VALUE
    uninstall_action_options(VALUE self)
    {
        tr1::shared_ptr<Action> * p;
        Data_Get_Struct(self, tr1::shared_ptr<Action>, p);
        return uninstall_action_options_to_value(tr1::static_pointer_cast<UninstallAction>(*p)->options);
    }

    void do_register_action()
    {
        /*
         * Document-class: Paludis::SupportsActionTestBase
         *
         * Base class for action tests, used by Paludis::PackageID#supports_action.
         */
        c_supports_action_test_base = rb_define_class_under(paludis_module(), "SupportsActionTestBase", rb_cObject);
        rb_funcall(c_supports_action_test_base, rb_intern("private_class_method"), 1, rb_str_new2("new"));

        /*
         * Document-class: Paludis::SupportsFetchActionTest
         *
         * Tests whether a Paludis::PackageID supports a Paludis::FetchAction.
         */
        c_supports_fetch_action_test = rb_define_class_under(paludis_module(), "SupportsFetchActionTest", c_supports_action_test_base);
        rb_define_singleton_method(c_supports_fetch_action_test, "new",
                RUBY_FUNC_CAST((&SupportsActionTestNew<FetchAction>::supports_action_test_new)), 0);
        rb_define_method(c_supports_fetch_action_test, "initialize", RUBY_FUNC_CAST(&empty_init), -1);

        /*
         * Document-class: Paludis::SupportsInfoActionTest
         *
         * Tests whether a Paludis::PackageID supports a Paludis::InfoAction.
         */
        c_supports_info_action_test = rb_define_class_under(paludis_module(), "SupportsInfoActionTest", c_supports_action_test_base);
        rb_define_singleton_method(c_supports_info_action_test, "new",
                RUBY_FUNC_CAST((&SupportsActionTestNew<InfoAction>::supports_action_test_new)), 0);
        rb_define_method(c_supports_info_action_test, "initialize", RUBY_FUNC_CAST(&empty_init), -1);

        /*
         * Document-class: Paludis::SupportsConfigActionTest
         *
         * Tests whether a Paludis::PackageID supports a Paludis::ConfigAction.
         */
        c_supports_config_action_test = rb_define_class_under(paludis_module(), "SupportsConfigActionTest", c_supports_action_test_base);
        rb_define_singleton_method(c_supports_config_action_test, "new",
                RUBY_FUNC_CAST((&SupportsActionTestNew<ConfigAction>::supports_action_test_new)), 0);
        rb_define_method(c_supports_config_action_test, "initialize", RUBY_FUNC_CAST(&empty_init), -1);

        /*
         * Document-class: Paludis::SupportsInstallActionTest
         *
         * Tests whether a Paludis::PackageID supports a Paludis::InstallAction.
         */
        c_supports_install_action_test = rb_define_class_under(paludis_module(), "SupportsInstallActionTest", c_supports_action_test_base);
        rb_define_singleton_method(c_supports_install_action_test, "new",
                RUBY_FUNC_CAST((&SupportsActionTestNew<InstallAction>::supports_action_test_new)), 0);
        rb_define_method(c_supports_install_action_test, "initialize", RUBY_FUNC_CAST(&empty_init), -1);

        /*
         * Document-class: Paludis::SupportsUninstallActionTest
         *
         * Tests whether a Paludis::PackageID supports a Paludis::UninstallAction.
         */
        c_supports_uninstall_action_test = rb_define_class_under(paludis_module(), "SupportsUninstallActionTest", c_supports_action_test_base);
        rb_define_singleton_method(c_supports_uninstall_action_test, "new",
                RUBY_FUNC_CAST((&SupportsActionTestNew<UninstallAction>::supports_action_test_new)), 0);
        rb_define_method(c_supports_uninstall_action_test, "initialize", RUBY_FUNC_CAST(&empty_init), -1);

        /*
         * Document-class: Paludis::Action
         *
         * Base class for actions, used by Paludis::PackageID#perform_action.
         */
        c_action = rb_define_class_under(paludis_module(), "Action", rb_cObject);
        rb_funcall(c_action, rb_intern("private_class_method"), 1, rb_str_new2("new"));

        /*
         * Document-class: Paludis::FetchAction
         *
         * An action for fetching.
         */
        c_fetch_action = rb_define_class_under(paludis_module(), "FetchAction", c_action);
        rb_define_singleton_method(c_fetch_action, "new", RUBY_FUNC_CAST(&fetch_action_new), 1);
        rb_define_method(c_fetch_action, "initialize", RUBY_FUNC_CAST(&empty_init), -1);
        rb_define_method(c_fetch_action, "options", RUBY_FUNC_CAST(&fetch_action_options), 0);

        /*
         * Document-class: Paludis::FetchActionOptions
         *
         * Options for Paludis::FetchAction.
         */
        c_fetch_action_options = rb_define_class_under(paludis_module(), "FetchActionOptions", rb_cObject);
        rb_define_singleton_method(c_fetch_action_options, "new", RUBY_FUNC_CAST(&fetch_action_options_new), -1);
        rb_define_method(c_fetch_action_options, "initialize", RUBY_FUNC_CAST(&empty_init), -1);
        rb_define_method(c_fetch_action_options, "fetch_unneeded?",
                RUBY_FUNC_CAST((&BoolFetch<FetchActionOptions, &FetchActionOptions::fetch_unneeded>::fetch)), 0);
        rb_define_method(c_fetch_action_options, "safe_resume?",
                RUBY_FUNC_CAST((&BoolFetch<FetchActionOptions, &FetchActionOptions::safe_resume>::fetch)), 0);

        /*
         * Document-class: Paludis::FetchActionFailure
         *
         * A failed fetch action part.
         */
        c_fetch_action_failure = rb_define_class_under(paludis_module(), "FetchActionFailure", rb_cObject);
        rb_define_singleton_method(c_fetch_action_failure, "new", RUBY_FUNC_CAST(&fetch_action_failure_new), -1);
        rb_define_method(c_fetch_action_failure, "initialize", RUBY_FUNC_CAST(&empty_init), -1);
        rb_define_method(c_fetch_action_failure, "target_file",
                RUBY_FUNC_CAST((&FailureStringFetch<&FetchActionFailure::target_file>::fetch)), 0);
        rb_define_method(c_fetch_action_failure, "requires_manual_fetching?",
                RUBY_FUNC_CAST((&BoolFetch<FetchActionFailure, &FetchActionFailure::requires_manual_fetching>::fetch)), 0);
        rb_define_method(c_fetch_action_failure, "failed_automatic_fetching?",
                RUBY_FUNC_CAST((&BoolFetch<FetchActionFailure, &FetchActionFailure::failed_automatic_fetching>::fetch)), 0);
        rb_define_method(c_fetch_action_failure, "failed_integrity_checks",
                RUBY_FUNC_CAST((&FailureStringFetch<&FetchActionFailure::failed_integrity_checks>::fetch)), 0);

        /*
         * Document-class: Paludis::InfoAction
         *
         * An action for fetching.
         */
        c_info_action = rb_define_class_under(paludis_module(), "InfoAction", c_action);
        rb_define_singleton_method(c_info_action, "new",
                RUBY_FUNC_CAST((&EasyActionNew<InfoAction>::easy_action_new)), 0);
        rb_define_method(c_info_action, "initialize", RUBY_FUNC_CAST(&empty_init), -1);

        /*
         * Document-class: Paludis::ConfigAction
         *
         * An action for fetching.
         */
        c_config_action = rb_define_class_under(paludis_module(), "ConfigAction", c_action);
        rb_define_singleton_method(c_config_action, "new",
                RUBY_FUNC_CAST((&EasyActionNew<ConfigAction>::easy_action_new)), 0);
        rb_define_method(c_config_action, "initialize", RUBY_FUNC_CAST(&empty_init), -1);

        /*
         * Document-module: Paludis::InstallActionDebugOption
         *
         * Debug build mode for an InstallAction.
         *
         * May be ignored by some repositories, and by packages where there isn't a sensible concept of debugging.
         *
         */
        c_install_action_debug_option = rb_define_module_under(paludis_module(), "InstallActionDebugOption");
        for (InstallActionDebugOption l(static_cast<InstallActionDebugOption>(0)), l_end(last_iado) ; l != l_end ;
                l = static_cast<InstallActionDebugOption>(static_cast<int>(l) + 1))
            rb_define_const(c_install_action_debug_option, value_case_to_RubyCase(stringify(l)).c_str(), INT2FIX(l));

        // cc_enum_special<paludis/action-se.hh, InstallActionDebugOption, c_install_action_debug_option>

        /*
         * Document-module: Paludis::InstallActionChecksOption
         *
         * Whether to run post-build checks (for example, 'make check' or 'src_test'), if they are available.
         *
         */
        c_install_action_checks_option = rb_define_module_under(paludis_module(), "InstallActionChecksOption");
        for (InstallActionChecksOption l(static_cast<InstallActionChecksOption>(0)), l_end(last_iaco) ; l != l_end ;
                l = static_cast<InstallActionChecksOption>(static_cast<int>(l) + 1))
            rb_define_const(c_install_action_checks_option, value_case_to_RubyCase(stringify(l)).c_str(), INT2FIX(l));

        // cc_enum_special<paludis/action-se.hh, InstallActionChecksOption, c_install_action_checks_option>

        /*
         * Document-class: Paludis::InstallActionOptions
         *
         * Options for Paludis::InstallAction.
         */
        c_install_action_options = rb_define_class_under(paludis_module(), "InstallActionOptions", rb_cObject);
        rb_define_singleton_method(c_install_action_options, "new", RUBY_FUNC_CAST(&install_action_options_new), -1);
        rb_define_method(c_install_action_options, "initialize", RUBY_FUNC_CAST(&empty_init), -1);
        rb_define_method(c_install_action_options, "no_config_protect?",
                RUBY_FUNC_CAST((&BoolFetch<InstallActionOptions, &InstallActionOptions::no_config_protect>::fetch)), 0);
        rb_define_method(c_install_action_options, "debug_build",
                RUBY_FUNC_CAST((&IAOMember<InstallActionDebugOption, &InstallActionOptions::debug_build>::fetch)), 0);
        rb_define_method(c_install_action_options, "checks",
                RUBY_FUNC_CAST((&IAOMember<InstallActionChecksOption, &InstallActionOptions::checks>::fetch)), 0);
        rb_define_method(c_install_action_options, "destination", RUBY_FUNC_CAST(&install_action_options_destination), 0);

        /*
         * Document-class: Paludis::InstallAction
         *
         * An InstallAction is used by InstallTask to install a PackageID.
         */
        c_install_action = rb_define_class_under(paludis_module(), "InstallAction", c_action);
        rb_define_singleton_method(c_install_action, "new", RUBY_FUNC_CAST(&install_action_new), 1);
        rb_define_method(c_install_action, "initialize", RUBY_FUNC_CAST(&empty_init), -1);
        rb_define_method(c_install_action, "options", RUBY_FUNC_CAST(&install_action_options), 0);

        /*
         * Document-class: Paludis::UninstallActionOptions
         *
         * Options for Paludis::UninstallAction.
         */
        c_uninstall_action_options = rb_define_class_under(paludis_module(), "UninstallActionOptions", rb_cObject);
        rb_define_singleton_method(c_uninstall_action_options, "new", RUBY_FUNC_CAST(&uninstall_action_options_new), -1);
        rb_define_method(c_uninstall_action_options, "initialize", RUBY_FUNC_CAST(&empty_init), -1);
        rb_define_method(c_uninstall_action_options, "no_config_protect?",
                RUBY_FUNC_CAST((&BoolFetch<UninstallActionOptions, &UninstallActionOptions::no_config_protect>::fetch)), 0);

        /*
         * Document-class: Paludis::UninstallAction
         *
         * An UninstallAction is used by UninstallTask to uninstall a PackageID.
         */
        c_uninstall_action = rb_define_class_under(paludis_module(), "UninstallAction", c_action);
        rb_define_singleton_method(c_uninstall_action, "new", RUBY_FUNC_CAST(&uninstall_action_new), 1);
        rb_define_method(c_uninstall_action, "initialize", RUBY_FUNC_CAST(&empty_init), -1);
        rb_define_method(c_uninstall_action, "options", RUBY_FUNC_CAST(&uninstall_action_options), 0);
    }
}

tr1::shared_ptr<const SupportsActionTestBase>
paludis::ruby::value_to_supports_action_test_base(VALUE v)
{
    if (rb_obj_is_kind_of(v, c_supports_action_test_base))
    {
        tr1::shared_ptr<const SupportsActionTestBase> * v_ptr;
        Data_Get_Struct(v, tr1::shared_ptr<const SupportsActionTestBase>, v_ptr);
        return *v_ptr;
    }
    else
    {
        rb_raise(rb_eTypeError, "Can't convert %s into SupportsActionTestBase", rb_obj_classname(v));
    }

}

tr1::shared_ptr<Action>
paludis::ruby::value_to_action(VALUE v)
{
    if (rb_obj_is_kind_of(v, c_action))
    {
        tr1::shared_ptr<Action> * v_ptr;
        Data_Get_Struct(v, tr1::shared_ptr<Action>, v_ptr);
        return *v_ptr;
    }
    else
    {
        rb_raise(rb_eTypeError, "Can't convert %s into Action", rb_obj_classname(v));
    }
}

VALUE
paludis::ruby::fetch_action_failure_to_value(const FetchActionFailure & m)
{
    FetchActionFailure * m_ptr(new FetchActionFailure(m));
    try
    {
        return Data_Wrap_Struct(c_fetch_action_failure, 0, &Common<FetchActionFailure>::free, m_ptr);
    }
    catch (const std::exception & e)
    {
        delete m_ptr;
        exception_to_ruby_exception(e);
    }
}

RegisterRubyClass::Register paludis_ruby_register_action PALUDIS_ATTRIBUTE((used))
    (&do_register_action);

