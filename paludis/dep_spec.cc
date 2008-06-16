/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008 Ciaran McCreesh
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

#include <paludis/dep_spec.hh>
#include <paludis/environment.hh>
#include <paludis/version_operator.hh>
#include <paludis/version_spec.hh>
#include <paludis/version_requirements.hh>
#include <paludis/util/clone-impl.hh>
#include <paludis/util/log.hh>
#include <paludis/util/join.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/sequence-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/iterator_funcs.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/options.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/visitor-impl.hh>
#include <tr1/functional>
#include <algorithm>
#include <list>
#include <map>

using namespace paludis;

DepSpec::DepSpec()
{
}

DepSpec::~DepSpec()
{
}

const ConditionalDepSpec *
DepSpec::as_conditional_dep_spec() const
{
    return 0;
}

const PackageDepSpec *
DepSpec::as_package_dep_spec() const
{
    return 0;
}

AnyDepSpec::AnyDepSpec()
{
}

std::tr1::shared_ptr<DepSpec>
AnyDepSpec::clone() const
{
    return std::tr1::shared_ptr<AnyDepSpec>(new AnyDepSpec());
}

AllDepSpec::AllDepSpec()
{
}

std::tr1::shared_ptr<DepSpec>
AllDepSpec::clone() const
{
    return std::tr1::shared_ptr<AllDepSpec>(new AllDepSpec());
}

namespace paludis
{
    template <>
    struct Implementation<ConditionalDepSpec>
    {
        const std::tr1::shared_ptr<const ConditionalDepSpecData> data;
        Mutex mutex;
        bool added_keys;

        Implementation(const std::tr1::shared_ptr<const ConditionalDepSpecData> & d) :
            data(d),
            added_keys(false)
        {
        }
    };
}

ConditionalDepSpec::ConditionalDepSpec(const std::tr1::shared_ptr<const ConditionalDepSpecData> & d) :
    PrivateImplementationPattern<ConditionalDepSpec>(new Implementation<ConditionalDepSpec>(d)),
    _imp(PrivateImplementationPattern<ConditionalDepSpec>::_imp)
{
}

namespace
{
    template <void (ConditionalDepSpec::* f_) () const>
    const ConditionalDepSpec & horrible_hack_to_force_key_copy(const ConditionalDepSpec & spec)
    {
        (spec.*f_)();
        return spec;
    }
}

ConditionalDepSpec::ConditionalDepSpec(const ConditionalDepSpec & other) :
    Cloneable<DepSpec>(),
    DepSpec(),
    PrivateImplementationPattern<ConditionalDepSpec>(new Implementation<ConditionalDepSpec>(other._imp->data)),
    MetadataKeyHolder(),
    CloneUsingThis<DepSpec, ConditionalDepSpec>(other),
    _imp(PrivateImplementationPattern<ConditionalDepSpec>::_imp)
{
}

ConditionalDepSpec::~ConditionalDepSpec()
{
}

void
ConditionalDepSpec::need_keys_added() const
{
    Lock l(_imp->mutex);
    if (! _imp->added_keys)
    {
        _imp->added_keys = true;
        using namespace std::tr1::placeholders;
        std::for_each(_imp->data->begin_metadata(), _imp->data->end_metadata(),
                std::tr1::bind(&ConditionalDepSpec::add_metadata_key, this, _1));
    }
}

void
ConditionalDepSpec::clear_metadata_keys() const
{
    Lock l(_imp->mutex);
    _imp->added_keys = false;
    MetadataKeyHolder::clear_metadata_keys();
}

bool
ConditionalDepSpec::condition_met() const
{
    return _imp->data->condition_met();
}

bool
ConditionalDepSpec::condition_meetable() const
{
    return _imp->data->condition_meetable();
}

const std::tr1::shared_ptr<const ConditionalDepSpecData>
ConditionalDepSpec::data() const
{
    return _imp->data;
}

const ConditionalDepSpec *
ConditionalDepSpec::as_conditional_dep_spec() const
{
    return this;
}

std::string
ConditionalDepSpec::_as_string() const
{
    return _imp->data->as_string();
}

ConditionalDepSpecData::~ConditionalDepSpecData()
{
}

std::string
StringDepSpec::text() const
{
    return _str;
}

NamedSetDepSpec::NamedSetDepSpec(const SetName & n) :
    StringDepSpec(stringify(n)),
    _name(n)
{
}

const SetName
NamedSetDepSpec::name() const
{
    return _name;
}

std::tr1::shared_ptr<DepSpec>
NamedSetDepSpec::clone() const
{
    return std::tr1::shared_ptr<NamedSetDepSpec>(new NamedSetDepSpec(_name));
}

const PackageDepSpec *
PackageDepSpec::as_package_dep_spec() const
{
    return this;
}

BlockDepSpec::BlockDepSpec(std::tr1::shared_ptr<const PackageDepSpec> a) :
    StringDepSpec("!" + a->text()),
    _spec(a)
{
}

std::ostream &
paludis::operator<< (std::ostream & s, const PlainTextDepSpec & a)
{
    s << a.text();
    return s;
}

std::ostream &
paludis::operator<< (std::ostream & s, const LicenseDepSpec & a)
{
    s << a.text();
    return s;
}

std::ostream &
paludis::operator<< (std::ostream & s, const NamedSetDepSpec & a)
{
    s << a.name();
    return s;
}

std::ostream &
paludis::operator<< (std::ostream & s, const BlockDepSpec & a)
{
    s << "!" << *a.blocked_spec();
    return s;
}

std::ostream &
paludis::operator<< (std::ostream & s, const FetchableURIDepSpec & p)
{
    if (! p.renamed_url_suffix().empty())
        s << p.original_url() << " -> " << p.renamed_url_suffix();
    else
        s << p.original_url();

    return s;
}

std::ostream &
paludis::operator<< (std::ostream & s, const SimpleURIDepSpec & p)
{
    s << p.text();
    return s;
}

std::ostream &
paludis::operator<< (std::ostream & s, const PackageDepSpec & a)
{
    s << a._as_string();
    return s;
}

std::ostream &
paludis::operator<< (std::ostream & s, const ConditionalDepSpec & a)
{
    s << a._as_string();
    return s;
}

std::ostream &
paludis::operator<< (std::ostream & s, const URILabelsDepSpec & l)
{
    s << join(indirect_iterator(l.begin()), indirect_iterator(l.end()), "+") << ":";
    return s;
}

std::ostream &
paludis::operator<< (std::ostream & s, const DependencyLabelsDepSpec & l)
{
    s << join(indirect_iterator(l.begin()), indirect_iterator(l.end()), ",") << ":";
    return s;
}

PackageDepSpecError::PackageDepSpecError(const std::string & msg) throw () :
    Exception(msg)
{
}

StringDepSpec::StringDepSpec(const std::string & s) :
    _str(s)
{
}

StringDepSpec::~StringDepSpec()
{
}


PlainTextDepSpec::PlainTextDepSpec(const std::string & s) :
    StringDepSpec(s)
{
}

std::tr1::shared_ptr<DepSpec>
PlainTextDepSpec::clone() const
{
    return std::tr1::shared_ptr<DepSpec>(new PlainTextDepSpec(text()));
}

LicenseDepSpec::LicenseDepSpec(const std::string & s) :
    StringDepSpec(s)
{
}

std::tr1::shared_ptr<DepSpec>
LicenseDepSpec::clone() const
{
    return std::tr1::shared_ptr<DepSpec>(new LicenseDepSpec(text()));
}

SimpleURIDepSpec::SimpleURIDepSpec(const std::string & s) :
    StringDepSpec(s)
{
}

std::tr1::shared_ptr<DepSpec>
SimpleURIDepSpec::clone() const
{
    return std::tr1::shared_ptr<DepSpec>(new SimpleURIDepSpec(text()));
}

std::tr1::shared_ptr<const PackageDepSpec>
BlockDepSpec::blocked_spec() const
{
    return _spec;
}

std::tr1::shared_ptr<DepSpec>
BlockDepSpec::clone() const
{
    return std::tr1::shared_ptr<DepSpec>(new BlockDepSpec(std::tr1::static_pointer_cast<PackageDepSpec>(_spec->clone())));
}

FetchableURIDepSpec::FetchableURIDepSpec(const std::string & s) :
    StringDepSpec(s)
{
}

std::string
FetchableURIDepSpec::original_url() const
{
    std::string::size_type p(text().find(" -> "));
    if (std::string::npos == p)
        return text();
    else
        return text().substr(0, p);
}

std::string
FetchableURIDepSpec::renamed_url_suffix() const
{
    std::string::size_type p(text().find(" -> "));
    if (std::string::npos == p)
        return "";
    else
        return text().substr(p + 4);
}

std::string
FetchableURIDepSpec::filename() const
{
    std::string rus = renamed_url_suffix();
    if (! rus.empty())
        return rus;

    std::string orig = original_url();
    std::string::size_type p(orig.rfind('/'));

    if (std::string::npos == p)
        return orig;
    return orig.substr(p+1);
}

std::tr1::shared_ptr<DepSpec>
FetchableURIDepSpec::clone() const
{
    return std::tr1::shared_ptr<FetchableURIDepSpec>(new FetchableURIDepSpec(text()));
}

namespace paludis
{
#ifndef PALUDIS_NO_DOUBLE_TEMPLATE
    template <>
#endif
    template <typename T_>
    struct Implementation<LabelsDepSpec<T_ > >
    {
        std::list<std::tr1::shared_ptr<const typename T_::BasicNode> > items;
    };
}

template <typename T_>
LabelsDepSpec<T_>::LabelsDepSpec() :
    PrivateImplementationPattern<LabelsDepSpec<T_> >(new Implementation<LabelsDepSpec<T_> >)
{
}

template <typename T_>
LabelsDepSpec<T_>::~LabelsDepSpec()
{
}

template <typename T_>
std::tr1::shared_ptr<DepSpec>
LabelsDepSpec<T_>::clone() const
{
    using namespace std::tr1::placeholders;
    std::tr1::shared_ptr<LabelsDepSpec<T_> > my_clone(new LabelsDepSpec<T_>);
    std::for_each(begin(), end(), std::tr1::bind(std::tr1::mem_fn(&LabelsDepSpec<T_>::add_label), my_clone.get(), _1));
    return my_clone;
}

template <typename T_>
typename LabelsDepSpec<T_>::ConstIterator
LabelsDepSpec<T_>::begin() const
{
    return ConstIterator(_imp->items.begin());
}

template <typename T_>
typename LabelsDepSpec<T_>::ConstIterator
LabelsDepSpec<T_>::end() const
{
    return ConstIterator(_imp->items.end());
}

template <typename T_>
void
LabelsDepSpec<T_>::add_label(const std::tr1::shared_ptr<const typename T_::BasicNode> & item)
{
    _imp->items.push_back(item);
}

PackageDepSpecData::~PackageDepSpecData()
{
}

namespace paludis
{
    template <>
    struct Implementation<PackageDepSpec>
    {
        const std::tr1::shared_ptr<const PackageDepSpecData> data;
        std::tr1::shared_ptr<const DepTag> tag;

        Implementation(const std::tr1::shared_ptr<const PackageDepSpecData> & d, const std::tr1::shared_ptr<const DepTag> & t) :
            data(d),
            tag(t)
        {
        }
    };
}

PackageDepSpec::PackageDepSpec(const std::tr1::shared_ptr<const PackageDepSpecData> & d) :
    Cloneable<DepSpec>(),
    StringDepSpec(d->as_string()),
    PrivateImplementationPattern<PackageDepSpec>(new Implementation<PackageDepSpec>(d, std::tr1::shared_ptr<const DepTag>()))
{
}

PackageDepSpec::~PackageDepSpec()
{
}

PackageDepSpec::PackageDepSpec(const PackageDepSpec & d) :
    Cloneable<DepSpec>(d),
    StringDepSpec(d._imp->data->as_string()),
    PrivateImplementationPattern<PackageDepSpec>(new Implementation<PackageDepSpec>(d._imp->data, d._imp->tag)),
    CloneUsingThis<DepSpec, PackageDepSpec>(d)
{
}

std::tr1::shared_ptr<const QualifiedPackageName>
PackageDepSpec::package_ptr() const
{
    return _imp->data->package_ptr();
}

std::tr1::shared_ptr<const PackageNamePart>
PackageDepSpec::package_name_part_ptr() const
{
    return _imp->data->package_name_part_ptr();
}

std::tr1::shared_ptr<const CategoryNamePart>
PackageDepSpec::category_name_part_ptr() const
{
    return _imp->data->category_name_part_ptr();
}

std::tr1::shared_ptr<const VersionRequirements>
PackageDepSpec::version_requirements_ptr() const
{
    return _imp->data->version_requirements_ptr();
}

VersionRequirementsMode
PackageDepSpec::version_requirements_mode() const
{
    return _imp->data->version_requirements_mode();
}

std::tr1::shared_ptr<const SlotRequirement>
PackageDepSpec::slot_requirement_ptr() const
{
    return _imp->data->slot_requirement_ptr();
}

std::tr1::shared_ptr<const RepositoryName>
PackageDepSpec::repository_ptr() const
{
    return _imp->data->repository_ptr();
}

std::tr1::shared_ptr<const AdditionalPackageDepSpecRequirements>
PackageDepSpec::additional_requirements_ptr() const
{
    return _imp->data->additional_requirements_ptr();
}

std::tr1::shared_ptr<PackageDepSpec>
PackageDepSpec::without_additional_requirements() const
{
    using namespace std::tr1::placeholders;

    PartiallyMadePackageDepSpec result;

    if (package_ptr())
        result.package(*package_ptr());

    if (package_name_part_ptr())
        result.package_name_part(*package_name_part_ptr());

    if (category_name_part_ptr())
        result.category_name_part(*category_name_part_ptr());

    if (version_requirements_ptr())
        std::for_each(version_requirements_ptr()->begin(), version_requirements_ptr()->end(),
                std::tr1::bind(&PartiallyMadePackageDepSpec::version_requirement, &result, _1));

    result.version_requirements_mode(version_requirements_mode());

    if (slot_requirement_ptr())
        result.slot_requirement(slot_requirement_ptr());

    if (repository_ptr())
        result.repository(*repository_ptr());

    return make_shared_ptr(new PackageDepSpec(result));
}

std::tr1::shared_ptr<const DepTag>
PackageDepSpec::tag() const
{
    return _imp->tag;
}

void
PackageDepSpec::set_tag(const std::tr1::shared_ptr<const DepTag> & s)
{
    _imp->tag = s;
}

std::string
PackageDepSpec::_as_string() const
{
    return _imp->data->as_string();
}

AdditionalPackageDepSpecRequirement::~AdditionalPackageDepSpecRequirement()
{
}

PartiallyMadePackageDepSpec
paludis::make_package_dep_spec()
{
    return PartiallyMadePackageDepSpec();
}

namespace
{
    struct PartiallyMadePackageDepSpecData :
        PackageDepSpecData
    {
        std::tr1::shared_ptr<QualifiedPackageName> package;
        std::tr1::shared_ptr<PackageNamePart> package_name_part;
        std::tr1::shared_ptr<CategoryNamePart> category_name_part;
        std::tr1::shared_ptr<VersionRequirements> version_requirements;
        VersionRequirementsMode version_requirements_mode_v;
        std::tr1::shared_ptr<const SlotRequirement> slot;
        std::tr1::shared_ptr<RepositoryName> repository;
        std::tr1::shared_ptr<AdditionalPackageDepSpecRequirements> additional_requirements;

        PartiallyMadePackageDepSpecData() :
            PackageDepSpecData(),
            version_requirements_mode_v(vr_and)
        {
        }

        PartiallyMadePackageDepSpecData(const PartiallyMadePackageDepSpecData & other) :
            PackageDepSpecData(other),
            package(other.package),
            package_name_part(other.package_name_part),
            category_name_part(other.category_name_part),
            version_requirements(other.version_requirements),
            version_requirements_mode_v(other.version_requirements_mode_v),
            slot(other.slot),
            repository(other.repository),
            additional_requirements(other.additional_requirements)
        {
        }

        virtual std::string as_string() const
        {
            std::ostringstream s;

            if (version_requirements_ptr())
            {
                if (version_requirements_ptr()->begin() == version_requirements_ptr()->end())
                {
                }
                else if (next(version_requirements_ptr()->begin()) == version_requirements_ptr()->end())
                {
                    if (version_requirements_ptr()->begin()->version_operator == vo_equal_star)
                        s << "=";
                    else
                        s << version_requirements_ptr()->begin()->version_operator;
                }
            }

            if (package_ptr())
                s << *package_ptr();
            else
            {
                if (category_name_part_ptr())
                    s << *category_name_part_ptr();
                else
                    s << "*";

                s << "/";

                if (package_name_part_ptr())
                    s << *package_name_part_ptr();
                else
                    s << "*";
            }

            if (version_requirements_ptr())
            {
                if (version_requirements_ptr()->begin() == version_requirements_ptr()->end())
                {
                }
                else if (next(version_requirements_ptr()->begin()) == version_requirements_ptr()->end())
                {
                    s << "-" << version_requirements_ptr()->begin()->version_spec;
                    if (version_requirements_ptr()->begin()->version_operator == vo_equal_star)
                        s << "*";
                }
            }

            if (slot_requirement_ptr())
                s << stringify(*slot_requirement_ptr());
            if (repository_ptr())
                s << "::" << *repository_ptr();

            if (version_requirements_ptr())
            {
                if (version_requirements_ptr()->begin() == version_requirements_ptr()->end())
                {
                }
                else if (next(version_requirements_ptr()->begin()) == version_requirements_ptr()->end())
                {
                }
                else
                {
                    bool need_op(false);
                    s << "[";
                    for (VersionRequirements::ConstIterator r(version_requirements_ptr()->begin()),
                            r_end(version_requirements_ptr()->end()) ; r != r_end ; ++r)
                    {
                        if (need_op)
                        {
                            do
                            {
                                switch (version_requirements_mode())
                                {
                                    case vr_and:
                                        s << "&";
                                        continue;

                                    case vr_or:
                                        s << "|";
                                        continue;

                                    case last_vr:
                                        ;
                                }
                                throw InternalError(PALUDIS_HERE, "Bad version_requirements_mode");
                            } while (false);
                        }

                        if (r->version_operator == vo_equal_star)
                            s << "=";
                        else
                            s << r->version_operator;

                        s << r->version_spec;

                        if (r->version_operator == vo_equal_star)
                            s << "*";

                        need_op = true;
                    }
                    s << "]";
                }
            }

            if (additional_requirements_ptr())
                for (AdditionalPackageDepSpecRequirements::ConstIterator u(additional_requirements_ptr()->begin()),
                        u_end(additional_requirements_ptr()->end()) ; u != u_end ; ++u)
                    s << (*u)->as_raw_string();

            return s.str();
        }

        virtual std::tr1::shared_ptr<const QualifiedPackageName> package_ptr() const
        {
            return package;
        }

        virtual std::tr1::shared_ptr<const PackageNamePart> package_name_part_ptr() const
        {
            return package_name_part;
        }

        virtual std::tr1::shared_ptr<const CategoryNamePart> category_name_part_ptr() const
        {
            return category_name_part;
        }

        virtual std::tr1::shared_ptr<const VersionRequirements> version_requirements_ptr() const
        {
            return version_requirements;
        }

        virtual VersionRequirementsMode version_requirements_mode() const
        {
            return version_requirements_mode_v;
        }

        virtual std::tr1::shared_ptr<const SlotRequirement> slot_requirement_ptr() const
        {
            return slot;
        }

        virtual std::tr1::shared_ptr<const RepositoryName> repository_ptr() const
        {
            return repository;
        }

        virtual std::tr1::shared_ptr<const AdditionalPackageDepSpecRequirements> additional_requirements_ptr() const
        {
            return additional_requirements;
        }
    };
}

namespace paludis
{
    template <>
    struct Implementation<PartiallyMadePackageDepSpec>
    {
        std::tr1::shared_ptr<PartiallyMadePackageDepSpecData> data;

        Implementation() :
            data(new PartiallyMadePackageDepSpecData)
        {
        }

        Implementation(const Implementation & other) :
            data(new PartiallyMadePackageDepSpecData(*other.data))
        {
        }
    };
}

PartiallyMadePackageDepSpec::PartiallyMadePackageDepSpec() :
    PrivateImplementationPattern<PartiallyMadePackageDepSpec>(new Implementation<PartiallyMadePackageDepSpec>)
{
}

PartiallyMadePackageDepSpec::PartiallyMadePackageDepSpec(const PartiallyMadePackageDepSpec & other) :
    PrivateImplementationPattern<PartiallyMadePackageDepSpec>(new Implementation<PartiallyMadePackageDepSpec>(*other._imp.get()))
{
}

PartiallyMadePackageDepSpec::~PartiallyMadePackageDepSpec()
{
}

PartiallyMadePackageDepSpec &
PartiallyMadePackageDepSpec::package(const QualifiedPackageName & name)
{
    _imp->data->package.reset(new QualifiedPackageName(name));
    return *this;
}

PartiallyMadePackageDepSpec &
PartiallyMadePackageDepSpec::slot_requirement(const std::tr1::shared_ptr<const SlotRequirement> & s)
{
    _imp->data->slot = s;
    return *this;
}

PartiallyMadePackageDepSpec &
PartiallyMadePackageDepSpec::repository(const RepositoryName & repo)
{
   _imp->data->repository.reset(new RepositoryName(repo));
   return *this;
}

PartiallyMadePackageDepSpec &
PartiallyMadePackageDepSpec::package_name_part(const PackageNamePart & part)
{
    _imp->data->package_name_part.reset(new PackageNamePart(part));
    return *this;
}

PartiallyMadePackageDepSpec &
PartiallyMadePackageDepSpec::category_name_part(const CategoryNamePart & part)
{
    _imp->data->category_name_part.reset(new CategoryNamePart(part));
    return *this;
}

PartiallyMadePackageDepSpec &
PartiallyMadePackageDepSpec::version_requirement(const VersionRequirement & req)
{
    if (! _imp->data->version_requirements)
        _imp->data->version_requirements.reset(new VersionRequirements);
    _imp->data->version_requirements->push_back(req);
    return *this;
}

PartiallyMadePackageDepSpec &
PartiallyMadePackageDepSpec::version_requirements_mode(const VersionRequirementsMode & mode)
{
    _imp->data->version_requirements_mode_v = mode;
    return *this;
}

PartiallyMadePackageDepSpec &
PartiallyMadePackageDepSpec::additional_requirement(const std::tr1::shared_ptr<const AdditionalPackageDepSpecRequirement> & req)
{
    if (! _imp->data->additional_requirements)
        _imp->data->additional_requirements.reset(new AdditionalPackageDepSpecRequirements);
    _imp->data->additional_requirements->push_back(req);
    return *this;
}

PartiallyMadePackageDepSpec::operator const PackageDepSpec() const
{
    return PackageDepSpec(_imp->data);
}

const PackageDepSpec
PartiallyMadePackageDepSpec::to_package_dep_spec() const
{
    return operator const PackageDepSpec();
}

template class LabelsDepSpec<URILabelVisitorTypes>;
template class WrappedForwardIterator<LabelsDepSpec<URILabelVisitorTypes>::ConstIteratorTag,
         const std::tr1::shared_ptr<const URILabelVisitorTypes::BasicNode> >;

template class LabelsDepSpec<DependencyLabelVisitorTypes>;
template class WrappedForwardIterator<LabelsDepSpec<DependencyLabelVisitorTypes>::ConstIteratorTag,
         const std::tr1::shared_ptr<const DependencyLabelVisitorTypes::BasicNode> >;

template class Sequence<std::tr1::shared_ptr<const AdditionalPackageDepSpecRequirement> >;

template class Cloneable<DepSpec>;
template class PrivateImplementationPattern<ConditionalDepSpec>;
template class CloneUsingThis<DepSpec, ConditionalDepSpec>;
template class PrivateImplementationPattern<PartiallyMadePackageDepSpec>;
template class PrivateImplementationPattern<PackageDepSpec>;
template class CloneUsingThis<DepSpec, PackageDepSpec>;
template class PrivateImplementationPattern<DependencyLabelsDepSpec>;
template class PrivateImplementationPattern<URILabelsDepSpec>;

