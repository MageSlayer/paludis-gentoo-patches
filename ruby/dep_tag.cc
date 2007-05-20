/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Richard Brown <rbrown@gentoo.org>
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
#include <paludis/dep_tag.hh>
#include <ruby.h>

using namespace paludis;
using namespace paludis::ruby;

#define RUBY_FUNC_CAST(x) reinterpret_cast<VALUE (*)(...)>(x)

namespace
{
    static VALUE c_dep_tag;
    static VALUE c_dependency_dep_tag;
    static VALUE c_glsa_dep_tag;
    static VALUE c_general_set_dep_tag;

    VALUE
    dep_tag_init(VALUE self, VALUE)
    {
        return self;
    }

    VALUE
    dep_tag_init_1(int, VALUE*, VALUE self)
    {
        return self;
    }

    VALUE
    dependency_dep_tag_new(VALUE self, VALUE pde)
    {
        tr1::shared_ptr<const DependencyDepTag> * ptr(0);
        try
        {
            ptr = new tr1::shared_ptr<const DependencyDepTag>(new DependencyDepTag(value_to_package_database_entry(pde)));
            VALUE tdata(Data_Wrap_Struct(self, 0, &Common<tr1::shared_ptr<const DependencyDepTag> >::free, ptr));
            rb_obj_call_init(tdata, 1, &pde);
            return tdata;
        }
        catch (const std::exception & e)
        {
            delete ptr;
            exception_to_ruby_exception(e);
        }
    }

    VALUE
    glsa_dep_tag_new(int argc, VALUE * argv, VALUE self)
    {
        if (2 != argc)
            rb_raise(rb_eArgError, "GLSADepTag expects two arguments, but got %d",argc);

        tr1::shared_ptr<const GLSADepTag> * ptr(0);
        try
        {
            ptr = new tr1::shared_ptr<const GLSADepTag>(new GLSADepTag(StringValuePtr(argv[0]), StringValuePtr(argv[1])));
            VALUE tdata(Data_Wrap_Struct(self, 0, &Common<tr1::shared_ptr<const GLSADepTag> >::free, ptr));
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

        tr1::shared_ptr<const GeneralSetDepTag> * ptr(0);
        try
        {
            ptr = new tr1::shared_ptr<const GeneralSetDepTag>(new GeneralSetDepTag(SetName(StringValuePtr(argv[0])), StringValuePtr(argv[1])));
            VALUE tdata(Data_Wrap_Struct(self, 0, &Common<tr1::shared_ptr<const GeneralSetDepTag> >::free, ptr));
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
            tr1::shared_ptr<const T_> * ptr;
            Data_Get_Struct(self, tr1::shared_ptr<const T_>, ptr);
            return rb_str_new2((((**ptr).*m_)()).c_str());
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
        rb_define_singleton_method(c_dependency_dep_tag, "new", RUBY_FUNC_CAST(&dependency_dep_tag_new), 1);
        rb_define_method(c_dependency_dep_tag, "initialize", RUBY_FUNC_CAST(&dep_tag_init), 1);

        /*
         * Document-class: Paludis::GLSADepTag
         *
         * DepTag subclass for GLSAs.
         */
        c_glsa_dep_tag = rb_define_class_under(paludis_module(), "GLSADepTag", c_dep_tag);
        rb_define_singleton_method(c_glsa_dep_tag, "new", RUBY_FUNC_CAST(&glsa_dep_tag_new), -1);
        rb_define_method(c_glsa_dep_tag, "glsa_title", RUBY_FUNC_CAST((&DepTagThings<GLSADepTag,
                        &GLSADepTag::glsa_title>::fetch)), 0);

        /*
         * Document-class: Paludis::GeneralSetDepTag
         *
         * DepTag subclass for general sets.
         */
        c_general_set_dep_tag = rb_define_class_under(paludis_module(), "GeneralSetDepTag", c_dep_tag);
        rb_define_singleton_method(c_general_set_dep_tag, "new", RUBY_FUNC_CAST(&general_set_dep_tag_new), -1);
        rb_define_method(c_general_set_dep_tag, "source", RUBY_FUNC_CAST((&DepTagThings<GeneralSetDepTag,
                        &GeneralSetDepTag::source>::fetch)), 0);
    }
}

VALUE
paludis::ruby::dep_tag_to_value(tr1::shared_ptr<const DepTag> m)
{
    struct V :
        DepTagVisitorTypes::ConstVisitor
    {
        VALUE value;
        tr1::shared_ptr<const DepTag> mm;

        V(tr1::shared_ptr<const DepTag> _m) :
            mm(_m)
        {
        }

        void visit(const DependencyDepTag *)
        {
            value = Data_Wrap_Struct(c_dependency_dep_tag, 0, &Common<tr1::shared_ptr<const DependencyDepTag> >::free,
                    new tr1::shared_ptr<const DependencyDepTag>(tr1::static_pointer_cast<const DependencyDepTag>(mm)));
        }

        void visit(const GLSADepTag *)
        {
            value = Data_Wrap_Struct(c_glsa_dep_tag, 0, &Common<tr1::shared_ptr<const GLSADepTag> >::free,
                    new tr1::shared_ptr<const GLSADepTag>(tr1::static_pointer_cast<const GLSADepTag>(mm)));
        }

        void visit(const GeneralSetDepTag *)
        {
            value = Data_Wrap_Struct(c_general_set_dep_tag, 0, &Common<tr1::shared_ptr<const GeneralSetDepTag> >::free,
                    new tr1::shared_ptr<const GeneralSetDepTag>(tr1::static_pointer_cast<const GeneralSetDepTag>(mm)));
        }
    };

    tr1::shared_ptr<const DepTag> * m_ptr(0);
    try
    {
        V v(m);
        m->accept(&v);
        return v.value;
    }
    catch (const std::exception & e)
    {
        delete m_ptr;
        exception_to_ruby_exception(e);
    }
}

RegisterRubyClass::Register paludis_ruby_register_dep_tag PALUDIS_ATTRIBUTE((used))
    (&do_register_dep_tag);

