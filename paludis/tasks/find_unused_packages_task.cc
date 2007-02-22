/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Danny van Dyk <kugelfang@gentoo.org>
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

#include "find_unused_packages_task.hh"

#include <paludis/environment.hh>
#include <paludis/repository.hh>
#include <paludis/query.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/compare.hh>
#include <paludis/util/collection_concrete.hh>

#include <set>

using namespace paludis;

FindUnusedPackagesTask::~FindUnusedPackagesTask()
{
}

std::tr1::shared_ptr<const PackageDatabaseEntryCollection>
FindUnusedPackagesTask::execute(const QualifiedPackageName & package)
{
    std::tr1::shared_ptr<PackageDatabaseEntryCollection> result(new PackageDatabaseEntryCollection::Concrete);
    std::tr1::shared_ptr<const PackageDatabaseEntryCollection> packages(_env->package_database()->query(
            query::Matches(PackageDepSpec(stringify(package) + "::" + stringify(_repo->name()))),
            qo_group_by_slot));

    SlotName old_slot("I_am_a_slot");
    std::set<KeywordName> keywords;
    for (PackageDatabaseEntryCollection::ReverseIterator p(packages->rbegin()), p_end(packages->rend()) ;
            p != p_end ; ++p)
    {
        std::tr1::shared_ptr<const VersionMetadata> metadata(_repo->version_metadata(package, p->version));
        if (! metadata->ebuild_interface)
            continue;

        if (metadata->slot != old_slot)
        {
            keywords.clear();
            old_slot = metadata->slot;
        }

        std::set<KeywordName> current_keywords;
        WhitespaceTokeniser::get_instance()->tokenise(metadata->ebuild_interface->keywords,
                create_inserter<KeywordName>(std::inserter(current_keywords, current_keywords.end())));

        bool used(false);
        for (std::set<KeywordName>::const_iterator k(current_keywords.begin()), k_end(current_keywords.end()) ;
                k != k_end ; ++k)
        {
            std::string stable_keyword(stringify(*k));
            if (stable_keyword[0] == '~')
                stable_keyword.erase(0, 1);

            if ((keywords.end() == keywords.find(*k)) && (keywords.end() == keywords.find(KeywordName(stable_keyword))))
            {
                used = true;
                break;
            }
        }

        if (! used)
            result->append(PackageDatabaseEntry(package, p->version, _repo->name()));

        keywords.insert(current_keywords.begin(), current_keywords.end());
    }

    return result;
}
