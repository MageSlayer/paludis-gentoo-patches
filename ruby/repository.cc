/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh
 * Copyright (c) 2006, 2007 Richard Brown
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
#include <paludis/repositories/fake/fake_package_id.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/set.hh>
#include <paludis/util/sequence.hh>
#include <ruby.h>

using namespace paludis;
using namespace paludis::ruby;

#define RUBY_FUNC_CAST(x) reinterpret_cast<VALUE (*)(...)>(x)

namespace
{
    static VALUE c_repository;
    static VALUE c_profiles_desc_line;
    static VALUE c_fake_repository_base;
    static VALUE c_fake_repository;

    VALUE
    profiles_desc_line_to_value(const RepositoryEInterface::ProfilesDescLine & v)
    {
        RepositoryEInterface::ProfilesDescLine * vv(new RepositoryEInterface::ProfilesDescLine(v));
        return Data_Wrap_Struct(c_profiles_desc_line, 0, &Common<RepositoryEInterface::ProfilesDescLine>::free, vv);
    }

    tr1::shared_ptr<FakeRepositoryBase>
    value_to_fake_repository_base(VALUE v)
    {
        if (rb_obj_is_kind_of(v, c_fake_repository_base))
        {
            tr1::shared_ptr<Repository> * v_ptr;
            Data_Get_Struct(v, tr1::shared_ptr<Repository>, v_ptr);
            return tr1::static_pointer_cast<FakeRepositoryBase>(*v_ptr);
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
            tr1::shared_ptr<Repository> * self_ptr;
            Data_Get_Struct(self, tr1::shared_ptr<Repository>, self_ptr);
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
            tr1::shared_ptr<Repository> * self_ptr;
            Data_Get_Struct(self, tr1::shared_ptr<Repository>, self_ptr);
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
            tr1::shared_ptr<Repository> * self_ptr;
            Data_Get_Struct(self, tr1::shared_ptr<Repository>, self_ptr);
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
            tr1::shared_ptr<Repository> * self_ptr;
            Data_Get_Struct(self, tr1::shared_ptr<Repository>, self_ptr);
            if (rb_block_given_p())
            {
                tr1::shared_ptr<const CategoryNamePartSet> c((*self_ptr)->category_names());
                for (CategoryNamePartSet::ConstIterator i(c->begin()), i_end(c->end()) ; i != i_end ; ++i)
                    rb_yield(rb_str_new2(stringify(*i).c_str()));
                return Qnil;
            }
            VALUE result(rb_ary_new());
            tr1::shared_ptr<const CategoryNamePartSet> c((*self_ptr)->category_names());
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
            tr1::shared_ptr<Repository> * self_ptr;
            Data_Get_Struct(self, tr1::shared_ptr<Repository>, self_ptr);
            PackageNamePart package(StringValuePtr(pkg));

            if (rb_block_given_p())
            {
                tr1::shared_ptr<const CategoryNamePartSet> c((*self_ptr)->category_names_containing_package(package));
                for (CategoryNamePartSet::ConstIterator i(c->begin()), i_end(c->end()) ; i != i_end ; ++i)
                    rb_yield(rb_str_new2(stringify(*i).c_str()));
                return Qnil;
            }
            VALUE result(rb_ary_new());
            tr1::shared_ptr<const CategoryNamePartSet> c((*self_ptr)->category_names_containing_package(package));
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
            tr1::shared_ptr<Repository> * self_ptr;
            Data_Get_Struct(self, tr1::shared_ptr<Repository>, self_ptr);
            CategoryNamePart category(StringValuePtr(cat));

            if (rb_block_given_p())
            {
                tr1::shared_ptr<const QualifiedPackageNameSet> c((*self_ptr)->package_names(category));
                for (QualifiedPackageNameSet::ConstIterator i(c->begin()), i_end(c->end()) ; i != i_end ; ++i)
                    rb_yield(qualified_package_name_to_value(*i));
                return Qnil;
            }
            VALUE result(rb_ary_new());
            tr1::shared_ptr<const QualifiedPackageNameSet> c((*self_ptr)->package_names(category));
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
            tr1::shared_ptr<Repository> * self_ptr;
            Data_Get_Struct(self, tr1::shared_ptr<Repository>, self_ptr);
            QualifiedPackageName q = value_to_qualified_package_name(qpn);

            if (rb_block_given_p())
            {
                tr1::shared_ptr<const PackageIDSequence> c((*self_ptr)->package_ids(q));
                for (PackageIDSequence::ConstIterator i(c->begin()), i_end(c->end()) ; i != i_end ; ++i)
                    rb_yield(package_id_to_value(*i));
                return Qnil;
            }
            VALUE result(rb_ary_new());
            tr1::shared_ptr<const PackageIDSequence> c((*self_ptr)->package_ids(q));
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
     * Document-method: sets_interface
     *
     * call-seq:
     *     sets_interface -> self or Nil
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
     * Document-method: world_interface
     *
     * call-seq:
     *     world_interface -> self or Nil
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
    /*
     * Document-method: qa_interface
     *
     * call-seq:
     *     qa_interface -> self or Nil
     *
     * Returns self if the repository supports the interface, otherwise Nil.
     */
    template <typename T_, T_ * RepositoryCapabilities::* m_>
    struct Interface
    {
        static VALUE fetch(VALUE self)
        {
            tr1::shared_ptr<Repository> * self_ptr;
            Data_Get_Struct(self, tr1::shared_ptr<Repository>, self_ptr);
            return ((**self_ptr).*m_) ? self : Qnil;
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
        tr1::shared_ptr<Repository> * self_ptr;
        tr1::shared_ptr<const SupportsActionTestBase> test_ptr(value_to_supports_action_test_base(test));
        Data_Get_Struct(self, tr1::shared_ptr<Repository>, self_ptr);
        return (*self_ptr)->some_ids_might_support_action(*test_ptr) ? Qtrue : Qfalse;
    }

    /*
     * Document-method: query_use
     *
     * call-seq:
     *     query_use(use_flag) -> true or false or nil
     *     query_use(use_flag, package_database_entry) -> true or false or nil
     *
     * Query the state of the specified use flag: true if set, false
     * if unset, nil if unspecified.  nil if the repository doesn't
     * implement use_interface.
     */
    /*
     * Document-method: query_use_mask
     *
     * call-seq:
     *     query_use_mask(use_flag) -> true or false or nil
     *     query_use_mask(use_flag, package_database_entry) -> true or false or nil
     *
     * Query whether the specified use flag is masked.  nil if the
     * repository doesn't implement use_interface.
     */
    /*
     * Document-method: query_use_force
     *
     * call-seq:
     *     query_use_force(use_flag) -> true or false or nil
     *     query_use_force(use_flag, package_database_entry) -> true or false or nil
     *
     * Query whether the specified use flag is forced.  nil if the
     * repository doesn't implement use_interface.
     */


    template <typename T_, T_ (RepositoryUseInterface::* m_) (const UseFlagName &, const PackageID &) const> struct QueryUseMessage;

    template <typename T_, T_ trueval_, T_ falseval_, T_ (RepositoryUseInterface::* m_) (const UseFlagName &, const PackageID &) const>
    struct QueryUse
    {
        static VALUE
        query(int argc, VALUE * argv, VALUE self)
        {
            try
            {
                tr1::shared_ptr<Repository> * self_ptr;
                Data_Get_Struct(self, tr1::shared_ptr<Repository>, self_ptr);
                RepositoryUseInterface * const use_interface ((**self_ptr).use_interface);

                if (use_interface)
                {
                    if (2 != argc)
                    {
                        rb_raise(rb_eArgError, QueryUseMessage<T_, m_>::message, argc);
                    }

                    T_ status(((*use_interface).*m_)(UseFlagName(StringValuePtr(argv[0])), *value_to_package_id(argv[1])));

                    return status == trueval_ ? Qtrue : status == falseval_ ? Qfalse : Qnil;
                }
                else
                {
                    return Qnil;
                }
            }
            catch (const std::exception & e)
            {
                exception_to_ruby_exception(e);
            }
        }
    };

    template<>
    struct QueryUseMessage<UseFlagState, &RepositoryUseInterface::query_use>
    {
        static const char * message;
    };
    const char * QueryUseMessage<UseFlagState, &RepositoryUseInterface::query_use>::message =
        "Repository.query_use expects two arguments, but got %d";

    template<>
    struct QueryUseMessage<bool, &RepositoryUseInterface::query_use_mask>
    {
        static const char * message;
    };
    const char * QueryUseMessage<bool, &RepositoryUseInterface::query_use_mask>::message =
        "Repository.query_use_mask expects two arguments, but got %d";

    template<>
    struct QueryUseMessage<bool, &RepositoryUseInterface::query_use_force>
    {
        static const char * message;
    };
    const char * QueryUseMessage<bool, &RepositoryUseInterface::query_use_force>::message =
        "Repository.query_use_force expects two arguments, but got %d";

    /*
     * call-seq:
     *     describe_use_flag(flag_name) -> String or Nil
     *     describe_use_flag(flag_name, package_id) -> String or Nil
     *
     * Returns the description for a use flag name, or nil if the repository does not include 
     * the use_flag_interface.
     */

    VALUE
    repository_describe_use_flag(int argc, VALUE * argv, VALUE self)
    {
        try
        {
            tr1::shared_ptr<Repository> * self_ptr;
            Data_Get_Struct(self, tr1::shared_ptr<Repository>, self_ptr);
            if ((*self_ptr)->use_interface) {
                if (1 == argc || 2 ==argc)
                {
                    UseFlagName ufn = UseFlagName(StringValuePtr(argv[0]));
                    tr1::shared_ptr<const PackageID> pid(value_to_package_id(argv[1]));
                    return rb_str_new2(((*self_ptr)->use_interface->describe_use_flag(ufn, *pid).c_str()));

                }
                else
                {
                    rb_raise(rb_eArgError, "describe_use_flag expects one or two arguments, but got %d", argc);
                }
            }
            else
            {
                return Qnil;
            }

        }
            catch (const std::exception & e)
            {
                exception_to_ruby_exception(e);
            }
    }

    /*
     * call-seq:
     *     profiles -> Array
     *
     * Fetch an array of our profiles, as ProfilesDescLine.
     */
    VALUE
    repository_profiles(VALUE self)
    {
        try
        {
            tr1::shared_ptr<Repository> * self_ptr;
            Data_Get_Struct(self, tr1::shared_ptr<Repository>, self_ptr);
            if ((*self_ptr)->e_interface) {
                VALUE result(rb_ary_new());
                for (RepositoryEInterface::ProfilesConstIterator i((*self_ptr)->e_interface->begin_profiles()),
                        i_end((*self_ptr)->e_interface->end_profiles()) ; i != i_end ; ++i)
                {
                    rb_ary_push(result, profiles_desc_line_to_value(*i));
                }
                return result;
            }
            else
            {
                return Qnil;
            }
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     find_profile(profile_location) -> ProfilesDescLine
     *
     * Fetches the named profile.
     */
    VALUE
    repository_find_profile(VALUE self, VALUE profile)
    {
        try
        {
            tr1::shared_ptr<Repository> * self_ptr;
            Data_Get_Struct(self, tr1::shared_ptr<Repository>, self_ptr);

            if ((*self_ptr)->e_interface)
            {
                RepositoryEInterface::ProfilesConstIterator p((*self_ptr)->e_interface->find_profile(FSEntry(StringValuePtr(profile))));

                if (p == (*self_ptr)->e_interface->end_profiles())
                    return Qnil;

                return profiles_desc_line_to_value(*p);
            }
            else
            {
                return Qnil;
            }
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     set_profile(profile) -> Nil
     *
     * Sets the repository profile to the given profile.
     */
    VALUE
    repository_set_profile(VALUE self, VALUE profile)
    {
        try
        {
            tr1::shared_ptr<Repository> * self_ptr;
            Data_Get_Struct(self, tr1::shared_ptr<Repository>, self_ptr);
            if ((*self_ptr)->e_interface)
                (*self_ptr)->e_interface->set_profile(
                    (*self_ptr)->e_interface->find_profile(
                        value_to_profiles_desc_line(profile).path));
            return Qnil;
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     profile_variable(variable) -> String
     *
     * Fetches the named variable.
     */
    VALUE
    repository_profile_variable(VALUE self, VALUE var)
    {
        try
        {
            tr1::shared_ptr<Repository> * self_ptr;
            Data_Get_Struct(self, tr1::shared_ptr<Repository>, self_ptr);
            if ((*self_ptr)->e_interface)
                return rb_str_new2(((*self_ptr)->e_interface->profile_variable(StringValuePtr(var))).c_str());
            return Qnil;
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * Document-method: arch
     *
     * call-seq:
     *     arch -> String
     *
     * Fetch arch for this ProfilesDescLine.
     */
    /*
     * Document-method: status
     *
     * call-seq:
     *     status -> String
     *
     * Fetch status for this ProfilesDescLine.
     */
    /*
     * Document-method: path
     *
     * call-seq:
     *     path -> String
     *
     * Fetch path to this ProfilesDescLine.
     */
    template <typename T_, T_ RepositoryEInterface::ProfilesDescLine::* m_>
    struct DescLineValue
    {
        static VALUE
        fetch(VALUE self)
        {
            RepositoryEInterface::ProfilesDescLine * ptr;
            Data_Get_Struct(self, RepositoryEInterface::ProfilesDescLine, ptr);
            return rb_str_new2(stringify((*ptr).*m_).c_str());
        }
    };

    /*
     * call-seq:
     *     check_qa(qa_reporter, qa_check_properties_ignore_if, qa_check_properties_ignore_unless, qa_message_minimum_level, dir) -> Qnil
     *
     * Check qa in the specified dir. qa_reporter.message (QAReporter) will be called for each error found.
     *
     */
    VALUE
    repository_check_qa(VALUE self, VALUE reporter, VALUE ignore_if, VALUE ignore_unless, VALUE minumum_level, VALUE dir)
    {
        try
        {
            tr1::shared_ptr<Repository> * self_ptr;
            Data_Get_Struct(self, tr1::shared_ptr<Repository>, self_ptr);
#ifdef ENABLE_RUBY_QA
            if ((*self_ptr)->qa_interface)
            {
                RubyQAReporter* qar = new RubyQAReporter(&reporter);
                (*self_ptr)->qa_interface->check_qa(*qar,
                        value_to_qa_check_properties((ignore_if)),
                        value_to_qa_check_properties((ignore_unless)),
                        static_cast<QAMessageLevel>(NUM2INT(minumum_level)),
                        FSEntry(StringValuePtr(dir)));
            }
#endif
            return Qnil;
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
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
            tr1::shared_ptr<FakeRepositoryBase> repo(value_to_fake_repository_base(self));
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
            tr1::shared_ptr<FakeRepositoryBase> repo(value_to_fake_repository_base(self));
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
     *
     * Add a version, and a package and category if necessary, and set some
     * default values for its metadata, and return said metadata.
     */
    VALUE
    fake_repository_base_add_version(int argc, VALUE* argv, VALUE self)
    {
        try
        {
            tr1::shared_ptr<FakeRepositoryBase> repo(value_to_fake_repository_base(self));
            tr1::shared_ptr<PackageID> pkg;

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

            tr1::shared_ptr<Repository> * r = new tr1::shared_ptr<Repository>(new
                    FakeRepository(value_to_environment(argv[0]).get(), RepositoryName(StringValuePtr(argv[1]))));
            VALUE tdata(Data_Wrap_Struct(self, 0, &Common<tr1::shared_ptr<Repository> >::free, r));
            rb_obj_call_init(tdata, argc, argv);
            return tdata;
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
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

        rb_define_method(c_repository, "sets_interface", RUBY_FUNC_CAST((&Interface<RepositorySetsInterface,
                        &Repository::sets_interface>::fetch)), 0);
        rb_define_method(c_repository, "syncable_interface", RUBY_FUNC_CAST((&Interface<RepositorySyncableInterface,
                        &Repository::syncable_interface>::fetch)), 0);
        rb_define_method(c_repository, "use_interface", RUBY_FUNC_CAST((&Interface<RepositoryUseInterface,
                        &Repository::use_interface>::fetch)), 0);
        rb_define_method(c_repository, "world_interface", RUBY_FUNC_CAST((&Interface<RepositoryWorldInterface,
                        &Repository::world_interface>::fetch)), 0);
        rb_define_method(c_repository, "mirrors_interface", RUBY_FUNC_CAST((&Interface<RepositoryMirrorsInterface,
                        &Repository::mirrors_interface>::fetch)), 0);
        rb_define_method(c_repository, "environment_variable_interface", RUBY_FUNC_CAST((&Interface<RepositoryEnvironmentVariableInterface,
                        &Repository::environment_variable_interface>::fetch)), 0);
        rb_define_method(c_repository, "provides_interface", RUBY_FUNC_CAST((&Interface<RepositoryProvidesInterface,
                        &Repository::provides_interface>::fetch)), 0);
        rb_define_method(c_repository, "virtuals_interface", RUBY_FUNC_CAST((&Interface<RepositoryVirtualsInterface,
                        &Repository::virtuals_interface>::fetch)), 0);
        rb_define_method(c_repository, "e_interface", RUBY_FUNC_CAST((&Interface<RepositoryEInterface,
                        &Repository::e_interface>::fetch)), 0);
        rb_define_method(c_repository, "qa_interface", RUBY_FUNC_CAST((&Interface<RepositoryQAInterface,
                        &Repository::qa_interface>::fetch)), 0);

        rb_define_method(c_repository, "some_ids_might_support_action", RUBY_FUNC_CAST(&repository_some_ids_might_support_action), 1);

        rb_define_method(c_repository, "query_use", RUBY_FUNC_CAST((&QueryUse<UseFlagState, use_enabled, use_disabled, &RepositoryUseInterface::query_use>::query)), -1);
        rb_define_method(c_repository, "query_use_mask", RUBY_FUNC_CAST((&QueryUse<bool, true, false, &RepositoryUseInterface::query_use_mask>::query)), -1);
        rb_define_method(c_repository, "query_use_force", RUBY_FUNC_CAST((&QueryUse<bool, true, false, &RepositoryUseInterface::query_use_force>::query)), -1);

        rb_define_method(c_repository, "describe_use_flag", RUBY_FUNC_CAST(&repository_describe_use_flag),-1);

        rb_define_method(c_repository, "profiles", RUBY_FUNC_CAST(&repository_profiles),0);
        rb_define_method(c_repository, "find_profile", RUBY_FUNC_CAST(&repository_find_profile),1);
        rb_define_method(c_repository, "profile_variable", RUBY_FUNC_CAST(&repository_profile_variable),1);
        rb_define_method(c_repository, "set_profile", RUBY_FUNC_CAST(&repository_set_profile),1);

        rb_define_method(c_repository, "check_qa", RUBY_FUNC_CAST(&repository_check_qa),5);

        /*
         * Document-class: Paludis::ProfilesDescLine
         *
         *
         */
        c_profiles_desc_line = rb_define_class_under(paludis_module(), "ProfilesDescLine", rb_cObject);
        rb_funcall(c_profiles_desc_line, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_method(c_profiles_desc_line, "path",
                RUBY_FUNC_CAST((&DescLineValue<FSEntry,&RepositoryEInterface::ProfilesDescLine::path>::fetch)), 0);
        rb_define_method(c_profiles_desc_line, "arch",
                RUBY_FUNC_CAST((&DescLineValue<std::string,&RepositoryEInterface::ProfilesDescLine::arch>::fetch)), 0);
        rb_define_method(c_profiles_desc_line, "status",
                RUBY_FUNC_CAST((&DescLineValue<std::string,&RepositoryEInterface::ProfilesDescLine::status>::fetch)), 0);

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
paludis::ruby::repository_to_value(tr1::shared_ptr<Repository> m)
{
    if (0 == m)
        return Qnil;
    else
        return repo_to_value<tr1::shared_ptr<Repository> >(m, &c_repository);
}

tr1::shared_ptr<Repository>
paludis::ruby::value_to_repository(VALUE v)
{
    if (rb_obj_is_kind_of(v, c_repository))
    {
        tr1::shared_ptr<Repository> * v_ptr;
        Data_Get_Struct(v, tr1::shared_ptr<Repository>, v_ptr);
        return *v_ptr;
    }
    else
    {
        rb_raise(rb_eTypeError, "Can't convert %s into Repository", rb_obj_classname(v));
    }
}

RepositoryEInterface::ProfilesDescLine
paludis::ruby::value_to_profiles_desc_line(VALUE v)
{
    if (rb_obj_is_kind_of(v, c_profiles_desc_line))
    {
        RepositoryEInterface::ProfilesDescLine * v_ptr;
        Data_Get_Struct(v, RepositoryEInterface::ProfilesDescLine, v_ptr);
        return *v_ptr;
    }
    else
    {
        rb_raise(rb_eTypeError, "Can't convert %s into ProfilesDescLine", rb_obj_classname(v));
    }
}

RegisterRubyClass::Register paludis_ruby_register_repository PALUDIS_ATTRIBUTE((used))
    (&do_register_repository);


