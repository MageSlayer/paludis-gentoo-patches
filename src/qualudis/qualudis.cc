/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <paludis/args/args.hh>
#include <paludis/paludis.hh>
#include <paludis/qa/qa.hh>
#include <paludis/util/join.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/log.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/strip.hh>

#include <cstdlib>
#include <iostream>
#include <algorithm>

#include <libebt/libebt.hh>
#include <libwrapiter/libwrapiter.hh>

#include "qualudis_command_line.hh"
#include "colour.hh"
#include "config.h"

using namespace paludis;
using std::cout;
using std::cerr;
using std::endl;

namespace
{
    struct DoHelp
    {
        const std::string message;

        DoHelp(const std::string & m = "") :
            message(m)
        {
        }
    };

    struct DoVersion
    {
    };

    static std::string current_entry_heading;

    void
    need_entry_heading()
    {
        static std::string last_displayed_entry_heading;
        if (last_displayed_entry_heading != current_entry_heading)
        {
            cout << endl;
            cout << current_entry_heading << endl;
            last_displayed_entry_heading = current_entry_heading;
        }
    }

    void
    set_entry_heading(const std::string & s)
    {
        current_entry_heading = s;
        if (! QualudisCommandLine::get_instance()->a_quiet.specified())
            need_entry_heading();
    }

    void
    display_header(const qa::CheckResult & r)
    {
        need_entry_heading();
        cout << r.item() << ": " << r.rule() << ":" << endl;
    }

    void
    display_header_once(bool * const once, const qa::CheckResult & r)
    {
        if (! *once)
        {
            display_header(r);
            *once = true;
        }
    }

    void
    display_no_errors(const qa::CheckResult & r)
    {
        if (QualudisCommandLine::get_instance()->a_verbose.specified())
            display_header(r);
    }

    void
    display_errors(const qa::CheckResult & r)
    {
        bool done_out(false);

        for (qa::CheckResult::Iterator i(r.begin()), i_end(r.end()) ; i != i_end ; ++i)
        {
            if (i->level < QualudisCommandLine::get_instance()->message_level)
                continue;

            bool show(true);
            do
            {
                switch (i->level)
                {
                    case qa::qal_info:
                        display_header_once(&done_out, r);
                        cout << "  info:         ";
                        continue;

                    case qa::qal_skip:
                        if (QualudisCommandLine::get_instance()->a_verbose.specified())
                        {
                            display_header_once(&done_out, r);
                            cout << "  skip:         ";
                        }
                        else
                            show = false;
                        continue;

                    case qa::qal_minor:
                        display_header_once(&done_out, r);
                        cout << "  minor:        ";
                        continue;

                    case qa::qal_major:
                        display_header_once(&done_out, r);
                        cout << "  major:        ";
                        continue;

                    case qa::qal_fatal:
                        display_header_once(&done_out, r);
                        cout << "  fatal:        ";
                        continue;

                    case qa::qal_maybe:
                        display_header_once(&done_out, r);
                        cout << "  maybe:        ";
                        continue;
                }

                throw InternalError(PALUDIS_HERE, "Bad mk_level");
            }
            while (false);

            if (show)
                cout << i->msg << endl;
        }
    }

    template <typename VC_>
    struct IsImportant :
        std::binary_function<bool, std::string, std::string>
    {
        bool operator() (const std::string & k1, const std::string & k2) const
        {
            return (*VC_::get_instance()->find_maker(k1))()->is_important() >
                (*VC_::get_instance()->find_maker(k2))()->is_important();
        }
    };

    template <typename VC_, typename P_>
    void do_check_kind(bool & ok, bool & fatal, const P_ & value)
    {
        std::list<std::string> checks;
        VC_::get_instance()->copy_keys(std::back_inserter(checks));
        checks.sort();
        checks.sort(IsImportant<VC_>());

        for (std::list<std::string>::const_iterator i(checks.begin()),
                i_end(checks.end()) ; i != i_end ; ++i)
        {
            if (QualudisCommandLine::get_instance()->a_qa_checks.specified())
                if (QualudisCommandLine::get_instance()->a_qa_checks.args_end() == std::find(
                            QualudisCommandLine::get_instance()->a_qa_checks.args_begin(),
                            QualudisCommandLine::get_instance()->a_qa_checks.args_end(),
                            *i))
                    continue;

            try
            {
                Context context("When performing check '" + stringify(*i) + "':");

                qa::CheckResult r((*VC_::get_instance()->find_maker(*i)())(value));

                if (r.empty())
                {
                    display_no_errors(r);
                    continue;
                }

                display_errors(r);

                do
                {
                    switch (r.most_severe_level())
                    {
                        case qa::qal_info:
                        case qa::qal_skip:
                        case qa::qal_maybe:
                            continue;

                        case qa::qal_minor:
                        case qa::qal_major:
                            ok = false;
                            continue;

                        case qa::qal_fatal:
                            ok = false;
                            fatal = true;
                            return;
                    }
                    throw InternalError(PALUDIS_HERE, "Bad most_severe_level");
                } while (0);
            }
            catch (const InternalError &)
            {
                throw;
            }
            catch (const Exception & e)
            {
                need_entry_heading();
                std::cout << "Eek! Caught Exception '" << e.message() << "' (" << e.what()
                    << ") when doing check '" << *i << "'" << endl;
                ok = false;
            }
            catch (const std::exception & e)
            {
                need_entry_heading();
                std::cout << "Eek! Caught std::exception '" << e.what()
                    << "' when doing check '" << *i << "'" << endl;
                ok = false;
            }
        }
    }

    bool
    do_check_package_dir(const FSEntry & dir, const Environment & env)
    {
        Context context("When checking package '" + stringify(dir) + "':");
        cerr << xterm_title("Checking " + dir.dirname().basename() + "/" +
                dir.basename() + " - qualudis") << std::flush;

        bool ok(true), fatal(false);

        set_entry_heading("QA checks for package directory " + stringify(dir) + ":");

        if (! fatal)
            do_check_kind<qa::PackageDirCheckMaker>(ok, fatal, dir);

        if (! fatal)
        {
            std::list<FSEntry> files((DirIterator(dir)), DirIterator());
            for (std::list<FSEntry>::iterator f(files.begin()) ; f != files.end() ; ++f)
            {
                if (f->basename() == "CVS" || '.' == f->basename().at(0))
                    continue;
                if (fatal)
                    break;

                do_check_kind<qa::FileCheckMaker>(ok, fatal, *f);
            }
        }

        if (! fatal)
        {
            std::list<FSEntry> files((DirIterator(dir)), DirIterator());
            for (std::list<FSEntry>::iterator f(files.begin()) ; f != files.end() ; ++f)
            {
                if (! IsFileWithExtension(".ebuild")(*f))
                    continue;

                qa::EbuildCheckData d(
                        QualifiedPackageName(CategoryNamePart(stringify(dir.dirname().basename())),
                                PackageNamePart(stringify(dir.basename()))),
                        VersionSpec(strip_leading_string(strip_trailing_string(f->basename(), ".ebuild"),
                                    stringify(dir.basename()) + "-")),
                        &env);
                do_check_kind<qa::EbuildCheckMaker>(ok, fatal, d);

                if (fatal)
                    break;
            }
        }

        if (! ok && (dir / "metadata.xml").is_regular_file())
        {
            cout << "metadata.xml:" << endl;
            qa::MetadataFile metadata(dir / "metadata.xml");
            if (metadata.end_herds() != metadata.begin_herds())
                cout << "  herds:        " << join(metadata.begin_herds(), metadata.end_herds(), ", ") << endl;
            if (metadata.end_maintainers() != metadata.begin_maintainers())
                for (qa::MetadataFile::MaintainersIterator i(metadata.begin_maintainers()),
                        i_end(metadata.end_maintainers()) ; i != i_end ; ++i)
                {
                    if (i->first.empty() && i->second.empty())
                        continue;

                    cout << "  maintainer:   ";
                    if (i->first.empty())
                        cout << i->second;
                    else if (i->second.empty())
                        cout << "<" << i->first << ">";
                    else
                        cout << i->second << " <" << i->first << ">";
                    cout << endl;
                }
        }

        return ok;
    }

    bool
    do_check_category_dir(const FSEntry & dir, const Environment & env)
    {
        Context context("When checking category '" + stringify(dir) + "':");

        cerr << xterm_title("Checking " + dir.basename() + " - qualudis") << std::flush;

        set_entry_heading("QA checks for category directory " + stringify(dir) + ":");

        bool ok(true);

        for (DirIterator d(dir) ; d != DirIterator() ; ++d)
        {
            if ("CVS" == d->basename())
                continue;
            else if ('.' == d->basename().at(0))
                continue;
            else if (d->is_directory())
                ok &= do_check_package_dir(*d, env);
            else if ("metadata.xml" == d->basename())
            {
                bool fatal(false);

                set_entry_heading("QA checks for category file " + stringify(*d) + ":");

                do_check_kind<qa::FileCheckMaker>(ok, fatal, *d);

                if (fatal)
                    break;
            }
        }

        return ok;
    }

    bool
    do_check_eclass_dir(const FSEntry & dir, const Environment &)
    {
        Context context("When checking eclass directory '" + stringify(dir) + "':");

        cerr << xterm_title("Checking " + dir.basename() + " - qualudis") << std::flush;

        set_entry_heading("QA checks for eclass directory " + stringify(dir) + ":");

        bool ok(true);

        for (DirIterator d(dir) ; d != DirIterator() ; ++d)
        {
            if ("CVS" == d->basename())
                continue;
            else if ('.' == d->basename().at(0))
                continue;
            else if (IsFileWithExtension(".eclass")(d->basename()))
            {
                bool fatal(false);

                set_entry_heading("QA checks for eclass file " + stringify(*d) + ":");

                do_check_kind<qa::FileCheckMaker>(ok, fatal, *d);

                if (fatal)
                    break;
            }
        }

        return ok;
    }

    bool
    do_check_top_level(const FSEntry & dir)
    {
        Context context("When checking top level '" + stringify(dir) + "':");

        set_entry_heading("QA checks for top level directory " + stringify(dir) + ":");

        qa::QAEnvironment env(dir);
        bool ok(true);

        for (DirIterator d(dir) ; d != DirIterator() ; ++d)
        {
            if (d->basename() == "CVS" || '.' == d->basename().at(0))
                continue;
            if (! d->is_directory())
                continue;
            if (d->basename() == "eclass")
                ok &= do_check_eclass_dir(*d, env);
            else if (env.package_database()->fetch_repository(
                        env.package_database()->favourite_repository())->
                    has_category_named(CategoryNamePart(d->basename())))
                ok &= do_check_category_dir(*d, env);
        }

        return ok;
    }


    bool
    do_check(const FSEntry & dir)
    {
        Context context("When checking directory '" + stringify(dir) + "':");

        if (dir.basename() == "eclass" && dir.is_directory())
        {
            qa::QAEnvironment env(dir.dirname());
            return do_check_eclass_dir(dir, env);
        }

        else if (std::count_if(DirIterator(dir), DirIterator(), IsFileWithExtension(
                        dir.basename() + "-", ".ebuild")))
        {
            qa::QAEnvironment env(dir.dirname().dirname());
            return do_check_package_dir(dir, env);
        }

        else if ((dir / "profiles").is_directory())
            return do_check_top_level(dir);

        else if ((dir.dirname() / "profiles").is_directory())
        {
            qa::QAEnvironment env(dir.dirname());
            return do_check_category_dir(dir, env);
        }

        else
            throw DoHelp("qualudis should be run inside a repository");
    }
}

int main(int argc, char *argv[])
{
    Context context("In main program:");

    try
    {
        QualudisCommandLine::get_instance()->run(argc, argv);

        if (QualudisCommandLine::get_instance()->a_help.specified())
            throw DoHelp();

        if (! QualudisCommandLine::get_instance()->a_log_level.specified())
            Log::get_instance()->set_log_level(ll_qa);
        else if (QualudisCommandLine::get_instance()->a_log_level.argument() == "debug")
            Log::get_instance()->set_log_level(ll_debug);
        else if (QualudisCommandLine::get_instance()->a_log_level.argument() == "qa")
            Log::get_instance()->set_log_level(ll_qa);
        else if (QualudisCommandLine::get_instance()->a_log_level.argument() == "warning")
            Log::get_instance()->set_log_level(ll_warning);
        else if (QualudisCommandLine::get_instance()->a_log_level.argument() == "silent")
            Log::get_instance()->set_log_level(ll_silent);
        else
            throw DoHelp("bad value for --log-level");

        if (! QualudisCommandLine::get_instance()->a_message_level.specified())
            QualudisCommandLine::get_instance()->message_level = qa::qal_info;
        else if (QualudisCommandLine::get_instance()->a_message_level.argument() == "info")
            QualudisCommandLine::get_instance()->message_level = qa::qal_info;
        else if (QualudisCommandLine::get_instance()->a_message_level.argument() == "minor")
            QualudisCommandLine::get_instance()->message_level = qa::qal_minor;
        else if (QualudisCommandLine::get_instance()->a_message_level.argument() == "major")
            QualudisCommandLine::get_instance()->message_level = qa::qal_major;
        else if (QualudisCommandLine::get_instance()->a_message_level.argument() == "fatal")
            QualudisCommandLine::get_instance()->message_level = qa::qal_fatal;
        else
            throw DoHelp("bad value for --message-level");

        if (QualudisCommandLine::get_instance()->a_version.specified())
            throw DoVersion();

        if (QualudisCommandLine::get_instance()->a_describe.specified())
        {
            if (! QualudisCommandLine::get_instance()->empty())
                throw DoHelp("describe action takes no parameters");

            cout << "Package directory checks:" << endl;
            std::list<std::string> package_dir_checks;
            qa::PackageDirCheckMaker::get_instance()->copy_keys(std::back_inserter(package_dir_checks));
            for (std::list<std::string>::const_iterator i(package_dir_checks.begin()),
                    i_end(package_dir_checks.end()) ; i != i_end ; ++i)
                cout << "  " << *i << ":" << endl << "    " <<
                    (*qa::PackageDirCheckMaker::get_instance()->find_maker(*i))()->describe() << endl;
            cout << endl;

            cout << "File checks:" << endl;
            std::list<std::string> file_checks;
            qa::FileCheckMaker::get_instance()->copy_keys(std::back_inserter(file_checks));
            for (std::list<std::string>::const_iterator i(file_checks.begin()),
                    i_end(file_checks.end()) ; i != i_end ; ++i)
                cout << "  " << *i << ":" << endl << "    " <<
                    (*qa::FileCheckMaker::get_instance()->find_maker(*i))()->describe() << endl;
            cout << endl;

            cout << "Ebuild checks:" << endl;
            std::list<std::string> ebuild_checks;
            qa::EbuildCheckMaker::get_instance()->copy_keys(std::back_inserter(ebuild_checks));
            for (std::list<std::string>::const_iterator i(ebuild_checks.begin()),
                    i_end(ebuild_checks.end()) ; i != i_end ; ++i)
                cout << "  " << *i << ":" << endl << "    " <<
                    (*qa::EbuildCheckMaker::get_instance()->find_maker(*i))()->describe() << endl;
            cout << endl;

            return EXIT_SUCCESS;
        }

        if (! QualudisCommandLine::get_instance()->empty())
        {
            QualudisCommandLine *c1 = QualudisCommandLine::get_instance();
            QualudisCommandLine::ParametersIterator argit = c1->begin_parameters(), arge = c1->end_parameters();
            for ( ; argit != arge; ++argit )
            {
                std::string arg = *argit;
                try
                {
                    do_check(FSEntry::cwd() / arg);
                }
                catch(const DirOpenError & e)
                {
                    cout << e.message() << endl;
                }
            }
            return EXIT_SUCCESS;
        }
        else
            return do_check(FSEntry::cwd()) ? EXIT_SUCCESS : EXIT_FAILURE;

        throw InternalError(__PRETTY_FUNCTION__, "no action?");
    }
    catch (const DoVersion &)
    {
        cout << "qualudis, part of " << PALUDIS_PACKAGE << " " << PALUDIS_VERSION_MAJOR << "."
            << PALUDIS_VERSION_MINOR << "." << PALUDIS_VERSION_MICRO;
        if (! std::string(PALUDIS_SUBVERSION_REVISION).empty())
            cout << " svn " << PALUDIS_SUBVERSION_REVISION;
        cout << endl << endl;
        cout << "Built by " << PALUDIS_BUILD_USER << "@" << PALUDIS_BUILD_HOST
            << " on " << PALUDIS_BUILD_DATE << endl;
        cout << "CXX:         " << PALUDIS_BUILD_CXX
#if defined(__ICC)
            << " " << __ICC
#elif defined(__VERSION__)
            << " " << __VERSION__
#endif
            << endl;
        cout << "CXXFLAGS:    " << PALUDIS_BUILD_CXXFLAGS << endl;
        cout << "LDFLAGS:     " << PALUDIS_BUILD_LDFLAGS << endl;
        cout << "SYSCONFDIR:  " << SYSCONFDIR << endl;
        cout << "LIBEXECDIR:  " << LIBEXECDIR << endl;
        cout << "BIGTEMPDIR:  " << BIGTEMPDIR << endl;
        cout << "stdlib:      "
#if defined(__GLIBCXX__)
#  define XSTRINGIFY(x) #x
#  define STRINGIFY(x) XSTRINGIFY(x)
            << "GNU libstdc++ " << STRINGIFY(__GLIBCXX__)
#endif
            << endl;

        cout << "libebt:      " << LIBEBT_VERSION_MAJOR << "." << LIBEBT_VERSION_MINOR
            << "." << LIBEBT_VERSION_MICRO << endl;
        cout << "libwrapiter: " << LIBWRAPITER_VERSION_MAJOR << "." << LIBWRAPITER_VERSION_MINOR
            << "." << LIBWRAPITER_VERSION_MICRO << endl;

        cout << endl;
        cout << "Paludis comes with ABSOLUTELY NO WARRANTY. Paludis is free software, and you" << endl;
        cout << "are welcome to redistribute it under the terms of the GNU General Public" << endl;
        cout << "License, version 2." << endl;

        return EXIT_SUCCESS;
    }
    catch (const paludis::args::ArgsError & e)
    {
        cerr << "Usage error: " << e.message() << endl;
        cerr << "Try " << argv[0] << " --help" << endl;
        return EXIT_FAILURE;
    }
    catch (const DoHelp & h)
    {
        if (h.message.empty())
        {
            cout << "Usage: " << argv[0] << " [options]" << endl;
            cout << "   or: " << argv[0] << " [package/category ..]" << endl;
            cout << endl;
            cout << *QualudisCommandLine::get_instance();
            return EXIT_SUCCESS;
        }
        else
        {
            cerr << "Usage error: " << h.message << endl;
            cerr << "Try " << argv[0] << " --help" << endl;
            return EXIT_FAILURE;
        }
    }
    catch (const Exception & e)
    {
        cout << endl;
        cerr << "Unhandled exception:" << endl
            << "  * " << e.backtrace("\n  * ")
            << e.message() << " (" << e.what() << ")" << endl;
        return EXIT_FAILURE;
    }
    catch (const std::exception & e)
    {
        cout << endl;
        cerr << "Unhandled exception:" << endl
            << "  * " << e.what() << endl;
        return EXIT_FAILURE;
    }
    catch (...)
    {
        cout << endl;
        cerr << "Unhandled exception:" << endl
            << "  * Unknown exception type. Ouch..." << endl;
        return EXIT_FAILURE;
    }
}

