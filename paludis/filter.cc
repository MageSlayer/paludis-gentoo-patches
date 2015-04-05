/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010, 2011, 2014 Ciaran McCreesh
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
#include <paludis/metadata_key.hh>
#include <paludis/match_package.hh>
#include <paludis/repository.hh>
#include <paludis/action_names.hh>
#include <paludis/slot.hh>

#include <paludis/util/pimp-impl.hh>
#include <paludis/util/set.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/stringify.hh>

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

Filter::~Filter() = default;

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
        const std::shared_ptr<const PackageIDSet> & ids) const
{
    return _imp->handler->ids(env, ids);
}

const RepositoryContentMayExcludes
Filter::may_excludes() const
{
    return _imp->handler->may_excludes();
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
        std::string as_string() const override
        {
            return "all matches";
        }
    };

    template <typename A_>
    struct SupportsActionFilterHandler :
        AllFilterHandlerBase
    {
        std::shared_ptr<const RepositoryNameSet>
        repositories(const Environment * const env, const std::shared_ptr<const RepositoryNameSet> & repos) const override
        {
            std::shared_ptr<RepositoryNameSet> result(std::make_shared<RepositoryNameSet>());

            for (const auto & repository : *repos)
                if (env->fetch_repository(repository)->some_ids_might_support_action(SupportsActionTest<A_>()))
                    result->insert(repository);

            return result;
        }

        std::shared_ptr<const PackageIDSet>
        ids(const Environment * const, const std::shared_ptr<const PackageIDSet> & ids) const override
        {
            std::shared_ptr<PackageIDSet> result(std::make_shared<PackageIDSet>());

            for (const auto & id : *ids)
                if (id->supports_action(SupportsActionTest<A_>()))
                    result->insert(id);

            return result;
        }

        std::string as_string() const override
        {
            return "supports action " + stringify(ActionNames<A_>::value);
        }
    };

    struct NotMaskedFilterHandler :
        AllFilterHandlerBase
    {
        const RepositoryContentMayExcludes may_excludes() const override
        {
            return { rcme_masked };
        }

        std::shared_ptr<const RepositoryNameSet>
        repositories(const Environment * const env, const std::shared_ptr<const RepositoryNameSet> & repos) const override
        {
            std::shared_ptr<RepositoryNameSet> result(std::make_shared<RepositoryNameSet>());

            for (const auto & repository : *repos)
                if (env->fetch_repository(repository)->some_ids_might_not_be_masked())
                    result->insert(repository);

            return result;
        }

        std::shared_ptr<const PackageIDSet>
        ids(const Environment * const, const std::shared_ptr<const PackageIDSet> & ids) const override
        {
            std::shared_ptr<PackageIDSet> result(std::make_shared<PackageIDSet>());

            for (const auto & id : *ids)
                if (! id->masked())
                    result->insert(id);

            return result;
        }

        std::string as_string() const override
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

        const RepositoryContentMayExcludes may_excludes() const override
        {
            return { rcme_not_installed };
        }

        std::shared_ptr<const RepositoryNameSet>
        repositories(const Environment * const env, const std::shared_ptr<const RepositoryNameSet> & repos) const override
        {
            std::shared_ptr<RepositoryNameSet> result(std::make_shared<RepositoryNameSet>());

            for (const auto & repository : *repos)
            {
                const std::shared_ptr<const Repository> repo(env->fetch_repository(repository));
                if (repo->installed_root_key() && (equal == (root == repo->installed_root_key()->parse_value())))
                    result->insert(repository);
            }

            return result;
        }

        std::shared_ptr<const PackageIDSet>
        ids(const Environment * const, const std::shared_ptr<const PackageIDSet> & ids) const override
        {
            std::shared_ptr<PackageIDSet> result(std::make_shared<PackageIDSet>());

            for (const auto & id : *ids)
                result->insert(id);

            return result;
        }

        std::string as_string() const override
        {
            return "installed " + std::string(equal ? "" : "not ") + "at root " + stringify(root);
        }
    };

    struct CrossCompileHostHandler :
        AllFilterHandlerBase
    {
        const std::string & host;

        CrossCompileHostHandler(const std::string & h) : host(h)
        {
        }

        std::shared_ptr<const RepositoryNameSet>
        repositories(const Environment * const env, const std::shared_ptr<const RepositoryNameSet> & repos) const override
        {
            auto result = std::make_shared<RepositoryNameSet>();

            if (host.empty())
                for (const auto & repository : *repos)
                    if (auto cross_ompile_host_key = env->fetch_repository(repository)->cross_compile_host_key())
                        continue;
                    else
                        result->insert(repository);
            else
                for (const auto & repository : *repos)
                    if (auto cross_compile_host_key = env->fetch_repository(repository)->cross_compile_host_key())
                        if (cross_compile_host_key->parse_value() == host)
                            result->insert(repository);

            return result;
        }

        std::string as_string() const override
        {
            return "cross compiled to " + host;
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

        const RepositoryContentMayExcludes may_excludes() const override
        {
            /* we can exclude anything either filter would reject */
            return f1.may_excludes() | f2.may_excludes();
        }

        std::shared_ptr<const RepositoryNameSet> repositories(
                const Environment * const env,
                const std::shared_ptr<const RepositoryNameSet> & s) const override
        {
            return f2.repositories(env, f1.repositories(env, s));
        }

        std::shared_ptr<const CategoryNamePartSet> categories(
                const Environment * const env,
                const std::shared_ptr<const RepositoryNameSet> & r,
                const std::shared_ptr<const CategoryNamePartSet> & c) const override
        {
            return f2.categories(env, r, f1.categories(env, r, c));
        }

        std::shared_ptr<const QualifiedPackageNameSet> packages(
                const Environment * const env,
                const std::shared_ptr<const RepositoryNameSet> & r,
                const std::shared_ptr<const QualifiedPackageNameSet> & q) const override
        {
            return f2.packages(env, r, f1.packages(env, r, q));
        }

        std::shared_ptr<const PackageIDSet> ids(
                const Environment * const env,
                const std::shared_ptr<const PackageIDSet> & ids) const override
        {
            return f2.ids(env, f1.ids(env, ids));
        }

        std::string as_string() const override
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

        const RepositoryContentMayExcludes may_excludes() const override
        {
            return { };
        }

        std::shared_ptr<const PackageIDSet>
        ids(const Environment * const, const std::shared_ptr<const PackageIDSet> & ids) const override
        {
            std::shared_ptr<PackageIDSet> result(std::make_shared<PackageIDSet>());

            for (const auto & id : *ids)
            {
                if (as_id->slot_key())
                {
                    if (id->slot_key() && id->slot_key()->parse_value().parallel_value() == as_id->slot_key()->parse_value().parallel_value())
                        result->insert(id);
                }
                else
                {
                    if (! id->slot_key())
                        result->insert(id);
                }
            }

            return result;
        }

        std::string as_string() const override
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

        const RepositoryContentMayExcludes may_excludes() const override
        {
            return { };
        }

        std::shared_ptr<const PackageIDSet>
        ids(const Environment * const, const std::shared_ptr<const PackageIDSet> & ids) const override
        {
            std::shared_ptr<PackageIDSet> result(std::make_shared<PackageIDSet>());

            for (const auto & id : *ids)
                if (id->slot_key() && id->slot_key()->parse_value().parallel_value() == slot)
                    result->insert(id);

            return result;
        }

        std::string as_string() const override
        {
            return "slot is " + stringify(slot);
        }
    };

    struct NoSlotHandler :
        AllFilterHandlerBase
    {
        std::shared_ptr<const PackageIDSet>
        ids(const Environment * const, const std::shared_ptr<const PackageIDSet> & ids) const override
        {
            std::shared_ptr<PackageIDSet> result(std::make_shared<PackageIDSet>());

            for (const auto & id : *ids)
                if (! id->slot_key())
                    result->insert(id);

            return result;
        }

        const RepositoryContentMayExcludes may_excludes() const override
        {
            return { };
        }

        std::string as_string() const override
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

        const RepositoryContentMayExcludes may_excludes() const override
        {
            return { };
        }

        std::shared_ptr<const PackageIDSet> ids(
                const Environment * const env,
                const std::shared_ptr<const PackageIDSet> & ids) const override
        {
            std::shared_ptr<PackageIDSet> result(std::make_shared<PackageIDSet>());

            for (const auto & id : *ids)
            {
                if (match_package(*env, spec, id, from_id, options))
                    result->insert(id);
            }

            return result;
        }

        std::string as_string() const override
        {
            std::string suffix;
            if (options[mpo_ignore_additional_requirements])
                suffix = " (ignoring additional requirements)";
            return "packages matching " + stringify(spec) + suffix;
        }
    };

    struct ByFunctionHandler :
        AllFilterHandlerBase
    {
        const std::function<bool (const std::shared_ptr<const PackageID> &)> func;
        const std::string desc;

        ByFunctionHandler(const std::function<bool (const std::shared_ptr<const PackageID> &)> & f, const std::string & s) :
            func(f),
            desc(s)
        {
        }

        std::shared_ptr<const PackageIDSet> ids(
                const Environment * const,
                const std::shared_ptr<const PackageIDSet> & ids) const override
        {
            std::shared_ptr<PackageIDSet> result(std::make_shared<PackageIDSet>());

            for (const auto & id : *ids)
            {
                if (! func(id))
                    result->insert(id);
            }

            return result;
        }

        std::string as_string() const override
        {
            return desc;
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

filter::CrossCompileHost::CrossCompileHost(const std::string & host) :
    Filter(std::make_shared<CrossCompileHostHandler>(host))
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

filter::ByFunction::ByFunction(
        const std::function<bool (const std::shared_ptr<const PackageID> &)> & f,
        const std::string & s) :
    Filter(std::make_shared<ByFunctionHandler>(f, s))
{
}

std::ostream &
paludis::operator<< (std::ostream & s, const Filter & f)
{
    s << f.as_string();
    return s;
}

namespace paludis
{
    template class Pimp<Filter>;
    template class filter::SupportsAction<InstallAction>;
    template class filter::SupportsAction<UninstallAction>;
    template class filter::SupportsAction<PretendAction>;
    template class filter::SupportsAction<ConfigAction>;
    template class filter::SupportsAction<FetchAction>;
    template class filter::SupportsAction<InfoAction>;
    template class filter::SupportsAction<PretendFetchAction>;
}

