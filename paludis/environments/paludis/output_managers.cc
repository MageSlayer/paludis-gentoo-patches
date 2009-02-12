/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009 Ciaran McCreesh
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

#include <paludis/environments/paludis/output_managers.hh>
#include <paludis/environments/paludis/bashable_conf.hh>
#include <paludis/environments/paludis/paludis_config.hh>
#include <paludis/environments/paludis/paludis_environment.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/config_file.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/map.hh>
#include <paludis/util/strip.hh>
#include <paludis/output_manager_factory.hh>
#include <tr1/unordered_map>

using namespace paludis;
using namespace paludis::paludis_environment;

namespace
{
    typedef std::tr1::unordered_map<std::string, std::tr1::shared_ptr<const KeyValueConfigFile> > Store;

    std::string from_kv(const std::tr1::shared_ptr<const KeyValueConfigFile> & m,
            const std::string & k)
    {
        return m->get(k);
    }
}

namespace paludis
{
    template<>
    struct Implementation<OutputManagers>
    {
        Store store;
    };
}

OutputManagers::OutputManagers(const PaludisEnvironment * const) :
    PrivateImplementationPattern<OutputManagers>(new Implementation<OutputManagers>)
{
}

OutputManagers::~OutputManagers()
{
}

void
OutputManagers::add(const FSEntry & filename)
{
    Context context("When adding source '" + stringify(filename) + "' as an output manager file:");

    std::tr1::shared_ptr<KeyValueConfigFile> f(make_bashable_kv_conf(filename));
    if (! f)
        return;

    std::string manager(filename.basename());
    manager = strip_trailing_string(manager, ".conf");
    manager = strip_trailing_string(manager, ".bash");

    _imp->store[manager] = f;
}

const std::tr1::shared_ptr<OutputManager>
OutputManagers::create_named_output_manager(const std::string & s) const
{
    Context context("When creating output manager named '" + s + "':");

    Store::const_iterator i(_imp->store.find(s));
    if (i == _imp->store.end())
        throw PaludisConfigError("No output manager named '" + s + "' exists");

    return OutputManagerFactory::get_instance()->create(
            std::tr1::bind(&from_kv, i->second, std::tr1::placeholders::_1),
            std::tr1::bind(&OutputManagers::create_named_output_manager, this, std::tr1::placeholders::_1)
            );
}

template class PrivateImplementationPattern<paludis_environment::OutputManagers>;


