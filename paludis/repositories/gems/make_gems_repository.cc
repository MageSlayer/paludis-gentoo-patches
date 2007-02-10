/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include "make_gems_repository.hh"
#include <paludis/repositories/gems/gems_repository_exceptions.hh>

using namespace paludis;

std::tr1::shared_ptr<Repository>
paludis::make_gems_repository(Environment * const env,
        std::tr1::shared_ptr<const AssociativeCollection<std::string, std::string> > m)
{
    std::string repo_file(m->end() == m->find("repo_file") ? std::string("?") :
            m->find("repo_file")->second);

    Context context("When making gems repository from repo_file '" + repo_file + "':");

    std::string location;
    if (m->end() == m->find("location") || ((location = m->find("location")->second)).empty())
        throw GemsRepositoryConfigurationError("Key 'location' not specified or empty");

    std::string distdir;
    if (m->end() == m->find("distdir") || ((distdir = m->find("distdir")->second)).empty())
        distdir = location + "/distfiles";

    std::string yaml_uri;
    if (m->end() != m->find("yaml_uri"))
        yaml_uri = m->find("yaml_uri")->second;

    std::string buildroot;
    if (m->end() == m->find("buildroot") || ((buildroot = m->find("buildroot")->second)).empty())
        buildroot = "/var/tmp/paludis";

    return std::tr1::shared_ptr<Repository>(new GemsRepository(GemsRepositoryParams::create()
                .environment(env)
                .location(location)
                .distdir(distdir)
                .yaml_uri(yaml_uri)
                .buildroot(buildroot)));

}


