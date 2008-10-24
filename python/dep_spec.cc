/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Piotr Jaroszyński
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

#include <python/dep_spec.hh>
#include <python/paludis_python.hh>
#include <python/exception.hh>
#include <python/options.hh>
#include <python/nice_names-nn.hh>

#include <paludis/dep_tag.hh>
#include <paludis/dep_spec.hh>
#include <paludis/environment.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/version_requirements.hh>
#include <paludis/util/clone-impl.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/save.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/make_shared_ptr.hh>

#include <tr1/type_traits>
#include <list>

using namespace paludis;
using namespace paludis::python;
namespace bp = boost::python;

template class ConstVisitor<PythonDepSpecVisitorTypes>;
template class ConstAcceptInterface<PythonDepSpecVisitorTypes>;

template class ConstAcceptInterfaceVisitsThis<PythonDepSpecVisitorTypes, PythonAllDepSpec>;
template class ConstAcceptInterfaceVisitsThis<PythonDepSpecVisitorTypes, PythonAnyDepSpec>;
template class ConstAcceptInterfaceVisitsThis<PythonDepSpecVisitorTypes, PythonConditionalDepSpec>;
template class ConstAcceptInterfaceVisitsThis<PythonDepSpecVisitorTypes, PythonPackageDepSpec>;
template class ConstAcceptInterfaceVisitsThis<PythonDepSpecVisitorTypes, PythonBlockDepSpec>;
template class ConstAcceptInterfaceVisitsThis<PythonDepSpecVisitorTypes, PythonPlainTextDepSpec>;
template class ConstAcceptInterfaceVisitsThis<PythonDepSpecVisitorTypes, PythonSimpleURIDepSpec>;
template class ConstAcceptInterfaceVisitsThis<PythonDepSpecVisitorTypes, PythonFetchableURIDepSpec>;
template class ConstAcceptInterfaceVisitsThis<PythonDepSpecVisitorTypes, PythonLicenseDepSpec>;
template class ConstAcceptInterfaceVisitsThis<PythonDepSpecVisitorTypes, PythonURILabelsDepSpec>;
template class ConstAcceptInterfaceVisitsThis<PythonDepSpecVisitorTypes, PythonDependencyLabelsDepSpec>;
template class ConstAcceptInterfaceVisitsThis<PythonDepSpecVisitorTypes, PythonNamedSetDepSpec>;

template class Visits<const PythonAllDepSpec>;
template class Visits<const PythonAnyDepSpec>;
template class Visits<const PythonConditionalDepSpec>;
template class Visits<const PythonPackageDepSpec>;
template class Visits<const PythonBlockDepSpec>;
template class Visits<const PythonPlainTextDepSpec>;
template class Visits<const PythonSimpleURIDepSpec>;
template class Visits<const PythonFetchableURIDepSpec>;
template class Visits<const PythonLicenseDepSpec>;
template class Visits<const PythonURILabelsDepSpec>;
template class Visits<const PythonDependencyLabelsDepSpec>;
template class Visits<const PythonNamedSetDepSpec>;

template class WrappedForwardIterator<PythonCompositeDepSpec::ConstIteratorTag,
         const std::tr1::shared_ptr<const PythonDepSpec> >;

PythonDepSpec::PythonDepSpec()
{
}

PythonDepSpec::~PythonDepSpec()
{
}

const PythonConditionalDepSpec *
PythonDepSpec::as_conditional_dep_spec() const
{
    return 0;
}

const PythonPackageDepSpec *
PythonDepSpec::as_package_dep_spec() const
{
    return 0;
}

namespace paludis
{
    template<>
    struct Implementation<PythonCompositeDepSpec>
    {
        std::list<std::tr1::shared_ptr<const PythonDepSpec> > children;
    };

    template<>
    struct Implementation<PythonPackageDepSpec>
    {
        std::tr1::shared_ptr<const QualifiedPackageName> package_ptr;
        std::tr1::shared_ptr<const CategoryNamePart> category_name_part_ptr;
        std::tr1::shared_ptr<const PackageNamePart> package_name_part_ptr;
        std::tr1::shared_ptr<VersionRequirements> version_requirements;
        VersionRequirementsMode version_requirements_mode;
        std::tr1::shared_ptr<const SlotRequirement> slot;
        std::tr1::shared_ptr<const RepositoryName> in_repository;
        std::tr1::shared_ptr<const RepositoryName> from_repository;
        std::tr1::shared_ptr<const AdditionalPackageDepSpecRequirements> additional_requirements;
        std::tr1::shared_ptr<const DepTag> tag;
        const std::string str;

        Implementation(
                const std::tr1::shared_ptr<const QualifiedPackageName> & q,
                const std::tr1::shared_ptr<const CategoryNamePart> & c,
                const std::tr1::shared_ptr<const PackageNamePart> & p,
                const std::tr1::shared_ptr<VersionRequirements> & v,
                const VersionRequirementsMode m,
                const std::tr1::shared_ptr<const SlotRequirement> & s,
                const std::tr1::shared_ptr<const RepositoryName> & ri,
                const std::tr1::shared_ptr<const RepositoryName> & rf,
                const std::tr1::shared_ptr<const AdditionalPackageDepSpecRequirements> & u,
                const std::tr1::shared_ptr<const DepTag> & t,
                const std::string & st) :
            package_ptr(q),
            category_name_part_ptr(c),
            package_name_part_ptr(p),
            version_requirements(v),
            version_requirements_mode(m),
            slot(s),
            in_repository(ri),
            from_repository(rf),
            additional_requirements(u),
            tag(t),
            str(st)
        {
        }
    };
}

PythonCompositeDepSpec::PythonCompositeDepSpec() :
    PrivateImplementationPattern<PythonCompositeDepSpec>(new Implementation<PythonCompositeDepSpec>)
{
}

PythonCompositeDepSpec::~PythonCompositeDepSpec()
{
}

void
PythonCompositeDepSpec::add_child(const std::tr1::shared_ptr<const PythonDepSpec> c)
{
    _imp->children.push_back(c);
}

PythonCompositeDepSpec::ConstIterator
PythonCompositeDepSpec::begin() const
{
    return ConstIterator(_imp->children.begin());
}

PythonCompositeDepSpec::ConstIterator
PythonCompositeDepSpec::end() const
{
    return ConstIterator(_imp->children.end());
}

PythonAnyDepSpec::PythonAnyDepSpec()
{
}

PythonAnyDepSpec::PythonAnyDepSpec(const AnyDepSpec &)
{
}

PythonAllDepSpec::PythonAllDepSpec()
{
}

PythonAllDepSpec::PythonAllDepSpec(const AllDepSpec &)
{
}

PythonConditionalDepSpec::PythonConditionalDepSpec(const ConditionalDepSpec & d) :
    _data(d.data())
{
}

const PythonConditionalDepSpec *
PythonConditionalDepSpec::as_conditional_dep_spec() const
{
    return this;
}

bool
PythonConditionalDepSpec::condition_met() const
{
    return _data->condition_met();
}

bool
PythonConditionalDepSpec::condition_meetable() const
{
    return _data->condition_meetable();
}

const std::tr1::shared_ptr<const ConditionalDepSpecData>
PythonConditionalDepSpec::data() const
{
    return _data;
}

PythonStringDepSpec::PythonStringDepSpec(const std::string & s) :
    _str(s)
{
}

PythonStringDepSpec::PythonStringDepSpec(const StringDepSpec & d) :
    _str(d.text())
{
}

PythonStringDepSpec::~PythonStringDepSpec()
{
}

void
PythonStringDepSpec::set_text(const std::string & t)
{
    _str = t;
}

std::string
PythonStringDepSpec::text() const
{
    return _str;
}

template <typename T_>
std::tr1::shared_ptr<T_>
deep_copy(const std::tr1::shared_ptr<const T_> & x)
{
    if (x)
        return std::tr1::shared_ptr<T_>(new T_(*x));
    else
        return std::tr1::shared_ptr<T_>();
}

PythonPackageDepSpec::PythonPackageDepSpec(const PackageDepSpec & p) :
    PythonStringDepSpec(p.text()),
    PrivateImplementationPattern<PythonPackageDepSpec>(new Implementation<PythonPackageDepSpec>(
                deep_copy(p.package_ptr()),
                deep_copy(p.category_name_part_ptr()),
                deep_copy(p.package_name_part_ptr()),
                make_shared_ptr(new VersionRequirements),
                p.version_requirements_mode(),
                p.slot_requirement_ptr(),
                deep_copy(p.in_repository_ptr()),
                deep_copy(p.from_repository_ptr()),
                p.additional_requirements_ptr(),
                p.tag(),
                stringify(p)))
{
    if (p.version_requirements_ptr())
    {
        std::copy(p.version_requirements_ptr()->begin(), p.version_requirements_ptr()->end(),
            _imp->version_requirements->back_inserter());
    }
}

PythonPackageDepSpec::PythonPackageDepSpec(const PythonPackageDepSpec & p) :
    PythonStringDepSpec(p.text()),
    PrivateImplementationPattern<PythonPackageDepSpec>(new Implementation<PythonPackageDepSpec>(
                deep_copy(p.package_ptr()),
                deep_copy(p.category_name_part_ptr()),
                deep_copy(p.package_name_part_ptr()),
                make_shared_ptr(new VersionRequirements),
                p.version_requirements_mode(),
                p.slot_requirement_ptr(),
                deep_copy(p.in_repository_ptr()),
                deep_copy(p.from_repository_ptr()),
                p.additional_requirements_ptr(),
                p.tag(),
                p.py_str()))
{
    std::copy(p.version_requirements_ptr()->begin(), p.version_requirements_ptr()->end(),
            _imp->version_requirements->back_inserter());
}

PythonPackageDepSpec::~PythonPackageDepSpec()
{
}

PythonPackageDepSpec::operator PackageDepSpec() const
{
    PartiallyMadePackageDepSpec p;

    if (package_ptr())
        p.package(*package_ptr());

    if (category_name_part_ptr())
        p.category_name_part(*category_name_part_ptr());

    if (package_name_part_ptr())
        p.package_name_part(*package_name_part_ptr());

    p.version_requirements_mode(version_requirements_mode());

    if (slot_requirement_ptr())
        p.slot_requirement(slot_requirement_ptr());

    if (in_repository_ptr())
        p.in_repository(*in_repository_ptr());

    if (from_repository_ptr())
        p.from_repository(*from_repository_ptr());

    if (additional_requirements_ptr())
    {
        for (AdditionalPackageDepSpecRequirements::ConstIterator i(additional_requirements_ptr()->begin()),
                i_end(additional_requirements_ptr()->end()) ; i != i_end ; ++i)
            p.additional_requirement(*i);
    }

    if (version_requirements_ptr())
    {
        for (VersionRequirements::ConstIterator i(version_requirements_ptr()->begin()),
                i_end(version_requirements_ptr()->end()) ; i != i_end ; ++i)
            p.version_requirement(*i);
    }

    return p.to_package_dep_spec();
}


PythonPackageDepSpec::operator std::tr1::shared_ptr<PackageDepSpec>() const
{
    return make_shared_ptr(new PackageDepSpec(*this));
}

const PythonPackageDepSpec *
PythonPackageDepSpec::as_package_dep_spec() const
{
    return this;
}

const std::tr1::shared_ptr<const PythonPackageDepSpec>
PythonPackageDepSpec::without_additional_requirements() const
{
    PackageDepSpec p(*this);

    return make_shared_ptr(new PythonPackageDepSpec(*p.without_additional_requirements()));
}

std::tr1::shared_ptr<const QualifiedPackageName>
PythonPackageDepSpec::package_ptr() const
{
    return _imp->package_ptr;
}

std::tr1::shared_ptr<const PackageNamePart>
PythonPackageDepSpec::package_name_part_ptr() const
{
    return _imp->package_name_part_ptr;
}

std::tr1::shared_ptr<const CategoryNamePart>
PythonPackageDepSpec::category_name_part_ptr() const
{
    return _imp->category_name_part_ptr;
}

std::tr1::shared_ptr<const VersionRequirements>
PythonPackageDepSpec::version_requirements_ptr() const
{
    return _imp->version_requirements;
}

VersionRequirementsMode
PythonPackageDepSpec::version_requirements_mode() const
{
    return _imp->version_requirements_mode;
}

void
PythonPackageDepSpec::set_version_requirements_mode(const VersionRequirementsMode m)
{
    _imp->version_requirements_mode = m;
}

std::tr1::shared_ptr<const SlotRequirement>
PythonPackageDepSpec::slot_requirement_ptr() const
{
    return _imp->slot;
}

std::tr1::shared_ptr<const RepositoryName>
PythonPackageDepSpec::in_repository_ptr() const
{
    return _imp->in_repository;
}

std::tr1::shared_ptr<const RepositoryName>
PythonPackageDepSpec::from_repository_ptr() const
{
    return _imp->from_repository;
}

std::tr1::shared_ptr<const AdditionalPackageDepSpecRequirements>
PythonPackageDepSpec::additional_requirements_ptr() const
{
    return _imp->additional_requirements;
}

std::tr1::shared_ptr<const DepTag>
PythonPackageDepSpec::tag() const
{
    return _imp->tag;
}

std::string
PythonPackageDepSpec::py_str() const
{
    return _imp->str;
}

void
PythonPackageDepSpec::set_tag(const std::tr1::shared_ptr<const DepTag> & s)
{
    _imp->tag = s;
}

PythonPlainTextDepSpec::PythonPlainTextDepSpec(const std::string & s) :
    PythonStringDepSpec(s)
{
}

PythonPlainTextDepSpec::PythonPlainTextDepSpec(const PlainTextDepSpec & d) :
    PythonStringDepSpec(d.text())
{
}

PythonPlainTextDepSpec::PythonPlainTextDepSpec(const PythonPlainTextDepSpec & d) :
    PythonStringDepSpec(d.text())
{
}

PythonNamedSetDepSpec::PythonNamedSetDepSpec(const SetName & s) :
    PythonStringDepSpec(stringify(s)),
    _name(s)
{
}

const SetName
PythonNamedSetDepSpec::name() const
{
    return _name;
}

PythonNamedSetDepSpec::PythonNamedSetDepSpec(const NamedSetDepSpec & d) :
    PythonStringDepSpec(d.text()),
    _name(d.name())
{
}

PythonLicenseDepSpec::PythonLicenseDepSpec(const std::string & s) :
    PythonStringDepSpec(s)
{
}

PythonLicenseDepSpec::PythonLicenseDepSpec(const LicenseDepSpec & d) :
    PythonStringDepSpec(d.text())
{
}

PythonSimpleURIDepSpec::PythonSimpleURIDepSpec(const std::string & s) :
    PythonStringDepSpec(s)
{
}

PythonSimpleURIDepSpec::PythonSimpleURIDepSpec(const SimpleURIDepSpec & d) :
    PythonStringDepSpec(d.text())
{
}

PythonBlockDepSpec::PythonBlockDepSpec(std::tr1::shared_ptr<const PythonPackageDepSpec> & a) :
    PythonStringDepSpec("!" + a->text()),
    _spec(a)
{
}

PythonBlockDepSpec::PythonBlockDepSpec(const BlockDepSpec & d) :
    PythonStringDepSpec(d.text()),
    _spec(make_shared_ptr(new PythonPackageDepSpec(*d.blocked_spec())))
{
}

std::tr1::shared_ptr<const PythonPackageDepSpec>
PythonBlockDepSpec::blocked_spec() const
{
    return _spec;
}

PythonFetchableURIDepSpec::PythonFetchableURIDepSpec(const std::string & s) :
    PythonStringDepSpec(s)
{
}

PythonFetchableURIDepSpec::PythonFetchableURIDepSpec(const FetchableURIDepSpec & d) :
    PythonStringDepSpec(d.text())
{
}

std::string
PythonFetchableURIDepSpec::original_url() const
{
    std::string::size_type p(text().find(" -> "));
    if (std::string::npos == p)
        return text();
    else
        return text().substr(0, p);
}

std::string
PythonFetchableURIDepSpec::renamed_url_suffix() const
{
    std::string::size_type p(text().find(" -> "));
    if (std::string::npos == p)
        return "";
    else
        return text().substr(p + 4);
}

PythonURILabelsDepSpec::PythonURILabelsDepSpec(const std::string &)
{
}

PythonURILabelsDepSpec::PythonURILabelsDepSpec(const URILabelsDepSpec &)
{
}

PythonPlainTextLabelDepSpec::PythonPlainTextLabelDepSpec(const std::string & s) :
    PythonStringDepSpec(s)
{
}

PythonPlainTextLabelDepSpec::PythonPlainTextLabelDepSpec(const PlainTextLabelDepSpec & s) :
    PythonStringDepSpec(s.text())
{
}

PythonDependencyLabelsDepSpec::PythonDependencyLabelsDepSpec(const std::string &)
{
}

PythonDependencyLabelsDepSpec::PythonDependencyLabelsDepSpec(const DependencyLabelsDepSpec &)
{
}

SpecTreeToPython::SpecTreeToPython() :
    _current_parent(new PythonAllDepSpec())
{
}

SpecTreeToPython::~SpecTreeToPython()
{
}

void
SpecTreeToPython::visit_sequence(const AllDepSpec & d,
        GenericSpecTree::ConstSequenceIterator cur,
        GenericSpecTree::ConstSequenceIterator end)
{
    std::tr1::shared_ptr<PythonAllDepSpec> py_cds(new PythonAllDepSpec(d));
    _current_parent->add_child(py_cds);
    Save<std::tr1::shared_ptr<PythonCompositeDepSpec> > old_parent(&_current_parent, py_cds);
    std::for_each(cur, end, accept_visitor(*this));
}

void
SpecTreeToPython::visit_sequence(const AnyDepSpec & d,
        GenericSpecTree::ConstSequenceIterator cur,
        GenericSpecTree::ConstSequenceIterator end)
{
    std::tr1::shared_ptr<PythonAnyDepSpec> py_cds(new PythonAnyDepSpec(d));
    _current_parent->add_child(py_cds);
    Save<std::tr1::shared_ptr<PythonCompositeDepSpec> > old_parent(&_current_parent, py_cds);
    std::for_each(cur, end, accept_visitor(*this));
}

void
SpecTreeToPython::visit_sequence(const ConditionalDepSpec & d,
        GenericSpecTree::ConstSequenceIterator cur,
        GenericSpecTree::ConstSequenceIterator end)
{
    std::tr1::shared_ptr<PythonConditionalDepSpec> py_cds(new PythonConditionalDepSpec(d));
    _current_parent->add_child(py_cds);
    Save<std::tr1::shared_ptr<PythonCompositeDepSpec> > old_parent(&_current_parent, py_cds);
    std::for_each(cur, end, accept_visitor(*this));
}

void
SpecTreeToPython::visit_leaf(const PackageDepSpec & d)
{
    _current_parent->add_child(make_shared_ptr(new PythonPackageDepSpec(d)));
}

void
SpecTreeToPython::visit_leaf(const PlainTextDepSpec & d)
{
    _current_parent->add_child(make_shared_ptr(new PythonPlainTextDepSpec(d)));
}

void
SpecTreeToPython::visit_leaf(const NamedSetDepSpec & d)
{
    _current_parent->add_child(make_shared_ptr(new PythonNamedSetDepSpec(d)));
}

void
SpecTreeToPython::visit_leaf(const LicenseDepSpec & d)
{
    _current_parent->add_child(make_shared_ptr(new PythonLicenseDepSpec(d)));
}

void
SpecTreeToPython::visit_leaf(const SimpleURIDepSpec & d)
{
    _current_parent->add_child(make_shared_ptr(new PythonSimpleURIDepSpec(d)));
}

void
SpecTreeToPython::visit_leaf(const FetchableURIDepSpec & d)
{
    _current_parent->add_child(make_shared_ptr(new PythonFetchableURIDepSpec(d)));
}

void
SpecTreeToPython::visit_leaf(const BlockDepSpec & d)
{
    _current_parent->add_child(make_shared_ptr(new PythonBlockDepSpec(d)));
}

void
SpecTreeToPython::visit_leaf(const URILabelsDepSpec & d)
{
    _current_parent->add_child(make_shared_ptr(new PythonURILabelsDepSpec(d)));
}

void
SpecTreeToPython::visit_leaf(const PlainTextLabelDepSpec & d)
{
    _current_parent->add_child(make_shared_ptr(new PythonPlainTextLabelDepSpec(d)));
}

void
SpecTreeToPython::visit_leaf(const DependencyLabelsDepSpec & d)
{
    _current_parent->add_child(make_shared_ptr(new PythonDependencyLabelsDepSpec(d)));
}

const std::tr1::shared_ptr<const PythonDepSpec>
SpecTreeToPython::result() const
{
    return *_current_parent->begin();
}

template <typename H_>
struct AllowedTypes;

template<>
struct AllowedTypes<LicenseSpecTree>
{
    AllowedTypes(const AllDepSpec &) {};
    AllowedTypes(const AnyDepSpec &) {};
    AllowedTypes(const ConditionalDepSpec &) {};
    AllowedTypes(const LicenseDepSpec &) {};
};

template<>
struct AllowedTypes<FetchableURISpecTree>
{
    AllowedTypes(const AllDepSpec &) {};
    AllowedTypes(const ConditionalDepSpec &) {};
    AllowedTypes(const FetchableURISpecTree &) {};
    AllowedTypes(const URILabelsDepSpec &) {};
};

template<>
struct AllowedTypes<SimpleURISpecTree>
{
    AllowedTypes(const AllDepSpec &) {};
    AllowedTypes(const ConditionalDepSpec &) {};
    AllowedTypes(const SimpleURIDepSpec &) {};
};

template<>
struct AllowedTypes<ProvideSpecTree>
{
    AllowedTypes(const AllDepSpec &) {};
    AllowedTypes(const ConditionalDepSpec &) {};
    AllowedTypes(const PackageDepSpec &) {};
};

template<>
struct AllowedTypes<PlainTextSpecTree>
{
    AllowedTypes(const AllDepSpec &) {};
    AllowedTypes(const ConditionalDepSpec &) {};
    AllowedTypes(const PlainTextDepSpec &) {};
    AllowedTypes(const PlainTextLabelDepSpec &) {};
};

template<>
struct AllowedTypes<DependencySpecTree>
{
    AllowedTypes(const AllDepSpec &) {};
    AllowedTypes(const AnyDepSpec &) {};
    AllowedTypes(const ConditionalDepSpec &) {};
    AllowedTypes(const PackageDepSpec &) {};
    AllowedTypes(const BlockDepSpec &) {};
    AllowedTypes(const DependencyLabelsDepSpec &) {};
    AllowedTypes(const NamedSetDepSpec &) {};
};

template<>
struct AllowedTypes<SetSpecTree>
{
    AllowedTypes(const AllDepSpec &) {};
    AllowedTypes(const PackageDepSpec &) {};
    AllowedTypes(const NamedSetDepSpec &) {};
};

class PALUDIS_VISIBLE NotAllowedInThisHeirarchy :
    public Exception
{
    public:
        NotAllowedInThisHeirarchy(const std::string & msg) throw () :
            Exception(msg)
    {
    }
};

template <typename H_, typename D_, typename PyD_, bool>
struct Dispatcher;

template <typename H_, typename D_, typename PyD_>
struct Dispatcher<H_, D_, PyD_, true>
{
    static void do_dispatch(SpecTreeFromPython<H_> * v, const PyD_ & d)
    {
        v->real_visit(d);
    }
};

template <typename H_, typename D_, typename PyD_>
struct Dispatcher<H_, D_, PyD_, false>
{
    static void do_dispatch(SpecTreeFromPython<H_> *, const PyD_ &) PALUDIS_ATTRIBUTE((noreturn));
};

template <typename H_, typename D_, typename PyD_>
void
Dispatcher<H_, D_, PyD_, false>::do_dispatch(SpecTreeFromPython<H_> *, const PyD_ &)
{
    throw NotAllowedInThisHeirarchy(std::string("Spec parts of type '") + NiceNames<D_>::name +
            " are not allowed in a heirarchy of type '" + NiceNames<H_>::name + "'");
}

template <typename H_, typename D_, typename PyD_>
void dispatch(SpecTreeFromPython<H_> * const v, const PyD_ & d)
{
    Dispatcher<H_, D_, PyD_, std::tr1::is_convertible<D_, AllowedTypes<H_> >::value>::do_dispatch(v, d);
}

template <typename H_>
SpecTreeFromPython<H_>::SpecTreeFromPython() :
    _result(new ConstTreeSequence<H_, AllDepSpec>(std::tr1::shared_ptr<AllDepSpec>(new AllDepSpec()))),
    _add(std::tr1::bind(&ConstTreeSequence<H_, AllDepSpec>::add, _result, std::tr1::placeholders::_1))
{
}

template <typename H_>
SpecTreeFromPython<H_>::~SpecTreeFromPython()
{
}

template <typename H_>
void
SpecTreeFromPython<H_>::visit(const PythonAllDepSpec & d)
{
    dispatch<H_, AllDepSpec>(this, d);
}

template <typename H_>
void
SpecTreeFromPython<H_>::visit(const PythonAnyDepSpec & d)
{
    dispatch<H_, AnyDepSpec>(this, d);
}

template <typename H_>
void
SpecTreeFromPython<H_>::visit(const PythonConditionalDepSpec & d)
{
    dispatch<H_, ConditionalDepSpec>(this, d);
}

template <typename H_>
void
SpecTreeFromPython<H_>::visit(const PythonPackageDepSpec & d)
{
    dispatch<H_, PackageDepSpec>(this, d);
}

template <typename H_>
void
SpecTreeFromPython<H_>::visit(const PythonSimpleURIDepSpec & d)
{
    dispatch<H_, SimpleURIDepSpec>(this, d);
}

template <typename H_>
void
SpecTreeFromPython<H_>::visit(const PythonLicenseDepSpec & d)
{
    dispatch<H_, LicenseDepSpec>(this, d);
}

template <typename H_>
void
SpecTreeFromPython<H_>::visit(const PythonPlainTextDepSpec & d)
{
    dispatch<H_, PlainTextDepSpec>(this, d);
}

template <typename H_>
void
SpecTreeFromPython<H_>::visit(const PythonNamedSetDepSpec & d)
{
    dispatch<H_, NamedSetDepSpec>(this, d);
}

template <typename H_>
void
SpecTreeFromPython<H_>::visit(const PythonFetchableURIDepSpec & d)
{
    dispatch<H_, FetchableURIDepSpec>(this, d);
}

template <typename H_>
void
SpecTreeFromPython<H_>::visit(const PythonURILabelsDepSpec & d)
{
    dispatch<H_, URILabelsDepSpec>(this, d);
}

template <typename H_>
void
SpecTreeFromPython<H_>::visit(const PythonPlainTextLabelDepSpec & d)
{
    dispatch<H_, PlainTextLabelDepSpec>(this, d);
}

template <typename H_>
void
SpecTreeFromPython<H_>::visit(const PythonDependencyLabelsDepSpec & d)
{
    dispatch<H_, DependencyLabelsDepSpec>(this, d);
}

template <typename H_>
void
SpecTreeFromPython<H_>::visit(const PythonBlockDepSpec & d)
{
    dispatch<H_, BlockDepSpec>(this, d);
}

template <typename H_>
void
SpecTreeFromPython<H_>::real_visit(const PythonAllDepSpec & d)
{
    std::tr1::shared_ptr<ConstTreeSequence<H_, AllDepSpec> > cds(
                new ConstTreeSequence<H_, AllDepSpec>(make_shared_ptr(new AllDepSpec())));

    _add(cds);

    Save<std::tr1::function<void (std::tr1::shared_ptr<ConstAcceptInterface<H_> >)> > old_add(&_add,
            std::tr1::bind(&ConstTreeSequence<H_, AllDepSpec>::add, cds, std::tr1::placeholders::_1));
    std::for_each(IndirectIterator<PythonCompositeDepSpec::ConstIterator, const PythonDepSpec>(d.begin()),
            IndirectIterator<PythonCompositeDepSpec::ConstIterator, const PythonDepSpec>(d.end()),
            accept_visitor(*this));
}

template <typename H_>
void
SpecTreeFromPython<H_>::real_visit(const PythonAnyDepSpec & d)
{
    std::tr1::shared_ptr<ConstTreeSequence<H_, AnyDepSpec> > cds(
                new ConstTreeSequence<H_, AnyDepSpec>(make_shared_ptr(new AnyDepSpec())));

    _add(cds);

    Save<std::tr1::function<void (std::tr1::shared_ptr<ConstAcceptInterface<H_> >)> > old_add(&_add,
            std::tr1::bind(&ConstTreeSequence<H_, AnyDepSpec>::add, cds, std::tr1::placeholders::_1));
    std::for_each(IndirectIterator<PythonCompositeDepSpec::ConstIterator, const PythonDepSpec>(d.begin()),
            IndirectIterator<PythonCompositeDepSpec::ConstIterator, const PythonDepSpec>(d.end()),
            accept_visitor(*this));
}

template <typename H_>
void
SpecTreeFromPython<H_>::real_visit(const PythonConditionalDepSpec & d)
{
    std::tr1::shared_ptr<ConstTreeSequence<H_, ConditionalDepSpec> > cds(
                new ConstTreeSequence<H_, ConditionalDepSpec>(make_shared_ptr(
                        new ConditionalDepSpec(d.data()))));

    _add(cds);

    Save<std::tr1::function<void (std::tr1::shared_ptr<ConstAcceptInterface<H_> >)> > old_add(&_add,
            std::tr1::bind(&ConstTreeSequence<H_, ConditionalDepSpec>::add, cds, std::tr1::placeholders::_1));
    std::for_each(IndirectIterator<PythonCompositeDepSpec::ConstIterator, const PythonDepSpec>(d.begin()),
            IndirectIterator<PythonCompositeDepSpec::ConstIterator, const PythonDepSpec>(d.end()),
            accept_visitor(*this));
}

template <typename H_>
void
SpecTreeFromPython<H_>::real_visit(const PythonPackageDepSpec & d)
{
    _add(make_shared_ptr(new TreeLeaf<H_, PackageDepSpec>(make_shared_ptr(new PackageDepSpec(d)))));
}

template <typename H_>
void
SpecTreeFromPython<H_>::real_visit(const PythonLicenseDepSpec & d)
{
    _add(make_shared_ptr(new TreeLeaf<H_, LicenseDepSpec>(make_shared_ptr(new LicenseDepSpec(d.text())))));
}

template <typename H_>
void
SpecTreeFromPython<H_>::real_visit(const PythonSimpleURIDepSpec & d)
{
    _add(make_shared_ptr(new TreeLeaf<H_, SimpleURIDepSpec>(make_shared_ptr(new SimpleURIDepSpec(d.text())))));
}

template <typename H_>
void
SpecTreeFromPython<H_>::real_visit(const PythonPlainTextDepSpec & d)
{
    _add(make_shared_ptr(new TreeLeaf<H_, PlainTextDepSpec>(make_shared_ptr(new PlainTextDepSpec(d.text())))));
}

template <typename H_>
void
SpecTreeFromPython<H_>::real_visit(const PythonNamedSetDepSpec & d)
{
    _add(make_shared_ptr(new TreeLeaf<H_, NamedSetDepSpec>(make_shared_ptr(new NamedSetDepSpec(d.name())))));
}

template <typename H_>
void
SpecTreeFromPython<H_>::real_visit(const PythonFetchableURIDepSpec & d)
{
    _add(make_shared_ptr(new TreeLeaf<H_, FetchableURIDepSpec>(
                    make_shared_ptr(new FetchableURIDepSpec(d.text())))));
}

template <typename H_>
void
SpecTreeFromPython<H_>::real_visit(const PythonURILabelsDepSpec &)
{
    _add(make_shared_ptr(new TreeLeaf<H_, URILabelsDepSpec>(make_shared_ptr(new URILabelsDepSpec))));
}

template <typename H_>
void
SpecTreeFromPython<H_>::real_visit(const PythonPlainTextLabelDepSpec & s)
{
    _add(make_shared_ptr(new TreeLeaf<H_, PlainTextLabelDepSpec>(make_shared_ptr(new PlainTextLabelDepSpec(s.text())))));
}

template <typename H_>
void
SpecTreeFromPython<H_>::real_visit(const PythonDependencyLabelsDepSpec &)
{
    _add(make_shared_ptr(new TreeLeaf<H_, DependencyLabelsDepSpec>(make_shared_ptr(
                        new DependencyLabelsDepSpec))));
}

template <typename H_>
void
SpecTreeFromPython<H_>::real_visit(const PythonBlockDepSpec & d)
{
    _add(make_shared_ptr(new TreeLeaf<H_, BlockDepSpec>(make_shared_ptr(
                        new BlockDepSpec(make_shared_ptr(new PackageDepSpec(*d.blocked_spec())))))));
}

template <typename H_>
std::tr1::shared_ptr<typename H_::ConstItem>
SpecTreeFromPython<H_>::result() const
{
    return _result;
}

template <typename T_>
struct RegisterSpecTreeToPython
{
    RegisterSpecTreeToPython()
    {
        bp::to_python_converter<std::tr1::shared_ptr<typename T_::ConstItem>, RegisterSpecTreeToPython<T_> >();
    }

    static PyObject *
    convert(const std::tr1::shared_ptr<typename T_::ConstItem> & n)
    {
        SpecTreeToPython v;
        n->accept(v);
        return bp::incref(bp::object(v.result()).ptr());
    }
};

template <typename From_, typename To_>
struct RegisterDepSpecToPython
{
    RegisterDepSpecToPython()
    {
        bp::to_python_converter<From_, RegisterDepSpecToPython<From_, To_> >();
        bp::to_python_converter<std::tr1::shared_ptr<const From_>, RegisterDepSpecToPython<From_, To_> >();
    }

    static PyObject *
    convert(const From_ & d)
    {
        To_ pyd(d);
        return bp::incref(bp::object(pyd).ptr());
    }

    static PyObject *
    convert(const std::tr1::shared_ptr<const From_> & d)
    {
        To_ pyd(*d);
        return bp::incref(bp::object(pyd).ptr());
    }
};

template <typename H_>
struct RegisterSpecTreeSharedPtrFromPython
{
    RegisterSpecTreeSharedPtrFromPython()
    {
        bp::converter::registry::push_back(&convertible, &construct,
                boost::python::type_id<std::tr1::shared_ptr<const typename H_::ConstItem> >());
    }

    static void *
    convertible(PyObject * obj_ptr)
    {
        if (bp::extract<PythonDepSpec *>(obj_ptr).check())
            return obj_ptr;
        else
            return 0;
    }

    static void
    construct(PyObject * obj_ptr, bp::converter::rvalue_from_python_stage1_data * data)
    {
        typedef bp::converter::rvalue_from_python_storage<std::tr1::shared_ptr<const typename H_::ConstItem> > Storage;
        void * storage = reinterpret_cast<Storage *>(data)->storage.bytes;

        SpecTreeFromPython<H_> v;
        PythonDepSpec * p = bp::extract<PythonDepSpec *>(obj_ptr);
        p->accept(v);

        new (storage) std::tr1::shared_ptr<const typename H_::ConstItem>(v.result());
        data->convertible = storage;
    }
};

void expose_dep_spec()
{
    /**
     * Exceptions
     */
    ExceptionRegister::get_instance()->add_exception<PackageDepSpecError>
        ("PackageDepSpecError", "BaseException",
         "Thrown if an invalid package dep spec specification is encountered.");

    ExceptionRegister::get_instance()->add_exception<NotAllowedInThisHeirarchy>
        ("NotAllowedInThisHeirarchy", "BaseException",
         "Thrown if a spec part not suitable for a particular heirarchy is present.");

    ExceptionRegister::get_instance()->add_exception<GotASetNotAPackageDepSpec>
        ("GotASetNotAPackageDepSpec", "BaseException",
         "Thrown by parse_user_package_dep_spec if options includes THROW_IF_SET"
         " and we're given a set.");

    /**
     * Enums
     */
    enum_auto("UserPackageDepSpecOption", last_updso,
            "Options for parse_user_package_dep_spec.");

    /**
     * Options
     */
    class_options<UserPackageDepSpecOptions>("UserPackageDepSpecOptions", "UserPackageDepSpecOption",
            "Options for parse_user_package_dep_spec.");

    RegisterSpecTreeToPython<DependencySpecTree>();
    RegisterSpecTreeToPython<ProvideSpecTree>();
    RegisterSpecTreeToPython<PlainTextSpecTree>();
    RegisterSpecTreeToPython<FetchableURISpecTree>();
    RegisterSpecTreeToPython<SimpleURISpecTree>();
    RegisterSpecTreeToPython<LicenseSpecTree>();
    RegisterSpecTreeToPython<SetSpecTree>();

    RegisterSpecTreeSharedPtrFromPython<DependencySpecTree>();
    RegisterSpecTreeSharedPtrFromPython<ProvideSpecTree>();
    RegisterSpecTreeSharedPtrFromPython<PlainTextSpecTree>();
    RegisterSpecTreeSharedPtrFromPython<FetchableURISpecTree>();
    RegisterSpecTreeSharedPtrFromPython<SimpleURISpecTree>();
    RegisterSpecTreeSharedPtrFromPython<LicenseSpecTree>();
    RegisterSpecTreeSharedPtrFromPython<SetSpecTree>();

    /**
     * DepSpec
     */
    register_shared_ptrs_to_python<PythonDepSpec>();
    bp::class_<PythonDepSpec, boost::noncopyable>
        (
         "DepSpec",
         "Base class for a dependency spec.",
         bp::no_init
        )
        .def("as_conditional_dep_spec", &PythonDepSpec::as_conditional_dep_spec,
                bp::return_value_policy<bp::reference_existing_object>(),
                "as_conditional_dep_spec() -> ConditionalDepSpec\n"
                "Return us as a ConditionalDepSpec, or None if we are not a ConditionalDepSpec."
            )

        .def("as_package_dep_spec", &PythonDepSpec::as_package_dep_spec,
                bp::return_value_policy<bp::reference_existing_object>(),
                "as_package_dep_spec() -> PackageDepSpec\n"
                "Return us as a PackageDepSpec, or None if we are not a PackageDepSpec."
            )
        ;

    /**
     * CompositeDepSpec
     */
    register_shared_ptrs_to_python<PythonCompositeDepSpec>();
    bp::class_<PythonCompositeDepSpec, bp::bases<PythonDepSpec>, boost::noncopyable>
        (
         "CompositeDepSpec",
         "Iterable class for dependency specs that have a number of child dependency specs.",
         bp::no_init
        )
        .def("add_child", &PythonCompositeDepSpec::add_child)

        .def("__iter__", bp::range(&PythonCompositeDepSpec::begin, &PythonCompositeDepSpec::end))
        ;

    /**
     * AnyDepSpec
     */
    bp::class_<PythonAnyDepSpec, bp::bases<PythonCompositeDepSpec>, boost::noncopyable>
        (
         "AnyDepSpec",
         "Represents a \"|| ( )\" dependency block.",
         bp::init<>("__init__()")
        );

    /**
     * AllDepSpec
     */
    bp::class_<PythonAllDepSpec, bp::bases<PythonCompositeDepSpec>, boost::noncopyable>
        (
         "AllDepSpec",
         "Represents a ( first second third ) or top level group of dependency specs.",
         bp::init<>("__init__()")
        );

    /**
     * ConditionalDepSpec
     */
    bp::class_<PythonConditionalDepSpec, bp::bases<PythonCompositeDepSpec>, boost::noncopyable>
        (
         "ConditionalDepSpec",
         "Represents a use? ( ) dependency spec.",
         bp::no_init
        )
        .def("condition_met", &PythonConditionalDepSpec::condition_met,
                "condition_met() -> bool\n"
                "Is our condition met?"
            )
        .def("condition_meetable", &PythonConditionalDepSpec::condition_meetable,
                "condition_met() -> bool\n"
                "Is our condition meetable?"
            )
        ;

    /**
     * StringDepSpec
     */
    bp::class_<PythonStringDepSpec, bp::bases<PythonDepSpec>, boost::noncopyable>
        (
         "StringDepSpec",
         "A StringDepSpec represents a non-composite dep spec with an associated piece of text.",
         bp::no_init
        )
        .add_property("text", &PythonStringDepSpec::text,
                "[ro] string\n"
                "Our text."
                )
        ;

    /**
     * PackageDepSpec
     */

    bp::def("parse_user_package_dep_spec", &parse_user_package_dep_spec,
            (bp::arg("str"), bp::arg("env"), bp::arg("options"), bp::arg("filter")=filter::All()),
            "parse_user_package_dep_spec(str, Environment, options=UserPackageDepSpecOptions(), Filter)"
            " -> PackageDepSpec\n"
            "Create a PackageDepSpec from user input."
           );

    bp::implicitly_convertible<PythonPackageDepSpec, PackageDepSpec>();
    bp::implicitly_convertible<PythonPackageDepSpec, std::tr1::shared_ptr<PackageDepSpec> >();
    bp::implicitly_convertible<std::tr1::shared_ptr<PackageDepSpec>, std::tr1::shared_ptr<const PackageDepSpec> >();
    RegisterDepSpecToPython<PackageDepSpec, PythonPackageDepSpec>();

    bp::class_<PythonPackageDepSpec, std::tr1::shared_ptr<const PythonPackageDepSpec>, bp::bases<PythonStringDepSpec> >
        (
         "PackageDepSpec",
         "A PackageDepSpec represents a package name (for example, 'app-editors/vim'),"
         " possibly with associated version and SLOT restrictions.",
         bp::no_init
        )

        .add_property("package", &PythonPackageDepSpec::package_ptr,
                "[ro] QualifiedPackageName\n"
                "Qualified package name."
                )

        .add_property("package_name_part", &PythonPackageDepSpec::package_name_part_ptr,
                "[ro] PackageNamePart\n"
                "Package name part (may be None)"
                )

        .add_property("category_name_part", &PythonPackageDepSpec::category_name_part_ptr,
                "[ro] CategoryNamePart\n"
                "Category name part (may be None)."
                )

        .add_property("version_requirements", &PythonPackageDepSpec::version_requirements_ptr,
                "[ro] VersionRequirements\n"
                "Version requirements (may be None)."
                )

        .add_property("version_requirements_mode", &PythonPackageDepSpec::version_requirements_mode,
                "[ro] VersionRequirementsMode\n"
                "Version requirements mode."
                )

#if 0
        .add_property("slot", &PythonPackageDepSpec::slot_ptr,
                "[ro] SlotName\n"
                "Slot name (may be None)."
                )
#endif

        .add_property("in_repository", &PythonPackageDepSpec::in_repository_ptr,
                "[ro] RepositoryName\n"
                "In repository name (may be None)."

                )

        .add_property("from_repository", &PythonPackageDepSpec::from_repository_ptr,
                "[ro] RepositoryName\n"
                "From repository name (may be None)."
                )

#if 0
        .add_property("use_requirements", &PythonPackageDepSpec::use_requirements_ptr,
                "[ro] UseRequirements\n"
                "Use requirements (may be None)."
                )
#endif

        .def("without_additional_requirements", &PythonPackageDepSpec::without_additional_requirements,
                "without_additional_requirements() -> PackageDepSpec\n"
                "Fetch a copy of ourself without additional requirements."
            )

        .def("__str__", &PythonPackageDepSpec::py_str)
        ;

    /**
     * PlainTextDepSpec
     */
    RegisterDepSpecToPython<PlainTextDepSpec, PythonPlainTextDepSpec>();
    bp::class_<PythonPlainTextDepSpec, bp::bases<PythonStringDepSpec> >
        (
         "PlainTextDepSpec",
         "A PlainTextDepSpec represents a plain text entry (for example, a RESTRICT keyword).",
         bp::init<const std::string &>("__init__(string)")
        )
        .def("__str__", &PythonPlainTextDepSpec::text)
        ;

    /**
     * NamedSetDepSpec
     */
    RegisterDepSpecToPython<NamedSetDepSpec, PythonNamedSetDepSpec>();
    bp::class_<PythonNamedSetDepSpec, bp::bases<PythonStringDepSpec>, boost::noncopyable>
        (
         "NamedSetDepSpec",
         "A NamedSetDepSpec represents a named package set.",
         bp::init<const SetName &>("__init__(SetName)")
        )
        .def("__str__", &PythonNamedSetDepSpec::text)
        .add_property("name", &PythonNamedSetDepSpec::name,
                "[ro] SetName"
                )
        ;

    /**
     * LicenseDepSpec
     */
    RegisterDepSpecToPython<LicenseDepSpec, PythonLicenseDepSpec>();
    bp::class_<PythonLicenseDepSpec, bp::bases<PythonStringDepSpec>, boost::noncopyable>
        (
         "LicenseDepSpec",
         "A LicenseDepSpec represents a license.",
         bp::init<const std::string &>("__init__(string)")
        )
        .def("__str__", &PythonLicenseDepSpec::text)
        ;

    /**
     * SimpleURIDepSpec
     */
    RegisterDepSpecToPython<SimpleURIDepSpec, PythonSimpleURIDepSpec>();
    bp::class_<PythonSimpleURIDepSpec, bp::bases<PythonStringDepSpec>, boost::noncopyable>
        (
         "SimpleURIDepSpec",
         "A SimpleURIDepSpec represents a simple URI.",
         bp::init<const std::string &>("__init__(string)")
        )
        .def("__str__", &PythonSimpleURIDepSpec::text)
        ;

    /**
     * FetchableURIDepSpec
     */
    RegisterDepSpecToPython<FetchableURIDepSpec, PythonFetchableURIDepSpec>();
    bp::class_<PythonFetchableURIDepSpec, bp::bases<PythonStringDepSpec>, boost::noncopyable>
        (
         "FetchableURIDepSpec",
         "A FetchableURIDepSpec represents a fetchable URI part.",
         bp::init<const std::string &>("__init__(str)")
        )
        .add_property("original_url", &PythonFetchableURIDepSpec::original_url,
                "[ro] str"
                )

        .add_property("renamed_url_suffix", &PythonFetchableURIDepSpec::renamed_url_suffix,
                "[ro] str"
                )
        ;

    /**
     * URILabelsDepSpec
     */
    RegisterDepSpecToPython<URILabelsDepSpec, PythonURILabelsDepSpec>();
    bp::class_<PythonURILabelsDepSpec, bp::bases<PythonDepSpec>, boost::noncopyable>
        (
         "URILabelsDepSpec",
         "A URILabelsDepSpec represents a URI label.",
         bp::init<const std::string &>("__init__(str)")
        )
        ;

    /**
     * DependencyLabelsDepSpec
     */
    RegisterDepSpecToPython<DependencyLabelsDepSpec, PythonDependencyLabelsDepSpec>();
    bp::class_<PythonDependencyLabelsDepSpec, bp::bases<PythonDepSpec>, boost::noncopyable>
        (
         "DependencyLabelsDepSpec",
         "A DependencyLabelsDepSpec represents a dependency label.",
         bp::init<const std::string &>("__init__(str)")
        )
        ;

    /**
     * BlockDepSpec
     */
    RegisterDepSpecToPython<BlockDepSpec, PythonBlockDepSpec>();
    bp::class_<PythonBlockDepSpec, bp::bases<PythonStringDepSpec>, boost::noncopyable >
        (
         "BlockDepSpec",
         "A BlockDepSpec represents a block on a package name (for example, 'app-editors/vim'), \n"
         "possibly with associated version and SLOT restrictions.",
         bp::init<std::tr1::shared_ptr<const PackageDepSpec> >("__init__(PackageDepSpec)")
        )
        .add_property("blocked_spec", &PythonBlockDepSpec::blocked_spec,
                "[ro] PackageDepSpec\n"
                "The spec we're blocking."
                )

        //Work around epydoc bug
        .add_property("text", &PythonBlockDepSpec::text,
                "[ro] string\n"
                "Our text."
                )
        ;
}

