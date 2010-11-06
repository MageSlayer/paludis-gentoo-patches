/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010 Ciaran McCreesh
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

#include <paludis/repositories/unwritten/unwritten_repository_file.hh>
#include <paludis/repositories/unwritten/unwritten_repository.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/log.hh>
#include <paludis/util/simple_parser.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/join.hh>
#include <paludis/util/safe_ifstream.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/accept_visitor.hh>
#include <paludis/name.hh>
#include <paludis/version_spec.hh>
#include <paludis/literal_metadata_key.hh>
#include <paludis/metadata_key-fwd.hh>
#include <paludis/dep_spec.hh>
#include <paludis/formatter.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/spec_tree.hh>

#include <paludis/util/pimp-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/indirect_iterator-impl.hh>

#include <list>
#include <algorithm>

using namespace paludis;
using namespace paludis::unwritten_repository;

typedef std::list<UnwrittenRepositoryFileEntry> Entries;

namespace paludis
{
    template <>
    struct Imp<UnwrittenRepositoryFile>
    {
        Entries entries;
    };

    template <>
    struct WrappedForwardIteratorTraits<UnwrittenRepositoryFile::ConstIteratorTag>
    {
        typedef Entries::const_iterator UnderlyingIterator;
    };
}

UnwrittenRepositoryFile::UnwrittenRepositoryFile(const FSPath & f) :
    Pimp<UnwrittenRepositoryFile>()
{
    _load(f);
}

UnwrittenRepositoryFile::~UnwrittenRepositoryFile()
{
}

UnwrittenRepositoryFile::ConstIterator
UnwrittenRepositoryFile::begin() const
{
    return ConstIterator(_imp->entries.begin());
}

UnwrittenRepositoryFile::ConstIterator
UnwrittenRepositoryFile::end() const
{
    return ConstIterator(_imp->entries.end());
}

namespace
{
    struct UnwrittenHomepagePrinter
    {
        std::stringstream s;
        const SimpleURISpecTree::ItemFormatter & formatter;

        UnwrittenHomepagePrinter(const SimpleURISpecTree::ItemFormatter & f) :
            formatter(f)
        {
        }

        void visit(const SimpleURISpecTree::NodeType<AllDepSpec>::Type & node)
        {
            std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
        }

        void visit(const SimpleURISpecTree::NodeType<ConditionalDepSpec>::Type & node)
        {
            std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
        }

        void visit(const SimpleURISpecTree::NodeType<SimpleURIDepSpec>::Type & node)
        {
            if (! s.str().empty())
                s << " ";
            s << formatter.format(*node.spec(), format::Plain());
        }
    };

    struct UnwrittenHomepageKey :
        MetadataSpecTreeKey<SimpleURISpecTree>
    {
        const std::shared_ptr<const SimpleURISpecTree> vv;

        const std::string _raw_name;
        const std::string _human_name;
        const MetadataKeyType _type;

        UnwrittenHomepageKey(const std::string & r, const std::string & h, const MetadataKeyType t,
                const std::shared_ptr<const SimpleURISpecTree> & v) :
            vv(v),
            _raw_name(r),
            _human_name(h),
            _type(t)
        {
        }

        const std::shared_ptr<const SimpleURISpecTree> value() const
        {
            return vv;
        }

        std::string pretty_print(const SimpleURISpecTree::ItemFormatter & f) const
        {
            UnwrittenHomepagePrinter p(f);
            value()->top()->accept(p);
            return p.s.str();
        }

        std::string pretty_print_flat(const SimpleURISpecTree::ItemFormatter & f) const
        {
            UnwrittenHomepagePrinter p(f);
            value()->top()->accept(p);
            return p.s.str();
        }

        virtual const std::string raw_name() const PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            return _raw_name;
        }

        virtual const std::string human_name() const PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            return _human_name;
        }

        virtual MetadataKeyType type() const PALUDIS_ATTRIBUTE((warn_unused_result))
        {
            return _type;
        }
    };
}

void
UnwrittenRepositoryFile::_load(const FSPath & f)
{
    SafeIFStream file(f);

    std::string line;
    while (std::getline(file, line))
    {
        if (line.empty())
            break;

        std::string key, value;
        SimpleParser line_parser(line);
        if (line_parser.consume(
                    (+simple_parser::any_except(" \t") >> key) &
                    (*simple_parser::any_of(" \t")) &
                    (simple_parser::exact("=")) &
                    (*simple_parser::any_of(" \t")) &
                    (*simple_parser::any_except("") >> value)
                    ))
        {
            if (key == "format")
            {
                if (value != "unwritten-1")
                    throw UnwrittenRepositoryConfigurationError(
                            "Unsupported format '" + value + "' in '" + stringify(f) + "'");
            }
            else
                Log::get_instance()->message("unwritten_repository.file.unknown_key", ll_warning, lc_context)
                    << "Ignoring unknown key '" << key << "' with value '" << value << "'";
        }
        else
            throw UnwrittenRepositoryConfigurationError(
                    "Cannot parse header line '" + line + "' in '" + stringify(f) + "'");
    }

    CategoryNamePart category("x");
    PackageNamePart package("x");
    std::shared_ptr<MetadataValueKey<SlotName> > slot;
    VersionSpec version("0", { });
    std::shared_ptr<UnwrittenRepositoryFileEntry> entry;
    while (std::getline(file, line))
    {
        SimpleParser line_parser(line);

        std::string token, token2;
        if (line.empty())
        {
        }
        else if (line_parser.consume(
                    (*simple_parser::any_of(" \t")) &
                    (simple_parser::exact("#")) &
                    (*simple_parser::any_except(""))))
        {
        }
        else if (line_parser.consume(
                    (+simple_parser::any_except(" \t/") >> token) &
                    (simple_parser::exact("/"))
                    ))
        {
            if (! line_parser.eof())
                throw UnwrittenRepositoryConfigurationError(
                        "Cannot parse body category line '" + line + "' in '" + stringify(f) + "'");

            category = CategoryNamePart(token);
        }
        else if (line_parser.consume(
                    (+simple_parser::any_of(" \t")) &
                    (+simple_parser::any_except(" \t/") >> token) &
                    (simple_parser::exact("/"))
                    ))
        {
            if (! line_parser.eof())
                throw UnwrittenRepositoryConfigurationError(
                        "Cannot parse body package line '" + line + " in '" + stringify(f) + "'");

            package = PackageNamePart(token);
        }
        else if (line_parser.consume(
                    (+simple_parser::any_of(" \t")) &
                    (+simple_parser::exact(":")) &
                    (+simple_parser::any_except(" \t") >> token) &
                    (+simple_parser::any_of(" \t"))
                    ))
        {
            slot = std::make_shared<LiteralMetadataValueKey<SlotName>>("SLOT", "Slot", mkt_internal, SlotName(token));

            if (line_parser.consume(
                        (+simple_parser::any_except(" \t") >> token)
                        ))
                version = VersionSpec(token, user_version_spec_options());
            else
                throw UnwrittenRepositoryConfigurationError(
                        "Cannot parse body slot+version line '" + line + " in '" + stringify(f) + "'");

            if (! line_parser.eof())
                throw UnwrittenRepositoryConfigurationError(
                        "Cannot parse body slot+version line '" + line + " in '" + stringify(f) + "'");

            if (entry)
            {
                _imp->entries.push_back(*entry);
                entry.reset();
            }
        }
        else if (line_parser.consume(
                    (+simple_parser::any_of(" \t")) &
                    (+simple_parser::any_except(" \t") >> token) &
                    (+simple_parser::any_of(" \t")) &
                    (+simple_parser::exact("=")) &
                    (+simple_parser::any_of(" \t")) &
                    (+simple_parser::any_except("") >> token2)
                    ))
        {
            if (! line_parser.eof())
                throw UnwrittenRepositoryConfigurationError(
                        "Cannot parse body key = value line '" + line + " in '" + stringify(f) + "'");

            if (! entry)
                entry = std::make_shared<UnwrittenRepositoryFileEntry>(make_named_values<UnwrittenRepositoryFileEntry>(
                                n::added_by() = std::shared_ptr<const MetadataValueKey<std::string> >(),
                                n::bug_ids() = std::shared_ptr<const MetadataCollectionKey<Sequence<std::string> > >(),
                                n::comment() = std::shared_ptr<const MetadataValueKey<std::string> >(),
                                n::commit_id() = std::shared_ptr<const MetadataValueKey<std::string> >(),
                                n::description() = std::shared_ptr<const MetadataValueKey<std::string> >(),
                                n::homepage() = std::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> >(),
                                n::name() = category + package,
                                n::remote_ids() = std::shared_ptr<const MetadataCollectionKey<Sequence<std::string> > >(),
                                n::removed_by() = std::shared_ptr<const MetadataValueKey<std::string> >(),
                                n::removed_from() = std::shared_ptr<const MetadataCollectionKey<Set<std::string> > >(),
                                n::slot() = slot,
                                n::version() = version
                                ));

            if (token == "description")
                entry->description() = std::make_shared<LiteralMetadataValueKey<std::string>>("description", "Description", mkt_significant, token2);
            else if (token == "homepage")
            {
                std::shared_ptr<AllDepSpec> all_spec(std::make_shared<AllDepSpec>());
                std::shared_ptr<SimpleURISpecTree> tree(std::make_shared<SimpleURISpecTree>(all_spec));
                std::list<std::string> tokens;
                tokenise_whitespace(token2, std::back_inserter(tokens));
                for (std::list<std::string>::const_iterator t(tokens.begin()), t_end(tokens.end()) ;
                        t != t_end ; ++t)
                    tree->top()->append(std::make_shared<SimpleURIDepSpec>(*t));
                entry->homepage() = std::make_shared<UnwrittenHomepageKey>("homepage", "Homepage", mkt_normal, tree);
            }
            else if (token == "comment")
                entry->comment() = std::make_shared<LiteralMetadataValueKey<std::string>>("comment", "Comment", mkt_normal, token2);
            else if (token == "commit-id")
                entry->commit_id() = std::make_shared<LiteralMetadataValueKey<std::string>>("commit-id", "Commit ID", mkt_normal, token2);
            else if (token == "added-by")
                entry->added_by() = std::make_shared<LiteralMetadataValueKey<std::string>>("added-by", "Added by", mkt_author, token2);
            else if (token == "removed-by")
                entry->removed_by() = std::make_shared<LiteralMetadataValueKey<std::string>>("removed-by", "Removed by", mkt_author, token2);
            else if (token == "removed-from")
            {
                auto t2s(std::make_shared<Set<std::string> >());
                t2s->insert(token2);
                entry->removed_from() = std::make_shared<LiteralMetadataStringSetKey>("removed-from", "Removed from", mkt_author, t2s);
            }
            else if (token == "bug-ids")
            {
                std::shared_ptr<Sequence<std::string> > seq(std::make_shared<Sequence<std::string>>());
                std::list<std::string> tokens;
                tokenise_whitespace(token2, std::back_inserter(tokens));
                for (std::list<std::string>::const_iterator t(tokens.begin()), t_end(tokens.end()) ;
                        t != t_end ; ++t)
                    seq->push_back(*t);
                entry->bug_ids() = std::make_shared<LiteralMetadataStringSequenceKey>("bug-ids", "Bug IDs", mkt_normal, seq);
            }
            else if (token == "remote-ids")
            {
                std::shared_ptr<Sequence<std::string> > seq(std::make_shared<Sequence<std::string>>());
                std::list<std::string> tokens;
                tokenise_whitespace(token2, std::back_inserter(tokens));
                for (std::list<std::string>::const_iterator t(tokens.begin()), t_end(tokens.end()) ;
                        t != t_end ; ++t)
                    seq->push_back(*t);
                entry->remote_ids() = std::make_shared<LiteralMetadataStringSequenceKey>("remote-ids", "Remote IDs", mkt_internal, seq);
            }
            else
                Log::get_instance()->message("unwritten_repository.file.unknown_key", ll_warning, lc_context)
                    << "Ignoring unknown key '" << token << "' with value '" << token2 << "'";
        }
        else
            throw UnwrittenRepositoryConfigurationError(
                    "Cannot parse body line '" + line + " in '" + stringify(f) + "'");
    }

    if (entry)
        _imp->entries.push_back(*entry);
}

template class Pimp<UnwrittenRepositoryFile>;
template class WrappedForwardIterator<UnwrittenRepositoryFile::ConstIteratorTag,
         const UnwrittenRepositoryFileEntry>;


