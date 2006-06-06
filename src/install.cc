/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#include "src/colour.hh"
#include "src/install.hh"
#include "src/licence.hh"
#include <iostream>
#include <paludis/paludis.hh>
#include <paludis/util/iterator.hh>
#include <paludis/util/tokeniser.hh>

/** \file
 * Handle the --install action for the main paludis program.
 */

namespace p = paludis;

using std::cerr;
using std::cout;
using std::endl;

namespace
{
    struct TagDisplayer :
        p::DepTagVisitorTypes::ConstVisitor
    {
        void visit(const p::GLSADepTag * const tag)
        {
            cout << "* " << colour(cl_yellow, tag->short_text()) << ": "
                << tag->glsa_title() << endl;
        }

        void visit(const p::GeneralSetDepTag * const tag)
        {
            cout << "* " << colour(cl_yellow, tag->short_text());
            if (tag->short_text() == "world")
                cout << ":      " << "Packages that have been explicitly installed";
            else if (tag->short_text() == "everything")
                cout << ": " << "All installed packages";
            else if (tag->short_text() == "system")
                cout << ":     " << "Packages that are part of the base system";
            cout << endl;
        }
    };
}

int
do_install()
{
    int return_code(0);

    p::Context context("When performing install action from command line:");
    p::Environment * const env(p::DefaultEnvironment::get_instance());

    cout << colour(cl_heading, "These packages will be installed:") << endl << endl;

    p::CompositeDepAtom::Pointer targets(new p::AllDepAtom);

    p::DepList dep_list(env);

    dep_list.set_drop_self_circular(CommandLine::get_instance()->a_dl_drop_self_circular.specified());
    dep_list.set_drop_circular(CommandLine::get_instance()->a_dl_drop_circular.specified());
    dep_list.set_drop_all(CommandLine::get_instance()->a_dl_drop_all.specified());
    dep_list.set_ignore_installed(CommandLine::get_instance()->a_dl_ignore_installed.specified());
    dep_list.set_recursive_deps(! CommandLine::get_instance()->a_dl_no_recursive_deps.specified());
    dep_list.set_max_stack_depth(CommandLine::get_instance()->a_dl_max_stack_depth.argument());

    if (CommandLine::get_instance()->a_dl_rdepend_post.argument() == "always")
        dep_list.set_rdepend_post(p::dlro_always);
    else if (CommandLine::get_instance()->a_dl_rdepend_post.argument() == "never")
        dep_list.set_rdepend_post(p::dlro_never);
    else
        dep_list.set_rdepend_post(p::dlro_as_needed);

    p::InstallOptions opts(false, false);
    if (CommandLine::get_instance()->a_no_config_protection.specified())
        opts.set<p::io_noconfigprotect>(true);
    if (CommandLine::get_instance()->a_fetch.specified())
        opts.set<p::io_fetchonly>(true);

    try
    {
        CommandLine::ParametersIterator q(CommandLine::get_instance()->begin_parameters()),
            q_end(CommandLine::get_instance()->end_parameters());

        bool had_set_targets(false), had_pkg_targets(false);
        for ( ; q != q_end ; ++q)
        {
            p::DepAtom::Pointer s(0);
            if (s = ((env->package_set(*q))))
            {
                if (had_set_targets)
                    throw DoHelp("You should not specify more than one set target.");

                had_set_targets = true;
                targets->add_child(s);
            }
            else
            {
                had_pkg_targets = true;

                /* we might have a dep atom, but we might just have a simple package name
                 * without a category. either should work. also allow full atoms, to make
                 * it easy to test things like '|| ( foo/bar foo/baz )'. */
                if (std::string::npos != q->find('/'))
                    targets->add_child(p::PortageDepParser::parse(*q));
                else
                    targets->add_child(p::DepAtom::Pointer(new p::PackageDepAtom(
                                    env->package_database()->fetch_unique_qualified_package_name(
                                        p::PackageNamePart(*q)))));
            }
        }

        if (had_set_targets && had_pkg_targets)
            throw DoHelp("You should not specify set and package targets at the same time.");

        if (had_set_targets)
            dep_list.set_reinstall(false);
        else if ((! CommandLine::get_instance()->a_pretend.specified()) &&
                (! opts.get<p::io_fetchonly>()))
            if (! CommandLine::get_instance()->a_preserve_world.specified())
                env->add_appropriate_to_world(targets);
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
        return 1;
    }

    try
    {
        dep_list.add(targets);

        if (CommandLine::get_instance()->a_pretend.specified())
            env->perform_hook(p::Hook("install_pretend_pre")("TARGETS", p::join(
                            CommandLine::get_instance()->begin_parameters(),
                            CommandLine::get_instance()->end_parameters(), " ")));

        std::set<p::DepTag::ConstPointer, p::DepTag::Comparator> all_tags;

        for (p::DepList::Iterator dep(dep_list.begin()), dep_end(dep_list.end()) ;
                dep != dep_end ; ++dep)
        {
            p::Context loop_context("When displaying DepList entry '" + stringify(*dep) + "':");

            /* display name */
            cout << "* " << colour(cl_package_name, dep->get<p::dle_name>());

            /* display version, unless it's 0 and our category is "virtual" */
            if ((p::VersionSpec("0") != dep->get<p::dle_version>()) ||
                    p::CategoryNamePart("virtual") != dep->get<p::dle_name>().get<p::qpn_category>())
                cout << "-" << dep->get<p::dle_version>();

            /* display repository, unless it's our main repository */
            if (env->package_database()->favourite_repository() != dep->get<p::dle_repository>())
                cout << "::" << dep->get<p::dle_repository>();

            /* display slot name, unless it's 0 */
            if (p::SlotName("0") != dep->get<p::dle_metadata>()->get<p::vm_slot>())
                cout << colour(cl_slot, " {:" + p::stringify(
                            dep->get<p::dle_metadata>()->get<p::vm_slot>()) + "}");

            /* indicate [U], [S] or [N]. display existing version, if we're
             * already installed */
            p::PackageDatabaseEntryCollection::Pointer existing(env->package_database()->
                    query(p::PackageDepAtom::Pointer(new p::PackageDepAtom(p::stringify(
                                    dep->get<p::dle_name>()))), p::is_installed_only));

            if (existing->empty())
                cout << colour(cl_updatemode, " [N]");
            else
            {
                existing = env->package_database()->query(p::PackageDepAtom::Pointer(
                            new p::PackageDepAtom(p::stringify(dep->get<p::dle_name>()) + ":" +
                                stringify(dep->get<p::dle_metadata>()->get<p::vm_slot>()))),
                        p::is_installed_only);
                if (existing->empty())
                    cout << colour(cl_updatemode, " [S]");
                else if (existing->last()->get<p::pde_version>() < dep->get<p::dle_version>())
                    cout << colour(cl_updatemode, " [U " + p::stringify(
                                existing->last()->get<p::pde_version>()) + "]");
                else if (existing->last()->get<p::pde_version>() > dep->get<p::dle_version>())
                    cout << colour(cl_updatemode, " [D " + p::stringify(
                                existing->last()->get<p::pde_version>()) + "]");
                else
                    cout << colour(cl_updatemode, " [R]");
            }

            /* fetch db entry */
            p::PackageDatabaseEntry p(p::PackageDatabaseEntry(dep->get<p::dle_name>(),
                        dep->get<p::dle_version>(), dep->get<p::dle_repository>()));

            /* display USE flags */
            if (dep->get<p::dle_metadata>()->get_ebuild_interface())
            {
                const p::Repository::UseInterface * const use_interface(
                        env->package_database()->fetch_repository(dep->get<p::dle_repository>())->
                        get_interface<p::repo_use>());
                std::set<p::UseFlagName> iuse;
                p::WhitespaceTokeniser::get_instance()->tokenise(
                        dep->get<p::dle_metadata>()->get_ebuild_interface()->get<p::evm_iuse>(),
                        p::create_inserter<p::UseFlagName>(std::inserter(iuse, iuse.end())));
                for (std::set<p::UseFlagName>::const_iterator i(iuse.begin()), i_end(iuse.end()) ;
                        i != i_end ; ++i)
                {
                    if (env->query_use(*i, &p))
                    {
                        if (use_interface && use_interface->query_use_force(*i, &p))
                            cout << " " << colour(cl_flag_on, "(" + p::stringify(*i) + ")");
                        else
                            cout << " " << colour(cl_flag_on, *i);
                    }
                    else
                    {
                        if (use_interface && use_interface->query_use_mask(*i, &p))
                            cout << " " << colour(cl_flag_off, "(-" + p::stringify(*i) + ")");
                        else
                            cout << " " << colour(cl_flag_off, "-" + p::stringify(*i));
                    }
                }
            }

            /* display tag, add tag to our post display list */
            if (! dep->get<p::dle_tag>().empty())
            {
                std::string tag_titles;
                for (std::set<p::DepTag::ConstPointer, p::DepTag::Comparator>::const_iterator
                        tag(dep->get<p::dle_tag>().begin()),
                        tag_end(dep->get<p::dle_tag>().end()) ;
                        tag != tag_end ; ++tag)
                {
                    all_tags.insert(*tag);
                    tag_titles.append((*tag)->short_text());
                    tag_titles.append(",");
                }
                tag_titles.erase(tag_titles.length() - 1);
                cout << " " << colour(cl_tag, "<" + tag_titles + ">");
            }

            cout << endl;
        }

        int current_count = 0, max_count = std::distance(dep_list.begin(), dep_list.end());

        cout << endl << "Total: " << max_count <<
            (max_count == 1 ? " package" : " packages") << endl << endl;

        if (CommandLine::get_instance()->a_pretend.specified() && ! all_tags.empty())
        {
            TagDisplayer tag_displayer;

            std::set<std::string> tag_categories;
            std::transform(
                    p::indirect_iterator<const p::DepTag>(all_tags.begin()),
                    p::indirect_iterator<const p::DepTag>(all_tags.end()),
                    std::inserter(tag_categories, tag_categories.begin()),
                    std::mem_fun_ref(&p::DepTag::category));

            for (std::set<std::string>::iterator cat(tag_categories.begin()),
                    cat_end(tag_categories.end()) ; cat != cat_end ; ++cat)
            {
                p::DepTagCategory::ConstPointer c(p::DepTagCategoryMaker::get_instance()->
                        find_maker(*cat)());

                if (! c->title().empty())
                    cout << colour(cl_heading, c->title()) << ":" << endl << endl;
                if (! c->pre_text().empty())
                    cout << c->pre_text() << endl << endl;

                for (std::set<p::DepTag::ConstPointer, p::DepTag::Comparator>::const_iterator
                        t(all_tags.begin()), t_end(all_tags.end()) ;
                        t != t_end ; ++t)
                {
                    if ((*t)->category() != *cat)
                        continue;
                    (*t)->accept(&tag_displayer);
                }
                cout << endl;

                if (! c->post_text().empty())
                    cout << c->post_text() << endl << endl;
            }
        }

        if (CommandLine::get_instance()->a_pretend.specified())
        {
            env->perform_hook(p::Hook("install_pretend_post")("TARGETS", p::join(
                            CommandLine::get_instance()->begin_parameters(),
                            CommandLine::get_instance()->end_parameters(), " ")));
            return return_code;
        }

        if (opts.get<p::io_fetchonly>())
            env->perform_hook(p::Hook("fetch_all_pre")("TARGETS", p::join(
                            CommandLine::get_instance()->begin_parameters(),
                            CommandLine::get_instance()->end_parameters(), " ")));
        else
            env->perform_hook(p::Hook("install_all_pre")("TARGETS", p::join(
                            CommandLine::get_instance()->begin_parameters(),
                            CommandLine::get_instance()->end_parameters(), " ")));

        for (p::DepList::Iterator dep(dep_list.begin()), dep_end(dep_list.end()) ;
                dep != dep_end ; ++dep)
        {
            std::string cpvr = p::stringify(dep->get<p::dle_name>()) + "-" +
                p::stringify(dep->get<p::dle_version>()) + "::" +
                p::stringify(dep->get<p::dle_repository>());

            if (opts.get<p::io_fetchonly>())
            {
                cout << endl << colour(cl_heading, "Fetching " + cpvr) << endl << endl;

                // TODO: some way to reset this properly would be nice.
                cerr << xterm_title("(" + p::stringify(++current_count) + " of " +
                        p::stringify(max_count) + ") Fetching " + cpvr);
 
            }
            else
            {
                cout << endl << colour(cl_heading,
                        "Installing " + cpvr) << endl << endl;

                // TODO: some way to reset this properly would be nice.
                cerr << xterm_title("(" + p::stringify(++current_count) + " of " +
                        p::stringify(max_count) + ") Installing " + cpvr);
            }

            if (opts.get<p::io_fetchonly>())
                env->perform_hook(p::Hook("fetch_pre")("TARGET", cpvr));
            else
                env->perform_hook(p::Hook("install_pre")("TARGET", cpvr));

            const p::Repository::InstallableInterface * const installable_interface(
                    env->package_database()->fetch_repository(dep->get<p::dle_repository>())->
                    get_interface<p::repo_installable>());
            if (! installable_interface)
                throw p::InternalError(PALUDIS_HERE, "Trying to install from a non-installable repository");
            installable_interface->install(dep->get<p::dle_name>(), dep->get<p::dle_version>(), opts);

            if (opts.get<p::io_fetchonly>())
                env->perform_hook(p::Hook("fetch_post")("TARGET", cpvr));
            else
                env->perform_hook(p::Hook("install_post")("TARGET", cpvr));

            if (! opts.get<p::io_fetchonly>())
            {
                // figure out if we need to unmerge anything
                cout << endl << colour(cl_heading,
                        "Cleaning stale versions after installing " + cpvr) << endl << endl;

                // manually invalidate any installed repos, they're probably
                // wrong now
                for (p::PackageDatabase::RepositoryIterator r(env->package_database()->begin_repositories()),
                        r_end(env->package_database()->end_repositories()) ; r != r_end ; ++r)
                    if ((*r)->get_interface<p::repo_installed>())
                        (*r)->invalidate();

                // look for packages with the same name in the same slot
                p::PackageDatabaseEntryCollection::Pointer collision_list(env->package_database()->query(
                            p::PackageDepAtom::Pointer(new p::PackageDepAtom(
                                    p::stringify(dep->get<p::dle_name>()) + ":" +
                                    p::stringify(dep->get<p::dle_metadata>()->get<p::vm_slot>()))),
                            p::is_installed_only));

                // don't clean the thing we just installed
                p::PackageDatabaseEntryCollection clean_list;
                for (p::PackageDatabaseEntryCollection::Iterator c(collision_list->begin()),
                        c_end(collision_list->end()) ; c != c_end ; ++c)
                    if (dep->get<p::dle_version>() != c->get<p::pde_version>())
                        clean_list.insert(*c);

                if (clean_list.empty())
                {
                    cout << "* No cleaning required" << endl;
                }
                else
                {
                    for (p::PackageDatabaseEntryCollection::Iterator c(clean_list.begin()),
                            c_end(clean_list.end()) ; c != c_end ; ++c)
                        cout << "* " << colour(cl_package_name, *c) << endl;
                    cout << endl;

                    p::PackageDatabaseEntryCollection::Iterator c(clean_list.begin()),
                        c_end(clean_list.end());
                    env->perform_hook(p::Hook("uninstall_all_pre")("TARGETS", p::join(c, c_end, " ")));
                    for ( ; c != c_end ; ++c)
                    {
                        cout << endl << colour(cl_heading, "Cleaning " + p::stringify(*c)) << endl << endl;

                        // TODO: some way to reset this properly would be nice.
                        cerr << xterm_title("(" + p::stringify(current_count) + " of " +
                                p::stringify(max_count) + ") Cleaning " + cpvr + ": " + stringify(*c));

                        env->perform_hook(p::Hook("uninstall_pre")("TARGET", stringify(*c)));

                        const p::Repository::UninstallableInterface * const uninstall_interface(
                                env->package_database()->fetch_repository(c->get<p::pde_repository>())->
                                get_interface<p::repo_uninstallable>());
                        if (! uninstall_interface)
                            throw p::InternalError(PALUDIS_HERE, "Trying to uninstall from a non-uninstallable repo");
                        uninstall_interface->uninstall(c->get<p::pde_name>(), c->get<p::pde_version>(), opts);
                        env->perform_hook(p::Hook("uninstall_post")("TARGET", stringify(*c)));
                    }
                    env->perform_hook(p::Hook("uninstall_all_pre")("TARGETS", p::join(c, c_end, " ")));
                }
            }
        }

        if (opts.get<p::io_fetchonly>())
            env->perform_hook(p::Hook("fetch_all_post")("TARGETS", p::join(
                            CommandLine::get_instance()->begin_parameters(),
                            CommandLine::get_instance()->end_parameters(), " ")));
        else
            env->perform_hook(p::Hook("install_all_post")("TARGETS", p::join(
                            CommandLine::get_instance()->begin_parameters(),
                            CommandLine::get_instance()->end_parameters(), " ")));

        cout << endl;
    }
    catch (const p::PackageInstallActionError & e)
    {
        cout << endl;
        cerr << "Install error:" << endl;
        cerr << "  * " << e.backtrace("\n  * ");
        cerr << e.message() << endl;

        return_code |= 1;
    }
    catch (const p::NoSuchPackageError & e)
    {
        cout << endl;
        cerr << "Query error:" << endl;
        cerr << "  * " << e.backtrace("\n  * ");
        cerr << "No such package '" << e.name() << "'" << endl;
        return 1;
    }
    catch (const p::AllMaskedError & e)
    {
        try
        {
            p::PackageDatabaseEntryCollection::ConstPointer p(env->package_database()->query(
                        p::PackageDepAtom::ConstPointer(new p::PackageDepAtom(e.query())),
                        p::is_uninstalled_only));
            if (p->empty())
            {
                cout << endl;
                cerr << "Query error:" << endl;
                cerr << "  * " << e.backtrace("\n  * ");
                cerr << "All versions of '" << e.query() << "' are masked" << endl;
            }
            else
            {
                cout << endl;
                cerr << "Query error:" << endl;
                cerr << "  * " << e.backtrace("\n  * ");
                cerr << "All versions of '" << e.query() << "' are masked. Candidates are:" << endl;
                for (p::PackageDatabaseEntryCollection::Iterator pp(p->begin()), pp_end(p->end()) ;
                        pp != pp_end ; ++pp)
                {
                    cerr << "    * " << colour(cl_package_name, *pp) << ": Masked by ";

                    bool need_comma(false);
                    p::MaskReasons m(env->mask_reasons(*pp));
                    for (unsigned mm = 0 ; mm < m.size() ; ++mm)
                        if (m[mm])
                        {
                            if (need_comma)
                                cerr << ", ";
                            cerr << p::MaskReason(mm);

                            if (p::mr_eapi == mm)
                            {
                                std::string eapi_str(env->package_database()->fetch_repository(
                                            pp->get<p::pde_repository>())->version_metadata(
                                            pp->get<p::pde_name>(), pp->get<p::pde_version>())->get<p::vm_eapi>());

                                cerr << " ( " << colour(cl_red, eapi_str) << " )";
                            }
                            else if (p::mr_license == mm)
                            {
                                cerr << " ";

                                LicenceDisplayer ld(cerr, env, &*pp);
                                env->package_database()->fetch_repository(
                                        pp->get<p::pde_repository>())->version_metadata(
                                        pp->get<p::pde_name>(), pp->get<p::pde_version>())->license()->
                                        accept(&ld);
                            }
                            else if (p::mr_keyword == mm)
                            {
                                p::VersionMetadata::ConstPointer m(env->package_database()->fetch_repository(
                                            pp->get<p::pde_repository>())->version_metadata(
                                            pp->get<p::pde_name>(), pp->get<p::pde_version>()));
                                if (m->get_ebuild_interface())
                                {
                                    std::set<p::KeywordName> keywords;
                                    p::WhitespaceTokeniser::get_instance()->tokenise(
                                            m->get_ebuild_interface()->get<p::evm_keywords>(),
                                            p::create_inserter<p::KeywordName>(
                                                std::inserter(keywords, keywords.end())));

                                    cerr << " ( " << colour(cl_red, p::join(keywords.begin(),
                                                    keywords.end(), " ")) << " )";
                                }
                            }

                            need_comma = true;
                        }
                    cerr << endl;
                }
            }
        }
        catch (...)
        {
            throw e;
        }

        return 1;
    }
    catch (const p::DepListStackTooDeepError & e)
    {
        cout << endl;
        cerr << "DepList stack too deep error:" << endl;
        cerr << "  * " << e.backtrace("\n  * ") << e.message() << endl;
        cerr << endl;
        cerr << "Try '--dl-max-stack-depth " << std::max(
                CommandLine::get_instance()->a_dl_max_stack_depth.argument() * 2, 100)
            << "'." << endl << endl;

        return_code |= 1;
    }
    catch (const p::DepListError & e)
    {
        cout << endl;
        cerr << "Dependency error:" << endl;
        cerr << "  * " << e.backtrace("\n  * ") << e.message() << " ("
            << e.what() << ")" << endl;
        cerr << endl;

        return_code |= 1;
    }

    return return_code;
}

