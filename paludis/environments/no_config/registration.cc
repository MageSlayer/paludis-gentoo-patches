/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010 Ciaran McCreesh
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

#include <paludis/environments/no_config/no_config_environment.hh>
#include <paludis/environment_factory.hh>
#include <paludis/util/map.hh>
#include <paludis/util/set.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/destringify.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/make_named_values.hh>
#include <list>
#include "config.h"

using namespace paludis;

namespace
{
    std::tr1::shared_ptr<Environment>
    make_no_config_environment(const std::string & s)
    {
        Context context("When making NoConfigEnvironment using spec '" + s + "':");

        std::tr1::shared_ptr<Map<std::string, std::string> > extra_params(
                make_shared_ptr(new Map<std::string, std::string>));
        FSEntry repository_dir(FSEntry::cwd());
        std::tr1::shared_ptr<FSEntrySequence> extra_repository_dirs(new FSEntrySequence);
        FSEntry write_cache("/var/empty");
        std::string profile;
        std::string master_repository_name;
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
                else if (key == "master-repository-name")
                    master_repository_name = value;
                else if (key == "master-repository-dir")
                    throw ConfigurationError("NoConfigEnvironment key master-repository-dir is no longer "
                            "supported, use master-repository-name and extra-repository-dir");
                else if (key == "extra-repository-dir")
                    extra_repository_dirs->push_back(value);
                else if (key == "profile")
                    profile = value;
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
                    make_named_values<no_config_environment::Params>(
                        n::accept_unstable() = accept_unstable,
                        n::disable_metadata_cache() = disable_metadata_cache,
                        n::extra_accept_keywords() = extra_accept_keywords,
                        n::extra_params() = extra_params,
                        n::extra_repository_dirs() = extra_repository_dirs,
                        n::master_repository_name() = master_repository_name,
                        n::profiles_if_not_auto() = profile,
                        n::repository_dir() = repository_dir,
                        n::repository_type() = repository_type,
                        n::write_cache() = write_cache
                    )));
    }
}

namespace paludis
{
    namespace environment_groups
    {
        ENVIRONMENT_GROUPS_DECLS;
    }

    template <>
    void register_environment<environment_groups::no_config>(const environment_groups::no_config * const,
            EnvironmentFactory * const factory)
    {
        std::tr1::shared_ptr<Set<std::string> > no_config_formats(new Set<std::string>);
        no_config_formats->insert("no_config");
        no_config_formats->insert("no-config");
        factory->add_environment_format(no_config_formats, &make_no_config_environment);
    }
}

