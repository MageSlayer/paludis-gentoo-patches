/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
 * Copyright (c) 2007, 2008 Richard Brown
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
#include <paludis/environments/test/test_environment.hh>
#include <paludis/environment_factory.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/set.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/make_named_values.hh>
#include <ruby.h>

using namespace paludis;
using namespace paludis::ruby;

namespace
{
    static VALUE c_environment;
    static VALUE c_paludis_environment;
    static VALUE c_test_environment;
    static VALUE c_environment_factory;

    /*
     * call-seq:
     *     [](Selection) -> Array of PackageID
     *
     * Fetch PackageID instances using the supplied Selection.
     */

    VALUE
    environment_square_brackets(VALUE self, VALUE selection)
    {
        try
        {
            std::shared_ptr<const PackageIDSequence> ids(value_to_environment(self)->operator[] (value_to_selection(selection)));
            VALUE result(rb_ary_new());
            for (const auto & id : *ids)
                rb_ary_push(result, package_id_to_value(id));
            return result;
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     set(set_name) -> DepSpec
     *
     * Fetch a named package set as a DepSpec.
     */
    VALUE
    environment_set(VALUE self, VALUE set_name)
    {
        try
        {
            SetName s(StringValuePtr(set_name));
            std::shared_ptr<const SetSpecTree> set = (value_to_environment(self)->set(s));
            if (set)
                return dep_tree_to_value<SetSpecTree>(set);
            else
                return Qnil;
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     accept_license(license, package_id) -> true or false
     *
     * Do we accept a particular license for a particular package?
     */

    VALUE
    environment_accept_license(VALUE self, VALUE license, VALUE p)
    {
        try
        {
            return value_to_environment(self)->accept_license(
                    std::string(StringValuePtr(license)), (value_to_package_id(p))) ? Qtrue : Qfalse;
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     accept_keywords(keywords, package_id) -> true or false
     *
     * Do we accept any of the specified keywords for a particular package?
     */

    VALUE
    environment_accept_keywords(VALUE self, VALUE keywords, VALUE p)
    {
        if (rb_obj_is_kind_of(keywords, rb_cArray))
        {
            try
            {
                std::shared_ptr<KeywordNameSet> knc (std::make_shared<KeywordNameSet>());
                long len = NUM2LONG(rb_funcall(keywords,rb_intern("length"),0));
                for (long i = 0; i < len; i++)
                {
                    // Stupid macros won't let me do it on one line.
                    VALUE kw = rb_ary_entry(keywords, i);
                    knc->insert(KeywordName(StringValuePtr(kw)));
                }
                return value_to_environment(self)->accept_keywords(knc, value_to_package_id(p)) ? Qtrue : Qfalse;
            }
            catch (const std::exception & e)
            {
                exception_to_ruby_exception(e);
            }
        }
        else
        {
            rb_raise(rb_eTypeError, "Can't convert %s into Array", rb_obj_classname(keywords));
        }
    }

    /*
     * call-seq:
     *     mirrors(mirror_name) -> Array
     *
     * Return the mirror URI prefixes for a named mirror.
     */
    VALUE
    environment_mirrors(VALUE self, VALUE mirror_name)
    {
        try
        {
            VALUE result(rb_ary_new());
            std::shared_ptr<const MirrorsSequence> m(value_to_environment(self)->mirrors(StringValuePtr(mirror_name)));
            for (const auto & mirror : *m)
                rb_ary_push(result, rb_str_new2(stringify(mirror).c_str()));
            return result;
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    VALUE
    environment_distribution(VALUE self)
    {
        return rb_str_new2(value_to_environment(self)->distribution().c_str());

    }

    template <typename T_, const std::shared_ptr<const T_> (Environment::* m_) () const>
    struct EnvironmentKey
    {
        static VALUE
        fetch(VALUE self)
        {
            std::shared_ptr<Environment> * self_ptr;
            Data_Get_Struct(self, std::shared_ptr<Environment>, self_ptr);
            return (((**self_ptr).*m_)()) ? metadata_key_to_value(((**self_ptr).*m_)()) : Qnil;
        }
    };

    std::shared_ptr<PaludisEnvironment>
    value_to_paludis_environment(VALUE v)
    {
        if (rb_obj_is_kind_of(v, c_paludis_environment))
        {
            std::shared_ptr<Environment> * v_ptr;
            Data_Get_Struct(v, std::shared_ptr<Environment>, v_ptr);
            return std::static_pointer_cast<PaludisEnvironment>(*v_ptr);
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

            std::shared_ptr<Environment> * e = new std::shared_ptr<Environment>(std::make_shared<PaludisEnvironment>(config_suffix));
            VALUE tdata(Data_Wrap_Struct(self, 0, &Common<std::shared_ptr<Environment> >::free, e));
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
     *     TestEnvironment.new -> TestEnvironment
     *
     * Create a new TestEnvironment.
     */
    VALUE
    test_environment_new(VALUE self)
    {
        try
        {
            std::shared_ptr<Environment> * e = new std::shared_ptr<Environment>(std::make_shared<TestEnvironment>());
            VALUE tdata(Data_Wrap_Struct(self, 0, &Common<std::shared_ptr<Environment> >::free, e));
            rb_obj_call_init(tdata, 0, &self);
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

    /*
     * call-seq:
     *     create(spec) -> Environment
     *
     * Create an environment from the given spec.
     * A spec consisits of <b>class:suffix</b> both of which may be omitted. <b>class</b> is the environment class,
     * e.g. paludis or portage, <b>suffix</b> is the configuration directory suffix.
     *
     */
    VALUE
    environment_factory_create(VALUE, VALUE spec)
    {
        try
        {
            std::shared_ptr<Environment> * e = new std::shared_ptr<Environment>(EnvironmentFactory::get_instance()->create(
                        StringValuePtr(spec)));

            VALUE tdata(Data_Wrap_Struct(c_environment, 0, &Common<std::shared_ptr<Environment> >::free, e));
            return tdata;
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     fetch_unique_qualified_package_name(package_name) -> QualifiedPackageName
     *     fetch_unique_qualified_package_name(package_name, filter) -> QualifiedPackageName
     *
     * Disambiguate a package name.  If a filter is specified, limit
     * the potential results to packages that match.
     */
    VALUE
    environment_fetch_unique_qualified_package_name(int argc, VALUE *argv, VALUE self)
    {
        try
        {
            if (1 == argc || 2 == argc)
            {
                std::shared_ptr<Environment> * self_ptr;
                Data_Get_Struct(self, std::shared_ptr<Environment>, self_ptr);
                return qualified_package_name_to_value((*self_ptr)->fetch_unique_qualified_package_name(
                                PackageNamePart(StringValuePtr(argv[0])), 2 == argc ? value_to_filter(argv[1]) : filter::All()));
            }
            else
                rb_raise(rb_eArgError, "fetch_unique_qualified_package_name expects one or two arguments, but got %d",argc);
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     repositories -> Array
     *     repositories {|repository| block } -> Nil
     *
     *  Returns all the repositories in the package database, either as an Array, or as
     *  the parameters to a block.
     */
    VALUE
    environment_repositories(VALUE self)
    {
        try
        {
            std::shared_ptr<Environment> * self_ptr;
            Data_Get_Struct(self, std::shared_ptr<Environment>, self_ptr);

            if (rb_block_given_p())
            {
                for (Environment::RepositoryConstIterator r((*self_ptr)->begin_repositories()),
                        r_end((*self_ptr)->end_repositories()) ; r != r_end ; ++r)
                    rb_yield(repository_to_value(*r));
                return Qnil;
            }
            VALUE result(rb_ary_new());
            for (Environment::RepositoryConstIterator r((*self_ptr)->begin_repositories()),
                    r_end((*self_ptr)->end_repositories()) ; r != r_end ; ++r)
                rb_ary_push(result, repository_to_value(*r));

            return result;
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     fetch_repository(repository_name) -> Repository
     *
     *  Fetch a named repository.
     */
    VALUE
    environment_fetch_repository(VALUE self, VALUE name)
    {
        try
        {
            std::shared_ptr<Environment> * self_ptr;
            Data_Get_Struct(self, std::shared_ptr<Environment>, self_ptr);

            return repository_to_value((*self_ptr)->fetch_repository(RepositoryName(StringValuePtr(name))));
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     more_important_than(repository_name_a, repository_name_b) -> bool
     *
     * True if repository_name_a is more important than repository_name_b .
     */
    VALUE
    environment_more_important_than(VALUE self, VALUE name1, VALUE name2)
    {
        try
        {
            std::shared_ptr<Environment> * self_ptr;
            Data_Get_Struct(self, std::shared_ptr<Environment>, self_ptr);

            return (*self_ptr)->more_important_than(RepositoryName(StringValuePtr(name1)),
                    RepositoryName(StringValuePtr(name2))) ? Qtrue : Qfalse;
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     has_repository_named?(repository_name) -> true or false
     *
     *  Do we have a named repository?
     */
    VALUE
    environment_has_repository_named(VALUE self, VALUE name)
    {
        try
        {
            std::shared_ptr<Environment> * self_ptr;
            Data_Get_Struct(self, std::shared_ptr<Environment>, self_ptr);

            return ((*self_ptr)->has_repository_named(RepositoryName(StringValuePtr(name)))) ? true : false;
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     add_repository(importance, repository) -> nil
     *
     *  Add a repository.
     */
    VALUE
    environment_add_repository(VALUE self, VALUE importance, VALUE repo_v)
    {
        try
        {
            std::shared_ptr<Environment> * self_ptr;
            Data_Get_Struct(self, std::shared_ptr<Environment>, self_ptr);

            std::shared_ptr<Repository> repo(value_to_repository(repo_v));

            (*self_ptr)->add_repository(NUM2INT(importance), repo);
            return Qnil;
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
        c_environment = rb_define_class_under(paludis_module(), "Environment", rb_cObject);
        rb_funcall(c_environment, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_method(c_environment, "set", RUBY_FUNC_CAST(&environment_set), 1);
        rb_define_method(c_environment, "distribution", RUBY_FUNC_CAST(&environment_distribution), 0);
        rb_define_method(c_environment, "accept_license", RUBY_FUNC_CAST(&environment_accept_license), 2);
        rb_define_method(c_environment, "accept_keywords", RUBY_FUNC_CAST(&environment_accept_keywords), 2);
        rb_define_method(c_environment, "mirrors", RUBY_FUNC_CAST(&environment_mirrors), 1);
        rb_define_method(c_environment, "[]", RUBY_FUNC_CAST(&environment_square_brackets), 1);
        rb_define_method(c_environment, "format_key",
                RUBY_FUNC_CAST((&EnvironmentKey<MetadataValueKey<std::string> , &Environment::format_key>::fetch)), 0);
        rb_define_method(c_environment, "config_location_key",
                RUBY_FUNC_CAST((&EnvironmentKey<MetadataValueKey<FSPath>, &Environment::config_location_key>::fetch)), 0);
        rb_define_method(c_environment, "preferred_root_key",
                RUBY_FUNC_CAST((&EnvironmentKey<MetadataValueKey<FSPath>, &Environment::preferred_root_key>::fetch)), 0);
        rb_define_method(c_environment, "fetch_unique_qualified_package_name",
                RUBY_FUNC_CAST(&environment_fetch_unique_qualified_package_name), -1);
        rb_define_method(c_environment, "repositories",
                RUBY_FUNC_CAST(&environment_repositories), 0);
        rb_define_method(c_environment, "fetch_repository",
                RUBY_FUNC_CAST(&environment_fetch_repository), 1);
        rb_define_method(c_environment, "more_important_than",
                RUBY_FUNC_CAST(&environment_more_important_than), 2);
        rb_define_method(c_environment, "has_repository_named?",
                RUBY_FUNC_CAST(&environment_has_repository_named), 1);
        rb_define_method(c_environment, "add_repository",
                RUBY_FUNC_CAST(&environment_add_repository), 2);

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
         * Document-class: Paludis::TestEnvironment
         *
         * A crude test environment.
         */
        c_test_environment = rb_define_class_under(paludis_module(), "TestEnvironment", c_environment);
        rb_define_singleton_method(c_test_environment, "new", RUBY_FUNC_CAST(&test_environment_new), 0);

        /*
         * Document-class: Paludis::EnvironmentFactory
         *
         * A class that holds methods to create environments.
         *
         * To access the default environment use create("")
         */
        c_environment_factory = rb_define_class_under(paludis_module(), "EnvironmentFactory", rb_cObject);
        rb_funcall(rb_const_get(rb_cObject, rb_intern("Singleton")), rb_intern("included"), 1, c_environment_factory);
        rb_define_method(c_environment_factory, "create", RUBY_FUNC_CAST(&environment_factory_create), 1);
    }
}

std::shared_ptr<Environment>
paludis::ruby::value_to_environment(VALUE v)
{
    if (rb_obj_is_kind_of(v, c_environment))
    {
        std::shared_ptr<Environment> * v_ptr;
        Data_Get_Struct(v, std::shared_ptr<Environment>, v_ptr);
        return *v_ptr;
    }
    else
    {
        rb_raise(rb_eTypeError, "Can't convert %s into Environment", rb_obj_classname(v));
    }
}

RegisterRubyClass::Register paludis_ruby_register_environment PALUDIS_ATTRIBUTE((used))
    (&do_register_environment);

