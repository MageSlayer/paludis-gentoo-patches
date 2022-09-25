/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#include <paludis/repositories/e/e_repository.hh>
#include <paludis/repositories/e/e_repository_news.hh>
#include <paludis/repositories/e/eapi.hh>
#include <paludis/repositories/e/extra_distribution_data.hh>

#include <paludis/util/config_file.hh>
#include <paludis/util/log.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/sequence-impl.hh>
#include <paludis/util/options.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/safe_ofstream.hh>
#include <paludis/util/fs_path.hh>
#include <paludis/util/fs_stat.hh>
#include <paludis/util/fs_iterator.hh>

#include <paludis/environment.hh>
#include <paludis/distribution.hh>
#include <paludis/elike_package_dep_spec.hh>
#include <paludis/selection.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/metadata_key.hh>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <set>
#include <ostream>
#include <list>

using namespace paludis;
using namespace paludis::erepository;

typedef std::list<std::string> DisplayIfList;

namespace paludis
{
    /**
     * Imp data for ERepositoryNews.
     *
     * \ingroup grperepository
     */
    template<>
    struct Imp<ERepositoryNews>
    {
        const Environment * const environment;
        const ERepository * const e_repository;
        const erepository::ERepositoryParams params;

        const FSPath news_directory;
        const FSPath skip_file;
        const FSPath unread_file;

        Imp(const Environment * const e, const ERepository * const p,
                const erepository::ERepositoryParams & k) :
            environment(e),
            e_repository(p),
            params(k),
            news_directory(EExtraDistributionData::get_instance()->data_from_distribution(
                        *DistributionData::get_instance()->distribution_from_string(
                            e->distribution()))->news_directory()),
            skip_file(e->preferred_root_key()->parse_value() / news_directory /
                    ("news-" + stringify(e_repository->name()) + ".skip")),
            unread_file(e->preferred_root_key()->parse_value() / news_directory /
                    ("news-" + stringify(e_repository->name()) + ".unread"))
        {
        }
    };

    template <>
    struct WrappedForwardIteratorTraits<NewsFile::DisplayIfInstalledConstIteratorTag>
    {
        typedef DisplayIfList::const_iterator UnderlyingIterator;
    };

    template <>
    struct WrappedForwardIteratorTraits<NewsFile::DisplayIfKeywordConstIteratorTag>
    {
        typedef DisplayIfList::const_iterator UnderlyingIterator;
    };

    template <>
    struct WrappedForwardIteratorTraits<NewsFile::DisplayIfProfileConstIteratorTag>
    {
        typedef DisplayIfList::const_iterator UnderlyingIterator;
    };
}

ERepositoryNews::ERepositoryNews(const Environment * const e, const ERepository * const p,
        const erepository::ERepositoryParams & k) :
    _imp(e, p, k)
{
}

ERepositoryNews::~ERepositoryNews() = default;

void
ERepositoryNews::update_news() const
{
    Context context("When updating news at location '" +
            stringify(_imp->params.newsdir()) + "' for repository '" +
            stringify(_imp->e_repository->name()) + "':");

    if (! _imp->params.newsdir().stat().is_directory_or_symlink_to_directory())
        return;

    std::set<std::string> skip;

    if (_imp->skip_file.stat().is_regular_file_or_symlink_to_regular_file())
    {
        Context local_context("When handling news skip file '" + stringify(
                _imp->skip_file) + "':");
        LineConfigFile s(_imp->skip_file, { lcfo_disallow_continuations });
        std::copy(s.begin(), s.end(), std::inserter(skip, skip.end()));
    }

    for (FSIterator d(_imp->params.newsdir(), { }), d_end ; d != d_end ; ++d)
    {
        Context local_context("When handling news entry '" + stringify(*d) + "':");

        if (0 == stringify(_imp->e_repository->name()).compare(0, 2, "x-"))
        {
            Log::get_instance()->message("e.news.no_repo_name", ll_warning, lc_context)
                << "Cannot enable GLEP 42 news items for repository '"
                << _imp->e_repository->name() << "' because it is using a generated repository name";
            return;
        }

        if (! d->stat().is_directory_or_symlink_to_directory())
            continue;
        if (! (*d / (d->basename() + ".en.txt")).stat().is_regular_file_or_symlink_to_regular_file())
            continue;

        if (skip.end() != skip.find(d->basename()))
            continue;

        try
        {
            NewsFile news(*d / (d->basename() + ".en.txt"));
            bool show(true);

            const EAPI & eapi(*erepository::EAPIData::get_instance()->eapi_from_string(
                        _imp->e_repository->params().profile_eapi_when_unspecified()));

            if (news.begin_display_if_installed() != news.end_display_if_installed())
            {
                Context header_context("When checking Display-If-Installed headers:");

                bool local_show(false);
                for (NewsFile::DisplayIfInstalledConstIterator i(news.begin_display_if_installed()),
                        i_end(news.end_display_if_installed()) ; i != i_end ; ++i)
                    if (! (*_imp->environment)[selection::SomeArbitraryVersion(
                                generator::Matches(PackageDepSpec(parse_elike_package_dep_spec(*i,
                                            eapi.supported()->package_dep_spec_parse_options(),
                                            eapi.supported()->version_spec_options())), nullptr, { }) |
                                filter::InstalledAtRoot(_imp->environment->preferred_root_key()->parse_value()))]->empty())
                        local_show = true;
                show &= local_show;
            }

            if (news.begin_display_if_keyword() != news.end_display_if_keyword())
            {
                Context header_context("When checking Display-If-Keyword headers:");

                bool local_show(false);
                for (NewsFile::DisplayIfKeywordConstIterator i(news.begin_display_if_keyword()),
                        i_end(news.end_display_if_keyword()) ; i != i_end && ! local_show ; ++i)
                    if (*i == _imp->e_repository->profile_variable("ARCH"))
                        local_show = true;
                show &= local_show;
            }

            if (news.begin_display_if_profile() != news.end_display_if_profile())
            {
                Context header_context("When checking Display-If-Profile headers:");

                bool local_show(false);
                std::shared_ptr<const FSPathSequence> c(_imp->params.profiles());
                for (const auto & p : *c)
                {
                    std::string profile(strip_leading_string(strip_trailing_string(
                                strip_leading_string(stringify(p.realpath()),
                                    stringify(_imp->e_repository->location_key()->parse_value().realpath() / "profiles")), "/"), "/"));
                    Log::get_instance()->message("e.news.profile_path", ll_debug, lc_no_context) <<
                        "Profile path is '" << profile << "'";
                    for (NewsFile::DisplayIfProfileConstIterator i(news.begin_display_if_profile()),
                            i_end(news.end_display_if_profile()) ; i != i_end ; ++i)
                        if (profile == *i)
                            local_show = true;
                }
                show &= local_show;
            }

            if (show)
            {
                Context update_context("When updating skip and unread files:");

                SafeOFStream s(_imp->skip_file, O_CREAT | O_WRONLY | O_APPEND, false);
                if (! s)
                    Log::get_instance()->message("e.news.skip_file.append_failure", ll_warning, lc_no_context) <<
                        "Cannot append to news skip file '" << _imp->skip_file <<
                        "', skipping news item '" << *d << "'";

                SafeOFStream t(_imp->unread_file, O_CREAT | O_WRONLY | O_APPEND, false);
                if (! t)
                    Log::get_instance()->message("e.news.unread_file.append_failure", ll_warning, lc_no_context) <<
                        "Cannot append to unread file '" << _imp->unread_file <<
                        "', skipping news item '" << *d << "'";

                if (s && t)
                {
                    s << d->basename() << std::endl;
                    t << d->basename() << std::endl;
                }
            }
        }
        catch (const InternalError &)
        {
            throw;
        }
        catch (const Exception & e)
        {
            Log::get_instance()->message("e.news.skipping", ll_warning, lc_context) <<
                "Skipping news item '" << *d << "' because of exception '" << e.message() << "' ("
                << e.what() << ")";
        }
    }

}

namespace paludis
{
    /**
     * Imp data for NewsFile.
     *
     * \ingroup grpnewsconfigfile
     */
    template<>
    struct Imp<NewsFile>
    {
        DisplayIfList display_if_installed;
        DisplayIfList display_if_keyword;
        DisplayIfList display_if_profile;
    };
}

NewsFile::NewsFile(const FSPath & our_filename) :
    _imp()
{
    Context context("When parsing GLEP 42 news file '" + stringify(our_filename) + "':");

    bool seen_content_type(false);
    bool seen_title(false);
    bool seen_author(false);
    bool seen_news_item_format(false);
    bool seen_posted(false);
    bool seen_revision(false);

    LineConfigFile line_file(our_filename, { lcfo_disallow_continuations, lcfo_no_skip_blank_lines, lcfo_disallow_comments });
    for (const auto & line : line_file)
    {
        if (line.empty())
            break;

        std::string::size_type p(line.find(':'));
        if (std::string::npos == p)
        {
            throw NewsError(our_filename, "Bad header line '" + line + "'");
        }
        else
        {
            std::string k(strip_leading(strip_trailing(line.substr(0, p), " \t\n"), " \t\n"));
            std::string v(strip_leading(strip_trailing(line.substr(p + 1), " \t\n"), " \t\n"));
            if (k == "Display-If-Installed")
                _imp->display_if_installed.push_back(v);
            else if (k == "Display-If-Keyword")
                _imp->display_if_keyword.push_back(v);
            else if (k == "Display-If-Profile")
                _imp->display_if_profile.push_back(v);
            else if (k == "Title")
                seen_title = true;
            else if (k == "Author")
                seen_author = true;
            else if (k == "Translator")
            {
            }
            else if (k == "Content-Type")
            {
                if (seen_content_type)
                    throw NewsError(our_filename, "Multiple Content-Type headers specified");

                seen_content_type = true;
                if (v != "text/plain")
                    throw NewsError(our_filename, "Bad Content-Type line '" + line + "'");
            }
            else if (k == "News-Item-Format")
            {
                if (seen_news_item_format)
                    throw NewsError(our_filename, "Multiple News-Item-Format headers specified");

                seen_news_item_format = true;
                if (0 != v.compare(0, 2, "1.", 0, 2))
                    throw NewsError(our_filename, "Unsupported News-Item-Format '" + v + "'");
                if (v != "1.0")
                    Log::get_instance()->message("e.news.format", ll_warning, lc_context) <<
                        "News file '" << our_filename << "' uses news item format '" << v << "', but we only support "
                        "versions up to 1.0.";
            }
            else if (k == "Posted")
                seen_posted = true;
            else if (k == "Revision")
                seen_revision = true;
            else
                throw NewsError(our_filename, "Invalid header '" + line + "'");
        }
    }

    if (! seen_news_item_format)
        throw NewsError(our_filename, "No News-Item-Format header specified");
    if (! seen_author)
        throw NewsError(our_filename, "No Author header specified");
    if (! seen_content_type)
        throw NewsError(our_filename, "No Content-Type header specified");
    if (! seen_title)
        throw NewsError(our_filename, "No Title header specified");
    if (! seen_posted)
        throw NewsError(our_filename, "No Posted header specified");
    if (! seen_revision)
        throw NewsError(our_filename, "No Revision header specified");
}

NewsFile::~NewsFile() = default;

NewsFile::DisplayIfInstalledConstIterator
NewsFile::begin_display_if_installed() const
{
    return DisplayIfInstalledConstIterator(_imp->display_if_installed.begin());
}

NewsFile::DisplayIfInstalledConstIterator
NewsFile::end_display_if_installed() const
{
    return DisplayIfInstalledConstIterator(_imp->display_if_installed.end());
}

NewsFile::DisplayIfKeywordConstIterator
NewsFile::begin_display_if_keyword() const
{
    return DisplayIfKeywordConstIterator(_imp->display_if_keyword.begin());
}

NewsFile::DisplayIfKeywordConstIterator
NewsFile::end_display_if_keyword() const
{
    return DisplayIfKeywordConstIterator(_imp->display_if_keyword.end());
}

NewsFile::DisplayIfProfileConstIterator
NewsFile::begin_display_if_profile() const
{
    return DisplayIfProfileConstIterator(_imp->display_if_profile.begin());
}

NewsFile::DisplayIfProfileConstIterator
NewsFile::end_display_if_profile() const
{
    return DisplayIfProfileConstIterator(_imp->display_if_profile.end());
}

NewsError::NewsError(const FSPath & f, const std::string & m) noexcept :
    Exception("Error in news file '" + stringify(f) + "': " + m)
{
}

namespace paludis
{
    template class WrappedForwardIterator<NewsFile::DisplayIfInstalledConstIteratorTag, const std::string>;
    template class WrappedForwardIterator<NewsFile::DisplayIfKeywordConstIteratorTag, const std::string>;
    template class WrappedForwardIterator<NewsFile::DisplayIfProfileConstIteratorTag, const std::string>;
}
