/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010 Ciaran McCreesh
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

#include <paludis/range_rewriter.hh>
#include <paludis/util/sequence.hh>
#include <paludis/version_requirements.hh>
#include <paludis/package_dep_spec_properties.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/join.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/options.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/dep_spec.hh>
#include <list>
#include <sstream>
#include <algorithm>

using namespace paludis;

namespace
{
    struct RangeRewrittenPackageDepSpecData :
        PackageDepSpecData
    {
        std::shared_ptr<QualifiedPackageName> package;
        std::shared_ptr<VersionRequirements> version_requirements;
        VersionRequirementsMode version_requirements_mode_v;
        std::list<std::string> strings;

        virtual std::string as_string() const
        {
            std::ostringstream s;
            s << *package_ptr();

            if (version_requirements_ptr())
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

            return s.str() + " (rewritten from { " + join(strings.begin(), strings.end(), ", ") + " })";
        }

        virtual std::shared_ptr<const QualifiedPackageName> package_ptr() const
        {
            return package;
        }

        virtual std::shared_ptr<const PackageNamePart> package_name_part_ptr() const
        {
            return std::shared_ptr<const PackageNamePart>();
        }

        virtual std::shared_ptr<const CategoryNamePart> category_name_part_ptr() const
        {
            return std::shared_ptr<const CategoryNamePart>();
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
            return std::shared_ptr<const SlotRequirement>();
        }

        virtual std::shared_ptr<const MetadataSectionKey> annotations_key() const
        {
            return std::shared_ptr<const MetadataSectionKey>();
        }

        virtual std::shared_ptr<const AdditionalPackageDepSpecRequirements> additional_requirements_ptr() const
        {
            return std::shared_ptr<const AdditionalPackageDepSpecRequirements>();
        }

        void add_spec(const PackageDepSpec & spec)
        {
            strings.push_back(stringify(spec));
            std::copy(spec.version_requirements_ptr()->begin(), spec.version_requirements_ptr()->end(),
                    version_requirements->back_inserter());
        }

        RangeRewrittenPackageDepSpecData(const PackageDepSpec & spec) :
            package(new QualifiedPackageName(*spec.package_ptr())),
            version_requirements(new VersionRequirements),
            version_requirements_mode_v(vr_or)
        {
            strings.push_back(stringify(spec));
            std::copy(spec.version_requirements_ptr()->begin(), spec.version_requirements_ptr()->end(),
                    version_requirements->back_inserter());
        }

        std::shared_ptr<const PackageDepSpecData> without_slot_requirements() const
        {
            return std::make_shared<RangeRewrittenPackageDepSpecData>(*this);
        }

        virtual std::shared_ptr<const RepositoryName> in_repository_ptr() const
        {
            return make_null_shared_ptr();
        }

        virtual std::shared_ptr<const InstallableToRepository> installable_to_repository_ptr() const
        {
            return make_null_shared_ptr();
        }

        virtual std::shared_ptr<const RepositoryName> from_repository_ptr() const
        {
            return make_null_shared_ptr();
        }

        virtual std::shared_ptr<const FSEntry> installed_at_path_ptr() const
        {
            return make_null_shared_ptr();
        }

        virtual std::shared_ptr<const InstallableToPath> installable_to_path_ptr() const
        {
            return make_null_shared_ptr();
        }

        virtual const PartiallyMadePackageDepSpecOptions options_for_partially_made_package_dep_spec() const
        {
            return PartiallyMadePackageDepSpecOptions() + pmpdso_always_use_ranged_deps;
        }
    };
}

namespace paludis
{
    template <>
    struct Imp<RangeRewriter>
    {
        bool invalid;
        std::shared_ptr<RangeRewrittenPackageDepSpecData> spec_data;

        Imp() :
            invalid(false)
        {
        }
    };
}

RangeRewriter::RangeRewriter() :
    Pimp<RangeRewriter>()
{
}

RangeRewriter::~RangeRewriter()
{
}

void
RangeRewriter::visit(const DependencySpecTree::NodeType<AllDepSpec>::Type & node)
{
    if (node.begin() != node.end())
        _imp->invalid = true;
}

void
RangeRewriter::visit(const DependencySpecTree::NodeType<AnyDepSpec>::Type & node)
{
    if (node.begin() != node.end())
        _imp->invalid = true;
}

void
RangeRewriter::visit(const DependencySpecTree::NodeType<ConditionalDepSpec>::Type &)
{
    _imp->invalid = true;
}

void
RangeRewriter::visit(const DependencySpecTree::NodeType<PackageDepSpec>::Type & node)
{
    if (_imp->invalid)
        return;

    const PackageDepSpec & a(*node.spec());
    if (! package_dep_spec_has_properties(a, make_named_values<PackageDepSpecProperties>(
                    n::has_additional_requirements() = false,
                    n::has_category_name_part() = false,
                    n::has_from_repository() = false,
                    n::has_in_repository() = false,
                    n::has_installable_to_path() = false,
                    n::has_installable_to_repository() = false,
                    n::has_installed_at_path() = false,
                    n::has_package() = true,
                    n::has_package_name_part() = false,
                    n::has_slot_requirement() = false,
                    n::has_tag() = indeterminate,
                    n::has_version_requirements() = true
                    )))
    {
        _imp->invalid = true;
        return;
    }

    if (a.version_requirements_mode() == vr_and &&
            1 != std::distance(a.version_requirements_ptr()->begin(), a.version_requirements_ptr()->end()))
    {
        _imp->invalid = true;
        return;
    }

    if (_imp->spec_data)
    {
        if (*_imp->spec_data->package_ptr() != *a.package_ptr())
        {
            _imp->invalid = true;
            return;
        }

        _imp->spec_data->add_spec(a);
    }
    else
    {
        _imp->spec_data = std::make_shared<RangeRewrittenPackageDepSpecData>(a);
    }
}

void
RangeRewriter::visit(const DependencySpecTree::NodeType<BlockDepSpec>::Type &)
{
    _imp->invalid = true;
}

void
RangeRewriter::visit(const DependencySpecTree::NodeType<DependenciesLabelsDepSpec>::Type &)
{
    _imp->invalid = true;
}

void
RangeRewriter::visit(const DependencySpecTree::NodeType<NamedSetDepSpec>::Type &)
{
    _imp->invalid = true;
}

std::shared_ptr<PackageDepSpec>
RangeRewriter::spec() const
{
    if (_imp->invalid || ! _imp->spec_data)
        return std::shared_ptr<PackageDepSpec>();

    return std::shared_ptr<PackageDepSpec>(new PackageDepSpec(_imp->spec_data));
}

