/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
 * Copyright (c) 2006 David Morgan <david.morgan@wadham.oxford.ac.uk>
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

#include <paludis/package_database_entry.hh>
#include <paludis/qa/qa_environment.hh>
#include <paludis/util/collection_concrete.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/log.hh>
#include <paludis/config_file.hh>
#include <map>

using namespace paludis;
using namespace paludis::qa;

namespace paludis
{
    namespace qa
    {
#include <paludis/qa/qa_environment-sr.hh>
#include <paludis/qa/qa_environment-sr.cc>
    }
}

QAEnvironment::QAEnvironment(const FSEntry & base, const FSEntry & write_cache) :
    NoConfigEnvironment(NoConfigEnvironmentParams::create()
            .repository_dir(base)
            .write_cache(write_cache)
            .accept_unstable(false)
            .repository_type(ncer_portage))
{
}

QAEnvironment::~QAEnvironment()
{
}

std::string
QAEnvironment::paludis_command() const
{
    return "diefunc 'qa_environment.cc' 'QAEnvironment::paludis_command()' "
        "'paludis_command called from within QAEnvironment'";
}

