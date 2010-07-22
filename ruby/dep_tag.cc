/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Richard Brown
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
#include <paludis/dep_spec.hh>
#include <paludis/dep_tag.hh>
#include <ruby.h>

using namespace paludis;
using namespace paludis::ruby;

namespace
{
    static VALUE c_dep_tag;
    static VALUE c_dependency_dep_tag;
    static VALUE c_glsa_dep_tag;
    static VALUE c_general_set_dep_tag;
    static VALUE c_target_dep_tag;

    struct V
    {
        VALUE value;
        std::shared_ptr<const DepTag> mm;

        V(std::shared_ptr<const DepTag> _m) :
            mm(_m)
        {
        }

        void visit(const DependencyDepTag &)
        {
            value = Data_Wrap_Struct(c_dependency_dep_tag, 0, &Common<std::shared_ptr<const DependencyDepTag> >::free,
                    new std::shared_ptr<const DepTag>(mm));
        }

        void visit(const GLSADepTag &)
        {
            value = Data_Wrap_Struct(c_glsa_dep_tag, 0, &Common<std::shared_ptr<const DependencyDepTag> >::free,
                    new std::shared_ptr<const DepTag>(mm));
        }

        void visit(const GeneralSetDepTag &)
        {
            value = Data_Wrap_Struct(c_general_set_dep_tag, 0, &Common<std::shared_ptr<const DependencyDepTag> >::free,
                    new std::shared_ptr<const DepTag>(mm));
        }

        void visit(const TargetDepTag &)
        {
            value = Data_Wrap_Struct(c_target_dep_tag, 0, &Common<std::shared_ptr<const DependencyDepTag> >::free,
                    new std::shared_ptr<const DepTag>(mm));
        }
    };

    VALUE
    dep_tag_init_1(int, VALUE*, VALUE self)
    {
        return self;
    }

    VALUE
    dependency_dep_tag_new(int argc, VALUE * argv, VALUE self)
    {
        if (2 != argc)
            rb_raise(rb_eArgError, "DependencyDepTag expects two arguments, but got %d", argc);

        std::shared_ptr<const DepTag> * ptr(0);
        try
        {
            ptr = new std::shared_ptr<const DepTag>(
                    new DependencyDepTag(value_to_package_id(argv[0]),
                        *value_to_package_dep_spec(argv[1])));
            VALUE tdata(Data_Wrap_Struct(self, 0, &Common<std::shared_ptr<const DepTag> >::free, ptr));
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
     *     package_id -> PackageID
     *
     * The PackageID that contains our dependency.
     */
    VALUE
    dependency_dep_tag_package_id(VALUE self)
    {
        std::shared_ptr<const DepTag> * ptr;
        Data_Get_Struct(self, std::shared_ptr<const DepTag>, ptr);
        return package_id_to_value((std::static_pointer_cast<const DependencyDepTag>(*ptr))->package_id());
    }

    /*
     * call-seq:
     *     dependency -> PackageDepSpec
     *
     * The PackageDepSpec that pulled us in.
     */
    VALUE
    dependency_dep_tag_dependency(VALUE self)
    {
        std::shared_ptr<const DepTag> * ptr;
        Data_Get_Struct(self, std::shared_ptr<const DepTag>, ptr);
        return package_dep_spec_to_value((std::static_pointer_cast<const DependencyDepTag>(*ptr))->dependency());
    }

    VALUE
    glsa_dep_tag_new(int argc, VALUE * argv, VALUE self)
    {
        if (3 != argc)
            rb_raise(rb_eArgError, "GLSADepTag expects three arguments, but got %d",argc);

        std::shared_ptr<const DepTag> * ptr(0);
        try
        {
            ptr = new std::shared_ptr<const DepTag>(new GLSADepTag(StringValuePtr(argv[0]), StringValuePtr(argv[1]),
                        FSEntry(StringValuePtr(argv[2]))));
            VALUE tdata(Data_Wrap_Struct(self, 0, &Common<std::shared_ptr<const DepTag> >::free, ptr));
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
    general_set_dep_tag_new(int argc, VALUE * argv, VALUE self)
    {
        if (2 != argc)
            rb_raise(rb_eArgError, "GeneralSetDepTag expects two arguments, but got %d",argc);

        std::shared_ptr<const DepTag> * ptr(0);
        try
        {
            ptr = new std::shared_ptr<const DepTag>(new GeneralSetDepTag(SetName(StringValuePtr(argv[0])), StringValuePtr(argv[1])));
            VALUE tdata(Data_Wrap_Struct(self, 0, &Common<std::shared_ptr<const DepTag> >::free, ptr));
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
    target_dep_tag_new(VALUE self)
    {
        std::shared_ptr<const DepTag> * ptr(0);
        try
        {
            ptr = new std::shared_ptr<const DepTag>(new TargetDepTag);
            VALUE tdata(Data_Wrap_Struct(self, 0, &Common<std::shared_ptr<const DepTag> >::free, ptr));
            rb_obj_call_init(tdata, 0, 0);
            return tdata;
        }
        catch (const std::exception & e)
        {
            delete ptr;
            exception_to_ruby_exception(e);
        }
    }

    /*
     * Document-method: short_text
     *
     * call-seq:
     *     short_text -> String
     *
     * Fetch our short text (for example, 'GLSA-1234') that is
     * displayed with the dep list entry.
     */
    /*
     * Document-method: category
     *
     * call-seq:
     *     category -> String
     *
     * Fetch our DepTagCategory's tag.
     */
    /*
     * Document-method: glsa_title
     *
     * call-seq:
     *     glsa_title -> String
     *
     * Fetch our GLSA title (for example, 'Yet another PHP remote access hole').
     */
    /*
     * Document-method: source
     *
     * call-seq:
     *     source -> String
     *
     * From which repository or environment did we originate?
     */
    template <typename T_, std::string (T_::* m_) () const>
    struct DepTagThings
    {
        static VALUE
        fetch(VALUE self)
        {
            std::shared_ptr<const DepTag> * ptr;
            Data_Get_Struct(self, std::shared_ptr<const DepTag>, ptr);
            return rb_str_new2((((*std::static_pointer_cast<const T_>(*ptr)).*m_)()).c_str());
        }
    };

    /*
     * Document-method: glsa_file
     *
     * call-seq:
     *     glsa_file -> String
     *
     * Fetch our GLSA file.
     */
    template <typename T_, const FSEntry (T_::* m_) () const>
    struct DepTagFSEntryThings
    {
        static VALUE
        fetch(VALUE self)
        {
            std::shared_ptr<const DepTag> * ptr;
            Data_Get_Struct(self, std::shared_ptr<const DepTag>, ptr);
            return rb_str_new2(stringify(((*std::static_pointer_cast<const T_>(*ptr)).*m_)()).c_str());
        }
    };

    void do_register_dep_tag()
    {
        /*
         * Document-class: Paludis::DepTag
         *
         * A DepTag can be associated with a PackageDepSpec, and is transferred onto any associated DepListEntry instances.
         *
         * It is used for tagging dep list entries visually, for example to indicate an associated GLSA.
         */
        c_dep_tag = rb_define_class_under(paludis_module(), "DepTag", rb_cObject);
        rb_funcall(c_dep_tag, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_method(c_dep_tag, "initialize", RUBY_FUNC_CAST(&dep_tag_init_1), -1);
        rb_define_method(c_dep_tag, "short_text", RUBY_FUNC_CAST((&DepTagThings<DepTag,&DepTag::short_text>::fetch)), 0);
        rb_define_method(c_dep_tag, "category", RUBY_FUNC_CAST((&DepTagThings<DepTag,&DepTag::category>::fetch)), 0);

        /*
         * Document-class: Paludis::DependencyDepTag
         *
         * DepTag subclass for dependencies.
         */
        c_dependency_dep_tag = rb_define_class_under(paludis_module(), "DependencyDepTag", c_dep_tag);
        rb_define_singleton_method(c_dependency_dep_tag, "new", RUBY_FUNC_CAST(&dependency_dep_tag_new), -1);
        rb_define_method(c_dependency_dep_tag, "package_id", RUBY_FUNC_CAST(&dependency_dep_tag_package_id), 0);
        rb_define_method(c_dependency_dep_tag, "dependency", RUBY_FUNC_CAST(&dependency_dep_tag_dependency), 0);

        /*
         * Document-class: Paludis::GLSADepTag
         *
         * DepTag subclass for GLSAs.
         */
        c_glsa_dep_tag = rb_define_class_under(paludis_module(), "GLSADepTag", c_dep_tag);
        rb_define_singleton_method(c_glsa_dep_tag, "new", RUBY_FUNC_CAST(&glsa_dep_tag_new), -1);
        rb_define_method(c_glsa_dep_tag, "glsa_title", RUBY_FUNC_CAST((&DepTagThings<GLSADepTag,
                        &GLSADepTag::glsa_title>::fetch)), 0);
        rb_define_method(c_glsa_dep_tag, "glsa_file", RUBY_FUNC_CAST((&DepTagFSEntryThings<GLSADepTag,
                        &GLSADepTag::glsa_file>::fetch)), 0);

        /*
         * Document-class: Paludis::GeneralSetDepTag
         *
         * DepTag subclass for general sets.
         */
        c_general_set_dep_tag = rb_define_class_under(paludis_module(), "GeneralSetDepTag", c_dep_tag);
        rb_define_singleton_method(c_general_set_dep_tag, "new", RUBY_FUNC_CAST(&general_set_dep_tag_new), -1);
        rb_define_method(c_general_set_dep_tag, "source", RUBY_FUNC_CAST((&DepTagThings<GeneralSetDepTag,
                        &GeneralSetDepTag::source>::fetch)), 0);

        /*
         * Document-class: Paludis::TargetDepTag
         *
         * DepTag subclass for general sets.
         */
        c_target_dep_tag = rb_define_class_under(paludis_module(), "TargetDepTag", c_dep_tag);
        rb_define_singleton_method(c_target_dep_tag, "new", RUBY_FUNC_CAST(&target_dep_tag_new), 0);
    }
}

VALUE
paludis::ruby::dep_tag_to_value(std::shared_ptr<const DepTag> m)
{
    try
    {
        V v(m);
        m->accept(v);
        return v.value;
    }
    catch (const std::exception & e)
    {
        exception_to_ruby_exception(e);
    }
}

std::shared_ptr<const DepTag>
paludis::ruby::value_to_dep_tag(VALUE v)
{
    if (rb_obj_is_kind_of(v, c_dep_tag))
    {
        std::shared_ptr<const DepTag> * v_ptr;
        Data_Get_Struct(v, std::shared_ptr<const DepTag>, v_ptr);
        return *v_ptr;
    }
    else
    {
        rb_raise(rb_eTypeError, "Can't convert %s into DepTag", rb_obj_classname(v));
    }
}

RegisterRubyClass::Register paludis_ruby_register_dep_tag PALUDIS_ATTRIBUTE((used))
    (&do_register_dep_tag);

