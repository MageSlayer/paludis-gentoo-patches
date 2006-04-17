/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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

using namespace paludis;

namespace
{
    DepTagCategory::ConstPointer
    make_glsa_dep_tag()
    {
        return DepTagCategory::ConstPointer(new DepTagCategory(
                    "glsa",
                    "Security advisories",
                    "Your system is potentially affected by these security issues:",
                    "Please read the advisories carefully and take appropriate action."));
    }
}
static const DepTagCategoryMaker::RegisterMaker register_glsa_dep_tag("glsa",
        &make_glsa_dep_tag);

DepTagCategory::DepTagCategory(const std::string & id,
        const std::string & t, const std::string & pre,
        const std::string & post) :
    _id(id),
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

GLSADepTag::GLSADepTag(const std::string & id, const std::string & glsa_title) :
    _id(id),
    _glsa_title(glsa_title)
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

