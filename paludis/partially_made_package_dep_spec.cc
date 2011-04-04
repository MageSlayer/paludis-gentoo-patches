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

#include <paludis/partially_made_package_dep_spec.hh>
#include <paludis/dep_spec_data.hh>
#include <paludis/package_dep_spec_constraint.hh>
#include <paludis/version_operator.hh>
#include <paludis/version_spec.hh>

#include <paludis/util/stringify.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/options.hh>
#include <paludis/util/wrapped_output_iterator-impl.hh>
#include <paludis/util/iterator_funcs.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/sequence-impl.hh>

#include <iterator>
#include <algorithm>
#include <ostream>

using namespace paludis;

#include <paludis/partially_made_package_dep_spec-se.cc>

PartiallyMadePackageDepSpec
paludis::make_package_dep_spec(const PartiallyMadePackageDepSpecOptions & o)
{
    return PartiallyMadePackageDepSpec(o);
}

namespace
{
    struct PartiallyMadePackageDepSpecData :
        PackageDepSpecData
    {
        std::shared_ptr<const NameConstraint> package;
        std::shared_ptr<const PackageNamePartConstraint> package_name_part;
        std::shared_ptr<const CategoryNamePartConstraint> category_name_part;
        std::shared_ptr<VersionConstraintSequence> all_versions;
        std::shared_ptr<const AnySlotConstraint> any_slot;
        std::shared_ptr<const ExactSlotConstraint> exact_slot;
        std::shared_ptr<const InRepositoryConstraint> in_repository;
        std::shared_ptr<const FromRepositoryConstraint> from_repository;
        std::shared_ptr<const InstallableToRepositoryConstraint> installable_to_repository;
        std::shared_ptr<const InstalledAtPathConstraint> installed_at_path;
        std::shared_ptr<const InstallableToPathConstraint> installable_to_path;
        std::shared_ptr<KeyConstraintSequence> all_keys;
        std::shared_ptr<ChoiceConstraintSequence> all_choices;
        PartiallyMadePackageDepSpecOptions options_for_partially_made_package_dep_spec_v;

        PartiallyMadePackageDepSpecData(const PartiallyMadePackageDepSpecOptions & o) :
            PackageDepSpecData(),
            options_for_partially_made_package_dep_spec_v(o)
        {
        }

        PartiallyMadePackageDepSpecData(const PackageDepSpecData & other) :
            PackageDepSpecData(other),
            package(other.package_name_constraint()),
            package_name_part(other.package_name_part_constraint()),
            category_name_part(other.category_name_part_constraint()),
            all_versions(other.all_version_constraints() ? new VersionConstraintSequence : 0),
            any_slot(other.any_slot_constraint()),
            exact_slot(other.exact_slot_constraint()),
            in_repository(other.in_repository_constraint()),
            from_repository(other.from_repository_constraint()),
            installable_to_repository(other.installable_to_repository_constraint()),
            installed_at_path(other.installed_at_path_constraint()),
            installable_to_path(other.installable_to_path_constraint()),
            all_keys(other.all_key_constraints() ? new KeyConstraintSequence : 0),
            all_choices(other.all_choice_constraints() ? new ChoiceConstraintSequence : 0),
            options_for_partially_made_package_dep_spec_v(other.options_for_partially_made_package_dep_spec())
        {
            if (all_versions)
                std::copy(other.all_version_constraints()->begin(), other.all_version_constraints()->end(),
                        all_versions->back_inserter());

            if (all_keys)
                std::copy(other.all_key_constraints()->begin(), other.all_key_constraints()->end(),
                        all_keys->back_inserter());

            if (all_choices)
                std::copy(other.all_choice_constraints()->begin(), other.all_choice_constraints()->end(),
                        all_choices->back_inserter());
        }

        PartiallyMadePackageDepSpecData(const PartiallyMadePackageDepSpecData & other) :
            PackageDepSpecData(other),
            package(other.package),
            package_name_part(other.package_name_part),
            category_name_part(other.category_name_part),
            all_versions(other.all_versions),
            any_slot(other.any_slot),
            exact_slot(other.exact_slot),
            in_repository(other.in_repository),
            from_repository(other.from_repository),
            installable_to_repository(other.installable_to_repository),
            installed_at_path(other.installed_at_path),
            installable_to_path(other.installable_to_path),
            all_keys(other.all_keys),
            all_choices(other.all_choices),
            options_for_partially_made_package_dep_spec_v(other.options_for_partially_made_package_dep_spec_v)
        {
        }

        virtual std::string as_string() const
        {
            std::ostringstream s;

            if (all_version_constraints())
            {
                if (all_version_constraints()->begin() == all_version_constraints()->end())
                {
                }
                else if (next(all_version_constraints()->begin()) == all_version_constraints()->end() &&
                        ! options_for_partially_made_package_dep_spec_v[pmpdso_always_use_ranged_deps])
                {
                    if ((*all_version_constraints()->begin())->version_operator() == vo_stupid_equal_star ||
                            (*all_version_constraints()->begin())->version_operator() == vo_nice_equal_star)
                        s << "=";
                    else
                        s << (*all_version_constraints()->begin())->version_operator();
                }
            }

            if (package_name_constraint())
                s << package_name_constraint()->name();
            else
            {
                if (category_name_part_constraint())
                    s << category_name_part_constraint()->name_part();
                else
                    s << "*";

                s << "/";

                if (package_name_part_constraint())
                    s << package_name_part_constraint()->name_part();
                else
                    s << "*";
            }

            if (all_version_constraints())
            {
                if (all_version_constraints()->begin() == all_version_constraints()->end())
                {
                }
                else if (next(all_version_constraints()->begin()) == all_version_constraints()->end() &&
                        ! options_for_partially_made_package_dep_spec_v[pmpdso_always_use_ranged_deps])
                {
                    s << "-" << (*all_version_constraints()->begin())->version_spec();
                    if ((*all_version_constraints()->begin())->version_operator() == vo_stupid_equal_star ||
                            (*all_version_constraints()->begin())->version_operator() == vo_nice_equal_star)
                        s << "*";
                }
            }

            if (exact_slot_constraint())
            {
                if (exact_slot_constraint()->locked())
                    s << ":=";
                else
                    s << ":";

                s << stringify(exact_slot_constraint()->name());
            }

            if (any_slot_constraint())
            {
                if (any_slot_constraint()->locking())
                    s << ":=";
                else
                    s << ":*";
            }

            std::string left, right;
            bool need_arrow(false);

            if (from_repository_constraint())
                left = stringify(from_repository_constraint()->name());

            if (in_repository_constraint())
                right = stringify(in_repository_constraint()->name());

            if (installed_at_path_constraint())
            {
                if (! right.empty())
                {
                    need_arrow = true;
                    right.append("->");
                }
                right.append(stringify(installed_at_path_constraint()->path()));
            }

            if (installable_to_repository_constraint())
            {
                if (! right.empty())
                {
                    need_arrow = true;
                    right.append("->");
                }
                if (installable_to_repository_constraint()->include_masked())
                    right.append(stringify(installable_to_repository_constraint()->name()) + "??");
                else
                    right.append(stringify(installable_to_repository_constraint()->name()) + "?");
            }

            if (installable_to_path_constraint())
            {
                if (! right.empty())
                {
                    need_arrow = true;
                    right.append("->");
                }
                if (installable_to_path_constraint()->include_masked())
                    right.append(stringify(installable_to_path_constraint()->path()) + "??");
                else
                    right.append(stringify(installable_to_path_constraint()->path()) + "?");
            }

            if (need_arrow || ((! left.empty()) && (! right.empty())))
                s << "::" << left << "->" << right;
            else if (! right.empty())
                s << "::" << right;
            else if (! left.empty())
                s << "::" << left << "->";

            if (all_version_constraints())
            {
                if (all_version_constraints()->begin() == all_version_constraints()->end())
                {
                }
                else if (next(all_version_constraints()->begin()) == all_version_constraints()->end() &&
                        ! options_for_partially_made_package_dep_spec_v[pmpdso_always_use_ranged_deps])
                {
                }
                else
                {
                    bool need_op(false);
                    s << "[";
                    for (auto r(all_version_constraints()->begin()), r_end(all_version_constraints()->end()) ; r != r_end ; ++r)
                    {
                        if (need_op)
                        {
                            do
                            {
                                switch ((*r)->combiner())
                                {
                                    case vcc_and:
                                        s << "&";
                                        continue;

                                    case vcc_or:
                                        s << "|";
                                        continue;

                                    case last_vcc:
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

            if (all_choice_constraints())
                for (auto u(all_choice_constraints()->begin()), u_end(all_choice_constraints()->end()) ; u != u_end ; ++u)
                    s << (*u)->as_raw_string();

            if (all_key_constraints())
                for (auto u(all_key_constraints()->begin()), u_end(all_key_constraints()->end()) ; u != u_end ; ++u)
                {
                    s << "[" << (*u)->key();

                    switch ((*u)->operation())
                    {
                        case kco_equals:         s << "=" << (*u)->pattern(); break;
                        case kco_less_than:      s << "<" << (*u)->pattern(); break;
                        case kco_greater_than:   s << ">" << (*u)->pattern(); break;
                        case kco_question:       s << "?";                    break;

                        case last_kco:
                            throw InternalError(PALUDIS_HERE, "Bad KeyConstraintOperation");
                    }

                    s << "]";
                }

            return s.str();
        }

        virtual const std::shared_ptr<const NameConstraint> package_name_constraint() const
        {
            return package;
        }

        virtual const std::shared_ptr<const PackageNamePartConstraint> package_name_part_constraint() const
        {
            return package_name_part;
        }

        virtual const std::shared_ptr<const CategoryNamePartConstraint> category_name_part_constraint() const
        {
            return category_name_part;
        }

        virtual const std::shared_ptr<const VersionConstraintSequence> all_version_constraints() const
        {
            return all_versions;
        }

        virtual const std::shared_ptr<const ExactSlotConstraint> exact_slot_constraint() const
        {
            return exact_slot;
        }

        virtual const std::shared_ptr<const AnySlotConstraint> any_slot_constraint() const
        {
            return any_slot;
        }

        virtual const std::shared_ptr<const InRepositoryConstraint> in_repository_constraint() const
        {
            return in_repository;
        }

        virtual const std::shared_ptr<const InstallableToRepositoryConstraint> installable_to_repository_constraint() const
        {
            return installable_to_repository;
        }

        virtual const std::shared_ptr<const FromRepositoryConstraint> from_repository_constraint() const
        {
            return from_repository;
        }

        virtual const std::shared_ptr<const InstalledAtPathConstraint> installed_at_path_constraint() const
        {
            return installed_at_path;
        }

        virtual const std::shared_ptr<const InstallableToPathConstraint> installable_to_path_constraint() const
        {
            return installable_to_path;
        }

        virtual const std::shared_ptr<const KeyConstraintSequence> all_key_constraints() const
        {
            return all_keys;
        }

        virtual const std::shared_ptr<const ChoiceConstraintSequence> all_choice_constraints() const
        {
            return all_choices;
        }

        virtual const PartiallyMadePackageDepSpecOptions options_for_partially_made_package_dep_spec() const
        {
            return options_for_partially_made_package_dep_spec_v;
        }
    };
}

namespace paludis
{
    template <>
    struct Imp<PartiallyMadePackageDepSpec>
    {
        std::shared_ptr<PartiallyMadePackageDepSpecData> data;

        Imp(const PartiallyMadePackageDepSpecOptions & o) :
            data(std::make_shared<PartiallyMadePackageDepSpecData>(o))
        {
        }

        Imp(const Imp & other) :
            data(std::make_shared<PartiallyMadePackageDepSpecData>(*other.data))
        {
        }

        Imp(const PackageDepSpec & other) :
            data(std::make_shared<PartiallyMadePackageDepSpecData>(*other.data()))
        {
        }
    };
}

PartiallyMadePackageDepSpec::PartiallyMadePackageDepSpec(const PartiallyMadePackageDepSpecOptions & o) :
    _imp(o)
{
}

PartiallyMadePackageDepSpec::PartiallyMadePackageDepSpec(const PartiallyMadePackageDepSpec & other) :
    _imp(*other._imp.get())
{
}

PartiallyMadePackageDepSpec::PartiallyMadePackageDepSpec(const PackageDepSpec & other) :
    _imp(other)
{
}

PartiallyMadePackageDepSpec::~PartiallyMadePackageDepSpec()
{
}

PartiallyMadePackageDepSpec &
PartiallyMadePackageDepSpec::package(const QualifiedPackageName & name)
{
    _imp->data->package_name_part.reset();
    _imp->data->category_name_part.reset();
    _imp->data->package = NameConstraintPool::get_instance()->create(name);
    return *this;
}

PartiallyMadePackageDepSpec &
PartiallyMadePackageDepSpec::clear_package()
{
    _imp->data->package.reset();
    return *this;
}

PartiallyMadePackageDepSpec &
PartiallyMadePackageDepSpec::any_slot_constraint(const bool s)
{
    _imp->data->any_slot = AnySlotConstraintPool::get_instance()->create(s);
    return *this;
}

PartiallyMadePackageDepSpec &
PartiallyMadePackageDepSpec::exact_slot_constraint(const SlotName & n, const bool s)
{
    _imp->data->exact_slot = ExactSlotConstraintPool::get_instance()->create(n, s);
    return *this;
}

PartiallyMadePackageDepSpec &
PartiallyMadePackageDepSpec::clear_exact_slot()
{
    _imp->data->exact_slot.reset();
    return *this;
}

PartiallyMadePackageDepSpec &
PartiallyMadePackageDepSpec::clear_any_slot()
{
    _imp->data->any_slot.reset();
    return *this;
}

PartiallyMadePackageDepSpec &
PartiallyMadePackageDepSpec::in_repository(const RepositoryName & s)
{
    _imp->data->in_repository = InRepositoryConstraintPool::get_instance()->create(s);
    return *this;
}

PartiallyMadePackageDepSpec &
PartiallyMadePackageDepSpec::clear_in_repository()
{
    _imp->data->in_repository.reset();
    return *this;
}

PartiallyMadePackageDepSpec &
PartiallyMadePackageDepSpec::from_repository(const RepositoryName & s)
{
    _imp->data->from_repository = FromRepositoryConstraintPool::get_instance()->create(s);
    return *this;
}

PartiallyMadePackageDepSpec &
PartiallyMadePackageDepSpec::clear_from_repository()
{
    _imp->data->from_repository.reset();
    return *this;
}

PartiallyMadePackageDepSpec &
PartiallyMadePackageDepSpec::installable_to_repository(const RepositoryName & n, const bool i)
{
    _imp->data->installable_to_repository = InstallableToRepositoryConstraintPool::get_instance()->create(n, i);
    return *this;
}

PartiallyMadePackageDepSpec &
PartiallyMadePackageDepSpec::clear_installable_to_repository()
{
    _imp->data->installable_to_repository.reset();
    return *this;
}

PartiallyMadePackageDepSpec &
PartiallyMadePackageDepSpec::installed_at_path(const FSPath & s)
{
    _imp->data->installed_at_path = InstalledAtPathConstraintPool::get_instance()->create(s);
    return *this;
}

PartiallyMadePackageDepSpec &
PartiallyMadePackageDepSpec::clear_installed_at_path()
{
    _imp->data->installed_at_path.reset();
    return *this;
}

PartiallyMadePackageDepSpec &
PartiallyMadePackageDepSpec::installable_to_path(const FSPath & s, const bool i)
{
    _imp->data->installable_to_path = InstallableToPathConstraintPool::get_instance()->create(s, i);
    return *this;
}

PartiallyMadePackageDepSpec &
PartiallyMadePackageDepSpec::clear_installable_to_path()
{
    _imp->data->installable_to_path.reset();
    return *this;
}

PartiallyMadePackageDepSpec &
PartiallyMadePackageDepSpec::package_name_part(const PackageNamePart & part)
{
    _imp->data->package.reset();
    if (_imp->data->category_name_part)
    {
        _imp->data->package = NameConstraintPool::get_instance()->create(_imp->data->category_name_part->name_part() + part);
        _imp->data->category_name_part.reset();
    }
    else
        _imp->data->package_name_part = PackageNamePartConstraintPool::get_instance()->create(part);

    return *this;
}

PartiallyMadePackageDepSpec &
PartiallyMadePackageDepSpec::clear_package_name_part()
{
    _imp->data->package_name_part.reset();
    return *this;
}

PartiallyMadePackageDepSpec &
PartiallyMadePackageDepSpec::category_name_part(const CategoryNamePart & part)
{
    _imp->data->category_name_part.reset();
    if (_imp->data->package_name_part)
    {
        _imp->data->package = NameConstraintPool::get_instance()->create(part + _imp->data->package_name_part->name_part());
        _imp->data->package_name_part.reset();
    }
    else
        _imp->data->category_name_part = CategoryNamePartConstraintPool::get_instance()->create(part);

    return *this;
}

PartiallyMadePackageDepSpec &
PartiallyMadePackageDepSpec::clear_category_name_part()
{
    _imp->data->category_name_part.reset();
    return *this;
}

PartiallyMadePackageDepSpec &
PartiallyMadePackageDepSpec::version_constraint(const VersionSpec & vs, const VersionOperator & vo, const VersionConstraintCombiner vc)
{
    if (! _imp->data->all_versions)
        _imp->data->all_versions = std::make_shared<VersionConstraintSequence>();

    if (_imp->data->all_versions->empty() && vc != vcc_and)
        throw InternalError(PALUDIS_HERE, "First vc must be vcc_and");

    _imp->data->all_versions->push_back(std::make_shared<VersionConstraint>(vs, vo, vc));
    return *this;
}

PartiallyMadePackageDepSpec &
PartiallyMadePackageDepSpec::clear_version()
{
    _imp->data->all_versions.reset();
    return *this;
}

PartiallyMadePackageDepSpec &
PartiallyMadePackageDepSpec::key_constraint(const std::string & k, const KeyConstraintOperation o, const std::string & p)
{
    if (! _imp->data->all_keys)
        _imp->data->all_keys = std::make_shared<KeyConstraintSequence>();
    _imp->data->all_keys->push_back(KeyConstraintPool::get_instance()->create(k, o, p));
    return *this;
}

PartiallyMadePackageDepSpec &
PartiallyMadePackageDepSpec::choice_constraint(const std::shared_ptr<const ChoiceConstraint> & c)
{
    if (! _imp->data->all_choices)
        _imp->data->all_choices = std::make_shared<ChoiceConstraintSequence>();
    _imp->data->all_choices->push_back(c);
    return *this;
}

PartiallyMadePackageDepSpec &
PartiallyMadePackageDepSpec::clear_choices()
{
    _imp->data->all_choices.reset();
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

template class Pimp<PartiallyMadePackageDepSpec>;

