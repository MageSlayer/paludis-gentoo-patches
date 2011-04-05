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

#include <paludis/dep_spec_data.hh>
#include <paludis/package_dep_spec_constraint.hh>
#include <paludis/dep_spec.hh>
#include <paludis/version_operator.hh>
#include <paludis/version_spec.hh>

#include <paludis/util/stringify.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/options.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/wrapped_output_iterator.hh>

#include <istream>
#include <ostream>

using namespace paludis;

#include <paludis/dep_spec_data-se.cc>

ConditionalDepSpecData::~ConditionalDepSpecData() = default;

namespace paludis
{
    template <>
    struct Imp<PackageDepSpecData>
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
        PackageDepSpecDataOptions options;

        Imp(const PackageDepSpecDataOptions & o) :
            options(o)
        {
        }

        Imp(const PackageDepSpecData & other) :
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
            options(other.options())
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
    };
}

PackageDepSpecData::PackageDepSpecData(const PackageDepSpecDataOptions & o) :
    _imp(o)
{
}

PackageDepSpecData::PackageDepSpecData(const PackageDepSpecData & o) :
    _imp(o)
{
}

PackageDepSpecData::~PackageDepSpecData() = default;

const std::shared_ptr<const NameConstraint>
PackageDepSpecData::package_name_constraint() const
{
    return _imp->package;
}

const std::shared_ptr<const PackageNamePartConstraint>
PackageDepSpecData::package_name_part_constraint() const
{
    return _imp->package_name_part;
}

const std::shared_ptr<const CategoryNamePartConstraint>
PackageDepSpecData::category_name_part_constraint() const
{
    return _imp->category_name_part;
}

const std::shared_ptr<const VersionConstraintSequence>
PackageDepSpecData::all_version_constraints() const
{
    return _imp->all_versions;
}

const std::shared_ptr<const ExactSlotConstraint>
PackageDepSpecData::exact_slot_constraint() const
{
    return _imp->exact_slot;
}

const std::shared_ptr<const AnySlotConstraint>
PackageDepSpecData::any_slot_constraint() const
{
    return _imp->any_slot;
}

const std::shared_ptr<const InRepositoryConstraint>
PackageDepSpecData::in_repository_constraint() const
{
    return _imp->in_repository;
}

const std::shared_ptr<const InstallableToRepositoryConstraint>
PackageDepSpecData::installable_to_repository_constraint() const
{
    return _imp->installable_to_repository;
}

const std::shared_ptr<const FromRepositoryConstraint>
PackageDepSpecData::from_repository_constraint() const
{
    return _imp->from_repository;
}

const std::shared_ptr<const InstalledAtPathConstraint>
PackageDepSpecData::installed_at_path_constraint() const
{
    return _imp->installed_at_path;
}

const std::shared_ptr<const InstallableToPathConstraint>
PackageDepSpecData::installable_to_path_constraint() const
{
    return _imp->installable_to_path;
}

const std::shared_ptr<const KeyConstraintSequence>
PackageDepSpecData::all_key_constraints() const
{
    return _imp->all_keys;
}

const std::shared_ptr<const ChoiceConstraintSequence>
PackageDepSpecData::all_choice_constraints() const
{
    return _imp->all_choices;
}

const PackageDepSpecDataOptions
PackageDepSpecData::options() const
{
    return _imp->options;
}

std::string
PackageDepSpecData::as_string() const
{
    std::ostringstream s;

    if (all_version_constraints())
    {
        if (all_version_constraints()->begin() == all_version_constraints()->end())
        {
        }
        else if (next(all_version_constraints()->begin()) == all_version_constraints()->end() &&
                ! options()[pdsdo_always_use_ranged_deps])
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
                ! options()[pdsdo_always_use_ranged_deps])
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
                ! options()[pdsdo_always_use_ranged_deps])
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
            s << (*u)->as_raw_string();

    return s.str();
}


MutablePackageDepSpecData::MutablePackageDepSpecData(const PackageDepSpecDataOptions & o) :
    PackageDepSpecData(o)
{
}

MutablePackageDepSpecData::MutablePackageDepSpecData(const PackageDepSpecData & o) :
    PackageDepSpecData(o)
{
}

MutablePackageDepSpecData::MutablePackageDepSpecData(const MutablePackageDepSpecData & o) :
    PackageDepSpecData(o)
{
}

MutablePackageDepSpecData::~MutablePackageDepSpecData() = default;

MutablePackageDepSpecData &
MutablePackageDepSpecData::constrain_package(const QualifiedPackageName & name)
{
    _imp->package = NameConstraintPool::get_instance()->create(name);
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::unconstrain_package()
{
    _imp->package.reset();
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::constrain_package_name_part(const PackageNamePart & part)
{
    _imp->package_name_part = PackageNamePartConstraintPool::get_instance()->create(part);
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::unconstrain_package_name_part()
{
    _imp->package_name_part.reset();
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::constrain_category_name_part(const CategoryNamePart & part)
{
    _imp->category_name_part = CategoryNamePartConstraintPool::get_instance()->create(part);
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::unconstrain_category_name_part()
{
    _imp->category_name_part.reset();
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::constrain_version(const VersionConstraintCombiner vc, const VersionOperator & vo, const VersionSpec & vs)
{
    if (! _imp->all_versions)
        _imp->all_versions = std::make_shared<VersionConstraintSequence>();
    _imp->all_versions->push_back(std::make_shared<VersionConstraint>(vs, vo, vc));
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::unconstrain_versions()
{
    _imp->all_versions.reset();
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::constrain_exact_slot(const SlotName & n, const bool s)
{
    _imp->exact_slot = ExactSlotConstraintPool::get_instance()->create(n, s);
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::unconstrain_exact_slot()
{
    _imp->exact_slot.reset();
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::constrain_in_repository(const RepositoryName & s)
{
    _imp->in_repository = InRepositoryConstraintPool::get_instance()->create(s);
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::unconstrain_in_repository()
{
    _imp->in_repository.reset();
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::constrain_installable_to_path(const FSPath & s, const bool i)
{
    _imp->installable_to_path = InstallableToPathConstraintPool::get_instance()->create(s, i);
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::unconstrain_installable_to_path()
{
    _imp->installable_to_path.reset();
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::constrain_installable_to_repository(const RepositoryName & n, const bool i)
{
    _imp->installable_to_repository = InstallableToRepositoryConstraintPool::get_instance()->create(n, i);
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::unconstrain_installable_to_repository()
{
    _imp->installable_to_repository.reset();
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::constrain_from_repository(const RepositoryName & n)
{
    _imp->from_repository = FromRepositoryConstraintPool::get_instance()->create(n);
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::unconstrain_from_repository()
{
    _imp->from_repository.reset();
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::constrain_installed_at_path(const FSPath & s)
{
    _imp->installed_at_path = InstalledAtPathConstraintPool::get_instance()->create(s);
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::unconstrain_installed_at_path()
{
    _imp->installed_at_path.reset();
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::constrain_any_slot(const bool s)
{
    _imp->any_slot = AnySlotConstraintPool::get_instance()->create(s);
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::unconstrain_any_slot()
{
    _imp->any_slot.reset();
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::constrain_choice(const std::shared_ptr<const ChoiceConstraint> & c)
{
    if (! _imp->all_choices)
        _imp->all_choices = std::make_shared<ChoiceConstraintSequence>();
    _imp->all_choices->push_back(c);
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::unconstrain_choices()
{
    _imp->all_choices.reset();
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::constrain_key(const KeyConstraintKeyType t, const std::string & k, const KeyConstraintOperation o, const std::string & p)
{
    if (! _imp->all_keys)
        _imp->all_keys = std::make_shared<KeyConstraintSequence>();
    _imp->all_keys->push_back(KeyConstraintPool::get_instance()->create(t, k, o, p));
    return *this;
}

MutablePackageDepSpecData &
MutablePackageDepSpecData::unconstrain_keys()
{
    _imp->all_keys.reset();
    return *this;
}

MutablePackageDepSpecData::operator PackageDepSpec() const
{
    return PackageDepSpec(std::make_shared<MutablePackageDepSpecData>(*this));
}

