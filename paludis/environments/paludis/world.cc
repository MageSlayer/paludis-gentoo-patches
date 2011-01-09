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

#include <paludis/environments/paludis/world.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/log.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/safe_ofstream.hh>
#include <paludis/util/fs_stat.hh>
#include <paludis/set_file.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/dep_tag.hh>
#include <paludis/partially_made_package_dep_spec.hh>
#include <functional>

using namespace paludis;
using namespace paludis::paludis_environment;

namespace paludis
{
    template <>
    struct Imp<World>
    {
        const Environment * const env;
        const std::shared_ptr<const FSPath> maybe_world_file;
        mutable Mutex mutex;

        Imp(const Environment * const e, const std::shared_ptr<const FSPath> & m) :
            env(e),
            maybe_world_file(m)
        {
        }
    };
}

World::World(const Environment * const e, const std::shared_ptr<const FSPath> & f) :
    _imp(e, f)
{
}

World::~World()
{
}

bool
World::add_to_world(const SetName & s) const
{
    return _add_string_to_world(stringify(s));
}

bool
World::add_to_world(const QualifiedPackageName & q) const
{
    return _add_string_to_world(stringify(q));
}

bool
World::remove_from_world(const SetName & s) const
{
    return _remove_string_from_world(stringify(s));
}

bool
World::remove_from_world(const QualifiedPackageName & q) const
{
    return _remove_string_from_world(stringify(q));
}

bool
World::_add_string_to_world(const std::string & n) const
{
    using namespace std::placeholders;

    if (! _imp->maybe_world_file)
    {
        Log::get_instance()->message("paludis_environment.world.no_world", ll_warning, lc_context)
            << "Not adding '" << n << "' to world because "
            "no world file has been configured. If you have recently upgraded from <paludis-0.26.0_alpha13, consult "
            "the FAQ Upgrades section.";
        return false;
    }

    Lock l(_imp->mutex);

    Context context("When adding '" + n + "' to world file '" + stringify(*_imp->maybe_world_file) + "':");

    if (! _imp->maybe_world_file->stat().exists())
    {
        try
        {
            SafeOFStream f(*_imp->maybe_world_file, -1, true);
        }
        catch (const SafeOFStreamError & e)
        {
            Log::get_instance()->message("paludis_environment.world.cannot_create", ll_warning, lc_no_context)
                << "Cannot create world file '" << *_imp->maybe_world_file << "': '" << e.message() << "' (" << e.what() << ")";
            return false;
        }
    }

    SetFile world(make_named_values<SetFileParams>(
                n::environment() = _imp->env,
                n::file_name() = *_imp->maybe_world_file,
                n::parser() = std::bind(&parse_user_package_dep_spec, _1, _imp->env, UserPackageDepSpecOptions() + updso_no_disambiguation + updso_throw_if_set, filter::All()),
                n::set_operator_mode() = sfsmo_natural,
                n::tag() = std::shared_ptr<DepTag>(),
                n::type() = sft_simple
                ));
    bool result(world.add(n));
    world.rewrite();

    return result;
}

bool
World::_remove_string_from_world(const std::string & n) const
{
    using namespace std::placeholders;
    bool result(false);

    if (! _imp->maybe_world_file)
    {
        Log::get_instance()->message("paludis_environment.world.no_world", ll_warning, lc_context)
            << "Not removing '" << n << "' from world because no world file has been configured";
        return result;
    }

    Lock l(_imp->mutex);

    Context context("When removing '" + n + "' from world file '" + stringify(*_imp->maybe_world_file) + "':");

    if (_imp->maybe_world_file->stat().exists())
    {
        SetFile world(make_named_values<SetFileParams>(
                n::environment() = _imp->env,
                n::file_name() = *_imp->maybe_world_file,
                n::parser() = std::bind(&parse_user_package_dep_spec, _1, _imp->env, UserPackageDepSpecOptions() + updso_no_disambiguation + updso_throw_if_set, filter::All()),
                n::set_operator_mode() = sfsmo_natural,
                n::tag() = std::shared_ptr<DepTag>(),
                n::type() = sft_simple
                ));

        result = world.remove(n);
        world.rewrite();
    }

    return result;
}

void
World::update_config_files_for_package_move(const PackageDepSpec & s, const QualifiedPackageName & n) const
{
    if (_remove_string_from_world(stringify(s)))
        _add_string_to_world(stringify(PartiallyMadePackageDepSpec(s).package(n)));
}

const std::shared_ptr<const SetSpecTree>
World::world_set() const
{
    using namespace std::placeholders;

    std::shared_ptr<GeneralSetDepTag> tag(std::make_shared<GeneralSetDepTag>(SetName("world"), "Environment"));

    if (_imp->maybe_world_file)
    {
        if (_imp->maybe_world_file->stat().exists())
        {
            SetFile world(make_named_values<SetFileParams>(
                    n::environment() = _imp->env,
                    n::file_name() = *_imp->maybe_world_file,
                    n::parser() = std::bind(&parse_user_package_dep_spec, _1, _imp->env, UserPackageDepSpecOptions() + updso_no_disambiguation + updso_throw_if_set, filter::All()),
                    n::set_operator_mode() = sfsmo_natural,
                    n::tag() = tag,
                    n::type() = sft_simple
                    ));
            return world.contents();
        }
        else
            Log::get_instance()->message("paludis_environment.world.no_world", ll_warning, lc_no_context)
                << "World file '" << *_imp->maybe_world_file << "' doesn't exist";
    }

    return std::make_shared<SetSpecTree>(std::make_shared<AllDepSpec>());
}

std::shared_ptr<const FSPath>
World::location_if_set() const
{
    return _imp->maybe_world_file;
}

