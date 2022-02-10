/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#include <paludis/repositories/e/source_uri_finder.hh>
#include <paludis/repositories/e/eapi.hh>
#include <paludis/util/log.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/strip.hh>
#include <paludis/repository.hh>
#include <paludis/environment.hh>
#include <paludis/action.hh>

#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/sequence-impl.hh>
#include <paludis/util/set-impl.hh>
#include <paludis/util/pimp-impl.hh>

#include <list>
#include <algorithm>

using namespace paludis;
using namespace paludis::erepository;

typedef std::list<std::pair<std::string, std::string> > Items;

namespace paludis
{
    template <>
    struct Imp<SourceURIFinder>
    {
        const Environment * const env;
        const Repository * const repo;
        const EAPI & eapi;
        const std::string filename;
        const std::string mirrors_name;
        const GetMirrorsFunction get_mirrors_fn;
        std::string url;

        Items items;

        Imp(const Environment * const e, const Repository * const r, const EAPI & p, const std::string & f,
                const std::string & m, const GetMirrorsFunction & g, const std::string & u) :
            env(e),
            repo(r),
            eapi(p),
            filename(f),
            mirrors_name(m),
            get_mirrors_fn(g),
            url(u)
        {
        }
    };

    template <>
    struct WrappedForwardIteratorTraits<SourceURIFinder::ConstIteratorTag>
    {
        typedef Items::const_iterator UnderlyingIterator;
    };
}

SourceURIFinder::SourceURIFinder(const Environment * const e, const Repository * const repo, const EAPI & p,
        const std::string & f, const std::string & m, const GetMirrorsFunction & g, const std::string & u) :
    _imp(e, repo, p, f, m, g, u)
{
}

SourceURIFinder::~SourceURIFinder() = default;

SourceURIFinder::ConstIterator
SourceURIFinder::begin() const
{
    return ConstIterator(_imp->items.begin());
}

SourceURIFinder::ConstIterator
SourceURIFinder::end() const
{
    return ConstIterator(_imp->items.end());
}

namespace
{
    bool uri_start_with_override(const std::string & overridestr, const std::string & uri)
    {
        return (0 == uri.rfind(overridestr, 0));
    }
}

void SourceURIFinder::uri_remove_prefix(const std::string & overridestr) {
    _imp->url.erase(0, overridestr.size());
}

void SourceURIFinder::uri_remove_prefixes() {
    auto mirror_override = std::find_if(_imp->eapi.supported()->ebuild_options()->restrict_mirror_override()->begin(),
                                _imp->eapi.supported()->ebuild_options()->restrict_mirror_override()->end(),
                                std::bind(&uri_start_with_override, std::placeholders::_1, _imp->url));
    auto fetch_override = std::find_if(_imp->eapi.supported()->ebuild_options()->restrict_fetch_override()->begin(),
                        _imp->eapi.supported()->ebuild_options()->restrict_fetch_override()->end(),
                        std::bind(&uri_start_with_override, std::placeholders::_1, _imp->url));
    if (_imp->eapi.supported()->ebuild_options()->restrict_mirror_override()->end() !=
            mirror_override) {
        uri_remove_prefix(*mirror_override);
    }
    else if (_imp->eapi.supported()->ebuild_options()->restrict_fetch_override()->end() !=
                fetch_override) {
        uri_remove_prefix(*fetch_override);
    }
}

void
SourceURIFinder::visit(const URIMirrorsThenListedLabel &)
{
    Context context("When using URIMirrorsThenListedLabel:");

    uri_remove_prefixes();

    add_local_mirrors();
    add_mirrors();
    add_listed();
}

void
SourceURIFinder::visit(const URIListedThenMirrorsLabel &)
{
    Context context("When using URIListedThenMirrorsLabel:");

    uri_remove_prefixes();

    add_local_mirrors();
    add_listed();
    add_mirrors();
}

void
SourceURIFinder::visit(const URIMirrorsOnlyLabel &)
{
    Context context("When using URIMirrorsOnlyLabel:");

    uri_remove_prefixes();

    add_local_mirrors();
    add_mirrors();
}

void
SourceURIFinder::visit(const URIListedOnlyLabel &label)
{
    Context context("When using URIListedOnlyLabel:");

    /* Check for overrides. */
    if (_imp->eapi.supported()->ebuild_options()->restrict_mirror_override()->end() !=
            std::find_if(_imp->eapi.supported()->ebuild_options()->restrict_mirror_override()->begin(),
                _imp->eapi.supported()->ebuild_options()->restrict_mirror_override()->end(),
                std::bind(&uri_start_with_override, std::placeholders::_1, _imp->url))) {
        visit(URIMirrorsThenListedLabel(label.text()));
        return;
    }

    uri_remove_prefixes();

    add_local_mirrors();
    add_listed();
}

void
SourceURIFinder::visit(const URIManualOnlyLabel &label)
{
    Context context("When using URIManualOnlyLabel:");

    /* Check for overrides. */
    if (_imp->eapi.supported()->ebuild_options()->restrict_mirror_override()->end() !=
            std::find_if(_imp->eapi.supported()->ebuild_options()->restrict_mirror_override()->begin(),
                _imp->eapi.supported()->ebuild_options()->restrict_mirror_override()->end(),
                std::bind(&uri_start_with_override, std::placeholders::_1, _imp->url))) {
        /* Divert to the other, less restrictive label. */
        visit(URIMirrorsThenListedLabel(label.text()));
        return;
    }
    else if (_imp->eapi.supported()->ebuild_options()->restrict_fetch_override()->end() !=
                std::find_if(_imp->eapi.supported()->ebuild_options()->restrict_fetch_override()->begin(),
                    _imp->eapi.supported()->ebuild_options()->restrict_fetch_override()->end(),
                    std::bind(&uri_start_with_override, std::placeholders::_1, _imp->url))) {
        /* Divert to the other, less restrictive label. */
        visit(URIListedOnlyLabel(label.text()));
        return;
    }

    uri_remove_prefixes();
}

void
SourceURIFinder::visit(const URILocalMirrorsOnlyLabel &)
{
    Context context("When using URILocalMirrorsOnlyLabel:");

    uri_remove_prefixes();

    add_local_mirrors();
}

void
SourceURIFinder::add_local_mirrors()
{
    Context context("When adding local mirrors:");

    std::shared_ptr<const MirrorsSequence> mirrors(_imp->env->mirrors("*"));
    if (mirrors->empty())
        Log::get_instance()->message("e.source_uri_finder.no_mirrors", ll_debug, lc_context) << "Mirrors set is empty";

    for (const auto & mirror : *mirrors)
    {
        Log::get_instance()->message("e.source_uri_finder.adding_mirror", ll_debug, lc_context)
            << "Adding " << strip_trailing(mirror, "/") << "/" << _imp->filename;
        _imp->items.push_back(std::make_pair(strip_trailing(mirror, "/") + "/" + _imp->filename, _imp->filename));
    }
}

void
SourceURIFinder::add_mirrors()
{
    Context context("When adding repository mirrors from '" + _imp->mirrors_name + "':");

    {
        std::shared_ptr<const MirrorsSequence> mirrors(_imp->env->mirrors(_imp->mirrors_name));
        if (mirrors->empty())
            Log::get_instance()->message("e.source_uri_finder.no_mirrors", ll_debug, lc_context) << "Environment mirrors set is empty";

        for (const auto & mirror : *mirrors)
        {
            Log::get_instance()->message("e.source_uri_finder.adding_mirror", ll_debug, lc_context)
                << "Adding " << strip_trailing(mirror, "/") << "/" << _imp->filename;
            _imp->items.push_back(std::make_pair(strip_trailing(mirror, "/") + "/" + _imp->filename, _imp->filename));
        }
    }

    {
        Context local_context("When adding repository mirrors '" + _imp->mirrors_name + "':");
        std::shared_ptr<const MirrorsSequence> mirrors(_imp->get_mirrors_fn(_imp->mirrors_name));
        for (const auto & mirror : *mirrors)
        {
            Log::get_instance()->message("e.source_uri_finder.adding_mirror", ll_debug, lc_context)
                << "Adding " << strip_trailing(mirror, "/") << "/" << _imp->filename;
            _imp->items.push_back(std::make_pair(strip_trailing(mirror, "/") + "/" + _imp->filename, _imp->filename));
        }
    }
}

void
SourceURIFinder::add_listed()
{
    Context context("When adding listed locations:");

    if (0 == _imp->url.compare(0, 9, "mirror://"))
    {
        std::string mirror_name(_imp->url.substr(9));
        std::string::size_type p(mirror_name.find('/'));
        if (std::string::npos == p)
            throw ActionFailedError("Broken URI component '" + _imp->url + "'");
        std::string original_name(mirror_name.substr(p + 1));
        mirror_name.erase(p);

        {
            Context local_context("When adding from environment for listed mirror '" + mirror_name + "':");
            std::shared_ptr<const MirrorsSequence> mirrors(_imp->env->mirrors(mirror_name));
            if (mirrors->empty())
                Log::get_instance()->message("e.source_uri_finder.no_mirrors", ll_debug, lc_context) << "Mirrors set is empty";
            for (const auto & mirror : *mirrors)
            {
                Log::get_instance()->message("e.source_uri_finder.adding_mirror", ll_debug, lc_context)
                    << "Adding " << strip_trailing(mirror, "/") << "/" << original_name;
                _imp->items.push_back(std::make_pair(strip_trailing(mirror, "/") + "/" + original_name, _imp->filename));
            }
        }

        {
            Context local_context("When adding from repository for listed mirror '" + mirror_name + "':");
            std::shared_ptr<const MirrorsSequence> mirrors(_imp->get_mirrors_fn(mirror_name));
            for (const auto & mirror : *mirrors)
            {
                Log::get_instance()->message("e.source_uri_finder.adding_mirror", ll_debug, lc_context)
                    << "Adding " << strip_trailing(mirror, "/")
                    << "/" << original_name;
                _imp->items.push_back(std::make_pair(strip_trailing(mirror, "/") + "/" + original_name, _imp->filename));
            }
        }
    }
    else
    {
        Log::get_instance()->message("e.source_uri_finder.adding", ll_debug, lc_context) << "Adding " << _imp->url;
        _imp->items.push_back(std::make_pair(_imp->url, _imp->filename));
    }
}

namespace paludis
{
    template class WrappedForwardIterator<SourceURIFinder::ConstIteratorTag, const std::pair<std::string, std::string> >;
}
