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

#include "keywords_graph.hh"
#include "command_line.hh"
#include <output/colour.hh>

#include <paludis/find_unused_packages_task.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/set.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/log.hh>
#include <paludis/repository.hh>
#include <paludis/package_database.hh>
#include <paludis/metadata_key.hh>
#include <paludis/package_id.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/selection.hh>
#include <paludis/choice.hh>
#include <functional>
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
    template <typename T_>
    struct CompareByStringLength :
        std::binary_function<const T_ &, const T_ &, bool>
    {
        bool
        operator() (const T_ & l, const T_ & r) const
        {
            return stringify(l).length() < stringify(r).length();
        }
    };

    std::string slot_as_string(const PackageID & id)
    {
        if (id.slot_key())
            return stringify(id.slot_key()->value());
        else
            return "(none)";
    }

    void
    write_keywords_graph(const Environment & e, const Repository & repo,
            const QualifiedPackageName & package)
    {
        using namespace std::placeholders;

        Context context("When writing keyword graph for '" + stringify(package) + "' in '"
                + stringify(repo.name()) + "':");

        cout << "Keywords for " << package << ":" << endl;
        cout << endl;

        FindUnusedPackagesTask task(&e, &repo);
        std::shared_ptr<const PackageIDSequence> packages(e[selection::AllVersionsGroupedBySlot(
                generator::InRepository(repo.name()) &
                generator::Matches(make_package_dep_spec({ }).package(package), { }))]);
        std::shared_ptr<const PackageIDSequence> unused(task.execute(package));

        if (packages->empty())
            return;

        std::set<std::string> arch_flags;
        for (PackageIDSequence::ConstIterator p(packages->begin()), p_end(packages->end()) ;
                p != p_end ; ++p)
        {
            if (! (*p)->choices_key())
                continue;

            for (Choices::ConstIterator c((*p)->choices_key()->value()->begin()), c_end((*p)->choices_key()->value()->end()) ;
                    c != c_end ; ++c)
            {
                if ((*c)->raw_name() != "ARCH")
                    continue;

                for (Choice::ConstIterator i((*c)->begin()), i_end((*c)->end()) ;
                        i != i_end ; ++i)
                    arch_flags.insert(stringify((*i)->unprefixed_name()));
            }
        }

        if (arch_flags.empty())
        {
            Log::get_instance()->message("adjutrix.keywords_graph.no_arch_flags", ll_warning, lc_context) << "Couldn't find any arch flags";
            return;
        }

        std::set<std::string> slots;
        std::transform(indirect_iterator(packages->begin()), indirect_iterator(packages->end()),
                std::inserter(slots, slots.begin()),
                std::bind(&slot_as_string, std::placeholders::_1));

        unsigned version_specs_columns_width(std::max_element(indirect_iterator(packages->begin()),
                    indirect_iterator(packages->end()),
                    std::bind(CompareByStringLength<std::string>(),
                        std::bind(std::mem_fn(&PackageID::canonical_form), _1, idcf_version),
                        std::bind(std::mem_fn(&PackageID::canonical_form), _2, idcf_version))
                    )->canonical_form(idcf_version).length() + 1);

        unsigned tallest_arch_name(std::max(stringify(*std::max_element(arch_flags.begin(),
                            arch_flags.end(), CompareByStringLength<std::string>())).length(), static_cast<std::size_t>(6)));

        unsigned longest_slot_name(stringify(*std::max_element(slots.begin(),
                        slots.end(), CompareByStringLength<std::string>())).length());

        for (unsigned h = 0 ; h < tallest_arch_name ; ++h)
        {
            cout << std::left << std::setw(version_specs_columns_width) << " " << "| ";
            for (std::set<std::string>::const_iterator a(arch_flags.begin()), a_end(arch_flags.end()) ;
                    a != a_end ; ++a)
            {
                if ((tallest_arch_name - h) > a->length())
                    cout << "  ";
                else
                    cout << a->at(a->length() - tallest_arch_name + h) << " ";
            }
            cout << "| ";
            if ((tallest_arch_name - h) <= 6)
                cout << std::string("unused").at(6 - tallest_arch_name + h);
            else
                cout << " ";

            cout << " ";
            if ((tallest_arch_name - h) <= 4)
                cout << std::string("slot").at(4 - tallest_arch_name + h);

            cout << endl;
        }

        cout << std::string(version_specs_columns_width, '-') << "+"
            << std::string(arch_flags.size() * 2 + 1, '-') << "+"
            << std::string(longest_slot_name + 3, '-') << endl;

        std::string old_slot("the first slot");
        for (IndirectIterator<PackageIDSequence::ConstIterator> p(packages->begin()), p_end(packages->end()) ;
                p != p_end ; ++p)
        {
            if (! p->keywords_key())
                continue;

            if (slot_as_string(*p) != old_slot)
                if (old_slot != "the first slot")
                    cout << std::string(version_specs_columns_width, '-') << "+"
                        << std::string(arch_flags.size() * 2 + 1, '-') << "+"
                        << std::string(longest_slot_name + 3, '-') << endl;

            cout << std::left << std::setw(version_specs_columns_width) << p->canonical_form(idcf_version) << "| ";

            std::shared_ptr<const KeywordNameSet> keywords(p->keywords_key()->value());

            for (std::set<std::string>::const_iterator a(arch_flags.begin()), a_end(arch_flags.end()) ;
                    a != a_end ; ++a)
            {
                if (keywords->end() != keywords->find(KeywordName(stringify(*a))))
                    cout << colour(cl_bold_green, "+ ");
                else if (keywords->end() != keywords->find(KeywordName("~" + stringify(*a))))
                    cout << colour(cl_bold_yellow, "~ ");
                else if (keywords->end() != keywords->find(KeywordName("-" + stringify(*a))))
                    cout << colour(cl_red, "- ");
                else if (keywords->end() != keywords->find(KeywordName("-*")))
                    cout << colour(cl_red, "* ");
                else
                    cout << "  ";
            }

            cout << "| " << (indirect_iterator(unused->end()) !=
                    std::find(indirect_iterator(unused->begin()), indirect_iterator(unused->end()), *p) ? "* " : "  ");

            if (slot_as_string(*p) != old_slot)
            {
                cout << slot_as_string(*p);
                old_slot = slot_as_string(*p);
            }

            cout << endl;
        }

        cout << endl;
    }
}

void do_keywords_graph(const NoConfigEnvironment & env)
{
    Context context("When performing keywords-graph action:");

    for (IndirectIterator<PackageDatabase::RepositoryConstIterator, const Repository>
            r(env.package_database()->begin_repositories()),
            r_end(env.package_database()->end_repositories()) ; r != r_end ; ++r)
    {
        if (r->name() == RepositoryName("virtuals"))
            continue;
        if (env.master_repository() && r->name() == env.master_repository()->name())
            continue;

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

                write_keywords_graph(env, *r, *p);
            }
        }
    }
}

