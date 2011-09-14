/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010, 2011 Ciaran McCreesh
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

#include "cmd_match.hh"
#include "parse_spec_with_nice_error.hh"
#include <paludis/args/args.hh>
#include <paludis/args/do_help.hh>
#include <paludis/name.hh>
#include <paludis/environment.hh>
#include <paludis/repository.hh>
#include <paludis/util/set.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/visitor_cast.hh>
#include <paludis/util/iterator_funcs.hh>
#include <paludis/util/singleton-impl.hh>
#include <paludis/util/accept_visitor.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/generator.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/filter.hh>
#include <paludis/filter_handler.hh>
#include <paludis/selection.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/package_id.hh>
#include <paludis/mask.hh>
#include <paludis/metadata_key.hh>
#include <paludis/choice.hh>
#include <paludis/about.hh>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <list>
#include <string.h>
#include <dlfcn.h>
#include <stdint.h>

#include "config.h"
#include "command_command_line.hh"

using namespace paludis;
using namespace cave;
using std::cout;
using std::endl;

#define STUPID_CAST(type, val) reinterpret_cast<type>(reinterpret_cast<uintptr_t>(val))

namespace
{
    struct ExtrasHandle :
        Singleton<ExtrasHandle>
    {
        typedef bool (* MatchFunction)(const std::string &, const std::string &);

        void * handle;
        MatchFunction match_function;

        ExtrasHandle() :
            handle(0),
            match_function(0)
        {
            handle = ::dlopen(("libcavematchextras_" + stringify(PALUDIS_PC_SLOT) + ".so").c_str(), RTLD_NOW | RTLD_GLOBAL);
            if (! handle)
                throw args::DoHelp("Regular expression match not available because dlopen said " + stringify(::dlerror()));

            match_function = STUPID_CAST(MatchFunction, ::dlsym(handle, "cave_match_extras_match_regex"));
            if (! match_function)
                throw args::DoHelp("Regular expression match not available because dlsym said " + stringify(::dlerror()));
        }

        ~ExtrasHandle()
        {
            if (handle)
                ::dlclose(handle);
        }
    };

    struct MatchCommandLine :
        CaveCommandCommandLine
    {
        virtual std::string app_name() const
        {
            return "cave match";
        }

        virtual std::string app_synopsis() const
        {
            return "Determine whether a particular package version has certain properties";
        }

        virtual std::string app_description() const
        {
            return "Determines whether a particular package version has certain properties. Mostly for "
                "use by 'cave search'; not generally for use by end users.";
        }

        SearchCommandLineMatchOptions match_options;

        MatchCommandLine() :
            match_options(this)
        {
            add_usage_line("spec pattern ...");
        }
    };

    bool match_text(const std::string & text, const std::string & pattern)
    {
        return 0 != strcasestr(text.c_str(), pattern.c_str());
    }

    bool match_exact(const std::string & text, const std::string & pattern)
    {
        return 0 == strcasecmp(text.c_str(), pattern.c_str());
    }

    bool match_regex(const std::string & text, const std::string & pattern)
    {
        return ExtrasHandle::get_instance()->match_function(text, pattern);
    }

    bool match(const std::string & text, const std::string & pattern, const std::string & algorithm)
    {
        if (algorithm == "text")
            return match_text(text, pattern);
        else if (algorithm == "exact")
            return match_exact(text, pattern);
        else if (algorithm == "regex")
            return match_regex(text, pattern);
        else
            throw args::DoHelp("Unknown algoritm '" + algorithm + "'");
    }

    std::string stringify_string_pair(const std::pair<const std::string, std::string> & s)
    {
        if (s.first.empty())
            return s.second;
        else
            return s.first + "=" + s.second;
    }

    struct SpecTreeAsString
    {
        const Environment * const env;
        const std::shared_ptr<const PackageID> id;
        std::list<std::string> & texts;
        const SearchCommandLineMatchOptions & match_options;

        void visit(const GenericSpecTree::NodeType<AllDepSpec>::Type & node)
        {
            std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
        }

        void visit(const GenericSpecTree::NodeType<ConditionalDepSpec>::Type & node)
        {
            if ((! match_options.a_enabled_only.specified()) || node.spec()->condition_met(env, id))
                std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
        }

        void visit(const GenericSpecTree::NodeType<AnyDepSpec>::Type & node)
        {
            std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
        }

        void visit(const GenericSpecTree::NodeType<ExactlyOneDepSpec>::Type & node)
        {
            std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
        }

        void visit(const GenericSpecTree::NodeType<SimpleURIDepSpec>::Type & node)
        {
            texts.push_back(stringify(*node.spec()));
        }

        void visit(const GenericSpecTree::NodeType<URILabelsDepSpec>::Type & node)
        {
            texts.push_back(stringify(*node.spec()));
        }

        void visit(const GenericSpecTree::NodeType<PlainTextDepSpec>::Type & node)
        {
            texts.push_back(stringify(*node.spec()));
        }

        void visit(const GenericSpecTree::NodeType<FetchableURIDepSpec>::Type & node)
        {
            texts.push_back(stringify(*node.spec()));
        }

        void visit(const GenericSpecTree::NodeType<PackageDepSpec>::Type & node)
        {
            texts.push_back(stringify(*node.spec()));
        }

        void visit(const GenericSpecTree::NodeType<BlockDepSpec>::Type & node)
        {
            texts.push_back(stringify(*node.spec()));
        }

        void visit(const GenericSpecTree::NodeType<LicenseDepSpec>::Type & node)
        {
            texts.push_back(stringify(*node.spec()));
        }

        void visit(const GenericSpecTree::NodeType<PlainTextLabelDepSpec>::Type & node)
        {
            texts.push_back(stringify(*node.spec()));
        }

        void visit(const GenericSpecTree::NodeType<DependenciesLabelsDepSpec>::Type & node)
        {
            texts.push_back(stringify(*node.spec()));
        }

        void visit(const GenericSpecTree::NodeType<NamedSetDepSpec>::Type & node)
        {
            texts.push_back(stringify(*node.spec()));
        }
    };

    struct MetadataKeyAsString
    {
        const Environment * const env;
        const std::shared_ptr<const PackageID> id;

        std::list<std::string> & texts;
        const SearchCommandLineMatchOptions & match_options;

        void visit(const MetadataValueKey<std::string> & k)
        {
            texts.push_back(stringify(k.parse_value()));
        }

        void visit(const MetadataValueKey<FSPath> & k)
        {
            texts.push_back(stringify(k.parse_value()));
        }

        void visit(const MetadataValueKey<SlotName> & k)
        {
            texts.push_back(stringify(k.parse_value()));
        }

        void visit(const MetadataValueKey<long> & k)
        {
            texts.push_back(stringify(k.parse_value()));
        }

        void visit(const MetadataValueKey<bool> & k)
        {
            texts.push_back(stringify(k.parse_value()));
        }

        void visit(const MetadataValueKey<std::shared_ptr<const PackageID> > & k)
        {
            texts.push_back(stringify(*k.parse_value()));
        }

        void visit(const MetadataValueKey<std::shared_ptr<const Choices> > & k)
        {
            auto choices(k.parse_value());
            for (Choices::ConstIterator c(choices->begin()), c_end(choices->end()) ;
                    c != c_end ; ++c)
                for (Choice::ConstIterator i((*c)->begin()), i_end((*c)->end()) ;
                        i != i_end ; ++i)
                    texts.push_back(stringify((*i)->name_with_prefix()));
        }

        void visit(const MetadataTimeKey & k)
        {
            texts.push_back(stringify(k.parse_value().seconds()));
        }

        void visit(const MetadataSectionKey & k)
        {
            std::for_each(indirect_iterator(k.begin_metadata()), indirect_iterator(k.end_metadata()),
                    accept_visitor(*this));
        }

        void visit(const MetadataSpecTreeKey<PlainTextSpecTree> & k)
        {
            SpecTreeAsString m = { env, id, texts, match_options };
            k.parse_value()->top()->accept(m);
        }

        void visit(const MetadataSpecTreeKey<RequiredUseSpecTree> & k)
        {
            SpecTreeAsString m = { env, id, texts, match_options };
            k.parse_value()->top()->accept(m);
        }

        void visit(const MetadataSpecTreeKey<DependencySpecTree> & k)
        {
            SpecTreeAsString m = { env, id, texts, match_options };
            k.parse_value()->top()->accept(m);
        }

        void visit(const MetadataSpecTreeKey<LicenseSpecTree> & k)
        {
            SpecTreeAsString m = { env, id, texts, match_options };
            k.parse_value()->top()->accept(m);
        }

        void visit(const MetadataSpecTreeKey<SimpleURISpecTree> & k)
        {
            SpecTreeAsString m = { env, id, texts, match_options };
            k.parse_value()->top()->accept(m);
        }

        void visit(const MetadataSpecTreeKey<FetchableURISpecTree> & k)
        {
            SpecTreeAsString m = { env, id, texts, match_options };
            k.parse_value()->top()->accept(m);
        }

        void visit(const MetadataCollectionKey<Sequence<FSPath> > & k)
        {
            auto v(k.parse_value());
            std::transform(v->begin(), v->end(), std::back_inserter(texts), &stringify<FSPath>);
        }

        void visit(const MetadataCollectionKey<Sequence<std::string> > & k)
        {
            auto v(k.parse_value());
            std::transform(v->begin(), v->end(), std::back_inserter(texts), &stringify<std::string>);
        }

        void visit(const MetadataCollectionKey<Set<std::string> > & k)
        {
            auto v(k.parse_value());
            std::transform(v->begin(), v->end(), std::back_inserter(texts), &stringify<std::string>);
        }

        void visit(const MetadataCollectionKey<Map<std::string, std::string> > & k)
        {
            auto v(k.parse_value());
            std::transform(v->begin(), v->end(), std::back_inserter(texts), stringify_string_pair);
        }

        void visit(const MetadataCollectionKey<Set<KeywordName> > & k)
        {
            auto v(k.parse_value());
            std::transform(v->begin(), v->end(), std::back_inserter(texts), &stringify<KeywordName>);
        }

        void visit(const MetadataCollectionKey<Sequence<std::shared_ptr<const PackageID> > > & k)
        {
            auto v(k.parse_value());
            std::transform(indirect_iterator(v->begin()), indirect_iterator(v->end()),
                    std::back_inserter(texts), &stringify<PackageID>);
        }

        void visit(const MetadataCollectionKey<Maintainers> & k)
        {
            auto v(k.parse_value());
            std::transform(v->begin(), v->end(), std::back_inserter(texts), &stringify<Maintainer>);
        }
    };
}

int
MatchCommand::run(
        const std::shared_ptr<Environment> & env,
        const std::shared_ptr<const Sequence<std::string > > & args
        )
{
    MatchCommandLine cmdline;
    cmdline.run(args, "CAVE", "CAVE_MATCH_OPTIONS", "CAVE_MATCH_CMDLINE");

    if (cmdline.a_help.specified())
    {
        cout << cmdline;
        return EXIT_SUCCESS;
    }

    if (capped_distance(cmdline.begin_parameters(), cmdline.end_parameters(), 2) < 2)
        throw args::DoHelp("match requires at least two parameters");

    PackageDepSpec spec(parse_spec_with_nice_error(*cmdline.begin_parameters(), env.get(), { }, filter::All()));

    const std::shared_ptr<Set<std::string> > patterns(std::make_shared<Set<std::string>>());
    std::copy(next(cmdline.begin_parameters()), cmdline.end_parameters(), patterns->inserter());

    return run_hosted(env, cmdline.match_options, patterns, spec) ? EXIT_SUCCESS : EXIT_FAILURE;
}

bool
MatchCommand::run_hosted(
        const std::shared_ptr<Environment> & env,
        const SearchCommandLineMatchOptions & match_options,
        const std::shared_ptr<const Set<std::string> > & patterns,
        const PackageDepSpec & spec)
{
    const std::shared_ptr<const PackageID> id(*((*env)[selection::RequireExactlyOne(
                    generator::Matches(spec, make_null_shared_ptr(), { }))])->begin());

    std::list<std::string> texts;

    /* also in cmd_search.cc */
    bool default_names_and_descriptions((! match_options.a_name.specified()) &&
            (! match_options.a_description.specified()) && (! match_options.a_key.specified()));

    if (default_names_and_descriptions || match_options.a_name.specified())
        texts.push_back(stringify(id->name()));

    if (default_names_and_descriptions || match_options.a_description.specified())
    {
        if (id->short_description_key())
            texts.push_back(stringify(id->short_description_key()->parse_value()));
        if (id->long_description_key())
            texts.push_back(stringify(id->long_description_key()->parse_value()));
    }

    for (args::StringSetArg::ConstIterator a(match_options.a_key.begin_args()),
            a_end(match_options.a_key.end_args()) ;
            a != a_end ; ++a)
    {
        PackageID::MetadataConstIterator i(id->find_metadata(*a));
        if (i == id->end_metadata())
            continue;

        MetadataKeyAsString m = { env.get(), id, texts, match_options };
        (*i)->accept(m);
    }

    bool any(false), all(true);
    for (auto p(patterns->begin()), p_end(patterns->end()) ;
            p != p_end ; ++p)
    {
        bool current(texts.end() != std::find_if(texts.begin(), texts.end(),
                    std::bind(&match, std::placeholders::_1, *p, match_options.a_type.argument())));

        if (match_options.a_not.specified())
            current = ! current;

        any = any || current;
        all = all && current;
    }

    return match_options.a_and.specified() ? all : any;
}

std::shared_ptr<args::ArgsHandler>
MatchCommand::make_doc_cmdline()
{
    return std::make_shared<MatchCommandLine>();
}

CommandImportance
MatchCommand::importance() const
{
    return ci_internal;
}

