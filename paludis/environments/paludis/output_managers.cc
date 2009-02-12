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
#include <paludis/environments/paludis/action_to_string.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/config_file.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/map.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/simple_parser.hh>
#include <paludis/output_manager_factory.hh>
#include <paludis/create_output_manager_info.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <algorithm>
#include <tr1/unordered_map>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

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
OutputManagers::create_named_output_manager(const std::string & s, const CreateOutputManagerInfo & n) const
{
    Context context("When creating output manager named '" + s + "':");

    Store::const_iterator i(_imp->store.find(s));
    if (i == _imp->store.end())
        throw PaludisConfigError("No output manager named '" + s + "' exists");

    return OutputManagerFactory::get_instance()->create(
            std::tr1::bind(&from_kv, i->second, std::tr1::placeholders::_1),
            std::tr1::bind(&OutputManagers::create_named_output_manager, this, std::tr1::placeholders::_1, std::tr1::cref(n)),
            std::tr1::bind(&OutputManagers::replace_vars, this, std::tr1::placeholders::_1, std::tr1::cref(n))
            );
}

namespace
{
    std::string escape(const std::string & s)
    {
        std::string result(s);
        std::replace(result.begin(), result.end(), ' ', '_');
        std::replace(result.begin(), result.end(), '/', '_');
        return result;
    }

    struct CreateVarsFromInfo
    {
        std::tr1::unordered_map<std::string, std::string> & m;

        CreateVarsFromInfo(std::tr1::unordered_map<std::string, std::string> & mm) :
            m(mm)
        {
        }

        void visit(const CreateOutputManagerForRepositorySyncInfo & i)
        {
            m["type"] = "repository";
            m["action"] = "sync";
            m["name"] = stringify(i.repository().name());
            m["pid"] = stringify(getpid());
            m["time"] = stringify(time(0));
        }

        void visit(const CreateOutputManagerForPackageIDActionInfo & i)
        {
            m["type"] = "package";
            m["action"] = action_to_string(i.action());
            m["name"] = stringify(i.package_id()->name());
            m["id"] = escape(stringify(*i.package_id()));
            if (i.package_id()->slot_key())
                m["slot"] = stringify(i.package_id()->slot_key()->value());
            m["version"] = stringify(i.package_id()->version());
            m["repository"] = stringify(i.package_id()->repository()->name());
            m["category"] = stringify(i.package_id()->name().category());
            m["package"] = stringify(i.package_id()->name().package());
            m["pid"] = stringify(getpid());
            m["time"] = stringify(time(0));
        }
    };

    void create_vars_from_info(const CreateOutputManagerInfo & i,
            std::tr1::unordered_map<std::string, std::string> & m)
    {
        CreateVarsFromInfo v(m);
        i.accept(v);
    }
}

std::string
OutputManagers::replace_vars(
        const std::string & s,
        const CreateOutputManagerInfo & i) const
{
    Context context("When expanding variables in '" + s + "':");

    SimpleParser parser(s);
    std::string result, token;
    std::tr1::unordered_map<std::string, std::string> m;
    create_vars_from_info(i, m);

    while (! parser.eof())
    {
        if (parser.consume((+simple_parser::any_except("%")) >> token))
            result.append(token);
        else if (parser.consume(simple_parser::exact("%%")))
            result.append("%");
        else if (parser.consume(simple_parser::exact("%{") &
                    ((+simple_parser::any_except("} \t\r\n%")) >> token) &
                    simple_parser::exact("}")))
        {
            std::tr1::unordered_map<std::string, std::string>::const_iterator j(m.find(token));
            if (j == m.end())
                throw PaludisConfigError("No variable named '" + token + "' in var string '" + s + "'");
            else
                result.append(j->second);
        }
        else
            throw PaludisConfigError("Invalid var string '" + s + "'");
    }

    return result;
}

template class PrivateImplementationPattern<paludis_environment::OutputManagers>;

