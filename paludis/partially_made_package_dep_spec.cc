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
#include <paludis/util/stringify.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/options.hh>
#include <paludis/util/wrapped_output_iterator-impl.hh>
#include <paludis/util/iterator_funcs.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/sequence-impl.hh>
#include <paludis/version_requirements.hh>
#include <paludis/additional_package_dep_spec_requirement.hh>
#include <paludis/dep_spec_data.hh>
#include <paludis/package_dep_spec_constraint.hh>
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
        std::shared_ptr<VersionRequirements> version_requirements;
        VersionRequirementsMode version_requirements_mode_v;
        std::shared_ptr<const AnySlotConstraint> any_slot;
        std::shared_ptr<const ExactSlotConstraint> exact_slot;
        std::shared_ptr<const InRepositoryConstraint> in_repository;
        std::shared_ptr<const FromRepositoryConstraint> from_repository;
        std::shared_ptr<const InstallableToRepositoryConstraint> installable_to_repository;
        std::shared_ptr<const InstalledAtPathConstraint> installed_at_path;
        std::shared_ptr<const InstallableToPathConstraint> installable_to_path;
        std::shared_ptr<AdditionalPackageDepSpecRequirements> additional_requirements;
        PartiallyMadePackageDepSpecOptions options_for_partially_made_package_dep_spec_v;

        PartiallyMadePackageDepSpecData(const PartiallyMadePackageDepSpecOptions & o) :
            PackageDepSpecData(),
            version_requirements_mode_v(vr_and),
            options_for_partially_made_package_dep_spec_v(o)
        {
        }

        PartiallyMadePackageDepSpecData(const PackageDepSpecData & other) :
            PackageDepSpecData(other),
            package(other.package_name_constraint()),
            package_name_part(other.package_name_part_constraint()),
            category_name_part(other.category_name_part_constraint()),
            version_requirements(other.version_requirements_ptr() ? new VersionRequirements : 0),
            version_requirements_mode_v(other.version_requirements_mode()),
            any_slot(other.any_slot_constraint()),
            exact_slot(other.exact_slot_constraint()),
            in_repository(other.in_repository_constraint()),
            from_repository(other.from_repository_constraint()),
            installable_to_repository(other.installable_to_repository_constraint()),
            installed_at_path(other.installed_at_path_constraint()),
            installable_to_path(other.installable_to_path_constraint()),
            additional_requirements(other.additional_requirements_ptr() ? new AdditionalPackageDepSpecRequirements : 0),
            options_for_partially_made_package_dep_spec_v(other.options_for_partially_made_package_dep_spec())
        {
            if (version_requirements)
                std::copy(other.version_requirements_ptr()->begin(), other.version_requirements_ptr()->end(),
                        version_requirements->back_inserter());

            if (additional_requirements)
                std::copy(other.additional_requirements_ptr()->begin(), other.additional_requirements_ptr()->end(),
                        additional_requirements->back_inserter());
        }

        PartiallyMadePackageDepSpecData(const PartiallyMadePackageDepSpecData & other) :
            PackageDepSpecData(other),
            package(other.package),
            package_name_part(other.package_name_part),
            category_name_part(other.category_name_part),
            version_requirements(other.version_requirements),
            version_requirements_mode_v(other.version_requirements_mode_v),
            any_slot(other.any_slot),
            exact_slot(other.exact_slot),
            in_repository(other.in_repository),
            from_repository(other.from_repository),
            installable_to_repository(other.installable_to_repository),
            installed_at_path(other.installed_at_path),
            installable_to_path(other.installable_to_path),
            additional_requirements(other.additional_requirements),
            options_for_partially_made_package_dep_spec_v(other.options_for_partially_made_package_dep_spec_v)
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
                else if (next(version_requirements_ptr()->begin()) == version_requirements_ptr()->end() &&
                        ! options_for_partially_made_package_dep_spec_v[pmpdso_always_use_ranged_deps])
                {
                    if (version_requirements_ptr()->begin()->version_operator() == vo_stupid_equal_star || version_requirements_ptr()->begin()->version_operator() == vo_nice_equal_star)
                        s << "=";
                    else
                        s << version_requirements_ptr()->begin()->version_operator();
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

            if (version_requirements_ptr())
            {
                if (version_requirements_ptr()->begin() == version_requirements_ptr()->end())
                {
                }
                else if (next(version_requirements_ptr()->begin()) == version_requirements_ptr()->end() &&
                        ! options_for_partially_made_package_dep_spec_v[pmpdso_always_use_ranged_deps])
                {
                    s << "-" << version_requirements_ptr()->begin()->version_spec();
                    if (version_requirements_ptr()->begin()->version_operator() == vo_stupid_equal_star || version_requirements_ptr()->begin()->version_operator() == vo_nice_equal_star)
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

            if (version_requirements_ptr())
            {
                if (version_requirements_ptr()->begin() == version_requirements_ptr()->end())
                {
                }
                else if (next(version_requirements_ptr()->begin()) == version_requirements_ptr()->end() &&
                        ! options_for_partially_made_package_dep_spec_v[pmpdso_always_use_ranged_deps])
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

                        if (r->version_operator() == vo_stupid_equal_star || r->version_operator() == vo_nice_equal_star)
                            s << "=";
                        else
                            s << r->version_operator();

                        s << r->version_spec();

                        if (r->version_operator() == vo_stupid_equal_star || r->version_operator() == vo_nice_equal_star)
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

        virtual std::shared_ptr<const VersionRequirements> version_requirements_ptr() const
        {
            return version_requirements;
        }

        virtual VersionRequirementsMode version_requirements_mode() const
        {
            return version_requirements_mode_v;
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

        virtual std::shared_ptr<const AdditionalPackageDepSpecRequirements> additional_requirements_ptr() const
        {
            return additional_requirements;
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
PartiallyMadePackageDepSpec::version_requirement(const VersionRequirement & req)
{
    if (! _imp->data->version_requirements)
        _imp->data->version_requirements = std::make_shared<VersionRequirements>();
    _imp->data->version_requirements->push_back(req);
    return *this;
}

PartiallyMadePackageDepSpec &
PartiallyMadePackageDepSpec::clear_version_requirements()
{
    _imp->data->version_requirements.reset();
    return *this;
}

PartiallyMadePackageDepSpec &
PartiallyMadePackageDepSpec::version_requirements_mode(const VersionRequirementsMode & mode)
{
    _imp->data->version_requirements_mode_v = mode;
    return *this;
}

PartiallyMadePackageDepSpec &
PartiallyMadePackageDepSpec::additional_requirement(const std::shared_ptr<const AdditionalPackageDepSpecRequirement> & req)
{
    if (! _imp->data->additional_requirements)
        _imp->data->additional_requirements = std::make_shared<AdditionalPackageDepSpecRequirements>();
    _imp->data->additional_requirements->push_back(req);
    return *this;
}

PartiallyMadePackageDepSpec &
PartiallyMadePackageDepSpec::clear_additional_requirements()
{
    _imp->data->additional_requirements.reset();
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

