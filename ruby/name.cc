/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include <paludis_ruby.hh>
#include <paludis/name.hh>
#include <paludis/util/compare.hh>
#include <ruby.h>

using namespace paludis;
using namespace paludis::ruby;

#define RUBY_FUNC_CAST(x) reinterpret_cast<VALUE (*)(...)>(x)

namespace
{
    template <typename T_, typename E_>
    struct NameWrapper
    {
        static VALUE c_class;
        static VALUE c_class_except;

        static void
        name_part_error_free(void * ptr)
        {
            delete static_cast<std::string *>(ptr);
        }

        static VALUE
        name_part_error_new(VALUE the_class, VALUE value)
        {
            VALUE argv[1];
            std::string * ptr(new std::string(STR2CSTR(value)));
            VALUE tdata(Data_Wrap_Struct(the_class, 0, &name_part_error_free, ptr));
            argv[0] = value;
            rb_obj_call_init(tdata, 1, argv);
            rb_call_super(1, &value);
            return tdata;
        }

        static VALUE
        name_part_error_init(VALUE self, VALUE)
        {
            return self;
        }

        static void
        name_part_free(void * ptr)
        {
            delete static_cast<T_ *>(ptr);
        }

        static VALUE
        name_part_new(VALUE the_class, VALUE value)
        {
            VALUE argv[1];
            T_ * ptr(0);
            try
            {
                ptr = new T_(std::string(STR2CSTR(value)));
                VALUE tdata(Data_Wrap_Struct(the_class, 0, &name_part_free, ptr));
                argv[0] = value;
                rb_obj_call_init(tdata, 1, argv);
                return tdata;
            }
            catch (const NameError & e)
            {
                delete ptr;
                rb_raise(c_class_except, e.message().c_str());
            }
            catch (const std::exception & e)
            {
                delete ptr;
                exception_to_ruby_exception(e);
            }
        }

        static VALUE
        name_part_compare(VALUE left, VALUE right)
        {
            T_ * left_ptr, * right_ptr;
            Data_Get_Struct(left, T_, left_ptr);
            Data_Get_Struct(right, T_, right_ptr);

            return INT2FIX(compare(*left_ptr, *right_ptr));
        }

        static VALUE
        name_part_to_s(VALUE left)
        {
            T_ * left_ptr;
            Data_Get_Struct(left, T_, left_ptr);

            return rb_str_new2(left_ptr->data().c_str());
        }

        static VALUE
        name_part_init(VALUE self, VALUE)
        {
            return self;
        }

        static void do_register(const std::string & name)
        {
            c_class = rb_define_class(name.c_str(), rb_cObject);
            rb_define_singleton_method(c_class, "new", RUBY_FUNC_CAST(&name_part_new), 1);
            rb_define_method(c_class, "initialize", RUBY_FUNC_CAST(&name_part_init), 1);
            rb_define_method(c_class, "<=>", RUBY_FUNC_CAST(&name_part_compare), 1);
            rb_include_module(c_class, rb_mComparable);
            rb_define_method(c_class, "to_s", RUBY_FUNC_CAST(&name_part_to_s), 0);

            c_class_except = rb_define_class((name + "Error").c_str(), rb_eRuntimeError);
            rb_define_singleton_method(c_class_except, "new", RUBY_FUNC_CAST(&name_part_error_new), 1);
            rb_define_method(c_class_except, "initialize", RUBY_FUNC_CAST(&name_part_error_init), 1);
        }
    };

    template <typename T_, typename E_> VALUE NameWrapper<T_, E_>::c_class;
    template <typename T_, typename E_> VALUE NameWrapper<T_, E_>::c_class_except;

    void do_register_names()
    {
        NameWrapper<PackageNamePart, PackageNamePartError>::do_register("PackageNamePart");
        NameWrapper<CategoryNamePart, CategoryNamePartError>::do_register("CategoryNamePart");
        NameWrapper<UseFlagName, UseFlagNameError>::do_register("UseFlagName");
        NameWrapper<SlotName, SlotNameError>::do_register("SlotName");
        NameWrapper<RepositoryName, RepositoryNameError>::do_register("RepositoryName");
        NameWrapper<KeywordName, KeywordNameError>::do_register("KeywordName");
    }
}

RegisterRubyClass::Register paludis_ruby_register_name PALUDIS_ATTRIBUTE((used)) (&do_register_names);

