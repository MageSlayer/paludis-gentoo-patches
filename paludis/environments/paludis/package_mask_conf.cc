/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include "package_mask_conf.hh"
#include <paludis/environment.hh>
#include <paludis/package_database_entry.hh>
#include <paludis/hashed_containers.hh>
#include <paludis/name.hh>
#include <paludis/dep_spec.hh>
#include <paludis/match_package.hh>
#include <paludis/environments/paludis/paludis_environment.hh>
#include <paludis/environments/paludis/bashable_conf.hh>
#include <paludis/util/log.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <list>
#include <algorithm>
#include <paludis/util/tr1_functional.hh>

using namespace paludis;

namespace paludis
{
    template<>
    struct Implementation<PackageMaskConf>
    {
        const PaludisEnvironment * const env;
        std::list<tr1::shared_ptr<const PackageDepSpec> > masks;

        Implementation(const PaludisEnvironment * const e) :
            env(e)
        {
        }
    };
}

PackageMaskConf::PackageMaskConf(const PaludisEnvironment * const e) :
    PrivateImplementationPattern<PackageMaskConf>(new Implementation<PackageMaskConf>(e))
{
}

PackageMaskConf::~PackageMaskConf()
{
}

void
PackageMaskConf::add(const FSEntry & filename)
{
    Context context("When adding source '" + stringify(filename) + "' as a package mask or unmask file:");

    tr1::shared_ptr<LineConfigFile> f(make_bashable_conf(filename));
    if (! f)
        return;

    for (LineConfigFile::Iterator line(f->begin()), line_end(f->end()) ;
            line != line_end ; ++line)
        _imp->masks.push_back(tr1::shared_ptr<PackageDepSpec>(new PackageDepSpec(*line, pds_pm_unspecific)));
}

bool
PackageMaskConf::query(const PackageDatabaseEntry & e) const
{
    using namespace tr1::placeholders;
    return indirect_iterator(_imp->masks.end()) != std::find_if(
            indirect_iterator(_imp->masks.begin()),
            indirect_iterator(_imp->masks.end()),
            tr1::bind(&match_package, tr1::ref(*_imp->env), _1, e));
}

