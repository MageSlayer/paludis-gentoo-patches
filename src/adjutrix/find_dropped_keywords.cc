/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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
#include <paludis/util/compare.hh>

#include <set>
#include <map>
#include <iostream>
#include <iomanip>
#include <algorithm>

using namespace paludis;
using std::cout;
using std::cerr;
using std::endl;

namespace
{

#include "find_dropped_keywords-sr.hh"
#include "find_dropped_keywords-sr.cc"

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
    write_package(const QualifiedPackageName & package, const SlotName & slot,
            const VersionSpec & best_keyworded, const VersionSpec & best_anywhere)
    {
        static CategoryNamePart previous_category("not-on-a-boat");
        if (package.category != previous_category)
        {
            cout << std::setw(col_width_package) << (stringify(package.category) + "/") << endl;
            previous_category = package.category;
        }

        std::string p(stringify(package.package));
        if (SlotName("0") != slot)
            p += ":" + stringify(slot);
        cout << "  " << std::setw(col_width_package - 2) << p;

        if (best_keyworded != VersionSpec("0"))
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
        VersionSpec worst_keyworded("99999999");
        typedef std::map<SlotName, VersionsEntry> VersionsInSlots;
        VersionsInSlots versions_in_slots;

        VersionSpecCollection::ConstPointer versions(repo.version_specs(package));
        for (VersionSpecCollection::Iterator v(versions->begin()), v_end(versions->end()) ;
                v != v_end ; ++v)
        {
            VersionMetadata::ConstPointer metadata(repo.version_metadata(package, *v));
            if (! metadata->get_ebuild_interface())
                continue;

            std::set<KeywordName> keywords;
            WhitespaceTokeniser::get_instance()->tokenise(metadata->get_ebuild_interface()->keywords,
                    create_inserter<KeywordName>(std::inserter(keywords, keywords.end())));

            /* ensure that there's an entry for this SLOT */
            versions_in_slots.insert(std::make_pair(metadata->slot, VersionsEntry(
                            VersionsEntry::create()
                            .best_keyworded(VersionSpec("0"))
                            .best_anywhere(VersionSpec("0")))));

            if (keywords.end() != keywords.find(keyword) ||
                    keywords.end() != keywords.find(KeywordName("~" + stringify(keyword))))
            {
                is_interesting = true;
                versions_in_slots.find(metadata->slot)->second.best_keyworded =
                    std::max(versions_in_slots.find(metadata->slot)->second.best_keyworded, *v);
                worst_keyworded = std::min(worst_keyworded, *v);
            }

            if (keywords.end() != std::find_if(keywords.begin(), keywords.end(), IsStableOrUnstableKeyword()))
                versions_in_slots.find(metadata->slot)->second.best_anywhere =
                    std::max(versions_in_slots.find(metadata->slot)->second.best_anywhere, *v);
        }

        if (! is_interesting)
            return;

        /* for each slot, if there's a higher version on another arch, flag it */
        for (VersionsInSlots::const_iterator s(versions_in_slots.begin()),
                s_end(versions_in_slots.end()) ; s != s_end ; ++s)
        {
            if (s->second.best_keyworded >= s->second.best_anywhere)
                continue;

            if (s->second.best_anywhere < worst_keyworded)
                continue;

            write_package(package, s->first, s->second.best_keyworded, s->second.best_anywhere);
        }
    }
}

void do_find_dropped_keywords(const Environment & env)
{
    Context context("When performing find-dropped-keywords action:");

    KeywordName keyword(*CommandLine::get_instance()->begin_parameters());

    for (IndirectIterator<PackageDatabase::RepositoryIterator, const Repository>
            r(env.package_database()->begin_repositories()),
            r_end(env.package_database()->end_repositories()) ; r != r_end ; ++r)
    {
        write_repository_header(keyword, r->name());

        CategoryNamePartCollection::ConstPointer cat_names(r->category_names());
        for (CategoryNamePartCollection::Iterator c(cat_names->begin()), c_end(cat_names->end()) ;
                c != c_end ; ++c)
        {
            if (CommandLine::get_instance()->a_category.specified())
                if (CommandLine::get_instance()->a_category.args_end() == std::find(
                            CommandLine::get_instance()->a_category.args_begin(),
                            CommandLine::get_instance()->a_category.args_end(),
                            stringify(*c)))
                    continue;

            QualifiedPackageNameCollection::ConstPointer pkg_names(r->package_names(*c));
            for (QualifiedPackageNameCollection::Iterator p(pkg_names->begin()), p_end(pkg_names->end()) ;
                    p != p_end ; ++p)
            {
                if (CommandLine::get_instance()->a_package.specified())
                    if (CommandLine::get_instance()->a_package.args_end() == std::find(
                                CommandLine::get_instance()->a_package.args_begin(),
                                CommandLine::get_instance()->a_package.args_end(),
                                stringify(p->package)))
                        continue;

                check_one_package(env, keyword, *r, *p);
            }
        }
    }
}


