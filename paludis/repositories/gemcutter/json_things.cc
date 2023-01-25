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

#include <paludis/repositories/gemcutter/json_things.hh>
#include <paludis/repositories/gemcutter/json_common.hh>
#include <paludis/util/fs_path.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/stringify.hh>

#include <jansson.h>

using namespace paludis;
using namespace paludis::gemcutter_repository;

namespace
{
    std::shared_ptr<GemJSONDependencies> parse_deps(json_t * dependencies_id)
    {
        std::shared_ptr<GemJSONDependencies> dependencies(std::make_shared<GemJSONDependencies>(
                    make_named_values<GemJSONDependencies>(
                        n::dependencies() = std::list<GemJSONDependency>()
                        )));
        for (int n(0), n_end(json_array_size(dependencies_id)) ; n != n_end ; ++n)
        {
            json_t * dep(json_array_get(dependencies_id, n));
            if ((! dep) || (! json_is_object(dep)))
                throw JSONError("Entry dep is not an object");

            std::string dep_name;
            std::string dep_requirements;

            json_t * dep_name_id(json_object_get(dep, "name"));
            if ((! dep_name_id) || json_is_null(dep_name_id) || ! json_is_string(dep_name_id))
                throw JSONError("Entry dep name is not a string");
            dep_name = std::string(json_string_value(dep_name_id));

            json_t * dep_requirements_id(json_object_get(dep, "requirements"));
            if ((! dep_requirements_id) || json_is_null(dep_requirements_id) || ! json_is_string(dep_requirements_id))
                throw JSONError("Entry dep requirements is not a string");
            dep_requirements = std::string(json_string_value(dep_requirements_id));

            dependencies->dependencies().push_back(make_named_values<GemJSONDependency>(
                        n::name() = dep_name,
                        n::requirements() = dep_requirements
                        ));
        }

        return dependencies;
    }
}

extern "C"
void
gemcutter_json_things_parse_all_gems(
        const FSPath & path,
        const ParsedOneGemCallback & parsed_one_gem)
{
    Context context("When parsing '" + stringify(path) + "':");

    json_error_t error;
#if JANSSON_MAJOR_VERSION >= 2
    json_t * root(json_load_file(stringify(path).c_str(), 0, &error));
#else
    json_t * root(json_load_file(stringify(path).c_str(), &error));
#endif

    if (! root)
        throw JSONError("Couldn't load '" + stringify(path) + "': jansson said: " + error.text);

    if (! json_is_array(root))
        throw JSONError("Root is not an array");

    for (int i(0), i_end(json_array_size(root)) ;
            i != i_end ; ++i)
    {
        Context item_context("When parsing entry " + stringify(i) + ":");

        std::string name;
        std::string version;
        std::string authors;
        std::string bug_tracker_uri;
        std::string documentation_uri;
        std::string gem_uri;
        std::string homepage_uri;
        std::string info;
        std::string mailing_list_uri;
        std::string project_uri;
        std::string source_code_uri;
        std::string wiki_uri;

        std::shared_ptr<GemJSONDependencies> development_dependencies;
        std::shared_ptr<GemJSONDependencies> runtime_dependencies;

        json_t * entry(json_array_get(root, i));
        if ((! entry) || (! json_is_object(entry)))
            throw JSONError("Entry is not an object");

        json_t * name_id(json_object_get(entry, "name"));
        if ((! name_id) || (! json_is_string(name_id)))
            throw JSONError("Entry name is not a string");
        name = std::string(json_string_value(name_id));

        json_t * version_id(json_object_get(entry, "version"));
        if ((! version_id) || (! json_is_string(version_id)))
            throw JSONError("Entry version is not a string");
        version = std::string(json_string_value(version_id));

        Context better_item_context("When handling metadata for '" + name + "' '" + version + "':");

        json_t * authors_id(json_object_get(entry, "authors"));
        if (! json_is_null(authors_id))
        {
            if ((! authors_id) || (! json_is_string(authors_id)))
                throw JSONError("Entry authors is not a string");
            authors = std::string(json_string_value(authors_id));
        }

        json_t * bug_tracker_uri_id(json_object_get(entry, "bug_tracker_uri"));
        if (! json_is_null(bug_tracker_uri_id))
        {
            if ((! bug_tracker_uri_id) || (! json_is_string(bug_tracker_uri_id)))
                throw JSONError("Entry bug_tracker_uri is not a string");
            bug_tracker_uri = std::string(json_string_value(bug_tracker_uri_id));
        }

        json_t * documentation_uri_id(json_object_get(entry, "documentation_uri"));
        if (! json_is_null(documentation_uri_id))
        {
            if ((! documentation_uri_id) || (! json_is_string(documentation_uri_id)))
                throw JSONError("Entry documentation_uri is not a string");
            documentation_uri = std::string(json_string_value(documentation_uri_id));
        }

        json_t * gem_uri_id(json_object_get(entry, "gem_uri"));
        if (! json_is_null(gem_uri_id))
        {
            if ((! gem_uri_id) || (! json_is_string(gem_uri_id)))
                throw JSONError("Entry gem_uri is not a string");
            gem_uri = std::string(json_string_value(gem_uri_id));
        }

        json_t * homepage_uri_id(json_object_get(entry, "homepage_uri"));
        if (! json_is_null(homepage_uri_id))
        {
            if ((! homepage_uri_id) || (! json_is_string(homepage_uri_id)))
                throw JSONError("Entry homepage_uri is not a string");
            homepage_uri = std::string(json_string_value(homepage_uri_id));
        }

        json_t * info_id(json_object_get(entry, "info"));
        if (! json_is_null(info_id))
        {
            if ((! info_id) || (! json_is_string(info_id)))
                throw JSONError("Entry info is not a string");
            info = std::string(json_string_value(info_id));
        }

        json_t * mailing_list_uri_id(json_object_get(entry, "mailing_list_uri"));
        if (! json_is_null(mailing_list_uri_id))
        {
            if ((! mailing_list_uri_id) || (! json_is_string(mailing_list_uri_id)))
                throw JSONError("Entry mailing_list_uri is not a string");
            mailing_list_uri = std::string(json_string_value(mailing_list_uri_id));
        }

        json_t * project_uri_id(json_object_get(entry, "project_uri"));
        if (! json_is_null(project_uri_id))
        {
            if ((! project_uri_id) || (! json_is_string(project_uri_id)))
                throw JSONError("Entry project_uri is not a string");
            project_uri = std::string(json_string_value(project_uri_id));
        }

        json_t * source_code_uri_id(json_object_get(entry, "source_code_uri"));
        if (! json_is_null(source_code_uri_id))
        {
            if ((! source_code_uri_id) || (! json_is_string(source_code_uri_id)))
                throw JSONError("Entry source_code_uri is not a string");
            source_code_uri = std::string(json_string_value(source_code_uri_id));
        }

        json_t * wiki_uri_id(json_object_get(entry, "wiki_uri"));
        if (! json_is_null(wiki_uri_id))
        {
            if ((! wiki_uri_id) || (! json_is_string(wiki_uri_id)))
                throw JSONError("Entry wiki_uri is not a string");
            wiki_uri = std::string(json_string_value(wiki_uri_id));
        }

        json_t * dependencies_id(json_object_get(entry, "dependencies"));
        if (! json_is_null(dependencies_id))
        {
            if ((! dependencies_id) || (! json_is_object(dependencies_id)))
                throw JSONError("Entry dependencies is not an object");

            json_t * development_dependencies_id(json_object_get(dependencies_id, "development"));
            if (! json_is_null(development_dependencies_id))
            {
                if ((! development_dependencies_id) || (! json_is_array(development_dependencies_id)))
                    throw JSONError("Entry development dependencies is not an array");

                development_dependencies = parse_deps(development_dependencies_id);
            }

            json_t * runtime_dependencies_id(json_object_get(dependencies_id, "runtime"));
            if (! json_is_null(runtime_dependencies_id))
            {
                if ((! runtime_dependencies_id) || (! json_is_array(runtime_dependencies_id)))
                    throw JSONError("Entry runtime dependencies is not an array");

                runtime_dependencies = parse_deps(runtime_dependencies_id);
            }
        }

        GemJSONInfo json_info(make_named_values<GemJSONInfo>(
                    n::authors() = authors,
                    n::bug_tracker_uri() = bug_tracker_uri,
                    n::development_dependencies() = development_dependencies,
                    n::documentation_uri() = documentation_uri,
                    n::gem_uri() = gem_uri,
                    n::homepage_uri() = homepage_uri,
                    n::info() = info,
                    n::mailing_list_uri() = mailing_list_uri,
                    n::name() = name,
                    n::project_uri() = project_uri,
                    n::runtime_dependencies() = runtime_dependencies,
                    n::source_code_uri() = source_code_uri,
                    n::version() = version,
                    n::wiki_uri() = wiki_uri
                    ));
        parsed_one_gem(json_info);
    }

    json_decref(root);
}

