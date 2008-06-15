/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008 Ciaran McCreesh
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

#include <paludis/environments/paludis/world.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/log.hh>
#include <paludis/set_file.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/dep_tag.hh>
#include <tr1/functional>
#include <fstream>

using namespace paludis;
using namespace paludis::paludis_environment;

namespace paludis
{
    template <>
    struct Implementation<World>
    {
        const Environment * const env;
        const std::tr1::shared_ptr<const FSEntry> maybe_world_file;
        mutable Mutex mutex;

        Implementation(const Environment * const e, const std::tr1::shared_ptr<const FSEntry> & m) :
            env(e),
            maybe_world_file(m)
        {
        }
    };
}

World::World(const Environment * const e, const std::tr1::shared_ptr<const FSEntry> & f) :
    PrivateImplementationPattern<World>(new Implementation<World>(e, f))
{
}

World::~World()
{
}

void
World::add_to_world(const SetName & s) const
{
    _add_string_to_world(stringify(s));
}

void
World::add_to_world(const QualifiedPackageName & q) const
{
    _add_string_to_world(stringify(q));
}

void
World::remove_from_world(const SetName & s) const
{
    _remove_string_from_world(stringify(s));
}

void
World::remove_from_world(const QualifiedPackageName & q) const
{
    _remove_string_from_world(stringify(q));
}

void
World::_add_string_to_world(const std::string & n) const
{
    using namespace std::tr1::placeholders;

    if (! _imp->maybe_world_file)
    {
        Log::get_instance()->message("paludis_environment.world.no_world", ll_warning, lc_context)
            << "Not adding '" << n << "' to world because "
            "no world file has been configured. If you have recently upgraded from <paludis-0.26.0_alpha13, consult "
            "the FAQ Upgrades section.";
        return;
    }

    Lock l(_imp->mutex);

    Context context("When adding '" + n + "' to world file '" + stringify(*_imp->maybe_world_file) + "':");

    if (! _imp->maybe_world_file->exists())
    {
        std::ofstream f(stringify(*_imp->maybe_world_file).c_str());
        if (! f)
        {
            Log::get_instance()->message("paludis_environment.world.cannot_create", ll_warning, lc_no_context)
                << "Cannot create world file '" << *_imp->maybe_world_file << "'";
            return;
        }
    }

    SetFile world(SetFileParams::create()
            .file_name(*_imp->maybe_world_file)
            .type(sft_simple)
            .parser(std::tr1::bind(&parse_user_package_dep_spec, _1, _imp->env, UserPackageDepSpecOptions() + updso_no_disambiguation + updso_throw_if_set,
                    filter::All()))
            .tag(std::tr1::shared_ptr<DepTag>())
            .set_operator_mode(sfsmo_natural)
            .environment(_imp->env));
    world.add(n);
    world.rewrite();
}

void
World::_remove_string_from_world(const std::string & n) const
{
    using namespace std::tr1::placeholders;

    if (! _imp->maybe_world_file)
    {
        Log::get_instance()->message("paludis_environment.world.no_world", ll_warning, lc_context)
            << "Not removing '" << n << "' from world because no world file has been configured";
        return;
    }

    Lock l(_imp->mutex);

    Context context("When removing '" + n + "' from world file '" + stringify(*_imp->maybe_world_file) + "':");

    if (_imp->maybe_world_file->exists())
    {
        SetFile world(SetFileParams::create()
                .file_name(*_imp->maybe_world_file)
                .type(sft_simple)
                .parser(std::tr1::bind(&parse_user_package_dep_spec, _1, _imp->env, UserPackageDepSpecOptions() + updso_no_disambiguation + updso_throw_if_set,
                        filter::All()))
                .tag(std::tr1::shared_ptr<DepTag>())
                .set_operator_mode(sfsmo_natural)
                .environment(_imp->env));

        world.remove(n);
        world.rewrite();
    }
}

std::tr1::shared_ptr<SetSpecTree::ConstItem>
World::world_set() const
{
    using namespace std::tr1::placeholders;

    std::tr1::shared_ptr<GeneralSetDepTag> tag(new GeneralSetDepTag(SetName("world"), "Environment"));

    if (_imp->maybe_world_file)
    {
        if (_imp->maybe_world_file->exists())
        {
            SetFile world(SetFileParams::create()
                    .file_name(*_imp->maybe_world_file)
                    .type(sft_simple)
                    .parser(std::tr1::bind(&parse_user_package_dep_spec, _1, _imp->env, UserPackageDepSpecOptions() + updso_no_disambiguation + updso_throw_if_set,
                            filter::All()))
                    .tag(tag)
                    .set_operator_mode(sfsmo_natural)
                    .environment(_imp->env));
            return world.contents();
        }
        else
            Log::get_instance()->message("paludis_environment.world.no_world", ll_warning, lc_no_context)
                << "World file '" << *_imp->maybe_world_file << "' doesn't exist";
    }

    return std::tr1::shared_ptr<SetSpecTree::ConstItem>(new ConstTreeSequence<SetSpecTree, AllDepSpec>(
                std::tr1::shared_ptr<AllDepSpec>(new AllDepSpec)));
}

std::tr1::shared_ptr<const FSEntry>
World::location_if_set() const
{
    return _imp->maybe_world_file;
}

