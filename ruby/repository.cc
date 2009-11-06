/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009 Ciaran McCreesh
 * Copyright (c) 2006, 2007, 2008 Richard Brown
 * Copyright (c) 2007 David Leverton
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
#include <paludis/repository.hh>
#include <paludis/util/options.hh>
#include <paludis/repositories/fake/fake_repository.hh>
#include <paludis/repositories/fake/fake_installed_repository.hh>
#include <paludis/repositories/fake/fake_package_id.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/set.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/thread.hh>
#include <paludis/util/condition_variable.hh>
#include <paludis/util/make_named_values.hh>
#include <ruby.h>
#include <list>
#include <tr1/functional>

using namespace paludis;
using namespace paludis::ruby;

namespace
{
    static VALUE c_repository;
    static VALUE c_fake_repository_base;
    static VALUE c_fake_repository;
    static VALUE c_fake_installed_repository;

    std::tr1::shared_ptr<FakeRepositoryBase>
    value_to_fake_repository_base(VALUE v)
    {
        if (rb_obj_is_kind_of(v, c_fake_repository_base))
        {
            std::tr1::shared_ptr<Repository> * v_ptr;
            Data_Get_Struct(v, std::tr1::shared_ptr<Repository>, v_ptr);
            return std::tr1::static_pointer_cast<FakeRepositoryBase>(*v_ptr);
        }
        else
        {
            rb_raise(rb_eTypeError, "Can't convert %s into FakeRepositoryBase", rb_obj_classname(v));
        }
    }

    /*
     * call-seq:
     *     name -> String
     *
     * Returns our name.
     */
    VALUE
    repository_name(VALUE self)
    {
        try
        {
            std::tr1::shared_ptr<Repository> * self_ptr;
            Data_Get_Struct(self, std::tr1::shared_ptr<Repository>, self_ptr);
            return rb_str_new2(stringify((*self_ptr)->name()).c_str());
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     has_category_named?(category_name) -> true or false
     *
     * Do we have a category with the given name?
     */
    VALUE
    repository_has_category_named(VALUE self, VALUE cat)
    {
        try
        {
            std::tr1::shared_ptr<Repository> * self_ptr;
            Data_Get_Struct(self, std::tr1::shared_ptr<Repository>, self_ptr);
            return (*self_ptr)->has_category_named(CategoryNamePart(StringValuePtr(cat))) ? Qtrue : Qfalse;
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    VALUE

    //TODO why do we need Document-method here?
    /*
     * Document-method: has_package_named?
     *
     * call-seq:
     *     has_package_named?(package_name) -> true or false
     *
     * Do we have a package with the given name?
     */
    repository_has_package_named(VALUE self, VALUE name)
    {
        try
        {
            std::tr1::shared_ptr<Repository> * self_ptr;
            Data_Get_Struct(self, std::tr1::shared_ptr<Repository>, self_ptr);
            return (*self_ptr)->has_package_named(value_to_qualified_package_name(name)) ? Qtrue : Qfalse;
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     *  call-seq:
     *      category_names -> Array
     *      category_names {|category_name| block } -> Nil
     *
     * Returns the names of all categories, either as an Array or as the parameters to a block.
     */
    VALUE
    repository_category_names(VALUE self)
    {
        try
        {
            std::tr1::shared_ptr<Repository> * self_ptr;
            Data_Get_Struct(self, std::tr1::shared_ptr<Repository>, self_ptr);
            if (rb_block_given_p())
            {
                std::tr1::shared_ptr<const CategoryNamePartSet> c((*self_ptr)->category_names());
                for (CategoryNamePartSet::ConstIterator i(c->begin()), i_end(c->end()) ; i != i_end ; ++i)
                    rb_yield(rb_str_new2(stringify(*i).c_str()));
                return Qnil;
            }
            VALUE result(rb_ary_new());
            std::tr1::shared_ptr<const CategoryNamePartSet> c((*self_ptr)->category_names());
            for (CategoryNamePartSet::ConstIterator i(c->begin()), i_end(c->end()) ; i != i_end ; ++i)
                rb_ary_push(result, rb_str_new2(stringify(*i).c_str()));
            return result;
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     *  call-seq:
     *      category_names_containing_package(package) -> Array
     *      category_names_containing_package(package) {|category_name| block } -> Nil
     *
     * Returns the names of all categories containing the given package, either as an Array
     * or as the parameters to a block.
     */
    VALUE
    repository_category_names_containing_package(VALUE self, VALUE pkg)
    {
        try
        {
            std::tr1::shared_ptr<Repository> * self_ptr;
            Data_Get_Struct(self, std::tr1::shared_ptr<Repository>, self_ptr);
            PackageNamePart package(StringValuePtr(pkg));

            if (rb_block_given_p())
            {
                std::tr1::shared_ptr<const CategoryNamePartSet> c((*self_ptr)->category_names_containing_package(package));
                for (CategoryNamePartSet::ConstIterator i(c->begin()), i_end(c->end()) ; i != i_end ; ++i)
                    rb_yield(rb_str_new2(stringify(*i).c_str()));
                return Qnil;
            }
            VALUE result(rb_ary_new());
            std::tr1::shared_ptr<const CategoryNamePartSet> c((*self_ptr)->category_names_containing_package(package));
            for (CategoryNamePartSet::ConstIterator i(c->begin()), i_end(c->end()) ; i != i_end ; ++i)
                rb_ary_push(result, rb_str_new2(stringify(*i).c_str()));
            return result;
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     *  call-seq:
     *      package_names(category) -> Array
     *      package_names(category) {|qualified_package_name| block } -> Nil
     *
     * Returns the names of all packages within the given category, either as an Array,
     * or as the parameters to a block.
     */
    VALUE
    repository_package_names(VALUE self, VALUE cat)
    {
        try
        {
            std::tr1::shared_ptr<Repository> * self_ptr;
            Data_Get_Struct(self, std::tr1::shared_ptr<Repository>, self_ptr);
            CategoryNamePart category(StringValuePtr(cat));

            if (rb_block_given_p())
            {
                std::tr1::shared_ptr<const QualifiedPackageNameSet> c((*self_ptr)->package_names(category));
                for (QualifiedPackageNameSet::ConstIterator i(c->begin()), i_end(c->end()) ; i != i_end ; ++i)
                    rb_yield(qualified_package_name_to_value(*i));
                return Qnil;
            }
            VALUE result(rb_ary_new());
            std::tr1::shared_ptr<const QualifiedPackageNameSet> c((*self_ptr)->package_names(category));
            for (QualifiedPackageNameSet::ConstIterator i(c->begin()), i_end(c->end()) ; i != i_end ; ++i)
                rb_ary_push(result, qualified_package_name_to_value(*i));
            return result;
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     package_ids(qualified_package_name) -> Array
     *     package_ids(qualified_package_name) {|package_id| block } -> Qnil
     *
     * Returns the package IDs for the given package, either as an Array, or as the parameters
     * to a block.
     */

    VALUE
    repository_package_ids(VALUE self, VALUE qpn)
    {
        try
        {
            std::tr1::shared_ptr<Repository> * self_ptr;
            Data_Get_Struct(self, std::tr1::shared_ptr<Repository>, self_ptr);
            QualifiedPackageName q = value_to_qualified_package_name(qpn);

            if (rb_block_given_p())
            {
                std::tr1::shared_ptr<const PackageIDSequence> c((*self_ptr)->package_ids(q));
                for (PackageIDSequence::ConstIterator i(c->begin()), i_end(c->end()) ; i != i_end ; ++i)
                    rb_yield(package_id_to_value(*i));
                return Qnil;
            }
            VALUE result(rb_ary_new());
            std::tr1::shared_ptr<const PackageIDSequence> c((*self_ptr)->package_ids(q));
            for (PackageIDSequence::ConstIterator i(c->begin()), i_end(c->end()) ; i != i_end ; ++i)
                rb_ary_push(result, package_id_to_value(*i));
            return result;
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * Document-method: installable_interface
     *
     * call-seq:
     *     installable_interface -> self or Nil
     *
     * Returns self if the repository supports the interface, otherwise Nil.
     */
    /*
     * Document-method: installed_interface
     *
     * call-seq:
     *     installed_interface -> self or Nil
     *
     * Returns self if the repository supports the interface, otherwise Nil.
     */
    /*
     * Document-method: mask_interface
     *
     * call-seq:
     *     mask_interface -> self or Nil
     *
     * Returns self if the repository supports the interface, otherwise Nil.
     */
    /*
     * Document-method: news_interface
     *
     * call-seq:
     *     news_interface -> self or Nil
     *
     * Returns self if the repository supports the interface, otherwise Nil.
     */
    /*
     * Document-method: uninstallable_interface
     *
     * call-seq:
     *     uninstallable_interface -> self or Nil
     *
     * Returns self if the repository supports the interface, otherwise Nil.
     */
    /*
     * Document-method: use_interface
     *
     * call-seq:
     *     use_interface -> self or Nil
     *
     * Returns self if the repository supports the interface, otherwise Nil.
     */
    /*
     * Document-method: mirrors_interface
     *
     * call-seq:
     *     mirrors_interface -> self or Nil
     *
     * Returns self if the repository supports the interface, otherwise Nil.
     */
    /*
     * Document-method: environment_variable_interface
     *
     * call-seq:
     *     environment_variable_interface -> self or Nil
     *
     * Returns self if the repository supports the interface, otherwise Nil.
     */
    /*
     * Document-method: provides_interface
     *
     * call-seq:
     *     provides_interface -> self or Nil
     *
     * Returns self if the repository supports the interface, otherwise Nil.
     */
    /*
     * Document-method: virtuals_interface
     *
     * call-seq:
     *     virtuals_interface -> self or Nil
     *
     * Returns self if the repository supports the interface, otherwise Nil.
     */
    /*
     * Document-method: e_interface
     *
     * call-seq:
     *     e_interface -> self or Nil
     *
     * Returns self if the repository supports the interface, otherwise Nil.
     */
    template <typename T_, typename R_, NamedValue<T_, R_ *> (RepositoryCapabilities::* f_)>
    struct Interface
    {
        static VALUE fetch(VALUE self)
        {
            std::tr1::shared_ptr<Repository> * self_ptr;
            Data_Get_Struct(self, std::tr1::shared_ptr<Repository>, self_ptr);
            return ((**self_ptr).*f_)() ? self : Qnil;
        }
    };

    /*
     * call-seq:
     *     some_ids_might_support_action(action_test) -> true or false
     *
     * Might some of our IDs support a particular action?
     */
    VALUE
    repository_some_ids_might_support_action(VALUE self, VALUE test)
    {
        std::tr1::shared_ptr<Repository> * self_ptr;
        std::tr1::shared_ptr<const SupportsActionTestBase> test_ptr(value_to_supports_action_test_base(test));
        Data_Get_Struct(self, std::tr1::shared_ptr<Repository>, self_ptr);
        return (*self_ptr)->some_ids_might_support_action(*test_ptr) ? Qtrue : Qfalse;
    }

    /*
     * call-seq:
     *     add_category(category_name) -> Nil
     *
     * Add a category.
     */
    VALUE
    fake_repository_base_add_category(VALUE self, VALUE category)
    {
        try
        {
            std::tr1::shared_ptr<FakeRepositoryBase> repo(value_to_fake_repository_base(self));
            std::string cat_str(StringValuePtr(category));
            repo->add_category(CategoryNamePart(cat_str));
            return Qnil;
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     add_package(qualified_package_name) -> Nil
     *
     * Add a package, and a category if necessary.
     */
    VALUE
    fake_repository_base_add_package(VALUE self, VALUE qpn)
    {
        try
        {
            std::tr1::shared_ptr<FakeRepositoryBase> repo(value_to_fake_repository_base(self));
            QualifiedPackageName name(value_to_qualified_package_name(qpn));
            repo->add_package(name);
            return Qnil;
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     add_version(qualified_package_name, version_spec) -> PackageID
     *     add_version(category_name, package_name, version_string) -> PackageID
     *     add_version(category_name, package_name, version_string, slot) -> PackageID
     *
     * Add a version, and a package and category if necessary, and set some
     * default values for its metadata, and return said metadata.
     */
    VALUE
    fake_repository_base_add_version(int argc, VALUE* argv, VALUE self)
    {
        try
        {
            std::tr1::shared_ptr<FakeRepositoryBase> repo(value_to_fake_repository_base(self));
            std::tr1::shared_ptr<FakePackageID> pkg;

            switch (argc)
            {
                case 2: {
                    QualifiedPackageName qpn(value_to_qualified_package_name(argv[0]));
                    VersionSpec ver(value_to_version_spec(argv[1]));
                    pkg = repo->add_version(qpn, ver);
                    break;
                }

                case 3: {
                    std::string cat(StringValuePtr(argv[0]));
                    std::string name(StringValuePtr(argv[1]));
                    std::string ver(StringValuePtr(argv[2]));
                    pkg = repo->add_version(cat, name, ver);
                    break;
                }

                case 4: {
                    std::string cat(StringValuePtr(argv[0]));
                    std::string name(StringValuePtr(argv[1]));
                    std::string ver(StringValuePtr(argv[2]));
                    std::string slot(StringValuePtr(argv[3]));
                    pkg = repo->add_version(cat, name, ver);
                    pkg->set_slot(SlotName(slot));
                    break;
                }

                default:
                    rb_raise(rb_eArgError, "FakeRepositoryBase.add_version expects two or three arguments, but got %d", argc);
            }

            return package_id_to_value(pkg);
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    VALUE
    fake_repository_init(int, VALUE*, VALUE self)
    {
        return self;
    }

    /*
     * call-seq:
     *     FakeRepository.new(environment, repo_name) -> FakeRepository
     *
     * Create a new FakeRepository.
     */
    VALUE
    fake_repository_new(int argc, VALUE* argv, VALUE self)
    {
        try
        {
            if (2 != argc)
                rb_raise(rb_eArgError, "FakeRepository.new expects two arguments, but got %d", argc);

            std::tr1::shared_ptr<Repository> * r = new std::tr1::shared_ptr<Repository>(new FakeRepository(
                        make_named_values<FakeRepositoryParams>(
                            value_for<n::environment>(value_to_environment(argv[0]).get()),
                            value_for<n::name>(RepositoryName(StringValuePtr(argv[1]))))));
            VALUE tdata(Data_Wrap_Struct(self, 0, &Common<std::tr1::shared_ptr<Repository> >::free, r));
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
     *     FakeInstalledRepository.new(environment, repo_name) -> FakeInstalledRepository
     *
     * Create a new FakeInstalledRepository.
     */
    VALUE
    fake_installed_repository_new(int argc, VALUE* argv, VALUE self)
    {
        try
        {
            if (2 != argc)
                rb_raise(rb_eArgError, "FakeInstalledRepository.new expects two arguments, but got %d", argc);

            std::tr1::shared_ptr<Repository> * r = new std::tr1::shared_ptr<Repository>(new
                    FakeInstalledRepository(make_named_values<FakeInstalledRepositoryParams>(
                            value_for<n::environment>(value_to_environment(argv[0]).get()),
                            value_for<n::name>(RepositoryName(StringValuePtr(argv[1]))),
                            value_for<n::suitable_destination>(true),
                            value_for<n::supports_uninstall>(true)
                        )));
            VALUE tdata(Data_Wrap_Struct(self, 0, &Common<std::tr1::shared_ptr<Repository> >::free, r));
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
     *     [String] -> MetadataKey or Nil
     *
     * The named metadata key.
     */
    VALUE
    repository_subscript(VALUE self, VALUE raw_name)
    {
        std::tr1::shared_ptr<const Repository> * self_ptr;
        Data_Get_Struct(self, std::tr1::shared_ptr<const Repository>, self_ptr);
        Repository::MetadataConstIterator it((*self_ptr)->find_metadata(StringValuePtr(raw_name)));
        if ((*self_ptr)->end_metadata() == it)
            return Qnil;
        return metadata_key_to_value(*it);
    }

    /*
     * call-seq:
     *     each_metadata {|key| block } -> Nil
     *
     * Our metadata.
     */
    VALUE
    repository_each_metadata(VALUE self)
    {
        std::tr1::shared_ptr<Repository> * self_ptr;
        Data_Get_Struct(self, std::tr1::shared_ptr<Repository>, self_ptr);
        for (Repository::MetadataConstIterator it((*self_ptr)->begin_metadata()),
                it_end((*self_ptr)->end_metadata()); it_end != it; ++it)
        {
            VALUE val(metadata_key_to_value(*it));
            if (Qnil != val)
                rb_yield(val);
        }
            return Qnil;
    }

    template <typename T_, const std::tr1::shared_ptr<const T_> (Repository::* m_) () const>
    struct RepositoryKey
    {
        static VALUE
        fetch(VALUE self)
        {
            std::tr1::shared_ptr<Repository> * self_ptr;
            Data_Get_Struct(self, std::tr1::shared_ptr<Repository>, self_ptr);
            return (((**self_ptr).*m_)()) ? metadata_key_to_value(((**self_ptr).*m_)()) : Qnil;
        }
    };

    /*
     * call-seq:
     *     get_environment_variable(package_id, environment_variable) -> String
     *
     * Query an environment variable.
     */
    VALUE
    repository_get_environment_variable(VALUE self, VALUE pid, VALUE ev)
    {
        std::tr1::shared_ptr<Repository> * self_ptr;
        Data_Get_Struct(self, std::tr1::shared_ptr<Repository>, self_ptr);
        if ((**self_ptr).environment_variable_interface())
            return rb_str_new2((**self_ptr).environment_variable_interface()->get_environment_variable(
                        value_to_package_id(pid),
                        StringValuePtr(ev)).c_str());
        return Qnil;
    }

    /*
     * call-seq:
     *     is_mirror?(mirror_name) -> true or false
     *
     * Is the named item a mirror?
     */
    VALUE
    repository_is_mirror(VALUE self, VALUE mirror)
    {
        std::tr1::shared_ptr<Repository> * self_ptr;
        Data_Get_Struct(self, std::tr1::shared_ptr<Repository>, self_ptr);
        if ((**self_ptr).mirrors_interface())
            return (**self_ptr).mirrors_interface()->is_mirror(StringValuePtr(mirror)) ? Qtrue : Qfalse;
        return Qnil;
    }

    /*
     * call-seq:
     *     mirrors(mirror_name) -> Array
     *
     * Return the mirror URI prefixes for a named mirror.
     */
    VALUE
    repository_mirrors(VALUE self, VALUE mirror)
    {
        std::tr1::shared_ptr<Repository> * self_ptr;
        Data_Get_Struct(self, std::tr1::shared_ptr<Repository>, self_ptr);
        if (!(**self_ptr).mirrors_interface())
            return Qnil;
        VALUE result(rb_ary_new());
        for (RepositoryMirrorsInterface::MirrorsConstIterator m((**self_ptr).mirrors_interface()->begin_mirrors(StringValuePtr(mirror))),
                    m_end((**self_ptr).mirrors_interface()->end_mirrors(StringValuePtr(mirror))) ;
                    m != m_end ; ++m)
            rb_ary_push(result, rb_str_new2((m->second).c_str()));
        return result;
    }

    void do_register_repository()
    {
        /*
         * Document-class: Paludis::Repository
         *
         * A Repository provides a representation of a physical repository to a PackageDatabase.
         */
        c_repository = rb_define_class_under(paludis_module(), "Repository", rb_cObject);
        rb_funcall(c_repository, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_method(c_repository, "name", RUBY_FUNC_CAST(&repository_name), 0);

        rb_define_method(c_repository, "has_category_named?", RUBY_FUNC_CAST(&repository_has_category_named), 1);
        rb_define_method(c_repository, "has_package_named?", RUBY_FUNC_CAST(&repository_has_package_named), 1);

        rb_define_method(c_repository, "category_names", RUBY_FUNC_CAST(&repository_category_names), 0);
        rb_define_method(c_repository, "category_names_containing_package",
                RUBY_FUNC_CAST(&repository_category_names_containing_package), 1);
        rb_define_method(c_repository, "package_names", RUBY_FUNC_CAST(&repository_package_names), 1);
        rb_define_method(c_repository, "package_ids", RUBY_FUNC_CAST(&repository_package_ids), 1);

        rb_define_method(c_repository, "mirrors_interface", RUBY_FUNC_CAST((&Interface<
                        n::mirrors_interface, RepositoryMirrorsInterface, &Repository::mirrors_interface>::fetch)), 0);
        rb_define_method(c_repository, "environment_variable_interface", RUBY_FUNC_CAST((&Interface<
                        n::environment_variable_interface, RepositoryEnvironmentVariableInterface, &Repository::environment_variable_interface>::fetch)), 0);
        rb_define_method(c_repository, "provides_interface", RUBY_FUNC_CAST((&Interface<
                        n::provides_interface, RepositoryProvidesInterface, &Repository::provides_interface>::fetch)), 0);
        rb_define_method(c_repository, "virtuals_interface", RUBY_FUNC_CAST((&Interface<
                        n::virtuals_interface, RepositoryVirtualsInterface, &Repository::virtuals_interface>::fetch)), 0);
        rb_define_method(c_repository, "some_ids_might_support_action", RUBY_FUNC_CAST(&repository_some_ids_might_support_action), 1);

        rb_define_method(c_repository, "[]", RUBY_FUNC_CAST(&repository_subscript), 1);
        rb_define_method(c_repository, "each_metadata", RUBY_FUNC_CAST(&repository_each_metadata), 0);
        rb_define_method(c_repository, "format_key",
                RUBY_FUNC_CAST((&RepositoryKey<MetadataValueKey<std::string> , &Repository::format_key>::fetch)), 0);
        rb_define_method(c_repository, "location_key",
                RUBY_FUNC_CAST((&RepositoryKey<MetadataValueKey<FSEntry>, &Repository::location_key>::fetch)), 0);
        rb_define_method(c_repository, "installed_root_key",
                RUBY_FUNC_CAST((&RepositoryKey<MetadataValueKey<FSEntry>, &Repository::installed_root_key>::fetch)), 0);
        rb_define_method(c_repository, "get_environment_variable", RUBY_FUNC_CAST(&repository_get_environment_variable), 2);

        rb_define_method(c_repository, "is_mirror?", RUBY_FUNC_CAST(&repository_is_mirror), 1);
        rb_define_method(c_repository, "mirrors", RUBY_FUNC_CAST(&repository_mirrors), 1);

        /*
         * Document-class: Paludis::FakeRepositoryBase
         *
         * A FakeRepositoryBase is a Repository subclass whose subclasses are used for
         * various test cases.
         */
        c_fake_repository_base = rb_define_class_under(paludis_module(), "FakeRepositoryBase", c_repository);
        rb_funcall(c_fake_repository_base, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_method(c_fake_repository_base, "add_category", RUBY_FUNC_CAST(&fake_repository_base_add_category), 1);
        rb_define_method(c_fake_repository_base, "add_package", RUBY_FUNC_CAST(&fake_repository_base_add_package), 1);
        rb_define_method(c_fake_repository_base, "add_version", RUBY_FUNC_CAST(&fake_repository_base_add_version), -1);

        /*
         * Document-class: Paludis::FakeRepository
         *
         * Fake repository for use in test cases.
         */
        c_fake_repository = rb_define_class_under(paludis_module(), "FakeRepository", c_fake_repository_base);
        rb_define_singleton_method(c_fake_repository, "new", RUBY_FUNC_CAST(&fake_repository_new), -1);
        rb_define_method(c_fake_repository, "initialize", RUBY_FUNC_CAST(&fake_repository_init), -1);

        /*
         * Document-class: Paludis::FakeInstalledRepository
         *
         * Fake repository for use in test cases.
         */
        c_fake_installed_repository = rb_define_class_under(paludis_module(), "FakeInstalledRepository", c_fake_repository_base);
        rb_define_singleton_method(c_fake_installed_repository, "new", RUBY_FUNC_CAST(&fake_installed_repository_new), -1);
        rb_define_method(c_fake_installed_repository, "initialize", RUBY_FUNC_CAST(&fake_repository_init), -1);
    }
}

template <typename T_>
VALUE repo_to_value(T_ m, VALUE * klass)
{
    T_ * m_ptr(0);
    try
    {
        m_ptr = new T_(m);
        return Data_Wrap_Struct(*klass, 0, &Common<T_>::free, m_ptr);
    }
    catch (const std::exception & e)
    {
        delete m_ptr;
        exception_to_ruby_exception(e);
    }
}

VALUE
paludis::ruby::repository_to_value(std::tr1::shared_ptr<Repository> m)
{
    if (0 == m)
        return Qnil;
    else
        return repo_to_value<std::tr1::shared_ptr<Repository> >(m, &c_repository);
}

std::tr1::shared_ptr<Repository>
paludis::ruby::value_to_repository(VALUE v)
{
    if (rb_obj_is_kind_of(v, c_repository))
    {
        std::tr1::shared_ptr<Repository> * v_ptr;
        Data_Get_Struct(v, std::tr1::shared_ptr<Repository>, v_ptr);
        return *v_ptr;
    }
    else
    {
        rb_raise(rb_eTypeError, "Can't convert %s into Repository", rb_obj_classname(v));
    }
}

RegisterRubyClass::Register paludis_ruby_register_repository PALUDIS_ATTRIBUTE((used))
    (&do_register_repository);


