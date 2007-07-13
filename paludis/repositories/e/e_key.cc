/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <paludis/repositories/e/e_key.hh>
#include <paludis/repositories/e/ebuild_id.hh>

#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/iterator.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/log.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/set.hh>
#include <paludis/util/tr1_functional.hh>
#include <paludis/util/idle_action_pool.hh>

#include <paludis/portage_dep_parser.hh>
#include <paludis/eapi.hh>
#include <paludis/contents.hh>

#include <libwrapiter/libwrapiter_forward_iterator.hh>
#include <libwrapiter/libwrapiter_output_iterator.hh>

#include <list>
#include <vector>
#include <fstream>

using namespace paludis;
using namespace paludis::erepository;

EStringKey::EStringKey(const tr1::shared_ptr<const PackageID> &,
        const std::string & r, const std::string & h, const std::string & v, const MetadataKeyType t) :
    MetadataStringKey(r, h, t),
    _value(v)
{
}

EStringKey::~EStringKey()
{
}

const std::string
EStringKey::value() const
{
    return _value;
}

namespace paludis
{
    template <>
    struct Implementation<EDependenciesKey>
    {
        const tr1::shared_ptr<const PackageID> id;
        const std::string string_value;
        mutable Mutex value_mutex;
        mutable tr1::shared_ptr<const DependencySpecTree::ConstItem> value;
        mutable tr1::function<void () throw ()> value_used;

        Implementation(const tr1::shared_ptr<const PackageID> & i, const std::string & v) :
            id(i),
            string_value(v)
        {
        }
    };
}

EDependenciesKey::EDependenciesKey(const tr1::shared_ptr<const PackageID> & id,
        const std::string & r, const std::string & h, const std::string & v, const MetadataKeyType t) :
    MetadataSpecTreeKey<DependencySpecTree>(r, h, t),
    PrivateImplementationPattern<EDependenciesKey>(new Implementation<EDependenciesKey>(id, v)),
    _imp(PrivateImplementationPattern<EDependenciesKey>::_imp.get())
{
}

EDependenciesKey::~EDependenciesKey()
{
}

const tr1::shared_ptr<const DependencySpecTree::ConstItem>
EDependenciesKey::value() const
{
    Lock l(_imp->value_mutex);
    if (_imp->value)
    {
        if (_imp->value_used)
        {
            _imp->value_used();
            _imp->value_used = tr1::function<void () throw ()>();
        }
        return _imp->value;
    }

    IdleActionPool::get_instance()->increase_unprepared_stat();

    Context context("When parsing metadata key '" + raw_name() + "' from '" + stringify(*_imp->id) + "':");
    _imp->value = PortageDepParser::parse_depend(_imp->string_value, *_imp->id->eapi());
    return _imp->value;
}

IdleActionResult
EDependenciesKey::idle_load() const
{
    TryLock l(_imp->value_mutex);
    if (l() && ! _imp->value)
    {
        try
        {
            Context context("When parsing metadata key '" + raw_name() + "' from '" + stringify(*_imp->id) + "' as idle action:");
            _imp->value = PortageDepParser::parse_depend(_imp->string_value, *_imp->id->eapi());
            _imp->value_used = tr1::bind(tr1::mem_fn(&IdleActionPool::increase_used_stat), IdleActionPool::get_instance());
            return iar_success;
        }
        catch (...)
        {
            // the exception will be repeated in the relevant thread
            return iar_failure;
        }
    }

    return iar_already_completed;
}

namespace paludis
{
    template <>
    struct Implementation<ELicenseKey>
    {
        const tr1::shared_ptr<const PackageID> id;
        const std::string string_value;
        mutable Mutex value_mutex;
        mutable tr1::shared_ptr<const LicenseSpecTree::ConstItem> value;
        mutable tr1::function<void () throw ()> value_used;

        Implementation(const tr1::shared_ptr<const PackageID> & i, const std::string & v) :
            id(i),
            string_value(v)
        {
        }
    };
}

ELicenseKey::ELicenseKey(const tr1::shared_ptr<const PackageID> & id,
        const std::string & r, const std::string & h, const std::string & v, const MetadataKeyType t) :
    MetadataSpecTreeKey<LicenseSpecTree>(r, h, t),
    PrivateImplementationPattern<ELicenseKey>(new Implementation<ELicenseKey>(id, v)),
    _imp(PrivateImplementationPattern<ELicenseKey>::_imp.get())
{
}

ELicenseKey::~ELicenseKey()
{
}

const tr1::shared_ptr<const LicenseSpecTree::ConstItem>
ELicenseKey::value() const
{
    Lock l(_imp->value_mutex);
    if (_imp->value)
    {
        if (_imp->value_used)
        {
            _imp->value_used();
            _imp->value_used = tr1::function<void () throw ()>();
        }
        return _imp->value;
    }

    IdleActionPool::get_instance()->increase_unprepared_stat();

    Context context("When parsing metadata key '" + raw_name() + "' from '" + stringify(*_imp->id) + "':");
    _imp->value = PortageDepParser::parse_license(_imp->string_value, *_imp->id->eapi());
    return _imp->value;
}

IdleActionResult
ELicenseKey::idle_load() const
{
    TryLock l(_imp->value_mutex);
    if (l() && ! _imp->value)
    {
        try
        {
            Context context("When parsing metadata key '" + raw_name() + "' from '" + stringify(*_imp->id) + "' as idle action:");
            _imp->value = PortageDepParser::parse_license(_imp->string_value, *_imp->id->eapi());
            _imp->value_used = tr1::bind(tr1::mem_fn(&IdleActionPool::increase_used_stat), IdleActionPool::get_instance());
            return iar_success;
        }
        catch (...)
        {
            // the exception will be repeated in the relevant thread
            return iar_failure;
        }
    }

    return iar_already_completed;
}

namespace paludis
{
    template <>
    struct Implementation<EURIKey>
    {
        const tr1::shared_ptr<const PackageID> id;
        const std::string string_value;
        mutable tr1::shared_ptr<const URISpecTree::ConstItem> value;

        Implementation(const tr1::shared_ptr<const PackageID> & i, const std::string & v) :
            id(i),
            string_value(v)
        {
        }
    };
}

EURIKey::EURIKey(const tr1::shared_ptr<const PackageID> & id,
        const std::string & r, const std::string & h, const std::string & v, const MetadataKeyType t) :
    MetadataSpecTreeKey<URISpecTree>(r, h, t),
    PrivateImplementationPattern<EURIKey>(new Implementation<EURIKey>(id, v)),
    _imp(PrivateImplementationPattern<EURIKey>::_imp.get())
{
}

EURIKey::~EURIKey()
{
}

const tr1::shared_ptr<const URISpecTree::ConstItem>
EURIKey::value() const
{
    if (_imp->value)
        return _imp->value;

    Context context("When parsing metadata key '" + raw_name() + "' from '" + stringify(*_imp->id) + "':");
    _imp->value = PortageDepParser::parse_uri(_imp->string_value, *_imp->id->eapi());
    return _imp->value;
}

namespace paludis
{
    template <>
    struct Implementation<ERestrictKey>
    {
        const tr1::shared_ptr<const PackageID> id;
        const std::string string_value;
        mutable tr1::shared_ptr<const RestrictSpecTree::ConstItem> value;

        Implementation(const tr1::shared_ptr<const PackageID> & i, const std::string & v) :
            id(i),
            string_value(v)
        {
        }
    };
}

ERestrictKey::ERestrictKey(const tr1::shared_ptr<const PackageID> & id,
        const std::string & r, const std::string & h, const std::string & v, const MetadataKeyType t) :
    MetadataSpecTreeKey<RestrictSpecTree>(r, h, t),
    PrivateImplementationPattern<ERestrictKey>(new Implementation<ERestrictKey>(id, v)),
    _imp(PrivateImplementationPattern<ERestrictKey>::_imp.get())
{
}

ERestrictKey::~ERestrictKey()
{
}

const tr1::shared_ptr<const RestrictSpecTree::ConstItem>
ERestrictKey::value() const
{
    if (_imp->value)
        return _imp->value;

    Context context("When parsing metadata key '" + raw_name() + "' from '" + stringify(*_imp->id) + "':");
    _imp->value = PortageDepParser::parse_restrict(_imp->string_value, *_imp->id->eapi());
    return _imp->value;
}

namespace paludis
{
    template <>
    struct Implementation<EProvideKey>
    {
        const tr1::shared_ptr<const PackageID> id;
        const std::string string_value;
        mutable tr1::shared_ptr<const ProvideSpecTree::ConstItem> value;

        Implementation(const tr1::shared_ptr<const PackageID> & i, const std::string & v) :
            id(i),
            string_value(v)
        {
        }
    };
}

EProvideKey::EProvideKey(const tr1::shared_ptr<const PackageID> & id,
        const std::string & r, const std::string & h, const std::string & v, const MetadataKeyType t) :
    MetadataSpecTreeKey<ProvideSpecTree>(r, h, t),
    PrivateImplementationPattern<EProvideKey>(new Implementation<EProvideKey>(id, v)),
    _imp(PrivateImplementationPattern<EProvideKey>::_imp.get())
{
}

EProvideKey::~EProvideKey()
{
}

const tr1::shared_ptr<const ProvideSpecTree::ConstItem>
EProvideKey::value() const
{
    if (_imp->value)
        return _imp->value;

    Context context("When parsing metadata key '" + raw_name() + "' from '" + stringify(*_imp->id) + "':");
    _imp->value = PortageDepParser::parse_provide(_imp->string_value, *_imp->id->eapi());
    return _imp->value;
}

namespace paludis
{
    template <>
    struct Implementation<EIUseKey>
    {
        const tr1::shared_ptr<const PackageID> id;
        const std::string string_value;
        mutable Mutex value_mutex;
        mutable tr1::shared_ptr<IUseFlagSet> value;
        mutable tr1::function<void () throw ()> value_used;

        Implementation(const tr1::shared_ptr<const PackageID> & i, const std::string & v) :
            id(i),
            string_value(v)
        {
        }
    };
}

EIUseKey::EIUseKey(const tr1::shared_ptr<const PackageID> & id,
        const std::string & r, const std::string & h, const std::string & v, const MetadataKeyType t) :
    MetadataSetKey<IUseFlagSet>(r, h, t),
    PrivateImplementationPattern<EIUseKey>(new Implementation<EIUseKey>(id, v)),
    _imp(PrivateImplementationPattern<EIUseKey>::_imp.get())
{
}

EIUseKey::~EIUseKey()
{
}

const tr1::shared_ptr<const IUseFlagSet>
EIUseKey::value() const
{
    Lock l(_imp->value_mutex);
    if (_imp->value)
    {
        if (_imp->value_used)
        {
            _imp->value_used();
            _imp->value_used = tr1::function<void () throw ()>();
        }
        return _imp->value;
    }

    IdleActionPool::get_instance()->increase_unprepared_stat();

    Context context("When parsing metadata key '" + raw_name() + "' from '" + stringify(*_imp->id) + "':");
    _imp->value.reset(new IUseFlagSet);
    std::list<std::string> tokens;
    WhitespaceTokeniser::get_instance()->tokenise(_imp->string_value, std::back_inserter(tokens));
    for (std::list<std::string>::const_iterator t(tokens.begin()), t_end(tokens.end()) ;
            t != t_end ; ++t)
        _imp->value->insert(IUseFlag(*t, _imp->id->eapi()->supported->iuse_flag_parse_mode));

    return _imp->value;
}

IdleActionResult
EIUseKey::idle_load() const
{
    TryLock l(_imp->value_mutex);
    if (l() && ! _imp->value)
    {
        try
        {
            _imp->value.reset(new IUseFlagSet);
            std::list<std::string> tokens;
            WhitespaceTokeniser::get_instance()->tokenise(_imp->string_value, std::back_inserter(tokens));
            for (std::list<std::string>::const_iterator t(tokens.begin()), t_end(tokens.end()) ;
                    t != t_end ; ++t)
                _imp->value->insert(IUseFlag(*t, _imp->id->eapi()->supported->iuse_flag_parse_mode));
            _imp->value_used = tr1::bind(tr1::mem_fn(&IdleActionPool::increase_used_stat), IdleActionPool::get_instance());
        }
        catch (...)
        {
            // the exception will be repeated in the relevant thread
            _imp->value.reset();
            return iar_failure;
        }

        return iar_success;
    }

    return iar_already_completed;
}

namespace paludis
{
    template <>
    struct Implementation<EKeywordsKey>
    {
        const tr1::shared_ptr<const PackageID> id;
        const std::string string_value;
        mutable Mutex value_mutex;
        mutable tr1::shared_ptr<KeywordNameSet> value;
        mutable tr1::function<void () throw ()> value_used;

        Implementation(const tr1::shared_ptr<const PackageID> & i, const std::string & v) :
            id(i),
            string_value(v)
        {
        }
    };
}

EKeywordsKey::EKeywordsKey(const tr1::shared_ptr<const PackageID> & id,
        const std::string & r, const std::string & h, const std::string & v, const MetadataKeyType t) :
    MetadataSetKey<KeywordNameSet>(r, h, t),
    PrivateImplementationPattern<EKeywordsKey>(new Implementation<EKeywordsKey>(id, v)),
    _imp(PrivateImplementationPattern<EKeywordsKey>::_imp.get())
{
}

EKeywordsKey::~EKeywordsKey()
{
}

const tr1::shared_ptr<const KeywordNameSet>
EKeywordsKey::value() const
{
    Lock l(_imp->value_mutex);
    if (_imp->value)
    {
        if (_imp->value_used)
        {
            _imp->value_used();
            _imp->value_used = tr1::function<void () throw ()>();
        }
        return _imp->value;
    }

    IdleActionPool::get_instance()->increase_unprepared_stat();

    _imp->value.reset(new KeywordNameSet);
    Context context("When parsing metadata key '" + raw_name() + "' from '" + stringify(*_imp->id) + "':");
    WhitespaceTokeniser::get_instance()->tokenise(_imp->string_value, create_inserter<KeywordName>(_imp->value->inserter()));
    return _imp->value;
}

IdleActionResult
EKeywordsKey::idle_load() const
{
    TryLock l(_imp->value_mutex);
    if (l() && ! _imp->value)
    {
        try
        {
            _imp->value.reset(new KeywordNameSet);
            Context context("When parsing metadata key '" + raw_name() + "' from '" + stringify(*_imp->id) + "' as idle action:");
            WhitespaceTokeniser::get_instance()->tokenise(_imp->string_value, create_inserter<KeywordName>(_imp->value->inserter()));
            _imp->value_used = tr1::bind(tr1::mem_fn(&IdleActionPool::increase_used_stat), IdleActionPool::get_instance());
            return iar_success;
        }
        catch (...)
        {
            // the exception will be repeated in the relevant thread
            _imp->value.reset();
            return iar_failure;
        }
    }

    return iar_already_completed;
}

namespace paludis
{
    template <>
    struct Implementation<EUseKey>
    {
        const tr1::shared_ptr<const PackageID> id;
        const std::string string_value;
        mutable tr1::shared_ptr<UseFlagNameSet> value;

        Implementation(const tr1::shared_ptr<const PackageID> & i, const std::string & v) :
            id(i),
            string_value(v)
        {
        }
    };
}

EUseKey::EUseKey(const tr1::shared_ptr<const PackageID> & id,
        const std::string & r, const std::string & h, const std::string & v, const MetadataKeyType t) :
    MetadataSetKey<UseFlagNameSet>(r, h, t),
    PrivateImplementationPattern<EUseKey>(new Implementation<EUseKey>(id, v)),
    _imp(PrivateImplementationPattern<EUseKey>::_imp.get())
{
}

EUseKey::~EUseKey()
{
}

const tr1::shared_ptr<const UseFlagNameSet>
EUseKey::value() const
{
    if (_imp->value)
        return _imp->value;

    _imp->value.reset(new UseFlagNameSet);
    Context context("When parsing metadata key '" + raw_name() + "' from '" + stringify(*_imp->id) + "':");
    std::list<std::string> tokens;
    WhitespaceTokeniser::get_instance()->tokenise(_imp->string_value, std::back_inserter(tokens));
    for (std::list<std::string>::const_iterator t(tokens.begin()), t_end(tokens.end()) ;
            t != t_end ; ++t)
        if ('-' != t->at(0))
            _imp->value->insert(UseFlagName(*t));
    return _imp->value;
}

namespace paludis
{
    template <>
    struct Implementation<EInheritedKey>
    {
        const tr1::shared_ptr<const PackageID> id;
        const std::string string_value;
        mutable tr1::shared_ptr<InheritedSet> value;

        Implementation(const tr1::shared_ptr<const PackageID> & i, const std::string & v) :
            id(i),
            string_value(v)
        {
        }
    };
}

EInheritedKey::EInheritedKey(const tr1::shared_ptr<const PackageID> & id,
        const std::string & r, const std::string & h, const std::string & v, const MetadataKeyType t) :
    MetadataSetKey<InheritedSet>(r, h, t),
    PrivateImplementationPattern<EInheritedKey>(new Implementation<EInheritedKey>(id, v)),
    _imp(PrivateImplementationPattern<EInheritedKey>::_imp.get())
{
}

EInheritedKey::~EInheritedKey()
{
}

const tr1::shared_ptr<const InheritedSet>
EInheritedKey::value() const
{
    if (_imp->value)
        return _imp->value;

    _imp->value.reset(new InheritedSet);
    Context context("When parsing metadata key '" + raw_name() + "' from '" + stringify(*_imp->id) + "':");
    WhitespaceTokeniser::get_instance()->tokenise(_imp->string_value, _imp->value->inserter());
    return _imp->value;
}

namespace paludis
{
    template <>
    struct Implementation<EContentsKey>
    {
        const tr1::shared_ptr<const PackageID> id;
        const FSEntry filename;
        mutable tr1::shared_ptr<Contents> value;

        Implementation(const tr1::shared_ptr<const PackageID> & i, const FSEntry & v) :
            id(i),
            filename(v)
        {
        }
    };
}

EContentsKey::EContentsKey(const tr1::shared_ptr<const PackageID> & id,
        const std::string & r, const std::string & h, const FSEntry & v, const MetadataKeyType t) :
    MetadataContentsKey(r, h, t),
    PrivateImplementationPattern<EContentsKey>(new Implementation<EContentsKey>(id, v)),
    _imp(PrivateImplementationPattern<EContentsKey>::_imp.get())
{
}

EContentsKey::~EContentsKey()
{
}

const tr1::shared_ptr<const Contents>
EContentsKey::value() const
{
    if (_imp->value)
        return _imp->value;

    Context context("When creating contents for VDB key '" + stringify(*_imp->id) + "' from '" + stringify(_imp->filename) + "':");

    _imp->value.reset(new Contents);

    FSEntry f(_imp->filename);
    if (! f.is_regular_file_or_symlink_to_regular_file())
    {
        Log::get_instance()->message(ll_warning, lc_context) << "CONTENTS lookup failed for request for '" <<
                *_imp->id << "' using '" << _imp->filename << "'";
        return _imp->value;
    }

    std::ifstream ff(stringify(f).c_str());
    if (! ff)
        throw ConfigurationError("Could not read '" + stringify(f) + "'");

    std::string line;
    unsigned line_number(0);
    while (std::getline(ff, line))
    {
        ++line_number;

        std::vector<std::string> tokens;
        WhitespaceTokeniser::get_instance()->tokenise(line, std::back_inserter(tokens));
        if (tokens.empty())
            continue;

        if (tokens.size() < 2)
        {
            Log::get_instance()->message(ll_warning, lc_no_context) << "CONTENTS has broken line " <<
                line_number << ", skipping";
            continue;
        }

        if ("obj" == tokens.at(0))
            _imp->value->add(tr1::shared_ptr<ContentsEntry>(new ContentsFileEntry(tokens.at(1))));
        else if ("dir" == tokens.at(0))
            _imp->value->add(tr1::shared_ptr<ContentsEntry>(new ContentsDirEntry(tokens.at(1))));
        else if ("misc" == tokens.at(0))
            _imp->value->add(tr1::shared_ptr<ContentsEntry>(new ContentsMiscEntry(tokens.at(1))));
        else if ("fif" == tokens.at(0))
            _imp->value->add(tr1::shared_ptr<ContentsEntry>(new ContentsFifoEntry(tokens.at(1))));
        else if ("dev" == tokens.at(0))
            _imp->value->add(tr1::shared_ptr<ContentsEntry>(new ContentsDevEntry(tokens.at(1))));
        else if ("sym" == tokens.at(0))
        {
            if (tokens.size() < 4)
            {
                Log::get_instance()->message(ll_warning, lc_no_context) << "CONTENTS has broken sym line " <<
                    line_number << ", skipping";
                continue;
            }

            _imp->value->add(tr1::shared_ptr<ContentsEntry>(new ContentsSymEntry(tokens.at(1), tokens.at(3))));
        }
    }

    return _imp->value;
}

namespace paludis
{
    template <>
    struct Implementation<ECTimeKey>
    {
        const tr1::shared_ptr<const PackageID> id;
        const FSEntry filename;
        mutable tr1::shared_ptr<time_t> value;

        Implementation(const tr1::shared_ptr<const PackageID> & i, const FSEntry & v) :
            id(i),
            filename(v)
        {
        }
    };
}

ECTimeKey::ECTimeKey(const tr1::shared_ptr<const PackageID> & id,
        const std::string & r, const std::string & h, const FSEntry & v, const MetadataKeyType t) :
    MetadataTimeKey(r, h, t),
    PrivateImplementationPattern<ECTimeKey>(new Implementation<ECTimeKey>(id, v)),
    _imp(PrivateImplementationPattern<ECTimeKey>::_imp.get())
{
}

ECTimeKey::~ECTimeKey()
{
}

const time_t
ECTimeKey::value() const
{
    if (_imp->value)
        return *_imp->value;

    _imp->value.reset(new time_t(0));

    try
    {
        *_imp->value = _imp->filename.ctime();
    }
    catch (const FSError & e)
    {
        Log::get_instance()->message(ll_warning, lc_context) << "Couldn't get ctime for '"
            << _imp->filename << "' for ID '" << *_imp->id << "' due to exception '" << e.message()
            << "' (" << e.what() << ")";
    }

    return *_imp->value;
}

