/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009 Ciaran McCreesh
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

#include "key_extractor.hh"
#include "matcher.hh"
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/set.hh>
#include <paludis/util/join.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/timestamp.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/stringify_formatter.hh>
#include <paludis/dep_spec.hh>
#include <paludis/environment.hh>
#include <paludis/choice.hh>
#include <functional>
#include <algorithm>

using namespace paludis;
using namespace inquisitio;

namespace paludis
{
    template <>
    struct Implementation<KeyExtractor>
    {
        const std::string key;
        const bool flatten;
        const bool visible_only;
        const Environment & env;

        Implementation(const std::string & k, const bool f, const bool v, const Environment & e) :
            key(k),
            flatten(f),
            visible_only(v),
            env(e)
        {
        }
    };
}

KeyExtractor::KeyExtractor(const std::string & k, const bool f, const bool v,
        const Environment & e) :
    PrivateImplementationPattern<KeyExtractor>(new Implementation<KeyExtractor>(k, f, v, e))
{
}

KeyExtractor::~KeyExtractor()
{
}

namespace
{
    class TreeVisitor
    {
        private:
            const std::string _key;
            const bool _visible_only;
            const Environment & _env;
            const PackageID & _id;
            const Matcher & _m;

        public:
            bool result;

            TreeVisitor(const std::string & k, const bool v, const Environment & e,
                    const PackageID & i, const Matcher & m) :
                _key(k),
                _visible_only(v),
                _env(e),
                _id(i),
                _m(m),
                result(false)
            {
            }

            void visit(const GenericSpecTree::NodeType<AllDepSpec>::Type & node)
            {
                std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
            }

            void visit(const GenericSpecTree::NodeType<AnyDepSpec>::Type & node)
            {
                std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
            }

            void visit(const GenericSpecTree::NodeType<ConditionalDepSpec>::Type & node)
            {
                if (! result)
                {
                    result |= _m(stringify(*node.spec()));

                    if (! result)
                    {
                        if (! _visible_only)
                            std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
                        else if (node.spec()->condition_met())
                            std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
                    }
                }
            }

            void visit(const GenericSpecTree::NodeType<SimpleURIDepSpec>::Type & node)
            {
                if (! result)
                    result |= _m(stringify(*node.spec()));
            }

            void visit(const GenericSpecTree::NodeType<URILabelsDepSpec>::Type & node)
            {
                if (! result)
                    result |= _m(stringify(*node.spec()));
            }

            void visit(const GenericSpecTree::NodeType<PlainTextDepSpec>::Type & node)
            {
                if (! result)
                    result |= _m(stringify(*node.spec()));
            }

            void visit(const GenericSpecTree::NodeType<FetchableURIDepSpec>::Type & node)
            {
                if (! result)
                    result |= _m(stringify(*node.spec()));
            }

            void visit(const GenericSpecTree::NodeType<PackageDepSpec>::Type & node)
            {
                if (! result)
                    result |= _m(stringify(*node.spec()));
            }

            void visit(const GenericSpecTree::NodeType<BlockDepSpec>::Type & node)
            {
                if (! result)
                    result |= _m(stringify(*node.spec()));
            }

            void visit(const GenericSpecTree::NodeType<LicenseDepSpec>::Type & node)
            {
                if (! result)
                    result |= _m(stringify(*node.spec()));
            }

            void visit(const GenericSpecTree::NodeType<PlainTextLabelDepSpec>::Type & node)
            {
                if (! result)
                    result |= _m(stringify(*node.spec()));
            }

            void visit(const GenericSpecTree::NodeType<DependenciesLabelsDepSpec>::Type & node)
            {
                if (! result)
                    result |= _m(stringify(*node.spec()));
            }

            void visit(const GenericSpecTree::NodeType<NamedSetDepSpec>::Type & node)
            {
                if (! result)
                    result |= _m(stringify(*node.spec()));
            }
    };

    class KeyVisitor
    {
        private:
            const std::string _key;
            const bool _flatten;
            const bool _visible_only;
            const Environment & _env;
            const PackageID & _id;
            const Matcher & _m;

        public:
            bool result;

            KeyVisitor(const std::string & k, const bool f, const bool v, const Environment & e,
                    const PackageID & i, const Matcher & m) :
                _key(k),
                _flatten(f),
                _visible_only(v),
                _env(e),
                _id(i),
                _m(m),
                result(false)
            {
            }

            void visit(const MetadataValueKey<std::string> & s)
            {
                result = _m(s.value());
            }

            void visit(const MetadataValueKey<SlotName> & s)
            {
                result = _m(stringify(s.value()));
            }

            void visit(const MetadataValueKey<long> & s)
            {
                result = _m(stringify(s.value()));
            }

            void visit(const MetadataValueKey<bool> & s)
            {
                result = _m(stringify(s.value()));
            }

            void visit(const MetadataTimeKey & s)
            {
                result = _m(stringify(s.value().seconds()));
            }

            void visit(const MetadataValueKey<std::shared_ptr<const Contents> > &)
            {
            }

            void visit(const MetadataValueKey<std::shared_ptr<const RepositoryMaskInfo> > &)
            {
            }

            void visit(const MetadataValueKey<FSEntry> & s)
            {
                result = _m(stringify(s.value()));
            }

            void visit(const MetadataValueKey<std::shared_ptr<const PackageID> > & s)
            {
                result = _m(stringify(*s.value()));
            }

            void visit(const MetadataCollectionKey<KeywordNameSet> & s)
            {
                using namespace std::placeholders;

                if (_flatten)
                    result = _m(join(s.value()->begin(), s.value()->end(), " "));
                else
                    result = s.value()->end() != std::find_if(s.value()->begin(), s.value()->end(),
                            std::bind(&Matcher::operator(), std::cref(_m), std::bind(&stringify<KeywordName>, _1)));
            }

            void visit(const MetadataValueKey<std::shared_ptr<const Choices> > & s)
            {
                if (_flatten)
                {
                    std::string r;
                    for (Choices::ConstIterator c(s.value()->begin()), c_end(s.value()->end()) ;
                            c != c_end ; ++c)
                        for (Choice::ConstIterator i((*c)->begin()), i_end((*c)->end()) ;
                                i != i_end ; ++i)
                        {
                            if (! r.empty())
                                r.append(" ");
                            r.append(stringify((*i)->name_with_prefix()));
                        }

                    result = _m(r);
                }
                else
                {
                    for (Choices::ConstIterator c(s.value()->begin()), c_end(s.value()->end()) ;
                            c != c_end && ! result ; ++c)
                        for (Choice::ConstIterator i((*c)->begin()), i_end((*c)->end()) ;
                                i != i_end && ! result ; ++i)
                            result = _m(stringify((*i)->name_with_prefix()));
                }
            }

            void visit(const MetadataCollectionKey<Set<std::string> > & s)
            {
                using namespace std::placeholders;

                if (_flatten)
                    result = _m(join(s.value()->begin(), s.value()->end(), " "));
                else
                    result = s.value()->end() != std::find_if(s.value()->begin(), s.value()->end(),
                            std::bind(&Matcher::operator(), std::cref(_m), _1));
            }

            void visit(const MetadataCollectionKey<Sequence<std::string> > & s)
            {
                using namespace std::placeholders;

                if (_flatten)
                    result = _m(join(s.value()->begin(), s.value()->end(), " "));
                else
                    result = s.value()->end() != std::find_if(s.value()->begin(), s.value()->end(),
                            std::bind(&Matcher::operator(), std::cref(_m), _1));
            }

            void visit(const MetadataCollectionKey<FSEntrySequence> & s)
            {
                using namespace std::placeholders;

                if (_flatten)
                    result = _m(join(s.value()->begin(), s.value()->end(), " "));
                else
                    result = s.value()->end() != std::find_if(s.value()->begin(), s.value()->end(),
                            std::bind(&Matcher::operator(), std::cref(_m), std::bind(&stringify<FSEntry>, _1)));
            }

            void visit(const MetadataCollectionKey<PackageIDSequence> & s)
            {
                using namespace std::placeholders;

                if (_flatten)
                    result = _m(join(indirect_iterator(s.value()->begin()), indirect_iterator(s.value()->end()), " "));
                else
                    result = indirect_iterator(s.value()->end()) != std::find_if(
                            indirect_iterator(s.value()->begin()), indirect_iterator(s.value()->end()),
                            std::bind(&Matcher::operator(), std::cref(_m), std::bind(&stringify<PackageID>, _1)));
            }

            void visit(const MetadataSpecTreeKey<PlainTextSpecTree> & s)
            {
                if (_flatten)
                {
                    StringifyFormatter f;
                    result = _m(s.pretty_print_flat(f));
                }
                else
                {
                    TreeVisitor v(_key, _visible_only, _env, _id, _m);
                    s.value()->root()->accept(v);
                    result = v.result;
                }
            }

            void visit(const MetadataSpecTreeKey<DependencySpecTree> & s)
            {
                if (_flatten)
                {
                    StringifyFormatter f;
                    result = _m(s.pretty_print_flat(f));
                }
                else
                {
                    TreeVisitor v(_key, _visible_only, _env, _id, _m);
                    s.value()->root()->accept(v);
                    result = v.result;
                }
            }

            void visit(const MetadataSpecTreeKey<SetSpecTree> & s)
            {
                if (_flatten)
                {
                    StringifyFormatter f;
                    result = _m(s.pretty_print_flat(f));
                }
                else
                {
                    TreeVisitor v(_key, _visible_only, _env, _id, _m);
                    s.value()->root()->accept(v);
                    result = v.result;
                }
            }

            void visit(const MetadataSpecTreeKey<LicenseSpecTree> & s)
            {
                if (_flatten)
                {
                    StringifyFormatter f;
                    result = _m(s.pretty_print_flat(f));
                }
                else
                {
                    TreeVisitor v(_key, _visible_only, _env, _id, _m);
                    s.value()->root()->accept(v);
                    result = v.result;
                }
            }

            void visit(const MetadataSpecTreeKey<FetchableURISpecTree> & s)
            {
                if (_flatten)
                {
                    StringifyFormatter f;
                    result = _m(s.pretty_print_flat(f));
                }
                else
                {
                    TreeVisitor v(_key, _visible_only, _env, _id, _m);
                    s.value()->root()->accept(v);
                    result = v.result;
                }
            }

            void visit(const MetadataSpecTreeKey<SimpleURISpecTree> & s)
            {
                if (_flatten)
                {
                    StringifyFormatter f;
                    result = _m(s.pretty_print_flat(f));
                }
                else
                {
                    TreeVisitor v(_key, _visible_only, _env, _id, _m);
                    s.value()->root()->accept(v);
                    result = v.result;
                }
            }

            void visit(const MetadataSpecTreeKey<ProvideSpecTree> & s)
            {
                if (_flatten)
                {
                    StringifyFormatter f;
                    result = _m(s.pretty_print_flat(f));
                }
                else
                {
                    TreeVisitor v(_key, _visible_only, _env, _id, _m);
                    s.value()->root()->accept(v);
                    result = v.result;
                }
            }

            void visit(const MetadataSectionKey & k)
            {
                std::for_each(indirect_iterator(k.begin_metadata()),
                        indirect_iterator(k.end_metadata()), accept_visitor(*this));
            }
    };
}

bool
KeyExtractor::operator() (const Matcher & m, const PackageID & id) const
{
    PackageID::MetadataConstIterator mi(id.find_metadata(_imp->key));
    if (id.end_metadata() == mi)
        return false;

    KeyVisitor v(_imp->key, _imp->flatten, _imp->visible_only, _imp->env, id, m);
    (*mi)->accept(v);
    return v.result;
}

