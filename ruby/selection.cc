/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008 Ciaran McCreesh
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

using namespace paludis;
using namespace paludis::ruby;

namespace
{
    static VALUE c_selection_module;
    static VALUE c_selection;
    static VALUE c_selection_some_arbitrary_version;
    static VALUE c_selection_best_version_only;
    static VALUE c_selection_best_version_in_each_slot;
    static VALUE c_selection_all_versions_sorted;
    static VALUE c_selection_all_versions_grouped_by_slot;
    static VALUE c_selection_all_versions_unsorted;
    static VALUE c_selection_require_exactly_one;

    VALUE
    selection_init(int, VALUE *, VALUE self)
    {
        return self;
    }

    template <typename T_>
    struct SelectionNew
    {
        static VALUE
        selection_new(VALUE self, VALUE fg_v)
        {
            Selection * ptr(0);
            try
            {
                FilteredGenerator fg(value_to_filtered_generator(fg_v));
                ptr = new T_(fg);
                VALUE data(Data_Wrap_Struct(self, 0, &Common<Selection>::free, ptr));
                rb_obj_call_init(data, 0, &self);
                return data;
            }
            catch (const std::exception & e)
            {
                delete ptr;
                exception_to_ruby_exception(e);
            }
        }
    };

    /*
     *  call-seq:
     *      to_s -> String
     *
     * Return as String
     */
    FAKE_RDOC_METHOD(selection_to_s)

    /*
     * call-seq:
     *     new(FilteredGenerator) -> SomeArbitraryVersion
     *
     * Create new SomeArbitrartyVersion Selection
     */
    FAKE_RDOC_METHOD(some_arbitrary_version_new)

    /*
     * call-seq:
     *     new(FilteredGenerator) -> BestVersionOnly
     *
     * Create new BestVersionOnly Selection
     */
    FAKE_RDOC_METHOD(best_version_only_new)

    /*
     * call-seq:
     *     new(FilteredGenerator) -> BestVersionInEachSlot
     *
     * Create new BestVersionOnly Selection
     */
    FAKE_RDOC_METHOD(best_version_in_each_slot_new)

    /*
     * call-seq:
     *     new(FilteredGenerator) -> AllVersionsSorted
     *
     * Create new AllVersionsSorted Selection
     */
    FAKE_RDOC_METHOD(all_versions_sorted_new)

    /*
     * call-seq:
     *     new(FilteredGenerator) -> AllVersionsGroupedBySlot
     *
     * Create new AllVersionsGroupedBySlot Selection
     */
    FAKE_RDOC_METHOD(all_versions_sorted_new)

    /*
     * call-seq:
     *     new(FilteredGenerator) -> AllVersionsUnsorted
     *
     * Create new AllVersionsUnsorted Selection
     */
    FAKE_RDOC_METHOD(all_versions_sorted_new)

    /*
     * call-seq:
     *     new(FilteredGenerator) -> RequireExactlyOne
     *
     * Create new RequireExactlyOne Selection
     */
    FAKE_RDOC_METHOD(all_versions_sorted_new)

    void do_register_selection()
    {
        /*
         * Document-module: Paludis::Selection
         *
         * Collection of classes to sort and select the results for an
         * Environment selection.
         */
        c_selection_module = rb_define_module_under(paludis_module(), "Selection");

        /*
         * Document-class: Paludis::Selection::Selection
         *
         * Selection for an Environment selection.
         */
        c_selection = rb_define_class_under(c_selection_module, "Selection", rb_cObject);
        rb_funcall(c_selection, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_method(c_selection, "initialize", RUBY_FUNC_CAST(&selection_init), -1);
        rb_define_method(c_selection, "to_s", RDOC_IS_STUPID(selection_to_s,&Common<Selection>::to_s), 0);

        /*
         * Document-class: Paludis::Selection::SomeArbitraryVersion
         *
         * Select some arbitrary version of some arbitrary package.
         */
        c_selection_some_arbitrary_version = rb_define_class_under(c_selection_module, "SomeArbitraryVersion", c_selection);
        rb_define_singleton_method(c_selection_some_arbitrary_version, "new",
                RDOC_IS_STUPID(some_arbitrary_version_new, &SelectionNew<selection::SomeArbitraryVersion>::selection_new), 1);

        /*
         * Document-class: Paludis::Selection::BestVersionOnly
         *
         * Select the best version only of each package.
         */
        c_selection_best_version_only = rb_define_class_under(c_selection_module, "BestVersionOnly", c_selection);
        rb_define_singleton_method(c_selection_best_version_only, "new",
                RDOC_IS_STUPID(best_version_only_new, &SelectionNew<selection::BestVersionOnly>::selection_new), 1);

        /*
         * Document-class: Paludis::Selection::BestVersionInEachSlot
         *
         * Select the best version in each slot of each package.
         */
        c_selection_best_version_in_each_slot = rb_define_class_under(c_selection_module, "BestVersionInEachSlot", c_selection);
        rb_define_singleton_method(c_selection_best_version_in_each_slot, "new",
                RDOC_IS_STUPID(best_version_in_each_slot_new, &SelectionNew<selection::BestVersionInEachSlot>::selection_new), 1);

        /*
         * Document-class: Paludis::Selection::AllVersionsSorted
         *
         * Select all versions, sorted.
         */
        c_selection_all_versions_sorted = rb_define_class_under(c_selection_module, "AllVersionsSorted", c_selection);
        rb_define_singleton_method(c_selection_all_versions_sorted, "new",
                RDOC_IS_STUPID(all_versions_sorted_new, &SelectionNew<selection::AllVersionsSorted>::selection_new), 1);

        /*
         * Document-class: Paludis::Selection::AllVersionsGroupedBySlot
         *
         * Select all versions, sorted and grouped by slot.
         */
        c_selection_all_versions_grouped_by_slot = rb_define_class_under(c_selection_module, "AllVersionsGroupedBySlot", c_selection);
        rb_define_singleton_method(c_selection_all_versions_grouped_by_slot, "new",
                RDOC_IS_STUPID(all_versions_grouped_by_slot, &SelectionNew<selection::AllVersionsGroupedBySlot>::selection_new), 1);

        /*
         * Document-class: Paludis::Selection::AllVersionsUnsorted
         *
         * Select all versions, in no particular order.
         */
        c_selection_all_versions_unsorted = rb_define_class_under(c_selection_module, "AllVersionsUnsorted", c_selection);
        rb_define_singleton_method(c_selection_all_versions_unsorted, "new",
                RDOC_IS_STUPID(all_versions_unsorted_new, &SelectionNew<selection::AllVersionsUnsorted>::selection_new), 1);

        /*
         * Document-class: Paludis::Selection::RequireExactlyOne
         *
         * Require exactly one matching ID.
         */
        c_selection_require_exactly_one = rb_define_class_under(c_selection_module, "RequireExactlyOne", c_selection);
        rb_define_singleton_method(c_selection_require_exactly_one, "new",
                RDOC_IS_STUPID(require_exactly_one_new, &SelectionNew<selection::RequireExactlyOne>::selection_new), 1);
    }
}

Selection
paludis::ruby::value_to_selection(VALUE v)
{
    if (rb_obj_is_kind_of(v, c_selection))
    {
        Selection * f_ptr;
        Data_Get_Struct(v, Selection, f_ptr);
        return *f_ptr;
    }
    else
        rb_raise(rb_eTypeError, "Can't convert %s into Selection", rb_obj_classname(v));
}

RegisterRubyClass::Register paludis_ruby_register_selection PALUDIS_ATTRIBUTE((used))
    (&do_register_selection);

