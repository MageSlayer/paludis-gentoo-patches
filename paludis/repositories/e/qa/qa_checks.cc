/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh
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
#include <paludis/repositories/e/qa/gpg.hh>
#include <paludis/repositories/e/qa/misc_files.hh>
#include <paludis/repositories/e/qa/files_dir_size.hh>
#include <paludis/repositories/e/qa/eapi_supported.hh>
#include <paludis/repositories/e/qa/metadata_keys.hh>
#include <paludis/repositories/e/qa/short_description_key.hh>
#include <paludis/repositories/e/qa/homepage_key.hh>
#include <paludis/repositories/e/qa/iuse_key.hh>
#include <paludis/repositories/e/qa/spec_keys.hh>
#include <paludis/repositories/e/qa/extractors.hh>
#include <paludis/repositories/e/qa/restrict_key.hh>
#include <paludis/repositories/e/qa/visibility.hh>
#include <paludis/repositories/e/qa/default_functions.hh>
#include <paludis/repositories/e/qa/kv_variables.hh>
#include <paludis/repositories/e/qa/whitespace.hh>
#include <paludis/repositories/e/qa/header.hh>
#include <paludis/repositories/e/qa/repo_name.hh>

using namespace paludis;
using namespace paludis::erepository;

template class InstantiationPolicy<QAChecks, instantiation_method::SingletonTag>;

namespace paludis
{
    template <>
    struct Implementation<QAChecks>
    {
        const tr1::shared_ptr<QAChecksGroup<TreeCheckFunction> > tree_checks_group;
        const tr1::shared_ptr<QAChecksGroup<EclassFileContentsCheckFunction> > eclass_file_contents_checks_group;
        const tr1::shared_ptr<QAChecksGroup<CategoryDirCheckFunction> > category_dir_checks_group;
        const tr1::shared_ptr<QAChecksGroup<PackageDirCheckFunction> > package_dir_checks_group;
        const tr1::shared_ptr<QAChecksGroup<PackageIDCheckFunction> > package_id_checks_group;
        const tr1::shared_ptr<QAChecksGroup<PackageIDFileContentsCheckFunction> > package_id_file_contents_checks_group;

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
    using namespace tr1::placeholders;

    _imp->tree_checks_group->add_check("stray_tree_files",
            tr1::bind(stray_files_check, _2, _4, _1, is_stray_at_tree_dir, "stray_tree_files"));

    _imp->tree_checks_group->add_check("repo_name",
            tr1::bind(repo_name_check, _2, _1, "repo_name"));

    _imp->category_dir_checks_group->add_check("stray_category_dir_files",
            tr1::bind(stray_files_check, _2, _4, _1, is_stray_at_category_dir, "stray_category_dir_files"));

    _imp->package_dir_checks_group->add_check("gpg",
            tr1::bind(gpg_check, _2, _1, "gpg"));

    _imp->package_dir_checks_group->add_check("misc_files",
            tr1::bind(misc_files_check, _2, _1, "misc_files"));

    _imp->package_dir_checks_group->add_check("files_dir_size",
            tr1::bind(files_dir_size_check, _2, _1, "files_dir_size"));

    _imp->package_id_checks_group->add_check("eapi_supported",
            tr1::bind(eapi_supported_check, _1, _2, _5, "eapi_supported"));

    _imp->package_id_checks_group->add_check("metadata_keys",
            tr1::bind(metadata_keys_check, _1, _2, _5, "metadata_keys"));
    _imp->package_id_checks_group->add_prerequirement("metadata_keys", "eapi_supported");

    _imp->package_id_checks_group->add_check("short_description_key",
            tr1::bind(short_description_key_check, _1, _2, _5, "short_description_key"));
    _imp->package_id_checks_group->add_prerequirement("short_description_key", "metadata_keys");

    _imp->package_id_checks_group->add_check("homepage_key",
            tr1::bind(homepage_key_check, _1, _2, _5, "homepage_key"));
    _imp->package_id_checks_group->add_prerequirement("homepage_key", "metadata_keys");

    _imp->package_id_checks_group->add_check("iuse_key",
            tr1::bind(iuse_key_check, _1, _2, _4, _5, "iuse_key"));
    _imp->package_id_checks_group->add_prerequirement("iuse_key", "metadata_keys");

    _imp->package_id_checks_group->add_check("spec_keys",
            tr1::bind(spec_keys_check, _1, _2, _5, "spec_keys"));
    _imp->package_id_checks_group->add_prerequirement("spec_keys", "metadata_keys");

    _imp->package_id_checks_group->add_check("extractors",
            tr1::bind(extractors_check, _1, _2, _5, "extractors"));
    _imp->package_id_checks_group->add_prerequirement("extractors", "metadata_keys");

    _imp->package_id_checks_group->add_check("restrict_key",
            tr1::bind(restrict_key_check, _1, _2, _5, "restrict_key"));
    _imp->package_id_checks_group->add_prerequirement("restrict_key", "metadata_keys");

    _imp->package_id_checks_group->add_check("visibility",
            tr1::bind(visibility_check, _1, _2, _3, _4, _5, "visibility"));
    _imp->package_id_checks_group->add_prerequirement("visibility", "metadata_keys");

    _imp->package_id_file_contents_checks_group->add_check("default_functions",
            tr1::bind(default_functions_check, _1, _2, _5, _6, "default_functions"));

    _imp->package_id_file_contents_checks_group->add_check("kv_variables",
            tr1::bind(kv_variables_check, _1, _2, _5, _6, "kv_variables"));

    _imp->package_id_file_contents_checks_group->add_check("whitespace",
            tr1::bind(whitespace_check, _1, _2, _5, _6, "whitespace"));

    _imp->package_id_file_contents_checks_group->add_check("header",
            tr1::bind(header_check, _1, _2, _5, _6, "header"));
}

QAChecks::~QAChecks()
{
}

const tr1::shared_ptr<QAChecksGroup<TreeCheckFunction> >
QAChecks::tree_checks_group()
{
    return _imp->tree_checks_group;
}

const tr1::shared_ptr<QAChecksGroup<EclassFileContentsCheckFunction> >
QAChecks::eclass_file_contents_checks_group()
{
    return _imp->eclass_file_contents_checks_group;
}

const tr1::shared_ptr<QAChecksGroup<CategoryDirCheckFunction> >
QAChecks::category_dir_checks_group()
{
    return _imp->category_dir_checks_group;
}

const tr1::shared_ptr<QAChecksGroup<PackageDirCheckFunction> >
QAChecks::package_dir_checks_group()
{
    return _imp->package_dir_checks_group;
}

const tr1::shared_ptr<QAChecksGroup<PackageIDCheckFunction> >
QAChecks::package_id_checks_group()
{
    return _imp->package_id_checks_group;
}

const tr1::shared_ptr<QAChecksGroup<PackageIDFileContentsCheckFunction> >
QAChecks::package_id_file_contents_checks_group()
{
    return _imp->package_id_file_contents_checks_group;
}

