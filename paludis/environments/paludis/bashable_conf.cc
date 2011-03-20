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

#include <paludis/environments/paludis/bashable_conf.hh>

#include <paludis/util/config_file.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/log.hh>
#include <paludis/util/system.hh>
#include <paludis/util/process.hh>
#include <paludis/util/map.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/options.hh>
#include <paludis/util/fs_path.hh>
#include <paludis/util/env_var_names.hh>

#include <functional>

using namespace paludis;
using namespace paludis::paludis_environment;

std::string defined_vars_to_kv_func(
        const std::shared_ptr<const Map<std::string, std::string> > v,
        const KeyValueConfigFile &,
        const std::string & k)
{
    Map<std::string, std::string>::ConstIterator i(v->find(k));
    if (i != v->end())
        return i->second;
    return "";
}

std::shared_ptr<LineConfigFile>
paludis::paludis_environment::make_bashable_conf(const FSPath & f, const LineConfigFileOptions & o)
{
    Context context("When making a config file out of '" + stringify(f) + "':");

    std::shared_ptr<LineConfigFile> result;

    if (is_file_with_extension(f, ".bash", { }))
    {
        std::stringstream s;

        Process process(ProcessCommand({ "bash", stringify(f) }));
        process
            .setenv("PALUDIS_LOG_LEVEL", stringify(Log::get_instance()->log_level()))
            .setenv("PALUDIS_EBUILD_DIR", getenv_with_default(env_vars::ebuild_dir, LIBEXECDIR "/paludis"))
            .prefix_stderr(f.basename() + "> ")
            .capture_stdout(s);
        int exit_status(process.run().wait());
        result = std::make_shared<LineConfigFile>(s, o);

        if (exit_status != 0)
        {
            Log::get_instance()->message("paludis_environment.bash_conf.failure", ll_warning, lc_context)
                << "Script '" << f <<"' returned non-zero exit status '" << exit_status << "'";
            result.reset();
        }
    }
    else
        result = std::make_shared<LineConfigFile>(f, o);

    return result;
}

std::shared_ptr<KeyValueConfigFile>
paludis::paludis_environment::make_bashable_kv_conf(const FSPath & f,
        const std::shared_ptr<const Map<std::string, std::string> > & predefined_variables,
        const KeyValueConfigFileOptions & o)
{
    Context context("When making a key=value config file out of '" + stringify(f) + "':");

    std::shared_ptr<KeyValueConfigFile> result;

    if (is_file_with_extension(f, ".bash", { }))
    {
        std::stringstream s;
        Process process(ProcessCommand({ "bash", stringify(f) }));
        process
            .setenv("PALUDIS_LOG_LEVEL", stringify(Log::get_instance()->log_level()))
            .setenv("PALUDIS_EBUILD_DIR", getenv_with_default(env_vars::ebuild_dir, LIBEXECDIR "/paludis"))
            .prefix_stderr(f.basename() + "> ")
            .capture_stdout(s);
        for (Map<std::string, std::string>::ConstIterator i(predefined_variables->begin()),
                end(predefined_variables->end()); i != end; ++i)
            process.setenv(i->first, i->second);
        int exit_status(process.run().wait());
        result = std::make_shared<KeyValueConfigFile>(s, o, &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation);

        if (exit_status != 0)
        {
            Log::get_instance()->message("paludis_environment.bash_conf.failure", ll_warning, lc_context)
                << "Script '" << f <<"' returned non-zero exit status '" << exit_status << "'";
            result.reset();
        }
    }
    else
    {
        result = std::make_shared<KeyValueConfigFile>(f, o,
                std::bind(&defined_vars_to_kv_func, std::cref(predefined_variables), std::placeholders::_1, std::placeholders::_2),
                &KeyValueConfigFile::no_transformation);
    }

    return result;
}

