/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include "dep_tag.hh"
#include <paludis/dep_spec.hh>
#include <paludis/dep_spec_pretty_printer.hh>
#include <paludis/util/virtual_constructor-impl.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/instantiation_policy-impl.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>

/** \file
 * Implementation for DepTag, DepTagCategory etc.
 *
 * \ingroup grpdeptag
 */

using namespace paludis;

template class VirtualConstructor<std::string, tr1::shared_ptr<const DepTagCategory> (*) (),
         virtual_constructor_not_found::ThrowException<NoSuchDepTagCategory> >;

template class ConstVisitor<DepTagVisitorTypes>;
template class ConstAcceptInterface<DepTagVisitorTypes>;

template class ConstAcceptInterfaceVisitsThis<DepTagVisitorTypes, GeneralSetDepTag>;
template class ConstAcceptInterfaceVisitsThis<DepTagVisitorTypes, GLSADepTag>;
template class ConstAcceptInterfaceVisitsThis<DepTagVisitorTypes, DependencyDepTag>;
template class ConstAcceptInterfaceVisitsThis<DepTagVisitorTypes, TargetDepTag>;

template class Visits<const GeneralSetDepTag>;
template class Visits<const GLSADepTag>;
template class Visits<const DependencyDepTag>;
template class Visits<const TargetDepTag>;

template class InstantiationPolicy<DepTagCategoryMaker, instantiation_method::SingletonTag>;

#include <paludis/dep_tag-sr.cc>

namespace
{
    /**
     * Create the DepTagCategory for GLSAs.
     *
     * \ingroup grpdeptag
     */
    tr1::shared_ptr<const DepTagCategory>
    make_glsa_dep_tag()
    {
        return tr1::shared_ptr<const DepTagCategory>(new DepTagCategory(
                    true,
                    "glsa",
                    "Security advisories",
                    "Your system is potentially affected by these security issues:",
                    "Please read the advisories carefully and take appropriate action."));
    }

    /**
     * Create the DepTagCategory for general sets.
     *
     * \ingroup grpdeptag
     */
    tr1::shared_ptr<const DepTagCategory>
    make_general_set_dep_tag()
    {
        return tr1::shared_ptr<const DepTagCategory>(new DepTagCategory(
                    true,
                    "general",
                    "General sets",
                    "",
                    ""));
    }

    /**
     * Create the DepTagCategory for dependency sets.
     *
     * \ingroup grpdeptag
     */
    tr1::shared_ptr<const DepTagCategory>
    make_dependency_set_dep_tag()
    {
        return tr1::shared_ptr<const DepTagCategory>(new DepTagCategory(
                    false,
                    "dependency",
                    "Dependencies",
                    "",
                    ""));
    }

    /**
     * Create the DepTagCategory for targets.
     *
     * \ingroup grpdeptag
     */
    tr1::shared_ptr<const DepTagCategory>
    make_target_dep_tag()
    {
        return tr1::shared_ptr<const DepTagCategory>(new DepTagCategory(
                    false,
                    "target",
                    "Targets",
                    "",
                    ""));
    }
}

DepTagCategory::DepTagCategory(
        bool vis,
        const std::string & our_id,
        const std::string & t, const std::string & pre,
        const std::string & post) :
    _visible(vis),
    _id(our_id),
    _title(t),
    _pre_text(pre),
    _post_text(post)
{
}

bool
DepTagCategory::visible() const
{
    return _visible;
}

std::string
DepTagCategory::id() const
{
    return _id;
}

std::string
DepTagCategory::title() const
{
    return _title;
}

std::string
DepTagCategory::pre_text() const
{
    return _pre_text;
}

std::string
DepTagCategory::post_text() const
{
    return _post_text;
}

NoSuchDepTagCategory::NoSuchDepTagCategory(const std::string & s) throw () :
    Exception("No such dep tag category '" + s + "'")
{
}

DepTag::DepTag()
{
}

DepTag::~DepTag()
{
}

std::string
DepTag::full_text() const
{
    return short_text();
}

bool
DepTag::operator== (const DepTag & other) const
{
    return full_text() == other.full_text();
}

bool
DepTag::operator< (const DepTag & other) const
{
    return full_text() < other.full_text();
}

GLSADepTag::GLSADepTag(const std::string & id, const std::string & our_glsa_title) :
    _id(id),
    _glsa_title(our_glsa_title)
{
}

GLSADepTag::~GLSADepTag()
{
}

std::string
GLSADepTag::short_text() const
{
    return "GLSA-" + _id;
}

std::string
GLSADepTag::category() const
{
    return "glsa";
}

std::string
GLSADepTag::glsa_title() const
{
    return _glsa_title;
}

namespace paludis
{
    template <>
    struct Implementation<GeneralSetDepTag>
    {
        const SetName id;
        const std::string source;

        Implementation(const SetName & n, const std::string s) :
            id(n),
            source(s)
        {
        }
    };
}

GeneralSetDepTag::GeneralSetDepTag(const SetName & id, const std::string & r) :
    PrivateImplementationPattern<GeneralSetDepTag>(new Implementation<GeneralSetDepTag>(id, r))
{
}

GeneralSetDepTag::~GeneralSetDepTag()
{
}

std::string
GeneralSetDepTag::short_text() const
{
    return stringify(_imp->id);
}

std::string
GeneralSetDepTag::category() const
{
    return "general";
}

std::string
GeneralSetDepTag::source() const
{
    return _imp->source;
}

namespace paludis
{
    template <>
    struct Implementation<DependencyDepTag>
    {
        mutable std::string str;

        tr1::shared_ptr<const PackageID> id;
        const tr1::shared_ptr<PackageDepSpec> spec;
        const tr1::shared_ptr<const DependencySpecTree::ConstItem> cond;

        Implementation(const tr1::shared_ptr<const PackageID> & i,
                const PackageDepSpec & d, const tr1::shared_ptr<const DependencySpecTree::ConstItem> & s) :
            id(i),
            spec(tr1::static_pointer_cast<PackageDepSpec>(d.clone())),
            cond(s)
        {
            spec->set_tag(tr1::shared_ptr<const DepTag>());
        }
    };
}

DependencyDepTag::DependencyDepTag(const tr1::shared_ptr<const PackageID> & i, const PackageDepSpec & d,
        const tr1::shared_ptr<const DependencySpecTree::ConstItem> & s) :
    PrivateImplementationPattern<DependencyDepTag>(new Implementation<DependencyDepTag>(i, d, s))
{
}

DependencyDepTag::~DependencyDepTag()
{
}

std::string
DependencyDepTag::full_text() const
{
    if (_imp->str.empty())
    {
        _imp->str.append(stringify(*_imp->id));
        _imp->str.append(",");
        _imp->str.append(stringify(*_imp->spec));
        _imp->str.append(",");
        DepSpecPrettyPrinter pretty(0, false);
        _imp->cond->accept(pretty);
        _imp->str.append(stringify(pretty));
    }
    return _imp->str;
}

std::string
DependencyDepTag::short_text() const
{
    return stringify(*_imp->id);
}

std::string
DependencyDepTag::category() const
{
    return "dependency";
}

const tr1::shared_ptr<const PackageID>
DependencyDepTag::package_id() const
{
    return _imp->id;
}

const tr1::shared_ptr<const PackageDepSpec>
DependencyDepTag::dependency() const
{
    return _imp->spec;
}

const tr1::shared_ptr<const DependencySpecTree::ConstItem>
DependencyDepTag::conditions() const
{
    return _imp->cond;
}

TargetDepTag::TargetDepTag()
{
}

TargetDepTag::~TargetDepTag()
{
}

std::string
TargetDepTag::short_text() const
{
    return "target";
}

std::string
TargetDepTag::category() const
{
    return "target";
}

DepTagCategoryMaker::DepTagCategoryMaker()
{
    register_maker("glsa", &make_glsa_dep_tag);
    register_maker("general", &make_general_set_dep_tag);
    register_maker("dependency", &make_dependency_set_dep_tag);
    register_maker("target", &make_target_dep_tag);
}

