/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009 Ciaran McCreesh
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

#include <paludis/repository.hh>
#include <paludis/util/log.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/options.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/sequence-impl.hh>
#include <paludis/util/set.hh>
#include <paludis/util/set-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/action.hh>
#include <paludis/metadata_key.hh>
#include <paludis/distribution-impl.hh>
#include <paludis/environment.hh>
#include <tr1/functional>
#include <map>
#include <list>
#include <utility>
#include <algorithm>
#include <ctype.h>

using namespace paludis;

template class Set<std::tr1::shared_ptr<Repository> >;
template class WrappedForwardIterator<Set<std::tr1::shared_ptr<Repository> >::ConstIteratorTag, const std::tr1::shared_ptr<Repository> >;
template class WrappedOutputIterator<Set<std::tr1::shared_ptr<Repository> >::InserterTag, std::tr1::shared_ptr<Repository> >;

template class Sequence<RepositoryVirtualsEntry>;
template class WrappedForwardIterator<Sequence<RepositoryVirtualsEntry>::ConstIteratorTag, const RepositoryVirtualsEntry>;
template class WrappedOutputIterator<Sequence<RepositoryVirtualsEntry>::InserterTag, RepositoryVirtualsEntry>;

template class Sequence<RepositoryProvidesEntry>;
template class WrappedForwardIterator<Sequence<RepositoryProvidesEntry>::ConstIteratorTag, const RepositoryProvidesEntry>;
template class WrappedOutputIterator<Sequence<RepositoryProvidesEntry>::InserterTag, RepositoryProvidesEntry>;

template class WrappedForwardIterator<RepositoryMirrorsInterface::MirrorsConstIteratorTag,
         const std::pair<const std::string, std::string> >;
template class WrappedForwardIterator<RepositoryEInterface::ProfilesConstIteratorTag, const RepositoryEInterface::ProfilesDescLine>;

NoSuchSetError::NoSuchSetError(const std::string & our_name) throw () :
    Exception("Could not find '" + our_name + "'"),
    _name(our_name)
{
}

NoSuchSetError::~NoSuchSetError() throw ()
{
}

RecursivelyDefinedSetError::RecursivelyDefinedSetError(const std::string & our_name) throw () :
    Exception("Set '" + our_name + "' is recursively defined"),
    _name(our_name)
{
}

RecursivelyDefinedSetError::~RecursivelyDefinedSetError() throw ()
{
}

namespace paludis
{
    namespace n
    {
        struct repository_blacklist;
    }
}

namespace
{
    struct RepositoryDistribution
    {
        NamedValue<n::repository_blacklist, std::tr1::function<std::string (const std::string &)> > repository_blacklist;
    };

    typedef ExtraDistributionData<RepositoryDistribution> RepositoryDistributionData;
}

namespace paludis
{
    template <>
    struct ExtraDistributionDataData<RepositoryDistribution>
    {
        static std::string config_file_name()
        {
            return "repository_blacklist.conf";
        }

        static std::tr1::shared_ptr<RepositoryDistribution> make_data(const std::tr1::shared_ptr<const KeyValueConfigFile> & k)
        {
            return make_shared_ptr(new RepositoryDistribution(make_named_values<RepositoryDistribution>(
                            value_for<n::repository_blacklist>(std::tr1::bind(std::tr1::mem_fn(&KeyValueConfigFile::get),
                                    k, std::tr1::placeholders::_1))
                            )));
        }
    };
}

namespace paludis
{
    template <>
    struct Implementation<Repository>
    {
        const RepositoryName name;

        Implementation(const RepositoryName & n) :
            name(n)
        {
        }
    };
}

Repository::Repository(
        const Environment * const env,
        const RepositoryName & our_name,
        const RepositoryCapabilities & caps) :
    PrivateImplementationPattern<Repository>(new Implementation<Repository>(our_name)),
    RepositoryCapabilities(caps),
    _imp(PrivateImplementationPattern<Repository>::_imp)
{
    std::string reason(RepositoryDistributionData::get_instance()->data_from_distribution(
                *DistributionData::get_instance()->distribution_from_string(
                    env->distribution()))->repository_blacklist()(stringify(our_name)));

    if (! reason.empty())
        Log::get_instance()->message("repository.blacklisted", ll_warning, lc_no_context)
            << "Repository '" << stringify(name())
            << "' is blacklisted with reason '" << reason << "'.";
}

Repository::~Repository()
{
}

const RepositoryName
Repository::name() const
{
    return _imp->name;
}

std::tr1::shared_ptr<const CategoryNamePartSet>
Repository::category_names_containing_package(const PackageNamePart & p) const
{
    Context context("When finding category names containing package '" + stringify(p) + "':");

    std::tr1::shared_ptr<CategoryNamePartSet> result(new CategoryNamePartSet);
    std::tr1::shared_ptr<const CategoryNamePartSet> cats(category_names());
    for (CategoryNamePartSet::ConstIterator c(cats->begin()), c_end(cats->end()) ;
            c != c_end ; ++c)
        if (has_package_named(*c + p))
            result->insert(*c);

    return result;
}

void
Repository::regenerate_cache() const
{
}

void
Repository::purge_invalid_cache() const
{
}

RepositoryEnvironmentVariableInterface::~RepositoryEnvironmentVariableInterface()
{
}

RepositoryMirrorsInterface::~RepositoryMirrorsInterface()
{
}

RepositoryProvidesInterface::~RepositoryProvidesInterface()
{
}

RepositoryVirtualsInterface::~RepositoryVirtualsInterface()
{
}

RepositoryDestinationInterface::~RepositoryDestinationInterface()
{
}

RepositoryEInterface::~RepositoryEInterface()
{
}

RepositoryMakeVirtualsInterface::~RepositoryMakeVirtualsInterface()
{
}

RepositoryManifestInterface::~RepositoryManifestInterface()
{
}

bool
RepositoryMirrorsInterface::is_mirror(const std::string & s) const
{
    return begin_mirrors(s) != end_mirrors(s);
}

bool
Repository::can_be_favourite_repository() const
{
    return some_ids_might_support_action(SupportsActionTest<InstallAction>());
}

std::tr1::shared_ptr<const CategoryNamePartSet>
Repository::unimportant_category_names() const
{
    return make_shared_ptr(new CategoryNamePartSet);
}

void
Repository::can_drop_in_memory_cache() const
{
}

