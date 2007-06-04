/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <paludis/repositories/gems/gem_specification.hh>
#include <paludis/repositories/gems/yaml.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/tr1_functional.hh>

using namespace paludis;
using namespace paludis::gems;

namespace paludis
{
    template <>
    struct Implementation<GemSpecification>
    {
        std::string name;
        std::string version;
        std::string summary;
        std::string authors;
        std::string date;
        std::string description;
        std::string platform;
        std::string rubyforge_project;
        std::string homepage;
    };
}

namespace
{
    std::string extract_text_only(const yaml::Node & n, const std::string & extra);

    struct VersionVisitor :
        ConstVisitor<yaml::NodeVisitorTypes>
    {
        std::string text;

        void visit(const yaml::StringNode & n) PALUDIS_ATTRIBUTE((noreturn));
        void visit(const yaml::SequenceNode & n) PALUDIS_ATTRIBUTE((noreturn));

        void visit(const yaml::MapNode & n)
        {
            yaml::MapNode::Iterator i(n.find("version"));
            if (i == n.end())
                throw BadSpecificationError("Version has no version: key");
            text = extract_text_only(*i->second, "for Version version: key");
        }
    };

    void VersionVisitor::visit(const yaml::StringNode &)
    {
        throw BadSpecificationError("Version child node is string, not map");
    }

    void VersionVisitor::visit(const yaml::SequenceNode &)
    {
        throw BadSpecificationError("Version child node is sequence, not map");
    }

    struct ExtractTextVisitor :
        ConstVisitor<yaml::NodeVisitorTypes>
    {
        const std::string extra;
        const bool accept_sequence;
        std::string result;

        ExtractTextVisitor(const std::string & s, const bool b) :
            extra(s),
            accept_sequence(b)
        {
        }

        void visit(const yaml::StringNode & n)
        {
            result = n.text();
        }

        void visit(const yaml::SequenceNode & s)
        {
            if (! accept_sequence)
                throw BadSpecificationError("Found sequence rather than text " + extra);

            bool w(false);
            for (yaml::SequenceNode::Iterator i(s.begin()), i_end(s.end()) ; i != i_end ; ++i)
            {
                if (w)
                    result.append(", ");
                result.append(extract_text_only(**i, extra));
                w = true;
            }
        }

        void visit(const yaml::MapNode &) PALUDIS_ATTRIBUTE((noreturn));
    };

    void ExtractTextVisitor::visit(const yaml::MapNode &)
    {
        throw BadSpecificationError("Found map rather than text " + extra);
    }

    std::string extract_text_only(const yaml::Node & n, const std::string & extra)
    {
        ExtractTextVisitor v(extra, false);
        n.accept(v);
        return v.result;
    }

    std::string extract_text_sequence(const yaml::Node & n, const std::string & extra)
    {
        ExtractTextVisitor v(extra, true);
        n.accept(v);
        return v.result;
    }

    std::string required_text_only_key(const yaml::MapNode & n, const std::string & k)
    {
        yaml::MapNode::Iterator i(n.find(k));
        if (i == n.end())
            throw BadSpecificationError("Key '" + k + "' not defined");
        return extract_text_only(*i->second, "for key '" + k + "'");
    }

    std::string optional_text_sequence_key(const yaml::MapNode & n, const std::string & k)
    {
        yaml::MapNode::Iterator i(n.find(k));
        if (i == n.end())
            return "";
        return extract_text_sequence(*i->second, "for key '" + k + "'");
    }

    std::string optional_text_only_key(const yaml::MapNode & n, const std::string & k)
    {
        yaml::MapNode::Iterator i(n.find(k));
        if (i == n.end())
            return "";
        return extract_text_only(*i->second, "for key '" + k + "'");
    }

    std::string required_version(const yaml::MapNode & n, const std::string & k)
    {
        yaml::MapNode::Iterator i(n.find(k));
        if (i == n.end())
            throw BadSpecificationError("Key '" + k + "' not defined");

        VersionVisitor v;
        i->second->accept(v);
        return v.text;
    }

    struct TopVisitor :
        ConstVisitor<yaml::NodeVisitorTypes>
    {
        Implementation<GemSpecification> * const _imp;

        TopVisitor(Implementation<GemSpecification> * const i) :
            _imp(i)
        {
        }

        void visit(const yaml::MapNode & n)
        {
            _imp->summary = required_text_only_key(n, "summary");
            _imp->authors = optional_text_sequence_key(n, "authors");
            _imp->date = required_text_only_key(n, "date");
            _imp->description = optional_text_only_key(n, "description");
            _imp->platform = required_text_only_key(n, "platform");
            _imp->name = required_text_only_key(n, "name");
            _imp->version = required_version(n, "version");
            _imp->rubyforge_project = optional_text_sequence_key(n, "rubyforge_project");
        }

        void visit(const yaml::SequenceNode & n) PALUDIS_ATTRIBUTE((noreturn));

        void visit(const yaml::StringNode & n) PALUDIS_ATTRIBUTE((noreturn));
    };

    void TopVisitor::visit(const yaml::SequenceNode &)
    {
        throw BadSpecificationError("Top level node is sequence, not map");
    }

    void TopVisitor::visit(const yaml::StringNode & n)
    {
        throw BadSpecificationError("Top level node is text '" + n.text() + "', not map");
    }
}

GemSpecification::GemSpecification(const yaml::Node & node) :
    PrivateImplementationPattern<GemSpecification>(new Implementation<GemSpecification>),
    name(tr1::bind(&Implementation<GemSpecification>::name, _imp.get())),
    version(tr1::bind(&Implementation<GemSpecification>::version, _imp.get())),
    homepage(tr1::bind(&Implementation<GemSpecification>::homepage, _imp.get())),
    rubyforge_project(tr1::bind(&Implementation<GemSpecification>::rubyforge_project, _imp.get())),
    authors(tr1::bind(&Implementation<GemSpecification>::authors, _imp.get())),
    date(tr1::bind(&Implementation<GemSpecification>::date, _imp.get())),
    platform(tr1::bind(&Implementation<GemSpecification>::platform, _imp.get())),
    summary(tr1::bind(&Implementation<GemSpecification>::summary, _imp.get())),
    description(tr1::bind(&Implementation<GemSpecification>::description, _imp.get()))
{
    TopVisitor v(_imp.get());
    node.accept(v);
}

GemSpecification::~GemSpecification()
{
}

BadSpecificationError::BadSpecificationError(const std::string & s) throw () :
    Exception("Bad gem specification: " + s)
{
}

