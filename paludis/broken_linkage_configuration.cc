/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 David Leverton
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

#include <paludis/broken_linkage_configuration.hh>
#include <paludis/util/realpath.hh>
#include <paludis/util/wildcard_expander.hh>
#include <paludis/util/config_file.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/join.hh>
#include <paludis/util/log.hh>
#include <paludis/util/options.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/system.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <algorithm>
#include <functional>
#include <iterator>
#include <vector>

using namespace paludis;

namespace paludis
{
    template <>
    struct Imp<BrokenLinkageConfiguration>
    {
        std::vector<std::string> ld_library_mask;
        std::vector<FSEntry> search_dirs;
        std::vector<FSEntry> search_dirs_mask;
        std::vector<FSEntry> ld_so_conf;

        void load_from_environment();
        void load_from_etc_revdep_rebuild(const FSEntry &);
        void load_from_etc_profile_env(const FSEntry &);
        void load_from_etc_ld_so_conf(const FSEntry &);
        void add_defaults();
    };

    template <>
    struct WrappedForwardIteratorTraits<BrokenLinkageConfiguration::DirsIteratorTag>
    {
        typedef std::vector<FSEntry>::const_iterator UnderlyingIterator;
    };
}

namespace
{
    struct IsGarbageFile : std::unary_function<const FSEntry &, bool>
    {
        bool operator() (const FSEntry & file)
        {
            std::string basename(file.basename());
            return '#' == basename[0] || '~' == basename[basename.length() - 1];
        }
    };

    template <typename T_>
    void
    from_colon_string(const std::function<std::string (const std::string &)> & source,
                const std::string & varname, std::vector<T_> & vec)
    {
        std::string str(source.operator() (varname)); /* silly 4.3 ICE */
        if (! str.empty())
        {
            Log::get_instance()->message("reconcilio.broken_linkage_finder.config", ll_debug, lc_context)
                << "Got " << varname << "=\"" + str << "\"";
            tokenise<delim_kind::AnyOfTag, delim_mode::DelimiterTag>(str, ":", "", std::back_inserter(vec));
        }
    }

    template <typename T_>
    void
    from_string(const std::function<std::string (const std::string &)> & source,
                const std::string & varname, std::vector<T_> & vec)
    {
        std::string str(source.operator() (varname)); /* silly 4.3 ICE */
        if (! str.empty())
        {
            Log::get_instance()->message("reconcilio.broken_linkage_finder.config", ll_debug, lc_context)
                << "Got " << varname << "=\"" << str << "\"";
            tokenise_whitespace(str, std::back_inserter(vec));
        }
    }

    inline void
    do_wildcards(std::vector<std::string> &, const FSEntry &)
    {
    }

    inline void
    do_wildcards(std::vector<FSEntry> & vec, const FSEntry & root)
    {
        std::vector<FSEntry> scratch;

        for (std::vector<FSEntry>::const_iterator it(vec.begin()), it_end(vec.end()); it_end != it; ++it)
            std::copy(WildcardExpander(stringify(*it), root), WildcardExpander(),
                      std::back_inserter(scratch));

        using std::swap;
        swap(vec, scratch);
    }

    template <typename T_>
    void
    cleanup(const std::string & varname, std::vector<T_> & vec, const FSEntry & root)
    {
        vec.erase(std::find(vec.begin(), vec.end(), T_("-*")), vec.end());

        do_wildcards(vec, root);

        std::sort(vec.begin(), vec.end());
        vec.erase(std::unique(vec.begin(), vec.end()), vec.end());

        Log::get_instance()->message("reconcilio.broken_linkage_finder.config",
                ll_debug, lc_context) << "Final " << varname << "=\"" <<
            join(vec.begin(), vec.end(),  " ") << "\"";
    }
}

BrokenLinkageConfiguration::BrokenLinkageConfiguration(const FSEntry & root) :
    Pimp<BrokenLinkageConfiguration>()
{
    Context ctx("When loading broken linkage checker configuration for '" + stringify(root) + "':");

    _imp->load_from_environment();
    _imp->load_from_etc_revdep_rebuild(root);
    _imp->load_from_etc_profile_env(root);
    _imp->load_from_etc_ld_so_conf(root);
    _imp->add_defaults();

    cleanup("LD_LIBRARY_MASK",  _imp->ld_library_mask,  root);
    cleanup("SEARCH_DIRS",      _imp->search_dirs,      root);
    cleanup("SEARCH_DIRS_MASK", _imp->search_dirs_mask, root);

    // don't need the extra cleanup here
    std::sort(_imp->ld_so_conf.begin(), _imp->ld_so_conf.end());
    _imp->ld_so_conf.erase(std::unique(_imp->ld_so_conf.begin(), _imp->ld_so_conf.end()),
                           _imp->ld_so_conf.end());
    Log::get_instance()->message("reconcilio.broken_linkage_finder.config",
            ll_debug, lc_context) << "Final ld.so.conf contents is \"" <<
        join(_imp->ld_so_conf.begin(), _imp->ld_so_conf.end(), " ") << "\"";
}

BrokenLinkageConfiguration::~BrokenLinkageConfiguration()
{
}

void
Imp<BrokenLinkageConfiguration>::load_from_environment()
{
    using namespace std::placeholders;

    Context ctx("When checking environment variables:");

    std::function<std::string (const std::string &)> fromenv(
        std::bind(getenv_with_default, _1, ""));

    from_string(fromenv, "LD_LIBRARY_MASK",  ld_library_mask);
    from_string(fromenv, "SEARCH_DIRS",      search_dirs);
    from_string(fromenv, "SEARCH_DIRS_MASK", search_dirs_mask);
}

void
Imp<BrokenLinkageConfiguration>::load_from_etc_revdep_rebuild(const FSEntry & root)
{
    using namespace std::placeholders;

    FSEntry etc_revdep_rebuild(root / "etc" / "revdep-rebuild");
    Context ctx("When reading '" + stringify(etc_revdep_rebuild) + "':");

    if (etc_revdep_rebuild.is_directory_or_symlink_to_directory())
    {
        std::vector<FSEntry> conf_files = std::vector<FSEntry>(
            DirIterator(etc_revdep_rebuild), DirIterator());
        conf_files.erase(std::remove_if(conf_files.begin(), conf_files.end(),
                                        IsGarbageFile()),
                         conf_files.end());
        std::sort(conf_files.begin(), conf_files.end());

        KeyValueConfigFileOptions opts;
        opts += kvcfo_disallow_space_around_equals;
        opts += kvcfo_disallow_space_inside_unquoted_values;

        for (std::vector<FSEntry>::iterator it(conf_files.begin()),
                 it_end(conf_files.end()); it_end != it; ++it)
        {
            Context ctx_file("When reading '" + stringify(*it) + "':");

            if (it->is_regular_file_or_symlink_to_regular_file())
            {
                KeyValueConfigFile kvs(*it, opts,
                        &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation);

                std::function<std::string (const std::string &)> fromfile(
                    std::bind(&KeyValueConfigFile::get, std::cref(kvs), _1));

                from_string(fromfile, "LD_LIBRARY_MASK",  ld_library_mask);
                from_string(fromfile, "SEARCH_DIRS",      search_dirs);
                from_string(fromfile, "SEARCH_DIRS_MASK", search_dirs_mask);
            }
            else
                Log::get_instance()->message("reconcilio.broken_linkage_finder.failure", ll_warning, lc_context)
                    << "'" << *it << "' is not a regular file";
        }
    }
    else if (etc_revdep_rebuild.exists())
        Log::get_instance()->message("reconcilio.broken_linkage_finder.etc_revdep_rebuild.not_a_directory", ll_warning, lc_context)
            << "'" << etc_revdep_rebuild << "' exists but is not a directory";
}

void
Imp<BrokenLinkageConfiguration>::load_from_etc_profile_env(const FSEntry & root)
{
    using namespace std::placeholders;

    FSEntry etc_profile_env(root / "etc" / "profile.env");
    Context ctx("When reading '" + stringify(etc_profile_env) + "':");

    if (etc_profile_env.is_regular_file_or_symlink_to_regular_file())
    {
        KeyValueConfigFileOptions opts;
        opts += kvcfo_disallow_space_around_equals;
        opts += kvcfo_disallow_space_inside_unquoted_values;
        opts += kvcfo_ignore_export;

        KeyValueConfigFile kvs(etc_profile_env, opts,
                &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation);

        std::function<std::string (const std::string &)> fromfile(
            std::bind(&KeyValueConfigFile::get, std::cref(kvs), _1));

        from_colon_string(fromfile, "PATH",     search_dirs);
        from_colon_string(fromfile, "ROOTPATH", search_dirs);
    }
    else if (etc_profile_env.exists())
        Log::get_instance()->message("reconcilio.broken_linkage_finder.etc_profile_env.not_a_file", ll_warning, lc_context)
            << "'" << etc_profile_env << "' exists but is not a regular file";
}

void
Imp<BrokenLinkageConfiguration>::load_from_etc_ld_so_conf(const FSEntry & root)
{
    FSEntry etc_ld_so_conf(root / "etc" / "ld.so.conf");
    Context ctx("When reading '" + stringify(etc_ld_so_conf) + "':");

    if (etc_ld_so_conf.is_regular_file_or_symlink_to_regular_file())
    {
        LineConfigFileOptions opts;
        opts += lcfo_disallow_continuations;

        LineConfigFile lines(etc_ld_so_conf, opts);
        if (lines.begin() != lines.end())
        {
            Log::get_instance()->message("reconcilio.broken_linkage_finder.got", ll_debug, lc_context)
                << "Got " << join(lines.begin(), lines.end(), " ");
            std::copy(lines.begin(), lines.end(), std::back_inserter(search_dirs));
            std::copy(lines.begin(), lines.end(), std::back_inserter(ld_so_conf));
        }
    }
    else if (etc_ld_so_conf.exists())
        Log::get_instance()->message("reconcilio.broken_linkage_finder.etc_ld_so_conf.not_a_file", ll_warning, lc_context)
            << "'" << etc_ld_so_conf << "' exists but is not a regular file";
}

void
Imp<BrokenLinkageConfiguration>::add_defaults()
{
    Context ctx("When adding default settings:");

    static const std::string default_ld_library_mask(
        "libodbcinst.so libodbc.so libjava.so libjvm.so");
    static const std::string default_search_dirs(
        "/bin /sbin /usr/bin /usr/sbin /lib* /usr/lib*");
    static const std::string default_search_dirs_mask(
        "/lib*/modules");
    static const std::string default_ld_so_conf("/lib /usr/lib");

    Log::get_instance()->message("reconcilio.broken_linkage_finder.config", ll_debug, lc_context)
        << "Got LD_LIBRARY_MASK=\"" << default_ld_library_mask << "\"";
    tokenise_whitespace(
            default_ld_library_mask, std::back_inserter(ld_library_mask));

    Log::get_instance()->message("reconcilio.broken_linkage_finder.config", ll_debug, lc_context)
        << "Got SEARCH_DIRS=\"" << default_search_dirs << "\"";
    tokenise_whitespace(
            default_search_dirs, std::back_inserter(search_dirs));

    Log::get_instance()->message("reconcilio.broken_linkage_finder.config", ll_debug, lc_context)
        << "Got SEARCH_DIRS_MASK=\"" << default_search_dirs_mask << "\"";
    tokenise_whitespace(
            default_search_dirs_mask, std::back_inserter(search_dirs_mask));

    Log::get_instance()->message("reconcilio.broken_linkage_finder.config", ll_debug, lc_context)
        << "Default ld.so.conf contents is \"" << default_ld_so_conf << "\"";
    tokenise_whitespace(
            default_ld_so_conf, std::back_inserter(ld_so_conf));
}

BrokenLinkageConfiguration::DirsIterator
BrokenLinkageConfiguration::begin_search_dirs() const
{
    return DirsIterator(_imp->search_dirs.begin());
}

BrokenLinkageConfiguration::DirsIterator
BrokenLinkageConfiguration::end_search_dirs() const
{
    return DirsIterator(_imp->search_dirs.end());
}

BrokenLinkageConfiguration::DirsIterator
BrokenLinkageConfiguration::begin_ld_so_conf() const
{
    return DirsIterator(_imp->ld_so_conf.begin());
}

BrokenLinkageConfiguration::DirsIterator
BrokenLinkageConfiguration::end_ld_so_conf() const
{
    return DirsIterator(_imp->ld_so_conf.end());
}

bool
BrokenLinkageConfiguration::dir_is_masked(const FSEntry & dir) const
{
    return std::binary_search(_imp->search_dirs_mask.begin(), _imp->search_dirs_mask.end(), dir);
}

bool
BrokenLinkageConfiguration::lib_is_masked(const std::string & lib) const
{
    return std::binary_search(_imp->ld_library_mask.begin(), _imp->ld_library_mask.end(), lib);
}

template class WrappedForwardIterator<BrokenLinkageConfiguration::DirsIteratorTag, const paludis::FSEntry>;

