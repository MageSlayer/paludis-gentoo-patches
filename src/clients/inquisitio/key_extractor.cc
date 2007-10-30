/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh
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
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/tr1_functional.hh>
#include <paludis/util/set.hh>
#include <paludis/util/join.hh>
#include <paludis/util/sequence.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/stringify_formatter.hh>
#include <paludis/dep_spec.hh>
#include <paludis/environment.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>
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
    class TreeVisitor :
        public ConstVisitor<GenericSpecTree>,
        public ConstVisitor<GenericSpecTree>::VisitConstSequence<TreeVisitor, AllDepSpec>,
        public ConstVisitor<GenericSpecTree>::VisitConstSequence<TreeVisitor, AnyDepSpec>
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

            using ConstVisitor<GenericSpecTree>::VisitConstSequence<TreeVisitor, AllDepSpec>::visit_sequence;
            using ConstVisitor<GenericSpecTree>::VisitConstSequence<TreeVisitor, AnyDepSpec>::visit_sequence;

            void visit_sequence(const UseDepSpec & u,
                    GenericSpecTree::ConstSequenceIterator cur,
                    GenericSpecTree::ConstSequenceIterator end)
            {
                if (! result)
                {
                    result |= _m(stringify(u));

                    if (! result)
                    {
                        if (! _visible_only)
                            std::for_each(cur, end, accept_visitor(*this));
                        else if (u.inverse() ^ _env.query_use(u.flag(), _id))
                            std::for_each(cur, end, accept_visitor(*this));
                    }
                }
            }

            void visit_leaf(const SimpleURIDepSpec & s)
            {
                if (! result)
                    result |= _m(stringify(s));
            }

            void visit_leaf(const PlainTextDepSpec & s)
            {
                if (! result)
                    result |= _m(stringify(s));
            }

            void visit_leaf(const FetchableURIDepSpec & s)
            {
                if (! result)
                    result |= _m(stringify(s));
            }

            void visit_leaf(const PackageDepSpec & s)
            {
                if (! result)
                    result |= _m(stringify(s));
            }

            void visit_leaf(const BlockDepSpec & s)
            {
                if (! result)
                    result |= _m(stringify(s));
            }

            void visit_leaf(const LicenseDepSpec & s)
            {
                if (! result)
                    result |= _m(stringify(s));
            }

            void visit_leaf(const URILabelsDepSpec & s)
            {
                if (! result)
                    result |= _m(stringify(s));
            }

            void visit_leaf(const DependencyLabelsDepSpec & s)
            {
                if (! result)
                    result |= _m(stringify(s));
            }

            void visit_leaf(const NamedSetDepSpec & s)
            {
                if (! result)
                    result |= _m(stringify(s));
            }
    };

    class KeyVisitor :
        public ConstVisitor<MetadataKeyVisitorTypes>
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

            void visit(const MetadataStringKey & s)
            {
                result = _m(s.value());
            }

            void visit(const MetadataTimeKey & s)
            {
                result = _m(stringify(s.value()));
            }

            void visit(const MetadataContentsKey &)
            {
            }

            void visit(const MetadataRepositoryMaskInfoKey &)
            {
            }

            void visit(const MetadataFSEntryKey & s)
            {
                result = _m(stringify(s.value()));
            }

            void visit(const MetadataPackageIDKey & s)
            {
                result = _m(stringify(*s.value()));
            }

            void visit(const MetadataSetKey<UseFlagNameSet> & s)
            {
                using namespace tr1::placeholders;

                if (_flatten)
                    result = _m(join(s.value()->begin(), s.value()->end(), " "));
                else
                    result = s.value()->end() != std::find_if(s.value()->begin(), s.value()->end(),
                            tr1::bind(&Matcher::operator(), tr1::cref(_m), tr1::bind(&stringify<UseFlagName>, _1)));
            }

            void visit(const MetadataSetKey<IUseFlagSet> & s)
            {
                using namespace tr1::placeholders;

                if (_flatten)
                    result = _m(join(s.value()->begin(), s.value()->end(), " "));
                else
                    result = s.value()->end() != std::find_if(s.value()->begin(), s.value()->end(),
                            tr1::bind(&Matcher::operator(), tr1::cref(_m), tr1::bind(&stringify<IUseFlag>, _1)));
            }

            void visit(const MetadataSetKey<KeywordNameSet> & s)
            {
                using namespace tr1::placeholders;

                if (_flatten)
                    result = _m(join(s.value()->begin(), s.value()->end(), " "));
                else
                    result = s.value()->end() != std::find_if(s.value()->begin(), s.value()->end(),
                            tr1::bind(&Matcher::operator(), tr1::cref(_m), tr1::bind(&stringify<KeywordName>, _1)));
            }

            void visit(const MetadataSetKey<Set<std::string> > & s)
            {
                using namespace tr1::placeholders;

                if (_flatten)
                    result = _m(join(s.value()->begin(), s.value()->end(), " "));
                else
                    result = s.value()->end() != std::find_if(s.value()->begin(), s.value()->end(),
                            tr1::bind(&Matcher::operator(), tr1::cref(_m), _1));
            }

            void visit(const MetadataSetKey<PackageIDSequence> & s)
            {
                using namespace tr1::placeholders;

                if (_flatten)
                    result = _m(join(indirect_iterator(s.value()->begin()), indirect_iterator(s.value()->end()), " "));
                else
                    result = indirect_iterator(s.value()->end()) != std::find_if(
                            indirect_iterator(s.value()->begin()), indirect_iterator(s.value()->end()),
                            tr1::bind(&Matcher::operator(), tr1::cref(_m), tr1::bind(&stringify<PackageID>, _1)));
            }

            void visit(const MetadataSpecTreeKey<RestrictSpecTree> & s)
            {
                if (_flatten)
                {
                    StringifyFormatter f;
                    result = _m(s.pretty_print_flat(f));
                }
                else
                {
                    TreeVisitor v(_key, _visible_only, _env, _id, _m);
                    s.value()->accept(v);
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
                    s.value()->accept(v);
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
                    s.value()->accept(v);
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
                    s.value()->accept(v);
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
                    s.value()->accept(v);
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
                    s.value()->accept(v);
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
                    s.value()->accept(v);
                    result = v.result;
                }
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

