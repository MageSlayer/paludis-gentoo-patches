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

extern "C"
void
gemcutter_json_things_parse_all_gems(
        const FSPath & path,
        const ParsedOneGemCallback & parsed_one_gem)
{
    json_error_t error;
    json_t * root(json_load_file(stringify(path).c_str(), &error));

    if (! root)
        throw JSONError("Couldn't load '" + stringify(path) + "': jansson said: " + error.text);

    if (! json_is_array(root))
        throw JSONError("Root is not an array");

    for (int i(0), i_end(json_array_size(root)) ;
            i != i_end ; ++i)
    {
        std::string name, version;

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

        GemJSONInfo info(make_named_values<GemJSONInfo>(
                    n::name() = name,
                    n::version() = version
                    ));
        parsed_one_gem(info);
    }

    json_decref(root);
}

