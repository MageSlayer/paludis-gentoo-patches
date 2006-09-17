/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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

/** \file
 * Implementation for DepTag, DepTagCategory etc.
 *
 * \ingroup grpdeptag
 */

using namespace paludis;

namespace
{
    /**
     * Create the DepTagCategory for GLSAs.
     *
     * \see register_glsa_dep_tag
     *
     * \ingroup grpdeptag
     */
    DepTagCategory::ConstPointer
    make_glsa_dep_tag()
    {
        return DepTagCategory::ConstPointer(new DepTagCategory(
                    "glsa",
                    "Security advisories",
                    "Your system is potentially affected by these security issues:",
                    "Please read the advisories carefully and take appropriate action."));
    }

    /**
     * Register the GLSA dep tag category instance.
     *
     * \ingroup grpdeptag
     */
    static const DepTagCategoryMaker::RegisterMaker register_glsa_dep_tag("glsa",
            &make_glsa_dep_tag);

    /**
     * Create the DepTagCategory for seneral sets.
     *
     * \see register_general_set_dep_tag
     *
     * \ingroup grpdeptag
     */
    DepTagCategory::ConstPointer
    make_general_set_dep_tag()
    {
        return DepTagCategory::ConstPointer(new DepTagCategory(
                    "general",
                    "General sets",
                    "",
                    ""));
    }

    /**
     * Register the general set dep tag category instance.
     *
     * \ingroup grpdeptag
     */
    static const DepTagCategoryMaker::RegisterMaker register_general_set_dep_tag("general",
            &make_general_set_dep_tag);
}

DepTagCategory::DepTagCategory(const std::string & our_id,
        const std::string & t, const std::string & pre,
        const std::string & post) :
    _id(our_id),
    _title(t),
    _pre_text(pre),
    _post_text(post)
{
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

GeneralSetDepTag::GeneralSetDepTag(const std::string & id) :
    _id(id)
{
}

std::string
GeneralSetDepTag::short_text() const
{
    return _id;
}

std::string
GeneralSetDepTag::category() const
{
    return "general";
}

