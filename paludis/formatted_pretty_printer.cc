/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010, 2011 Ciaran McCreesh
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

#include <paludis/formatted_pretty_printer.hh>
#include <paludis/selection.hh>
#include <paludis/generator.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/filter.hh>
#include <paludis/environment.hh>
#include <paludis/metadata_key.hh>
#include <paludis/package_id.hh>
#include <paludis/repository.hh>

#include <paludis/util/pimp-impl.hh>
#include <paludis/util/stringify.hh>

using namespace paludis;

namespace paludis
{
    template <>
    struct Imp<FormattedPrettyPrinter>
    {
        const Environment * const env;
        const std::shared_ptr<const PackageID> package_id;

        Imp(const Environment * const e, const std::shared_ptr<const PackageID> & i) :
            env(e),
            package_id(i)
        {
        }
    };
}

FormattedPrettyPrinter::FormattedPrettyPrinter(
        const Environment * const e,
        const std::shared_ptr<const PackageID> & i) :
    _imp(e, i)
{
}

FormattedPrettyPrinter::~FormattedPrettyPrinter() = default;

const std::string
FormattedPrettyPrinter::prettify(const PackageDepSpec & v) const
{
    if (_imp->env)
    {
        {
            auto ids((*_imp->env)[selection::SomeArbitraryVersion(generator::Matches(v, _imp->package_id, { }) |
                        filter::InstalledAtRoot(_imp->env->preferred_root_key()->parse_value()))]);
            if (! ids->empty())
                return format_installed(stringify(v));
        }
        {
            auto ids((*_imp->env)[selection::SomeArbitraryVersion(generator::Matches(v, _imp->package_id, { }) |
                        filter::SupportsAction<InstallAction>() | filter::NotMasked())]);
            if (! ids->empty())
                return format_installable(stringify(v));
        }

        return format_masked(stringify(v));
    }
    else
        return format_plain(stringify(v));
}

const std::string
FormattedPrettyPrinter::prettify(const BlockDepSpec & v) const
{
    if (_imp->env)
    {
        {
            auto ids((*_imp->env)[selection::SomeArbitraryVersion(generator::Matches(v.blocking(), _imp->package_id, { }) |
                        filter::InstalledAtRoot(_imp->env->preferred_root_key()->parse_value()))]);
            if (! ids->empty())
                return format_masked(stringify(v));
        }

        return format_installable(stringify(v));
    }
    else
        return format_plain(stringify(v));
}

const std::string
FormattedPrettyPrinter::prettify(const ConditionalDepSpec & v) const
{
    if (_imp->env && _imp->package_id)
    {
        if (v.condition_met(_imp->env, _imp->package_id))
            return format_enabled(stringify(v));
        else
            return format_disabled(stringify(v));
    }
    else
        return format_plain(stringify(v));
}

const std::string
FormattedPrettyPrinter::prettify(const NamedSetDepSpec & v) const
{
    return format_plain(stringify(v));
}

const std::string
FormattedPrettyPrinter::prettify(const SimpleURIDepSpec & v) const
{
    return format_plain(stringify(v));
}

const std::string
FormattedPrettyPrinter::prettify(const PlainTextDepSpec & v) const
{
    return format_plain(stringify(v));
}

const std::string
FormattedPrettyPrinter::prettify(const LicenseDepSpec & v) const
{
    if (_imp->env && _imp->package_id)
    {
        if (_imp->env->accept_license(v.text(), _imp->package_id))
            return format_enabled(stringify(v));
        else
            return format_disabled(stringify(v));
    }
    else
        return format_plain(stringify(v));
}

const std::string
FormattedPrettyPrinter::prettify(const FetchableURIDepSpec & v) const
{
    return format_plain(stringify(v));
}

const std::string
FormattedPrettyPrinter::prettify(const URILabelsDepSpec & v) const
{
    return format_plain(stringify(v));
}

const std::string
FormattedPrettyPrinter::prettify(const DependenciesLabelsDepSpec & v) const
{
    return format_plain(stringify(v));
}

const std::string
FormattedPrettyPrinter::prettify(const PlainTextLabelDepSpec & v) const
{
    return format_plain(stringify(v));
}

const std::string
FormattedPrettyPrinter::prettify(const std::shared_ptr<const PackageID> & v) const
{
    if (_imp->env)
    {
        auto repo(_imp->env->fetch_repository(v->repository_name()));
        if (repo->installed_root_key())
            return format_installed(stringify(*v));
        else if (! v->masked())
            return format_installable(stringify(*v));
        else
            return format_masked(stringify(*v));
    }
    else
        return format_plain(stringify(*v));
}

const std::string
FormattedPrettyPrinter::prettify(const bool v) const
{
    return format_plain(stringify(v));
}

const std::string
FormattedPrettyPrinter::prettify(const long v) const
{
    return format_plain(stringify(v));
}

const std::string
FormattedPrettyPrinter::prettify(const std::string & v) const
{
    return format_plain(stringify(v));
}

const std::string
FormattedPrettyPrinter::prettify(const Maintainer & v) const
{
    return format_plain(stringify(v));
}

const std::string
FormattedPrettyPrinter::prettify(const std::pair<const std::string, std::string> & v) const
{
    if (v.first.empty())
        return format_plain(v.second);
    else
        return format_plain(v.first + ": " + v.second);
}

const std::string
FormattedPrettyPrinter::prettify(const FSPath & v) const
{
    return format_plain(stringify(v));
}

const std::string
FormattedPrettyPrinter::prettify(const KeywordName & v) const
{
    if (_imp->env && _imp->package_id)
    {
        auto k(std::make_shared<KeywordNameSet>());
        k->insert(v);

        if (_imp->env->accept_keywords(k, _imp->package_id))
            return format_enabled(stringify(v));
        else
            return format_disabled(stringify(v));
    }
    else
        return format_plain(stringify(v));
}

