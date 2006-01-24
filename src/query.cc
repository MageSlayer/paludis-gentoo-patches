/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@gentoo.org>
 *
 * This file is part of the Paludis package manager. Paludis is free software;
 * you can redistribute it and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
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

#include "src/query.hh"
#include "src/colour.hh"
#include <paludis/paludis.hh>
#include <iostream>
#include <iomanip>
#include <string>
#include <functional>

/** \file
 * Handle the --query action for the main paludis program.
 */

namespace p = paludis;

void do_one_query(
        const p::Environment * const env,
        const std::string & q)
{
    p::Context local_context("When handling query '" + q + "':");

    /* we might have a dep atom, but we might just have a simple package name
     * without a category. either should work. */
    p::PackageDepAtom::Pointer atom(std::string::npos == q.find('/') ?
            new p::PackageDepAtom(env->package_database()->fetch_unique_qualified_package_name(
                    p::PackageNamePart(q))) :
            new p::PackageDepAtom(q));

    p::PackageDatabaseEntryCollection::ConstPointer entries(env->package_database()->query(atom));
    if (entries->empty())
        throw p::NoSuchPackageError(q);

    /* match! display it. */
    std::cout << "* " << colour(cl_package_name, entries->begin()->get<p::pde_name>());
    if (atom->version_spec_ptr())
        std::cout << " (" << atom->version_operator() << *atom->version_spec_ptr() << ")";
    std::cout << std::endl;

    /* find all repository names. */
    p::RepositoryNameCollection repo_names;
    p::PackageDatabaseEntryCollection::Iterator e(entries->begin()), e_end(entries->end());
    for ( ; e != e_end ; ++e)
        repo_names.append(e->get<p::pde_repository>());

    /* display versions, by repository. */
    p::RepositoryNameCollection::Iterator r(repo_names.begin()), r_end(repo_names.end());
    for ( ; r != r_end ; ++r)
    {
        std::cout << "    " << std::setw(22) << std::left <<
            (p::stringify(*r) + ":") << std::setw(0) << " ";

        std::string old_slot;
        for (e = entries->begin() ; e != e_end ; ++e)
            if (e->get<p::pde_repository>() == *r)
            {
                p::VersionMetadata::ConstPointer metadata(env->package_database()->fetch_metadata(*e));
                if (CommandLine::get_instance()->a_show_slot.specified())
                {
                    /* show the slot, if we're about to move onto a new slot */
                    std::string slot_name(metadata->get(p::vmk_slot));
                    if (old_slot.empty())
                        old_slot = slot_name;
                    else if (old_slot != slot_name)
                        std::cout << colour(cl_slot, "{:" + old_slot + "} ");
                    old_slot = slot_name;
                }

                const p::MaskReasons masks(env->mask_reasons(*e));

                if (masks.none())
                    std::cout << colour(cl_visible, e->get<p::pde_version>());
                else
                {
                    std::string reasons;
                    if (masks.test(p::mr_keyword))
                        reasons.append("K");
                    if (masks.test(p::mr_user_mask))
                        reasons.append("U");
                    if (masks.test(p::mr_profile_mask))
                        reasons.append("P");
                    if (masks.test(p::mr_repository_mask))
                        reasons.append("R");
                    if (masks.test(p::mr_eapi))
                        reasons.append("E");
                    std::cout << colour(cl_masked, "(" + stringify(e->get<p::pde_version>()) + ")" + reasons);
                }
                /// \todo ^^ text description of masks

                if (e == entries->last())
                    std::cout << "*";
                std::cout << " ";
            }

        /* still need to show the slot for the last item */
        if (CommandLine::get_instance()->a_show_slot.specified())
            std::cout << colour(cl_slot, "{:" + old_slot + "} ");

        std::cout << std::endl;
    }

    /* display metadata */
    p::VersionMetadata::ConstPointer metadata(env->package_database()->fetch_metadata(*entries->last()));

    if (! metadata->get(p::vmk_homepage).empty())
        std::cout << "    " << std::setw(22) << std::left << "Homepage:" << std::setw(0) <<
            " " << metadata->get(p::vmk_homepage) << std::endl;
    if (! metadata->get(p::vmk_description).empty())
        std::cout << "    " << std::setw(22) << std::left << "Description:" << std::setw(0) <<
            " " << metadata->get(p::vmk_description) << std::endl;
    if (CommandLine::get_instance()->a_show_license.specified())
        if (! metadata->get(p::vmk_license).empty())
            std::cout << "    " << std::setw(22) << std::left << "License:" << std::setw(0) <<
                " " << metadata->get(p::vmk_license) << std::endl;
    if (! metadata->get(p::vmk_virtual).empty())
        std::cout << "    " << std::setw(22) << std::left << "Virtual for:" << std::setw(0) <<
            " " << metadata->get(p::vmk_virtual) << std::endl;


    /* blank line */
    std::cout << std::endl;
}

int do_query()
{
    int return_code(0);

    p::Context context("When performing query action from command line:");
    p::Environment * const env(p::DefaultEnvironment::get_instance());

    CommandLine::ParametersIterator q(CommandLine::get_instance()->begin_parameters()),
        q_end(CommandLine::get_instance()->end_parameters());
    for ( ; q != q_end ; ++q)
    {
        try
        {
            do_one_query(env, *q);
        }
        catch (const p::NameError & e)
        {
            return_code |= 1;
            std::cout << std::endl;
            std::cerr << "Query error:" << std::endl;
            std::cerr << "  * " << e.backtrace("\n  * ") << e.message() << std::endl;
            std::cerr << std::endl;
        }
        catch (const p::PackageDatabaseLookupError & e)
        {
            return_code |= 1;
            std::cout << std::endl;
            std::cerr << "Query error:" << std::endl;
            std::cerr << "  * " << e.backtrace("\n  * ") << e.message() << std::endl;
            std::cerr << std::endl;
        }
    }

    return return_code;
}

