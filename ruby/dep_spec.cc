/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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
#include <paludis/dep_spec.hh>
#include <ruby.h>

using namespace paludis;
using namespace paludis::ruby;

#define RUBY_FUNC_CAST(x) reinterpret_cast<VALUE (*)(...)>(x)

namespace
{
    static VALUE c_dep_spec;
    static VALUE c_composite_dep_spec;
    static VALUE c_package_dep_spec;
    static VALUE c_plain_text_dep_spec;
    static VALUE c_all_dep_spec;
    static VALUE c_any_dep_spec;
    static VALUE c_use_dep_spec;
    static VALUE c_block_dep_spec;
    static VALUE c_string_dep_spec;
    static VALUE c_version_requirements_mode;

    VALUE
    dep_spec_init_1(VALUE self, VALUE)
    {
        return self;
    }

    VALUE
    block_dep_spec_new(VALUE self, VALUE spec)
    {
        std::tr1::shared_ptr<const BlockDepSpec> * ptr(0);
        try
        {
            std::tr1::shared_ptr<const PackageDepSpec> pkg(value_to_package_dep_spec(spec));
            ptr = new std::tr1::shared_ptr<const BlockDepSpec>(new BlockDepSpec(pkg));
            VALUE tdata(Data_Wrap_Struct(self, 0, &Common<std::tr1::shared_ptr<const BlockDepSpec> >::free, ptr));
            rb_obj_call_init(tdata, 1, &spec);
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
     *     blocked_spec -> DepSpec
     *
     * Fetch the DepSpec we're blocking.
     */
    VALUE
    block_dep_spec_blocked_spec(VALUE self)
    {
        std::tr1::shared_ptr<const BlockDepSpec> * p;
        Data_Get_Struct(self, std::tr1::shared_ptr<const BlockDepSpec>, p);
        return dep_spec_to_value((*p)->blocked_spec());
    }

    template <typename A_>
    struct DepSpecThings
    {
        static VALUE
        dep_spec_new_1(VALUE self, VALUE s)
        {
            std::tr1::shared_ptr<const A_> * ptr(0);
            try
            {
                ptr = new std::tr1::shared_ptr<const A_>(new A_(StringValuePtr(s)));
                VALUE tdata(Data_Wrap_Struct(self, 0, &Common<std::tr1::shared_ptr<const A_> >::free, ptr));
                rb_obj_call_init(tdata, 1, &s);
                return tdata;
            }
            catch (const std::exception & e)
            {
                delete ptr;
                exception_to_ruby_exception(e);
            }
        }
    };

    /*
     * call-seq:
     *     flag -> String
     *
     * Fetch our use flag name.
     */
    VALUE
    use_dep_spec_flag(VALUE self)
    {
        std::tr1::shared_ptr<const UseDepSpec> * p;
        Data_Get_Struct(self, std::tr1::shared_ptr<const UseDepSpec>, p);
        return rb_str_new2(stringify((*p)->flag()).c_str());
    }

    /*
     * call-seq:
     *     inverse? -> true or false
     *
     * Fetch whether we are a ! flag.
     */
    VALUE
    use_dep_spec_inverse(VALUE self)
    {
        std::tr1::shared_ptr<const UseDepSpec> * p;
        Data_Get_Struct(self, std::tr1::shared_ptr<const UseDepSpec>, p);
        return (*p)->inverse() ? Qtrue : Qfalse;
    }

    /*
     * call-seq: each {|spec| block }
     *
     * Iterate over each child DepSpec.
     */
    VALUE
    composite_dep_spec_each(VALUE self)
    {
        std::tr1::shared_ptr<CompositeDepSpec> * m_ptr;
        Data_Get_Struct(self, std::tr1::shared_ptr<CompositeDepSpec>, m_ptr);
        for (CompositeDepSpec::Iterator i((*m_ptr)->begin()), i_end((*m_ptr)->end()) ; i != i_end ; ++i)
            rb_yield(dep_spec_to_value(*i));
        return self;
    }

    /*
     * call-seq:
     *     package -> String or Nil
     *
     * Fetch the package name.
     */
    VALUE
    package_dep_spec_package(VALUE self)
    {
        std::tr1::shared_ptr<const PackageDepSpec> * ptr;
        Data_Get_Struct(self, std::tr1::shared_ptr<const PackageDepSpec>, ptr);
        if (0 == (*ptr)->package_ptr())
            return Qnil;
        return rb_str_new2(stringify(*(*ptr)->package_ptr()).c_str());
    }

    /*
     * call-seq:
     *     text -> String
     *
     * Fetch our text.
     */
    VALUE
    package_dep_spec_text(VALUE self)
    {
        std::tr1::shared_ptr<const PackageDepSpec> * ptr;
        Data_Get_Struct(self, std::tr1::shared_ptr<const PackageDepSpec>, ptr);
        return rb_str_new2(stringify((*ptr)->text()).c_str());
    }

    /*
     * call-seq:
     *     slot -> String or Nil
     *
     * Fetch the slot name.
     */
    VALUE
    package_dep_spec_slot_ptr(VALUE self)
    {
        std::tr1::shared_ptr<const PackageDepSpec> * ptr;
        Data_Get_Struct(self, std::tr1::shared_ptr<const PackageDepSpec>, ptr);
        if (0 == (*ptr)->slot_ptr())
            return Qnil;
        return rb_str_new2(stringify((*(*ptr)->slot_ptr())).c_str());
    }

    /*
     * call-seq:
     *     repository -> String or Nil
     *
     * Fetch the repository name.
     */
    VALUE
    package_dep_spec_repository_ptr(VALUE self)
    {
        std::tr1::shared_ptr<const PackageDepSpec> * ptr;
        Data_Get_Struct(self, std::tr1::shared_ptr<const PackageDepSpec>, ptr);
        if (0 == (*ptr)->repository_ptr())
            return Qnil;
        return rb_str_new2(stringify((*(*ptr)->repository_ptr())).c_str());
    }

    /*
     * call-seq:
     *     version_requirements -> Array
     *
     * Fetch the version requirements. E.g. [ {:operator => '=', :spec => VersionSpec.new('0.1') } ]
     */
    VALUE
    package_dep_spec_version_requirements_ptr(VALUE self)
    {
        std::tr1::shared_ptr<const PackageDepSpec> * ptr;
        Data_Get_Struct(self, std::tr1::shared_ptr<const PackageDepSpec>, ptr);
        VALUE result(rb_ary_new());
        VALUE result_hash;
        if ((*ptr)->version_requirements_ptr())
            for (VersionRequirements::Iterator i((*ptr)->version_requirements_ptr()->begin()),
                        i_end((*ptr)->version_requirements_ptr()->end()) ; i != i_end; ++i)
            {
                result_hash = rb_hash_new();
                rb_hash_aset(result_hash, ID2SYM(rb_intern("operator")),
                    rb_str_new2(stringify(i->version_operator).c_str()));
                rb_hash_aset(result_hash, ID2SYM(rb_intern("spec")),
                    version_spec_to_value(i->version_spec));
                rb_ary_push(result, result_hash);
            }
        return result;
    }

    VALUE
    package_dep_spec_version_requirements_mode(VALUE self)
    {
        std::tr1::shared_ptr<const PackageDepSpec> * ptr;
        Data_Get_Struct(self, std::tr1::shared_ptr<const PackageDepSpec>, ptr);
        return INT2FIX((*ptr)->version_requirements_mode());
    }

    void do_register_dep_spec()
    {
        /*
         * Document-class: Paludis::DepSpec
         *
         * Base class for a dependency spec.
         */
        c_dep_spec = rb_define_class_under(paludis_module(), "DepSpec", rb_cObject);
        rb_funcall(c_dep_spec, rb_intern("private_class_method"), 1, rb_str_new2("new"));

        /*
         * Document-class: Paludis::CompositeDepSpec
         *
         * Class for dependency specs that have a number of child dependency specs.
         * Includes Enumerable[http://www.ruby-doc.org/core/classes/Enumerable.html]
         * but not Comparable.
         */
        c_composite_dep_spec = rb_define_class_under(paludis_module(), "CompositeDepSpec", c_dep_spec);
        rb_funcall(c_composite_dep_spec, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_method(c_composite_dep_spec, "each", RUBY_FUNC_CAST(&composite_dep_spec_each), 0);
        rb_include_module(c_composite_dep_spec, rb_mEnumerable);

        /*
         * Document-class: Paludis::AllDepSpec
         *
         * Represents a ( first second third ) or top level group of dependency specs.
         */
        c_all_dep_spec = rb_define_class_under(paludis_module(), "AllDepSpec", c_composite_dep_spec);
        rb_funcall(c_all_dep_spec, rb_intern("private_class_method"), 1, rb_str_new2("new"));

        /*
         * Document-class: Paludis::AnyDepSpec
         *
         * Represents a "|| ( )" dependency block.
         */
        c_any_dep_spec = rb_define_class_under(paludis_module(), "AnyDepSpec", c_composite_dep_spec);
        rb_funcall(c_any_dep_spec, rb_intern("private_class_method"), 1, rb_str_new2("new"));

        /*
         * Document-class: Paludis::UseDepSpec
         *
         * Represents a use? ( ) dependency spec.
         */
        c_use_dep_spec = rb_define_class_under(paludis_module(), "UseDepSpec", c_composite_dep_spec);
        rb_funcall(c_use_dep_spec, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_method(c_use_dep_spec, "flag", RUBY_FUNC_CAST(&use_dep_spec_flag), 0);
        rb_define_method(c_use_dep_spec, "inverse?", RUBY_FUNC_CAST(&use_dep_spec_inverse), 0);

        /*
         * Document-class: Paludis::StringDepSpec
         *
         * A StringDepSpec represents a non-composite dep spec with an associated piece of text.
         */
        c_string_dep_spec = rb_define_class_under(paludis_module(), "StringDepSpec", c_dep_spec);
        rb_funcall(c_string_dep_spec, rb_intern("private_class_method"), 1, rb_str_new2("new"));

        /*
         * Document-class: Paludis::PackageDepSpec
         *
         * A PackageDepSpec represents a package name (for example, 'app-editors/vim'),
         * possibly with associated version and SLOT restrictions.
         */
        c_package_dep_spec = rb_define_class_under(paludis_module(), "PackageDepSpec", c_string_dep_spec);
        rb_define_singleton_method(c_package_dep_spec, "new", RUBY_FUNC_CAST(&DepSpecThings<PackageDepSpec>::dep_spec_new_1), 1);
        rb_define_method(c_package_dep_spec, "initialize", RUBY_FUNC_CAST(&dep_spec_init_1), 1);
        rb_define_method(c_package_dep_spec, "to_s", RUBY_FUNC_CAST(&Common<std::tr1::shared_ptr<const PackageDepSpec> >::to_s_via_ptr), 0);
        rb_define_method(c_package_dep_spec, "package", RUBY_FUNC_CAST(&package_dep_spec_package), 0);
        rb_define_method(c_package_dep_spec, "text", RUBY_FUNC_CAST(&package_dep_spec_text), 0);
        rb_define_method(c_package_dep_spec, "slot", RUBY_FUNC_CAST(&package_dep_spec_slot_ptr), 0);
        rb_define_method(c_package_dep_spec, "repository", RUBY_FUNC_CAST(&package_dep_spec_repository_ptr), 0);
        rb_define_method(c_package_dep_spec, "version_requirements", RUBY_FUNC_CAST(&package_dep_spec_version_requirements_ptr), 0);
        rb_define_method(c_package_dep_spec, "version_requirements_mode", RUBY_FUNC_CAST(&package_dep_spec_version_requirements_mode), 0);

        /*
         * Document-class: Paludis::PlainTextDepSpec
         *
         * A PlainTextDepSpec represents a plain text entry (for example, a URI in SRC_URI).
         */
        c_plain_text_dep_spec = rb_define_class_under(paludis_module(), "PlainTextDepSpec", c_string_dep_spec);
        rb_define_singleton_method(c_plain_text_dep_spec, "new", RUBY_FUNC_CAST(&DepSpecThings<PlainTextDepSpec>::dep_spec_new_1), 1);
        rb_define_method(c_plain_text_dep_spec, "initialize", RUBY_FUNC_CAST(&dep_spec_init_1), 1);
        rb_define_method(c_plain_text_dep_spec, "to_s", RUBY_FUNC_CAST(&Common<std::tr1::shared_ptr<const PlainTextDepSpec> >::to_s_via_ptr), 0);

        /*
         * Document-class: Paludis::BlockDepSpec
         *
         * A BlockDepSpec represents a block on a package name (for example, 'app-editors/vim'), possibly with
         * associated version and SLOT restrictions.
         */
        c_block_dep_spec = rb_define_class_under(paludis_module(), "BlockDepSpec", c_string_dep_spec);
        rb_define_singleton_method(c_block_dep_spec, "new", RUBY_FUNC_CAST(&block_dep_spec_new), 1);
        rb_define_method(c_block_dep_spec, "initialize", RUBY_FUNC_CAST(&dep_spec_init_1), 1);
        rb_define_method(c_block_dep_spec, "blocked_spec", RUBY_FUNC_CAST(&block_dep_spec_blocked_spec), 0);

        /*
         * Document-module: Paludis::VersionRequirementsMode
         *
         * What sort of VersionRequirements to we have.
         *
         */
        c_version_requirements_mode = rb_define_module_under(paludis_module(), "VersionRequirementsMode");
        for (VersionRequirementsMode l(static_cast<VersionRequirementsMode>(0)), l_end(last_vr) ; l != l_end ;
                l = static_cast<VersionRequirementsMode>(static_cast<int>(l) + 1))
            rb_define_const(c_version_requirements_mode, value_case_to_RubyCase(stringify(l)).c_str(), INT2FIX(l));

        // cc_enum_special<paludis/version_requirements.hh, VersionRequirementsMode, c_version_requirements_mode>
    }
}

std::tr1::shared_ptr<const PackageDepSpec>
paludis::ruby::value_to_package_dep_spec(VALUE v)
{
    if (T_STRING == TYPE(v))
        return std::tr1::shared_ptr<const PackageDepSpec>(new PackageDepSpec(StringValuePtr(v)));
    else if (rb_obj_is_kind_of(v, c_package_dep_spec))
    {
        std::tr1::shared_ptr<const PackageDepSpec> * v_ptr;
        Data_Get_Struct(v, std::tr1::shared_ptr<const PackageDepSpec>, v_ptr);
        return *v_ptr;
    }
    else
    {
        rb_raise(rb_eTypeError, "Can't convert %s into PackageDepSpec", rb_obj_classname(v));
    }
}

std::tr1::shared_ptr<const DepSpec>
paludis::ruby::value_to_dep_spec(VALUE v)
{
    if (rb_obj_is_kind_of(v, c_dep_spec))
    {
        std::tr1::shared_ptr<const DepSpec> * v_ptr;
        Data_Get_Struct(v, std::tr1::shared_ptr<const DepSpec>, v_ptr);
        return *v_ptr;
    }
    else
    {
        rb_raise(rb_eTypeError, "Can't convert %s into PackageDepSpec", rb_obj_classname(v));
    }
}

VALUE
paludis::ruby::dep_spec_to_value(std::tr1::shared_ptr<const DepSpec> m)
{
    struct V :
        DepSpecVisitorTypes::ConstVisitor
    {
        VALUE value;
        std::tr1::shared_ptr<const DepSpec> mm;

        V(std::tr1::shared_ptr<const DepSpec> _m) :
            mm(_m)
        {
        }

        void visit(const AllDepSpec *)
        {
            value = Data_Wrap_Struct(c_all_dep_spec, 0, &Common<std::tr1::shared_ptr<const AllDepSpec> >::free,
                    new std::tr1::shared_ptr<const AllDepSpec>(std::tr1::static_pointer_cast<const AllDepSpec>(mm)));
        }

        void visit(const AnyDepSpec *)
        {
            value = Data_Wrap_Struct(c_any_dep_spec, 0, &Common<std::tr1::shared_ptr<const AnyDepSpec> >::free,
                    new std::tr1::shared_ptr<const AnyDepSpec>(std::tr1::static_pointer_cast<const AnyDepSpec>(mm)));
        }

        void visit(const UseDepSpec *)
        {
            value = Data_Wrap_Struct(c_use_dep_spec, 0, &Common<std::tr1::shared_ptr<const UseDepSpec> >::free,
                    new std::tr1::shared_ptr<const UseDepSpec>(std::tr1::static_pointer_cast<const UseDepSpec>(mm)));
        }

        void visit(const PlainTextDepSpec *)
        {
            value = Data_Wrap_Struct(c_plain_text_dep_spec, 0, &Common<std::tr1::shared_ptr<const PlainTextDepSpec> >::free,
                    new std::tr1::shared_ptr<const PlainTextDepSpec>(std::tr1::static_pointer_cast<const PlainTextDepSpec>(mm)));
        }

        void visit(const PackageDepSpec *)
        {
            value = Data_Wrap_Struct(c_package_dep_spec, 0, &Common<std::tr1::shared_ptr<const PackageDepSpec> >::free,
                    new std::tr1::shared_ptr<const PackageDepSpec>(std::tr1::static_pointer_cast<const PackageDepSpec>(mm)));
        }

        void visit(const BlockDepSpec *)
        {
            value = Data_Wrap_Struct(c_block_dep_spec, 0, &Common<std::tr1::shared_ptr<const BlockDepSpec> >::free,
                    new std::tr1::shared_ptr<const BlockDepSpec>(std::tr1::static_pointer_cast<const BlockDepSpec>(mm)));
        }
    };

    if (0 == m)
        return Qnil;

    std::tr1::shared_ptr<const DepSpec> * m_ptr(0);
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

RegisterRubyClass::Register paludis_ruby_register_dep_spec PALUDIS_ATTRIBUTE((used))
    (&do_register_dep_spec);

