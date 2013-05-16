/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010, 2011, 2013 Ciaran McCreesh
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

#include <paludis/repositories/e/info_metadata_key.hh>
#include <paludis/repositories/e/eapi.hh>
#include <paludis/repositories/e/e_repository.hh>

#include <paludis/util/pimp-impl.hh>
#include <paludis/util/config_file.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/options.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/join.hh>
#include <paludis/util/fs_stat.hh>

#include <paludis/literal_metadata_key.hh>
#include <paludis/selection.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/dep_spec.hh>
#include <paludis/environment.hh>
#include <paludis/package_id.hh>
#include <paludis/pretty_printer.hh>
#include <paludis/call_pretty_printer.hh>

#include <map>
#include <algorithm>
#include <functional>
#include <mutex>

using namespace paludis;
using namespace paludis::erepository;

namespace paludis
{
    template <>
    struct Imp<InfoVarsMetadataKey>
    {
        const std::shared_ptr<const FSPathSequence> locations;

        mutable std::mutex mutex;
        mutable std::shared_ptr<Set<std::string> > value;

        Imp(const std::shared_ptr<const FSPathSequence> & l) :
            locations(l)
        {
        }
    };

    template <>
    struct Imp<InfoPkgsMetadataKey>
    {
        const Environment * const env;
        const std::shared_ptr<const FSPathSequence> locations;
        const ERepository * const e_repository;

        mutable std::mutex mutex;
        mutable bool added;

        Imp(const Environment * const e, const std::shared_ptr<const FSPathSequence> & l,
                const ERepository * const r) :
            env(e),
            locations(l),
            e_repository(r),
            added(false)
        {
        }
    };
}

InfoVarsMetadataKey::InfoVarsMetadataKey(const std::shared_ptr<const FSPathSequence> & f) :
    _imp(f)
{
}

InfoVarsMetadataKey::~InfoVarsMetadataKey()
{
}

const std::shared_ptr<const Set<std::string> >
InfoVarsMetadataKey::parse_value() const
{
    std::unique_lock<std::mutex> lock(_imp->mutex);

    if (_imp->value)
        return _imp->value;
    _imp->value = std::make_shared<Set<std::string>>();

    for (FSPathSequence::ConstIterator location(_imp->locations->begin()), location_end(_imp->locations->end()) ;
            location != location_end ; ++location)
    {
        Context context("When loading info variables file '" + stringify(*location) + "':");

        if (location->stat().is_regular_file_or_symlink_to_regular_file())
        {
            LineConfigFile f(*location, { lcfo_disallow_continuations });
            for (LineConfigFile::ConstIterator line(f.begin()), line_end(f.end()) ;
                    line != line_end ; ++line)
                _imp->value->insert(*line);
        }
    }

    return _imp->value;
}

const std::string
InfoVarsMetadataKey::raw_name() const
{
    return "info_vars";
}

const std::string
InfoVarsMetadataKey::human_name() const
{
    return "Variable information names";
}

MetadataKeyType
InfoVarsMetadataKey::type() const
{
    return mkt_internal;
}

InfoPkgsMetadataKey::InfoPkgsMetadataKey(const Environment * const e,
        const std::shared_ptr<const FSPathSequence> & f,
        const ERepository * const r) :
    _imp(e, f, r)
{
}

InfoPkgsMetadataKey::~InfoPkgsMetadataKey()
{
}

void
InfoPkgsMetadataKey::need_keys_added() const
{
    std::unique_lock<std::mutex> lock(_imp->mutex);
    if (_imp->added)
        return;

    std::map<std::string, std::string> info_pkgs;
    for (FSPathSequence::ConstIterator location(_imp->locations->begin()), location_end(_imp->locations->end()) ;
            location != location_end ; ++location)
    {
        Context context("When loading info packages file '" + stringify(*location) + "':");
        _imp->added = true;

        if (location->stat().is_regular_file_or_symlink_to_regular_file())
        {
            std::string eapi(_imp->e_repository->eapi_for_file(*location));
            LineConfigFile p(*location, { lcfo_disallow_continuations });
            for (LineConfigFile::ConstIterator line(p.begin()), line_end(p.end()) ;
                    line != line_end ; ++line)
                info_pkgs.insert(std::make_pair(*line, eapi));
        }
    }

    for (std::map<std::string, std::string>::const_iterator i(info_pkgs.begin()), i_end(info_pkgs.end()) ;
            i != i_end ; ++i)
    {
        std::shared_ptr<const EAPI> eapi(erepository::EAPIData::get_instance()->eapi_from_string(i->second));
        std::shared_ptr<MetadataKey> key;

        if (eapi->supported())
        {
            std::shared_ptr<const PackageIDSequence> q((*_imp->env)[selection::AllVersionsSorted(
                        generator::Matches(parse_elike_package_dep_spec(i->first,
                                eapi->supported()->package_dep_spec_parse_options(),
                                eapi->supported()->version_spec_options()), nullptr, { }) |
                        filter::InstalledAtRoot(_imp->env->preferred_root_key()->parse_value()))]);

            if (q->empty())
                key = std::make_shared<LiteralMetadataValueKey<std::string>>(i->first, i->first, mkt_normal, "(none)");
            else
            {
                using namespace std::placeholders;
                std::shared_ptr<Set<std::string> > s(std::make_shared<Set<std::string>>());
                std::transform(indirect_iterator(q->begin()), indirect_iterator(q->end()), s->inserter(),
                        std::bind(std::mem_fn(&PackageID::canonical_form), _1, idcf_version));
                key = std::make_shared<LiteralMetadataStringSetKey>(i->first, i->first, mkt_normal, s);
            }
        }
        else
            key = std::make_shared<LiteralMetadataValueKey<std::string>>(i->first, i->first, mkt_normal, "(unknown EAPI)");

        add_metadata_key(key);
    }
}

const std::string
InfoVarsMetadataKey::pretty_print_value(
        const PrettyPrinter & pretty_printer,
        const PrettyPrintOptions &) const
{
    using namespace std::placeholders;
    auto v(parse_value());
    return join(v->begin(), v->end(), " ", CallPrettyPrinter(pretty_printer));
}

const std::string
InfoPkgsMetadataKey::raw_name() const
{
    return "info_pkgs";
}

const std::string
InfoPkgsMetadataKey::human_name() const
{
    return "Package information";
}

MetadataKeyType
InfoPkgsMetadataKey::type() const
{
    return mkt_normal;
}


namespace paludis
{
    template class Pimp<InfoPkgsMetadataKey>;
    template class Pimp<InfoVarsMetadataKey>;
}
