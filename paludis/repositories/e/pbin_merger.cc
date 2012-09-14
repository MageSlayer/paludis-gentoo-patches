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

#include <paludis/repositories/e/pbin_merger.hh>

#include <paludis/util/pimp-impl.hh>
#include <paludis/util/fs_stat.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/system.hh>
#include <paludis/util/join.hh>
#include <paludis/util/log.hh>
#include <paludis/util/env_var_names.hh>

#include <paludis/hook.hh>
#include <paludis/environment.hh>
#include <paludis/package_id.hh>
#include <paludis/version_spec.hh>
#include <paludis/metadata_key.hh>
#include <paludis/output_manager.hh>
#include <paludis/slot.hh>

using namespace paludis;
using namespace paludis::erepository;

namespace paludis
{
    template<>
    struct Imp<PbinMerger>
    {
        PbinMergerParams params;

        Imp(const PbinMergerParams & p) :
            params(p)
        {
        }
    };
}

namespace
{
    std::pair<uid_t, gid_t>
    get_new_ids_or_minus_one(const Environment * const env, const FSPath & f)
    {
        FSStat f_stat(f);
        uid_t uid = (f_stat.owner() == env->reduced_uid()) ? 0 : -1;
        gid_t gid = (f_stat.group() == env->reduced_gid()) ? 0 : -1;

        return std::make_pair(uid, gid);
    }
}

PbinMerger::PbinMerger(const PbinMergerParams & p) :
    TarMerger(make_named_values<TarMergerParams>(
                n::compression() = tmc_none,
                n::environment() = p.environment(),
                n::fix_mtimes_before() = p.fix_mtimes_before(),
                n::get_new_ids_or_minus_one() = std::bind(&get_new_ids_or_minus_one, p.environment(), std::placeholders::_1),
                n::image() = p.image(),
                n::install_under() = FSPath("/"),
                n::maybe_output_manager() = p.output_manager(),
                n::merged_entries() = p.merged_entries(),
                n::no_chown() = ! getenv_with_default(env_vars::no_chown, "").empty(),
                n::options() = p.options(),
                n::permit_destination() = p.permit_destination(),
                n::root() = p.root(),
                n::tar_file() = p.tar_file()
            )),
    _imp(p)
{
}

PbinMerger::~PbinMerger() = default;

Hook
PbinMerger::extend_hook(const Hook & h)
{
    std::shared_ptr<const FSPathSequence> bashrc_files(_imp->params.environment()->bashrc_files());

    if (_imp->params.package_id())
    {
        std::string cat(stringify(_imp->params.package_id()->name().category()));
        std::string pn(stringify(_imp->params.package_id()->name().package()));
        std::string pvr(stringify(_imp->params.package_id()->version()));
        std::string pv(stringify(_imp->params.package_id()->version().remove_revision()));
        std::string slot(_imp->params.package_id()->slot_key() ? stringify(_imp->params.package_id()->slot_key()->parse_value().raw_value()) : "");

        return TarMerger::extend_hook(h)
            ("P", pn + "-" + pv)
            ("PNV", pn + "-" + pv)
            ("PN", pn)
            ("CATEGORY", cat)
            ("PR", _imp->params.package_id()->version().revision_only())
            ("PV", pv)
            ("PVR", pvr)
            ("PF", pn + "-" + pvr)
            ("PNVR", pn + "-" + pvr)
            ("SLOT", slot)
            ("PALUDIS_BASHRC_FILES", join(bashrc_files->begin(), bashrc_files->end(), " "));
    }
    else
        return TarMerger::extend_hook(h)
            ("PALUDIS_BASHRC_FILES", join(bashrc_files->begin(), bashrc_files->end(), " "));
}

void
PbinMerger::on_error(bool is_check, const std::string & s)
{
    make_check_fail();

    if (is_check)
        _imp->params.output_manager()->stdout_stream() << "." << std::endl << "!!! " << s << std::endl;
    else
        throw MergerError(s);
}

void
PbinMerger::on_warn(bool is_check, const std::string & s)
{
    if (is_check)
        Log::get_instance()->message("e.tar_merger.warning", ll_warning, lc_context) << s;
}

void
PbinMerger::merge()
{
    display_override(">>> Creating " + stringify(_imp->params.tar_file()));
    TarMerger::merge();
}

bool
PbinMerger::check()
{
    _imp->params.output_manager()->stdout_stream() << ">>> Checking whether we can merge to tarball " << _imp->params.tar_file() << " ";
    bool result(TarMerger::check());
    _imp->params.output_manager()->stdout_stream() << std::endl;
    return result;
}

void
PbinMerger::on_enter_dir(bool is_check, const FSPath)
{
    if (! is_check)
        return;

    _imp->params.output_manager()->stdout_stream() << "." << std::flush;
}

void
PbinMerger::display_override(const std::string & message) const
{
    _imp->params.output_manager()->stdout_stream() << message << std::endl;
}

void
PbinMerger::on_done_merge()
{
    add_file(_imp->params.environment_file(), FSPath("/PBIN/environment"));
}

void
PbinMerger::track_install_file(const FSPath &, const FSPath & dst)
{
    display_override(">>> [obj] " + stringify(dst));
}

void
PbinMerger::track_install_sym(const FSPath &, const FSPath & dst)
{
    display_override(">>> [sym] " + stringify(dst));
}

