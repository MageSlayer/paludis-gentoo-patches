/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008 Ciaran McCreesh
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
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/join.hh>
#include <paludis/util/make_shared_ptr.hh>
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
        tr1::shared_ptr<QualifiedPackageName> package;
        tr1::shared_ptr<VersionRequirements> version_requirements;
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

            return s.str() + " (rewritten from { " + join(strings.begin(), strings.end(), ", ") + " })";
        }

        virtual tr1::shared_ptr<const QualifiedPackageName> package_ptr() const
        {
            return package;
        }

        virtual tr1::shared_ptr<const PackageNamePart> package_name_part_ptr() const
        {
            return tr1::shared_ptr<const PackageNamePart>();
        }

        virtual tr1::shared_ptr<const CategoryNamePart> category_name_part_ptr() const
        {
            return tr1::shared_ptr<const CategoryNamePart>();
        }

        virtual tr1::shared_ptr<const VersionRequirements> version_requirements_ptr() const
        {
            return version_requirements;
        }

        virtual VersionRequirementsMode version_requirements_mode() const
        {
            return version_requirements_mode_v;
        }

        virtual tr1::shared_ptr<const SlotRequirement> slot_requirement_ptr() const
        {
            return tr1::shared_ptr<const SlotRequirement>();
        }

        virtual tr1::shared_ptr<const RepositoryName> repository_ptr() const
        {
            return tr1::shared_ptr<const RepositoryName>();
        }

        virtual tr1::shared_ptr<const AdditionalPackageDepSpecRequirements> additional_requirements_ptr() const
        {
            return tr1::shared_ptr<const AdditionalPackageDepSpecRequirements>();
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

        tr1::shared_ptr<const PackageDepSpecData> without_additional_requirements() const
        {
            return make_shared_ptr(new RangeRewrittenPackageDepSpecData(*this));
        }

        tr1::shared_ptr<const PackageDepSpecData> without_slot_requirements() const
        {
            return make_shared_ptr(new RangeRewrittenPackageDepSpecData(*this));
        }
    };
}

namespace paludis
{
    template <>
    struct Implementation<RangeRewriter>
    {
        bool invalid;
        tr1::shared_ptr<RangeRewrittenPackageDepSpecData> spec_data;

        Implementation() :
            invalid(false)
        {
        }
    };
}

RangeRewriter::RangeRewriter() :
    PrivateImplementationPattern<RangeRewriter>(new Implementation<RangeRewriter>())
{
}

RangeRewriter::~RangeRewriter()
{
}

void
RangeRewriter::visit_sequence(const AllDepSpec &,
        DependencySpecTree::ConstSequenceIterator cur,
        DependencySpecTree::ConstSequenceIterator end)
{
    if (cur != end)
        _imp->invalid = true;
}

void
RangeRewriter::visit_sequence(const AnyDepSpec &,
        DependencySpecTree::ConstSequenceIterator cur,
        DependencySpecTree::ConstSequenceIterator end)
{
    if (cur != end)
        _imp->invalid = true;
}

void
RangeRewriter::visit_sequence(const ConditionalDepSpec &,
        DependencySpecTree::ConstSequenceIterator,
        DependencySpecTree::ConstSequenceIterator)
{
    _imp->invalid = true;
}

void
RangeRewriter::visit_leaf(const PackageDepSpec & a)
{
    if (_imp->invalid)
        return;

    if (a.additional_requirements_ptr() || a.slot_requirement_ptr() || a.repository_ptr() || a.package_name_part_ptr()
            || a.category_name_part_ptr() || ! a.version_requirements_ptr() || ! a.package_ptr())
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
        _imp->spec_data.reset(new RangeRewrittenPackageDepSpecData(a));
    }
}

void
RangeRewriter::visit_leaf(const BlockDepSpec &)
{
    _imp->invalid = true;
}

void
RangeRewriter::visit_leaf(const DependencyLabelsDepSpec &)
{
    _imp->invalid = true;
}

void
RangeRewriter::visit_leaf(const NamedSetDepSpec &)
{
    _imp->invalid = true;
}

tr1::shared_ptr<PackageDepSpec>
RangeRewriter::spec() const
{
    if (_imp->invalid || ! _imp->spec_data)
        return tr1::shared_ptr<PackageDepSpec>();

    return tr1::shared_ptr<PackageDepSpec>(new PackageDepSpec(_imp->spec_data));
}

