/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#include <paludis/filter.hh>
#include <paludis/filter_handler.hh>
#include <paludis/name.hh>
#include <paludis/action.hh>
#include <paludis/environment.hh>
#include <paludis/package_id.hh>
#include <paludis/package_database.hh>
#include <paludis/metadata_key.hh>
#include <paludis/match_package.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/set.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/action_names.hh>

using namespace paludis;

namespace paludis
{
    template <>
    struct Imp<Filter>
    {
        std::shared_ptr<const FilterHandler> handler;

        Imp(const std::shared_ptr<const FilterHandler> & h) :
            handler(h)
        {
        }
    };
}

Filter::Filter(const std::shared_ptr<const FilterHandler> & h) :
    _imp(h)
{
}

Filter::Filter(const Filter & other) :
    _imp(other._imp->handler)
{
}

Filter &
Filter::operator= (const Filter & other)
{
    if (this != &other)
        _imp->handler = other._imp->handler;
    return *this;
}

Filter::~Filter()
{
}

std::shared_ptr<const RepositoryNameSet>
Filter::repositories(
        const Environment * const env,
        const std::shared_ptr<const RepositoryNameSet> & r) const
{
    return _imp->handler->repositories(env, r);
}

std::shared_ptr<const CategoryNamePartSet>
Filter::categories(
        const Environment * const env,
        const std::shared_ptr<const RepositoryNameSet> & r,
        const std::shared_ptr<const CategoryNamePartSet> & c) const
{
    return _imp->handler->categories(env, r, c);
}

std::shared_ptr<const QualifiedPackageNameSet>
Filter::packages(
        const Environment * const env,
        const std::shared_ptr<const RepositoryNameSet> & r,
        const std::shared_ptr<const QualifiedPackageNameSet> & c) const
{
    return _imp->handler->packages(env, r, c);
}

std::shared_ptr<const PackageIDSet>
Filter::ids(
        const Environment * const env,
        const std::shared_ptr<const PackageIDSet> & i) const
{
    return _imp->handler->ids(env, i);
}

std::string
Filter::as_string() const
{
    return _imp->handler->as_string();
}

namespace
{
    struct AllFilterHandler :
        AllFilterHandlerBase
    {
        virtual std::string as_string() const
        {
            return "all matches";
        }
    };

    template <typename A_>
    struct SupportsActionFilterHandler :
        AllFilterHandlerBase
    {
        virtual std::shared_ptr<const RepositoryNameSet> repositories(
                const Environment * const env,
                const std::shared_ptr<const RepositoryNameSet> & repos) const
        {
            std::shared_ptr<RepositoryNameSet> result(std::make_shared<RepositoryNameSet>());

            for (RepositoryNameSet::ConstIterator r(repos->begin()), r_end(repos->end()) ;
                    r != r_end ; ++r)
            {
                if (env->package_database()->fetch_repository(*r)->some_ids_might_support_action(SupportsActionTest<A_>()))
                    result->insert(*r);
            }

            return result;
        }

        virtual std::shared_ptr<const PackageIDSet> ids(
                const Environment * const,
                const std::shared_ptr<const PackageIDSet> & id) const
        {
            std::shared_ptr<PackageIDSet> result(std::make_shared<PackageIDSet>());

            for (PackageIDSet::ConstIterator i(id->begin()), i_end(id->end()) ;
                    i != i_end ; ++i)
            {
                if ((*i)->supports_action(SupportsActionTest<A_>()))
                    result->insert(*i);
            }

            return result;
        }

        virtual std::string as_string() const
        {
            return "supports action " + stringify(ActionNames<A_>::value);
        }
    };

    struct NotMaskedFilterHandler :
        AllFilterHandlerBase
    {
        virtual std::shared_ptr<const RepositoryNameSet> repositories(
                const Environment * const env,
                const std::shared_ptr<const RepositoryNameSet> & repos) const
        {
            std::shared_ptr<RepositoryNameSet> result(std::make_shared<RepositoryNameSet>());

            for (RepositoryNameSet::ConstIterator r(repos->begin()), r_end(repos->end()) ;
                    r != r_end ; ++r)
            {
                if (env->package_database()->fetch_repository(*r)->some_ids_might_not_be_masked())
                    result->insert(*r);
            }

            return result;
        }

        virtual std::shared_ptr<const PackageIDSet> ids(
                const Environment * const,
                const std::shared_ptr<const PackageIDSet> & id) const
        {
            std::shared_ptr<PackageIDSet> result(std::make_shared<PackageIDSet>());

            for (PackageIDSet::ConstIterator i(id->begin()), i_end(id->end()) ;
                    i != i_end ; ++i)
                if (! (*i)->masked())
                    result->insert(*i);

            return result;
        }

        virtual std::string as_string() const
        {
            return "not masked";
        }
    };

    struct InstalledAtFilterHandler :
        AllFilterHandlerBase
    {
        const FSPath root;
        const bool equal;

        InstalledAtFilterHandler(const FSPath & r, const bool e) :
            root(r),
            equal(e)
        {
        }

        virtual std::shared_ptr<const RepositoryNameSet> repositories(
                const Environment * const env,
                const std::shared_ptr<const RepositoryNameSet> & repos) const
        {
            std::shared_ptr<RepositoryNameSet> result(std::make_shared<RepositoryNameSet>());

            for (RepositoryNameSet::ConstIterator r(repos->begin()), r_end(repos->end()) ;
                    r != r_end ; ++r)
            {
                const std::shared_ptr<const Repository> repo(env->package_database()->fetch_repository(*r));
                if (repo->installed_root_key() && (equal == (root == repo->installed_root_key()->value())))
                    result->insert(*r);
            }

            return result;
        }

        virtual std::shared_ptr<const PackageIDSet> ids(
                const Environment * const,
                const std::shared_ptr<const PackageIDSet> & id) const
        {
            std::shared_ptr<PackageIDSet> result(std::make_shared<PackageIDSet>());

            for (PackageIDSet::ConstIterator i(id->begin()), i_end(id->end()) ;
                    i != i_end ; ++i)
                result->insert(*i);

            return result;
        }

        virtual std::string as_string() const
        {
            return "installed " + std::string(equal ? "" : "not ") + "at root " + stringify(root);
        }
    };

    struct AndFilterHandler :
        FilterHandler
    {
        const Filter f1;
        const Filter f2;

        AndFilterHandler(const Filter & a1, const Filter & a2) :
            f1(a1),
            f2(a2)
        {
        }

        virtual std::shared_ptr<const RepositoryNameSet> repositories(
                const Environment * const env,
                const std::shared_ptr<const RepositoryNameSet> & s) const
        {
            return f2.repositories(env, f1.repositories(env, s));
        }

        virtual std::shared_ptr<const CategoryNamePartSet> categories(
                const Environment * const env,
                const std::shared_ptr<const RepositoryNameSet> & r,
                const std::shared_ptr<const CategoryNamePartSet> & c) const
        {
            return f2.categories(env, r, f1.categories(env, r, c));
        }

        virtual std::shared_ptr<const QualifiedPackageNameSet> packages(
                const Environment * const env,
                const std::shared_ptr<const RepositoryNameSet> & r,
                const std::shared_ptr<const QualifiedPackageNameSet> & q) const
        {
            return f2.packages(env, r, f1.packages(env, r, q));
        }

        virtual std::shared_ptr<const PackageIDSet> ids(
                const Environment * const env,
                const std::shared_ptr<const PackageIDSet> & s) const
        {
            return f2.ids(env, f1.ids(env, s));
        }

        virtual std::string as_string() const
        {
            return stringify(f1) + " filtered through " + stringify(f2);
        }
    };

    struct SameSlotHandler :
        AllFilterHandlerBase
    {
        const std::shared_ptr<const PackageID> as_id;

        SameSlotHandler(const std::shared_ptr<const PackageID> & i) :
            as_id(i)
        {
        }

        virtual std::shared_ptr<const PackageIDSet> ids(
                const Environment * const,
                const std::shared_ptr<const PackageIDSet> & id) const
        {
            std::shared_ptr<PackageIDSet> result(std::make_shared<PackageIDSet>());

            for (PackageIDSet::ConstIterator i(id->begin()), i_end(id->end()) ;
                    i != i_end ; ++i)
                if (as_id->slot_key())
                {
                    if ((*i)->slot_key() && (*i)->slot_key()->value() == as_id->slot_key()->value())
                        result->insert(*i);
                }
                else
                {
                    if (! (*i)->slot_key())
                        result->insert(*i);
                }

            return result;
        }

        virtual std::string as_string() const
        {
            return "same slot as " + stringify(*as_id);
        }
    };

    struct SlotHandler :
        AllFilterHandlerBase
    {
        const SlotName slot;

        SlotHandler(const SlotName & s) :
            slot(s)
        {
        }

        virtual std::shared_ptr<const PackageIDSet> ids(
                const Environment * const,
                const std::shared_ptr<const PackageIDSet> & id) const
        {
            std::shared_ptr<PackageIDSet> result(std::make_shared<PackageIDSet>());

            for (PackageIDSet::ConstIterator i(id->begin()), i_end(id->end()) ;
                    i != i_end ; ++i)
                if ((*i)->slot_key() && (*i)->slot_key()->value() == slot)
                    result->insert(*i);

            return result;
        }

        virtual std::string as_string() const
        {
            return "slot is " + stringify(slot);
        }
    };

    struct NoSlotHandler :
        AllFilterHandlerBase
    {
        virtual std::shared_ptr<const PackageIDSet> ids(
                const Environment * const,
                const std::shared_ptr<const PackageIDSet> & id) const
        {
            std::shared_ptr<PackageIDSet> result(std::make_shared<PackageIDSet>());

            for (PackageIDSet::ConstIterator i(id->begin()), i_end(id->end()) ;
                    i != i_end ; ++i)
                if (! (*i)->slot_key())
                    result->insert(*i);

            return result;
        }

        virtual std::string as_string() const
        {
            return "has no slot";
        }
    };

    struct MatchesHandler :
        AllFilterHandlerBase
    {
        const PackageDepSpec spec;
        const std::shared_ptr<const PackageID> from_id;
        const MatchPackageOptions options;

        MatchesHandler(const PackageDepSpec & s, const std::shared_ptr<const PackageID> & f, const MatchPackageOptions & o) :
            spec(s),
            from_id(f),
            options(o)
        {
        }

        virtual std::shared_ptr<const PackageIDSet> ids(
                const Environment * const env,
                const std::shared_ptr<const PackageIDSet> & id) const
        {
            std::shared_ptr<PackageIDSet> result(std::make_shared<PackageIDSet>());

            for (PackageIDSet::ConstIterator i(id->begin()), i_end(id->end()) ;
                    i != i_end ; ++i)
            {
                if (match_package(*env, spec, *i, from_id, options))
                    result->insert(*i);
            }

            return result;
        }

        virtual std::string as_string() const
        {
            std::string suffix;
            if (options[mpo_ignore_additional_requirements])
                suffix = " (ignoring additional requirements)";
            return "packages matching " + stringify(spec) + suffix;
        }
    };
}

filter::All::All() :
    Filter(std::make_shared<AllFilterHandler>())
{
}

template <typename A_>
filter::SupportsAction<A_>::SupportsAction() :
    Filter(std::make_shared<SupportsActionFilterHandler<A_>>())
{
}

filter::NotMasked::NotMasked() :
    Filter(std::make_shared<NotMaskedFilterHandler>())
{
}

filter::InstalledAtRoot::InstalledAtRoot(const FSPath & r) :
    Filter(std::make_shared<InstalledAtFilterHandler>(r, true))
{
}

filter::InstalledNotAtRoot::InstalledNotAtRoot(const FSPath & r) :
    Filter(std::make_shared<InstalledAtFilterHandler>(r, false))
{
}

filter::InstalledAtSlash::InstalledAtSlash() :
    Filter(std::make_shared<InstalledAtFilterHandler>(FSPath("/"), true))
{
}

filter::InstalledAtNotSlash::InstalledAtNotSlash() :
    Filter(std::make_shared<InstalledAtFilterHandler>(FSPath("/"), false))
{
}

filter::And::And(const Filter & f1, const Filter & f2) :
    Filter(std::make_shared<AndFilterHandler>(f1, f2))
{
}

filter::SameSlot::SameSlot(const std::shared_ptr<const PackageID> & i) :
    Filter(std::make_shared<SameSlotHandler>(i))
{
}

filter::Slot::Slot(const SlotName & s) :
    Filter(std::make_shared<SlotHandler>(s))
{
}

filter::NoSlot::NoSlot() :
    Filter(std::make_shared<NoSlotHandler>())
{
}

filter::Matches::Matches(const PackageDepSpec & spec, const std::shared_ptr<const PackageID> & f, const MatchPackageOptions & o) :
    Filter(std::make_shared<MatchesHandler>(spec, f, o))
{
}

std::ostream &
paludis::operator<< (std::ostream & s, const Filter & f)
{
    s << f.as_string();
    return s;
}

template class Pimp<Filter>;
template class filter::SupportsAction<InstallAction>;
template class filter::SupportsAction<UninstallAction>;
template class filter::SupportsAction<PretendAction>;
template class filter::SupportsAction<ConfigAction>;
template class filter::SupportsAction<FetchAction>;
template class filter::SupportsAction<InfoAction>;
template class filter::SupportsAction<PretendFetchAction>;

