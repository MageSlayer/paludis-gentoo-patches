/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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
#include <paludis/util/make_named_values.hh>
#include <paludis/standard_output_manager.hh>
#include <ruby.h>

using namespace paludis;
using namespace paludis::ruby;

namespace
{
    static VALUE c_supports_action_test;

    static VALUE c_action;

    static VALUE c_fetch_action;
    static VALUE c_fetch_action_options;
    static VALUE c_fetch_action_failure;

    static VALUE c_info_action_options;
    static VALUE c_info_action;

    static VALUE c_config_action_options;
    static VALUE c_config_action;

    static VALUE c_install_action_options;
    static VALUE c_install_action;

    static VALUE c_uninstall_action_options;
    static VALUE c_uninstall_action;

    static VALUE c_pretend_action_options;
    static VALUE c_pretend_action;

    static VALUE c_pretend_fetch_action;

    template <typename OptionClass>
    struct BoxedOptionsTraits;

    template <typename ActionClass>
    struct BoxedActionTraits;

    template <>
    struct BoxedOptionsTraits<InfoActionOptions>
    {
        using ActionClass = InfoAction;
        static constexpr const char *OptionClassName = "InfoActionOptions";
        static VALUE BoxedType() { return c_info_action_options; }
    };

    template <>
    struct BoxedActionTraits<InfoAction>
    {
        using OptionsClass = InfoActionOptions;
        static constexpr const char *ActionClassName = "InfoAction";
        static VALUE BoxedType() { return c_info_action; }
    };

    template <>
    struct BoxedOptionsTraits<ConfigActionOptions>
    {
        using ActionClass = ConfigAction;
        static constexpr const char *OptionClassName = "ConfigActionOptions";
        static VALUE BoxedType() { return c_config_action_options; }
    };

    template <>
    struct BoxedActionTraits<ConfigAction>
    {
        using OptionsClass = ConfigActionOptions;
        static constexpr const char *ActionClassName = "ConfigAction";
        static VALUE BoxedType() { return c_config_action; }
    };

    template <>
    struct BoxedOptionsTraits<FetchActionOptions>
    {
        using ActionClass = FetchAction;
        static constexpr const char *OptionClassName = "FetchActionOptions";
        static VALUE BoxedType() { return c_fetch_action_options; }
    };

    template <>
    struct BoxedActionTraits<FetchAction>
    {
        using OptionsClass = FetchActionOptions;
        static constexpr const char *ActionClassName = "FetchAction";
        static VALUE BoxedType() { return c_fetch_action; }
    };

    template <>
    struct BoxedOptionsTraits<InstallActionOptions>
    {
        using ActionClass = InstallAction;
        static constexpr const char *OptionClassName = "InstallActionOptions";
        static VALUE BoxedType() { return c_install_action_options; }
    };

    template <>
    struct BoxedActionTraits<InstallAction>
    {
        using OptionsClass = InstallActionOptions;
        static constexpr const char *ActionClassName = "InstallAction";
        static VALUE BoxedType() { return c_install_action; }
    };

    template <>
    struct BoxedOptionsTraits<PretendActionOptions>
    {
        using ActionClass = PretendAction;
        static constexpr const char *OptionClassName = "PretendActionOptions";
        static VALUE BoxedType() { return c_pretend_action_options; }
    };

    template <>
    struct BoxedActionTraits<PretendAction>
    {
        using OptionsClass = PretendActionOptions;
        static constexpr const char *ActionClassName = "PretendAction";
        static VALUE BoxedType() { return c_pretend_action; }
    };

    template <>
    struct BoxedOptionsTraits<UninstallActionOptions>
    {
        using ActionClass = UninstallAction;
        static constexpr const char *OptionClassName = "UninstallActionOptions";
        static VALUE BoxedType() { return c_uninstall_action_options; }
    };

    template <>
    struct BoxedActionTraits<UninstallAction>
    {
        using OptionsClass = UninstallActionOptions;
        static constexpr const char *OptionClassName = "UninstallAction";
        static VALUE BoxedType() { return c_uninstall_action; }
    };

    /*
     * Document-method: destination
     *
     * call-seq:
     *     destination -> Repository
     *
     * Our destination
     */
    /*
     * Document-method: destination
     *
     * call-seq:
     *     destination -> Repository
     *
     * Our destination
     */
    template <typename OptionClass, typename Traits = BoxedOptionsTraits<OptionClass>>
    struct BoxedOptions
    {
        static VALUE box(const OptionClass &options)
        {
            OptionClass *value = new OptionClass(options);
            try
            {
                return Data_Wrap_Struct(Traits::BoxedType(), 0, &Common<OptionClass>::free, value);
            }
            catch (const std::exception & e)
            {
                delete value;
                exception_to_ruby_exception(e);
            }
        }

        static const OptionClass unbox(VALUE value)
        {
            if (rb_obj_is_kind_of(value, Traits::BoxedType()))
            {
                OptionClass *instance;
                Data_Get_Struct(value, OptionClass, instance);
                return *instance;
            }
            else
            {
                rb_raise(rb_eTypeError, "Can't convert %s into %s",
                         rb_obj_classname(value), Traits::OptionClassName);
            }
        }

        static VALUE destination(VALUE self)
        {
            OptionClass *instance;
            Data_Get_Struct(self, OptionClass, instance);
            return repository_to_value(instance->destination());
        }
    };

    /*
     * call-seq:
     *     options -> FetchActionOptions
     *
     * Our FetchActionOptions.
     */
    /*
     * call-seq:
     *     options -> InstallActionOptions
     *
     * Our InstallActionOptions.
     */
    /*
     * call-seq:
     *     options -> PretendActionOptions
     *
     * Our PretendActionOptions.
     */
    /*
     * call-seq:
     *     options -> UninstallActionOptions
     *
     * Our UninstallActionOptions.
     */
    /*
     * call-seq:
     *     options -> InfoActionOptions
     *
     * Our InfoActionOptions.
     */
    /*
     * call-seq:
     *     options -> ConfigActionOptions
     *
     * Our ConfigActionOptions.
     */
    template <typename ActionClass, typename Traits = BoxedActionTraits<ActionClass>>
    struct BoxedAction
    {
        static VALUE options(VALUE self)
        {
            std::shared_ptr<Action> *instance;
            Data_Get_Struct(self, std::shared_ptr<Action>, instance);
            return BoxedOptions<typename Traits::OptionsClass>::box(
                std::static_pointer_cast<ActionClass>(*instance)->options);
        }
    };

    WantPhase want_all_phases(const std::string &)
    {
        return wp_yes;
    }

    VALUE
    empty_init(int, VALUE *, VALUE self)
    {
        return self;
    }

    /*
     * Document-method: SupportsActionTest.new
     *
     * call-seq:
     *     SupportsActionTest.new(ActionClass) -> SupportsActionTest
     *
     * Create new SupportsActionTest object. The ActionClass should be, e.g. InstallAction.
     */
    static VALUE
    supports_action_test_new(VALUE self, VALUE action_class)
    {
        std::shared_ptr<const SupportsActionTestBase> * ptr(0);

        try
        {
            if (Qtrue == rb_funcall2(action_class, rb_intern("<="), 1, install_action_value_ptr()))
                ptr = new std::shared_ptr<const SupportsActionTestBase>(std::make_shared<SupportsActionTest<InstallAction>>());
            else if (Qtrue == rb_funcall2(action_class, rb_intern("<="), 1, uninstall_action_value_ptr()))
                ptr = new std::shared_ptr<const SupportsActionTestBase>(std::make_shared<SupportsActionTest<UninstallAction>>());
            else if (Qtrue == rb_funcall2(action_class, rb_intern("<="), 1, pretend_action_value_ptr()))
                ptr = new std::shared_ptr<const SupportsActionTestBase>(std::make_shared<SupportsActionTest<PretendAction>>());
            else if (Qtrue == rb_funcall2(action_class, rb_intern("<="), 1, config_action_value_ptr()))
                ptr = new std::shared_ptr<const SupportsActionTestBase>(std::make_shared<SupportsActionTest<ConfigAction>>());
            else if (Qtrue == rb_funcall2(action_class, rb_intern("<="), 1, fetch_action_value_ptr()))
                ptr = new std::shared_ptr<const SupportsActionTestBase>(std::make_shared<SupportsActionTest<FetchAction>>());
            else if (Qtrue == rb_funcall2(action_class, rb_intern("<="), 1, info_action_value_ptr()))
                ptr = new std::shared_ptr<const SupportsActionTestBase>(std::make_shared<SupportsActionTest<InfoAction>>());
            else if (Qtrue == rb_funcall2(action_class, rb_intern("<="), 1, pretend_fetch_action_value_ptr()))
                ptr = new std::shared_ptr<const SupportsActionTestBase>(std::make_shared<SupportsActionTest<PretendFetchAction>>());
            else
                rb_raise(rb_eTypeError, "Can't convert %s into an Action subclass", rb_obj_classname(action_class));

            VALUE tdata(Data_Wrap_Struct(self, 0, &Common<std::shared_ptr<const SupportsActionTestBase> >::free, ptr));
            rb_obj_call_init(tdata, 0, &self);
            return tdata;
        }
        catch (const std::exception & e)
        {
            delete ptr;
            exception_to_ruby_exception(e);
        }
    }

    std::shared_ptr<OutputManager> make_standard_output_manager(const Action &)
    {
        return std::make_shared<StandardOutputManager>();
    }

    /*
     * call-seq:
     *     FetchActionOptions.new(exclude_unmirrorable, fetch_unneeded, safe_resume) -> FetchActionOptions
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
            bool v_exclude_unmirrorable;
            bool v_fetch_unneeded;
            bool v_safe_resume;

            if (1 == argc && rb_obj_is_kind_of(argv[0], rb_cHash))
            {
                if (Qnil == rb_hash_aref(argv[0], ID2SYM(rb_intern("fetch_unneeded"))))
                    rb_raise(rb_eArgError, "Missing Parameter: fetch_unneeded");
                if (Qnil == rb_hash_aref(argv[0], ID2SYM(rb_intern("safe_resume"))))
                    rb_raise(rb_eArgError, "Missing Parameter: safe_resume");
                v_fetch_unneeded = rb_hash_aref(argv[0], ID2SYM(rb_intern("fetch_unneeded"))) != Qfalse;
                v_safe_resume = rb_hash_aref(argv[0], ID2SYM(rb_intern("safe_resume"))) != Qfalse;
                v_exclude_unmirrorable = rb_hash_aref(argv[0], ID2SYM(rb_intern("exclude_unmirrorable"))) != Qfalse;
            }
            else if (3 == argc)
            {
                v_exclude_unmirrorable    = argv[0] != Qfalse;
                v_fetch_unneeded          = argv[0] != Qfalse;
                v_safe_resume             = argv[1] != Qfalse;
            }
            else
            {
                rb_raise(rb_eArgError, "FetchActionOptions expects one or three arguments, but got %d",argc);
            }

            FetchParts parts;
            parts = parts + fp_regulars + fp_extras;
            if (v_fetch_unneeded)
                parts += fp_unneeded;

            ptr = new FetchActionOptions(make_named_values<FetchActionOptions>(
                        n::errors() = std::make_shared<Sequence<FetchActionFailure>>(),
                        n::exclude_unmirrorable() = v_exclude_unmirrorable,
                        n::fetch_parts() = parts,
                        n::ignore_not_in_manifest() = false,
                        n::ignore_unfetched() = false,
                        n::make_output_manager() = &make_standard_output_manager,
                        n::safe_resume() = v_safe_resume,
                        n::want_phase() = &want_all_phases
                    ));

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
     *      InfoAction.new -> InfoAction
     */
    VALUE
    info_action_new(VALUE self, VALUE opts)
    {
        auto options = BoxedOptions<InfoActionOptions>::unbox(opts);
        std::shared_ptr<Action> *action = new std::shared_ptr<Action>(std::make_shared<InfoAction>(options));
        VALUE object = Data_Wrap_Struct(self, 0, &Common<std::shared_ptr<Action>>::free, action);
        rb_obj_call_init(object, 1, &self);
        return object;
    }

    /*
     * call-seq:
     *      ConfigAction.new -> ConfigAction
     */
    VALUE
    config_action_new(VALUE self, VALUE opts)
    {
        auto options = BoxedOptions<ConfigActionOptions>::unbox(opts);
        std::shared_ptr<Action> *action = new std::shared_ptr<Action>(std::make_shared<ConfigAction>(options));
        VALUE object = Data_Wrap_Struct(self, 0, &Common<std::shared_ptr<Action>>::free, action); rb_obj_call_init(object, 1, &self);
        return object;
    }

    /*
     * call-seq:
     *     InfoActionOptions.new() -> InfoActionOptions
     *     InfoActionOptions.new(Hash) -> InfoActionOptions
     *
     * InfoActionOptions.new can either be called with all parameters in order,
     * or with one hash parameter, where the hash keys are symbols with the
     * names above.
     */
    VALUE
    info_action_options_new(int argc, VALUE *argv, VALUE self)
    {
        InfoActionOptions *options = nullptr;
        try
        {
            if (argc == 1 && rb_obj_is_kind_of(argv[0], rb_cHash))
            {
                // TODO(compnerd) raise an error if hashtable is not empty
            }
            else if (argc == 0)
            {
            }
            else
            {
                rb_raise(rb_eArgError, "InfoActionOptions expects zero or one arguments, but got %d", argc);
            }

            options =
                new InfoActionOptions(make_named_values<InfoActionOptions>(
                    n::make_output_manager() = &make_standard_output_manager));
            VALUE object = Data_Wrap_Struct(self, 0, &Common<InfoActionOptions>::free, options);
            rb_obj_call_init(object, argc, argv);
            return object;
        }
        catch (const std::exception & e)
        {
            delete options;
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     ConfigActionOptions.new() -> ConfigActionOptions
     *     ConfigActionOptions.new(Hash) -> ConfigActionOptions
     *
     * ConfigActionOptions.new can either be called with all parameters in
     * order, or with one hash parameter, where the hash keys are symbols with
     * the names above.
     */
    VALUE
    config_action_options_new(int argc, VALUE *argv, VALUE self)
    {
        ConfigActionOptions *options = nullptr;
        try
        {
            if (argc == 1 && rb_obj_is_kind_of(argv[0], rb_cHash))
            {
                // TODO(compnerd) raise an error if hashtable is not empty
            }
            else if (argc == 0)
            {
            }
            else
            {
                rb_raise(rb_eArgError, "ConfigActionOptions expects zero or one arguments, but got %d", argc);
            }

            options =
                new ConfigActionOptions(make_named_values<ConfigActionOptions>(
                    n::make_output_manager() = &make_standard_output_manager));
            VALUE object = Data_Wrap_Struct(self, 0, &Common<ConfigActionOptions>::free, options);
            rb_obj_call_init(object, argc, argv);
            return object;
        }
        catch (const std::exception & e)
        {
            delete options;
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
        auto options = BoxedOptions<FetchActionOptions>::unbox(opts);
        std::shared_ptr<Action> * a(
                new std::shared_ptr<Action>(std::make_shared<FetchAction>(options)));
        VALUE tdata(Data_Wrap_Struct(self, 0, &Common<std::shared_ptr<Action> >::free, a));
        rb_obj_call_init(tdata, 1, &self);
        return tdata;
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

            ptr = new FetchActionFailure(make_named_values<FetchActionFailure>(
                        n::failed_automatic_fetching() = v_failed_automatic_fetching,
                        n::failed_integrity_checks() = v_failed_integrity_checks,
                        n::requires_manual_fetching() = v_requires_manual_fetching,
                        n::target_file() = v_target_file
                    ));

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
    template <typename T_, typename K_, typename R_, NamedValue<K_, R_> (T_::*) >
    struct NVFetch;

    template <typename T_, typename K_, NamedValue<K_, bool> (T_::* f_) >
    struct NVFetch<T_, K_, bool, f_>
    {
        static VALUE
        fetch(VALUE self)
        {
            T_ * p;
            Data_Get_Struct(self, T_, p);
            return bool_to_value((p->*f_)());
        }
    };

    template <typename T_, typename K_, NamedValue<K_, std::string> (T_::* f_) >
    struct NVFetch<T_, K_, std::string, f_>
    {
        static VALUE
        fetch(VALUE self)
        {
            T_ * p;
            Data_Get_Struct(self, T_, p);
            return rb_str_new2((p->*f_)().c_str());
        }
    };

    void cannot_perform_uninstall(const std::shared_ptr<const PackageID> & id, const UninstallActionOptions &)
    {
        throw InternalError(PALUDIS_HERE, "Can't uninstall '" + stringify(*id) + "'");
    }

    /*
     * call-seq:
     *     InstallActionOptions.new(destination) -> InstallActionOptions
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
            std::shared_ptr<Repository> v_destination;

            if (1 == argc && rb_obj_is_kind_of(argv[0], rb_cHash))
            {
                if (Qnil == rb_hash_aref(argv[0], ID2SYM(rb_intern("destination"))))
                    rb_raise(rb_eArgError, "Missing Parameter: destination");
                v_destination = value_to_repository(rb_hash_aref(argv[0], ID2SYM(rb_intern("destination"))));
            }
            else if (1 == argc)
            {
                v_destination = value_to_repository(argv[0]);
            }
            else
            {
                rb_raise(rb_eArgError, "InstallActionOptions expects one argument, but got %d",argc);
            }

            ptr = new InstallActionOptions(make_named_values<InstallActionOptions>(
                        n::destination() = v_destination,
                        n::make_output_manager() = &make_standard_output_manager,
                        n::perform_uninstall() = &cannot_perform_uninstall,
                        n::replacing() = std::make_shared<PackageIDSequence>(),
                        n::want_phase() = &want_all_phases
                    ));

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
     * call-seq:
     *     PretendActionOptions.new(destination) -> InstallActionOptions
     */
    VALUE
    pretend_action_options_new(int argc, VALUE *argv, VALUE self)
    {
        PretendActionOptions * ptr(0);
        try
        {
            std::shared_ptr<Repository> v_destination;

            if (1 == argc)
            {
                v_destination = value_to_repository(argv[0]);
            }
            else
            {
                rb_raise(rb_eArgError, "PretendActionOptions expects one argument, but got %d",argc);
            }

            ptr = new PretendActionOptions(make_named_values<PretendActionOptions>(
                        n::destination() = v_destination,
                        n::make_output_manager() = &make_standard_output_manager,
                        n::replacing() = std::make_shared<PackageIDSequence>()
                    ));

            VALUE tdata(Data_Wrap_Struct(self, 0, &Common<PretendActionOptions>::free, ptr));
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
     *     InstallAction.new(install_action_options) -> InstallAction
     *
     * Create a new InstallAction.
     */
    VALUE
    install_action_new(VALUE self, VALUE opts)
    {
        auto options = BoxedOptions<InstallActionOptions>::unbox(opts);
        std::shared_ptr<Action> * a(
                new std::shared_ptr<Action>(std::make_shared<InstallAction>(options)));
        VALUE tdata(Data_Wrap_Struct(self, 0, &Common<std::shared_ptr<Action> >::free, a));
        rb_obj_call_init(tdata, 1, &self);
        return tdata;
    }

    /*
     * call-seq:
     *     PretendAction.new(pretend_action_options) -> PretendAction
     *
     * Create a new PretendAction.
     */
    VALUE
    pretend_action_new(VALUE self, VALUE opts)
    {
        auto options = BoxedOptions<PretendActionOptions>::unbox(opts);
        std::shared_ptr<Action> * a(
                new std::shared_ptr<Action>(std::make_shared<PretendAction>(options)));
        VALUE tdata(Data_Wrap_Struct(self, 0, &Common<std::shared_ptr<Action> >::free, a));
        rb_obj_call_init(tdata, 1, &self);
        return tdata;
    }

    bool ignore_nothing(const FSPath &)
    {
        return false;
    }

    /*
     * call-seq:
     *     UninstallActionOptions.new(config_protect) -> UninstallActionOptions
     */
    VALUE
    uninstall_action_options_new(int argc, VALUE *argv, VALUE self)
    {
        UninstallActionOptions * ptr(0);
        try
        {
            std::string v_config_protect;

            if (1 == argc)
            {
                v_config_protect = std::string(StringValuePtr(argv[0]));
            }
            else
            {
                rb_raise(rb_eArgError, "UninstallActionOptions expects one argument, but got %d",argc);
            }

            ptr = new UninstallActionOptions(make_named_values<UninstallActionOptions>(
                        n::config_protect() = v_config_protect,
                        n::if_for_install_id() = nullptr,
                        n::ignore_for_unmerge() = &ignore_nothing,
                        n::is_overwrite() = false,
                        n::make_output_manager() = &make_standard_output_manager,
                        n::override_contents() = nullptr,
                        n::want_phase() = &want_all_phases
                    ));

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
     *     UninstallAction.new(uninstall_action_options) -> UninstallAction
     *
     * Create a new UninstallAction.
     */
    VALUE
    uninstall_action_new(VALUE self, VALUE opts)
    {
        auto options = BoxedOptions<UninstallActionOptions>::unbox(opts);
        std::shared_ptr<Action> * a(new std::shared_ptr<Action>(std::make_shared<UninstallAction>(options)));
        VALUE tdata(Data_Wrap_Struct(self, 0, &Common<std::shared_ptr<Action> >::free, a));
        rb_obj_call_init(tdata, 1, &self);
        return tdata;
    }

    /*
     * Document-method: config_protect
     *
     * call-seq:
     *     config_protect -> String
     *
     * Our config_protect value
     */
    VALUE
    uninstall_action_options_config_protect(VALUE self)
    {
        UninstallActionOptions * p;
        Data_Get_Struct(self, UninstallActionOptions, p);
        return rb_str_new2((*p).config_protect().c_str());
    }

    /*
     * call-seq:
     *     failed? -> true or false
     *
     * Did our pretend phase fail?
     */
    VALUE
    pretend_action_failed(VALUE self)
    {
        std::shared_ptr<Action> * p;
        Data_Get_Struct(self, std::shared_ptr<Action>, p);
        return bool_to_value(std::static_pointer_cast<PretendAction>(*p)->failed());
    }

    /*
     * call-seq:
     *     set_failed -> Qnil
     *
     * Mark the action as failed.
     */
    VALUE
    pretend_action_set_failed(VALUE self)
    {
        std::shared_ptr<Action> * p;
        Data_Get_Struct(self, std::shared_ptr<Action>, p);
        std::static_pointer_cast<PretendAction>(*p)->set_failed();
        return Qnil;
    }

    void do_register_action()
    {
        /*
         * Document-class: Paludis::SupportsActionTest
         *
         * Tests whether a Paludis::PackageID supports a particular action.
         */
        c_supports_action_test = rb_define_class_under(paludis_module(), "SupportsActionTest", rb_cObject);
        rb_define_singleton_method(c_supports_action_test, "new", RUBY_FUNC_CAST(&supports_action_test_new), 1);

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
        rb_define_method(c_fetch_action, "options", RUBY_FUNC_CAST(&BoxedAction<FetchAction>::options), 0);

        /*
         * Document-class: Paludis::FetchActionOptions
         *
         * Options for Paludis::FetchAction.
         */
        c_fetch_action_options = rb_define_class_under(paludis_module(), "FetchActionOptions", rb_cObject);
        rb_define_singleton_method(c_fetch_action_options, "new", RUBY_FUNC_CAST(&fetch_action_options_new), -1);
        rb_define_method(c_fetch_action_options, "initialize", RUBY_FUNC_CAST(&empty_init), -1);
        rb_define_method(c_fetch_action_options, "safe_resume?",
                RUBY_FUNC_CAST((&NVFetch<FetchActionOptions, n::safe_resume, bool,
                        &FetchActionOptions::safe_resume>::fetch)), 0);
        rb_define_method(c_fetch_action_options, "exclude_unmirrorable?",
                RUBY_FUNC_CAST((&NVFetch<FetchActionOptions, n::exclude_unmirrorable, bool,
                        &FetchActionOptions::exclude_unmirrorable>::fetch)), 0);

        /*
         * Document-class: Paludis::FetchActionFailure
         *
         * A failed fetch action part.
         */
        c_fetch_action_failure = rb_define_class_under(paludis_module(), "FetchActionFailure", rb_cObject);
        rb_define_singleton_method(c_fetch_action_failure, "new", RUBY_FUNC_CAST(&fetch_action_failure_new), -1);
        rb_define_method(c_fetch_action_failure, "initialize", RUBY_FUNC_CAST(&empty_init), -1);
        rb_define_method(c_fetch_action_failure, "target_file",
                RUBY_FUNC_CAST((&NVFetch<FetchActionFailure, n::target_file, std::string,
                        &FetchActionFailure::target_file>::fetch)), 0);
        rb_define_method(c_fetch_action_failure, "requires_manual_fetching?",
                RUBY_FUNC_CAST((&NVFetch<FetchActionFailure, n::requires_manual_fetching, bool,
                        &FetchActionFailure::requires_manual_fetching>::fetch)), 0);
        rb_define_method(c_fetch_action_failure, "failed_automatic_fetching?",
                RUBY_FUNC_CAST((&NVFetch<FetchActionFailure, n::failed_automatic_fetching, bool,
                        &FetchActionFailure::failed_automatic_fetching>::fetch)), 0);
        rb_define_method(c_fetch_action_failure, "failed_integrity_checks",
                RUBY_FUNC_CAST((&NVFetch<FetchActionFailure, n::failed_integrity_checks, std::string,
                        &FetchActionFailure::failed_integrity_checks>::fetch)), 0);

        /*
         * Document-class: Paludis::InfoActionOptions
         *
         * Options for Paludis::InfoAction
         */
        c_info_action_options = rb_define_class_under(paludis_module(), "InfoActionOptions", rb_cObject);
        rb_define_singleton_method(c_info_action_options, "new", RUBY_FUNC_CAST(&info_action_options_new), -1);
        rb_define_method(c_info_action_options, "initialize", RUBY_FUNC_CAST(&empty_init), -1);

        /*
         * Document-class: Paludis::InfoAction
         *
         * An action for fetching.
         */
        c_info_action = rb_define_class_under(paludis_module(), "InfoAction", c_action);
        rb_define_singleton_method(c_info_action, "new", RUBY_FUNC_CAST(&info_action_new), 1);
        rb_define_method(c_info_action, "initialize", RUBY_FUNC_CAST(&empty_init), -1);
        rb_define_method(c_info_action, "options", RUBY_FUNC_CAST(&BoxedAction<InfoAction>::options), 0);

        /*
         * Document-class: Paludis::ConfigActionOptions
         *
         * Options for Paludis::ConfigAction
         */
        c_config_action_options = rb_define_class_under(paludis_module(), "ConfigActionOptions", rb_cObject);
        rb_define_singleton_method(c_config_action_options, "new", RUBY_FUNC_CAST(&config_action_options_new), -1);
        rb_define_method(c_config_action_options, "initialize", RUBY_FUNC_CAST(&empty_init), -1);

        /*
         * Document-class: Paludis::ConfigAction
         *
         * An action for fetching.
         */
        c_config_action = rb_define_class_under(paludis_module(), "ConfigAction", c_action);
        rb_define_singleton_method(c_config_action, "new", RUBY_FUNC_CAST(&config_action_new), -1);
        rb_define_method(c_config_action, "initialize", RUBY_FUNC_CAST(&empty_init), -1);
        rb_define_method(c_config_action, "options", RUBY_FUNC_CAST(&BoxedAction<ConfigAction>::options), 0);

        /*
         * Document-class: Paludis::InstallActionOptions
         *
         * Options for Paludis::InstallAction.
         */
        c_install_action_options = rb_define_class_under(paludis_module(), "InstallActionOptions", rb_cObject);
        rb_define_singleton_method(c_install_action_options, "new", RUBY_FUNC_CAST(&install_action_options_new), -1);
        rb_define_method(c_install_action_options, "initialize", RUBY_FUNC_CAST(&empty_init), -1);
        rb_define_method(c_install_action_options, "destination", RUBY_FUNC_CAST(&BoxedOptions<InstallActionOptions>::destination), 0);

        /*
         * Document-class: Paludis::InstallAction
         *
         * An InstallAction is used by InstallTask to install a PackageID.
         */
        c_install_action = rb_define_class_under(paludis_module(), "InstallAction", c_action);
        rb_define_singleton_method(c_install_action, "new", RUBY_FUNC_CAST(&install_action_new), 1);
        rb_define_method(c_install_action, "initialize", RUBY_FUNC_CAST(&empty_init), -1);
        rb_define_method(c_install_action, "options", RUBY_FUNC_CAST(&BoxedAction<InstallAction>::options), 0);

        /*
         * Document-class: Paludis::UninstallActionOptions
         *
         * Options for Paludis::UninstallAction.
         */
        c_uninstall_action_options = rb_define_class_under(paludis_module(), "UninstallActionOptions", rb_cObject);
        rb_define_singleton_method(c_uninstall_action_options, "new", RUBY_FUNC_CAST(&uninstall_action_options_new), -1);
        rb_define_method(c_uninstall_action_options, "initialize", RUBY_FUNC_CAST(&empty_init), -1);
        rb_define_method(c_uninstall_action_options, "config_protect", RUBY_FUNC_CAST(&uninstall_action_options_config_protect), 0);

        /*
         * Document-class: Paludis::UninstallAction
         *
         * An UninstallAction is used by UninstallTask to uninstall a PackageID.
         */
        c_uninstall_action = rb_define_class_under(paludis_module(), "UninstallAction", c_action);
        rb_define_singleton_method(c_uninstall_action, "new", RUBY_FUNC_CAST(&uninstall_action_new), 1);
        rb_define_method(c_uninstall_action, "initialize", RUBY_FUNC_CAST(&empty_init), -1);
        rb_define_method(c_uninstall_action, "options", RUBY_FUNC_CAST(&BoxedAction<UninstallAction>::options), 0);

        /*
         * Document-class: Paludis::PretendActionOptions
         *
         * Options for Paludis::PretendAction.
         */
        c_pretend_action_options = rb_define_class_under(paludis_module(), "PretendActionOptions", rb_cObject);
        rb_define_singleton_method(c_pretend_action_options, "new", RUBY_FUNC_CAST(&pretend_action_options_new), -1);
        rb_define_method(c_pretend_action_options, "initialize", RUBY_FUNC_CAST(&empty_init), -1);
        rb_define_method(c_pretend_action_options, "destination", RUBY_FUNC_CAST(&BoxedOptions<PretendActionOptions>::destination), 0);

        /*
         * Document-class: Paludis::PretendAction
         *
         * A PretendAction is used by InstallTask to handle install-pretend-phase checks on a PackageID.
         */
        c_pretend_action = rb_define_class_under(paludis_module(), "PretendAction", c_action);
        rb_define_singleton_method(c_pretend_action, "new", RUBY_FUNC_CAST(&pretend_action_new), 1);
        rb_define_method(c_pretend_action, "initialize", RUBY_FUNC_CAST(&empty_init), -1);
        rb_define_method(c_pretend_action, "failed?", RUBY_FUNC_CAST(&pretend_action_failed), 0);
        rb_define_method(c_pretend_action, "set_failed", RUBY_FUNC_CAST(&pretend_action_set_failed), 0);
        rb_define_method(c_pretend_action, "options", RUBY_FUNC_CAST(&BoxedAction<PretendAction>::options), 0);

        c_pretend_fetch_action = rb_define_class_under(paludis_module(), "PretendFetchAction", c_action);
        rb_funcall(c_pretend_fetch_action, rb_intern("private_class_method"), 1, rb_str_new2("new"));
    }
}

std::shared_ptr<const SupportsActionTestBase>
paludis::ruby::value_to_supports_action_test_base(VALUE v)
{
    if (rb_obj_is_kind_of(v, c_supports_action_test))
    {
        std::shared_ptr<const SupportsActionTestBase> * v_ptr;
        Data_Get_Struct(v, std::shared_ptr<const SupportsActionTestBase>, v_ptr);
        return *v_ptr;
    }
    else
    {
        rb_raise(rb_eTypeError, "Can't convert %s into SupportsActionTest", rb_obj_classname(v));
    }

}

std::shared_ptr<Action>
paludis::ruby::value_to_action(VALUE v)
{
    if (rb_obj_is_kind_of(v, c_action))
    {
        std::shared_ptr<Action> * v_ptr;
        Data_Get_Struct(v, std::shared_ptr<Action>, v_ptr);
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

VALUE *
paludis::ruby::install_action_value_ptr()
{
    return &c_install_action;
}

VALUE *
paludis::ruby::pretend_action_value_ptr()
{
    return &c_pretend_action;
}

VALUE *
paludis::ruby::fetch_action_value_ptr()
{
    return &c_fetch_action;
}

VALUE *
paludis::ruby::config_action_value_ptr()
{
    return &c_config_action;
}

VALUE *
paludis::ruby::uninstall_action_value_ptr()
{
    return &c_uninstall_action;
}

VALUE *
paludis::ruby::info_action_value_ptr()
{
    return &c_info_action;
}

VALUE *
paludis::ruby::pretend_fetch_action_value_ptr()
{
    return &c_pretend_fetch_action;
}

RegisterRubyClass::Register paludis_ruby_register_action PALUDIS_ATTRIBUTE((used))
    (&do_register_action);

