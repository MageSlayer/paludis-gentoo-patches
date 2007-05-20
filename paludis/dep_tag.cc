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
#include <paludis/util/virtual_constructor-impl.hh>

/** \file
 * Implementation for DepTag, DepTagCategory etc.
 *
 * \ingroup grpdeptag
 */

using namespace paludis;

template class VirtualConstructor<std::string, tr1::shared_ptr<const DepTagCategory> (*) (),
         virtual_constructor_not_found::ThrowException<NoSuchDepTagCategory> >;

#include <paludis/dep_tag-sr.cc>

namespace
{
    /**
     * Create the DepTagCategory for GLSAs.
     *
     * \see register_glsa_dep_tag
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
     * \see register_general_set_dep_tag
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
     * \see register_dependency_set_dep_tag
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

bool
DepTag::operator== (const DepTag & other) const
{
    return short_text() == other.short_text();
}

bool
DepTag::operator< (const DepTag & other) const
{
    return short_text() < other.short_text();
}

GLSADepTag::GLSADepTag(const std::string & id, const std::string & our_glsa_title) :
    _id(id),
    _glsa_title(our_glsa_title)
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

GeneralSetDepTag::GeneralSetDepTag(const SetName & id, const std::string & r) :
    _id(id),
    _source(r)
{
}

std::string
GeneralSetDepTag::short_text() const
{
    return stringify(_id);
}

std::string
GeneralSetDepTag::category() const
{
    return "general";
}

std::string
GeneralSetDepTag::source() const
{
    return _source;
}

DependencyDepTag::DependencyDepTag(const PackageDatabaseEntry & pde) :
    _dbe(pde)
{
}

std::string
DependencyDepTag::short_text() const
{
    return stringify(_dbe);
}

std::string
DependencyDepTag::category() const
{
    return "dependency";
}

DepTagCategoryMaker::DepTagCategoryMaker()
{
    register_maker("glsa", &make_glsa_dep_tag);
    register_maker("general", &make_general_set_dep_tag);
    register_maker("dependency", &make_dependency_set_dep_tag);
}

