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

#include <paludis/repositories/gems/gem_specifications.hh>
#include <paludis/repositories/gems/gem_specification.hh>
#include <paludis/repositories/gems/yaml.hh>
#include <paludis/name.hh>
#include <paludis/version_spec.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/log.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/hashes.hh>
#include <unordered_map>

using namespace paludis;
using namespace paludis::gems;

typedef std::unordered_map<std::pair<QualifiedPackageName, VersionSpec>, std::shared_ptr<const GemSpecification>,
        Hash<std::pair<QualifiedPackageName, VersionSpec> > > Specs;

namespace paludis
{
    template <>
    struct Imp<GemSpecifications>
    {
        Specs specs;
    };

    template <>
    struct WrappedForwardIteratorTraits<GemSpecifications::ConstIteratorTag>
    {
        typedef Specs::const_iterator UnderlyingIterator;
    };
}

namespace
{
    std::string extract_text_only(const yaml::Node & n, const std::string & extra);

    struct ExtractTextVisitor
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
            for (yaml::SequenceNode::ConstIterator i(s.begin()), i_end(s.end()) ; i != i_end ; ++i)
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

    struct GemsVisitor
    {
        Imp<GemSpecifications> * const _imp;
        const Environment * const environment;
        const std::shared_ptr<const Repository> repository;

        GemsVisitor(const Environment * const e,
                const std::shared_ptr<const Repository> & r, Imp<GemSpecifications> * const i) :
            _imp(i),
            environment(e),
            repository(r)
        {
        }

        void visit(const yaml::MapNode & n)
        {
            for (yaml::MapNode::ConstIterator i(n.begin()), i_end(n.end()) ; i != i_end ; ++i)
            {
                std::string pv(extract_text_only(*i->first, " as key for gem"));
                Context c_item("When handling Gem entry '" + pv + "':");

                try
                {
                    std::shared_ptr<GemSpecification> spec(new GemSpecification(environment, repository, *i->second));
                    _imp->specs.insert(std::make_pair(std::make_pair(spec->name(), spec->version()), spec));
                }
                catch (const InternalError &)
                {
                    throw;
                }
                catch (const Exception & e)
                {
                    Log::get_instance()->message("gems.id.failure", ll_qa, lc_context) << "Skipping entry '"
                        << pv << "' due to exception '" << e.message() << "' (" << e.what() << ")";
                }
            }
        }

        void visit(const yaml::SequenceNode & n) PALUDIS_ATTRIBUTE((noreturn));

        void visit(const yaml::StringNode & n) PALUDIS_ATTRIBUTE((noreturn));
    };

    void GemsVisitor::visit(const yaml::SequenceNode &)
    {
        throw BadSpecificationError("Top level 'gems' right hand node is sequence, not map");
    }

    void GemsVisitor::visit(const yaml::StringNode & n)
    {
        throw BadSpecificationError("Top level 'gems' right hand node is text '" + n.text() + "', not map");
    }

    struct TopVisitor
    {
        Imp<GemSpecifications> * const _imp;
        const Environment * const environment;
        const std::shared_ptr<const Repository> repository;

        TopVisitor(const Environment * const e,
                const std::shared_ptr<const Repository> & r, Imp<GemSpecifications> * const i) :
            _imp(i),
            environment(e),
            repository(r)
        {
        }

        void visit(const yaml::MapNode & n)
        {
            yaml::MapNode::ConstIterator i(n.find("gems"));
            if (n.end() == i)
                throw BadSpecificationError("Top level map does not contain 'gems' node");

            GemsVisitor g(environment, repository, _imp);
            i->second->accept(g);
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

GemSpecifications::GemSpecifications(const Environment * const e,
        const std::shared_ptr<const Repository> & r, const yaml::Node & n) :
    Pimp<GemSpecifications>()
{
    TopVisitor v(e, r, _imp.get());
    n.accept(v);
}

GemSpecifications::~GemSpecifications()
{
}

GemSpecifications::ConstIterator
GemSpecifications::begin() const
{
    return ConstIterator(_imp->specs.begin());
}

GemSpecifications::ConstIterator
GemSpecifications::end() const
{
    return ConstIterator(_imp->specs.end());
}

template class WrappedForwardIterator<GemSpecifications::ConstIteratorTag,
         const std::pair<const std::pair<QualifiedPackageName, VersionSpec>, std::shared_ptr<const GemSpecification> > >;


