/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008 Ciaran McCreesh
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

#include <paludis/repositories/e/qa/qa_checks.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/instantiation_policy-impl.hh>

#include <paludis/repositories/e/qa/stray_files.hh>
#include <paludis/repositories/e/qa/manifest.hh>
#include <paludis/repositories/e/qa/ebuild_count.hh>
#include <paludis/repositories/e/qa/changelog.hh>
#include <paludis/repositories/e/qa/misc_files.hh>
#include <paludis/repositories/e/qa/files_dir_size.hh>
#include <paludis/repositories/e/qa/eapi_supported.hh>
#include <paludis/repositories/e/qa/metadata_keys.hh>
#include <paludis/repositories/e/qa/short_description_key.hh>
#include <paludis/repositories/e/qa/homepage_key.hh>
#include <paludis/repositories/e/qa/iuse_key.hh>
#include <paludis/repositories/e/qa/keywords_key.hh>
#include <paludis/repositories/e/qa/license_key.hh>
#include <paludis/repositories/e/qa/spec_keys.hh>
#include <paludis/repositories/e/qa/extractors.hh>
#include <paludis/repositories/e/qa/fetches_key.hh>
#include <paludis/repositories/e/qa/restrict_key.hh>
#include <paludis/repositories/e/qa/inherited_key.hh>
#include <paludis/repositories/e/qa/visibility.hh>
#include <paludis/repositories/e/qa/default_functions.hh>
#include <paludis/repositories/e/qa/variable_assigns.hh>
#include <paludis/repositories/e/qa/deprecated_functions.hh>
#include <paludis/repositories/e/qa/kv_variables.hh>
#include <paludis/repositories/e/qa/root_variable.hh>
#include <paludis/repositories/e/qa/subshell_die.hh>
#include <paludis/repositories/e/qa/function_keyword.hh>
#include <paludis/repositories/e/qa/whitespace.hh>
#include <paludis/repositories/e/qa/header.hh>
#include <paludis/repositories/e/qa/repo_name.hh>
#include <paludis/repositories/e/qa/categories.hh>

using namespace paludis;
using namespace paludis::erepository;

template class InstantiationPolicy<QAChecks, instantiation_method::SingletonTag>;

namespace paludis
{
    template <>
    struct Implementation<QAChecks>
    {
        const std::tr1::shared_ptr<QAChecksGroup<TreeCheckFunction> > tree_checks_group;
        const std::tr1::shared_ptr<QAChecksGroup<EclassFileContentsCheckFunction> > eclass_file_contents_checks_group;
        const std::tr1::shared_ptr<QAChecksGroup<CategoryDirCheckFunction> > category_dir_checks_group;
        const std::tr1::shared_ptr<QAChecksGroup<PackageDirCheckFunction> > package_dir_checks_group;
        const std::tr1::shared_ptr<QAChecksGroup<PackageIDCheckFunction> > package_id_checks_group;
        const std::tr1::shared_ptr<QAChecksGroup<PackageIDFileContentsCheckFunction> > package_id_file_contents_checks_group;

        Implementation() :
            tree_checks_group(new QAChecksGroup<TreeCheckFunction>),
            eclass_file_contents_checks_group(new QAChecksGroup<EclassFileContentsCheckFunction>),
            category_dir_checks_group(new QAChecksGroup<CategoryDirCheckFunction>),
            package_dir_checks_group(new QAChecksGroup<PackageDirCheckFunction>),
            package_id_checks_group(new QAChecksGroup<PackageIDCheckFunction>),
            package_id_file_contents_checks_group(new QAChecksGroup<PackageIDFileContentsCheckFunction>)
        {
        }
    };
}

QAChecks::QAChecks() :
    PrivateImplementationPattern<QAChecks>(new Implementation<QAChecks>())
{
    using namespace std::tr1::placeholders;

    _imp->tree_checks_group->add_check("stray_tree_files",
            std::tr1::bind(stray_files_check, _2, _4, _1, is_stray_at_tree_dir, "stray_tree_files"));

    _imp->tree_checks_group->add_check("repo_name",
            std::tr1::bind(repo_name_check, _2, _1, "repo_name"));

    _imp->tree_checks_group->add_check("categories",
            std::tr1::bind(categories_check, _2, _4, "categories"));

    _imp->eclass_file_contents_checks_group->add_check("variable_assigns",
            std::tr1::bind(variable_assigns_check, _1, _2, std::tr1::shared_ptr<const ERepositoryID>(), _5, "variable_assigns"));

    _imp->eclass_file_contents_checks_group->add_check("deprecated_functions",
            std::tr1::bind(deprecated_functions_check, _1, _2, std::tr1::shared_ptr<const ERepositoryID>(), _5, "deprecated_functions"));

    _imp->eclass_file_contents_checks_group->add_check("subshell_die",
            std::tr1::bind(subshell_die_check, _1, _2, std::tr1::shared_ptr<const ERepositoryID>(), _5, "subshell_die"));

    _imp->eclass_file_contents_checks_group->add_check("header",
            std::tr1::bind(header_check, _1, _2, std::tr1::shared_ptr<const ERepositoryID>(), _5, "header"));

    _imp->eclass_file_contents_checks_group->add_check("function_keyword",
            std::tr1::bind(function_keyword_check, _1, _2, std::tr1::shared_ptr<const ERepositoryID>(), _5, "function_keyword"));

    _imp->eclass_file_contents_checks_group->add_check("whitespace",
            std::tr1::bind(whitespace_check, _1, _2, std::tr1::shared_ptr<const ERepositoryID>(), _5, "whitespace"));

    _imp->category_dir_checks_group->add_check("stray_category_dir_files",
            std::tr1::bind(stray_files_check, _2, _4, _1, is_stray_at_category_dir, "stray_category_dir_files"));

    _imp->package_dir_checks_group->add_check("manifest",
            std::tr1::bind(manifest_check, _2, _1, _4, _5, "manifest"));

    _imp->package_dir_checks_group->add_check("ebuild_count",
            std::tr1::bind(ebuild_count_check, _2, _1, _4, _5, "ebuild_count"));

    _imp->package_dir_checks_group->add_check("changelog",
            std::tr1::bind(changelog_check, _2, _1, _5, "changelog"));

    _imp->package_dir_checks_group->add_check("misc_files",
            std::tr1::bind(misc_files_check, _2, _1, "misc_files"));

    _imp->package_dir_checks_group->add_check("files_dir_size",
            std::tr1::bind(files_dir_size_check, _2, _1, "files_dir_size"));

    _imp->package_id_checks_group->add_check("eapi_supported",
            std::tr1::bind(eapi_supported_check, _1, _2, _5, "eapi_supported"));

    _imp->package_id_checks_group->add_check("metadata_keys",
            std::tr1::bind(metadata_keys_check, _1, _2, _5, "metadata_keys"));
    _imp->package_id_checks_group->add_prerequirement("metadata_keys", "eapi_supported");

    _imp->package_id_checks_group->add_check("short_description_key",
            std::tr1::bind(short_description_key_check, _1, _2, _5, "short_description_key"));
    _imp->package_id_checks_group->add_prerequirement("short_description_key", "metadata_keys");

    _imp->package_id_checks_group->add_check("homepage_key",
            std::tr1::bind(homepage_key_check, _1, _2, _5, "homepage_key"));
    _imp->package_id_checks_group->add_prerequirement("homepage_key", "metadata_keys");

    _imp->package_id_checks_group->add_check("iuse_key",
            std::tr1::bind(iuse_key_check, _1, _2, _4, _5, "iuse_key"));
    _imp->package_id_checks_group->add_prerequirement("iuse_key", "metadata_keys");

    _imp->package_id_checks_group->add_check("keywords_key",
            std::tr1::bind(keywords_key_check, _1, _2, _5, "keywords_key"));
    _imp->package_id_checks_group->add_prerequirement("keywords_key", "metadata_keys");

    _imp->package_id_checks_group->add_check("license_key",
            std::tr1::bind(license_key_check, _1, _2, _4, _5, "license_key"));
    _imp->package_id_checks_group->add_prerequirement("license_key", "metadata_keys");

    _imp->package_id_checks_group->add_check("spec_keys",
            std::tr1::bind(spec_keys_check, _1, _2, _5, "spec_keys"));
    _imp->package_id_checks_group->add_prerequirement("spec_keys", "metadata_keys");

    _imp->package_id_checks_group->add_check("extractors",
            std::tr1::bind(extractors_check, _1, _2, _5, "extractors"));
    _imp->package_id_checks_group->add_prerequirement("extractors", "metadata_keys");

    _imp->package_id_checks_group->add_check("fetches_key",
            std::tr1::bind(fetches_key_check, _1, _2, _5, "fetches_key"));
    _imp->package_id_checks_group->add_prerequirement("fetches_key", "metadata_keys");

    _imp->package_id_checks_group->add_check("restrict_key",
            std::tr1::bind(restrict_key_check, _1, _2, _5, "restrict_key"));
    _imp->package_id_checks_group->add_prerequirement("restrict_key", "metadata_keys");

    _imp->package_id_checks_group->add_check("inherited_key",
            std::tr1::bind(inherited_key_check, _1, _2, _5, "inherited_key"));
    _imp->package_id_checks_group->add_prerequirement("inherited_key", "metadata_keys");

    _imp->package_id_checks_group->add_check("visibility",
            std::tr1::bind(visibility_check, _1, _2, _3, _4, _5, "visibility"));
    _imp->package_id_checks_group->add_prerequirement("visibility", "metadata_keys");

    _imp->package_id_file_contents_checks_group->add_check("default_functions",
            std::tr1::bind(default_functions_check, _1, _2, _5, _6, "default_functions"));

    _imp->package_id_file_contents_checks_group->add_check("variable_assigns",
            std::tr1::bind(variable_assigns_check, _1, _2, _5, _6, "variable_assigns"));

    _imp->package_id_file_contents_checks_group->add_check("deprecated_functions",
            std::tr1::bind(deprecated_functions_check, _1, _2, _5, _6, "deprecated_functions"));

    _imp->package_id_file_contents_checks_group->add_check("kv_variables",
            std::tr1::bind(kv_variables_check, _1, _2, _5, _6, "kv_variables"));

    _imp->package_id_file_contents_checks_group->add_check("root_variable",
            std::tr1::bind(root_variable_check, _1, _2, _5, _6, "root_variable"));

    _imp->package_id_file_contents_checks_group->add_check("subshell_die",
            std::tr1::bind(subshell_die_check, _1, _2, _5, _6, "subshell_die"));

    _imp->package_id_file_contents_checks_group->add_check("function_keyword",
            std::tr1::bind(function_keyword_check, _1, _2, _5, _6, "function_keyword"));

    _imp->package_id_file_contents_checks_group->add_check("whitespace",
            std::tr1::bind(whitespace_check, _1, _2, _5, _6, "whitespace"));

    _imp->package_id_file_contents_checks_group->add_check("header",
            std::tr1::bind(header_check, _1, _2, _5, _6, "header"));
}

QAChecks::~QAChecks()
{
}

const std::tr1::shared_ptr<QAChecksGroup<TreeCheckFunction> >
QAChecks::tree_checks_group()
{
    return _imp->tree_checks_group;
}

const std::tr1::shared_ptr<QAChecksGroup<EclassFileContentsCheckFunction> >
QAChecks::eclass_file_contents_checks_group()
{
    return _imp->eclass_file_contents_checks_group;
}

const std::tr1::shared_ptr<QAChecksGroup<CategoryDirCheckFunction> >
QAChecks::category_dir_checks_group()
{
    return _imp->category_dir_checks_group;
}

const std::tr1::shared_ptr<QAChecksGroup<PackageDirCheckFunction> >
QAChecks::package_dir_checks_group()
{
    return _imp->package_dir_checks_group;
}

const std::tr1::shared_ptr<QAChecksGroup<PackageIDCheckFunction> >
QAChecks::package_id_checks_group()
{
    return _imp->package_id_checks_group;
}

const std::tr1::shared_ptr<QAChecksGroup<PackageIDFileContentsCheckFunction> >
QAChecks::package_id_file_contents_checks_group()
{
    return _imp->package_id_file_contents_checks_group;
}

