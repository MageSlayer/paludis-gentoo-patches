/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2010, 2011 Ciaran McCreesh
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

#include <paludis/dep_tag.hh>
#include <paludis/dep_spec.hh>
#include <paludis/dep_label.hh>
#include <paludis/spec_tree.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/singleton-impl.hh>
#include <paludis/util/set-impl.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/accept_visitor.hh>
#include <algorithm>
#include <sstream>

using namespace paludis;

namespace
{
    std::shared_ptr<DepTagCategory>
    make_glsa_dep_tag()
    {
        return std::make_shared<DepTagCategory>(
                true,
                "glsa",
                "Security advisories",
                "Your system is potentially affected by these security issues:",
                "Please read the advisories carefully and take appropriate action.");
    }

    std::shared_ptr<DepTagCategory>
    make_general_set_dep_tag()
    {
        return std::make_shared<DepTagCategory>(
                true,
                "general",
                "General sets",
                "",
                "");
    }

    std::shared_ptr<DepTagCategory>
    make_dependency_set_dep_tag()
    {
        return std::make_shared<DepTagCategory>(
                false,
                "dependency",
                "Dependencies",
                "",
                "");
    }

    std::shared_ptr<DepTagCategory>
    make_target_dep_tag()
    {
        return std::make_shared<DepTagCategory>(
                false,
                "target",
                "Targets",
                "",
                "");
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

DepTag::DepTag()
{
}

DepTag::~DepTag()
{
}

namespace
{
    struct DepTagComparator
    {
        std::string value;

        void visit(const GLSADepTag & t)
        {
            value = t.short_text();
        }

        void visit(const GeneralSetDepTag & t)
        {
            value = t.short_text();
        }

        void visit(const DependencyDepTag & t)
        {
            value = stringify(*t.package_id()) + "," + stringify(*t.dependency()) + ",";
        }

        void visit(const TargetDepTag & t)
        {
            value = t.short_text();
        }
    };
}

bool
DepTag::operator== (const DepTag & other) const
{
    DepTagComparator c1, c2;
    accept(c1);
    other.accept(c2);
    return c1.value == c2.value;
}

bool
DepTag::operator< (const DepTag & other) const
{
    DepTagComparator c1, c2;
    accept(c1);
    other.accept(c2);
    return c1.value < c2.value;
}

GLSADepTag::GLSADepTag(const std::string & id, const std::string & our_glsa_title, const FSPath & our_glsa_file) :
    _id(id),
    _glsa_title(our_glsa_title),
    _glsa_file(our_glsa_file)
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

const FSPath
GLSADepTag::glsa_file() const
{
    return _glsa_file;
}

std::string
GLSADepTag::glsa_title() const
{
    return _glsa_title;
}

namespace paludis
{
    template <>
    struct Imp<GeneralSetDepTag>
    {
        const SetName id;
        const std::string source;

        Imp(const SetName & n, const std::string s) :
            id(n),
            source(s)
        {
        }
    };
}

GeneralSetDepTag::GeneralSetDepTag(const SetName & id, const std::string & r) :
    _imp(id, r)
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
    struct Imp<DependencyDepTag>
    {
        mutable Mutex mutex;
        mutable std::string str;

        std::shared_ptr<const PackageID> id;
        const std::shared_ptr<PackageDepSpec> spec;

        Imp(const std::shared_ptr<const PackageID> & i, const PackageDepSpec & d) :
            id(i),
            spec(std::static_pointer_cast<PackageDepSpec>(d.clone()))
        {
            spec->set_tag(std::shared_ptr<const DepTag>());
        }
    };
}

DependencyDepTag::DependencyDepTag(const std::shared_ptr<const PackageID> & i, const PackageDepSpec & d) :
    _imp(i, d)
{
}

DependencyDepTag::~DependencyDepTag()
{
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

const std::shared_ptr<const PackageID>
DependencyDepTag::package_id() const
{
    return _imp->id;
}

const std::shared_ptr<const PackageDepSpec>
DependencyDepTag::dependency() const
{
    return _imp->spec;
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

DepTagCategoryFactory::DepTagCategoryFactory()
{
}

const std::shared_ptr<DepTagCategory>
DepTagCategoryFactory::create(const std::string & s) const
{
    if (s == "glsa")
        return make_glsa_dep_tag();
    if (s == "general")
        return make_general_set_dep_tag();
    if (s == "dependency")
        return make_dependency_set_dep_tag();
    if (s == "target")
        return make_target_dep_tag();
    throw ConfigurationError("No dep tag category named '" + s + "'");
}

bool
DepTagEntryComparator::operator() (const DepTagEntry & l, const DepTagEntry & r) const
{
    return *l.tag() < *r.tag();
}

template class Singleton<DepTagCategoryFactory>;

template class Set<DepTagEntry, DepTagEntryComparator>;
template class WrappedForwardIterator<Set<DepTagEntry, DepTagEntryComparator>::ConstIteratorTag, const DepTagEntry>;
template class WrappedOutputIterator<Set<DepTagEntry, DepTagEntryComparator>::InserterTag, DepTagEntry>;

template class Pimp<GeneralSetDepTag>;
template class Pimp<DependencyDepTag>;

