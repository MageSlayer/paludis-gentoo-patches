/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_GEMCUTTER_JSON_COMMON_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_GEMCUTTER_JSON_COMMON_HH 1

#include <paludis/repositories/gemcutter/json_common-fwd.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/named_value.hh>
#include <list>
#include <memory>

namespace paludis
{
    namespace n
    {
        typedef Name<struct name_authors> authors;
        typedef Name<struct name_bug_tracker_uri> bug_tracker_uri;
        typedef Name<struct name_dependencies> dependencies;
        typedef Name<struct name_development_dependencies> development_dependencies;
        typedef Name<struct name_documentation_uri> documentation_uri;
        typedef Name<struct name_gem_uri> gem_uri;
        typedef Name<struct name_homepage_uri> homepage_uri;
        typedef Name<struct name_info> info;
        typedef Name<struct name_mailing_list_uri> mailing_list_uri;
        typedef Name<struct name_name> name;
        typedef Name<struct name_project_uri> project_uri;
        typedef Name<struct name_requirements> requirements;
        typedef Name<struct name_runtime_dependencies> runtime_dependencies;
        typedef Name<struct name_source_code_uri> source_code_uri;
        typedef Name<struct name_version> version;
        typedef Name<struct name_wiki_uri> wiki_uri;
    }

    namespace gemcutter_repository
    {
        struct GemJSONDependency
        {
            NamedValue<n::name, std::string> name;
            NamedValue<n::requirements, std::string> requirements;
        };

        struct GemJSONDependencies
        {
            NamedValue<n::dependencies, std::list<GemJSONDependency> > dependencies;
        };

        struct GemJSONInfo
        {
            NamedValue<n::authors, std::string> authors;
            NamedValue<n::bug_tracker_uri, std::string> bug_tracker_uri;
            NamedValue<n::development_dependencies, std::shared_ptr<GemJSONDependencies> > development_dependencies;
            NamedValue<n::documentation_uri, std::string> documentation_uri;
            NamedValue<n::gem_uri, std::string> gem_uri;
            NamedValue<n::homepage_uri, std::string> homepage_uri;
            NamedValue<n::info, std::string> info;
            NamedValue<n::mailing_list_uri, std::string> mailing_list_uri;
            NamedValue<n::name, std::string> name;
            NamedValue<n::project_uri, std::string> project_uri;
            NamedValue<n::runtime_dependencies, std::shared_ptr<GemJSONDependencies> > runtime_dependencies;
            NamedValue<n::source_code_uri, std::string> source_code_uri;
            NamedValue<n::version, std::string> version;
            NamedValue<n::wiki_uri, std::string> wiki_uri;
        };

        class PALUDIS_VISIBLE JSONError :
            public Exception
        {
            public:
                JSONError(const std::string &) throw ();
        };
    }
}

#endif
