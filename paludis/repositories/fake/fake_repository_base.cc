/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <paludis/repositories/fake/fake_repository_base.hh>
#include <paludis/repositories/fake/fake_package_id.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/iterator.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/tr1_functional.hh>
#include <paludis/util/set.hh>
#include <paludis/util/sequence.hh>
#include <paludis/portage_dep_parser.hh>
#include <paludis/eapi.hh>
#include <paludis/repository_info.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>
#include <libwrapiter/libwrapiter_output_iterator.hh>
#include <map>

/** \file
 * Implementation for FakeRepositoryBase.
 *
 * \ingroup grpfakerepository
 */

using namespace paludis;

namespace paludis
{
    /**
     * Implementation data for FakeRepositoryBase.
     *
     * \ingroup grpfakerepository
     */
    template<>
    struct Implementation<FakeRepositoryBase> :
        private InstantiationPolicy<Implementation<FakeRepositoryBase>, instantiation_method::NonCopyableTag>
    {
        /// Our category names.
        tr1::shared_ptr<CategoryNamePartSet> category_names;

        /// Our package names.
        std::map<CategoryNamePart, tr1::shared_ptr<PackageNamePartSet> > package_names;

        /// Our IDs.
        std::map<QualifiedPackageName, tr1::shared_ptr<PackageIDSequence> > ids;

        /// Our sets.
        std::map<SetName, tr1::shared_ptr<SetSpecTree::ConstItem> > sets;

        const Environment * const env;

        /// Constructor.
        Implementation(const Environment * const);
    };

    Implementation<FakeRepositoryBase>::Implementation(const Environment * const e) :
        category_names(new CategoryNamePartSet),
        env(e)
    {
    }
}

FakeRepositoryBase::FakeRepositoryBase(const Environment * const e,
        const RepositoryName & our_name, const RepositoryCapabilities & caps,
        const std::string & f) :
    Repository(our_name, caps, f),
    RepositoryUseInterface(),
    PrivateImplementationPattern<FakeRepositoryBase>(new Implementation<FakeRepositoryBase>(e))
{
    tr1::shared_ptr<RepositoryInfoSection> config_info(new RepositoryInfoSection("Configuration information"));
    config_info->add_kv("format", "fake");

    _info->add_section(config_info);
}

FakeRepositoryBase::~FakeRepositoryBase()
{
}

bool
FakeRepositoryBase::do_has_category_named(const CategoryNamePart & c) const
{
    return (_imp->category_names->end() != _imp->category_names->find(c));
}

bool
FakeRepositoryBase::do_has_package_named(const QualifiedPackageName & q) const
{
    return has_category_named(q.category) &&
        (_imp->package_names.find(q.category)->second->end() !=
         _imp->package_names.find(q.category)->second->find(q.package));
}

tr1::shared_ptr<const CategoryNamePartSet>
FakeRepositoryBase::do_category_names() const
{
    return _imp->category_names;
}

tr1::shared_ptr<const QualifiedPackageNameSet>
FakeRepositoryBase::do_package_names(const CategoryNamePart & c) const
{
    tr1::shared_ptr<QualifiedPackageNameSet> result(new QualifiedPackageNameSet);
    if (! has_category_named(c))
        return result;

    PackageNamePartSet::Iterator p(_imp->package_names.find(c)->second->begin()),
        p_end(_imp->package_names.find(c)->second->end());
    for ( ; p != p_end ; ++p)
        result->insert(c + *p);
    return result;
}

tr1::shared_ptr<const PackageIDSequence>
FakeRepositoryBase::do_package_ids(const QualifiedPackageName & n) const
{
    if (! has_category_named(n.category))
        return tr1::shared_ptr<PackageIDSequence>(new PackageIDSequence);
    if (! has_package_named(n))
        return tr1::shared_ptr<PackageIDSequence>(new PackageIDSequence);
    return _imp->ids.find(n)->second;
}

void
FakeRepositoryBase::add_category(const CategoryNamePart & c)
{
    _imp->category_names->insert(c);
    _imp->package_names.insert(std::make_pair(c, new PackageNamePartSet));
}

void
FakeRepositoryBase::add_package(const QualifiedPackageName & q)
{
    add_category(q.category);
    _imp->package_names.find(q.category)->second->insert(q.package);
    _imp->ids.insert(std::make_pair(q, new PackageIDSequence));
}

tr1::shared_ptr<FakePackageID>
FakeRepositoryBase::add_version(const QualifiedPackageName & q, const VersionSpec & v)
{
    add_package(q);
    tr1::shared_ptr<FakePackageID> id(new FakePackageID(_imp->env, shared_from_this(), q, v));
    _imp->ids.find(q)->second->push_back(id);
    return id;
}

UseFlagState
FakeRepositoryBase::do_query_use(const UseFlagName &, const PackageID &) const
{
    return use_unspecified;
}

bool
FakeRepositoryBase::do_query_use_mask(const UseFlagName &, const PackageID &) const
{
    return false;
}

bool
FakeRepositoryBase::do_query_use_force(const UseFlagName &, const PackageID &) const
{
    return false;
}

tr1::shared_ptr<const UseFlagNameSet>
FakeRepositoryBase::do_arch_flags() const
{
    return tr1::shared_ptr<const UseFlagNameSet>(new UseFlagNameSet);
}

void
FakeRepositoryBase::invalidate()
{
}

tr1::shared_ptr<const UseFlagNameSet>
FakeRepositoryBase::do_use_expand_flags() const
{
    return tr1::shared_ptr<const UseFlagNameSet>(new UseFlagNameSet);
}

tr1::shared_ptr<const UseFlagNameSet>
FakeRepositoryBase::do_use_expand_hidden_prefixes() const
{
    return tr1::shared_ptr<const UseFlagNameSet>(new UseFlagNameSet);
}

tr1::shared_ptr<const UseFlagNameSet>
FakeRepositoryBase::do_use_expand_prefixes() const
{
    return tr1::shared_ptr<const UseFlagNameSet>(new UseFlagNameSet);
}

void
FakeRepositoryBase::add_package_set(const SetName & n, tr1::shared_ptr<SetSpecTree::ConstItem> s)
{
    _imp->sets.insert(std::make_pair(n, s));
}

tr1::shared_ptr<SetSpecTree::ConstItem>
FakeRepositoryBase::do_package_set(const SetName & id) const
{
    std::map<SetName, tr1::shared_ptr<SetSpecTree::ConstItem> >::const_iterator i(_imp->sets.find(id));
    if (_imp->sets.end() == i)
        return tr1::shared_ptr<SetSpecTree::ConstItem>();
    else
        return i->second;
}

tr1::shared_ptr<const SetNameSet>
FakeRepositoryBase::sets_list() const
{
    tr1::shared_ptr<SetNameSet> result(new SetNameSet);
    std::copy(_imp->sets.begin(), _imp->sets.end(), transform_inserter(result->inserter(),
                tr1::mem_fn(&std::pair<const SetName, tr1::shared_ptr<SetSpecTree::ConstItem> >::first)));
    return result;
}

std::string
FakeRepositoryBase::do_describe_use_flag(const UseFlagName &,
        const PackageID &) const
{
    return "";
}

const Environment *
FakeRepositoryBase::environment() const
{
    return _imp->env;
}

