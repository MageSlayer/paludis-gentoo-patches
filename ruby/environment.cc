/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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
#include <paludis/environments/paludis/paludis_environment.hh>
#include <paludis/environments/no_config/no_config_environment.hh>
#include <paludis/environments/environment_maker.hh>
#include <ruby.h>

using namespace paludis;
using namespace paludis::ruby;

#define RUBY_FUNC_CAST(x) reinterpret_cast<VALUE (*)(...)>(x)

namespace
{
    static VALUE c_environment;
    static VALUE c_paludis_environment;
    static VALUE c_no_config_environment;
    static VALUE c_environment_maker;

    /*
     * call-seq:
     *     query_use(use_flag, package_database_entry)
     *
     * Does the user want the specified USE flag set for a PackageDatabaseEntry.
     */
    VALUE
    environment_query_use(VALUE self, VALUE flag, VALUE pdev)
    {
        EnvironmentData * env_data;
        Data_Get_Struct(self, EnvironmentData, env_data);

        try
        {
            PackageDatabaseEntry pde(value_to_package_database_entry(pdev));
            return env_data->env_ptr->query_use(UseFlagName(StringValuePtr(flag)), pde) ? Qtrue : Qfalse;
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     mask_reasons(package_database_entry)
     *
     * Fetch the MaskReasons for a PackageDatabaseEntry.
     */
    VALUE
    environment_mask_reasons(VALUE self, VALUE pde_value)
    {
        EnvironmentData * env_data;
        Data_Get_Struct(self, EnvironmentData, env_data);

        PackageDatabaseEntry pde = value_to_package_database_entry(pde_value);
        try
        {
            MaskReasons r(env_data->env_ptr->mask_reasons(pde));
            return mask_reasons_to_value(r);
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * Fetch our PackageDatabase.
     */
    VALUE
    environment_package_database(VALUE self)
    {
        EnvironmentData * env_data;
        Data_Get_Struct(self, EnvironmentData, env_data);

        try
        {
            return package_database_to_value(env_data->env_ptr->package_database());
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     set(set_name)
     *
     * Fetch a named package set as a DepSpec.
     */
    VALUE
    environment_set(VALUE self, VALUE set_name)
    {
        EnvironmentData * env_data;
        Data_Get_Struct(self, EnvironmentData, env_data);

        try
        {
            return dep_spec_to_value(env_data->env_ptr->set(SetName(StringValuePtr(set_name))));
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     root -> String
     *
     * Default root location, default is /.
     */
    VALUE
    environment_root(VALUE self)
    {
        EnvironmentData * env_data;
        Data_Get_Struct(self, EnvironmentData, env_data);
        return rb_str_new2(stringify(env_data->env_ptr->root()).c_str());
    }

    /*
     * call-seq:
     *     default_destinations -> Array
     *
     * Default: All repositories that provide RepositoryDestinationInterface and mark themselves
     * as a default destination.
     */
    VALUE
    environment_default_destinations(VALUE self)
    {
        EnvironmentData * env_data;
        Data_Get_Struct(self, EnvironmentData, env_data);
        std::tr1::shared_ptr<const DestinationsCollection> dc = env_data->env_ptr->default_destinations();
        VALUE result(rb_ary_new());
        for (DestinationsCollection::Iterator i(dc->begin()), i_end(dc->end()) ; i != i_end ; ++i)
            rb_ary_push(result, repository_to_value(*i));

        return result;

    }

    PaludisEnvironment *
    value_to_paludis_environment(VALUE v)
    {
        if (rb_obj_is_kind_of(v, c_paludis_environment))
        {
            return static_cast<PaludisEnvironment *>(value_to_environment_data(v)->env_ptr);
        }
        else
        {
            rb_raise(rb_eTypeError, "Can't convert %s into PaludisEnvironment", rb_obj_classname(v));
        }
    }

    VALUE
    paludis_environment_init(int, VALUE*, VALUE self)
    {
        return self;
    }

    /*
     * call-seq:
     *     PaludisEnvironment.new -> PaludisEnvironment
     *     PaludisEnvironment.new(config_suffix) -> PaludisEnvironment
     *
     * Create a new PaludisEnvironment, with the specified config suffix if any, otherwise the empty suffix.
     */
    VALUE
    paludis_environment_new(int argc, VALUE* argv, VALUE self)
    {
        try
        {
            std::string config_suffix;
            if (1 == argc)
                config_suffix = StringValuePtr(argv[0]);
            else if (0 != argc)
                rb_raise(rb_eArgError, "PaludisEnvironment.new expects one or zero arguments, but got %d", argc);

            PaludisEnvironment * e(new PaludisEnvironment(config_suffix));
            EnvironmentData * ptr(new EnvironmentData(e, e));
            VALUE tdata(Data_Wrap_Struct(self, 0, &Common<EnvironmentData>::free, ptr));
            rb_obj_call_init(tdata, argc, argv);
            return tdata;
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     config_dir -> String
     *
     * Configuration directory used by this PaludisEnvironment.
     */
    VALUE
    paludis_environment_config_dir(VALUE self)
    {
        return rb_str_new2(value_to_paludis_environment(self)->config_dir().c_str());
    }

    VALUE
    no_config_environment_init(int, VALUE*, VALUE self)
    {
        return self;
    }

    /*
     * call-seq:
     *     NoConfigEnvironment.new(environment_dir) -> NoConfigEnvironment
     *     NoConfigEnvironment.new(environment_dir, write_cache_dir) -> NoConfigEnvironment
     *     NoConfigEnvironment.new(environment_dir, write_cache_dir, master_repository_dir) -> NoConfigEnvironment
     *
     * Create a new NoConfigEnvironment from the specified directory. A write cache and master repository 
     * may also be specified.
     */
    VALUE
    no_config_environment_new(int argc, VALUE* argv, VALUE self)
    {
        try
        {
            std::string write_cache, master_repository_dir;
            if (1 == argc)
            {
                write_cache = "/var/empty/";
                master_repository_dir = "/var/empty/";
            }
            else if (2 == argc)
            {
                write_cache = StringValuePtr(argv[1]);
                master_repository_dir = "/var/empty/";
            }
            else if (3 == argc)
            {
                write_cache = StringValuePtr(argv[1]);
                master_repository_dir = StringValuePtr(argv[2]);
            }
            else
                rb_raise(rb_eArgError, "NoConfigEnvironment.new expects one to three arguments, but got %d", argc);

            std::string path;
            if (rb_obj_is_kind_of(argv[0], rb_cDir))
            {
                VALUE v = rb_funcall(argv[0], rb_intern("path"), 0);
                path = StringValuePtr(v);
            }
            else
                path = StringValuePtr(argv[0]);

            NoConfigEnvironment * e(new NoConfigEnvironment(NoConfigEnvironmentParams::create()
                        .repository_dir(FSEntry(path))
                        .write_cache(write_cache)
                        .accept_unstable(false)
                        .repository_type(ncer_auto)
                        .master_repository_dir(FSEntry(master_repository_dir))));
            EnvironmentData * ptr(new EnvironmentData(e, e));
            VALUE tdata(Data_Wrap_Struct(self, 0, &Common<EnvironmentData>::free, ptr));
            rb_obj_call_init(tdata, argc, argv);
            return tdata;
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     main_repository -> Repository
     *
     * Return the main Repository in this environment
     */
    VALUE
    no_config_environment_main_repository(VALUE self)
    {
        try
        {
            return repository_to_value(value_to_no_config_environment(self)->main_repository());
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     master_repository -> PortageRepository
     *
     * Return the master repository in this environment
     */
    VALUE
    no_config_environment_master_repository(VALUE self)
    {
        try
        {
            return repository_to_value(value_to_no_config_environment(self)->master_repository());
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     accept_unstable=(true or false)
     *
     * Should we accept unstable keywords?
     */
    VALUE
    no_config_environment_set_accept_unstable(VALUE self, VALUE unstable)
    {
        value_to_no_config_environment(self)->set_accept_unstable(!(Qfalse == unstable || Qnil == unstable));
        return Qnil;
    }

    /*
     * call-seq:
     *     make_from_spec(spec) -> Environment
     *
     * Create an environment from the given spec.
     * A spec consisits of <b>class:suffix</b> both of which may be omitted. <b>class</b> is the environment class,
     * e.g. paludis or portage, <b>suffix</b> is the configuration directory suffix.
     *
     */
    VALUE
    environment_maker_make_from_spec(VALUE, VALUE spec)
    {
        try
        {
            std::tr1::shared_ptr<Environment> e(EnvironmentMaker::get_instance()->make_from_spec(
                        StringValuePtr(spec)));

            EnvironmentData * ptr(new EnvironmentData(e.get(), 0, e));
            VALUE tdata(Data_Wrap_Struct(c_environment, 0, &Common<EnvironmentData>::free, ptr));
            return tdata;
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    void do_register_environment()
    {
        rb_require("singleton");

        /*
         * Document-class: Paludis::Environment
         *
         * Represents a working environment, which contains an available packages database and provides 
         * various methods for querying package visibility and options.
         */
        c_environment = environment_class();
        rb_funcall(c_environment, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_method(c_environment, "query_use", RUBY_FUNC_CAST(&environment_query_use), 2);
        rb_define_method(c_environment, "mask_reasons", RUBY_FUNC_CAST(&environment_mask_reasons), 1);
        rb_define_method(c_environment, "package_database", RUBY_FUNC_CAST(&environment_package_database), 0);
        rb_define_method(c_environment, "set", RUBY_FUNC_CAST(&environment_set), 1);
        rb_define_method(c_environment, "root", RUBY_FUNC_CAST(&environment_root), 0);
        rb_define_method(c_environment, "default_destinations", RUBY_FUNC_CAST(&environment_default_destinations), 0);

        /*
         * Document-class: Paludis::PaludisEnvironment
         *
         * An Environment that corresponds to the normal operating evironment.
         */
        c_paludis_environment = rb_define_class_under(paludis_module(), "PaludisEnvironment", c_environment);
        rb_define_singleton_method(c_paludis_environment, "new", RUBY_FUNC_CAST(&paludis_environment_new), -1);
        rb_define_method(c_paludis_environment, "initialize", RUBY_FUNC_CAST(&paludis_environment_init), -1);
        rb_define_method(c_paludis_environment, "config_dir", RUBY_FUNC_CAST(&paludis_environment_config_dir), 0);

        /*
         * Document-class: Paludis::NoConfigEnvironment
         *
         * An environment that uses a single repository, with no user configuration.
         */
        c_no_config_environment = no_config_environment_class();
        rb_define_singleton_method(c_no_config_environment, "new", RUBY_FUNC_CAST(&no_config_environment_new), -1);
        rb_define_method(c_no_config_environment, "initialize", RUBY_FUNC_CAST(&no_config_environment_init), -1);
        rb_define_method(c_no_config_environment, "main_repository", RUBY_FUNC_CAST(&no_config_environment_main_repository), 0);
        rb_define_method(c_no_config_environment, "master_repository", RUBY_FUNC_CAST(&no_config_environment_master_repository), 0);
        rb_define_method(c_no_config_environment, "accept_unstable=", RUBY_FUNC_CAST(&no_config_environment_set_accept_unstable), 0);

        /*
         * Document-class: Paludis::EnvironmentMaker
         *
         * A class that holds methods to create environments.
         *
         * To access the default environment use make_from_spec()
         */
        c_environment_maker = rb_define_class_under(paludis_module(), "EnvironmentMaker", rb_cObject);
        rb_funcall(rb_const_get(rb_cObject, rb_intern("Singleton")), rb_intern("included"), 1, c_environment_maker);
        rb_define_method(c_environment_maker, "make_from_spec", RUBY_FUNC_CAST(&environment_maker_make_from_spec), 1);
    }
}

EnvironmentData *
paludis::ruby::value_to_environment_data(VALUE v)
{
    if (rb_obj_is_kind_of(v, c_environment))
    {
        EnvironmentData * v_ptr;
        Data_Get_Struct(v, EnvironmentData, v_ptr);
        return v_ptr;
    }
    else
    {
        rb_raise(rb_eTypeError, "Can't convert %s into EnvironmentData", rb_obj_classname(v));
    }
}

NoConfigEnvironment *
paludis::ruby::value_to_no_config_environment(VALUE v)
{
    if (rb_obj_is_kind_of(v, c_no_config_environment))
    {
        return static_cast<NoConfigEnvironment *>(value_to_environment_data(v)->env_ptr);
    }
    else
    {
        rb_raise(rb_eTypeError, "Can't convert %s into NoConfigEnvironment", rb_obj_classname(v));
    }
}

RegisterRubyClass::Register paludis_ruby_register_environment PALUDIS_ATTRIBUTE((used))
    (&do_register_environment);

