/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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
#include <paludis/dep_atom.hh>
#include <ruby.h>

using namespace paludis;
using namespace paludis::ruby;

#define RUBY_FUNC_CAST(x) reinterpret_cast<VALUE (*)(...)>(x)

namespace
{
    static VALUE c_dep_atom;
    static VALUE c_composite_dep_atom;
    static VALUE c_package_dep_atom;
    static VALUE c_plain_text_dep_atom;
    static VALUE c_all_dep_atom;
    static VALUE c_any_dep_atom;
    static VALUE c_use_dep_atom;
    static VALUE c_block_dep_atom;
    static VALUE c_string_dep_atom;

    VALUE
    dep_atom_init_0(VALUE self)
    {
        return self;
    }

    VALUE
    dep_atom_init_1(VALUE self, VALUE)
    {
        return self;
    }

    VALUE
    dep_atom_init_2(VALUE self, VALUE, VALUE)
    {
        return self;
    }

    VALUE
    use_dep_atom_new(VALUE self, VALUE pkg, VALUE inverse)
    {
        UseDepAtom::ConstPointer * ptr(0);
        try
        {
            ptr = new UseDepAtom::ConstPointer(new UseDepAtom(
                        UseFlagName(StringValuePtr(pkg)), Qfalse != inverse && Qnil != inverse));
            VALUE tdata(Data_Wrap_Struct(self, 0, &Common<UseDepAtom::ConstPointer>::free, ptr));
            rb_obj_call_init(tdata, 2, &pkg);
            return tdata;
        }
        catch (const std::exception & e)
        {
            delete ptr;
            exception_to_ruby_exception(e);
        }
    }

    VALUE
    block_dep_atom_new(VALUE self, VALUE atom)
    {
        BlockDepAtom::ConstPointer * ptr(0);
        try
        {
            PackageDepAtom::ConstPointer pkg(value_to_package_dep_atom(atom));
            ptr = new BlockDepAtom::ConstPointer(new BlockDepAtom(pkg));
            VALUE tdata(Data_Wrap_Struct(self, 0, &Common<BlockDepAtom::ConstPointer>::free, ptr));
            rb_obj_call_init(tdata, 1, &atom);
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
     *     blocked_atom -> DepAtom
     *
     * Fetch the DepAtom we're blocking.
     */
    VALUE
    block_dep_atom_blocked_atom(VALUE self)
    {
        BlockDepAtom::ConstPointer * p;
        Data_Get_Struct(self, BlockDepAtom::ConstPointer, p);
        return dep_atom_to_value((*p)->blocked_atom());
    }

    template <typename A_>
    struct DepAtomThings
    {
        static VALUE
        dep_atom_new_1(VALUE self, VALUE s)
        {
            typename A_::ConstPointer * ptr(0);
            try
            {
                ptr = new typename A_::ConstPointer(new A_(StringValuePtr(s)));
                VALUE tdata(Data_Wrap_Struct(self, 0, &Common<typename A_::ConstPointer>::free, ptr));
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
    use_dep_atom_flag(VALUE self)
    {
        UseDepAtom::ConstPointer * p;
        Data_Get_Struct(self, UseDepAtom::ConstPointer, p);
        return rb_str_new2(stringify((*p)->flag()).c_str());
    }

    /*
     * call-seq:
     *     inverse? -> true or false
     *
     * Fetch whether we are a ! flag.
     */
    VALUE
    use_dep_atom_inverse(VALUE self)
    {
        UseDepAtom::ConstPointer * p;
        Data_Get_Struct(self, UseDepAtom::ConstPointer, p);
        return (*p)->inverse() ? Qtrue : Qfalse;
    }

    /*
     * call-seq: each {|atom| block }
     *
     * Iterate over each child DepAtom.
     */
    VALUE
    composite_dep_atom_each(VALUE self)
    {
        CompositeDepAtom::Pointer * m_ptr;
        Data_Get_Struct(self, CompositeDepAtom::Pointer, m_ptr);
        for (CompositeDepAtom::Iterator i((*m_ptr)->begin()), i_end((*m_ptr)->end()) ; i != i_end ; ++i)
            rb_yield(dep_atom_to_value(*i));
        return self;
    }

    /*
     * call-seq:
     *     package -> String
     *
     * Fetch the package name.
     */
    VALUE
    package_dep_atom_package(VALUE self)
    {
        PackageDepAtom::ConstPointer * ptr;
        Data_Get_Struct(self, PackageDepAtom::ConstPointer, ptr);
        return rb_str_new2(stringify((*ptr)->package()).c_str());
    }

    /*
     * call-seq:
     *     text -> String
     *
     * Fetch our text.
     */
    VALUE
    package_dep_atom_text(VALUE self)
    {
        PackageDepAtom::ConstPointer * ptr;
        Data_Get_Struct(self, PackageDepAtom::ConstPointer, ptr);
        return rb_str_new2(stringify((*ptr)->text()).c_str());
    }

    /*
     * call-seq:
     *     slot -> String or Nil
     *
     * Fetch the slot name.
     */
    VALUE
    package_dep_atom_slot_ptr(VALUE self)
    {
        PackageDepAtom::ConstPointer * ptr;
        Data_Get_Struct(self, PackageDepAtom::ConstPointer, ptr);
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
    package_dep_atom_repository_ptr(VALUE self)
    {
        PackageDepAtom::ConstPointer * ptr;
        Data_Get_Struct(self, PackageDepAtom::ConstPointer, ptr);
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
    package_dep_atom_version_requirements_ptr(VALUE self) {
        PackageDepAtom::ConstPointer * ptr;
        Data_Get_Struct(self, PackageDepAtom::ConstPointer, ptr);
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

    void do_register_dep_atom()
    {
        /*
         * Document-class: Paludis::DepAtom
         *
         * Base class for a dependency atom.
         */
        c_dep_atom = rb_define_class_under(paludis_module(), "DepAtom", rb_cObject);
        rb_funcall(c_dep_atom, rb_intern("private_class_method"), 1, rb_str_new2("new"));

        /*
         * Document-class: Paludis::CompositeDepAtom
         *
         * Class for dependency atoms that have a number of child dependency atoms.
         */
        c_composite_dep_atom = rb_define_class_under(paludis_module(), "CompositeDepAtom", c_dep_atom);
        rb_funcall(c_composite_dep_atom, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_method(c_composite_dep_atom, "each", RUBY_FUNC_CAST(&composite_dep_atom_each), 0);
        rb_include_module(c_composite_dep_atom, rb_mEnumerable);

        /*
         * Document-class: Paludis::AllDepAtom
         *
         * Represents a ( first second third ) or top level group of dependency atoms.
         */
        c_all_dep_atom = rb_define_class_under(paludis_module(), "AllDepAtom", c_composite_dep_atom);
        rb_funcall(c_all_dep_atom, rb_intern("private_class_method"), 1, rb_str_new2("new"));

        /*
         * Document-class: Paludis::AnyDepAtom
         *
         * Represents a "|| ( )" dependency block.
         */
        c_any_dep_atom = rb_define_class_under(paludis_module(), "AnyDepAtom", c_composite_dep_atom);
        rb_funcall(c_any_dep_atom, rb_intern("private_class_method"), 1, rb_str_new2("new"));

        /*
         * Document-class: Paludis::UseDepAtom
         *
         * Represents a use? ( ) dependency atom.
         */
        c_use_dep_atom = rb_define_class_under(paludis_module(), "UseDepAtom", c_composite_dep_atom);
        rb_funcall(c_use_dep_atom, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_method(c_use_dep_atom, "flag", RUBY_FUNC_CAST(&use_dep_atom_flag), 0);
        rb_define_method(c_use_dep_atom, "inverse?", RUBY_FUNC_CAST(&use_dep_atom_inverse), 0);

        /*
         * Document-class: Paludis::StringDepAtom
         *
         * A StringDepAtom represents a non-composite dep atom with an associated piece of text.
         */
        c_string_dep_atom = rb_define_class_under(paludis_module(), "StringDepAtom", c_dep_atom);
        rb_funcall(c_string_dep_atom, rb_intern("private_class_method"), 1, rb_str_new2("new"));

        /*
         * Document-class: Paludis::PackageDepAtom
         *
         * A PackageDepAtom represents a package name (for example, 'app-editors/vim'),
         * possibly with associated version and SLOT restrictions.
         */
        c_package_dep_atom = rb_define_class_under(paludis_module(), "PackageDepAtom", c_string_dep_atom);
        rb_define_singleton_method(c_package_dep_atom, "new", RUBY_FUNC_CAST(&DepAtomThings<PackageDepAtom>::dep_atom_new_1), 1);
        rb_define_method(c_package_dep_atom, "initialize", RUBY_FUNC_CAST(&dep_atom_init_1), 1);
        rb_define_method(c_package_dep_atom, "to_s", RUBY_FUNC_CAST(&Common<PackageDepAtom::ConstPointer>::to_s_via_ptr), 0);
        rb_define_method(c_package_dep_atom, "package", RUBY_FUNC_CAST(&package_dep_atom_package), 0);
        rb_define_method(c_package_dep_atom, "text", RUBY_FUNC_CAST(&package_dep_atom_text), 0);
        rb_define_method(c_package_dep_atom, "slot", RUBY_FUNC_CAST(&package_dep_atom_slot_ptr), 0);
        rb_define_method(c_package_dep_atom, "repository", RUBY_FUNC_CAST(&package_dep_atom_repository_ptr), 0);
        rb_define_method(c_package_dep_atom, "version_requirements", RUBY_FUNC_CAST(&package_dep_atom_version_requirements_ptr), 0);

        /*
         * Document-class: Paludis::PlainTextDepAtom
         *
         * A PlainTextDepAtom represents a plain text entry (for example, a URI in SRC_URI).
         */
        c_plain_text_dep_atom = rb_define_class_under(paludis_module(), "PlainTextDepAtom", c_string_dep_atom);
        rb_define_singleton_method(c_plain_text_dep_atom, "new", RUBY_FUNC_CAST(&DepAtomThings<PlainTextDepAtom>::dep_atom_new_1), 1);
        rb_define_method(c_plain_text_dep_atom, "initialize", RUBY_FUNC_CAST(&dep_atom_init_1), 1);
        rb_define_method(c_plain_text_dep_atom, "to_s", RUBY_FUNC_CAST(&Common<PlainTextDepAtom::ConstPointer>::to_s_via_ptr), 0);

        /*
         * Document-class: Paludis::BlockDepAtom
         *
         * A BlockDepAtom represents a block on a package name (for example, 'app-editors/vim'), possibly with
         * associated version and SLOT restrictions.
         */
        c_block_dep_atom = rb_define_class_under(paludis_module(), "BlockDepAtom", c_string_dep_atom);
        rb_define_singleton_method(c_block_dep_atom, "new", RUBY_FUNC_CAST(&block_dep_atom_new), 1);
        rb_define_method(c_block_dep_atom, "initialize", RUBY_FUNC_CAST(&dep_atom_init_1), 1);
        rb_define_method(c_block_dep_atom, "blocked_atom", RUBY_FUNC_CAST(&block_dep_atom_blocked_atom), 0);
    }
}

PackageDepAtom::ConstPointer
paludis::ruby::value_to_package_dep_atom(VALUE v)
{
    if (T_STRING == TYPE(v))
        return PackageDepAtom::ConstPointer(new PackageDepAtom(StringValuePtr(v)));
    else if (rb_obj_is_kind_of(v, c_package_dep_atom))
    {
        PackageDepAtom::ConstPointer * v_ptr;
        Data_Get_Struct(v, PackageDepAtom::ConstPointer, v_ptr);
        return *v_ptr;
    }
    else
    {
        rb_raise(rb_eTypeError, "Can't convert %s into PackageDepAtom", rb_obj_classname(v));
    }
}

VALUE
paludis::ruby::dep_atom_to_value(DepAtom::ConstPointer m)
{
    struct V :
        DepAtomVisitorTypes::ConstVisitor
    {
        VALUE value;
        DepAtom::ConstPointer mm;

        V(DepAtom::ConstPointer _m) :
            mm(_m)
        {
        }

        void visit(const AllDepAtom *)
        {
            value = Data_Wrap_Struct(c_all_dep_atom, 0, &Common<AllDepAtom::ConstPointer>::free,
                    new AllDepAtom::ConstPointer(mm));
        }

        void visit(const AnyDepAtom *)
        {
            value = Data_Wrap_Struct(c_any_dep_atom, 0, &Common<AnyDepAtom::ConstPointer>::free,
                    new AnyDepAtom::ConstPointer(mm));
        }

        void visit(const UseDepAtom *)
        {
            value = Data_Wrap_Struct(c_use_dep_atom, 0, &Common<UseDepAtom::ConstPointer>::free,
                    new UseDepAtom::ConstPointer(mm));
        }

        void visit(const PlainTextDepAtom *)
        {
            value = Data_Wrap_Struct(c_plain_text_dep_atom, 0, &Common<PlainTextDepAtom::ConstPointer>::free,
                    new PlainTextDepAtom::ConstPointer(mm));
        }

        void visit(const PackageDepAtom *)
        {
            value = Data_Wrap_Struct(c_package_dep_atom, 0, &Common<PackageDepAtom::ConstPointer>::free,
                    new PackageDepAtom::ConstPointer(mm));
        }

        void visit(const BlockDepAtom *)
        {
            value = Data_Wrap_Struct(c_block_dep_atom, 0, &Common<BlockDepAtom::ConstPointer>::free,
                    new BlockDepAtom::ConstPointer(mm));
        }
    };

    if (0 == m)
        return Qnil;

    DepAtom::ConstPointer * m_ptr(0);
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

RegisterRubyClass::Register paludis_ruby_register_dep_atom PALUDIS_ATTRIBUTE((used))
    (&do_register_dep_atom);

