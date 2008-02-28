/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008 Ciaran McCreesh
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

#include <paludis/dep_spec.hh>
#include <paludis/repositories/e/eapi.hh>
#include <paludis/repositories/e/dep_lexer.hh>
#include <paludis/repositories/e/dep_parser.hh>
#include <paludis/repositories/e/package_dep_spec.hh>
#include <paludis/repositories/e/conditional_dep_spec.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/tr1_functional.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/iterator_funcs.hh>
#include <paludis/util/kc.hh>
#include <stack>
#include <set>

/** \file
 * Implementation for dep_parser.hh things.
 *
 * \ingroup grpdepparser
 */

using namespace paludis;
using namespace paludis::erepository;

#include <paludis/repositories/e/dep_parser-se.cc>

DepStringParseError::DepStringParseError(const std::string & d,
        const std::string & m) throw () :
    DepStringError(d, "in parse phase: " + m)
{
}

DepStringNestingError::DepStringNestingError(const std::string & dep_string) throw () :
    DepStringParseError(dep_string, "improperly balanced parentheses")
{
}

namespace
{
    struct LabelsAreURI;
    struct LabelsAreDependency;

    enum DepParserState
    {
        dps_initial,
        dps_had_double_bar,
        dps_had_double_bar_space,
        dps_had_paren,
        dps_had_use_flag,
        dps_had_use_flag_space,
        dps_had_text_arrow,
        dps_had_text_arrow_space,
        dps_had_text_arrow_text,
        dps_had_label
    };

    using namespace tr1::placeholders;

    struct ParsePackageDepSpec
    {
        const EAPI & _eapi;
        const tr1::shared_ptr<const PackageID> _id;

        ParsePackageDepSpec(const EAPI & e, const tr1::shared_ptr<const PackageID> & i) :
            _eapi(e),
            _id(i)
        {
        }

        template <typename H_>
        void
        add(const std::string & s, tr1::function<void (tr1::shared_ptr<ConstAcceptInterface<H_> >)> & p) const
        {
            p(tr1::shared_ptr<TreeLeaf<H_, PackageDepSpec> >(new TreeLeaf<H_, PackageDepSpec>(
                            tr1::shared_ptr<PackageDepSpec>(new PackageDepSpec(parse_e_package_dep_spec(s, _eapi, _id))))));
        }

        template <typename H_>
        void
        add_arrow(const std::string & lhs, const std::string & rhs, tr1::function<void (tr1::shared_ptr<ConstAcceptInterface<H_> >)> &) const
        {
            throw DepStringParseError(lhs + " -> " + rhs, "Arrows not allowed in this context");
        }
    };

    struct ParsePackageOrBlockDepSpec
    {
        const EAPI & _eapi;
        const tr1::shared_ptr<const PackageID> _id;

        ParsePackageOrBlockDepSpec(const EAPI & e, const tr1::shared_ptr<const PackageID> & i) :
            _eapi(e),
            _id(i)
        {
        }

        template <typename H_>
        void
        add(const std::string & s, tr1::function<void (tr1::shared_ptr<ConstAcceptInterface<H_> >)> & p) const
        {
            if (s.empty() || '!' != s.at(0))
                p(tr1::shared_ptr<TreeLeaf<H_, PackageDepSpec> >(new TreeLeaf<H_, PackageDepSpec>(
                                tr1::shared_ptr<PackageDepSpec>(new PackageDepSpec(
                                        parse_e_package_dep_spec(s, _eapi, _id))))));
            else
                p(tr1::shared_ptr<TreeLeaf<H_, BlockDepSpec> >(new TreeLeaf<H_, BlockDepSpec>(
                                tr1::shared_ptr<BlockDepSpec>(new BlockDepSpec(
                                        tr1::shared_ptr<PackageDepSpec>(new PackageDepSpec(
                                                parse_e_package_dep_spec(s.substr(1), _eapi, _id))))))));
        }

        template <typename H_>
        void
        add_arrow(const std::string & lhs, const std::string & rhs, tr1::function<void (tr1::shared_ptr<ConstAcceptInterface<H_> >)> &) const
        {
            throw DepStringParseError(lhs + " -> " + rhs, "Arrows not allowed in this context");
        }
    };

    struct ParseTextDepSpec
    {
        template <typename H_>
        void
        add(const std::string & s, tr1::function<void (tr1::shared_ptr<ConstAcceptInterface<H_> >)> & p) const
        {
            p(tr1::shared_ptr<TreeLeaf<H_, PlainTextDepSpec> >(new TreeLeaf<H_, PlainTextDepSpec>(
                            tr1::shared_ptr<PlainTextDepSpec>(new PlainTextDepSpec(s)))));
        }

        template <typename H_>
        void
        add_arrow(const std::string & lhs, const std::string & rhs, tr1::function<void (tr1::shared_ptr<ConstAcceptInterface<H_> >)> &) const
        {
            throw DepStringParseError(lhs + " -> " + rhs, "Arrows not allowed in this context");
        }
    };

    struct ParseLicenseDepSpec
    {
        template <typename H_>
        void
        add(const std::string & s, tr1::function<void (tr1::shared_ptr<ConstAcceptInterface<H_> >)> & p) const
        {
            p(tr1::shared_ptr<TreeLeaf<H_, LicenseDepSpec> >(new TreeLeaf<H_, LicenseDepSpec>(
                            tr1::shared_ptr<LicenseDepSpec>(new LicenseDepSpec(s)))));
        }

        template <typename H_>
        void
        add_arrow(const std::string & lhs, const std::string & rhs, tr1::function<void (tr1::shared_ptr<ConstAcceptInterface<H_> >)> &) const
        {
            throw DepStringParseError(lhs + " -> " + rhs, "Arrows not allowed in this context");
        }
    };

    struct ParseFetchableURIDepSpec
    {
        const bool _supports_arrow;

        ParseFetchableURIDepSpec(bool a) :
            _supports_arrow(a)
        {
        }

        template <typename H_>
        void
        add(const std::string & s, tr1::function<void (tr1::shared_ptr<ConstAcceptInterface<H_> >)> & p) const
        {
            p(tr1::shared_ptr<TreeLeaf<H_, FetchableURIDepSpec> >(new TreeLeaf<H_, FetchableURIDepSpec>(
                            tr1::shared_ptr<FetchableURIDepSpec>(new FetchableURIDepSpec(s)))));
        }

        template <typename H_>
        void
        add_arrow(const std::string & lhs, const std::string & rhs, tr1::function<void (tr1::shared_ptr<ConstAcceptInterface<H_> >)> & p) const
        {
            if (_supports_arrow)
                p(tr1::shared_ptr<TreeLeaf<H_, FetchableURIDepSpec> >(new TreeLeaf<H_, FetchableURIDepSpec>(
                                tr1::shared_ptr<FetchableURIDepSpec>(new FetchableURIDepSpec(lhs + " -> " + rhs)))));
            else
                throw DepStringParseError(lhs + " -> " + rhs, "Arrows not allowed in this EAPI");
        }
    };

    struct ParseSimpleURIDepSpec
    {
        template <typename H_>
        void
        add(const std::string & s, tr1::function<void (tr1::shared_ptr<ConstAcceptInterface<H_> >)> & p) const
        {
            p(tr1::shared_ptr<TreeLeaf<H_, SimpleURIDepSpec> >(new TreeLeaf<H_, SimpleURIDepSpec>(
                            tr1::shared_ptr<SimpleURIDepSpec>(new SimpleURIDepSpec(s)))));
        }

        template <typename H_>
        void
        add_arrow(const std::string & lhs, const std::string & rhs, tr1::function<void (tr1::shared_ptr<ConstAcceptInterface<H_> >)> &) const
        {
            throw DepStringParseError(lhs + " -> " + rhs, "Arrows not allowed in this context");
        }
    };

    template <typename H_, bool>
    struct HandleUse
    {
        static void handle(const std::string &, const std::string & i,
                const tr1::shared_ptr<const PackageID> & id, const Environment * const env,
                const EAPI & eapi, std::stack<std::pair<tr1::function<void (tr1::shared_ptr<ConstAcceptInterface<H_> >)>, bool> > & stack)
        {
            tr1::shared_ptr<ConstTreeSequence<H_, ConditionalDepSpec> > a(
                    new ConstTreeSequence<H_, ConditionalDepSpec>(tr1::shared_ptr<ConditionalDepSpec>(
                            new ConditionalDepSpec(parse_e_conditional_dep_spec(i, env, id, eapi)))));
            stack.top().first(a);
            stack.push(std::make_pair(tr1::function<void (tr1::shared_ptr<ConstAcceptInterface<H_> >)>(
                    tr1::bind(&ConstTreeSequence<H_, ConditionalDepSpec>::add, a.get(), _1)), false));
        }
    };

    template <typename H_>
    struct HandleUse<H_, false>
    {
        static void handle(const std::string & s, const std::string &,
                const tr1::shared_ptr<const PackageID> &, const Environment * const,
                const EAPI &, std::stack<std::pair<tr1::function<void (tr1::shared_ptr<ConstAcceptInterface<H_> >)>, bool> > &)
        {
            throw DepStringParseError(s, "use? group is not allowed here");
        }
    };

    template <typename H_, bool>
    struct HandleAny
    {
        static void handle(const std::string &, std::stack<std::pair<tr1::function<void (tr1::shared_ptr<ConstAcceptInterface<H_> >)>, bool> > &
                stack)
        {
             tr1::shared_ptr<ConstTreeSequence<H_, AnyDepSpec> > a(new ConstTreeSequence<H_, AnyDepSpec>(
                         tr1::shared_ptr<AnyDepSpec>(new AnyDepSpec)));
             stack.top().first(a);
             stack.push(std::make_pair(tr1::function<void (tr1::shared_ptr<ConstAcceptInterface<H_> >)>(
                     tr1::bind(&ConstTreeSequence<H_, AnyDepSpec>::add, a.get(), _1)), true));
        }
    };

    template <typename H_>
    struct HandleAny<H_, false>
    {
        static void handle(const std::string & s, std::stack<std::pair<tr1::function<void (tr1::shared_ptr<ConstAcceptInterface<H_> >)>, bool> > &)
        {
             throw DepStringParseError(s, "|| is not allowed here");
        }
    };

    template <typename H_, typename K_>
    struct HandleLabel
    {
        static void add(const std::string & s, tr1::function<void (tr1::shared_ptr<ConstAcceptInterface<H_> >)> &,
                const EAPI &)
        {
            throw DepStringParseError(s, "label is not allowed here");
        }
    };

    template <typename H_>
    struct HandleLabel<H_, LabelsAreURI>
    {
        static void add(const std::string & s, tr1::function<void (tr1::shared_ptr<ConstAcceptInterface<H_> >)> & p,
                const EAPI & e)
        {
            if (e[k::supported()])
                p(tr1::shared_ptr<TreeLeaf<H_, URILabelsDepSpec> >(
                            new TreeLeaf<H_, URILabelsDepSpec>(parse_uri_label(s, e))));
            else
                throw DepStringParseError(s, "URI labels not allowed in this EAPI");
        }
    };

    template <typename H_>
    struct HandleLabel<H_, LabelsAreDependency>
    {
        static void add(const std::string & s, tr1::function<void (tr1::shared_ptr<ConstAcceptInterface<H_> >)> & p,
                const EAPI & e)
        {
            if (e[k::supported()])
                p(tr1::shared_ptr<TreeLeaf<H_, DependencyLabelsDepSpec> >(
                            new TreeLeaf<H_, DependencyLabelsDepSpec>(parse_dependency_label(s, e))));
            else
                throw DepStringParseError(s, "Dependency labels not allowed in this EAPI");
        }
    };
}

namespace
{
    template <typename H_, typename I_, bool any_, bool use_, typename Label_>
    tr1::shared_ptr<typename H_::ConstItem>
    parse(const std::string & s, bool disallow_any_use, const I_ & p, const Environment * const env,
            const EAPI & e, const tr1::shared_ptr<const PackageID> & id)
    {
        Context context("When parsing dependency string '" + s + "':");

        if (! id)
            throw InternalError(PALUDIS_HERE, "! id");

        tr1::shared_ptr<ConstTreeSequence<H_, AllDepSpec> > result(
            new ConstTreeSequence<H_, AllDepSpec>(tr1::shared_ptr<AllDepSpec>(new AllDepSpec)));
        std::stack<std::pair<tr1::function<void (tr1::shared_ptr<ConstAcceptInterface<H_> >)>, bool> > stack;
        stack.push(std::make_pair(tr1::function<void (tr1::shared_ptr<ConstAcceptInterface<H_> >)>(
                tr1::bind(&ConstTreeSequence<H_, AllDepSpec>::add, result.get(), _1)), false));

        std::string arrow_lhs;
        DepParserState state(dps_initial);
        DepLexer lexer(s);
        DepLexer::ConstIterator i(lexer.begin()), i_end(lexer.end());

        for ( ; i != i_end ; ++i)
        {
            Context local_context("When handling lexer token '" + i->second +
                    "' (" + stringify(i->first) + "):");
            do
            {
                switch (state)
                {
                    case dps_initial:
                        do
                        {
                            switch (i->first)
                            {
                                case dpl_whitespace:
                                     continue;

                                case dpl_arrow:
                                     throw DepStringParseError(s, "Arrow not allowed here");

                                case dpl_text:
                                     {
                                         if (i->second.empty())
                                             throw DepStringParseError(i->second, "Empty text entry");

                                         DepLexer::ConstIterator i_fwd(next(i));
                                         if (i_fwd != i_end && i_fwd->first == dpl_whitespace && ++i_fwd != i_end
                                                 && i_fwd->first == dpl_arrow)
                                         {
                                             arrow_lhs = i->second;
                                             i = i_fwd;
                                             state = dps_had_text_arrow;
                                         }
                                         else
                                             p.template add<H_>(i->second, stack.top().first);
                                     }
                                     continue;

                                case dpl_open_paren:
                                     {
                                         tr1::shared_ptr<ConstTreeSequence<H_, AllDepSpec> > a(new ConstTreeSequence<H_, AllDepSpec>(
                                                     tr1::shared_ptr<AllDepSpec>(new AllDepSpec)));
                                         stack.top().first(a);
                                         stack.push(std::make_pair(tr1::function<void (tr1::shared_ptr<ConstAcceptInterface<H_> >)>(
                                                         tr1::bind(&ConstTreeSequence<H_, AllDepSpec>::add, a.get(), _1)), false));
                                         state = dps_had_paren;
                                     }
                                     continue;

                                case dpl_close_paren:
                                     if (stack.empty())
                                         throw DepStringNestingError(s);
                                     stack.pop();
                                     if (stack.empty())
                                         throw DepStringNestingError(s);
                                     state = dps_had_paren;
                                     continue;

                                case dpl_double_bar:
                                     HandleAny<H_, any_>::handle(s, stack);
                                     state = dps_had_double_bar;
                                     continue;

                                case dpl_use_flag:
                                     if (use_ && disallow_any_use && stack.top().second)
                                         throw DepStringParseError(s, "use? group is not allowed immediately under a || ( )");
                                     HandleUse<H_, use_>::handle(s, i->second, id, env, e, stack);
                                     state = dps_had_use_flag;
                                     continue;

                                case dpl_label:
                                     HandleLabel<H_, Label_>::add(i->second, stack.top().first, e);
                                     state = dps_had_label;
                                     continue;
                            }
                            throw InternalError(PALUDIS_HERE,
                                    "dps_initial: i->first is " + stringify(i->first));

                        } while (0);
                        continue;

                    case dps_had_double_bar:
                        do
                        {
                            switch (i->first)
                            {
                                case dpl_whitespace:
                                    state = dps_had_double_bar_space;
                                    continue;

                                case dpl_text:
                                case dpl_arrow:
                                case dpl_use_flag:
                                case dpl_double_bar:
                                case dpl_open_paren:
                                case dpl_close_paren:
                                case dpl_label:
                                    throw DepStringParseError(s, "Expected space after '||'");
                            }
                            throw InternalError(PALUDIS_HERE,
                                    "dps_had_double_bar: i->first is " + stringify(i->first));

                        } while (0);
                        continue;

                    case dps_had_double_bar_space:
                        do
                        {
                            switch (i->first)
                            {
                                case dpl_open_paren:
                                    state = dps_initial;
                                    continue;

                                case dpl_whitespace:
                                case dpl_text:
                                case dpl_use_flag:
                                case dpl_double_bar:
                                case dpl_close_paren:
                                case dpl_arrow:
                                case dpl_label:
                                    throw DepStringParseError(s, "Expected '(' after '|| '");
                            }
                            throw InternalError(PALUDIS_HERE,
                                    "dps_had_double_bar_space: i->first is " + stringify(i->first));
                        } while (0);
                        continue;

                    case dps_had_paren:
                        do
                        {
                            switch (i->first)
                            {
                                case dpl_whitespace:
                                    state = dps_initial;
                                    continue;

                                case dpl_text:
                                case dpl_use_flag:
                                case dpl_double_bar:
                                case dpl_open_paren:
                                case dpl_close_paren:
                                case dpl_arrow:
                                case dpl_label:
                                    throw DepStringParseError(s, "Expected space after '(' or ')'");
                            }
                            throw InternalError(PALUDIS_HERE,
                                    "dps_had_paren: i->first is " + stringify(i->first));
                        } while (0);
                        continue;

                    case dps_had_use_flag:
                        do
                        {
                            switch (i->first)
                            {
                                case dpl_whitespace:
                                    state = dps_had_use_flag_space;
                                    continue;

                                case dpl_text:
                                case dpl_use_flag:
                                case dpl_double_bar:
                                case dpl_open_paren:
                                case dpl_close_paren:
                                case dpl_arrow:
                                case dpl_label:
                                    throw DepStringParseError(s, "Expected space after use flag");
                            }
                            throw InternalError(PALUDIS_HERE,
                                    "dps_had_use_flag: i->first is " + stringify(i->first));
                        } while (0);
                        continue;

                    case dps_had_use_flag_space:
                        do
                        {
                            switch (i->first)
                            {
                                case dpl_open_paren:
                                    state = dps_had_paren;
                                    continue;

                                case dpl_whitespace:
                                case dpl_text:
                                case dpl_use_flag:
                                case dpl_double_bar:
                                case dpl_close_paren:
                                case dpl_arrow:
                                case dpl_label:
                                    throw DepStringParseError(s, "Expected '(' after use flag");
                            }
                            throw InternalError(PALUDIS_HERE,
                                    "dps_had_use_flag_space: i->first is " + stringify(i->first));
                        } while (0);
                        continue;

                    case dps_had_label:
                        do
                        {
                            switch (i->first)
                            {
                                case dpl_whitespace:
                                    state = dps_initial;
                                    continue;

                                case dpl_text:
                                case dpl_use_flag:
                                case dpl_double_bar:
                                case dpl_open_paren:
                                case dpl_close_paren:
                                case dpl_arrow:
                                case dpl_label:
                                    throw DepStringParseError(s, "Expected space after label");
                            }
                            throw InternalError(PALUDIS_HERE,
                                    "dps_had_label: i->first is " + stringify(i->first));
                        } while (0);
                        continue;

                    case dps_had_text_arrow:
                        do
                        {
                            switch (i->first)
                            {
                                case dpl_whitespace:
                                    state = dps_had_text_arrow_space;
                                    continue;

                                case dpl_text:
                                case dpl_open_paren:
                                case dpl_use_flag:
                                case dpl_double_bar:
                                case dpl_close_paren:
                                case dpl_arrow:
                                case dpl_label:
                                    throw DepStringParseError(s, "Expected whitespace after arrow");
                            }
                            throw InternalError(PALUDIS_HERE,
                                    "dps_had_text_arrow: i->first is " + stringify(i->first));
                        } while (0);
                        continue;

                    case dps_had_text_arrow_space:
                        do
                        {
                            switch (i->first)
                            {
                                case dpl_whitespace:
                                    continue;

                                case dpl_text:
                                    state = dps_had_text_arrow_text;
                                    p.template add_arrow<H_>(arrow_lhs, i->second, stack.top().first);
                                    continue;

                                case dpl_open_paren:
                                case dpl_use_flag:
                                case dpl_double_bar:
                                case dpl_close_paren:
                                case dpl_arrow:
                                case dpl_label:
                                    throw DepStringParseError(s, "Expected text after whitespace after arrow");
                            }
                            throw InternalError(PALUDIS_HERE,
                                    "dps_had_text_arrow_space: i->first is " + stringify(i->first));
                        } while (0);
                        continue;

                    case dps_had_text_arrow_text:
                        do
                        {
                            switch (i->first)
                            {
                                case dpl_whitespace:
                                    state = dps_initial;
                                    continue;

                                case dpl_text:
                                case dpl_open_paren:
                                case dpl_use_flag:
                                case dpl_close_paren:
                                case dpl_double_bar:
                                case dpl_arrow:
                                case dpl_label:
                                    throw DepStringParseError(s, "Expected whitespace after text after whitespace after arrow");
                            }
                            throw InternalError(PALUDIS_HERE,
                                    "dps_had_text_arrow_text: i->first is " + stringify(i->first));
                        }
                        while (0);
                        continue;
                }
                throw InternalError(PALUDIS_HERE,
                        "state is " + stringify(state));

            } while (0);
        }

        if (stack.empty())
            throw DepStringNestingError(s);

        switch (state)
        {
            case dps_initial:
            case dps_had_paren:
            case dps_had_text_arrow_text:
            case dps_had_text_arrow_space:
            case dps_had_label:
                break;

            case dps_had_double_bar_space:
            case dps_had_double_bar:
            case dps_had_use_flag:
            case dps_had_use_flag_space:
            case dps_had_text_arrow:
                throw DepStringParseError(s, "Unexpected end of string");
        }

        stack.pop();
        if (! stack.empty())
            throw DepStringNestingError(s);
        return result;
    }
}

tr1::shared_ptr<DependencySpecTree::ConstItem>
paludis::erepository::parse_depend(const std::string & s, const Environment * const env,
        const tr1::shared_ptr<const PackageID> & id, const EAPI & e)
{
    Context c("When parsing dependency string '" + s + "' using EAPI '" + e[k::name()] + "':");

    if (! e[k::supported()])
        throw DepStringParseError(s, "Don't know how to parse EAPI '" + e[k::name()] + "' dependencies");

    return parse<DependencySpecTree, ParsePackageOrBlockDepSpec, true, true, LabelsAreDependency>(s,
            (*e[k::supported()])[k::dependency_spec_tree_parse_options()][dstpo_disallow_any_use],
            ParsePackageOrBlockDepSpec(e, id), env, e, id);
}

tr1::shared_ptr<ProvideSpecTree::ConstItem>
paludis::erepository::parse_provide(const std::string & s, const Environment * const env,
        const tr1::shared_ptr<const PackageID> & id, const EAPI & e)
{
    Context c("When parsing provide string '" + s + "' using EAPI '" + e[k::name()] + "':");

    if (! e[k::supported()])
        throw DepStringParseError(s, "Don't know how to parse EAPI '" + e[k::name()] + "' provides");

    return parse<ProvideSpecTree, ParsePackageDepSpec, false, true, void>(s, false,
            ParsePackageDepSpec(e, tr1::shared_ptr<const PackageID>()), env, e, id);
}

tr1::shared_ptr<RestrictSpecTree::ConstItem>
paludis::erepository::parse_restrict(const std::string & s, const Environment * const env,
        const tr1::shared_ptr<const PackageID> & id, const EAPI & e)
{
    Context c("When parsing restrict string '" + s + "' using EAPI '" + e[k::name()] + "':");

    if (! e[k::supported()])
        throw DepStringParseError(s, "Don't know how to parse EAPI '" + e[k::name()] + "' restrictions");

    return parse<RestrictSpecTree, ParseTextDepSpec, false, true, void>(s, false,
            ParseTextDepSpec(), env, e, id);
}

tr1::shared_ptr<FetchableURISpecTree::ConstItem>
paludis::erepository::parse_fetchable_uri(const std::string & s, const Environment * const env,
        const tr1::shared_ptr<const PackageID> & id, const EAPI & e)
{
    Context c("When parsing fetchable URI string '" + s + "' using EAPI '" + e[k::name()] + "':");

    if (! e[k::supported()])
        throw DepStringParseError(s, "Don't know how to parse EAPI '" + e[k::name()] + "' URIs");

    return parse<FetchableURISpecTree, ParseFetchableURIDepSpec, false, true, LabelsAreURI>(s, false,
            ParseFetchableURIDepSpec((*e[k::supported()])[k::dependency_spec_tree_parse_options()][dstpo_uri_supports_arrow]),
            env, e, id);
}

tr1::shared_ptr<SimpleURISpecTree::ConstItem>
paludis::erepository::parse_simple_uri(const std::string & s, const Environment * const env,
        const tr1::shared_ptr<const PackageID> & id, const EAPI & e)
{
    Context c("When parsing simple URI string '" + s + "' using EAPI '" + e[k::name()] + "':");

    if (! e[k::supported()])
        throw DepStringParseError(s, "Don't know how to parse EAPI '" + e[k::name()] + "' URIs");

    return parse<SimpleURISpecTree, ParseSimpleURIDepSpec, false, true, void>(s, false,
            ParseSimpleURIDepSpec(), env, e, id);
}

tr1::shared_ptr<LicenseSpecTree::ConstItem>
paludis::erepository::parse_license(const std::string & s, const Environment * const env,
        const tr1::shared_ptr<const PackageID> & id, const EAPI & e)
{
    Context c("When parsing license string '" + s + "' using EAPI '" + e[k::name()] + "':");

    if (! e[k::supported()])
        throw DepStringParseError(s, "Don't know how to parse EAPI '" + e[k::name()] + "' licenses");

    return parse<LicenseSpecTree, ParseLicenseDepSpec, true, true, void>(s,
            true, ParseLicenseDepSpec(), env, e, id);
}

tr1::shared_ptr<URILabelsDepSpec>
paludis::erepository::parse_uri_label(const std::string & s, const EAPI & e)
{
    Context context("When parsing label string '" + s + "' using EAPI '" + e[k::name()] + "':");

    if (s.empty())
        throw DepStringParseError(s, "Empty label");

    std::string c((*e[k::supported()])[k::uri_labels()].class_for_label(s.substr(0, s.length() - 1)));
    if (c.empty())
        throw DepStringParseError(s, "Unknown label");

    tr1::shared_ptr<URILabelsDepSpec> l(new LabelsDepSpec<URILabelVisitorTypes>);

    if (c == "URIMirrorsThenListedLabel")
        l->add_label(make_shared_ptr(new URIMirrorsThenListedLabel(s.substr(0, s.length() - 1))));
    else if (c == "URIMirrorsOnlyLabel")
        l->add_label(make_shared_ptr(new URIMirrorsOnlyLabel(s.substr(0, s.length() - 1))));
    else if (c == "URIListedOnlyLabel")
        l->add_label(make_shared_ptr(new URIListedOnlyLabel(s.substr(0, s.length() - 1))));
    else if (c == "URIListedThenMirrorsLabel")
        l->add_label(make_shared_ptr(new URIListedThenMirrorsLabel(s.substr(0, s.length() - 1))));
    else if (c == "URILocalMirrorsOnlyLabel")
        l->add_label(make_shared_ptr(new URILocalMirrorsOnlyLabel(s.substr(0, s.length() - 1))));
    else if (c == "URIManualOnlyLabel")
        l->add_label(make_shared_ptr(new URIManualOnlyLabel(s.substr(0, s.length() - 1))));
    else
        throw DepStringParseError(s, "Label '" + s + "' maps to unknown class '" + c + "'");

    return l;
}

tr1::shared_ptr<DependencyLabelsDepSpec>
paludis::erepository::parse_dependency_label(const std::string & s, const EAPI & e)
{
    Context context("When parsing label string '" + s + "' using EAPI '" + e[k::name()] + "':");

    if (s.empty())
        throw DepStringParseError(s, "Empty label");

    std::set<std::string> labels;
    std::string label(s.substr(0, s.length() - 1));
    tokenise<delim_kind::AnyOfTag, delim_mode::DelimiterTag>(label, ",+", "", std::inserter(labels, labels.end()));

    tr1::shared_ptr<DependencyLabelsDepSpec> l(new DependencyLabelsDepSpec);

    for (std::set<std::string>::iterator it = labels.begin(), it_e = labels.end(); it != it_e; ++it)
    {
        std::string c((*e[k::supported()])[k::dependency_labels()].class_for_label(*it));
        if (c.empty())
            throw DepStringParseError(s, "Unknown label '" + *it + "'");

        if (c == "DependencyHostLabel")
            l->add_label(make_shared_ptr(new DependencyHostLabel(*it)));
        else if (c == "DependencyTargetLabel")
            l->add_label(make_shared_ptr(new DependencyTargetLabel(*it)));
        else if (c == "DependencyBuildLabel")
            l->add_label(make_shared_ptr(new DependencyBuildLabel(*it)));
        else if (c == "DependencyRunLabel")
            l->add_label(make_shared_ptr(new DependencyRunLabel(*it)));
        else if (c == "DependencyPostLabel")
            l->add_label(make_shared_ptr(new DependencyPostLabel(*it)));
        else if (c == "DependencyInstallLabel")
            l->add_label(make_shared_ptr(new DependencyInstallLabel(*it)));
        else if (c == "DependencyCompileLabel")
            l->add_label(make_shared_ptr(new DependencyCompileLabel(*it)));
        else if (c == "DependencySuggestedLabel")
            l->add_label(make_shared_ptr(new DependencySuggestedLabel(*it)));
        else if (c == "DependencyRecommendedLabel")
            l->add_label(make_shared_ptr(new DependencyRecommendedLabel(*it)));
        else if (c == "DependencyRequiredLabel")
            l->add_label(make_shared_ptr(new DependencyRequiredLabel(*it)));
        else if (c == "DependencyAnyLabel")
            l->add_label(make_shared_ptr(new DependencyAnyLabel(*it)));
        else if (c == "DependencyMineLabel")
            l->add_label(make_shared_ptr(new DependencyMineLabel(*it)));
        else if (c == "DependencyPrimaryLabel")
            l->add_label(make_shared_ptr(new DependencyPrimaryLabel(*it)));
        else if (c == "DependencyABILabel")
            l->add_label(make_shared_ptr(new DependencyABILabel(*it)));
        else
            throw DepStringParseError(s, "Label '" + *it + "' maps to unknown class '" + c + "'");
    }

    return l;
}

