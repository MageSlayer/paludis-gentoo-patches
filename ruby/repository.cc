/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
 * Copyright (c) 2006, 2007 Richard Brown <mynamewasgone@gmail.com>
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
#include <paludis/repositories/portage/portage_repository.hh>
#include <paludis/util/stringify.hh>
#include <ruby.h>

using namespace paludis;
using namespace paludis::ruby;

#define RUBY_FUNC_CAST(x) reinterpret_cast<VALUE (*)(...)>(x)

namespace
{
    static VALUE c_repository;
    static VALUE c_repository_info;
    static VALUE c_repository_info_section;
    static VALUE c_portage_repository;
    static VALUE c_portage_repository_profiles_desc_line;

    /*
     * call-seq:
     *     name
     *
     * Returns our name.
     */
    VALUE
    repository_name(VALUE self)
    {
        try
        {
            Repository::ConstPointer * self_ptr;
            Data_Get_Struct(self, Repository::ConstPointer, self_ptr);
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
            Repository::ConstPointer * self_ptr;
            Data_Get_Struct(self, Repository::ConstPointer, self_ptr);
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
            Repository::ConstPointer * self_ptr;
            Data_Get_Struct(self, Repository::ConstPointer, self_ptr);
            return (*self_ptr)->has_package_named(value_to_qualified_package_name(name)) ? Qtrue : Qfalse;
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     has_version?(package_name, version) -> true or false
     *
     * Do we have a version spec?
     */
    VALUE
    repository_has_version(VALUE self, VALUE name, VALUE version)
    {
        try
        {
            Repository::ConstPointer * self_ptr;
            Data_Get_Struct(self, Repository::ConstPointer, self_ptr);
            return (*self_ptr)->has_version(value_to_qualified_package_name(name),
                    value_to_version_spec(version)) ? Qtrue : Qfalse;
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
            Repository::ConstPointer * self_ptr;
            Data_Get_Struct(self, Repository::ConstPointer, self_ptr);
            if (rb_block_given_p())
            {
                CategoryNamePartCollection::ConstPointer c((*self_ptr)->category_names());
                for (CategoryNamePartCollection::Iterator i(c->begin()), i_end(c->end()) ; i != i_end ; ++i)
                    rb_yield(rb_str_new2(stringify(*i).c_str()));
                return Qnil;
            }
            VALUE result(rb_ary_new());
            CategoryNamePartCollection::ConstPointer c((*self_ptr)->category_names());
            for (CategoryNamePartCollection::Iterator i(c->begin()), i_end(c->end()) ; i != i_end ; ++i)
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
            Repository::ConstPointer * self_ptr;
            Data_Get_Struct(self, Repository::ConstPointer, self_ptr);
            PackageNamePart package(StringValuePtr(pkg));

            if (rb_block_given_p())
            {
                CategoryNamePartCollection::ConstPointer c((*self_ptr)->category_names_containing_package(package));
                for (CategoryNamePartCollection::Iterator i(c->begin()), i_end(c->end()) ; i != i_end ; ++i)
                    rb_yield(rb_str_new2(stringify(*i).c_str()));
                return Qnil;
            }
            VALUE result(rb_ary_new());
            CategoryNamePartCollection::ConstPointer c((*self_ptr)->category_names_containing_package(package));
            for (CategoryNamePartCollection::Iterator i(c->begin()), i_end(c->end()) ; i != i_end ; ++i)
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
     * Returns the names of all packages within the given package, either as an Array,
     * or as the parameteris to a block.
     */
    VALUE
    repository_package_names(VALUE self, VALUE cat)
    {
        try
        {
            Repository::ConstPointer * self_ptr;
            Data_Get_Struct(self, Repository::ConstPointer, self_ptr);
            CategoryNamePart category(StringValuePtr(cat));

            if (rb_block_given_p())
            {
                QualifiedPackageNameCollection::ConstPointer c((*self_ptr)->package_names(category));
                for (QualifiedPackageNameCollection::Iterator i(c->begin()), i_end(c->end()) ; i != i_end ; ++i)
                    rb_yield(qualified_package_name_to_value(*i));
                return Qnil;
            }
            VALUE result(rb_ary_new());
            QualifiedPackageNameCollection::ConstPointer c((*self_ptr)->package_names(category));
            for (QualifiedPackageNameCollection::Iterator i(c->begin()), i_end(c->end()) ; i != i_end ; ++i)
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
     *     version_specs(qualified_package_name) -> Array
     *     version_specs(qualified_package_name) {|version_spec| block } -> Qnil
     *
     * Returns the versions for the given package, either as an Array, or as the parameters
     * to a block.
     */
    VALUE
    repository_version_specs(VALUE self, VALUE qpn)
    {
        try
        {
            Repository::ConstPointer * self_ptr;
            Data_Get_Struct(self, Repository::ConstPointer, self_ptr);
            QualifiedPackageName q = value_to_qualified_package_name(qpn);

            if (rb_block_given_p())
            {
                VersionSpecCollection::ConstPointer c((*self_ptr)->version_specs(q));
                for (VersionSpecCollection::Iterator i(c->begin()), i_end(c->end()) ; i != i_end ; ++i)
                    rb_yield(version_spec_to_value(*i));
                return Qnil;
            }
            VALUE result(rb_ary_new());
            VersionSpecCollection::ConstPointer c((*self_ptr)->version_specs(q));
            for (VersionSpecCollection::Iterator i(c->begin()), i_end(c->end()) ; i != i_end ; ++i)
                rb_ary_push(result, version_spec_to_value(*i));
            return result;
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     version_metadata(qualified_package_name, version_spec) -> VersionMetadata
     *
     * Fetch metadata.
     */
    VALUE
    repository_version_metadata(VALUE self, VALUE name, VALUE version)
    {
        try
        {
            Repository::ConstPointer * self_ptr;
            Data_Get_Struct(self, Repository::ConstPointer, self_ptr);
            return version_metadata_to_value((*self_ptr)->version_metadata(value_to_qualified_package_name(name),
                        value_to_version_spec(version)));
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
    template <typename T_, T_ * RepositoryCapabilities::* m_>
    struct Interface
    {
        static VALUE fetch(VALUE self)
        {
            Repository::ConstPointer * self_ptr;
            Data_Get_Struct(self, Repository::ConstPointer, self_ptr);
            return ((**self_ptr).*m_) ? self : Qnil;
        }
    };

    /*
     * call-seq:
     *      repository_info(verbose) -> RepositoryInfo
     *
     * Fetch our RepositoryInfo
     */
    VALUE
    repository_info(VALUE self, VALUE verbose)
    {
        Repository::ConstPointer * self_ptr;
        Data_Get_Struct(self, Repository::ConstPointer, self_ptr);

        RepositoryInfo::ConstPointer * p = new RepositoryInfo::ConstPointer((*self_ptr)->info(Qfalse == verbose));
        return Data_Wrap_Struct(c_repository_info, 0, &Common<RepositoryInfo::ConstPointer>::free, p);
    }

    /*
     * call-seq:
     *     info_sections -> Array
     *
     * Fetch an array of our sections.
     */
    VALUE
    repository_info_sections(VALUE self)
    {
        try
        {
            RepositoryInfo::ConstPointer * self_ptr;
            Data_Get_Struct(self, RepositoryInfo::ConstPointer, self_ptr);

            VALUE result(rb_ary_new());
            for (RepositoryInfo::SectionIterator i((*self_ptr)->begin_sections()),
                    i_end((*self_ptr)->end_sections()) ; i != i_end ; ++i)
            {
                RepositoryInfoSection::ConstPointer * s(new RepositoryInfoSection::ConstPointer(*i));
                rb_ary_push(result, Data_Wrap_Struct(c_repository_info_section, 0, &Common<RepositoryInfo::ConstPointer>::free, s));
            }
            return result;
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     kvs -> Hash
     *
     * Returns the key/value pairs within the section.
     */
    VALUE
    repository_info_section_kvs(VALUE self)
    {
        try
        {
            RepositoryInfoSection::ConstPointer * self_ptr;
            Data_Get_Struct(self, RepositoryInfoSection::ConstPointer, self_ptr);

            VALUE result(rb_hash_new());
            for (RepositoryInfoSection::KeyValueIterator i((*self_ptr)->begin_kvs()),
                    i_end((*self_ptr)->end_kvs()) ; i != i_end ; ++i)
                rb_hash_aset(result, rb_str_new2(i->first.c_str()), rb_str_new2(i->second.c_str()));
            return result;
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     header -> String
     *
     * Our heading
     */
    VALUE
    repository_info_section_header(VALUE self)
    {
        RepositoryInfoSection::ConstPointer * self_ptr;
        Data_Get_Struct(self, RepositoryInfoSection::ConstPointer, self_ptr);
        return rb_str_new2((*self_ptr)->heading().c_str());
    }

    /*
     * call-seq:
     *     contents(qualified_package_name, version_spec) -> Contents or Nil
     *
     * Fetches the package contents, if the Repository includes the installed_interface
     */
    VALUE
    repository_contents(VALUE self, VALUE qpn, VALUE vs)
    {
        try
        {
            Repository::ConstPointer * self_ptr;
            Data_Get_Struct(self, Repository::ConstPointer, self_ptr);
            const RepositoryInstalledInterface * const installed_interface ((**self_ptr).installed_interface);
            if (installed_interface)
            {
                return contents_to_value(
                        installed_interface->contents(
                            value_to_qualified_package_name(qpn),
                            value_to_version_spec(vs)
                            )
                        );
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
     *     installed_time(qualified_package_name, version_spec) -> Array or Nil
     *
     * Fetches the package install time, if the Repository includes the installed_interface
     */
    VALUE
    repository_installed_time(VALUE self, VALUE qpn, VALUE vs)
    {
        try
        {
            Repository::ConstPointer * self_ptr;
            Data_Get_Struct(self, Repository::ConstPointer, self_ptr);
            const RepositoryInstalledInterface * const installed_interface ((**self_ptr).installed_interface);
            if (installed_interface)
            {
                return rb_time_new(installed_interface->installed_time(
                            value_to_qualified_package_name(qpn),
                            value_to_version_spec(vs)
                            ), 0);
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
     * Fetch an array of our profiles, as PortageRepositoryProfilesDescLine.
     */
    VALUE
    portage_repository_profiles(VALUE self)
    {
        try
        {
            PortageRepository::ConstPointer * self_ptr;
            Data_Get_Struct(self, PortageRepository::ConstPointer, self_ptr);

            VALUE result(rb_ary_new());
            for (PortageRepository::ProfilesIterator i((*self_ptr)->begin_profiles()),
                    i_end((*self_ptr)->end_profiles()) ; i != i_end ; ++i)
            {
                rb_ary_push(result, portage_repository_profiles_desc_line_to_value(*i));
            }
            return result;
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
     * Fetch arch for this PortageRepositoryProfilesDescLine.
     */
    /*
     * Document-method: status
     *
     * call-seq:
     *     status -> String
     *
     * Fetch status for this PortageRepositoryProfilesDescLine.
     */
    /*
     * Document-method: path
     *
     * call-seq:
     *     path -> String
     *
     * Fetch path to this PortageRepositoryProfilesDescLine.
     */
    template <typename T_, T_ PortageRepositoryProfilesDescLine::* m_>
    struct DescLineValue
    {
        static VALUE
        fetch(VALUE self)
        {
            PortageRepositoryProfilesDescLine * ptr;
            Data_Get_Struct(self, PortageRepositoryProfilesDescLine, ptr);
            return rb_str_new2(stringify((*ptr).*m_).c_str());
        }
    };

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

    template <typename T_, T_ (RepositoryUseInterface::* m_) (const UseFlagName &, const PackageDatabaseEntry *) const> struct QueryUseMessage;

    template <typename T_, T_ trueval_, T_ falseval_, T_ (RepositoryUseInterface::* m_) (const UseFlagName &, const PackageDatabaseEntry *) const>
    struct QueryUse
    {
        static VALUE
        query(int argc, VALUE * argv, VALUE self)
        {
            try
            {
                Repository::ConstPointer * self_ptr;
                Data_Get_Struct(self, Repository::ConstPointer, self_ptr);
                const RepositoryUseInterface * const use_interface ((**self_ptr).use_interface);

                if (use_interface)
                {
                    if (1 != argc && 2 != argc) {
                        rb_raise(rb_eArgError, QueryUseMessage<T_, m_>::message, argc);
                    }

                    T_ status;

                    if (1 == argc)
                        status = ((*use_interface).*m_)(UseFlagName(StringValuePtr(argv[0])), 0);
                    else
                    {
                        PackageDatabaseEntry pde = value_to_package_database_entry(argv[1]);
                        status = ((*use_interface).*m_)(UseFlagName(StringValuePtr(argv[0])), &pde);
                    }

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
    const char * QueryUseMessage<UseFlagState, &RepositoryUseInterface::query_use>::message = "Repository.query_use expects one or two arguments, but got %d";

    template<>
    struct QueryUseMessage<bool, &RepositoryUseInterface::query_use_mask>
    {
        static const char * message;
    };
    const char * QueryUseMessage<bool, &RepositoryUseInterface::query_use_mask>::message = "Repository.query_use_mask expects one or two arguments, but got %d";

    template<>
    struct QueryUseMessage<bool, &RepositoryUseInterface::query_use_force>
    {
        static const char * message;
    };
    const char * QueryUseMessage<bool, &RepositoryUseInterface::query_use_force>::message = "Repository.query_use_force expects one or two arguments, but got %d";

    /*
     * Document-method: query_repository_masks
     *
     * call-seq:
     *     query_repository_masks(qualified_package_name, version_spec) -> true or false or nil
     *
     * Query repository masks.  nil if the repository doesn't implement mask_interface.
     */
    /*
     * Document-method: query_profile_masks
     *
     * call-seq:
     *     query_profile_masks(qualified_package_name, version_spec) -> true or false or nil
     *
     * Query profile masks.  nil if the repository doesn't implement mask_interface.
     */

    template <bool (RepositoryMaskInterface::* m_) (const QualifiedPackageName &, const VersionSpec &) const>
    struct QueryMasks
    {
        static VALUE
        query(VALUE self, VALUE qpn, VALUE ver)
        {
            try
            {
                Repository::ConstPointer * self_ptr;
                Data_Get_Struct(self, Repository::ConstPointer, self_ptr);
                const RepositoryMaskInterface * const mask_interface ((**self_ptr).mask_interface);

                if (mask_interface)
                {
                    QualifiedPackageName q = value_to_qualified_package_name(qpn);
                    VersionSpec v = value_to_version_spec(ver);

                    return ((*mask_interface).*m_)(q, v) ? Qtrue : Qfalse;
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

    /*
     * call-seq:
     *     describe_use_flag(flag_name) -> String or Nil
     *     describe_use_flag(flag_name, package_database_entry) -> String or Nil
     *
     * Returns the description for a use flag name, or nil if the repository does not include 
     * the use_flag_interface.
     */
    VALUE
    repository_describe_use_flag(int argc, VALUE * argv, VALUE self)
    {
        try
        {
            Repository::ConstPointer * self_ptr;
            Data_Get_Struct(self, Repository::ConstPointer, self_ptr);
            if ((*self_ptr)->use_interface) {
                if (1 == argc || 2 ==argc)
                {
                    UseFlagName ufn = UseFlagName(StringValuePtr(argv[0]));
                    PackageDatabaseEntry * pde(0);
                    if (2 == argc)
                    {
                        PackageDatabaseEntry pde2 = value_to_package_database_entry(argv[1]);
                        pde = &pde2;
                    }

                    return rb_str_new2(((*self_ptr)->use_interface->describe_use_flag(ufn, pde).c_str()));

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
        rb_define_method(c_repository, "has_version?", RUBY_FUNC_CAST(&repository_has_version), 2);

        rb_define_method(c_repository, "category_names", RUBY_FUNC_CAST(&repository_category_names), 0);
        rb_define_method(c_repository, "category_names_containing_package",
                RUBY_FUNC_CAST(&repository_category_names_containing_package), 1);
        rb_define_method(c_repository, "package_names", RUBY_FUNC_CAST(&repository_package_names), 1);
        rb_define_method(c_repository, "version_specs", RUBY_FUNC_CAST(&repository_version_specs), 1);

        rb_define_method(c_repository, "version_metadata", RUBY_FUNC_CAST(&repository_version_metadata), 2);

        rb_define_method(c_repository, "installable_interface", RUBY_FUNC_CAST((&Interface<RepositoryInstallableInterface,
                        &Repository::installable_interface>::fetch)), 0);
        rb_define_method(c_repository, "installed_interface", RUBY_FUNC_CAST((&Interface<RepositoryInstalledInterface,
                        &Repository::installed_interface>::fetch)), 0);
        rb_define_method(c_repository, "mask_interface", RUBY_FUNC_CAST((&Interface<RepositoryMaskInterface,
                        &Repository::mask_interface>::fetch)), 0);
        rb_define_method(c_repository, "news_interface", RUBY_FUNC_CAST((&Interface<RepositoryNewsInterface,
                        &Repository::news_interface>::fetch)), 0);
        rb_define_method(c_repository, "sets_interface", RUBY_FUNC_CAST((&Interface<RepositorySetsInterface,
                        &Repository::sets_interface>::fetch)), 0);
        rb_define_method(c_repository, "syncable_interface", RUBY_FUNC_CAST((&Interface<RepositorySyncableInterface,
                        &Repository::syncable_interface>::fetch)), 0);
        rb_define_method(c_repository, "uninstallable_interface", RUBY_FUNC_CAST((&Interface<RepositoryUninstallableInterface,
                        &Repository::uninstallable_interface>::fetch)), 0);
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

        rb_define_method(c_repository, "info", RUBY_FUNC_CAST(&repository_info), 1);
        rb_define_method(c_repository, "contents", RUBY_FUNC_CAST(&repository_contents), 2);
        rb_define_method(c_repository, "installed_time", RUBY_FUNC_CAST(&repository_installed_time), 2);

        rb_define_method(c_repository, "query_use", RUBY_FUNC_CAST((&QueryUse<UseFlagState, use_enabled, use_disabled, &RepositoryUseInterface::query_use>::query)), -1);
        rb_define_method(c_repository, "query_use_mask", RUBY_FUNC_CAST((&QueryUse<bool, true, false, &RepositoryUseInterface::query_use_mask>::query)), -1);
        rb_define_method(c_repository, "query_use_force", RUBY_FUNC_CAST((&QueryUse<bool, true, false, &RepositoryUseInterface::query_use_force>::query)), -1);

        rb_define_method(c_repository, "query_repository_masks", RUBY_FUNC_CAST(&QueryMasks<&RepositoryMaskInterface::query_repository_masks>::query), 2);
        rb_define_method(c_repository, "query_profile_masks", RUBY_FUNC_CAST(&QueryMasks<&RepositoryMaskInterface::query_profile_masks>::query), 2);

        rb_define_method(c_repository, "describe_use_flag", RUBY_FUNC_CAST(&repository_describe_use_flag),-1);

        /*
         * Document-class: Paludis::RepositoryInfo
         *
         * Information about a Repository, for the end user.
         */
        c_repository_info = rb_define_class_under(paludis_module(), "RepositoryInfo", rb_cObject);
        rb_funcall(c_repository_info, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_method(c_repository_info, "sections", RUBY_FUNC_CAST(&repository_info_sections), 0);

        /*
         * Document-class: Paludis::RepositoryInfoSection
         *
         * A section of information about a Repository.
         */
        c_repository_info_section = rb_define_class_under(paludis_module(), "RepositoryInfoSection", rb_cObject);
        rb_funcall(c_repository_info_section, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_method(c_repository_info_section, "kvs", RUBY_FUNC_CAST(&repository_info_section_kvs), 0);
        rb_define_method(c_repository_info_section, "header", RUBY_FUNC_CAST(&repository_info_section_header), 0);

        /*
         * Document-class: Paludis::PortageRepository
         *
         * A PortageRepository is a Repository that handles the layout used by Portage for the main Gentoo tree.
         */
        c_portage_repository = rb_define_class_under(paludis_module(), "PortageRepository", c_repository);
        rb_define_method(c_portage_repository, "profiles", RUBY_FUNC_CAST(&portage_repository_profiles), 0);

        /*
         * Document-class: Paludis::PortageRepositoryProfilesDescLine
         *
         *
         */
        c_portage_repository_profiles_desc_line = rb_define_class_under(paludis_module(), "PortageRepositoryProfilesDescLine", rb_cObject);
        rb_funcall(c_repository_info, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_method(c_portage_repository_profiles_desc_line, "path",
                RUBY_FUNC_CAST((&DescLineValue<FSEntry,&PortageRepositoryProfilesDescLine::path>::fetch)), 0);
        rb_define_method(c_portage_repository_profiles_desc_line, "arch",
                RUBY_FUNC_CAST((&DescLineValue<std::string,&PortageRepositoryProfilesDescLine::arch>::fetch)), 0);
        rb_define_method(c_portage_repository_profiles_desc_line, "status",
                RUBY_FUNC_CAST((&DescLineValue<std::string,&PortageRepositoryProfilesDescLine::status>::fetch)), 0);
    }
}

VALUE
paludis::ruby::repository_to_value(Repository::ConstPointer m)
{
    Repository::ConstPointer * m_ptr(0);
    try
    {
        m_ptr = new Repository::ConstPointer(m);
        return Data_Wrap_Struct(c_repository, 0, &Common<Repository::ConstPointer>::free, m_ptr);
    }
    catch (const std::exception & e)
    {
        delete m_ptr;
        exception_to_ruby_exception(e);
    }
}

VALUE
paludis::ruby::portage_repository_to_value(PortageRepository::ConstPointer m)
{
    PortageRepository::ConstPointer * m_ptr(0);
    try
    {
        m_ptr = new PortageRepository::ConstPointer(m);
        return Data_Wrap_Struct(c_portage_repository, 0, &Common<PortageRepository::ConstPointer>::free, m_ptr);
    }
    catch (const std::exception & e)
    {
        delete m_ptr;
        exception_to_ruby_exception(e);
    }
}

VALUE
paludis::ruby::portage_repository_profiles_desc_line_to_value(const PortageRepositoryProfilesDescLine & v)
{
    PortageRepositoryProfilesDescLine * vv(new PortageRepositoryProfilesDescLine(v));
    return Data_Wrap_Struct(c_portage_repository_profiles_desc_line, 0, &Common<PortageRepositoryProfilesDescLine>::free, vv);
}

PortageRepositoryProfilesDescLine
paludis::ruby::value_to_portage_repository_profiles_desc_line(VALUE v)
{
    if (rb_obj_is_kind_of(v, c_portage_repository_profiles_desc_line))
    {
        PortageRepositoryProfilesDescLine * v_ptr;
        Data_Get_Struct(v, PortageRepositoryProfilesDescLine, v_ptr);
        return *v_ptr;
    }
    else
    {
        rb_raise(rb_eTypeError, "Can't convert %s into PortageRepositoryProfilesDescLine", rb_obj_classname(v));
    }
}

RegisterRubyClass::Register paludis_ruby_register_repository PALUDIS_ATTRIBUTE((used))
    (&do_register_repository);


