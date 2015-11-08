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
        std::shared_ptr<const QualifiedPackageName> package;
        std::shared_ptr<const PackageNamePart> package_name_part;
        std::shared_ptr<const CategoryNamePart> category_name_part;
        std::shared_ptr<VersionRequirements> version_requirements;
        VersionRequirementsMode version_requirements_mode_v;
        std::shared_ptr<const SlotRequirement> slot;
        std::shared_ptr<const RepositoryName> in_repository;
        std::shared_ptr<const RepositoryName> from_repository;
        std::shared_ptr<const InstallableToRepository> installable_to_repository;
        std::shared_ptr<const FSPath> installed_at_path;
        std::shared_ptr<const InstallableToPath> installable_to_path;
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
            package(other.package_ptr()),
            package_name_part(other.package_name_part_ptr()),
            category_name_part(other.category_name_part_ptr()),
            version_requirements(other.version_requirements_ptr() ? new VersionRequirements : 0),
            version_requirements_mode_v(other.version_requirements_mode()),
            slot(other.slot_requirement_ptr()),
            in_repository(other.in_repository_ptr()),
            from_repository(other.from_repository_ptr()),
            installable_to_repository(other.installable_to_repository_ptr()),
            installed_at_path(other.installed_at_path_ptr()),
            installable_to_path(other.installable_to_path_ptr()),
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
            slot(other.slot),
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
                    if (version_requirements_ptr()->begin()->version_operator() == vo_equal_star)
                        s << "=";
                    else
                        s << version_requirements_ptr()->begin()->version_operator();
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
                else if (next(version_requirements_ptr()->begin()) == version_requirements_ptr()->end() &&
                        ! options_for_partially_made_package_dep_spec_v[pmpdso_always_use_ranged_deps])
                {
                    s << "-" << version_requirements_ptr()->begin()->version_spec();
                    if (version_requirements_ptr()->begin()->version_operator() == vo_equal_star)
                        s << "*";
                }
            }

            if (slot_requirement_ptr())
                s << stringify(*slot_requirement_ptr());

            std::string left, right;
            bool need_arrow(false);

            if (from_repository_ptr())
                left = stringify(*from_repository_ptr());

            if (in_repository_ptr())
                right = stringify(*in_repository_ptr());

            if (installed_at_path_ptr())
            {
                if (! right.empty())
                {
                    need_arrow = true;
                    right.append("->");
                }
                right.append(stringify(*installed_at_path_ptr()));
            }

            if (installable_to_repository_ptr())
            {
                if (! right.empty())
                {
                    need_arrow = true;
                    right.append("->");
                }
                if (installable_to_repository_ptr()->include_masked())
                    right.append(stringify(installable_to_repository_ptr()->repository()) + "??");
                else
                    right.append(stringify(installable_to_repository_ptr()->repository()) + "?");
            }

            if (installable_to_path_ptr())
            {
                if (! right.empty())
                {
                    need_arrow = true;
                    right.append("->");
                }
                if (installable_to_path_ptr()->include_masked())
                    right.append(stringify(installable_to_path_ptr()->path()) + "??");
                else
                    right.append(stringify(installable_to_path_ptr()->path()) + "?");
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

                        if (r->version_operator() == vo_equal_star)
                            s << "=";
                        else
                            s << r->version_operator();

                        s << r->version_spec();

                        if (r->version_operator() == vo_equal_star)
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

        virtual std::shared_ptr<const QualifiedPackageName> package_ptr() const
        {
            return package;
        }

        virtual std::shared_ptr<const PackageNamePart> package_name_part_ptr() const
        {
            return package_name_part;
        }

        virtual std::shared_ptr<const CategoryNamePart> category_name_part_ptr() const
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

        virtual std::shared_ptr<const SlotRequirement> slot_requirement_ptr() const
        {
            return slot;
        }

        virtual std::shared_ptr<const RepositoryName> in_repository_ptr() const
        {
            return in_repository;
        }

        virtual std::shared_ptr<const InstallableToRepository> installable_to_repository_ptr() const
        {
            return installable_to_repository;
        }

        virtual std::shared_ptr<const RepositoryName> from_repository_ptr() const
        {
            return from_repository;
        }

        virtual std::shared_ptr<const FSPath> installed_at_path_ptr() const
        {
            return installed_at_path;
        }

        virtual std::shared_ptr<const InstallableToPath> installable_to_path_ptr() const
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
    _imp->data->package = std::make_shared<QualifiedPackageName>(name);
    return *this;
}

PartiallyMadePackageDepSpec &
PartiallyMadePackageDepSpec::clear_package()
{
    _imp->data->package.reset();
    return *this;
}

PartiallyMadePackageDepSpec &
PartiallyMadePackageDepSpec::slot_requirement(const std::shared_ptr<const SlotRequirement> & s)
{
    _imp->data->slot = s;
    return *this;
}

PartiallyMadePackageDepSpec &
PartiallyMadePackageDepSpec::clear_slot_requirement()
{
    _imp->data->slot.reset();
    return *this;
}

PartiallyMadePackageDepSpec &
PartiallyMadePackageDepSpec::in_repository(const RepositoryName & s)
{
    _imp->data->in_repository = std::make_shared<RepositoryName>(s);
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
    _imp->data->from_repository = std::make_shared<RepositoryName>(s);
    return *this;
}

PartiallyMadePackageDepSpec &
PartiallyMadePackageDepSpec::clear_from_repository()
{
    _imp->data->from_repository.reset();
    return *this;
}

PartiallyMadePackageDepSpec &
PartiallyMadePackageDepSpec::installable_to_repository(const InstallableToRepository & s)
{
    _imp->data->installable_to_repository = std::make_shared<InstallableToRepository>(s);
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
    _imp->data->installed_at_path = std::make_shared<FSPath>(s);
    return *this;
}

PartiallyMadePackageDepSpec &
PartiallyMadePackageDepSpec::clear_installed_at_path()
{
    _imp->data->installed_at_path.reset();
    return *this;
}

PartiallyMadePackageDepSpec &
PartiallyMadePackageDepSpec::installable_to_path(const InstallableToPath & s)
{
    _imp->data->installable_to_path = std::make_shared<InstallableToPath>(s);
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
    _imp->data->package_name_part = std::make_shared<PackageNamePart>(part);
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
    _imp->data->category_name_part = std::make_shared<CategoryNamePart>(part);
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

namespace paludis
{
    template class Pimp<PartiallyMadePackageDepSpec>;
}
