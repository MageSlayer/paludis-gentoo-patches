/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010 Ciaran McCreesh
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

#include "find_dropped_keywords.hh"
#include "command_line.hh"

#include <paludis/util/tokeniser.hh>
#include <paludis/util/iterator_funcs.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/set.hh>
#include <paludis/util/named_value.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/version_spec.hh>
#include <paludis/repository.hh>
#include <paludis/package_database.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>

#include <set>
#include <map>
#include <iostream>
#include <iomanip>
#include <algorithm>

using namespace paludis;
using std::cout;
using std::cerr;
using std::endl;

namespace paludis
{
    namespace n
    {
        typedef Name<struct name_best_anywhere> best_anywhere;
        typedef Name<struct name_best_keyworded> best_keyworded;
    }
}

namespace
{
    std::string slot_as_string(const std::shared_ptr<const PackageID> & id)
    {
        if (id->slot_key())
            return stringify(id->slot_key()->value());
        else
            return "(none)";
    }

    struct VersionsEntry
    {
        NamedValue<n::best_anywhere, VersionSpec> best_anywhere;
        NamedValue<n::best_keyworded, VersionSpec> best_keyworded;
    };

    static const int col_width_package      = 30;
    static const int col_width_best_keyworded  = 20;
    static const int col_width_best_anywhere = 20;

    void
    write_repository_header(const KeywordName & keyword, const RepositoryName & repo)
    {
        std::string s("Dropped keywords for '" + stringify(repo) + "' on '"
                + stringify(keyword) + "'");
        cout << std::string(s.length(), '=') << endl;
        cout << s << endl;
        cout << std::string(s.length(), '=') << endl;
        cout << endl;

        cout << std::left
            << std::setw(col_width_package) << "category/package (:slot)"
            << std::setw(col_width_best_keyworded) << "best keyworded"
            << std::setw(col_width_best_anywhere) << "best anywhere"
            << endl;

        cout
            << std::string(col_width_package - 1, '-') << " "
            << std::string(col_width_best_keyworded - 1, '-') << " "
            << std::string(col_width_best_anywhere - 1, '-') << " "
            << endl;
    }

    struct IsStableOrUnstableKeyword
    {
        bool operator() (const KeywordName & k) const
        {
            return stringify(k).at(0) != '-';
        }
    };

    void
    write_package(const QualifiedPackageName & package, const std::string & slot,
            const VersionSpec & best_keyworded, const VersionSpec & best_anywhere)
    {
        static CategoryNamePart previous_category("not-on-a-boat");
        if (package.category() != previous_category)
        {
            cout << std::setw(col_width_package) << (stringify(package.category()) + "/") << endl;
            previous_category = package.category();
        }

        std::string p(stringify(package.package()));
        if ("0" != slot)
            p += ":" + stringify(slot);
        cout << "  " << std::setw(col_width_package - 2) << p;

        if (best_keyworded != VersionSpec("0", { }))
            cout << std::setw(col_width_best_keyworded) << best_keyworded;
        else
            cout << std::setw(col_width_best_keyworded) << " ";
        cout << std::setw(col_width_best_anywhere) << best_anywhere;
        cout << endl;
    }

    void
    check_one_package(const Environment &, const KeywordName & keyword,
            const Repository & repo, const QualifiedPackageName & package)
    {
        /* determine whether we have any interesting versions, and pick out
         * slots where we do. for slots, we map slot to a pair (best stable
         * version for us, best stable version for anyone). */

        bool is_interesting(false);
        VersionSpec worst_keyworded("99999999", { });
        typedef std::map<std::string, VersionsEntry> VersionsInSlots;
        VersionsInSlots versions_in_slots;

        std::shared_ptr<const PackageIDSequence> versions(repo.package_ids(package));
        for (PackageIDSequence::ConstIterator v(versions->begin()), v_end(versions->end()) ;
                v != v_end ; ++v)
        {
            if (! (*v)->keywords_key())
                continue;

            /* ensure that there's an entry for this SLOT */
            versions_in_slots.insert(std::make_pair(slot_as_string(*v), VersionsEntry(
                            make_named_values<VersionsEntry>(
                                n::best_anywhere() = VersionSpec("0", { }),
                                n::best_keyworded() = VersionSpec("0", { })
                                ))));

            if ((*v)->keywords_key()->value()->end() != (*v)->keywords_key()->value()->find(keyword) ||
                    (*v)->keywords_key()->value()->end() != (*v)->keywords_key()->value()->find(KeywordName("~" + stringify(keyword))))
            {
                is_interesting = true;
                versions_in_slots.find(slot_as_string(*v))->second.best_keyworded() =
                    std::max(versions_in_slots.find(slot_as_string(*v))->second.best_keyworded(), (*v)->version());
                worst_keyworded = std::min(worst_keyworded, (*v)->version());
            }

            if ((*v)->keywords_key()->value()->end() != std::find_if((*v)->keywords_key()->value()->begin(),
                        (*v)->keywords_key()->value()->end(), IsStableOrUnstableKeyword()))
                versions_in_slots.find(slot_as_string(*v))->second.best_anywhere() =
                    std::max(versions_in_slots.find(slot_as_string(*v))->second.best_anywhere(), (*v)->version());
        }

        if (! is_interesting)
            return;

        /* for each slot, if there's a higher version on another arch, flag it */
        for (VersionsInSlots::const_iterator s(versions_in_slots.begin()),
                s_end(versions_in_slots.end()) ; s != s_end ; ++s)
        {
            if (s->second.best_keyworded() >= s->second.best_anywhere())
                continue;

            if (s->second.best_anywhere() < worst_keyworded)
                continue;

            write_package(package, s->first, s->second.best_keyworded(), s->second.best_anywhere());
        }
    }
}

void do_find_dropped_keywords(const NoConfigEnvironment & env)
{
    Context context("When performing find-dropped-keywords action:");

    KeywordName keyword(*CommandLine::get_instance()->begin_parameters());

    for (IndirectIterator<PackageDatabase::RepositoryConstIterator, const Repository>
            r(env.package_database()->begin_repositories()),
            r_end(env.package_database()->end_repositories()) ; r != r_end ; ++r)
    {
        if (r->name() == RepositoryName("virtuals"))
            continue;
        if (env.master_repository() && r->name() == env.master_repository()->name())
            continue;

        write_repository_header(keyword, r->name());

        std::shared_ptr<const CategoryNamePartSet> cat_names(r->category_names());
        for (CategoryNamePartSet::ConstIterator c(cat_names->begin()), c_end(cat_names->end()) ;
                c != c_end ; ++c)
        {
            if (CommandLine::get_instance()->a_category.specified())
                if (CommandLine::get_instance()->a_category.end_args() == std::find(
                            CommandLine::get_instance()->a_category.begin_args(),
                            CommandLine::get_instance()->a_category.end_args(),
                            stringify(*c)))
                    continue;

            std::shared_ptr<const QualifiedPackageNameSet> pkg_names(r->package_names(*c));
            for (QualifiedPackageNameSet::ConstIterator p(pkg_names->begin()), p_end(pkg_names->end()) ;
                    p != p_end ; ++p)
            {
                if (CommandLine::get_instance()->a_package.specified())
                    if (CommandLine::get_instance()->a_package.end_args() == std::find(
                                CommandLine::get_instance()->a_package.begin_args(),
                                CommandLine::get_instance()->a_package.end_args(),
                                stringify(p->package())))
                        continue;

                check_one_package(env, keyword, *r, *p);
            }
        }
    }
}


