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

#include "colour.hh"
#include "query.hh"
#include "licence.hh"
#include <functional>
#include <iomanip>
#include <iostream>
#include <paludis/paludis.hh>
#include <paludis/util/collection_concrete.hh>
#include <string>

/** \file
 * Handle the --query action for the main paludis program.
 */

namespace p = paludis;
using std::cout;
using std::cerr;
using std::endl;

namespace
{
    struct CannotQueryPackageSet
    {
        const std::string query;

        CannotQueryPackageSet(const std::string & q) :
            query(q)
        {
        }
    };
}

void do_one_query(
        const p::Environment * const env,
        const std::string & q,
        p::MaskReasons & mask_reasons_to_explain)
{
    p::Context local_context("When handling query '" + q + "':");

    /* we might have a dep atom, but we might just have a simple package name
     * without a category. either should work. */
    p::PackageDepAtom::Pointer atom(0);
    if (std::string::npos == q.find('/'))
    {
        if (0 != env->package_set(q))
            throw CannotQueryPackageSet(q);
        else
            atom.assign(new p::PackageDepAtom(env->package_database()->fetch_unique_qualified_package_name(
                            p::PackageNamePart(q))));
    }
    else
        atom.assign(new p::PackageDepAtom(q));

    p::PackageDatabaseEntryCollection::ConstPointer
        entries(env->package_database()->query(atom, p::is_either)),
        preferred_entries(env->package_database()->query(atom, p::is_installed_only));
    if (entries->empty())
        throw p::NoSuchPackageError(q);
    if (preferred_entries->empty())
        preferred_entries = entries;

    const p::PackageDatabaseEntry display_entry(*preferred_entries->last());

    /* match! display it. */
    cout << "* " << colour(cl_package_name, entries->begin()->name);
    if (atom->version_spec_ptr())
        cout << " (" << atom->version_operator() << *atom->version_spec_ptr() << ")";
    if (atom->slot_ptr())
        cout << " (:" << *atom->slot_ptr() << ")";
    if (atom->repository_ptr())
        cout << " (::" << *atom->repository_ptr() << ")";
    cout << endl;

    /* find all repository names. */
    p::RepositoryNameCollection::Concrete repo_names;
    p::PackageDatabaseEntryCollection::Iterator e(entries->begin()), e_end(entries->end());
    for ( ; e != e_end ; ++e)
        repo_names.append(e->repository);

    /* display versions, by repository. */
    p::RepositoryNameCollection::Iterator r(repo_names.begin()), r_end(repo_names.end());
    for ( ; r != r_end ; ++r)
    {
        cout << "    " << std::setw(22) << std::left <<
            (p::stringify(*r) + ":") << std::setw(0) << " ";

        std::string old_slot;
        for (e = entries->begin() ; e != e_end ; ++e)
            if (e->repository == *r)
            {
                p::VersionMetadata::ConstPointer metadata(env->package_database()->fetch_repository(
                            e->repository)->version_metadata(e->name,
                            e->version));
                if (CommandLine::get_instance()->a_show_slot.specified())
                {
                    /* show the slot, if we're about to move onto a new slot */
                    std::string slot_name(stringify(metadata->slot));
                    if (old_slot.empty())
                        old_slot = slot_name;
                    else if (old_slot != slot_name)
                        cout << colour(cl_slot, "{:" + old_slot + "} ");
                    old_slot = slot_name;
                }

                const p::MaskReasons masks(env->mask_reasons(*e));

                if (masks.none())
                    cout << colour(cl_visible, e->version);
                else
                {
                    std::string reasons;
                    for (p::MaskReason m(p::MaskReason(0)) ; m < p::last_mr ;
                            m = p::MaskReason(static_cast<int>(m) + 1))
                    {
                        if (! masks.test(m))
                            continue;

                        switch (m)
                        {
                            case p::mr_keyword:
                                reasons.append("K");
                                break;
                            case p::mr_user_mask:
                                reasons.append("U");
                                break;
                            case p::mr_profile_mask:
                                reasons.append("P");
                                break;
                            case p::mr_repository_mask:
                                reasons.append("R");
                                break;
                            case p::mr_eapi:
                                reasons.append("E");
                                break;
                            case p::mr_license:
                                reasons.append("L");
                                break;
                            case p::mr_by_association:
                                reasons.append("A");
                                break;
                            case p::last_mr:
                                break;
                        }
                    }
                    mask_reasons_to_explain |= masks;
                    cout << colour(cl_masked, "(" + stringify(e->version) + ")" + reasons);
                }

                if (*e == display_entry)
                    cout << "*";
                cout << " ";
            }

        /* still need to show the slot for the last item */
        if (CommandLine::get_instance()->a_show_slot.specified())
            cout << colour(cl_slot, "{:" + old_slot + "} ");

        cout << endl;
    }

    /* display metadata */
    p::VersionMetadata::ConstPointer metadata(env->package_database()->fetch_repository(
                display_entry.repository)->version_metadata(
                display_entry.name, display_entry.version));

    if (CommandLine::get_instance()->a_show_metadata.specified())
    {
        cout << "    " << std::setw(22) << std::left << "DESCRIPTION:" << std::setw(0) <<
            " " << metadata->description << endl;
        cout << "    " << std::setw(22) << std::left << "HOMEPAGE:" << std::setw(0) <<
            " " << metadata->homepage << endl;
        cout << "    " << std::setw(22) << std::left << "LICENSE:" << std::setw(0) <<
            " " << metadata->license_string << endl;

        cout << "    " << std::setw(22) << std::left << "DEPEND:" << std::setw(0) <<
            " " << metadata->deps.build_depend_string << endl;
        cout << "    " << std::setw(22) << std::left << "RDEPEND:" << std::setw(0) <<
            " " << metadata->deps.run_depend_string << endl;
        cout << "    " << std::setw(22) << std::left << "PDEPEND:" << std::setw(0) <<
            " " << metadata->deps.post_depend_string << endl;

        if (metadata->get_cran_interface())
        {
            cout << "    " << std::setw(22) << std::left << "KEYWORDS:" << std::setw(0) <<
                " " << metadata->get_cran_interface()->keywords << endl;
            cout << "    " << std::setw(22) << std::left << "PACKAGE:" << std::setw(0) <<
                " " << metadata->get_cran_interface()->package << endl;
            cout << "    " << std::setw(22) << std::left << "VERSION:" << std::setw(0) <<
                " " << metadata->get_cran_interface()->version << endl;
            cout << "    " << std::setw(22) << std::left << "IS_BUNDLE:" << std::setw(0) <<
                " " << std::boolalpha << metadata->get_cran_interface()->is_bundle << endl;
        }

        if (metadata->get_ebuild_interface())
        {
            cout << "    " << std::setw(22) << std::left << "IUSE:" << std::setw(0) <<
                " " << metadata->get_ebuild_interface()->iuse << endl;
            cout << "    " << std::setw(22) << std::left << "KEYWORDS:" << std::setw(0) <<
                " " << metadata->get_ebuild_interface()->keywords << endl;
            cout << "    " << std::setw(22) << std::left << "PROVIDE:" << std::setw(0) <<
                " " << metadata->get_ebuild_interface()->provide_string << endl;
            cout << "    " << std::setw(22) << std::left << "RESTRICT:" << std::setw(0) <<
                " " << metadata->get_ebuild_interface()->restrict_string << endl;
            cout << "    " << std::setw(22) << std::left << "SRC_URI:" << std::setw(0) <<
                 " " << metadata->get_ebuild_interface()->src_uri << endl;
        }

        if (metadata->get_ebin_interface())
        {
            cout << "    " << std::setw(22) << std::left << "BIN_URI:" << std::setw(0) <<
                 " " << metadata->get_ebin_interface()->bin_uri << endl;
            cout << "    " << std::setw(22) << std::left << "SRC_REPOSITORY:" << std::setw(0) <<
                 " " << metadata->get_ebin_interface()->src_repository << endl;
        }
    }
    else
    {
        if (! metadata->homepage.empty())
            cout << "    " << std::setw(22) << std::left << "Homepage:" << std::setw(0) <<
                " " << metadata->homepage << endl;

        if (! metadata->description.empty())
            cout << "    " << std::setw(22) << std::left << "Description:" << std::setw(0) <<
                " " << metadata->description << endl;

        if (! metadata->license_string.empty())
        {
            cout << "    " << std::setw(22) << std::left << "License:" << std::setw(0) << " ";
            LicenceDisplayer d(cout, env, &display_entry);
            metadata->license()->accept(&d);
            cout << endl;
        }

        if (CommandLine::get_instance()->a_show_deps.specified())
        {
            if (! metadata->deps.build_depend_string.empty())
            {
                p::DepAtomPrettyPrinter p_depend(12);
                metadata->deps.build_depend()->accept(&p_depend);
                cout << "    " << std::setw(22) << std::left << "Build dependencies:" << std::setw(0)
                    << endl << p_depend;
            }

            if (! metadata->deps.run_depend_string.empty())
            {
                p::DepAtomPrettyPrinter p_depend(12);
                metadata->deps.run_depend()->accept(&p_depend);
                cout << "    " << std::setw(22) << std::left << "Runtime dependencies:" << std::setw(0)
                    << endl << p_depend;
            }

            if (! metadata->deps.post_depend_string.empty())
            {
                p::DepAtomPrettyPrinter p_depend(12);
                metadata->deps.post_depend()->accept(&p_depend);
                cout << "    " << std::setw(22) << std::left << "Post dependencies:" << std::setw(0)
                    << endl << p_depend;
            }
        }

        if (metadata->get_ebuild_interface() && ! metadata->get_ebuild_interface()->
                provide_string.empty())
            cout << "    " << std::setw(22) << std::left << "Provides:" << std::setw(0) <<
                " " << metadata->get_ebuild_interface()->provide_string << endl;

        if (metadata->get_virtual_interface())
            cout << "    " << std::setw(22) << std::left << "Virtual for:" << std::setw(0) <<
                " " << metadata->get_virtual_interface()->virtual_for << endl;
    }


    /* blank line */
    cout << endl;
}

int do_query()
{
    int return_code(0);

    p::Context context("When performing query action from command line:");
    p::Environment * const env(p::DefaultEnvironment::get_instance());

    p::MaskReasons mask_reasons_to_explain;

    CommandLine::ParametersIterator q(CommandLine::get_instance()->begin_parameters()),
        q_end(CommandLine::get_instance()->end_parameters());
    for ( ; q != q_end ; ++q)
    {
        try
        {
            do_one_query(env, *q, mask_reasons_to_explain);
        }
        catch (const p::AmbiguousPackageNameError & e)
        {
            cout << endl;
            cerr << "Query error:" << endl;
            cerr << "  * " << e.backtrace("\n  * ");
            cerr << "Ambiguous package name '" << e.name() << "'. Did you mean:" << endl;
            for (p::AmbiguousPackageNameError::OptionsIterator o(e.begin_options()),
                    o_end(e.end_options()) ; o != o_end ; ++o)
                cerr << "    * " << colour(cl_package_name, *o) << endl;
            cerr << endl;
        }
        catch (const CannotQueryPackageSet & e)
        {
            cout << endl;
            cerr << "Query error:" << endl;
            cerr << "  * Target '" << e.query << "' is a set, not a package." << endl;
            cerr << endl;
        }
        catch (const p::NameError & e)
        {
            return_code |= 1;
            cout << endl;
            cerr << "Query error:" << endl;
            cerr << "  * " << e.backtrace("\n  * ") << e.message() << endl;
            cerr << endl;
        }
        catch (const p::PackageDatabaseLookupError & e)
        {
            return_code |= 1;
            cout << endl;
            cerr << "Query error:" << endl;
            cerr << "  * " << e.backtrace("\n  * ") << e.message() << endl;
            cerr << endl;
        }
    }

    if (mask_reasons_to_explain.any())
    {
        cout << colour(cl_heading, "Key to mask reasons:") << endl << endl;

        /* use for/case to get compiler warnings when new mr_ are added */
        for (p::MaskReason m(p::MaskReason(0)) ; m < p::last_mr ;
                m = p::MaskReason(static_cast<int>(m) + 1))
        {
            if (! mask_reasons_to_explain.test(m))
                continue;

            switch (m)
            {
                case p::mr_keyword:
                    cout << "* " << colour(cl_masked, "K") << ": keyword";
                    break;
                case p::mr_user_mask:
                    cout << "* " << colour(cl_masked, "U") << ": user mask";
                    break;
                case p::mr_profile_mask:
                    cout << "* " << colour(cl_masked, "P") << ": profile mask";
                    break;
                case p::mr_repository_mask:
                    cout << "* " << colour(cl_masked, "R") << ": repository mask";
                    break;
                case p::mr_eapi:
                    cout << "* " << colour(cl_masked, "E") << ": EAPI";
                    break;
                case p::mr_license:
                    cout << "* " << colour(cl_masked, "L") << ": licence";
                    break;
                case p::mr_by_association:
                    cout << "* " << colour(cl_masked, "A") << ": by association";
                    break;

                case p::last_mr:
                    break;
            }

            cout << endl;
        }

        cout << endl;
    }

    return return_code;
}

