/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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
#include <paludis/repositories/e/qa/eapi_supported.hh>
#include <paludis/repositories/e/qa/short_description_key.hh>
#include <paludis/repositories/e/qa/homepage_key.hh>
#include <paludis/repositories/e/qa/spec_keys.hh>
#include <paludis/repositories/e/qa/extractors.hh>
#include <paludis/repositories/e/qa/visibility.hh>

using namespace paludis;
using namespace paludis::erepository;

template class InstantiationPolicy<QAChecks, instantiation_method::SingletonTag>;

namespace paludis
{
    template <>
    struct Implementation<QAChecks>
    {
        const tr1::shared_ptr<QAChecksGroup<TreeCheckFunction> > tree_checks_group;
        const tr1::shared_ptr<QAChecksGroup<CategoryDirCheckFunction> > category_dir_checks_group;
        const tr1::shared_ptr<QAChecksGroup<PackageIDCheckFunction> > package_id_checks_group;

        Implementation() :
            tree_checks_group(new QAChecksGroup<TreeCheckFunction>),
            category_dir_checks_group(new QAChecksGroup<CategoryDirCheckFunction>),
            package_id_checks_group(new QAChecksGroup<PackageIDCheckFunction>)
        {
        }
    };
}

QAChecks::QAChecks() :
    PrivateImplementationPattern<QAChecks>(new Implementation<QAChecks>())
{
    using namespace tr1::placeholders;

    _imp->tree_checks_group->add_check("stray_tree_files",
            tr1::bind(stray_files_check, _2, _4, _5, is_stray_at_tree_dir, "stray_tree_files"));

    _imp->category_dir_checks_group->add_check("stray_category_dir_files",
            tr1::bind(stray_files_check, _2, _4, _5, is_stray_at_category_dir, "stray_category_dir_files"));

    _imp->package_id_checks_group->add_check("eapi_supported",
            tr1::bind(eapi_supported_check, _1, _2, _5, "eapi_supported"));

    _imp->package_id_checks_group->add_check("short_description_key",
            tr1::bind(short_description_key_check, _1, _2, _5, "short_description_key"));
    _imp->package_id_checks_group->add_prerequirement("short_description_key", "eapi_supported");

    _imp->package_id_checks_group->add_check("homepage_key",
            tr1::bind(homepage_key_check, _1, _2, _5, "homepage_key"));
    _imp->package_id_checks_group->add_prerequirement("homepage_key", "eapi_supported");

    _imp->package_id_checks_group->add_check("spec_keys",
            tr1::bind(spec_keys_check, _1, _2, _5, "spec_keys"));
    _imp->package_id_checks_group->add_prerequirement("spec_keys", "eapi_supported");

    _imp->package_id_checks_group->add_check("extractors",
            tr1::bind(extractors_check, _1, _2, _5, "extractors"));
    _imp->package_id_checks_group->add_prerequirement("extractors", "eapi_supported");

    _imp->package_id_checks_group->add_check("visibility",
            tr1::bind(visibility_check, _1, _2, _3, _4, _5, "visibility"));
}

QAChecks::~QAChecks()
{
}

const tr1::shared_ptr<QAChecksGroup<TreeCheckFunction> >
QAChecks::tree_checks_group()
{
    return _imp->tree_checks_group;
}

const tr1::shared_ptr<QAChecksGroup<TreeCheckFunction> >
QAChecks::category_dir_checks_group()
{
    return _imp->category_dir_checks_group;
}

const tr1::shared_ptr<QAChecksGroup<PackageIDCheckFunction> >
QAChecks::package_id_checks_group()
{
    return _imp->package_id_checks_group;
}

