/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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
#include <paludis/dep_spec_data.hh>
#include <paludis/package_dep_spec_requirement.hh>
#include <paludis/package_dep_spec_properties.hh>

#include <paludis/util/clone-impl.hh>
#include <paludis/util/log.hh>
#include <paludis/util/join.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/sequence-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/iterator_funcs.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/options.hh>
#include <paludis/util/accept_visitor.hh>
#include <paludis/util/make_null_shared_ptr.hh>

#include <functional>
#include <algorithm>
#include <list>
#include <map>

using namespace paludis;

namespace paludis
{
    template <>
    struct Imp<DepSpec>
    {
        std::shared_ptr<const DepSpecAnnotations> annotations;
    };
}

DepSpec::DepSpec() :
    _imp()
{
}

DepSpec::~DepSpec()
{
}

const std::shared_ptr<const DepSpecAnnotations>
DepSpec::maybe_annotations() const
{
    return _imp->annotations;
}

void
DepSpec::set_annotations(const std::shared_ptr<const DepSpecAnnotations> & a)
{
    _imp->annotations = a;
}

AnyDepSpec::AnyDepSpec()
{
}

std::shared_ptr<DepSpec>
AnyDepSpec::clone() const
{
    std::shared_ptr<AnyDepSpec> result(std::make_shared<AnyDepSpec>());
    result->set_annotations(maybe_annotations());
    return result;
}

AllDepSpec::AllDepSpec()
{
}

std::shared_ptr<DepSpec>
AllDepSpec::clone() const
{
    std::shared_ptr<AllDepSpec> result(std::make_shared<AllDepSpec>());
    result->set_annotations(maybe_annotations());
    return result;
}

ExactlyOneDepSpec::ExactlyOneDepSpec()
{
}

std::shared_ptr<DepSpec>
ExactlyOneDepSpec::clone() const
{
    std::shared_ptr<ExactlyOneDepSpec> result(std::make_shared<ExactlyOneDepSpec>());
    result->set_annotations(maybe_annotations());
    return result;
}

namespace paludis
{
    template <>
    struct Imp<ConditionalDepSpec>
    {
        const std::shared_ptr<const ConditionalDepSpecData> data;

        Imp(const std::shared_ptr<const ConditionalDepSpecData> & d) :
            data(d)
        {
        }
    };
}

ConditionalDepSpec::ConditionalDepSpec(const std::shared_ptr<const ConditionalDepSpecData> & d) :
    _imp(d)
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
    CloneUsingThis<DepSpec, ConditionalDepSpec>(other),
    _imp(other._imp->data)
{
    set_annotations(other.maybe_annotations());
}

ConditionalDepSpec::~ConditionalDepSpec()
{
}

bool
ConditionalDepSpec::condition_met(const Environment * const env, const std::shared_ptr<const PackageID> & id) const
{
    return _imp->data->condition_met(env, id);
}

bool
ConditionalDepSpec::condition_would_be_met_when(
        const Environment * const env,
        const std::shared_ptr<const PackageID> & id,
        const ChangedChoices & c) const
{
    return _imp->data->condition_would_be_met_when(env, id, c);
}

bool
ConditionalDepSpec::condition_meetable(const Environment * const env, const std::shared_ptr<const PackageID> & id) const
{
    return _imp->data->condition_meetable(env, id);
}

const std::shared_ptr<const ConditionalDepSpecData>
ConditionalDepSpec::data() const
{
    return _imp->data;
}

std::string
ConditionalDepSpec::_as_string() const
{
    return _imp->data->as_string();
}

NamedSetDepSpec::NamedSetDepSpec(const SetName & n) :
    _name(n)
{
}

const SetName
NamedSetDepSpec::name() const
{
    return _name;
}

const std::string
NamedSetDepSpec::text() const
{
    return stringify(_name);
}

std::shared_ptr<DepSpec>
NamedSetDepSpec::clone() const
{
    std::shared_ptr<NamedSetDepSpec> result(std::make_shared<NamedSetDepSpec>(_name));
    result->set_annotations(maybe_annotations());
    return result;
}

BlockDepSpec::BlockDepSpec(const std::string & s, const PackageDepSpec & p) :
    _text(s),
    _spec(p)
{
}

BlockDepSpec::BlockDepSpec(const BlockDepSpec & other) :
    _text(other._text),
    _spec(other._spec)
{
    set_annotations(other.maybe_annotations());
}

const std::string
BlockDepSpec::text() const
{
    return _text;
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
    s << a.text();
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
    s << a.text();
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
paludis::operator<< (std::ostream & s, const DependenciesLabelsDepSpec & l)
{
    s << join(indirect_iterator(l.begin()), indirect_iterator(l.end()), "+") << ":";
    return s;
}

std::ostream &
paludis::operator<< (std::ostream & s, const PlainTextLabelDepSpec & l)
{
    s << l.label() << ":";
    return s;
}

PackageDepSpecError::PackageDepSpecError(const std::string & msg) throw () :
    Exception(msg)
{
}

PlainTextDepSpec::PlainTextDepSpec(const std::string & s) :
    _text(s)
{
}

std::shared_ptr<DepSpec>
PlainTextDepSpec::clone() const
{
    std::shared_ptr<PlainTextDepSpec> result(std::make_shared<PlainTextDepSpec>(text()));
    result->set_annotations(maybe_annotations());
    return result;
}

const std::string
PlainTextDepSpec::text() const
{
    return _text;
}

PlainTextLabelDepSpec::PlainTextLabelDepSpec(const std::string & s) :
    _text(s)
{
}

PlainTextLabelDepSpec::~PlainTextLabelDepSpec()
{
}

std::shared_ptr<DepSpec>
PlainTextLabelDepSpec::clone() const
{
    std::shared_ptr<PlainTextLabelDepSpec> result(std::make_shared<PlainTextLabelDepSpec>(text()));
    result->set_annotations(maybe_annotations());
    return result;
}

const std::string
PlainTextLabelDepSpec::text() const
{
    return _text;
}

const std::string
PlainTextLabelDepSpec::label() const
{
    return text().substr(0, text().length() - 1);
}

LicenseDepSpec::LicenseDepSpec(const std::string & s) :
    _text(s)
{
}

std::shared_ptr<DepSpec>
LicenseDepSpec::clone() const
{
    std::shared_ptr<LicenseDepSpec> result(std::make_shared<LicenseDepSpec>(text()));
    result->set_annotations(maybe_annotations());
    return result;
}

const std::string
LicenseDepSpec::text() const
{
    return _text;
}

SimpleURIDepSpec::SimpleURIDepSpec(const std::string & s) :
    _text(s)
{
}

std::shared_ptr<DepSpec>
SimpleURIDepSpec::clone() const
{
    std::shared_ptr<SimpleURIDepSpec> result(std::make_shared<SimpleURIDepSpec>(text()));
    result->set_annotations(maybe_annotations());
    return result;
}

const std::string
SimpleURIDepSpec::text() const
{
    return _text;
}

const PackageDepSpec
BlockDepSpec::blocking() const
{
    return _spec;
}

std::shared_ptr<DepSpec>
BlockDepSpec::clone() const
{
    std::shared_ptr<BlockDepSpec> result(std::make_shared<BlockDepSpec>(*this));
    result->set_annotations(maybe_annotations());
    return result;
}

FetchableURIDepSpec::FetchableURIDepSpec(const std::string & s) :
    _text(s)
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

std::shared_ptr<DepSpec>
FetchableURIDepSpec::clone() const
{
    std::shared_ptr<FetchableURIDepSpec> result(std::make_shared<FetchableURIDepSpec>(text()));
    result->set_annotations(maybe_annotations());
    return result;
}

const std::string
FetchableURIDepSpec::text() const
{
    return _text;
}

namespace paludis
{
    template <typename T_>
    struct Imp<LabelsDepSpec<T_ > >
    {
        std::list<std::shared_ptr<const T_> > items;
    };

    template <>
    struct WrappedForwardIteratorTraits<DependenciesLabelsDepSpec::ConstIteratorTag>
    {
        typedef std::list<std::shared_ptr<const DependenciesLabel> >::const_iterator UnderlyingIterator;
    };

    template <>
    struct WrappedForwardIteratorTraits<URILabelsDepSpec::ConstIteratorTag>
    {
        typedef std::list<std::shared_ptr<const URILabel> >::const_iterator UnderlyingIterator;
    };
}

template <typename T_>
LabelsDepSpec<T_>::LabelsDepSpec() :
    _imp()
{
}

template <typename T_>
LabelsDepSpec<T_>::~LabelsDepSpec()
{
}

template <typename T_>
std::shared_ptr<DepSpec>
LabelsDepSpec<T_>::clone() const
{
    using namespace std::placeholders;
    std::shared_ptr<LabelsDepSpec<T_> > my_clone(std::make_shared<LabelsDepSpec<T_>>());
    my_clone->set_annotations(maybe_annotations());
    std::for_each(begin(), end(), std::bind(std::mem_fn(&LabelsDepSpec<T_>::add_label), my_clone.get(), _1));
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
LabelsDepSpec<T_>::add_label(const std::shared_ptr<const T_> & item)
{
    _imp->items.push_back(item);
}

namespace
{
    template <typename I_, typename P_>
    I_ find_unique_if(I_ cur, I_ end, P_ pred)
    {
        I_ result(end);
        for ( ; cur != end ; ++cur)
            if (pred(*cur))
            {
                if (result != end)
                    return end;
                else
                    result = cur;
            }

        return result;
    }
}

namespace paludis
{
    template <>
    struct Imp<PackageDepSpec>
    {
        const std::shared_ptr<const PackageDepSpecData> data;
        std::string text;

        Imp(const std::shared_ptr<const PackageDepSpecData> & d) :
            data(d)
        {
        }
    };
}

PackageDepSpec::PackageDepSpec(const std::shared_ptr<const PackageDepSpecData> & d) :
    Cloneable<DepSpec>(),
    _imp(d)
{
    std::ostringstream s;

    std::shared_ptr<VersionRequirementSequence> all_versions;
    std::shared_ptr<KeyRequirementSequence> all_keys;
    std::shared_ptr<ChoiceRequirementSequence> all_choices;

    for (auto u(_imp->data->requirements()->begin()), u_end(_imp->data->requirements()->end()) ;
            u != u_end ; ++u)
    {
        if ((*u)->accept_returning<bool>(DetectPackageDepSpecRequirement<VersionRequirement>()))
        {
            if (! all_versions)
                all_versions = std::make_shared<VersionRequirementSequence>();
            all_versions->push_back(std::static_pointer_cast<const VersionRequirement>(*u));
        }
        else if ((*u)->accept_returning<bool>(DetectPackageDepSpecRequirement<KeyRequirement>()))
        {
            if (! all_keys)
                all_keys = std::make_shared<KeyRequirementSequence>();
            all_keys->push_back(std::static_pointer_cast<const KeyRequirement>(*u));
        }
        else if ((*u)->accept_returning<bool>(DetectPackageDepSpecRequirement<ChoiceRequirement>()))
        {
            if (! all_choices)
                all_choices = std::make_shared<ChoiceRequirementSequence>();
            all_choices->push_back(std::static_pointer_cast<const ChoiceRequirement>(*u));
        }
    }

    if (all_versions)
    {
        if (all_versions->begin() == all_versions->end())
        {
        }
        else if (next(all_versions->begin()) == all_versions->end() &&
                ! _imp->data->options()[pdsdo_always_use_ranged_deps])
        {
            if ((*all_versions->begin())->version_operator() == vo_stupid_equal_star ||
                    (*all_versions->begin())->version_operator() == vo_nice_equal_star)
                s << "=";
            else
                s << (*all_versions->begin())->version_operator();
        }
    }

    if (package_name_requirement())
        s << package_name_requirement()->name();
    else
    {
        if (category_name_part_requirement())
            s << category_name_part_requirement()->name_part();
        else
            s << "*";

        s << "/";

        if (package_name_part_requirement())
            s << package_name_part_requirement()->name_part();
        else
            s << "*";
    }

    if (all_versions)
    {
        if (all_versions->begin() == all_versions->end())
        {
        }
        else if (next(all_versions->begin()) == all_versions->end() &&
                ! _imp->data->options()[pdsdo_always_use_ranged_deps])
        {
            s << "-" << (*all_versions->begin())->version_spec();
            if ((*all_versions->begin())->version_operator() == vo_stupid_equal_star ||
                    (*all_versions->begin())->version_operator() == vo_nice_equal_star)
                s << "*";
        }
    }

    if (exact_slot_requirement())
    {
        if (exact_slot_requirement()->locked())
            s << ":=";
        else
            s << ":";

        s << stringify(exact_slot_requirement()->name());
    }

    if (any_slot_requirement())
    {
        if (any_slot_requirement()->locking())
            s << ":=";
        else
            s << ":*";
    }

    std::string left, right;
    bool need_arrow(false);

    if (from_repository_requirement())
        left = stringify(from_repository_requirement()->name());

    if (in_repository_requirement())
        right = stringify(in_repository_requirement()->name());

    if (installed_at_path_requirement())
    {
        if (! right.empty())
        {
            need_arrow = true;
            right.append("->");
        }
        right.append(stringify(installed_at_path_requirement()->path()));
    }

    if (installable_to_repository_requirement())
    {
        if (! right.empty())
        {
            need_arrow = true;
            right.append("->");
        }
        if (installable_to_repository_requirement()->include_masked())
            right.append(stringify(installable_to_repository_requirement()->name()) + "??");
        else
            right.append(stringify(installable_to_repository_requirement()->name()) + "?");
    }

    if (installable_to_path_requirement())
    {
        if (! right.empty())
        {
            need_arrow = true;
            right.append("->");
        }
        if (installable_to_path_requirement()->include_masked())
            right.append(stringify(installable_to_path_requirement()->path()) + "??");
        else
            right.append(stringify(installable_to_path_requirement()->path()) + "?");
    }

    if (need_arrow || ((! left.empty()) && (! right.empty())))
        s << "::" << left << "->" << right;
    else if (! right.empty())
        s << "::" << right;
    else if (! left.empty())
        s << "::" << left << "->";

    if (all_versions)
    {
        if (all_versions->begin() == all_versions->end())
        {
        }
        else if (next(all_versions->begin()) == all_versions->end() &&
                ! _imp->data->options()[pdsdo_always_use_ranged_deps])
        {
        }
        else
        {
            bool need_op(false);
            s << "[";
            for (auto r(all_versions->begin()), r_end(all_versions->end()) ; r != r_end ; ++r)
            {
                if (need_op)
                {
                    do
                    {
                        switch ((*r)->combiner())
                        {
                            case vrc_and:
                                s << "&";
                                continue;

                            case vrc_or:
                                s << "|";
                                continue;

                            case last_vrc:
                                ;
                        }
                        throw InternalError(PALUDIS_HERE, "Bad version_requirements_mode");
                    } while (false);
                }

                if ((*r)->version_operator() == vo_stupid_equal_star || (*r)->version_operator() == vo_nice_equal_star)
                    s << "=";
                else
                    s << (*r)->version_operator();

                s << (*r)->version_spec();

                if ((*r)->version_operator() == vo_stupid_equal_star || (*r)->version_operator() == vo_nice_equal_star)
                    s << "*";

                need_op = true;
            }
            s << "]";
        }
    }

    if (all_choices)
        for (auto u(all_choices->begin()), u_end(all_choices->end()) ; u != u_end ; ++u)
            s << (*u)->as_raw_string();

    if (all_keys)
        for (auto u(all_keys->begin()), u_end(all_keys->end()) ; u != u_end ; ++u)
            s << (*u)->as_raw_string();

    _imp->text = s.str();
}

PackageDepSpec::~PackageDepSpec()
{
}

PackageDepSpec::PackageDepSpec(const PackageDepSpec & d) :
    Cloneable<DepSpec>(d),
    CloneUsingThis<DepSpec, PackageDepSpec>(d),
    _imp(d._imp->data)
{
    set_annotations(d.maybe_annotations());
    _imp->text = d._imp->text;
}

const std::string
PackageDepSpec::text() const
{
    return _imp->text;
}

const std::shared_ptr<const NameRequirement>
PackageDepSpec::package_name_requirement() const
{
    return _imp->data->package_name_requirement();
}

const std::shared_ptr<const PackageNamePartRequirement>
PackageDepSpec::package_name_part_requirement() const
{
    DetectPackageDepSpecRequirement<PackageNamePartRequirement> v;
    auto r(find_unique_if(indirect_iterator(_imp->data->requirements()->begin()),
                indirect_iterator(_imp->data->requirements()->end()), accept_visitor_returning<bool>(v)));

    if (r != indirect_iterator(_imp->data->requirements()->end()))
        return std::static_pointer_cast<const PackageNamePartRequirement>(*r.underlying_iterator());
    else
        return make_null_shared_ptr();
}

const std::shared_ptr<const CategoryNamePartRequirement>
PackageDepSpec::category_name_part_requirement() const
{
    DetectPackageDepSpecRequirement<CategoryNamePartRequirement> v;
    auto r(find_unique_if(indirect_iterator(_imp->data->requirements()->begin()),
                indirect_iterator(_imp->data->requirements()->end()), accept_visitor_returning<bool>(v)));

    if (r != indirect_iterator(_imp->data->requirements()->end()))
        return std::static_pointer_cast<const CategoryNamePartRequirement>(*r.underlying_iterator());
    else
        return make_null_shared_ptr();
}

const std::shared_ptr<const ExactSlotRequirement>
PackageDepSpec::exact_slot_requirement() const
{
    return _imp->data->exact_slot_requirement();
}

const std::shared_ptr<const AnySlotRequirement>
PackageDepSpec::any_slot_requirement() const
{
    DetectPackageDepSpecRequirement<AnySlotRequirement> v;
    auto r(find_unique_if(indirect_iterator(_imp->data->requirements()->begin()),
                indirect_iterator(_imp->data->requirements()->end()), accept_visitor_returning<bool>(v)));

    if (r != indirect_iterator(_imp->data->requirements()->end()))
        return std::static_pointer_cast<const AnySlotRequirement>(*r.underlying_iterator());
    else
        return make_null_shared_ptr();
}

const std::shared_ptr<const InRepositoryRequirement>
PackageDepSpec::in_repository_requirement() const
{
    DetectPackageDepSpecRequirement<InRepositoryRequirement> v;
    auto r(find_unique_if(indirect_iterator(_imp->data->requirements()->begin()),
                indirect_iterator(_imp->data->requirements()->end()), accept_visitor_returning<bool>(v)));

    if (r != indirect_iterator(_imp->data->requirements()->end()))
        return std::static_pointer_cast<const InRepositoryRequirement>(*r.underlying_iterator());
    else
        return make_null_shared_ptr();
}

const std::shared_ptr<const InstallableToRepositoryRequirement>
PackageDepSpec::installable_to_repository_requirement() const
{
    DetectPackageDepSpecRequirement<InstallableToRepositoryRequirement> v;
    auto r(find_unique_if(indirect_iterator(_imp->data->requirements()->begin()),
                indirect_iterator(_imp->data->requirements()->end()), accept_visitor_returning<bool>(v)));

    if (r != indirect_iterator(_imp->data->requirements()->end()))
        return std::static_pointer_cast<const InstallableToRepositoryRequirement>(*r.underlying_iterator());
    else
        return make_null_shared_ptr();
}

const std::shared_ptr<const FromRepositoryRequirement>
PackageDepSpec::from_repository_requirement() const
{
    DetectPackageDepSpecRequirement<FromRepositoryRequirement> v;
    auto r(find_unique_if(indirect_iterator(_imp->data->requirements()->begin()),
                indirect_iterator(_imp->data->requirements()->end()), accept_visitor_returning<bool>(v)));

    if (r != indirect_iterator(_imp->data->requirements()->end()))
        return std::static_pointer_cast<const FromRepositoryRequirement>(*r.underlying_iterator());
    else
        return make_null_shared_ptr();
}

const std::shared_ptr<const InstalledAtPathRequirement>
PackageDepSpec::installed_at_path_requirement() const
{
    DetectPackageDepSpecRequirement<InstalledAtPathRequirement> v;
    auto r(find_unique_if(indirect_iterator(_imp->data->requirements()->begin()),
                indirect_iterator(_imp->data->requirements()->end()), accept_visitor_returning<bool>(v)));

    if (r != indirect_iterator(_imp->data->requirements()->end()))
        return std::static_pointer_cast<const InstalledAtPathRequirement>(*r.underlying_iterator());
    else
        return make_null_shared_ptr();
}

const std::shared_ptr<const InstallableToPathRequirement>
PackageDepSpec::installable_to_path_requirement() const
{
    DetectPackageDepSpecRequirement<InstallableToPathRequirement> v;
    auto r(find_unique_if(indirect_iterator(_imp->data->requirements()->begin()),
                indirect_iterator(_imp->data->requirements()->end()), accept_visitor_returning<bool>(v)));

    if (r != indirect_iterator(_imp->data->requirements()->end()))
        return std::static_pointer_cast<const InstallableToPathRequirement>(*r.underlying_iterator());
    else
        return make_null_shared_ptr();
}

const std::shared_ptr<const PackageDepSpecRequirementSequence>
PackageDepSpec::requirements() const
{
    return _imp->data->requirements();
}

const std::shared_ptr<const PackageDepSpecData>
PackageDepSpec::data() const
{
    return _imp->data;
}

template class LabelsDepSpec<URILabel>;
template class LabelsDepSpec<DependenciesLabel>;

template class Cloneable<DepSpec>;
template class Pimp<ConditionalDepSpec>;
template class CloneUsingThis<DepSpec, ConditionalDepSpec>;
template class Pimp<PackageDepSpec>;
template class CloneUsingThis<DepSpec, PackageDepSpec>;
template class Pimp<URILabelsDepSpec>;
template class Pimp<DependenciesLabelsDepSpec>;

template class WrappedForwardIterator<DependenciesLabelsDepSpec::ConstIteratorTag,
         const std::shared_ptr<const DependenciesLabel> >;
template class WrappedForwardIterator<URILabelsDepSpec::ConstIteratorTag,
         const std::shared_ptr<const URILabel> >;

