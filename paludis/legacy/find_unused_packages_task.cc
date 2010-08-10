/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Danny van Dyk
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

#include <paludis/legacy/find_unused_packages_task.hh>
#include <paludis/environment.hh>
#include <paludis/repository.hh>
#include <paludis/package_database.hh>
#include <paludis/metadata_key.hh>
#include <paludis/package_id.hh>
#include <paludis/selection.hh>
#include <paludis/filter.hh>
#include <paludis/generator.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/set.hh>

#include <set>

using namespace paludis;

namespace
{
    std::string slot_as_string(const std::shared_ptr<const PackageID> & id)
    {
        if (id->slot_key())
            return stringify(id->slot_key()->value());
        else
            return "(none)";
    }
}

FindUnusedPackagesTask::~FindUnusedPackagesTask()
{
}

std::shared_ptr<const PackageIDSequence>
FindUnusedPackagesTask::execute(const QualifiedPackageName & package)
{
    std::shared_ptr<PackageIDSequence> result(std::make_shared<PackageIDSequence>());
    std::shared_ptr<const PackageIDSequence> packages((*_env)[selection::AllVersionsGroupedBySlot(
                generator::InRepository(_repo->name()) &
                generator::Package(package)
                )]);

    std::string old_slot("I am not a slot");
    std::set<KeywordName> keywords;
    for (PackageIDSequence::ReverseConstIterator p(packages->rbegin()), p_end(packages->rend()) ;
            p != p_end ; ++p)
    {
        if (! (*p)->keywords_key())
            continue;

        if (slot_as_string(*p) != old_slot)
        {
            keywords.clear();
            old_slot = slot_as_string(*p);
        }

        std::shared_ptr<const KeywordNameSet> current_keywords((*p)->keywords_key()->value());
        bool used(false);
        for (KeywordNameSet::ConstIterator k(current_keywords->begin()), k_end(current_keywords->end()) ;
                k != k_end ; ++k)
        {
            std::string stable_keyword(stringify(*k));
            if (stable_keyword[0] == '-')
                continue;

            if (stable_keyword[0] == '~')
                stable_keyword.erase(0, 1);

            if ((keywords.end() == keywords.find(*k)) && (keywords.end() == keywords.find(KeywordName(stable_keyword))))
            {
                used = true;
                break;
            }
        }

        if (! used)
            result->push_back(*p);

        keywords.insert(current_keywords->begin(), current_keywords->end());
    }

    return result;
}

