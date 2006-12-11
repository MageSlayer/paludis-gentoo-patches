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

#include "cache.hh"
#include <paludis/repositories/gems/gems_repository_exceptions.hh>
#include <paludis/repositories/gems/yaml.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/save.hh>
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
    std::string
    as_string(YamlNode::ConstPointer n)
    {
        struct Visitor :
            YamlNodeVisitorTypes::ConstVisitor
        {
            std::string str;

            void visit(const YamlSequenceNode *) PALUDIS_ATTRIBUTE((noreturn))
            {
                throw GemsCacheError("Expected a scalar node, not a sequence");
            }

            void visit(const YamlMappingNode *) PALUDIS_ATTRIBUTE((noreturn))
            {
                throw GemsCacheError("Expected a scalar node, not a mapping");
            }

            void visit(const YamlScalarNode * nn)
            {
                str = nn->value();
            }
        };

        Visitor v;
        n->accept(&v);
        return v.str;
    }

    struct EntryVisitor :
        YamlNodeVisitorTypes::ConstVisitor
    {
        const std::string id;

        std::string name;
        std::string version;
        std::string summary;
        std::string description;
        std::string homepage;

        EntryVisitor(const std::string & _id) :
            id(_id)
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
                .required_ruby_version(SequentialCollection<std::string>::Pointer(0))
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
                if (i->first->value() == "name")
                    name = as_string(i->second);
            }
        }
    };

    struct TopLevelVisitor :
        YamlNodeVisitorTypes::ConstVisitor
    {
        Implementation<GemsCache>::Pointer imp;
        bool top_level;

        TopLevelVisitor(Implementation<GemsCache>::Pointer p) :
            imp(p),
            top_level(true)
        {
        }

        void visit(const YamlSequenceNode * n)
        {
            std::for_each(n->begin(), n->end(), accept_visitor(this));
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
                    EntryVisitor v(i->first->value());
                    i->second->accept(&v);
                    imp->entries.push_back(v.entry());
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

