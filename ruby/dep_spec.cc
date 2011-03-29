/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
 * Copyright (c) 2006, 2007, 2008 Richard Brown
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
#include <paludis/user_dep_spec.hh>
#include <paludis/version_requirements.hh>
#include <paludis/version_operator.hh>
#include <paludis/package_dep_spec_constraint.hh>

#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/options.hh>
#include <paludis/util/save.hh>
#include <paludis/util/member_iterator-impl.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/util/accept_visitor.hh>

#include <algorithm>
#include <list>
#include <ruby.h>

#include "nice_names-nn.hh"

using namespace paludis;
using namespace paludis::ruby;

namespace
{
    static VALUE c_dep_spec;
    static VALUE c_string_dep_spec;

    static VALUE c_block_dep_spec;
    static VALUE c_dependencies_labels_dep_spec;
    static VALUE c_fetchable_uri_dep_spec;
    static VALUE c_license_dep_spec;
    static VALUE c_named_set_dep_spec;
    static VALUE c_package_dep_spec;
    static VALUE c_plain_text_dep_spec;
    static VALUE c_simple_uri_dep_spec;
    static VALUE c_uri_labels_dep_spec;
    static VALUE c_plain_text_label_dep_spec;

    static VALUE c_all_dep_spec;
    static VALUE c_any_dep_spec;
    static VALUE c_exactly_one_dep_spec;
    static VALUE c_conditional_dep_spec;

    static VALUE c_version_requirements_mode;

    static VALUE c_slot_requirement;
    static VALUE c_slot_exact_requirement;
    static VALUE c_slot_any_locked_requirement;
    static VALUE c_slot_any_unlocked_requirement;

    struct V
    {
        VALUE value;
        std::shared_ptr<const SlotRequirement> mm;

        V(std::shared_ptr<const SlotRequirement> _m) :
            mm(_m)
        {
        }

        void visit(const SlotExactRequirement &)
        {
            value = Data_Wrap_Struct(c_slot_exact_requirement, 0, &Common<std::shared_ptr<const SlotRequirement> >::free,
                    new std::shared_ptr<const SlotRequirement>(mm));
        }

        void visit(const SlotAnyLockedRequirement &)
        {
            value = Data_Wrap_Struct(c_slot_any_locked_requirement, 0, &Common<std::shared_ptr<const SlotRequirement> >::free,
                    new std::shared_ptr<const SlotRequirement>(mm));
        }

        void visit(const SlotAnyUnlockedRequirement &)
        {
            value = Data_Wrap_Struct(c_slot_any_unlocked_requirement, 0, &Common<std::shared_ptr<const SlotRequirement> >::free,
                    new std::shared_ptr<const SlotRequirement>(mm));
        }

    };

    VALUE
    slot_requirement_to_value(std::shared_ptr<const SlotRequirement> m)
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

    struct WrappedSpecBase;
    template <typename> struct WrappedSpec;

    struct WrappedSpecBase :
        virtual DeclareAbstractAcceptMethods<WrappedSpecBase, MakeTypeList<
            WrappedSpec<PlainTextDepSpec>,
            WrappedSpec<SimpleURIDepSpec>,
            WrappedSpec<FetchableURIDepSpec>,
            WrappedSpec<LicenseDepSpec>,
            WrappedSpec<PackageDepSpec>,
            WrappedSpec<BlockDepSpec>,
            WrappedSpec<URILabelsDepSpec>,
            WrappedSpec<PlainTextLabelDepSpec>,
            WrappedSpec<DependenciesLabelsDepSpec>,
            WrappedSpec<NamedSetDepSpec>,
            WrappedSpec<AllDepSpec>,
            WrappedSpec<AnyDepSpec>,
            WrappedSpec<ExactlyOneDepSpec>,
            WrappedSpec<ConditionalDepSpec>
        >::Type>
    {
        typedef std::list<std::pair<VALUE, std::shared_ptr<const WrappedSpecBase> > > Children;

        virtual ~WrappedSpecBase()
        {
        }

        virtual const std::shared_ptr<const DepSpec> base_spec() const = 0;
        virtual const std::shared_ptr<const Children> children() const = 0;
    };

    template <typename T_>
    class WrappedSpec :
        public WrappedSpecBase,
        public ImplementAcceptMethods<WrappedSpecBase, WrappedSpec<T_> >
    {
        private:
            std::shared_ptr<T_> _spec;
            std::shared_ptr<Children> _children;

        public:
            WrappedSpec(const std::shared_ptr<T_> & s) :
                _spec(s),
                _children(new Children)
            {
            }

            virtual const std::shared_ptr<const DepSpec> base_spec() const
            {
                return _spec;
            }

            const std::shared_ptr<const T_> spec() const
            {
                return _spec;
            }

            const std::shared_ptr<T_> spec()
            {
                return _spec;
            }

            template <typename Iter_>
            WrappedSpec * add_children(Iter_ cur, const Iter_ end);

            virtual const std::shared_ptr<const Children> children() const
            {
                return _children;
            }
    };

    struct TreeToValue
    {
        std::shared_ptr<const WrappedSpecBase> wrapped;
        VALUE klass;

        void visit(const GenericSpecTree::NodeType<PackageDepSpec>::Type & node)
        {
            wrapped.reset(new WrappedSpec<PackageDepSpec>(std::static_pointer_cast<PackageDepSpec>(node.spec()->clone())));
            klass = c_package_dep_spec;
        }

        void visit(const GenericSpecTree::NodeType<BlockDepSpec>::Type & node)
        {
            wrapped.reset(new WrappedSpec<BlockDepSpec>(std::static_pointer_cast<BlockDepSpec>(node.spec()->clone())));
            klass = c_block_dep_spec;
        }

        void visit(const GenericSpecTree::NodeType<PlainTextDepSpec>::Type & node)
        {
            wrapped.reset(new WrappedSpec<PlainTextDepSpec>(std::static_pointer_cast<PlainTextDepSpec>(node.spec()->clone())));
            klass = c_plain_text_dep_spec;
        }

        void visit(const GenericSpecTree::NodeType<SimpleURIDepSpec>::Type & node)
        {
            wrapped.reset(new WrappedSpec<SimpleURIDepSpec>(std::static_pointer_cast<SimpleURIDepSpec>(node.spec()->clone())));
            klass = c_simple_uri_dep_spec;
        }

        void visit(const GenericSpecTree::NodeType<FetchableURIDepSpec>::Type & node)
        {
            wrapped.reset(new WrappedSpec<FetchableURIDepSpec>(std::static_pointer_cast<FetchableURIDepSpec>(node.spec()->clone())));
            klass = c_fetchable_uri_dep_spec;
        }

        void visit(const GenericSpecTree::NodeType<URILabelsDepSpec>::Type & node)
        {
            wrapped.reset(new WrappedSpec<URILabelsDepSpec>(std::static_pointer_cast<URILabelsDepSpec>(node.spec()->clone())));
            klass = c_uri_labels_dep_spec;
        }

        void visit(const GenericSpecTree::NodeType<PlainTextLabelDepSpec>::Type & node)
        {
            wrapped.reset(new WrappedSpec<PlainTextLabelDepSpec>(std::static_pointer_cast<PlainTextLabelDepSpec>(node.spec()->clone())));
            klass = c_plain_text_label_dep_spec;
        }

        void visit(const GenericSpecTree::NodeType<DependenciesLabelsDepSpec>::Type & node)
        {
            wrapped.reset(new WrappedSpec<DependenciesLabelsDepSpec>(std::static_pointer_cast<DependenciesLabelsDepSpec>(node.spec()->clone())));
            klass = c_dependencies_labels_dep_spec;
        }

        void visit(const GenericSpecTree::NodeType<NamedSetDepSpec>::Type & node)
        {
            wrapped.reset(new WrappedSpec<NamedSetDepSpec>(std::static_pointer_cast<NamedSetDepSpec>(node.spec()->clone())));
            klass = c_named_set_dep_spec;
        }

        void visit(const GenericSpecTree::NodeType<LicenseDepSpec>::Type & node)
        {
            wrapped.reset(new WrappedSpec<LicenseDepSpec>(std::static_pointer_cast<LicenseDepSpec>(node.spec()->clone())));
            klass = c_license_dep_spec;
        }

        void visit(const GenericSpecTree::NodeType<ConditionalDepSpec>::Type & node)
        {
            wrapped.reset((new WrappedSpec<ConditionalDepSpec>(std::static_pointer_cast<ConditionalDepSpec>(node.spec()->clone())))->add_children(node.begin(), node.end()));
            klass = c_conditional_dep_spec;
        }

        void visit(const GenericSpecTree::NodeType<AllDepSpec>::Type & node)
        {
            wrapped.reset((new WrappedSpec<AllDepSpec>(std::static_pointer_cast<AllDepSpec>(node.spec()->clone())))->add_children(node.begin(), node.end()));
            klass = c_all_dep_spec;
        }

        void visit(const GenericSpecTree::NodeType<AnyDepSpec>::Type & node)
        {
            wrapped.reset((new WrappedSpec<AnyDepSpec>(std::static_pointer_cast<AnyDepSpec>(node.spec()->clone())))->add_children(node.begin(), node.end()));
            klass = c_any_dep_spec;
        }

        void visit(const GenericSpecTree::NodeType<ExactlyOneDepSpec>::Type & node)
        {
            wrapped.reset((new WrappedSpec<ExactlyOneDepSpec>(std::static_pointer_cast<ExactlyOneDepSpec>(node.spec()->clone())))->add_children(node.begin(), node.end()));
            klass = c_exactly_one_dep_spec;
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
            (*cur)->accept(v);
            _children->push_back(std::make_pair(v.klass, v.wrapped));
        }

        return this;
    }

    template <typename H_, typename S_, bool>
    struct InnerNodeHandler;

    template <typename H_, typename S_, bool>
    struct LeafNodeHandler;

    template <typename H_>
    struct ValueToTree
    {
        std::shared_ptr<H_> result;
        std::shared_ptr<typename H_::BasicInnerNode> add_to;

        ValueToTree(VALUE val, std::shared_ptr<H_> r = make_null_shared_ptr()) :
            result(r)
        {
            if (result)
                add_to = result->top();

            std::shared_ptr<WrappedSpecBase> * p;
            Data_Get_Struct(val, std::shared_ptr<WrappedSpecBase>, p);
            (*p)->accept(*this);
        }

        void visit(const WrappedSpec<AllDepSpec> & spec)
        {
            InnerNodeHandler<H_, AllDepSpec,
                TypeListContains<typename H_::VisitableTypeList, typename H_::template NodeType<AllDepSpec>::Type>::value>::handle(this, spec);
        }

        void visit(const WrappedSpec<ConditionalDepSpec> & spec)
        {
            InnerNodeHandler<H_, ConditionalDepSpec,
                TypeListContains<typename H_::VisitableTypeList, typename H_::template NodeType<ConditionalDepSpec>::Type>::value>::handle(this, spec);
        }

        template <typename T_>
        void visit(const WrappedSpec<T_> & spec)
        {
            LeafNodeHandler<H_, T_,
                TypeListContains<typename H_::VisitableTypeList, typename H_::template NodeType<T_>::Type>::value>::handle(this, spec);
        }
    };

    template <typename H_, typename S_>
    struct InnerNodeHandler<H_, S_, false>
    {
        static void handle(ValueToTree<H_> * const, const WrappedSpec<S_> &)
        {
            rb_raise(rb_eTypeError, "Item of type %s is not allowed in hierarchy of type %s", NiceNames<S_>::name, NiceNames<H_>::name);
        }
    };

    template <typename H_, typename S_>
    struct LeafNodeHandler<H_, S_, false>
    {
        static void handle(ValueToTree<H_> * const, const WrappedSpec<S_> &)
        {
            rb_raise(rb_eTypeError, "Item of type %s is not allowed in hierarchy of type %s", NiceNames<S_>::name, NiceNames<H_>::name);
        }
    };

    template <typename H_, typename S_>
    struct TopNodeHandler
    {
        static void handle(ValueToTree<H_> * const, const WrappedSpec<S_> &)
        {
            rb_raise(rb_eTypeError, "Item of type %s is not allowed as the root for heirarchy of type %s", NiceNames<S_>::name, NiceNames<H_>::name);
        }
    };

    template <typename H_>
    struct TopNodeHandler<H_, AllDepSpec>
    {
        static void handle(ValueToTree<H_> * const t, const WrappedSpec<AllDepSpec> & spec)
        {
            t->result.reset(new H_(spec.spec()));
            t->add_to = t->result->top();
        }
    };

    template <typename H_, typename S_>
    struct InnerNodeHandler<H_, S_, true>
    {
        static void handle(ValueToTree<H_> * const t, const WrappedSpec<S_> & spec)
        {
            if (! t->result)
            {
                TopNodeHandler<H_, S_>::handle(t, spec);
                std::for_each(indirect_iterator(second_iterator(spec.children()->begin())),
                        indirect_iterator(second_iterator(spec.children()->end())),
                        accept_visitor(*t));
            }
            else
            {
                Save<std::shared_ptr<typename H_::BasicInnerNode> > save(&t->add_to, t->add_to->append(spec.spec()));
                std::for_each(indirect_iterator(second_iterator(spec.children()->begin())),
                        indirect_iterator(second_iterator(spec.children()->end())),
                        accept_visitor(*t));
            }
        }
    };

    template <typename H_, typename S_>
    struct LeafNodeHandler<H_, S_, true>
    {
        static void handle(ValueToTree<H_> * const t, const WrappedSpec<S_> & spec)
        {
            t->add_to->append(spec.spec());
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
        std::shared_ptr<WrappedSpecBase> * ptr;
        Data_Get_Struct(self, std::shared_ptr<WrappedSpecBase>, ptr);
        std::shared_ptr<const URILabelsDepSpec> real_ptr(std::static_pointer_cast<const WrappedSpec<URILabelsDepSpec> >(*ptr)->spec());

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
    dependencies_labels_dep_spec_labels(VALUE self)
    {
        std::shared_ptr<WrappedSpecBase> * ptr;
        Data_Get_Struct(self, std::shared_ptr<WrappedSpecBase>, ptr);
        std::shared_ptr<const DependenciesLabelsDepSpec> real_ptr(std::static_pointer_cast<const WrappedSpec<DependenciesLabelsDepSpec> >(*ptr)->spec());

        if (rb_block_given_p())
        {
            for (DependenciesLabelsDepSpec::ConstIterator it(real_ptr->begin()),
                     it_end(real_ptr->end()); it_end != it; ++it)
                rb_yield(dependencies_label_to_value(*it));

            return Qnil;
        }
        else
        {
            VALUE result(rb_ary_new());

            for (DependenciesLabelsDepSpec::ConstIterator it(real_ptr->begin()),
                     it_end(real_ptr->end()); it_end != it; ++it)
                rb_ary_push(result, dependencies_label_to_value(*it));

            return result;
        }
    }

    VALUE
    block_dep_spec_new(VALUE self, VALUE str, VALUE spec)
    {
        std::shared_ptr<const WrappedSpecBase> * ptr(0);
        try
        {
            std::shared_ptr<const PackageDepSpec> pkg(value_to_package_dep_spec(spec));
            ptr = new std::shared_ptr<const WrappedSpecBase>(std::make_shared<WrappedSpec<BlockDepSpec>>(
                        std::make_shared<BlockDepSpec>(StringValuePtr(str), *pkg)));
            VALUE tdata(Data_Wrap_Struct(self, 0, &Common<std::shared_ptr<const WrappedSpecBase> >::free, ptr));
            rb_obj_call_init(tdata, 2, &str);
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
     *     blocking -> DepSpec
     *
     * Fetch the DepSpec we're blocking.
     */
    VALUE
    block_dep_spec_blocking(VALUE self)
    {
        std::shared_ptr<WrappedSpecBase> * p;
        Data_Get_Struct(self, std::shared_ptr<WrappedSpecBase>, p);
        return package_dep_spec_to_value(*std::make_shared<PackageDepSpec>(
                    std::static_pointer_cast<const WrappedSpec<BlockDepSpec> >(*p)->spec()->blocking()));
    }

    template <typename A_>
    struct DepSpecThings
    {
        static VALUE
        dep_spec_new_0(VALUE self)
        {
            std::shared_ptr<const WrappedSpecBase> * ptr(0);
            try
            {
                ptr = new std::shared_ptr<const WrappedSpecBase>(std::make_shared<WrappedSpec<A_>>(std::make_shared<A_>()));
                VALUE tdata(Data_Wrap_Struct(self, 0, &Common<std::shared_ptr<const WrappedSpecBase> >::free, ptr));
                rb_obj_call_init(tdata, 0, &self);
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
            std::shared_ptr<const WrappedSpecBase> * ptr(0);
            try
            {
                ptr = new std::shared_ptr<const WrappedSpecBase>(std::make_shared<WrappedSpec<A_>>(std::make_shared<A_>(StringValuePtr(s))));
                VALUE tdata(Data_Wrap_Struct(self, 0, &Common<std::shared_ptr<const WrappedSpecBase> >::free, ptr));
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
     * Document-method: condition_met?
     * call-seq:
     *     condition_met?(Environment, PackageID) -> true or false
     *
     * Whether our condition is met.
     */
    /*
     * Document-method: condition_meetable?
     * call-seq:
     *     condition_meetable?(Environment, PackageID) -> true or false
     *
     * Whether our condition is meetable.
     */
    template <bool (ConditionalDepSpec::* f_) (const Environment * const, const std::shared_ptr<const PackageID> &) const>
    struct ConditionalDepSpecBoolFunc
    {
        static VALUE func(VALUE self, VALUE env, VALUE id)
        {
            std::shared_ptr<WrappedSpecBase> * p;
            Data_Get_Struct(self, std::shared_ptr<WrappedSpecBase>, p);
            std::shared_ptr<Environment> e(value_to_environment(env));
            std::shared_ptr<const PackageID> i(value_to_package_id(id));
            return ((*std::static_pointer_cast<const WrappedSpec<ConditionalDepSpec> >(*p)->spec().get()).*f_)(e.get(), i) ? Qtrue : Qfalse;
        }
    };

    /*
     * call-seq:
     *     package_name_constraint -> NameConstraint or Nil
     *
     * Fetch the package name constraint (may be Nil).
     */
    VALUE
    package_dep_spec_package_name_constraint(VALUE self)
    {
        std::shared_ptr<WrappedSpecBase> * ptr;
        Data_Get_Struct(self, std::shared_ptr<WrappedSpecBase>, ptr);
        if (! bool(std::static_pointer_cast<const WrappedSpec<PackageDepSpec> >(*ptr)->spec()->package_name_constraint()))
            return Qnil;
        return package_dep_spec_constraint_to_value(std::static_pointer_cast<const WrappedSpec<PackageDepSpec> >(*ptr)->spec()->package_name_constraint());
    }

    /*
     * call-seq:
     *     package_name_part_constraint -> PackageNamePartConstraint or Nil
     *
     * Fetch the package name part constraint (may be Nil).
     */
    VALUE
    package_dep_spec_package_name_part_constraint(VALUE self)
    {
        std::shared_ptr<WrappedSpecBase> * ptr;
        Data_Get_Struct(self, std::shared_ptr<WrappedSpecBase>, ptr);
        if (! bool(std::static_pointer_cast<const WrappedSpec<PackageDepSpec> >(*ptr)->spec()->package_name_part_constraint()))
            return Qnil;
        return package_dep_spec_constraint_to_value(std::static_pointer_cast<const WrappedSpec<PackageDepSpec> >(*ptr)->spec()->package_name_part_constraint());
    }

    /*
     * call-seq:
     *     category_name_part_constraint -> CategoryNamePartConstraint or Nil
     *
     * Fetch the category name part constraint (may be Nil).
     */
    VALUE
    package_dep_spec_category_name_part_constraint(VALUE self)
    {
        std::shared_ptr<WrappedSpecBase> * ptr;
        Data_Get_Struct(self, std::shared_ptr<WrappedSpecBase>, ptr);
        if (! bool(std::static_pointer_cast<const WrappedSpec<PackageDepSpec> >(*ptr)->spec()->category_name_part_constraint()))
            return Qnil;
        return package_dep_spec_constraint_to_value(std::static_pointer_cast<const WrappedSpec<PackageDepSpec> >(*ptr)->spec()->category_name_part_constraint());
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
        std::shared_ptr<WrappedSpecBase> * ptr;
        Data_Get_Struct(self, std::shared_ptr<WrappedSpecBase>, ptr);
        return rb_str_new2(stringify(std::static_pointer_cast<const StringDepSpec>((*ptr)->base_spec())->text()).c_str());
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
        std::shared_ptr<WrappedSpecBase> * ptr;
        Data_Get_Struct(self, std::shared_ptr<WrappedSpecBase>, ptr);
        return rb_str_new2(stringify(*std::static_pointer_cast<const WrappedSpec<T_> >(*ptr)->spec()).c_str());
    }

    /*
     * call-seq:
     *     slot_requirement -> SlotRequirement or Nil
     *
     * Fetch the slot requirement.
     */
    VALUE
    package_dep_spec_slot_requirement_ptr(VALUE self)
    {
        std::shared_ptr<WrappedSpecBase> * ptr;
        Data_Get_Struct(self, std::shared_ptr<WrappedSpecBase>, ptr);
        if (! bool(std::static_pointer_cast<const WrappedSpec<PackageDepSpec> >(*ptr)->spec()->slot_requirement_ptr()))
            return Qnil;
        return slot_requirement_to_value(std::static_pointer_cast<const WrappedSpec<PackageDepSpec> >(*ptr)->spec()->slot_requirement_ptr());
    }

    /*
     * call-seq:
     *     in_repository -> String or Nil
     *
     * Fetch the in-repository name.
     */
    VALUE
    package_dep_spec_in_repository_ptr(VALUE self)
    {
        std::shared_ptr<WrappedSpecBase> * ptr;
        Data_Get_Struct(self, std::shared_ptr<WrappedSpecBase>, ptr);
        if (! bool(std::static_pointer_cast<const WrappedSpec<PackageDepSpec> >(*ptr)->spec()->in_repository_ptr()))
            return Qnil;
        return rb_str_new2(stringify((*std::static_pointer_cast<const WrappedSpec<PackageDepSpec> >(*ptr)->spec()->in_repository_ptr())).c_str());
    }

    /*
     * call-seq:
     *     from_repository -> String or Nil
     *
     * Fetch the from-repository name.
     */
    VALUE
    package_dep_spec_from_repository_ptr(VALUE self)
    {
        std::shared_ptr<WrappedSpecBase> * ptr;
        Data_Get_Struct(self, std::shared_ptr<WrappedSpecBase>, ptr);
        if (! bool(std::static_pointer_cast<const WrappedSpec<PackageDepSpec> >(*ptr)->spec()->from_repository_ptr()))
            return Qnil;
        return rb_str_new2(stringify((*std::static_pointer_cast<const WrappedSpec<PackageDepSpec> >(*ptr)->spec()->from_repository_ptr())).c_str());
    }

    /*
     * call-seq:
     *     installable_to_repository -> Hash or Nil
     *
     * Fetch the installable-to-repository requirement.
     */
    VALUE
    package_dep_spec_installable_to_repository(VALUE self)
    {
        std::shared_ptr<WrappedSpecBase> * ptr;
        Data_Get_Struct(self, std::shared_ptr<WrappedSpecBase>, ptr);
        std::shared_ptr<const InstallableToRepository> i2r(std::static_pointer_cast<const WrappedSpec<PackageDepSpec> >(*ptr)->spec()->installable_to_repository_ptr());
        if (! i2r)
            return Qnil;
        VALUE result(rb_hash_new());
        rb_hash_aset(result, ID2SYM(rb_intern("repository")),
            rb_str_new2(stringify(i2r->repository()).c_str()));
        rb_hash_aset(result, ID2SYM(rb_intern("include_masked?")),
            i2r->include_masked() ? Qtrue : Qfalse);
        return result;
    }

    /*
     * call-seq:
     *     installed_at_path -> String or Nil
     *
     * Fetch the installed-at-path requirement.
     */
    VALUE
    package_dep_spec_installed_at_path(VALUE self)
    {
        std::shared_ptr<WrappedSpecBase> * ptr;
        Data_Get_Struct(self, std::shared_ptr<WrappedSpecBase>, ptr);
        if (! bool(std::static_pointer_cast<const WrappedSpec<PackageDepSpec> >(*ptr)->spec()->installed_at_path_ptr()))
            return Qnil;
        return rb_str_new2(stringify((*std::static_pointer_cast<const WrappedSpec<PackageDepSpec> >(*ptr)->spec()->installed_at_path_ptr())).c_str());
    }

    /*
     * call-seq:
     *     installable_to_path -> Hash or Nil
     *
     * Fetch the installable-to-path requirement.
     */
    VALUE
    package_dep_spec_installable_to_path(VALUE self)
    {
        std::shared_ptr<WrappedSpecBase> * ptr;
        Data_Get_Struct(self, std::shared_ptr<WrappedSpecBase>, ptr);
        std::shared_ptr<const InstallableToPath> i2p(std::static_pointer_cast<const WrappedSpec<PackageDepSpec> >(*ptr)->spec()->installable_to_path_ptr());
        if (! i2p)
            return Qnil;
        VALUE result(rb_hash_new());
        rb_hash_aset(result, ID2SYM(rb_intern("path")),
            rb_str_new2(stringify(i2p->path()).c_str()));
        rb_hash_aset(result, ID2SYM(rb_intern("include_masked?")),
            i2p->include_masked() ? Qtrue : Qfalse);
        return result;
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
        std::shared_ptr<WrappedSpecBase> * ptr;
        Data_Get_Struct(self, std::shared_ptr<WrappedSpecBase>, ptr);
        VALUE result(rb_ary_new());
        VALUE result_hash;
        if (std::static_pointer_cast<const WrappedSpec<PackageDepSpec> >(*ptr)->spec()->version_requirements_ptr())
            for (VersionRequirements::ConstIterator i(std::static_pointer_cast<const PackageDepSpec>((*ptr)->base_spec())->
                        version_requirements_ptr()->begin()),
                    i_end(std::static_pointer_cast<const PackageDepSpec>((*ptr)->base_spec())->version_requirements_ptr()->end()) ;
                    i != i_end; ++i)
            {
                result_hash = rb_hash_new();
                rb_hash_aset(result_hash, ID2SYM(rb_intern("operator")),
                    rb_str_new2(stringify(i->version_operator()).c_str()));
                rb_hash_aset(result_hash, ID2SYM(rb_intern("spec")),
                    version_spec_to_value(i->version_spec()));
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
        std::shared_ptr<WrappedSpecBase> * ptr;
        Data_Get_Struct(self, std::shared_ptr<WrappedSpecBase>, ptr);
        VALUE result(rb_ary_new());
        VALUE result_hash;
        if (std::static_pointer_cast<const WrappedSpec<PackageDepSpec> >(*ptr)->spec()->use_requirements_ptr())
            for (UseRequirements::ConstIterator
                    i(std::static_pointer_cast<const WrappedSpec<PackageDepSpec> >(*ptr)->spec()->use_requirements_ptr()->begin()),
                    i_end(std::static_pointer_cast<const WrappedSpec<PackageDepSpec> >(*ptr)->spec()->use_requirements_ptr()->end()) ;
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
        std::shared_ptr<WrappedSpecBase> * ptr;
        Data_Get_Struct(self, std::shared_ptr<WrappedSpecBase>, ptr);
        return INT2FIX(std::static_pointer_cast<const WrappedSpec<PackageDepSpec> >(*ptr)->spec()->version_requirements_mode());
    }

    /*
     * Document-method: original_url
     *     call-seq: original_url -> String
     *
     * The original URL (that is, the text to the left of the arrow, if present,
     * or the entire text otherwise).
     */
    /*
     * Document-method: renamed_url_suffix
     *     call-seq: renamed_url_suffix -> String
     *
     * The renamed URL filename (that is, the text to the right of the arrow,
     * if present, or an empty string otherwise).
     */
    /*
     * Document-method: filename
     *     call-seq: filename -> String
     *
     * The filename (that is, the renamed URL suffix, if present, or the text
     * after the final / in the original URL otherwise).
     */
    template <std::string (FetchableURIDepSpec::* m_) () const>
    struct FetchableURIDepSpecStringValue
    {
        static VALUE
        fetch(VALUE self)
        {
            std::shared_ptr<WrappedSpecBase> * ptr;
            Data_Get_Struct(self, std::shared_ptr<WrappedSpecBase>, ptr);
            std::shared_ptr<const FetchableURIDepSpec> f_ptr(std::static_pointer_cast<const FetchableURIDepSpec>((*ptr)->base_spec()));
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
            std::shared_ptr<WrappedSpecBase> * ptr;
            Data_Get_Struct(self, std::shared_ptr<WrappedSpecBase>, ptr);

            if ((*ptr)->children())
                for (WrappedSpecBase::Children::const_iterator i((*ptr)->children()->begin()), i_end((*ptr)->children()->end()) ;
                        i != i_end ; ++i)
                {
                    std::shared_ptr<const WrappedSpecBase> * newptr(new std::shared_ptr<const WrappedSpecBase>(i->second));
                    rb_yield(Data_Wrap_Struct(i->first, 0, &Common<std::shared_ptr<const WrappedSpecBase> >::free, newptr));
                }
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
        std::shared_ptr<WrappedSpecBase> * ptr;
        Data_Get_Struct(self, std::shared_ptr<WrappedSpecBase>, ptr);
        return rb_str_new2(stringify(std::static_pointer_cast<const WrappedSpec<NamedSetDepSpec> >(*ptr)->spec()->text()).c_str());
    }

    /*
     * Document-method: parse_user_package_dep_spec
     *
     * call-seq:
     *     parse_user_package_dep_spec(String, Env, Array) -> PackageDepSpec
     *     parse_user_package_dep_spec(String, Env, Array, Filter) -> PackageDepSpec
     *
     * Return a PackageDepSpec parsed from user input. The third parameter is either an empty
     * array, or can contain :allow_wildcards to allow wildcards, :throw_if_set to get a
     * GotASetNotAPackageDepSpec exception if the string is a set name and :no_disambiguation
     * to disallow disambiguation (require an explicit category). The Filter, if
     * provided, is used to restrict disambiguation as per
     * Environment#fetch_unique_qualified_package_name.
     *
     */
    VALUE paludis_parse_user_dep_spec(int argc, VALUE * argv, VALUE)
    {
        std::shared_ptr<const WrappedSpecBase> * ptr(0);

        if (argc < 3 || argc > 4)
            rb_raise(rb_eArgError, "parse_user_package_dep_spec expects three to four arguments, but got %d", argc);

        try
        {
            std::string s(StringValuePtr(argv[0]));
            std::shared_ptr<Environment> e(value_to_environment(argv[1]));

            Check_Type(argv[2], T_ARRAY);
            UserPackageDepSpecOptions o;
            for (int i(0) ; i < RARRAY_LEN(argv[2]) ; ++i)
            {
                VALUE entry(rb_ary_entry(argv[2], i));
                Check_Type(entry, T_SYMBOL);
                if (SYM2ID(entry) == rb_intern("allow_wildcards"))
                    o += updso_allow_wildcards;
                else if (SYM2ID(entry) == rb_intern("throw_if_set"))
                    o += updso_throw_if_set;
                else if (SYM2ID(entry) == rb_intern("no_disambiguation"))
                    o += updso_no_disambiguation;
                else
                    rb_raise(rb_eArgError, "Unknown parse_user_package_dep_spec option '%s'", rb_obj_as_string(entry));
            }

            Filter f(
                    argc >= 4 ?
                    value_to_filter(argv[3]) :
                    filter::All()
                    );

            ptr = new std::shared_ptr<const WrappedSpecBase>(std::make_shared<WrappedSpec<PackageDepSpec>>(
                        std::make_shared<PackageDepSpec>(parse_user_package_dep_spec(s, e.get(), o, f))));
            return Data_Wrap_Struct(c_package_dep_spec, 0,
                    &Common<std::shared_ptr<const WrappedSpecBase> >::free, ptr);
        }
        catch (const std::exception & e)
        {
            delete ptr;
            exception_to_ruby_exception(e);
        }

    }

    VALUE
    slot_exact_requirement_slot(VALUE self)
    {
        std::shared_ptr<const SlotExactRequirement> * ptr;
        Data_Get_Struct(self, std::shared_ptr<const SlotExactRequirement>, ptr);
        return rb_str_new2(stringify((*ptr)->slot()).c_str());
    }

    void do_register_dep_spec()
    {
        /*
         * Document-class: Paludis::DepSpec
         *
         * Base class for a dependencies spec.
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
         * Document-class: Paludis::ExactlyOneDepSpec
         *
         * Represents a "^^ ( )" dependency block. Includes
         * Enumerable[http://www.ruby-doc.org/core/classes/Enumerable.html].
         */
        c_exactly_one_dep_spec = rb_define_class_under(paludis_module(), "ExactlyOneDepSpec", c_dep_spec);
        rb_funcall(c_exactly_one_dep_spec, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_include_module(c_exactly_one_dep_spec, rb_mEnumerable);
        rb_define_method(c_exactly_one_dep_spec, "each", RUBY_FUNC_CAST((&Composite<AllDepSpec>::each)), 0);

        /*
         * Document-class: Paludis::ConditionalDepSpec
         *
         * Represents a use? ( ) dependency spec. Includes
         * Enumerable[http://www.ruby-doc.org/core/classes/Enumerable.html].
         */
        c_conditional_dep_spec = rb_define_class_under(paludis_module(), "ConditionalDepSpec", c_dep_spec);
        rb_funcall(c_conditional_dep_spec, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_include_module(c_conditional_dep_spec, rb_mEnumerable);
        /*
         * Document-method: condition
         *
         * call-seq:
         *     confition -> String
         *
         * Our condition
         */
        VALUE (* conditional_dep_spec_to_s) (VALUE) = &dep_spec_to_s<ConditionalDepSpec>;
        rb_define_method(c_conditional_dep_spec, "condition", RUBY_FUNC_CAST(conditional_dep_spec_to_s), 0);
        rb_define_alias(c_conditional_dep_spec, "to_s", "condition");
        rb_define_method(c_conditional_dep_spec, "condition_met?", RUBY_FUNC_CAST(
                    &ConditionalDepSpecBoolFunc<&ConditionalDepSpec::condition_met>::func), 2);
        rb_define_method(c_conditional_dep_spec, "condition_meetable?", RUBY_FUNC_CAST(
                    &ConditionalDepSpecBoolFunc<&ConditionalDepSpec::condition_meetable>::func), 2);
        rb_define_method(c_conditional_dep_spec, "each", RUBY_FUNC_CAST((&Composite<AllDepSpec>::each)), 0);

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
         * use Paludis::parse_user_package_dep_spec.
         */
        c_package_dep_spec = rb_define_class_under(paludis_module(), "PackageDepSpec", c_string_dep_spec);
        rb_define_method(c_package_dep_spec, "package_name_constraint", RUBY_FUNC_CAST(&package_dep_spec_package_name_constraint), 0);
        rb_define_method(c_package_dep_spec, "package_name_part_constraint", RUBY_FUNC_CAST(&package_dep_spec_package_name_part_constraint), 0);
        rb_define_method(c_package_dep_spec, "category_name_part_constraint", RUBY_FUNC_CAST(&package_dep_spec_category_name_part_constraint), 0);
        rb_define_method(c_package_dep_spec, "slot_requirement", RUBY_FUNC_CAST(&package_dep_spec_slot_requirement_ptr), 0);
        rb_define_method(c_package_dep_spec, "in_repository", RUBY_FUNC_CAST(&package_dep_spec_in_repository_ptr), 0);
        rb_define_method(c_package_dep_spec, "from_repository", RUBY_FUNC_CAST(&package_dep_spec_from_repository_ptr), 0);
        rb_define_method(c_package_dep_spec, "installable_to_repository", RUBY_FUNC_CAST(&package_dep_spec_installable_to_repository), 0);
        rb_define_method(c_package_dep_spec, "installed_at_path", RUBY_FUNC_CAST(&package_dep_spec_installed_at_path), 0);
        rb_define_method(c_package_dep_spec, "installable_to_path", RUBY_FUNC_CAST(&package_dep_spec_installable_to_path), 0);
        rb_define_method(c_package_dep_spec, "version_requirements", RUBY_FUNC_CAST(&package_dep_spec_version_requirements_ptr), 0);
        rb_define_method(c_package_dep_spec, "version_requirements_mode", RUBY_FUNC_CAST(&package_dep_spec_version_requirements_mode), 0);
#ifdef CIARANM_REMOVED_THIS
        rb_define_method(c_package_dep_spec, "use_requirements", RUBY_FUNC_CAST(&package_dep_spec_use_requirements), 0);
#endif
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
         * Document-class: Paludis::DependenciesLabelsDepSpec
         *
         * A DependenciesLabelsDepSpec holds dependencies labels.
         */
        c_dependencies_labels_dep_spec = rb_define_class_under(paludis_module(), "DependenciesLabelsDepSpec", c_string_dep_spec);
        rb_define_singleton_method(c_dependencies_labels_dep_spec, "new", RUBY_FUNC_CAST(&DepSpecThings<DependenciesLabelsDepSpec>::dep_spec_new_0), 0);
        rb_define_method(c_dependencies_labels_dep_spec, "initialize", RUBY_FUNC_CAST(&dep_spec_init_0), 0);
        VALUE (* dependencies_labels_dep_spec_to_s) (VALUE) = &dep_spec_to_s<DependenciesLabelsDepSpec>;
        rb_define_method(c_dependencies_labels_dep_spec, "to_s", RUBY_FUNC_CAST(dependencies_labels_dep_spec_to_s), 0);
        rb_define_method(c_dependencies_labels_dep_spec, "labels", RUBY_FUNC_CAST(&dependencies_labels_dep_spec_labels), 0);

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
         * Document-class: Paludis::PlainTextLabelDepSpec
         *
         * A PlainTextLabelDepSpec holds a plain text label.
         */
        c_plain_text_label_dep_spec = rb_define_class_under(paludis_module(), "PlainTextLabelDepSpec", c_string_dep_spec);
        rb_define_singleton_method(c_plain_text_label_dep_spec, "new", RUBY_FUNC_CAST(&DepSpecThings<PlainTextDepSpec>::dep_spec_new_1), 1);
        rb_define_method(c_plain_text_label_dep_spec, "initialize", RUBY_FUNC_CAST(&dep_spec_init_1), 1);
        VALUE (* plain_text_dep_label_spec_to_s) (VALUE) = &dep_spec_to_s<PlainTextLabelDepSpec>;
        rb_define_method(c_plain_text_label_dep_spec, "to_s", RUBY_FUNC_CAST(plain_text_dep_label_spec_to_s), 0);

        /*
         * Document-class: Paludis::BlockDepSpec
         *
         * A BlockDepSpec represents a block on a package name (for example, 'app-editors/vim'), possibly with
         * associated version and SLOT restrictions.
         */
        c_block_dep_spec = rb_define_class_under(paludis_module(), "BlockDepSpec", c_string_dep_spec);
        rb_define_singleton_method(c_block_dep_spec, "new", RUBY_FUNC_CAST(&block_dep_spec_new), 2);
        rb_define_method(c_block_dep_spec, "initialize", RUBY_FUNC_CAST(&dep_spec_init_1), 2);
        rb_define_method(c_block_dep_spec, "blocking", RUBY_FUNC_CAST(&block_dep_spec_blocking), 0);
        VALUE (* block_dep_spec_to_s) (VALUE) = &dep_spec_to_s<BlockDepSpec>;
        rb_define_method(c_block_dep_spec, "to_s", RUBY_FUNC_CAST(block_dep_spec_to_s), 0);

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

        rb_define_module_function(paludis_module(), "parse_user_package_dep_spec", RUBY_FUNC_CAST(&paludis_parse_user_dep_spec), -1);

        /*
         * Document-class: Paludis::SlotRequirement
         *
         * A SlotRequirement
         */
        c_slot_requirement = rb_define_class_under(paludis_module(), "SlotRequirement", rb_cObject);
        rb_funcall(c_slot_requirement, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_method(c_slot_requirement, "as_string", RUBY_FUNC_CAST(&Common<std::shared_ptr<const SlotRequirement> >::to_s_via_ptr), 0);
        rb_define_method(c_slot_requirement, "to_s", RUBY_FUNC_CAST(&Common<std::shared_ptr<const SlotRequirement> >::to_s_via_ptr), 0);

        /*
         * Document-class: Paludis::ExactSlotRequirement
         *
         * An exact slot requiremet (:)
         */
        c_slot_exact_requirement = rb_define_class_under(paludis_module(), "SlotExactRequirement", c_slot_requirement);
        rb_define_method(c_slot_exact_requirement, "slot", RUBY_FUNC_CAST(&slot_exact_requirement_slot), 0);

        /*
         * Document-class: Paludis::SlotAnyLockedRequirement
         *
         * An any locked slot requiremet (:=)
         */
        c_slot_any_locked_requirement = rb_define_class_under(paludis_module(), "SlotAnyLockedRequirement", c_slot_requirement);

        /*
         * Document-class: Paludis::ExactSlotRequirement
         *
         * An any unlocked slot requiremet (:*)
         */
        c_slot_any_unlocked_requirement = rb_define_class_under(paludis_module(), "SlotAnyUnlockedRequirement", c_slot_requirement);
    }
}

std::shared_ptr<const PackageDepSpec>
paludis::ruby::value_to_package_dep_spec(VALUE v)
{
    if (rb_obj_is_kind_of(v, c_package_dep_spec))
    {
        std::shared_ptr<WrappedSpecBase> * v_ptr;
        Data_Get_Struct(v, std::shared_ptr<WrappedSpecBase>, v_ptr);
        return std::static_pointer_cast<const WrappedSpec<PackageDepSpec> >(*v_ptr)->spec();
    }
    else
    {
        rb_raise(rb_eTypeError, "Can't convert %s into PackageDepSpec", rb_obj_classname(v));
    }
}

std::shared_ptr<const DependenciesLabelsDepSpec>
paludis::ruby::value_to_dependencies_labels_dep_spec(VALUE v)
{
    if (rb_obj_is_kind_of(v, c_dependencies_labels_dep_spec))
    {
        std::shared_ptr<WrappedSpecBase> * v_ptr;
        Data_Get_Struct(v, std::shared_ptr<WrappedSpecBase>, v_ptr);
        return std::static_pointer_cast<const WrappedSpec<DependenciesLabelsDepSpec> >(*v_ptr)->spec();
    }
    else
    {
        rb_raise(rb_eTypeError, "Can't convert %s into DependenciesLabelsDepSpec", rb_obj_classname(v));
    }
}

std::shared_ptr<const DepSpec>
paludis::ruby::value_to_dep_spec(VALUE v)
{
    if (rb_obj_is_kind_of(v, c_dep_spec))
    {
        std::shared_ptr<WrappedSpecBase> * v_ptr;
        Data_Get_Struct(v, std::shared_ptr<WrappedSpecBase>, v_ptr);
        return (*v_ptr)->base_spec();
    }
    else
    {
        rb_raise(rb_eTypeError, "Can't convert %s into PackageDepSpec", rb_obj_classname(v));
    }
}

template <typename H_>
std::shared_ptr<const H_>
paludis::ruby::value_to_dep_tree(VALUE v)
{
    if (rb_obj_is_kind_of(v, c_all_dep_spec))
    {
        ValueToTree<H_> vtt(v);
        return vtt.result;
    }
    else if (rb_obj_is_kind_of(v, c_dep_spec))
    {
        ValueToTree<H_> vtt(v, std::make_shared<H_>(std::make_shared<AllDepSpec>()));
        return vtt.result;
    }
    else
    {
        rb_raise(rb_eTypeError, "Can't convert %s into DepSpec", rb_obj_classname(v));
    }
}

VALUE
paludis::ruby::package_dep_spec_to_value(const PackageDepSpec & p)
{
    std::shared_ptr<const WrappedSpecBase> * v_ptr(new std::shared_ptr<const WrappedSpecBase>(
                std::make_shared<WrappedSpec<PackageDepSpec>>(std::static_pointer_cast<PackageDepSpec>(p.clone()))));
    return Data_Wrap_Struct(c_package_dep_spec, 0, &Common<std::shared_ptr<const WrappedSpecBase> >::free, v_ptr);
}

template <typename T_>
VALUE
paludis::ruby::dep_tree_to_value(const std::shared_ptr<const T_> & m)
{
    try
    {
        TreeToValue v;
        m->top()->accept(v);
        std::shared_ptr<const WrappedSpecBase> * ptr(new std::shared_ptr<const WrappedSpecBase>(v.wrapped));
        return Data_Wrap_Struct(v.klass, 0, &Common<std::shared_ptr<const WrappedSpecBase> >::free, ptr);
    }
    catch (const std::exception & e)
    {
        exception_to_ruby_exception(e);
    }
}

VALUE *
paludis::ruby::dependencies_labels_dep_spec_value_ptr()
{
    return &c_dependencies_labels_dep_spec;
}

template VALUE paludis::ruby::dep_tree_to_value<SetSpecTree> (const std::shared_ptr<const SetSpecTree> &);
template VALUE paludis::ruby::dep_tree_to_value<DependencySpecTree> (const std::shared_ptr<const DependencySpecTree> &);
template VALUE paludis::ruby::dep_tree_to_value<FetchableURISpecTree> (const std::shared_ptr<const FetchableURISpecTree> &);
template VALUE paludis::ruby::dep_tree_to_value<SimpleURISpecTree> (const std::shared_ptr<const SimpleURISpecTree> &);
template VALUE paludis::ruby::dep_tree_to_value<PlainTextSpecTree> (const std::shared_ptr<const PlainTextSpecTree> &);
template VALUE paludis::ruby::dep_tree_to_value<RequiredUseSpecTree> (const std::shared_ptr<const RequiredUseSpecTree> &);
template VALUE paludis::ruby::dep_tree_to_value<ProvideSpecTree> (const std::shared_ptr<const ProvideSpecTree> &);
template VALUE paludis::ruby::dep_tree_to_value<LicenseSpecTree> (const std::shared_ptr<const LicenseSpecTree> &);

template std::shared_ptr<const SetSpecTree> paludis::ruby::value_to_dep_tree <SetSpecTree> (VALUE);

RegisterRubyClass::Register paludis_ruby_register_dep_spec PALUDIS_ATTRIBUTE((used))
    (&do_register_dep_spec);

