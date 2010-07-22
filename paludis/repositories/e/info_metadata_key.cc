/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009 Ciaran McCreesh
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
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/config_file.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/options.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/join.hh>
#include <paludis/literal_metadata_key.hh>
#include <paludis/selection.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/dep_spec.hh>
#include <paludis/environment.hh>
#include <paludis/package_id.hh>
#include <paludis/formatter.hh>
#include <map>
#include <algorithm>
#include <functional>

using namespace paludis;
using namespace paludis::erepository;

namespace paludis
{
    template <>
    struct Implementation<InfoVarsMetadataKey>
    {
        const std::shared_ptr<const FSEntrySequence> locations;

        mutable Mutex mutex;
        mutable std::shared_ptr<Set<std::string> > value;

        Implementation(const std::shared_ptr<const FSEntrySequence> & l) :
            locations(l)
        {
        }
    };

    template <>
    struct Implementation<InfoPkgsMetadataKey>
    {
        const Environment * const env;
        const std::shared_ptr<const FSEntrySequence> locations;
        const ERepository * const e_repository;

        mutable Mutex mutex;
        mutable bool added;

        Implementation(const Environment * const e, const std::shared_ptr<const FSEntrySequence> & l,
                const ERepository * const r) :
            env(e),
            locations(l),
            e_repository(r),
            added(false)
        {
        }
    };
}

InfoVarsMetadataKey::InfoVarsMetadataKey(const std::shared_ptr<const FSEntrySequence> & f) :
    PrivateImplementationPattern<InfoVarsMetadataKey>(new Implementation<InfoVarsMetadataKey>(f))
{
}

InfoVarsMetadataKey::~InfoVarsMetadataKey()
{
}

const std::shared_ptr<const Set<std::string> >
InfoVarsMetadataKey::value() const
{
    Lock l(_imp->mutex);

    if (_imp->value)
        return _imp->value;
    _imp->value.reset(new Set<std::string>);

    for (FSEntrySequence::ConstIterator location(_imp->locations->begin()), location_end(_imp->locations->end()) ;
            location != location_end ; ++location)
    {
        Context context("When loading info variables file '" + stringify(*location) + "':");

        if (location->is_regular_file_or_symlink_to_regular_file())
        {
            LineConfigFile f(*location, LineConfigFileOptions() + lcfo_disallow_continuations);
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
        const std::shared_ptr<const FSEntrySequence> & f,
        const ERepository * const r) :
    PrivateImplementationPattern<InfoPkgsMetadataKey>(new Implementation<InfoPkgsMetadataKey>(e, f, r)),
    _imp(PrivateImplementationPattern<InfoPkgsMetadataKey>::_imp)
{
}

InfoPkgsMetadataKey::~InfoPkgsMetadataKey()
{
}

void
InfoPkgsMetadataKey::need_keys_added() const
{
    Lock l(_imp->mutex);
    if (_imp->added)
        return;

    std::map<std::string, std::string> info_pkgs;
    for (FSEntrySequence::ConstIterator location(_imp->locations->begin()), location_end(_imp->locations->end()) ;
            location != location_end ; ++location)
    {
        Context context("When loading info packages file '" + stringify(*location) + "':");
        _imp->added = true;

        if (location->is_regular_file_or_symlink_to_regular_file())
        {
            std::string eapi(_imp->e_repository->eapi_for_file(*location));
            LineConfigFile p(*location, LineConfigFileOptions() + lcfo_disallow_continuations);
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
                                eapi->supported()->version_spec_options(),
                                std::shared_ptr<const PackageID>()), MatchPackageOptions()) |
                        filter::InstalledAtRoot(_imp->env->root()))]);

            if (q->empty())
                key.reset(new LiteralMetadataValueKey<std::string>(i->first, i->first, mkt_normal, "(none)"));
            else
            {
                using namespace std::placeholders;
                std::shared_ptr<Set<std::string> > s(new Set<std::string>);
                std::transform(indirect_iterator(q->begin()), indirect_iterator(q->end()), s->inserter(),
                        std::bind(std::mem_fn(&PackageID::canonical_form), _1, idcf_version));
                key.reset(new LiteralMetadataStringSetKey(i->first, i->first, mkt_normal, s));
            }
        }
        else
            key.reset(new LiteralMetadataValueKey<std::string>(i->first, i->first, mkt_normal, "(unknown EAPI)"));

        add_metadata_key(key);
    }
}

namespace
{
    std::string format_string(const std::string & i, const Formatter<std::string> & f)
    {
        return f.format(i, format::Plain());
    }
}

std::string
InfoVarsMetadataKey::pretty_print_flat(const Formatter<std::string> & f) const
{
    using namespace std::placeholders;
    return join(value()->begin(), value()->end(), " ", std::bind(&format_string, _1, f));
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


template class PrivateImplementationPattern<InfoPkgsMetadataKey>;
template class PrivateImplementationPattern<InfoVarsMetadataKey>;

