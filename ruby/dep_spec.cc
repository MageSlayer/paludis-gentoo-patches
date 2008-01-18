/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh
 * Copyright (c) 2006, 2007 Richard Brown
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
#include <paludis/version_requirements.hh>
#include <paludis/version_operator.hh>
#include <paludis/use_requirements.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/options.hh>
#include <paludis/util/save.hh>
#include <paludis/util/member_iterator-impl.hh>
#include <list>
#include <ruby.h>

#include "nice_names-nn.hh"

using namespace paludis;
using namespace paludis::ruby;

#define RUBY_FUNC_CAST(x) reinterpret_cast<VALUE (*)(...)>(x)

namespace
{
    static VALUE c_dep_spec;
    static VALUE c_string_dep_spec;

    static VALUE c_block_dep_spec;
    static VALUE c_dependency_labels_dep_spec;
    static VALUE c_fetchable_uri_dep_spec;
    static VALUE c_license_dep_spec;
    static VALUE c_named_set_dep_spec;
    static VALUE c_package_dep_spec;
    static VALUE c_plain_text_dep_spec;
    static VALUE c_simple_uri_dep_spec;
    static VALUE c_uri_labels_dep_spec;

    static VALUE c_all_dep_spec;
    static VALUE c_any_dep_spec;
    static VALUE c_use_dep_spec;

    static VALUE c_version_requirements_mode;

    static VALUE c_uri_label;
    static VALUE c_uri_mirrors_then_listed_label;
    static VALUE c_uri_mirrors_only_label;
    static VALUE c_uri_listed_only_label;
    static VALUE c_uri_listed_then_mirrors_label;
    static VALUE c_uri_local_mirrors_only_label;
    static VALUE c_uri_manual_only_label;

    struct WrappedSpecBase;
    template <typename> struct WrappedSpec;

    struct WrappedSpecVisitorTypes :
        VisitorTypes<
            WrappedSpecVisitorTypes,
            WrappedSpecBase,
            WrappedSpec<PlainTextDepSpec>,
            WrappedSpec<SimpleURIDepSpec>,
            WrappedSpec<FetchableURIDepSpec>,
            WrappedSpec<LicenseDepSpec>,
            WrappedSpec<PackageDepSpec>,
            WrappedSpec<BlockDepSpec>,
            WrappedSpec<URILabelsDepSpec>,
            WrappedSpec<DependencyLabelsDepSpec>,
            WrappedSpec<NamedSetDepSpec>,
            WrappedSpec<AllDepSpec>,
            WrappedSpec<AnyDepSpec>,
            WrappedSpec<UseDepSpec>
        >
    {
    };

    struct WrappedSpecBase :
        virtual ConstAcceptInterface<WrappedSpecVisitorTypes>
    {
        typedef std::list<std::pair<VALUE, tr1::shared_ptr<const WrappedSpecBase> > > Children;

        virtual ~WrappedSpecBase()
        {
        }

        virtual const tr1::shared_ptr<const DepSpec> base_spec() const = 0;
        virtual const tr1::shared_ptr<const Children> children() const = 0;

        static void mark(tr1::shared_ptr<const WrappedSpecBase> *);
    };

    void
    WrappedSpecBase::mark(tr1::shared_ptr<const WrappedSpecBase> * ptr)
    {
        tr1::shared_ptr<const Children> children((*ptr)->children());
        for (Children::const_iterator it(children->begin()),
                 it_end(children->end()); it_end != it; ++it)
            rb_gc_mark(it->first);
    }

    template <typename T_>
    class WrappedSpec :
        public WrappedSpecBase,
        public ConstAcceptInterfaceVisitsThis<WrappedSpecVisitorTypes, WrappedSpec<T_> >
    {
        private:
            tr1::shared_ptr<T_> _spec;
            tr1::shared_ptr<Children> _children;

        public:
            WrappedSpec(const tr1::shared_ptr<T_> & s) :
                _spec(s),
                _children(new Children)
            {
            }

            virtual const tr1::shared_ptr<const DepSpec> base_spec() const
            {
                return _spec;
            }

            const tr1::shared_ptr<const T_> spec() const
            {
                return _spec;
            }

            const tr1::shared_ptr<T_> spec()
            {
                return _spec;
            }

            template <typename Iter_>
            WrappedSpec * add_children(Iter_ cur, const Iter_ end);

            virtual const tr1::shared_ptr<const Children> children() const
            {
                return _children;
            }
    };

    struct TreeToValue :
        ConstVisitor<GenericSpecTree>
    {
        tr1::shared_ptr<const WrappedSpecBase> * ptr;
        VALUE value;

        void visit_leaf(const PackageDepSpec & spec)
        {
            ptr = new tr1::shared_ptr<const WrappedSpecBase>(
                    new WrappedSpec<PackageDepSpec>(tr1::static_pointer_cast<PackageDepSpec>(spec.clone())));
            value = Data_Wrap_Struct(c_package_dep_spec, 0, &Common<tr1::shared_ptr<const WrappedSpecBase> >::free, ptr);
        }

        void visit_leaf(const BlockDepSpec & spec)
        {
            ptr = new tr1::shared_ptr<const WrappedSpecBase>(
                    new WrappedSpec<BlockDepSpec>(tr1::static_pointer_cast<BlockDepSpec>(spec.clone())));
            value = Data_Wrap_Struct(c_block_dep_spec, 0, &Common<tr1::shared_ptr<const WrappedSpecBase> >::free, ptr);
        }

        void visit_leaf(const PlainTextDepSpec & spec)
        {
            ptr = new tr1::shared_ptr<const WrappedSpecBase>(
                    new WrappedSpec<PlainTextDepSpec>(tr1::static_pointer_cast<PlainTextDepSpec>(spec.clone())));
            value = Data_Wrap_Struct(c_plain_text_dep_spec, 0, &Common<tr1::shared_ptr<const WrappedSpecBase> >::free, ptr);
        }

        void visit_leaf(const SimpleURIDepSpec & spec)
        {
            ptr = new tr1::shared_ptr<const WrappedSpecBase>(
                    new WrappedSpec<SimpleURIDepSpec>(tr1::static_pointer_cast<SimpleURIDepSpec>(spec.clone())));
            value = Data_Wrap_Struct(c_simple_uri_dep_spec, 0, &Common<tr1::shared_ptr<const WrappedSpecBase> >::free, ptr);
        }

        void visit_leaf(const FetchableURIDepSpec & spec)
        {
            ptr = new tr1::shared_ptr<const WrappedSpecBase>(
                    new WrappedSpec<FetchableURIDepSpec>(tr1::static_pointer_cast<FetchableURIDepSpec>(spec.clone())));
            value = Data_Wrap_Struct(c_fetchable_uri_dep_spec, 0, &Common<tr1::shared_ptr<const WrappedSpecBase> >::free, ptr);
        }

        void visit_leaf(const URILabelsDepSpec & spec)
        {
            ptr = new tr1::shared_ptr<const WrappedSpecBase>(
                    new WrappedSpec<URILabelsDepSpec>(tr1::static_pointer_cast<URILabelsDepSpec>(spec.clone())));
            value = Data_Wrap_Struct(c_uri_labels_dep_spec, 0, &Common<tr1::shared_ptr<const WrappedSpecBase> >::free, ptr);
        }

        void visit_leaf(const DependencyLabelsDepSpec & spec)
        {
            ptr = new tr1::shared_ptr<const WrappedSpecBase>(
                    new WrappedSpec<DependencyLabelsDepSpec>(tr1::static_pointer_cast<DependencyLabelsDepSpec>(spec.clone())));
            value = Data_Wrap_Struct(c_dependency_labels_dep_spec, 0, &Common<tr1::shared_ptr<const WrappedSpecBase> >::free, ptr);
        }

        void visit_leaf(const NamedSetDepSpec & spec)
        {
            ptr = new tr1::shared_ptr<const WrappedSpecBase>(
                    new WrappedSpec<NamedSetDepSpec>(tr1::static_pointer_cast<NamedSetDepSpec>(spec.clone())));
            value = Data_Wrap_Struct(c_named_set_dep_spec, 0, &Common<tr1::shared_ptr<const WrappedSpecBase> >::free, ptr);
        }

        void visit_leaf(const LicenseDepSpec & spec)
        {
            ptr = new tr1::shared_ptr<const WrappedSpecBase>(
                    new WrappedSpec<LicenseDepSpec>(tr1::static_pointer_cast<LicenseDepSpec>(spec.clone())));
            value = Data_Wrap_Struct(c_license_dep_spec, 0, &Common<tr1::shared_ptr<const WrappedSpecBase> >::free, ptr);
        }

        void visit_sequence(const UseDepSpec & spec,
                GenericSpecTree::ConstSequenceIterator cur,
                GenericSpecTree::ConstSequenceIterator end)
        {
            ptr = new tr1::shared_ptr<const WrappedSpecBase>(
                    (new WrappedSpec<UseDepSpec>(tr1::static_pointer_cast<UseDepSpec>(spec.clone())))->add_children(cur, end));
            value = Data_Wrap_Struct(c_use_dep_spec, &WrappedSpecBase::mark, &Common<tr1::shared_ptr<const WrappedSpecBase> >::free, ptr);
        }

        void visit_sequence(const AllDepSpec & spec,
                GenericSpecTree::ConstSequenceIterator cur,
                GenericSpecTree::ConstSequenceIterator end)
        {
            ptr = new tr1::shared_ptr<const WrappedSpecBase>(
                    (new WrappedSpec<AllDepSpec>(tr1::static_pointer_cast<AllDepSpec>(spec.clone())))->add_children(cur, end));
            value = Data_Wrap_Struct(c_all_dep_spec, &WrappedSpecBase::mark, &Common<tr1::shared_ptr<const WrappedSpecBase> >::free, ptr);
        }

        void visit_sequence(const AnyDepSpec & spec,
                GenericSpecTree::ConstSequenceIterator cur,
                GenericSpecTree::ConstSequenceIterator end)
        {
            ptr = new tr1::shared_ptr<const WrappedSpecBase>(
                    (new WrappedSpec<AnyDepSpec>(tr1::static_pointer_cast<AnyDepSpec>(spec.clone())))->add_children(cur, end));
            value = Data_Wrap_Struct(c_any_dep_spec, &WrappedSpecBase::mark, &Common<tr1::shared_ptr<const WrappedSpecBase> >::free, ptr);
        }
    };

    template <typename T_>
    template <typename Iter_>
    WrappedSpec<T_> *
    WrappedSpec<T_>::add_children(Iter_ cur, const Iter_ end)
    {
        for ( ; cur != end ; ++cur)
        {
            TreeToValue v;
            cur->accept(v);
            _children->push_back(std::make_pair(v.value, *v.ptr));
        }

        return this;
    }

    template <typename H_>
    struct ValueToTree :
        ConstVisitor<WrappedSpecVisitorTypes>
    {
        tr1::shared_ptr<typename H_::ConstItem> result;
        tr1::function<void (const tr1::shared_ptr<ConstAcceptInterface<H_> > &)> adder;

        ValueToTree(VALUE val) :
            adder(tr1::bind(&ValueToTree<H_>::set_result, this, tr1::placeholders::_1))
        {
            tr1::shared_ptr<WrappedSpecBase> * p;
            Data_Get_Struct(val, tr1::shared_ptr<WrappedSpecBase>, p);
            (*p)->accept(*this);
        }

        void set_result(const tr1::shared_ptr<ConstAcceptInterface<H_> > & res)
        {
            result = res;
        }

        template <typename T_>
        void do_visit_sequence(const WrappedSpec<T_> & item, tr1::true_type)
        {
            using namespace tr1::placeholders;

            tr1::shared_ptr<ConstTreeSequence<H_, T_> > a(
                new ConstTreeSequence<H_, T_>(
                    tr1::static_pointer_cast<T_>(item.spec()->clone())));
            adder(a);

            Save<tr1::function<void (const tr1::shared_ptr<ConstAcceptInterface<H_> > &)> > s(
                &adder, tr1::bind(&ConstTreeSequence<H_, T_>::add, a, _1));
            std::for_each(indirect_iterator(second_iterator(item.children()->begin())),
                          indirect_iterator(second_iterator(item.children()->end())),
                          accept_visitor(*this));
        }

        template <typename T_>
        void do_visit_sequence(const WrappedSpec<T_> &, tr1::false_type)
        {
            rb_raise(rb_eTypeError, "Item of type %s is not allowed in hierarchy of type %s", NiceNames<T_>::name, NiceNames<H_>::name);
        }

        template <typename T_>
        void do_visit_sequence(const WrappedSpec<T_> & s)
        {
            do_visit_sequence(s, tr1::is_convertible<ConstVisitor<H_> *, Visits<const ConstTreeSequence<H_, T_> > *>());
        }

        virtual void visit(const WrappedSpec<AllDepSpec> & s)
        {
            do_visit_sequence(s);
        }

        virtual void visit(const WrappedSpec<AnyDepSpec> & s)
        {
            do_visit_sequence(s);
        }

        virtual void visit(const WrappedSpec<UseDepSpec> & s)
        {
            do_visit_sequence(s);
        }

        template <typename T_>
        void do_visit_leaf(const WrappedSpec<T_> & item, tr1::true_type)
        {
            tr1::shared_ptr<TreeLeaf<H_, T_> > a(
                new TreeLeaf<H_, T_>(
                    tr1::static_pointer_cast<T_>(item.spec()->clone())));
            adder(a);
        }

        template <typename T_>
        void do_visit_leaf(const WrappedSpec<T_> &, tr1::false_type)
        {
            rb_raise(rb_eTypeError, "Item of type %s is not allowed in hierarchy of type %s", NiceNames<T_>::name, NiceNames<H_>::name);
        }

        template <typename T_>
        void do_visit_leaf(const WrappedSpec<T_> & s)
        {
            do_visit_leaf(s, tr1::is_convertible<ConstVisitor<H_> *, Visits<const TreeLeaf<H_, T_> > *>());
        }

        virtual void visit(const WrappedSpec<PlainTextDepSpec> & s)
        {
            do_visit_leaf(s);
        }

        virtual void visit(const WrappedSpec<SimpleURIDepSpec> & s)
        {
            do_visit_leaf(s);
        }

        virtual void visit(const WrappedSpec<FetchableURIDepSpec> & s)
        {
            do_visit_leaf(s);
        }

        virtual void visit(const WrappedSpec<LicenseDepSpec> & s)
        {
            do_visit_leaf(s);
        }

        virtual void visit(const WrappedSpec<PackageDepSpec> & s)
        {
            do_visit_leaf(s);
        }

        virtual void visit(const WrappedSpec<BlockDepSpec> & s)
        {
            do_visit_leaf(s);
        }

        virtual void visit(const WrappedSpec<URILabelsDepSpec> & s)
        {
            do_visit_leaf(s);
        }

        virtual void visit(const WrappedSpec<DependencyLabelsDepSpec> & s)
        {
            do_visit_leaf(s);
        }

        virtual void visit(const WrappedSpec<NamedSetDepSpec> & s)
        {
            do_visit_leaf(s);
        }
    };

    VALUE
    dep_spec_init_0(VALUE self)
    {
        return self;
    }

    VALUE
    dep_spec_init_1(VALUE self, VALUE)
    {
        return self;
    }

    VALUE
    package_dep_spec_init(int, VALUE *, VALUE self)
    {
        return self;
    }

    VALUE
    uri_labels_dep_spec_labels(VALUE self)
    {
        tr1::shared_ptr<WrappedSpecBase> * ptr;
        Data_Get_Struct(self, tr1::shared_ptr<WrappedSpecBase>, ptr);
        tr1::shared_ptr<const URILabelsDepSpec> real_ptr(tr1::static_pointer_cast<const WrappedSpec<URILabelsDepSpec> >(*ptr)->spec());

        if (rb_block_given_p())
        {
            for (URILabelsDepSpec::ConstIterator it(real_ptr->begin()),
                     it_end(real_ptr->end()); it_end != it; ++it)
                rb_yield(uri_label_to_value(*it));

            return Qnil;
        }
        else
        {
            VALUE result(rb_ary_new());

            for (URILabelsDepSpec::ConstIterator it(real_ptr->begin()),
                     it_end(real_ptr->end()); it_end != it; ++it)
                rb_ary_push(result, uri_label_to_value(*it));

            return result;
        }
    }

    VALUE
    block_dep_spec_new(VALUE self, VALUE spec)
    {
        tr1::shared_ptr<const WrappedSpecBase> * ptr(0);
        try
        {
            tr1::shared_ptr<const PackageDepSpec> pkg(value_to_package_dep_spec(spec));
            ptr = new tr1::shared_ptr<const WrappedSpecBase>(new WrappedSpec<BlockDepSpec>(make_shared_ptr(new BlockDepSpec(pkg))));
            VALUE tdata(Data_Wrap_Struct(self, 0, &Common<tr1::shared_ptr<const WrappedSpecBase> >::free, ptr));
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
        tr1::shared_ptr<WrappedSpecBase> * p;
        Data_Get_Struct(self, tr1::shared_ptr<WrappedSpecBase>, p);
        return package_dep_spec_to_value(tr1::static_pointer_cast<const WrappedSpec<BlockDepSpec> >(*p)->spec()->blocked_spec());
    }

    template <typename A_>
    struct DepSpecThings
    {
        static VALUE
        dep_spec_new_0(VALUE self, VALUE s)
        {
            tr1::shared_ptr<const WrappedSpecBase> * ptr(0);
            try
            {
                ptr = new tr1::shared_ptr<const WrappedSpecBase>(new WrappedSpec<A_>(make_shared_ptr(new A_)));
                VALUE tdata(Data_Wrap_Struct(self, 0, &Common<tr1::shared_ptr<const WrappedSpecBase> >::free, ptr));
                rb_obj_call_init(tdata, 1, &s);
                return tdata;
            }
            catch (const std::exception & e)
            {
                delete ptr;
                exception_to_ruby_exception(e);
            }
        }

        static VALUE
        dep_spec_new_1(VALUE self, VALUE s)
        {
            tr1::shared_ptr<const WrappedSpecBase> * ptr(0);
            try
            {
                ptr = new tr1::shared_ptr<const WrappedSpecBase>(new WrappedSpec<A_>(make_shared_ptr(new A_(StringValuePtr(s)))));
                VALUE tdata(Data_Wrap_Struct(self, 0, &Common<tr1::shared_ptr<const WrappedSpecBase> >::free, ptr));
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
        tr1::shared_ptr<WrappedSpecBase> * p;
        Data_Get_Struct(self, tr1::shared_ptr<WrappedSpecBase>, p);
        return rb_str_new2(stringify(tr1::static_pointer_cast<const WrappedSpec<UseDepSpec> >(*p)->spec()->flag()).c_str());
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
        tr1::shared_ptr<WrappedSpecBase> * p;
        Data_Get_Struct(self, tr1::shared_ptr<WrappedSpecBase>, p);
        return tr1::static_pointer_cast<const WrappedSpec<UseDepSpec> >(*p)->spec()->inverse() ? Qtrue : Qfalse;
    }

    /*
     * call-seq:
     *     tag -> DepTag or Nil
     *
     * Fetch the dep tag.
     */
    VALUE
    package_dep_spec_tag(VALUE self)
    {
        tr1::shared_ptr<WrappedSpecBase> * ptr;
        Data_Get_Struct(self, tr1::shared_ptr<WrappedSpecBase>, ptr);
        if (0 == tr1::static_pointer_cast<const WrappedSpec<PackageDepSpec> >(*ptr)->spec()->tag())
            return Qnil;
        return dep_tag_to_value(tr1::static_pointer_cast<const WrappedSpec<PackageDepSpec> >(*ptr)->spec()->tag());
    }

    /*
     * call-seq:
     *     tag=(dep_tag) -> Qnil
     *
     * Set the dep tag.
     */
    VALUE
    package_dep_spec_set_tag(VALUE self, VALUE dep_tag)
    {
        tr1::shared_ptr<WrappedSpecBase> * ptr;
        Data_Get_Struct(self, tr1::shared_ptr<WrappedSpecBase>, ptr);
        (tr1::static_pointer_cast<WrappedSpec<PackageDepSpec> >(*ptr))->spec()->set_tag(value_to_dep_tag(dep_tag));
        return Qnil;
    }

    /*
     * call-seq:
     *     without_use_requirements -> PackageDepSpec
     *
     * Fetch us without our use requirements.
     */
    VALUE
    package_dep_spec_without_use_requirements(VALUE self)
    {
        tr1::shared_ptr<WrappedSpecBase> * ptr;
        Data_Get_Struct(self, tr1::shared_ptr<WrappedSpecBase>, ptr);
        return package_dep_spec_to_value(tr1::static_pointer_cast<const WrappedSpec<PackageDepSpec> >(*ptr)->spec()->without_use_requirements());
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
        tr1::shared_ptr<WrappedSpecBase> * ptr;
        Data_Get_Struct(self, tr1::shared_ptr<WrappedSpecBase>, ptr);
        if (0 == tr1::static_pointer_cast<const WrappedSpec<PackageDepSpec> >(*ptr)->spec()->package_ptr())
            return Qnil;
        return rb_str_new2(stringify(*tr1::static_pointer_cast<const WrappedSpec<PackageDepSpec> >(*ptr)->spec()->package_ptr()).c_str());
    }

    /*
     * call-seq:
     *     package_name_part -> String or Nil
     *
     * Fetch the package name part.
     */
    VALUE
    package_dep_spec_package_name_part(VALUE self)
    {
        tr1::shared_ptr<WrappedSpecBase> * ptr;
        Data_Get_Struct(self, tr1::shared_ptr<WrappedSpecBase>, ptr);
        if (0 == tr1::static_pointer_cast<const WrappedSpec<PackageDepSpec> >(*ptr)->spec()->package_name_part_ptr())
            return Qnil;
        return rb_str_new2(stringify(*tr1::static_pointer_cast<const WrappedSpec<PackageDepSpec> >(*ptr)->spec()->
                    package_name_part_ptr()).c_str());
    }

    /*
     * call-seq:
     *     category_name_part -> String or Nil
     *
     * Fetch the category name part.
     */
    VALUE
    package_dep_spec_category_name_part(VALUE self)
    {
        tr1::shared_ptr<WrappedSpecBase> * ptr;
        Data_Get_Struct(self, tr1::shared_ptr<WrappedSpecBase>, ptr);
        if (0 == tr1::static_pointer_cast<const WrappedSpec<PackageDepSpec> >(*ptr)->spec()->category_name_part_ptr())
            return Qnil;
        return rb_str_new2(stringify(*tr1::static_pointer_cast<const WrappedSpec<PackageDepSpec> >(*ptr)->spec()->
                    category_name_part_ptr()).c_str());
    }

    /*
     * call-seq:
     *     text -> String
     *
     * Fetch our text.
     */
    VALUE
    string_dep_spec_text(VALUE self)
    {
        tr1::shared_ptr<WrappedSpecBase> * ptr;
        Data_Get_Struct(self, tr1::shared_ptr<WrappedSpecBase>, ptr);
        return rb_str_new2(stringify(tr1::static_pointer_cast<const StringDepSpec>((*ptr)->base_spec())->text()).c_str());
    }

    /*
     * call-seq:
     *     to_s -> String
     *
     * Fetch a string representation of ourself.
     */
    template <typename T_>
    VALUE dep_spec_to_s(VALUE self)
    {
        tr1::shared_ptr<WrappedSpecBase> * ptr;
        Data_Get_Struct(self, tr1::shared_ptr<WrappedSpecBase>, ptr);
        return rb_str_new2(stringify(*tr1::static_pointer_cast<const WrappedSpec<T_> >(*ptr)->spec()).c_str());
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
        tr1::shared_ptr<WrappedSpecBase> * ptr;
        Data_Get_Struct(self, tr1::shared_ptr<WrappedSpecBase>, ptr);
        if (0 == tr1::static_pointer_cast<const WrappedSpec<PackageDepSpec> >(*ptr)->spec()->slot_ptr())
            return Qnil;
        return rb_str_new2(stringify((*tr1::static_pointer_cast<const WrappedSpec<PackageDepSpec> >(*ptr)->spec()->slot_ptr())).c_str());
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
        tr1::shared_ptr<WrappedSpecBase> * ptr;
        Data_Get_Struct(self, tr1::shared_ptr<WrappedSpecBase>, ptr);
        if (0 == tr1::static_pointer_cast<const WrappedSpec<PackageDepSpec> >(*ptr)->spec()->repository_ptr())
            return Qnil;
        return rb_str_new2(stringify((*tr1::static_pointer_cast<const WrappedSpec<PackageDepSpec> >(*ptr)->spec()->repository_ptr())).c_str());
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
        tr1::shared_ptr<WrappedSpecBase> * ptr;
        Data_Get_Struct(self, tr1::shared_ptr<WrappedSpecBase>, ptr);
        VALUE result(rb_ary_new());
        VALUE result_hash;
        if (tr1::static_pointer_cast<const WrappedSpec<PackageDepSpec> >(*ptr)->spec()->version_requirements_ptr())
            for (VersionRequirements::ConstIterator i(tr1::static_pointer_cast<const PackageDepSpec>((*ptr)->base_spec())->
                        version_requirements_ptr()->begin()),
                    i_end(tr1::static_pointer_cast<const PackageDepSpec>((*ptr)->base_spec())->version_requirements_ptr()->end()) ;
                    i != i_end; ++i)
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

#ifdef CIARANM_REMOVED_THIS
    /*
     * call-seq:
     *     use_requirements -> Array
     *
     * Fetch the use requirements. E.g. [ {:flag => 'a', :state => true } ]
     */
    VALUE
    package_dep_spec_use_requirements(VALUE self)
    {
        tr1::shared_ptr<WrappedSpecBase> * ptr;
        Data_Get_Struct(self, tr1::shared_ptr<WrappedSpecBase>, ptr);
        VALUE result(rb_ary_new());
        VALUE result_hash;
        if (tr1::static_pointer_cast<const WrappedSpec<PackageDepSpec> >(*ptr)->spec()->use_requirements_ptr())
            for (UseRequirements::ConstIterator
                    i(tr1::static_pointer_cast<const WrappedSpec<PackageDepSpec> >(*ptr)->spec()->use_requirements_ptr()->begin()),
                    i_end(tr1::static_pointer_cast<const WrappedSpec<PackageDepSpec> >(*ptr)->spec()->use_requirements_ptr()->end()) ;
                    i != i_end; ++i)
            {
                result_hash = rb_hash_new();
                rb_hash_aset(result_hash, ID2SYM(rb_intern("flag")),
                    rb_str_new2(stringify(i->first).c_str()));
                rb_hash_aset(result_hash, ID2SYM(rb_intern("state")),
                        i->second == use_disabled ? Qfalse : Qtrue);

                rb_ary_push(result, result_hash);
            }
        return result;
    }
#endif

    VALUE
    package_dep_spec_version_requirements_mode(VALUE self)
    {
        tr1::shared_ptr<WrappedSpecBase> * ptr;
        Data_Get_Struct(self, tr1::shared_ptr<WrappedSpecBase>, ptr);
        return INT2FIX(tr1::static_pointer_cast<const WrappedSpec<PackageDepSpec> >(*ptr)->spec()->version_requirements_mode());
    }

    template <std::string (FetchableURIDepSpec::* m_) () const>
    struct FetchableURIDepSpecStringValue
    {
        static VALUE
        fetch(VALUE self)
        {
            tr1::shared_ptr<WrappedSpecBase> * ptr;
            Data_Get_Struct(self, tr1::shared_ptr<WrappedSpecBase>, ptr);
            tr1::shared_ptr<const FetchableURIDepSpec> f_ptr(tr1::static_pointer_cast<const FetchableURIDepSpec>((*ptr)->base_spec()));
            return rb_str_new2(((f_ptr.get())->*(m_))().c_str());
        }
    };

    template <typename T_>
    struct Composite
    {
        /*
         * call-seq:
         *     each {|contents_entry| block}
         *
         * Iterate through our entries.
         */
        static VALUE
        each(VALUE self)
        {
            tr1::shared_ptr<WrappedSpecBase> * ptr;
            Data_Get_Struct(self, tr1::shared_ptr<WrappedSpecBase>, ptr);

            if ((*ptr)->children())
                for (WrappedSpecBase::Children::const_iterator i((*ptr)->children()->begin()), i_end((*ptr)->children()->end()) ;
                        i != i_end ; ++i)
                    rb_yield(i->first);
            return self;
        }
    };

    /*
     * call-seq:
     *     name -> String
     *
     * Fetch our set name.
     */
    VALUE
    named_set_dep_spec_name(VALUE self)
    {
        tr1::shared_ptr<WrappedSpecBase> * ptr;
        Data_Get_Struct(self, tr1::shared_ptr<WrappedSpecBase>, ptr);
        return rb_str_new2(stringify(tr1::static_pointer_cast<const WrappedSpec<NamedSetDepSpec> >(*ptr)->spec()->text()).c_str());
    }

    /*
     * Document-method: parse_user_package_dep_spec
     *
     * call-seq:
     *     parse_user_package_dep_spec(String, Array) -> PackageDepSpec
     *
     * Return a PackageDepSpec parsed from user input. The second parameter is either an empty
     * array, or [ :allow_wildcards ] to allow wildcards.
     *
     */
    VALUE paludis_parse_user_dep_spec(VALUE, VALUE str, VALUE opts)
    {
        tr1::shared_ptr<const WrappedSpecBase> * ptr(0);

        try
        {
            std::string s(StringValuePtr(str));

            Check_Type(opts, T_ARRAY);
            UserPackageDepSpecOptions o;
            for (unsigned i(0) ; i < RARRAY(opts)->len ; ++i)
            {
                VALUE entry(rb_ary_entry(opts, i));
                Check_Type(entry, T_SYMBOL);
                if (SYM2ID(entry) == rb_intern("allow_wildcards"))
                    o += updso_allow_wildcards;
                else
                    rb_raise(rb_eArgError, "Unknown parse_user_package_dep_spec option '%s'", rb_obj_as_string(entry));
            }

            ptr = new tr1::shared_ptr<const WrappedSpecBase>(new WrappedSpec<PackageDepSpec>(
                        make_shared_ptr(new PackageDepSpec(parse_user_package_dep_spec(s, o)))));
            return Data_Wrap_Struct(c_package_dep_spec, 0, &Common<tr1::shared_ptr<const WrappedSpecBase> >::free, ptr);
        }
        catch (const std::exception & e)
        {
            delete ptr;
            exception_to_ruby_exception(e);
        }

    }

    struct URILabelToValue :
        ConstVisitor<URILabelVisitorTypes>
    {
        VALUE value;
        tr1::shared_ptr<const URILabel> mm;

        URILabelToValue(const tr1::shared_ptr<const URILabel> & _m) :
            mm(_m)
        {
        }

        void visit(const URIMirrorsThenListedLabel &)
        {
            value = Data_Wrap_Struct(c_uri_mirrors_then_listed_label, 0, &Common<tr1::shared_ptr<const URILabel> >::free,
                    new tr1::shared_ptr<const URILabel>(mm));
        }

        void visit(const URIMirrorsOnlyLabel &)
        {
            value = Data_Wrap_Struct(c_uri_mirrors_only_label, 0, &Common<tr1::shared_ptr<const URILabel> >::free,
                    new tr1::shared_ptr<const URILabel>(mm));
        }

        void visit(const URIListedOnlyLabel &)
        {
            value = Data_Wrap_Struct(c_uri_listed_only_label, 0, &Common<tr1::shared_ptr<const URILabel> >::free,
                    new tr1::shared_ptr<const URILabel>(mm));
        }

        void visit(const URIListedThenMirrorsLabel &)
        {
            value = Data_Wrap_Struct(c_uri_listed_then_mirrors_label, 0, &Common<tr1::shared_ptr<const URILabel> >::free,
                    new tr1::shared_ptr<const URILabel>(mm));
        }

        void visit(const URILocalMirrorsOnlyLabel &)
        {
            value = Data_Wrap_Struct(c_uri_local_mirrors_only_label, 0, &Common<tr1::shared_ptr<const URILabel> >::free,
                    new tr1::shared_ptr<const URILabel>(mm));
        }

        void visit(const URIManualOnlyLabel &)
        {
            value = Data_Wrap_Struct(c_uri_manual_only_label, 0, &Common<tr1::shared_ptr<const URILabel> >::free,
                    new tr1::shared_ptr<const URILabel>(mm));
        }
    };

    /*
     * Document-method: text
     *
     * call-seq:
     *     text -> String
     *
     * Our text.
     */
    /*
     * Document-method: to_s
     *
     * call-seq:
     *     to_s -> String
     *
     * Fetch a string representation of ourself.
     */
    VALUE
    uri_label_text(VALUE self)
    {
        tr1::shared_ptr<const URILabel> * ptr;
        Data_Get_Struct(self, tr1::shared_ptr<const URILabel>, ptr);
        return rb_str_new2((*ptr)->text().c_str());
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
         * Document-class: Paludis::AllDepSpec
         *
         * Represents a ( first second third ) or top level group of dependency specs. Includes
         * Enumerable[http://www.ruby-doc.org/core/classes/Enumerable.html].
         */
        c_all_dep_spec = rb_define_class_under(paludis_module(), "AllDepSpec", c_dep_spec);
        rb_funcall(c_all_dep_spec, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_include_module(c_all_dep_spec, rb_mEnumerable);
        rb_define_method(c_all_dep_spec, "each", RUBY_FUNC_CAST((&Composite<AllDepSpec>::each)), 0);

        /*
         * Document-class: Paludis::AnyDepSpec
         *
         * Represents a "|| ( )" dependency block. Includes
         * Enumerable[http://www.ruby-doc.org/core/classes/Enumerable.html].
         */
        c_any_dep_spec = rb_define_class_under(paludis_module(), "AnyDepSpec", c_dep_spec);
        rb_funcall(c_any_dep_spec, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_include_module(c_any_dep_spec, rb_mEnumerable);
        rb_define_method(c_any_dep_spec, "each", RUBY_FUNC_CAST((&Composite<AllDepSpec>::each)), 0);

        /*
         * Document-class: Paludis::UseDepSpec
         *
         * Represents a use? ( ) dependency spec. Includes
         * Enumerable[http://www.ruby-doc.org/core/classes/Enumerable.html].
         */
        c_use_dep_spec = rb_define_class_under(paludis_module(), "UseDepSpec", c_dep_spec);
        rb_funcall(c_use_dep_spec, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_include_module(c_use_dep_spec, rb_mEnumerable);
        rb_define_method(c_use_dep_spec, "flag", RUBY_FUNC_CAST(&use_dep_spec_flag), 0);
        rb_define_method(c_use_dep_spec, "inverse?", RUBY_FUNC_CAST(&use_dep_spec_inverse), 0);
        rb_define_method(c_use_dep_spec, "each", RUBY_FUNC_CAST((&Composite<AllDepSpec>::each)), 0);

        /*
         * Document-class: Paludis::StringDepSpec
         *
         * A StringDepSpec represents a non-composite dep spec with an associated piece of text.
         */
        c_string_dep_spec = rb_define_class_under(paludis_module(), "StringDepSpec", c_dep_spec);
        rb_funcall(c_string_dep_spec, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_method(c_string_dep_spec, "text", RUBY_FUNC_CAST(&string_dep_spec_text), 0);

        /*
         * Document-class: Paludis::FetchableURIDepSpec
         *
         * A FetchableURIDepSpec represents a fetchable URI spec.
         */
        c_fetchable_uri_dep_spec = rb_define_class_under(paludis_module(), "FetchableURIDepSpec", c_string_dep_spec);
        rb_funcall(c_fetchable_uri_dep_spec, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        VALUE (* fetchable_uri_dep_spec_to_s) (VALUE) = &dep_spec_to_s<FetchableURIDepSpec>;
        rb_define_method(c_fetchable_uri_dep_spec, "to_s", RUBY_FUNC_CAST(fetchable_uri_dep_spec_to_s), 0);
        rb_define_method(c_fetchable_uri_dep_spec, "original_url",
                RUBY_FUNC_CAST((&FetchableURIDepSpecStringValue<&FetchableURIDepSpec::original_url>::fetch)), 0);
        rb_define_method(c_fetchable_uri_dep_spec, "renamed_url_suffix",
                RUBY_FUNC_CAST((&FetchableURIDepSpecStringValue<&FetchableURIDepSpec::renamed_url_suffix>::fetch)), 0);
        rb_define_method(c_fetchable_uri_dep_spec, "filename",
                RUBY_FUNC_CAST((&FetchableURIDepSpecStringValue<&FetchableURIDepSpec::filename>::fetch)), 0);

        /*
         * Document-class: Paludis::SimpleURIDepSpec
         *
         * A SimpleURIDepSpec represents a simple URI spec.
         */
        c_simple_uri_dep_spec = rb_define_class_under(paludis_module(), "SimpleURIDepSpec", c_string_dep_spec);
        rb_funcall(c_simple_uri_dep_spec, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        VALUE (* simple_uri_dep_spec_to_s) (VALUE) = &dep_spec_to_s<SimpleURIDepSpec>;
        rb_define_method(c_simple_uri_dep_spec, "to_s", RUBY_FUNC_CAST(simple_uri_dep_spec_to_s), 0);

        /*
         * Document-class: Paludis::LicenseDepSpec
         *
         * A LicenseDepSpec represents a license dep spec.
         */
        c_license_dep_spec = rb_define_class_under(paludis_module(), "LicenseDepSpec", c_string_dep_spec);
        rb_funcall(c_license_dep_spec, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        VALUE (* license_dep_spec_to_s) (VALUE) = &dep_spec_to_s<LicenseDepSpec>;
        rb_define_method(c_license_dep_spec, "to_s", RUBY_FUNC_CAST(license_dep_spec_to_s), 0);

        /*
         * Document-class: Paludis::NamedSetDepSpec
         *
         * A NamedSetDepSpec represents a fetchable URI spec.
         */
        c_named_set_dep_spec = rb_define_class_under(paludis_module(), "NamedSetDepSpec", c_string_dep_spec);
        rb_funcall(c_named_set_dep_spec, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        VALUE (* named_set_dep_spec_to_s) (VALUE) = &dep_spec_to_s<NamedSetDepSpec>;
        rb_define_method(c_named_set_dep_spec, "to_s", RUBY_FUNC_CAST(named_set_dep_spec_to_s), 0);
        rb_define_method(c_named_set_dep_spec, "name", RUBY_FUNC_CAST(&named_set_dep_spec_name), 0);


        /*
         * Document-class: Paludis::PackageDepSpec
         *
         * A PackageDepSpec represents a package name (for example, 'app-editors/vim'),
         * possibly with associated version and SLOT restrictions. To create a PackageDepSpec,
         * use Paludis::parse_user_package_dep_spec or Paludis::make_package_dep_spec.
         */
        c_package_dep_spec = rb_define_class_under(paludis_module(), "PackageDepSpec", c_string_dep_spec);
        rb_define_method(c_package_dep_spec, "package", RUBY_FUNC_CAST(&package_dep_spec_package), 0);
        rb_define_method(c_package_dep_spec, "package_name_part", RUBY_FUNC_CAST(&package_dep_spec_package_name_part), 0);
        rb_define_method(c_package_dep_spec, "category_name_part", RUBY_FUNC_CAST(&package_dep_spec_category_name_part), 0);
        rb_define_method(c_package_dep_spec, "slot", RUBY_FUNC_CAST(&package_dep_spec_slot_ptr), 0);
        rb_define_method(c_package_dep_spec, "repository", RUBY_FUNC_CAST(&package_dep_spec_repository_ptr), 0);
        rb_define_method(c_package_dep_spec, "version_requirements", RUBY_FUNC_CAST(&package_dep_spec_version_requirements_ptr), 0);
        rb_define_method(c_package_dep_spec, "version_requirements_mode", RUBY_FUNC_CAST(&package_dep_spec_version_requirements_mode), 0);
#ifdef CIARANM_REMOVED_THIS
        rb_define_method(c_package_dep_spec, "use_requirements", RUBY_FUNC_CAST(&package_dep_spec_use_requirements), 0);
#endif
        rb_define_method(c_package_dep_spec, "tag", RUBY_FUNC_CAST(&package_dep_spec_tag), 0);
        rb_define_method(c_package_dep_spec, "tag=", RUBY_FUNC_CAST(&package_dep_spec_set_tag), 1);
        rb_define_method(c_package_dep_spec, "without_use_requirements", RUBY_FUNC_CAST(&package_dep_spec_without_use_requirements), 0);
        VALUE (* package_dep_spec_to_s) (VALUE) = &dep_spec_to_s<PackageDepSpec>;
        rb_define_method(c_package_dep_spec, "to_s", RUBY_FUNC_CAST(package_dep_spec_to_s), 0);

        /*
         * Document-class: Paludis::PlainTextDepSpec
         *
         * A PlainTextDepSpec represents a plain text entry (for example, a URI in SRC_URI).
         */
        c_plain_text_dep_spec = rb_define_class_under(paludis_module(), "PlainTextDepSpec", c_string_dep_spec);
        rb_define_singleton_method(c_plain_text_dep_spec, "new", RUBY_FUNC_CAST(&DepSpecThings<PlainTextDepSpec>::dep_spec_new_1), 1);
        rb_define_method(c_plain_text_dep_spec, "initialize", RUBY_FUNC_CAST(&dep_spec_init_1), 1);
        VALUE (* plain_text_dep_spec_to_s) (VALUE) = &dep_spec_to_s<PlainTextDepSpec>;
        rb_define_method(c_plain_text_dep_spec, "to_s", RUBY_FUNC_CAST(plain_text_dep_spec_to_s), 0);

        /*
         * Document-class: Paludis::DependencyLabelsDepSpec
         *
         * A DependencyLabelsDepSpec holds dependency labels.
         */
        c_dependency_labels_dep_spec = rb_define_class_under(paludis_module(), "DependencyLabelsDepSpec", c_string_dep_spec);
        rb_define_singleton_method(c_dependency_labels_dep_spec, "new", RUBY_FUNC_CAST(&DepSpecThings<DependencyLabelsDepSpec>::dep_spec_new_0), 0);
        rb_define_method(c_dependency_labels_dep_spec, "initialize", RUBY_FUNC_CAST(&dep_spec_init_0), 0);
        VALUE (* dependency_labels_dep_spec_to_s) (VALUE) = &dep_spec_to_s<DependencyLabelsDepSpec>;
        rb_define_method(c_dependency_labels_dep_spec, "to_s", RUBY_FUNC_CAST(dependency_labels_dep_spec_to_s), 0);

        /*
         * Document-class: Paludis::URILabelsDepSpec
         *
         * A URILabelsDepSpec holds URI labels.
         */
        c_uri_labels_dep_spec = rb_define_class_under(paludis_module(), "URILabelsDepSpec", c_string_dep_spec);
        rb_define_singleton_method(c_uri_labels_dep_spec, "new", RUBY_FUNC_CAST(&DepSpecThings<URILabelsDepSpec>::dep_spec_new_0), 0);
        rb_define_method(c_uri_labels_dep_spec, "initialize", RUBY_FUNC_CAST(&dep_spec_init_0), 0);
        VALUE (* uri_labels_dep_spec_to_s) (VALUE) = &dep_spec_to_s<URILabelsDepSpec>;
        rb_define_method(c_uri_labels_dep_spec, "to_s", RUBY_FUNC_CAST(uri_labels_dep_spec_to_s), 0);
        rb_define_method(c_uri_labels_dep_spec, "labels", RUBY_FUNC_CAST(&uri_labels_dep_spec_labels), 0);

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
        VALUE (* block_dep_spec_to_s) (VALUE) = &dep_spec_to_s<BlockDepSpec>;
        rb_define_method(c_block_dep_spec, "to_s", RUBY_FUNC_CAST(block_dep_spec_to_s), 0);

        /*
         * Document-class: Paludis::URILabel
         *
         * URI label base class.
         */
        c_uri_label = rb_define_class_under(paludis_module(), "URILabel", rb_cObject);
        rb_funcall(c_uri_label, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_method(c_uri_label, "text", RUBY_FUNC_CAST(&uri_label_text), 0);
        rb_define_method(c_uri_label, "to_s", RUBY_FUNC_CAST(&uri_label_text), 0);

        c_uri_mirrors_then_listed_label = rb_define_class_under(paludis_module(), "URIMirrorsThenListedLabel", c_uri_label);
        c_uri_mirrors_only_label = rb_define_class_under(paludis_module(), "URIMirrorsOnlyLabel", c_uri_label);
        c_uri_listed_only_label = rb_define_class_under(paludis_module(), "URIListedOnlyLabel", c_uri_label);
        c_uri_listed_then_mirrors_label = rb_define_class_under(paludis_module(), "URIListedThenMirrorsLabel", c_uri_label);
        c_uri_local_mirrors_only_label = rb_define_class_under(paludis_module(), "URILocalMirrorsOnlyLabel", c_uri_label);
        c_uri_manual_only_label = rb_define_class_under(paludis_module(), "URIManualOnlyLabel", c_uri_label);

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

        rb_define_module_function(paludis_module(), "parse_user_package_dep_spec", RUBY_FUNC_CAST(&paludis_parse_user_dep_spec), 2);
    }
}

tr1::shared_ptr<const PackageDepSpec>
paludis::ruby::value_to_package_dep_spec(VALUE v)
{
    if (rb_obj_is_kind_of(v, c_package_dep_spec))
    {
        tr1::shared_ptr<WrappedSpecBase> * v_ptr;
        Data_Get_Struct(v, tr1::shared_ptr<WrappedSpecBase>, v_ptr);
        return tr1::static_pointer_cast<const WrappedSpec<PackageDepSpec> >(*v_ptr)->spec();
    }
    else
    {
        rb_raise(rb_eTypeError, "Can't convert %s into PackageDepSpec", rb_obj_classname(v));
    }
}

tr1::shared_ptr<const DepSpec>
paludis::ruby::value_to_dep_spec(VALUE v)
{
    if (rb_obj_is_kind_of(v, c_dep_spec))
    {
        tr1::shared_ptr<WrappedSpecBase> * v_ptr;
        Data_Get_Struct(v, tr1::shared_ptr<WrappedSpecBase>, v_ptr);
        return (*v_ptr)->base_spec();
    }
    else
    {
        rb_raise(rb_eTypeError, "Can't convert %s into PackageDepSpec", rb_obj_classname(v));
    }
}

template <typename H_>
tr1::shared_ptr<const typename H_::ConstItem>
paludis::ruby::value_to_dep_tree(VALUE v)
{
    if (rb_obj_is_kind_of(v, c_dep_spec))
    {
        ValueToTree<H_> vtt(v);
        return vtt.result;
    }
    else
    {
        rb_raise(rb_eTypeError, "Can't convert %s into DepSpec", rb_obj_classname(v));
    }
}

VALUE
paludis::ruby::package_dep_spec_to_value(const tr1::shared_ptr<const PackageDepSpec> & p)
{
    tr1::shared_ptr<const WrappedSpecBase> * v_ptr(new tr1::shared_ptr<const WrappedSpecBase>(
                new WrappedSpec<PackageDepSpec>(tr1::static_pointer_cast<PackageDepSpec>(p->clone()))));
    return Data_Wrap_Struct(c_package_dep_spec, 0, &Common<tr1::shared_ptr<const WrappedSpecBase> >::free, v_ptr);
}

template <typename T_>
VALUE
paludis::ruby::dep_tree_to_value(const tr1::shared_ptr<const typename T_::ConstItem> & m)
{
    try
    {
        TreeToValue v;
        m->accept(v);
        return v.value;
    }
    catch (const std::exception & e)
    {
        exception_to_ruby_exception(e);
    }
}

VALUE
paludis::ruby::uri_label_to_value(const tr1::shared_ptr<const URILabel> & m)
{
    try
    {
        URILabelToValue v(m);
        m->accept(v);
        return v.value;
    }
    catch (const std::exception & e)
    {
        exception_to_ruby_exception(e);
    }
}

template VALUE dep_tree_to_value <SetSpecTree> (const tr1::shared_ptr<const SetSpecTree::ConstItem> &);
template VALUE dep_tree_to_value <DependencySpecTree> (const tr1::shared_ptr<const DependencySpecTree::ConstItem> &);
template VALUE dep_tree_to_value <FetchableURISpecTree> (const tr1::shared_ptr<const FetchableURISpecTree::ConstItem> &);
template VALUE dep_tree_to_value <SimpleURISpecTree> (const tr1::shared_ptr<const SimpleURISpecTree::ConstItem> &);
template VALUE dep_tree_to_value <RestrictSpecTree> (const tr1::shared_ptr<const RestrictSpecTree::ConstItem> &);
template VALUE dep_tree_to_value <ProvideSpecTree> (const tr1::shared_ptr<const ProvideSpecTree::ConstItem> &);
template VALUE dep_tree_to_value <LicenseSpecTree> (const tr1::shared_ptr<const LicenseSpecTree::ConstItem> &);

template tr1::shared_ptr<const SetSpecTree::ConstItem> value_to_dep_tree <SetSpecTree> (VALUE);

RegisterRubyClass::Register paludis_ruby_register_dep_spec PALUDIS_ATTRIBUTE((used))
    (&do_register_dep_spec);

