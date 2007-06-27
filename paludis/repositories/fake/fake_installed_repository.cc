/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include "fake_installed_repository.hh"
#include <paludis/util/collection_concrete.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/portage_dep_parser.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/environment.hh>
#include <paludis/dep_spec_flattener.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>
#include <libwrapiter/libwrapiter_output_iterator.hh>

using namespace paludis;

FakeInstalledRepository::FakeInstalledRepository(const Environment * const e, const RepositoryName & our_name) :
    FakeRepositoryBase(e, our_name, RepositoryCapabilities::create()
            .installable_interface(0)
            .installed_interface(this)
            .contents_interface(0)
            .mask_interface(this)
            .sets_interface(this)
            .syncable_interface(0)
            .uninstallable_interface(0)
            .use_interface(this)
            .world_interface(0)
            .mirrors_interface(0)
            .environment_variable_interface(0)
            .provides_interface(this)
            .virtuals_interface(0)
            .config_interface(0)
            .destination_interface(this)
            .licenses_interface(0)
            .portage_interface(0)
            .pretend_interface(0)
            .make_virtuals_interface(0)
            .hook_interface(0),
            "fake_installed")
{
}

FakeInstalledRepository::~FakeInstalledRepository()
{
}

bool
FakeInstalledRepository::is_suitable_destination_for(const PackageID &) const
{
    return true;
}

tr1::shared_ptr<const FakeInstalledRepository::ProvidesSequence>
FakeInstalledRepository::provided_packages() const
{
    tr1::shared_ptr<ProvidesSequence> result(new ProvidesSequence::Concrete);

    tr1::shared_ptr<const CategoryNamePartCollection> cats(category_names());
    for (CategoryNamePartCollection::Iterator c(cats->begin()), c_end(cats->end()) ;
            c != c_end ; ++c)
    {
        tr1::shared_ptr<const QualifiedPackageNameCollection> pkgs(package_names(*c));
        for (QualifiedPackageNameCollection::Iterator p(pkgs->begin()), p_end(pkgs->end()) ;
                p != p_end ; ++p)
        {
            tr1::shared_ptr<const PackageIDSequence> vers(package_ids(*p));
            for (PackageIDSequence::Iterator v(vers->begin()), v_end(vers->end()) ;
                    v != v_end ; ++v)
            {
                if (! (*v)->provide_key())
                    continue;

                DepSpecFlattener f(environment(), *v);
                (*v)->provide_key()->value()->accept(f);

                for (DepSpecFlattener::Iterator q(f.begin()), q_end(f.end()) ; q != q_end ; ++q)
                    result->push_back(RepositoryProvidesEntry::create()
                            .virtual_name(QualifiedPackageName((*q)->text()))
                            .provided_by(*v));
            }
        }
    }

    return result;
}

FSEntry
FakeInstalledRepository::root() const
{
    return FSEntry("/");
}

bool
FakeInstalledRepository::is_default_destination() const
{
    return environment()->root() == root();
}

bool
FakeInstalledRepository::want_pre_post_phases() const
{
    return false;
}

void
FakeInstalledRepository::merge(const MergeOptions &)
{
}

time_t
FakeInstalledRepository::do_installed_time(const PackageID &) const
{
    return 0;
}

