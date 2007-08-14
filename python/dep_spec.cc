/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Piotr Jaroszyński <peper@gentoo.org>
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

#include "dep_spec.hh"
#include <python/paludis_python.hh>
#include <python/exception.hh>

#include <paludis/dep_tag.hh>
#include <paludis/version_requirements.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/save.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/tr1_type_traits.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>
#include <libwrapiter/libwrapiter_output_iterator.hh>

#include <list>

using namespace paludis;
using namespace paludis::python;
namespace bp = boost::python;

template class ConstVisitor<PythonDepSpecVisitorTypes>;
template class ConstAcceptInterface<PythonDepSpecVisitorTypes>;

template class ConstAcceptInterfaceVisitsThis<PythonDepSpecVisitorTypes, PythonAllDepSpec>;
template class ConstAcceptInterfaceVisitsThis<PythonDepSpecVisitorTypes, PythonAnyDepSpec>;
template class ConstAcceptInterfaceVisitsThis<PythonDepSpecVisitorTypes, PythonUseDepSpec>;
template class ConstAcceptInterfaceVisitsThis<PythonDepSpecVisitorTypes, PythonPackageDepSpec>;
template class ConstAcceptInterfaceVisitsThis<PythonDepSpecVisitorTypes, PythonBlockDepSpec>;
template class ConstAcceptInterfaceVisitsThis<PythonDepSpecVisitorTypes, PythonPlainTextDepSpec>;
template class ConstAcceptInterfaceVisitsThis<PythonDepSpecVisitorTypes, PythonURIDepSpec>;
template class ConstAcceptInterfaceVisitsThis<PythonDepSpecVisitorTypes, PythonURILabelsDepSpec>;

template class Visits<const PythonAllDepSpec>;
template class Visits<const PythonAnyDepSpec>;
template class Visits<const PythonUseDepSpec>;
template class Visits<const PythonPackageDepSpec>;
template class Visits<const PythonBlockDepSpec>;
template class Visits<const PythonPlainTextDepSpec>;
template class Visits<const PythonURIDepSpec>;
template class Visits<const PythonURILabelsDepSpec>;

PythonDepSpec::PythonDepSpec()
{
}

PythonDepSpec::~PythonDepSpec()
{
}

const PythonUseDepSpec *
PythonDepSpec::as_use_dep_spec() const
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
        std::list<tr1::shared_ptr<const PythonDepSpec> > children;
    };

    template<>
    struct Implementation<PythonPackageDepSpec>
    {
        tr1::shared_ptr<const QualifiedPackageName> package_ptr;
        tr1::shared_ptr<const CategoryNamePart> category_name_part_ptr;
        tr1::shared_ptr<const PackageNamePart> package_name_part_ptr;
        tr1::shared_ptr<VersionRequirements> version_requirements;
        VersionRequirementsMode version_requirements_mode;
        tr1::shared_ptr<const SlotName> slot;
        tr1::shared_ptr<const RepositoryName> repository;
        tr1::shared_ptr<const UseRequirements> use_requirements;
        tr1::shared_ptr<const DepTag> tag;
        const std::string str;

        Implementation(
                const tr1::shared_ptr<const QualifiedPackageName> & q,
                const tr1::shared_ptr<const CategoryNamePart> & c,
                const tr1::shared_ptr<const PackageNamePart> & p,
                const tr1::shared_ptr<VersionRequirements> & v,
                const VersionRequirementsMode m,
                const tr1::shared_ptr<const SlotName> & s,
                const tr1::shared_ptr<const RepositoryName> & r,
                const tr1::shared_ptr<const UseRequirements> & u,
                const tr1::shared_ptr<const DepTag> & t,
                const std::string & st) :
            package_ptr(q),
            category_name_part_ptr(c),
            package_name_part_ptr(p),
            version_requirements(v),
            version_requirements_mode(m),
            slot(s),
            repository(r),
            use_requirements(u),
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
PythonCompositeDepSpec::add_child(const tr1::shared_ptr<const PythonDepSpec> c)
{
    _imp->children.push_back(c);
}

PythonCompositeDepSpec::Iterator
PythonCompositeDepSpec::begin() const
{
    return Iterator(_imp->children.begin());
}

PythonCompositeDepSpec::Iterator
PythonCompositeDepSpec::end() const
{
    return Iterator(_imp->children.end());
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

PythonUseDepSpec::PythonUseDepSpec(const UseFlagName & our_flag, bool is_inverse) :
    _flag(our_flag),
    _inverse(is_inverse)
{
}

PythonUseDepSpec::PythonUseDepSpec(const UseDepSpec & d) :
    _flag(d.flag()),
    _inverse(d.inverse())
{
}

const PythonUseDepSpec *
PythonUseDepSpec::as_use_dep_spec() const
{
    return this;
}

UseFlagName
PythonUseDepSpec::flag() const
{
    return _flag;
}

bool
PythonUseDepSpec::inverse() const
{
    return _inverse;
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
tr1::shared_ptr<T_>
deep_copy(const tr1::shared_ptr<const T_> & x)
{
    if (x)
        return tr1::shared_ptr<T_>(new T_(*x));
    else
        return tr1::shared_ptr<T_>();
}

PythonPackageDepSpec::PythonPackageDepSpec(const PackageDepSpec & p) :
    PythonStringDepSpec(p.text()),
    PrivateImplementationPattern<PythonPackageDepSpec>(new Implementation<PythonPackageDepSpec>(
                deep_copy(p.package_ptr()),
                deep_copy(p.category_name_part_ptr()),
                deep_copy(p.package_name_part_ptr()),
                tr1::shared_ptr<VersionRequirements>(new VersionRequirements),
                p.version_requirements_mode(),
                deep_copy(p.slot_ptr()),
                deep_copy(p.repository_ptr()),
                deep_copy(p.use_requirements_ptr()),
                tr1::shared_ptr<const DepTag>(p.tag()),
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
                tr1::shared_ptr<VersionRequirements>(new VersionRequirements),
                p.version_requirements_mode(),
                deep_copy(p.slot_ptr()),
                deep_copy(p.repository_ptr()),
                deep_copy(p.use_requirements_ptr()),
                tr1::shared_ptr<const DepTag>(p.tag()),
                p.py_str()))
{
    std::copy(p.version_requirements_ptr()->begin(), p.version_requirements_ptr()->end(),
            _imp->version_requirements->back_inserter());
}

PythonPackageDepSpec::~PythonPackageDepSpec()
{
}

tr1::shared_ptr<const PythonPackageDepSpec>
PythonPackageDepSpec::make_from_string(const std::string & ss, const PackageDepSpecParseMode p)
{
    return tr1::shared_ptr<PythonPackageDepSpec>(new PythonPackageDepSpec(PackageDepSpec(ss, p)));
}

const PythonPackageDepSpec *
PythonPackageDepSpec::as_package_dep_spec() const
{
    return this;
}

const tr1::shared_ptr<const PythonPackageDepSpec>
PythonPackageDepSpec::without_use_requirements() const
{
    PackageDepSpec p(
            deep_copy(package_ptr()),
            deep_copy(category_name_part_ptr()),
            deep_copy(package_name_part_ptr()),
            tr1::shared_ptr<VersionRequirements>(new VersionRequirements),
            version_requirements_mode(),
            deep_copy(slot_ptr()),
            deep_copy(repository_ptr()),
            deep_copy(use_requirements_ptr()),
            tr1::shared_ptr<const DepTag>(tag())
            );

    if (version_requirements_ptr())
    {
        std::copy(version_requirements_ptr()->begin(), version_requirements_ptr()->end(),
                p.version_requirements_ptr()->back_inserter());
    }

    return tr1::shared_ptr<PythonPackageDepSpec>(new PythonPackageDepSpec(*p.without_use_requirements()));
}

tr1::shared_ptr<const QualifiedPackageName>
PythonPackageDepSpec::package_ptr() const
{
    return _imp->package_ptr;
}

tr1::shared_ptr<const PackageNamePart>
PythonPackageDepSpec::package_name_part_ptr() const
{
    return _imp->package_name_part_ptr;
}

tr1::shared_ptr<const CategoryNamePart>
PythonPackageDepSpec::category_name_part_ptr() const
{
    return _imp->category_name_part_ptr;
}

tr1::shared_ptr<const VersionRequirements>
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

tr1::shared_ptr<const SlotName>
PythonPackageDepSpec::slot_ptr() const
{
    return _imp->slot;
}

tr1::shared_ptr<const RepositoryName>
PythonPackageDepSpec::repository_ptr() const
{
    return _imp->repository;
}

tr1::shared_ptr<const UseRequirements>
PythonPackageDepSpec::use_requirements_ptr() const
{
    return _imp->use_requirements;
}

tr1::shared_ptr<const DepTag>
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
PythonPackageDepSpec::set_tag(const tr1::shared_ptr<const DepTag> & s)
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

PythonBlockDepSpec::PythonBlockDepSpec(tr1::shared_ptr<const PythonPackageDepSpec> & a) :
    PythonStringDepSpec("!" + a->text()),
    _spec(a)
{
}

PythonBlockDepSpec::PythonBlockDepSpec(const BlockDepSpec & d) :
    PythonStringDepSpec(d.text()),
    _spec(tr1::shared_ptr<const PythonPackageDepSpec>(new PythonPackageDepSpec(*d.blocked_spec())))
{
}

tr1::shared_ptr<const PythonPackageDepSpec>
PythonBlockDepSpec::blocked_spec() const
{
    return _spec;
}

PythonURIDepSpec::PythonURIDepSpec(const std::string & s) :
    PythonStringDepSpec(s)
{
}

PythonURIDepSpec::PythonURIDepSpec(const URIDepSpec & d) :
    PythonStringDepSpec(d.text())
{
}

std::string
PythonURIDepSpec::original_url() const
{
    std::string::size_type p(text().find(" -> "));
    if (std::string::npos == p)
        return text();
    else
        return text().substr(0, p);
}

std::string
PythonURIDepSpec::renamed_url_suffix() const
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

PythonURILabelsDepSpec::PythonURILabelsDepSpec(const LabelsDepSpec<URILabelVisitorTypes> &)
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
    tr1::shared_ptr<PythonAllDepSpec> py_cds(new PythonAllDepSpec(d));
    _current_parent->add_child(py_cds);
    Save<tr1::shared_ptr<PythonCompositeDepSpec> > old_parent(&_current_parent, py_cds);
    std::for_each(cur, end, accept_visitor(*this));
}

void
SpecTreeToPython::visit_sequence(const AnyDepSpec & d,
        GenericSpecTree::ConstSequenceIterator cur,
        GenericSpecTree::ConstSequenceIterator end)
{
    tr1::shared_ptr<PythonAnyDepSpec> py_cds(new PythonAnyDepSpec(d));
    _current_parent->add_child(py_cds);
    Save<tr1::shared_ptr<PythonCompositeDepSpec> > old_parent(&_current_parent, py_cds);
    std::for_each(cur, end, accept_visitor(*this));
}

void
SpecTreeToPython::visit_sequence(const UseDepSpec & d,
        GenericSpecTree::ConstSequenceIterator cur,
        GenericSpecTree::ConstSequenceIterator end)
{
    tr1::shared_ptr<PythonUseDepSpec> py_cds(new PythonUseDepSpec(d));
    _current_parent->add_child(py_cds);
    Save<tr1::shared_ptr<PythonCompositeDepSpec> > old_parent(&_current_parent, py_cds);
    std::for_each(cur, end, accept_visitor(*this));
}

void
SpecTreeToPython::visit_leaf(const PackageDepSpec & d)
{
    _current_parent->add_child(tr1::shared_ptr<PythonPackageDepSpec>(new PythonPackageDepSpec(d)));
}

void
SpecTreeToPython::visit_leaf(const PlainTextDepSpec & d)
{
    _current_parent->add_child(tr1::shared_ptr<PythonPlainTextDepSpec>(new PythonPlainTextDepSpec(d)));
}

void
SpecTreeToPython::visit_leaf(const URIDepSpec & d)
{
    _current_parent->add_child(tr1::shared_ptr<PythonURIDepSpec>(new PythonURIDepSpec(d)));
}

void
SpecTreeToPython::visit_leaf(const BlockDepSpec & d)
{
    _current_parent->add_child(tr1::shared_ptr<PythonBlockDepSpec>(new PythonBlockDepSpec(d)));
}

void
SpecTreeToPython::visit_leaf(const LabelsDepSpec<URILabelVisitorTypes> & d)
{
    _current_parent->add_child(tr1::shared_ptr<PythonURILabelsDepSpec>(new PythonURILabelsDepSpec(d)));
}

const tr1::shared_ptr<const PythonDepSpec>
SpecTreeToPython::result() const
{
    return *_current_parent->begin();
}

PackageDepSpec *
package_dep_spec_from_python(const PythonPackageDepSpec & p)
{
    PackageDepSpec * result(new PackageDepSpec(
                deep_copy(p.package_ptr()),
                deep_copy(p.category_name_part_ptr()),
                deep_copy(p.package_name_part_ptr()),
                tr1::shared_ptr<VersionRequirements>(new VersionRequirements),
                p.version_requirements_mode(),
                deep_copy(p.slot_ptr()),
                deep_copy(p.repository_ptr()),
                deep_copy(p.use_requirements_ptr()),
                tr1::shared_ptr<const DepTag>(p.tag())
                ));
    if (p.version_requirements_ptr())
    {
        std::copy(p.version_requirements_ptr()->begin(), p.version_requirements_ptr()->end(),
                result->version_requirements_ptr()->back_inserter());
    }
    return result;
}

template <typename H_>
struct AllowedTypes;

template<>
struct AllowedTypes<LicenseSpecTree>
{
    AllowedTypes(const AllDepSpec &) {};
    AllowedTypes(const AnyDepSpec &) {};
    AllowedTypes(const UseDepSpec &) {};
    AllowedTypes(const PlainTextDepSpec &) {};
};

template<>
struct AllowedTypes<URISpecTree>
{
    AllowedTypes(const AllDepSpec &) {};
    AllowedTypes(const UseDepSpec &) {};
    AllowedTypes(const URIDepSpec &) {};
    AllowedTypes(const LabelsDepSpec<URILabelVisitorTypes> &) {};
};

template<>
struct AllowedTypes<ProvideSpecTree>
{
    AllowedTypes(const AllDepSpec &) {};
    AllowedTypes(const UseDepSpec &) {};
    AllowedTypes(const PackageDepSpec &) {};
};

template<>
struct AllowedTypes<RestrictSpecTree>
{
    AllowedTypes(const AllDepSpec &) {};
    AllowedTypes(const UseDepSpec &) {};
    AllowedTypes(const PlainTextDepSpec &) {};
};

template<>
struct AllowedTypes<DependencySpecTree>
{
    AllowedTypes(const AllDepSpec &) {};
    AllowedTypes(const AnyDepSpec &) {};
    AllowedTypes(const UseDepSpec &) {};
    AllowedTypes(const PackageDepSpec &) {};
    AllowedTypes(const BlockDepSpec &) {};
};

template<>
struct AllowedTypes<SetSpecTree>
{
    AllowedTypes(const AllDepSpec &) {};
    AllowedTypes(const PackageDepSpec &) {};
};


template <typename>
struct NiceClassNames;

template<>
struct NiceClassNames<DepSpec>
{
        static const char * name;
};
const char * NiceClassNames<DepSpec>::name = "DepSpec";

template<>
struct NiceClassNames<AllDepSpec>
{
        static const char * name;
};
const char * NiceClassNames<AllDepSpec>::name = "AllDepSpec";

template<>
struct NiceClassNames<AnyDepSpec>
{
        static const char * name;
};
const char * NiceClassNames<AnyDepSpec>::name = "AnyDepSpec";

template<>
struct NiceClassNames<UseDepSpec>
{
        static const char * name;
};
const char * NiceClassNames<UseDepSpec>::name = "UseDepSpec";

template<>
struct NiceClassNames<StringDepSpec>
{
        static const char * name;
};
const char * NiceClassNames<StringDepSpec>::name = "StringDepSpec";

template<>
struct NiceClassNames<PlainTextDepSpec>
{
        static const char * name;
};
const char * NiceClassNames<PlainTextDepSpec>::name = "PlainTextDepSpec";

template<>
struct NiceClassNames<PackageDepSpec>
{
        static const char * name;
};
const char * NiceClassNames<PackageDepSpec>::name = "PackageDepSpec";

template<>
struct NiceClassNames<URIDepSpec>
{
        static const char * name;
};
const char * NiceClassNames<URIDepSpec>::name = "URIDepSpec";

template<>
struct NiceClassNames<LabelsDepSpec<URILabelVisitorTypes> >
{
        static const char * name;
};
const char * NiceClassNames<LabelsDepSpec<URILabelVisitorTypes> >::name = "URILabelsDepSpec";

template<>
struct NiceClassNames<BlockDepSpec>
{
        static const char * name;
};
const char * NiceClassNames<BlockDepSpec>::name = "BlockDepSpec";

template<>
struct NiceClassNames<GenericSpecTree>
{
        static const char * name;
};
const char * NiceClassNames<GenericSpecTree>::name = "GenericSpecTree";

template<>
struct NiceClassNames<LicenseSpecTree>
{
        static const char * name;
};
const char * NiceClassNames<LicenseSpecTree>::name = "LicenseSpecTree";

template<>
struct NiceClassNames<URISpecTree>
{
        static const char * name;
};
const char * NiceClassNames<URISpecTree>::name = "URISpecTree";

template<>
struct NiceClassNames<FlattenableSpecTree>
{
        static const char * name;
};
const char * NiceClassNames<FlattenableSpecTree>::name = "FlattenableSpecTree";

template<>
struct NiceClassNames<ProvideSpecTree>
{
        static const char * name;
};
const char * NiceClassNames<ProvideSpecTree>::name = "ProvideSpecTree";

template<>
struct NiceClassNames<RestrictSpecTree>
{
        static const char * name;
};
const char * NiceClassNames<RestrictSpecTree>::name = "RestrictSpecTree";

template<>
struct NiceClassNames<DependencySpecTree>
{
        static const char * name;
};
const char * NiceClassNames<DependencySpecTree>::name = "DependencySpecTree";

template<>
struct NiceClassNames<SetSpecTree>
{
        static const char * name;
};
const char * NiceClassNames<SetSpecTree>::name = "SetSpecTree";


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
    throw NotAllowedInThisHeirarchy(std::string("Spec parts of type '") + NiceClassNames<D_>::name +
            " are not allowed in a heirarchy of type '" + NiceClassNames<H_>::name + "'");
}


template <typename H_, typename D_, typename PyD_>
void dispatch(SpecTreeFromPython<H_> * const v, const PyD_ & d)
{
    Dispatcher<H_, D_, PyD_, tr1::is_convertible<D_, AllowedTypes<H_> >::value>::do_dispatch(v, d);
}

template <typename H_>
SpecTreeFromPython<H_>::SpecTreeFromPython() :
    _result(new ConstTreeSequence<H_, AllDepSpec>(tr1::shared_ptr<AllDepSpec>(new AllDepSpec()))),
    _add(tr1::bind(&ConstTreeSequence<H_, AllDepSpec>::add, _result, tr1::placeholders::_1))
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
SpecTreeFromPython<H_>::visit(const PythonUseDepSpec & d)
{
    dispatch<H_, UseDepSpec>(this, d);
}

template <typename H_>
void
SpecTreeFromPython<H_>::visit(const PythonPackageDepSpec & d)
{
    dispatch<H_, PackageDepSpec>(this, d);
}

template <typename H_>
void
SpecTreeFromPython<H_>::visit(const PythonPlainTextDepSpec & d)
{
    dispatch<H_, PlainTextDepSpec>(this, d);
}

template <typename H_>
void
SpecTreeFromPython<H_>::visit(const PythonURIDepSpec & d)
{
    dispatch<H_, URIDepSpec>(this, d);
}

template <typename H_>
void
SpecTreeFromPython<H_>::visit(const PythonURILabelsDepSpec & d)
{
    dispatch<H_, LabelsDepSpec<URILabelVisitorTypes> >(this, d);
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
    tr1::shared_ptr<ConstTreeSequence<H_, AllDepSpec> > cds(tr1::shared_ptr<ConstTreeSequence<H_, AllDepSpec> >(
                new ConstTreeSequence<H_, AllDepSpec>(tr1::shared_ptr<AllDepSpec>(new AllDepSpec()))));

    _add(cds);

    Save<tr1::function<void (tr1::shared_ptr<ConstAcceptInterface<H_> >)> > old_add(&_add,
            tr1::bind(&ConstTreeSequence<H_, AllDepSpec>::add, cds, tr1::placeholders::_1));
    std::for_each(IndirectIterator<PythonCompositeDepSpec::Iterator, const PythonDepSpec>(d.begin()),
            IndirectIterator<PythonCompositeDepSpec::Iterator, const PythonDepSpec>(d.end()),
            accept_visitor(*this));
}

template <typename H_>
void
SpecTreeFromPython<H_>::real_visit(const PythonAnyDepSpec & d)
{
    tr1::shared_ptr<ConstTreeSequence<H_, AnyDepSpec> > cds(tr1::shared_ptr<ConstTreeSequence<H_, AnyDepSpec> >(
                new ConstTreeSequence<H_, AnyDepSpec>(tr1::shared_ptr<AnyDepSpec>(new AnyDepSpec()))));

    _add(cds);

    Save<tr1::function<void (tr1::shared_ptr<ConstAcceptInterface<H_> >)> > old_add(&_add,
            tr1::bind(&ConstTreeSequence<H_, AnyDepSpec>::add, cds, tr1::placeholders::_1));
    std::for_each(IndirectIterator<PythonCompositeDepSpec::Iterator, const PythonDepSpec>(d.begin()),
            IndirectIterator<PythonCompositeDepSpec::Iterator, const PythonDepSpec>(d.end()),
            accept_visitor(*this));
}

template <typename H_>
void
SpecTreeFromPython<H_>::real_visit(const PythonUseDepSpec & d)
{
    tr1::shared_ptr<ConstTreeSequence<H_, UseDepSpec> > cds(tr1::shared_ptr<ConstTreeSequence<H_, UseDepSpec> >(
                new ConstTreeSequence<H_, UseDepSpec>(tr1::shared_ptr<UseDepSpec>(
                        new UseDepSpec(d.flag(), d.inverse())))));

    _add(cds);

    Save<tr1::function<void (tr1::shared_ptr<ConstAcceptInterface<H_> >)> > old_add(&_add,
            tr1::bind(&ConstTreeSequence<H_, UseDepSpec>::add, cds, tr1::placeholders::_1));
    std::for_each(IndirectIterator<PythonCompositeDepSpec::Iterator, const PythonDepSpec>(d.begin()),
            IndirectIterator<PythonCompositeDepSpec::Iterator, const PythonDepSpec>(d.end()),
            accept_visitor(*this));
}

template <typename H_>
void
SpecTreeFromPython<H_>::real_visit(const PythonPackageDepSpec & d)
{
    _add(tr1::shared_ptr<TreeLeaf<H_, PackageDepSpec> >(
                new TreeLeaf<H_, PackageDepSpec>(tr1::shared_ptr<PackageDepSpec>(
                        package_dep_spec_from_python(d)))));
}

template <typename H_>
void
SpecTreeFromPython<H_>::real_visit(const PythonPlainTextDepSpec & d)
{
    _add(tr1::shared_ptr<TreeLeaf<H_, PlainTextDepSpec> >(
               new TreeLeaf<H_, PlainTextDepSpec>(tr1::shared_ptr<PlainTextDepSpec>(
                       new PlainTextDepSpec(d.text())))));
}

template <typename H_>
void
SpecTreeFromPython<H_>::real_visit(const PythonURIDepSpec & d)
{
    _add(tr1::shared_ptr<TreeLeaf<H_, URIDepSpec> >(
                new TreeLeaf<H_, URIDepSpec>(tr1::shared_ptr<URIDepSpec>(
                        new URIDepSpec(d.text())))));
}

template <typename H_>
void
SpecTreeFromPython<H_>::real_visit(const PythonURILabelsDepSpec &)
{
    _add(tr1::shared_ptr<TreeLeaf<H_, LabelsDepSpec<URILabelVisitorTypes> > >(
                new TreeLeaf<H_, LabelsDepSpec<URILabelVisitorTypes> >(tr1::shared_ptr<LabelsDepSpec<URILabelVisitorTypes> >(
                        new LabelsDepSpec<URILabelVisitorTypes>))));
}

template <typename H_>
void
SpecTreeFromPython<H_>::real_visit(const PythonBlockDepSpec & d)
{
    _add(tr1::shared_ptr<TreeLeaf<H_, BlockDepSpec> >(
                new TreeLeaf<H_, BlockDepSpec>(tr1::shared_ptr<BlockDepSpec>(
                        new BlockDepSpec(tr1::shared_ptr<PackageDepSpec>(
                                package_dep_spec_from_python(*d.blocked_spec())))))));
}

template <typename H_>
tr1::shared_ptr<typename H_::ConstItem>
SpecTreeFromPython<H_>::result() const
{
    return _result;
}

template <typename N_>
struct tree_to_python
{
    static PyObject *
    convert(const N_ & n)
    {
        SpecTreeToPython v;
        n->accept(v);
        return bp::incref(bp::object(v.result()).ptr());
    }
};

template <typename T_>
void register_tree_to_python()
{
    bp::to_python_converter<tr1::shared_ptr<typename T_::ConstItem>,
            tree_to_python<tr1::shared_ptr<typename T_::ConstItem> > >();
}

struct sp_package_dep_spec_to_python
{
    static PyObject *
    convert(const tr1::shared_ptr<const PackageDepSpec> & d)
    {
        PythonPackageDepSpec pyd(*d);
        return bp::incref(bp::object(pyd).ptr());
    }
};

void register_sp_package_dep_spec_to_python()
{
    bp::to_python_converter<tr1::shared_ptr<const PackageDepSpec>, sp_package_dep_spec_to_python>();
}


template <typename H_>
struct RegisterSpecTreeSPTRFromPython
{
    RegisterSpecTreeSPTRFromPython()
    {
        bp::converter::registry::push_back(&convertible, &construct,
                boost::python::type_id<tr1::shared_ptr<const typename H_::ConstItem> >());
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
        typedef bp::converter::rvalue_from_python_storage<tr1::shared_ptr<const typename H_::ConstItem> > Storage;
        void * storage = reinterpret_cast<Storage *>(data)->storage.bytes;

        SpecTreeFromPython<H_> v;
        PythonDepSpec * p = bp::extract<PythonDepSpec *>(obj_ptr);
        p->accept(v);

        new (storage) tr1::shared_ptr<const typename H_::ConstItem>(v.result());
        data->convertible = storage;
    }
};

struct RegisterPackageDepSpecFromPython
{
    RegisterPackageDepSpecFromPython()
    {
        bp::converter::registry::push_back(&convertible, &construct,
                boost::python::type_id<PackageDepSpec>());
    }

    static void *
    convertible(PyObject * obj_ptr)
    {
        if (bp::extract<PythonPackageDepSpec *>(obj_ptr).check())
            return obj_ptr;
        else
            return 0;
    }

    static void
    construct(PyObject * obj_ptr, bp::converter::rvalue_from_python_stage1_data * data)
    {
        typedef bp::converter::rvalue_from_python_storage<PackageDepSpec> Storage;
        void * storage = reinterpret_cast<Storage *>(data)->storage.bytes;
        PythonPackageDepSpec p = bp::extract<PythonPackageDepSpec>(obj_ptr);
        new (storage) PackageDepSpec(
                    deep_copy(p.package_ptr()),
                    deep_copy(p.category_name_part_ptr()),
                    deep_copy(p.package_name_part_ptr()),
                    tr1::shared_ptr<VersionRequirements>(new VersionRequirements),
                    p.version_requirements_mode(),
                    deep_copy(p.slot_ptr()),
                    deep_copy(p.repository_ptr()),
                    deep_copy(p.use_requirements_ptr()),
                    tr1::shared_ptr<const DepTag>(p.tag())
                );

        if (p.version_requirements_ptr())
        {
            std::copy(p.version_requirements_ptr()->begin(), p.version_requirements_ptr()->end(),
                    reinterpret_cast<PackageDepSpec *>(storage)->version_requirements_ptr()->back_inserter());
        }
        data->convertible = storage;
    }
};

struct RegisterPackageDepSpecSPTRFromPython
{
    RegisterPackageDepSpecSPTRFromPython()
    {
        bp::converter::registry::push_back(&convertible, &construct,
                boost::python::type_id<tr1::shared_ptr<const PackageDepSpec> >());
    }

    static void *
    convertible(PyObject * obj_ptr)
    {
        if (bp::extract<PythonPackageDepSpec *>(obj_ptr).check())
            return obj_ptr;
        else
            return 0;
    }

    static void
    construct(PyObject * obj_ptr, bp::converter::rvalue_from_python_stage1_data * data)
    {
        typedef bp::converter::rvalue_from_python_storage<tr1::shared_ptr<PackageDepSpec> > Storage;
        void * storage = reinterpret_cast<Storage *>(data)->storage.bytes;

        PythonPackageDepSpec p = bp::extract<PythonPackageDepSpec>(obj_ptr);

        new (storage) tr1::shared_ptr<PackageDepSpec>(new PackageDepSpec(
                    deep_copy(p.package_ptr()),
                    deep_copy(p.category_name_part_ptr()),
                    deep_copy(p.package_name_part_ptr()),
                    tr1::shared_ptr<VersionRequirements>(new VersionRequirements),
                    p.version_requirements_mode(),
                    deep_copy(p.slot_ptr()),
                    deep_copy(p.repository_ptr()),
                    deep_copy(p.use_requirements_ptr()),
                    tr1::shared_ptr<const DepTag>(p.tag())
                    ));
        if (p.version_requirements_ptr())
        {
            std::copy(p.version_requirements_ptr()->begin(), p.version_requirements_ptr()->end(),
                    (*reinterpret_cast<tr1::shared_ptr<PackageDepSpec> *>(storage))->version_requirements_ptr()->back_inserter());
        }
        data->convertible = storage;
    }
};

void expose_dep_spec() PALUDIS_VISIBLE
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

    /**
     * Enums
     */
    enum_auto("PackageDepSpecParseMode", last_pds_pm,
            "How to parse a PackageDepSpec string.");

    register_tree_to_python<DependencySpecTree>();
    register_tree_to_python<ProvideSpecTree>();
    register_tree_to_python<RestrictSpecTree>();
    register_tree_to_python<URISpecTree>();
    register_tree_to_python<LicenseSpecTree>();
    register_tree_to_python<SetSpecTree>();

    RegisterSpecTreeSPTRFromPython<DependencySpecTree>();
    RegisterSpecTreeSPTRFromPython<ProvideSpecTree>();
    RegisterSpecTreeSPTRFromPython<RestrictSpecTree>();
    RegisterSpecTreeSPTRFromPython<URISpecTree>();
    RegisterSpecTreeSPTRFromPython<LicenseSpecTree>();
    RegisterSpecTreeSPTRFromPython<SetSpecTree>();

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
        .def("as_use_dep_spec", &PythonDepSpec::as_use_dep_spec,
                bp::return_value_policy<bp::reference_existing_object>(),
                "as_use_dep_spec() -> UseDepSpec\n"
                "Return us as a UseDepSpec, or None if we are not a UseDepSpec."
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
        .def("__iter__", bp::range(&PythonCompositeDepSpec::begin, &PythonCompositeDepSpec::end))
        ;

    /**
     * AnyDepSpec
     */
    bp::class_<PythonAnyDepSpec, bp::bases<PythonCompositeDepSpec>, boost::noncopyable>
        (
         "AnyDepSpec",
         "Represents a \"|| ( )\" dependency block.",
         bp::no_init
        );

    /**
     * AllDepSpec
     */
    bp::class_<PythonAllDepSpec, bp::bases<PythonCompositeDepSpec>, boost::noncopyable>
        (
         "AllDepSpec",
         "Represents a ( first second third ) or top level group of dependency specs.",
         bp::no_init
        );

    /**
     * UseDepSpec
     */
    bp::class_<PythonUseDepSpec, bp::bases<PythonCompositeDepSpec>, boost::noncopyable>
        (
         "UseDepSpec",
         "Represents a use? ( ) dependency spec.",
         bp::no_init
        )
        .add_property("flag", &UseDepSpec::flag,
                "[ro] UseFlagName\n"
                "Our use flag name."
                )

        .add_property("inverse", &UseDepSpec::inverse,
                "[ro] bool\n"
                "Are we a ! flag?"
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
     * UseRequirements
     */
    bp::to_python_converter<std::pair<const UseFlagName, UseFlagState>,
        pair_to_tuple<const UseFlagName, UseFlagState> >();
    register_shared_ptrs_to_python<UseRequirements>();
    bp::class_<UseRequirements>
        (
         "UseRequirements",
         "A selection of USE flag requirements.",
         bp::no_init
        )
        .def("state", &UseRequirements::state,
                "state(UseFlagName) -> UseFlagState\n"
                "What state is desired for a particular use flag?"
            )

        .def("__iter__", bp::range(&UseRequirements::begin, &UseRequirements::end))
        ;

    /**
     * PackageDepSpec
     */
    RegisterPackageDepSpecFromPython();
    RegisterPackageDepSpecSPTRFromPython();

    bp::implicitly_convertible<PackageDepSpec, PythonPackageDepSpec>();
    register_sp_package_dep_spec_to_python();

    bp::class_<PythonPackageDepSpec, tr1::shared_ptr<const PythonPackageDepSpec>, bp::bases<PythonStringDepSpec> >
        (
         "PackageDepSpec",
         "A PackageDepSpec represents a package name (for example, 'app-editors/vim'),"
         " possibly with associated version and SLOT restrictions.",
         bp::no_init
        )
        .def("__init__", bp::make_constructor(&PythonPackageDepSpec::make_from_string),
                "__init__(string, PackageDepSpecParseMode)"
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

        .add_property("slot", &PythonPackageDepSpec::slot_ptr,
                "[ro] SlotName\n"
                "Slot name (may be None)."
                )

        .add_property("repository", &PythonPackageDepSpec::repository_ptr,
                "[ro] RepositoryName\n"
                "Repository name (may be None)."
                )

        .add_property("use_requirements", &PythonPackageDepSpec::use_requirements_ptr,
                "[ro] UseRequirements\n"
                "Use requirements (may be None)."
                )

        .def("without_use_requirements", &PythonPackageDepSpec::without_use_requirements,
                "without_use_requirements() -> PackageDepSpec\n"
                "Fetch a copy of ourself without the USE requirements."
            )

        .def("__str__", &PythonPackageDepSpec::py_str)
        ;

    /**
     * PlainTextDepSpec
     */
    bp::class_<PythonPlainTextDepSpec, bp::bases<PythonStringDepSpec>, boost::noncopyable>
        (
         "PlainTextDepSpec",
         "A PlainTextDepSpec represents a plain text entry (for example, a URI in SRC_URI).",
         bp::init<const std::string &>("__init__(string)")
        )
        .def("__str__", &PythonPlainTextDepSpec::text)
        ;

    /**
     * URIDepSpec
     */
    bp::class_<PythonURIDepSpec, bp::bases<PythonStringDepSpec>, boost::noncopyable>
        (
         "URIDepSpec",
         "A URIDepSpec represents a URI part.",
         bp::init<const std::string &>("__init__(str)")
        )
        .add_property("original_url", &PythonURIDepSpec::original_url,
                "[ro] str"
                )

        .add_property("renamed_url_suffix", &PythonURIDepSpec::renamed_url_suffix,
                "[ro] str"
                )
        ;

    /**
     * URILabelsDepSpec
     */
    bp::class_<PythonURILabelsDepSpec, bp::bases<PythonDepSpec>, boost::noncopyable>
        (
         "URILabelsDepSpec",
         "A URILabelsDepSpec represents a URI label.",
         bp::init<const std::string &>("__init__(str)")
        )
        ;

    /**
     * BlockDepSpec
     */
    bp::class_<PythonBlockDepSpec, bp::bases<PythonStringDepSpec>, boost::noncopyable >
        (
         "BlockDepSpec",
         "A BlockDepSpec represents a block on a package name (for example, 'app-editors/vim'), \n"
         "possibly with associated version and SLOT restrictions.",
         bp::init<tr1::shared_ptr<const PackageDepSpec> >("__init__(PackageDepSpec)")
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

