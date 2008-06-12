/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008 Ciaran McCreesh
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

#include <paludis/environment_maker.hh>
#include <paludis/environments/no_config/no_config_environment.hh>
#include <paludis/util/map.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/destringify.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <list>

using namespace paludis;

extern "C"
{
    void PALUDIS_VISIBLE register_environments(EnvironmentMaker * maker);
}

namespace
{
    std::tr1::shared_ptr<Environment>
    make_no_config_environment(const std::string & s)
    {
        Context context("When making NoConfigEnvironment using spec '" + s + "':");

        std::tr1::shared_ptr<Map<std::string, std::string> > extra_params(
                make_shared_ptr(new Map<std::string, std::string>));
        FSEntry repository_dir(FSEntry::cwd());
        FSEntry write_cache("/var/empty");
        FSEntry master_repository_dir("/var/empty");
        bool disable_metadata_cache(false);
        bool accept_unstable(false);
        no_config_environment::RepositoryType repository_type(no_config_environment::ncer_auto);
        std::string extra_accept_keywords;

        std::list<std::string> tokens;
        tokenise<delim_kind::AnyOfTag, delim_mode::DelimiterTag>(s, ":", "", std::back_inserter(tokens));
        for (std::list<std::string>::const_iterator t(tokens.begin()), t_end(tokens.end()) ;
                t != t_end ; ++t)
        {
            std::string::size_type p(t->find('='));
            if (std::string::npos == p)
                repository_dir = *t;
            else
            {
                std::string key(t->substr(0, p)), value(t->substr(p + 1));

                if (key == "write-cache")
                    write_cache = value;
                else if (key == "master-repository-dir")
                    master_repository_dir = value;
                else if (key == "repository-dir")
                    repository_dir = value;
                else if (key == "disable-metadata-cache")
                    disable_metadata_cache = destringify<bool>(value);
                else if (key == "accept-unstable")
                    accept_unstable = destringify<bool>(value);
                else if (key == "repository-type")
                    repository_type = destringify<no_config_environment::RepositoryType>(value);
                else if (key == "extra-accept-keywords")
                    extra_accept_keywords = value;
                else
                    extra_params->insert(key, value);
            }
        }

        return std::tr1::shared_ptr<Environment>(new NoConfigEnvironment(
                    no_config_environment::Params::create()
                    .repository_dir(repository_dir)
                    .write_cache(write_cache)
                    .master_repository_dir(master_repository_dir)
                    .extra_params(extra_params)
                    .repository_type(repository_type)
                    .disable_metadata_cache(disable_metadata_cache)
                    .accept_unstable(accept_unstable)
                    .extra_accept_keywords(extra_accept_keywords)
                    ));
    }
}

void register_environments(EnvironmentMaker * maker)
{
    maker->register_maker("no-config", &make_no_config_environment);
}

