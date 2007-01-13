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

#include "cache.hh"
#include <paludis/repositories/gems/gems_repository_exceptions.hh>
#include <paludis/repositories/gems/yaml.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/save.hh>
#include <paludis/util/collection_concrete.hh>
#include <paludis/util/log.hh>
#include <list>

#include <yaml.h>

using namespace paludis;

#include <paludis/repositories/gems/cache-sr.cc>

namespace paludis
{
    template<>
    struct Implementation<GemsCache> :
        InternalCounted<Implementation<GemsCache> >
    {
        std::list<GemsCacheEntry> entries;
    };
}

namespace
{
    struct AsStringVisitor :
        YamlNodeVisitorTypes::ConstVisitor
    {
        std::string str;
        std::string join;

        void visit(const YamlSequenceNode * nn)
        {
            if (join.empty())
                throw GemsCacheError("Expected a scalar node, not a sequence");
            else
                std::for_each(nn->begin(), nn->end(), accept_visitor(this));
        }

        void visit(const YamlMappingNode *) PALUDIS_ATTRIBUTE((noreturn))
        {
            throw GemsCacheError("Expected a scalar node, not a mapping");
        }

        void visit(const YamlScalarNode * nn)
        {
            if (! str.empty())
                str.append(join);
            str.append(nn->value());
        }
    };

    std::string
    as_string(YamlNode::ConstPointer n, const std::string & join = "")
    {
        AsStringVisitor v;
        v.join = join;
        n->accept(&v);
        return v.str;
    }

    struct VersionVisitor :
        YamlNodeVisitorTypes::ConstVisitor
    {
        std::string str;

        void visit(const YamlScalarNode * n) PALUDIS_ATTRIBUTE((noreturn))
        {
            throw GemsCacheError("Expected a mapping node, not scalar '" + n->value() + "'");
        }

        void visit(const YamlSequenceNode *) PALUDIS_ATTRIBUTE((noreturn))
        {
            throw GemsCacheError("Expected a mapping node, not a sequence");
        }

        void visit(const YamlMappingNode * n)
        {
            Context context("When looking for a version: key in a mapping:");
            for (YamlMappingNode::Iterator i(n->begin()), i_end(n->end()) ; i != i_end ; ++i)
                if (i->first->value() == "version")
                    str = as_string(i->second);
        }
    };

    struct RequirementsVisitor :
        YamlNodeVisitorTypes::ConstVisitor
    {
        VersionRequirements::Pointer r;
        bool top_level;

        std::string op;
        std::string v;

        RequirementsVisitor(VersionRequirements::Pointer rr) :
            r(rr),
            top_level(true)
        {
        }

        void visit(const YamlMappingNode * n)
        {
            Context context("When handling mapping node:");

            if (top_level)
            {
                for (YamlMappingNode::Iterator i(n->begin()), i_end(n->end()) ; i != i_end ; ++i)
                    if (i->first->value() == "requirements")
                    {
                        Context context2("When handling mapping node requirements key:");
                        i->second->accept(this);
                    }
            }
            else
                for (YamlMappingNode::Iterator i(n->begin()), i_end(n->end()) ; i != i_end ; ++i)
                    if (i->first->value() == "version")
                    {
                        Context local_context("When looking for a version: key in a requirements mapping:");
                        v = as_string(i->second);
                    }
        }

        void visit(const YamlSequenceNode * n)
        {
            Context context("When handling sequence node:");

            if (top_level)
            {
                Save<bool> save_top_level(&top_level, false);
                std::for_each(n->begin(), n->end(), accept_visitor(this));
            }
            else
            {
                op = "";
                v = "";

                YamlSequenceNode::Iterator i(n->begin()), i_end(n->end());
                if (i == i_end)
                    throw YamlError("Expected a sequence with two entries, not zero");
                op = as_string(*i++);
                if (i == i_end)
                    throw YamlError("Expected a sequence with two entries, not one");
                VersionVisitor vv;
                (*i++)->accept(&vv);
                v = vv.str;
                if (i != i_end)
                    throw YamlError("Expected a sequence with two entries, not more than two");

                r->push_back(VersionRequirement(VersionOperator(op), VersionSpec(v)));
            }
        }

        void visit(const YamlScalarNode * n)
        {
            if (top_level && n->value().empty())
                return;

            throw YamlError("Didn't expect a scalar");
        }
    };

    struct EntryVisitor :
        YamlNodeVisitorTypes::ConstVisitor
    {
        const std::string id;

        std::string name;
        std::string version;
        std::string summary;
        std::string description;
        std::string homepage;
        VersionRequirements::Pointer required_ruby_version;

        EntryVisitor(const std::string & _id) :
            id(_id),
            required_ruby_version(new VersionRequirements::Concrete)
        {
        }

        GemsCacheEntry
        entry() const
        {
            Context context("When creating GemsCacheEntry with id '" + id + "':");

            return GemsCacheEntry::create()
                .name(PackageNamePart(name))
                .version(VersionSpec(version))
                .summary(summary)
                .description(description)
                .homepage(homepage)
                .required_ruby_version(required_ruby_version)
                .authors(SequentialCollection<std::string>::Pointer(0))
                .dependencies(SequentialCollection<std::string>::Pointer(0))
                .requirements(SequentialCollection<std::string>::Pointer(0));
        }

        void visit(const YamlSequenceNode *) PALUDIS_ATTRIBUTE((noreturn))
        {
            throw GemsCacheError("Sequence node not expected here'");
        }

        void visit(const YamlScalarNode * n) PALUDIS_ATTRIBUTE((noreturn))
        {
            throw GemsCacheError("Node '" + n->value() + "' not expected here'");
        }

        void visit(const YamlMappingNode * n)
        {
            for (YamlMappingNode::Iterator i(n->begin()), i_end(n->end()) ; i != i_end ; ++i)
            {
                Context context("When handling entry key '" + i->first->value() + "':");

                if (i->first->value() == "name")
                    name = as_string(i->second);
                else if (i->first->value() == "summary")
                    summary = as_string(i->second);
                else if (i->first->value() == "description")
                    description = as_string(i->second);
                else if (i->first->value() == "homepage")
                    homepage = as_string(i->second, " ");
                else if (i->first->value() == "version")
                {
                    VersionVisitor v;
                    i->second->accept(&v);
                    version = v.str;
                }
                else if (i->first->value() == "required_ruby_version")
                {
                    Context context2("When handling required_ruby_version children:");
                    RequirementsVisitor v(required_ruby_version);
                    i->second->accept(&v);
                }
            }
        }
    };

    struct TopLevelVisitor :
        YamlNodeVisitorTypes::ConstVisitor,
        YamlNodeVisitorTypes::ConstVisitor::VisitChildren<TopLevelVisitor, YamlSequenceNode>
    {
        using YamlNodeVisitorTypes::ConstVisitor::VisitChildren<TopLevelVisitor, YamlSequenceNode>::visit;

        Implementation<GemsCache>::Pointer imp;
        bool top_level;

        TopLevelVisitor(Implementation<GemsCache>::Pointer p) :
            imp(p),
            top_level(true)
        {
        }

        void visit(const YamlMappingNode * n)
        {
            for (YamlMappingNode::Iterator i(n->begin()), i_end(n->end()) ; i != i_end ; ++i)
            {
                if (top_level)
                {
                    if (i->first->value() != "gems")
                        throw GemsCacheError("Node '" + i->first->value() + "' is not 'gems'");
                    Save<bool> save_top_level(&top_level, false);
                    i->second->accept(this);
                }
                else
                {
                    Context context("When processing ID '" + i->first->value() + "':");
                    try
                    {
                        EntryVisitor v(i->first->value());
                        i->second->accept(&v);
                        imp->entries.push_back(v.entry());
                    }
                    catch (const NameError & e)
                    {
                        Log::get_instance()->message(ll_qa, lc_context, "Skipping ID '"
                                + i->first->value() + "' due to exception '" + e.message() + "' ("
                                + e.what() + ")");
                    }
                }
            }
        }

        void visit(const YamlScalarNode * n) PALUDIS_ATTRIBUTE((noreturn))
        {
            throw GemsCacheError("Node '" + n->value() + "' not expected here'");
        }
    };
}

GemsCache::GemsCache(const FSEntry & loc) :
    PrivateImplementationPattern<GemsCache>(new Implementation<GemsCache>)
{
    Context context("When creating Gems cache from '" + stringify(loc) + "':");

    if (! loc.is_regular_file())
        throw GemsCacheError("Cache '" + stringify(loc) + "' is not a regular file");

    YamlDocument doc(loc);

    TopLevelVisitor v(_imp);
    doc.top()->accept(&v);
}

GemsCache::~GemsCache()
{
}

GemsCache::Iterator
GemsCache::begin() const
{
    return Iterator(_imp->entries.begin());
}

GemsCache::Iterator
GemsCache::end() const
{
    return Iterator(_imp->entries.end());
}

