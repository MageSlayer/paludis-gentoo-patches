/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#include <paludis/repositories/e/vdb_merger.hh>

#include <paludis/util/log.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/system.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/join.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/fs_stat.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/options.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/util/timestamp.hh>
#include <paludis/util/env_var_names.hh>
#include <paludis/util/safe_ofstream.hh>
#include <paludis/util/safe_ifstream.hh>
#include <paludis/util/env_var_names.hh>
#include <paludis/util/md5.hh>

#include <paludis/output_manager.hh>
#include <paludis/hook.hh>
#include <paludis/package_id.hh>
#include <paludis/environment.hh>
#include <paludis/metadata_key.hh>
#include <paludis/version_spec.hh>
#include <paludis/slot.hh>

#include <iomanip>
#include <list>

using namespace paludis;

namespace paludis
{
    template<>
    struct Imp<VDBMerger>
    {
        VDBMergerParams params;
        FSPath realroot;
        std::shared_ptr<SafeOFStream> contents_file;

        std::list<std::string> config_protect;
        std::list<std::string> config_protect_mask;

        Imp(const VDBMergerParams & p) :
            params(p),
            realroot(params.root().realpath())
        {
            tokenise_whitespace(params.config_protect(), std::back_inserter(config_protect));
            tokenise_whitespace(params.config_protect_mask(), std::back_inserter(config_protect_mask));
        }
    };
}

namespace
{
    std::pair<uid_t, gid_t>
    get_new_ids_or_minus_one(const Environment * const env, const FSPath & f)
    {
        FSStat f_stat(f.stat());
        uid_t uid = (f_stat.owner() == env->reduced_uid()) ? 0 : -1;
        gid_t gid = (f_stat.group() == env->reduced_gid()) ? 0 : -1;

        return std::make_pair(uid, gid);
    }
}

VDBMerger::VDBMerger(const VDBMergerParams & p) :
    FSMerger(make_named_values<FSMergerParams>(
                n::environment() = p.environment(),
                n::fix_mtimes_before() = p.fix_mtimes_before(),
                n::fs_merger_options() = p.fs_merger_options(),
                n::get_new_ids_or_minus_one() = std::bind(&get_new_ids_or_minus_one, p.environment(), std::placeholders::_1),
                n::image() = p.image(),
                n::install_under() = FSPath("/"),
                n::maybe_output_manager() = p.output_manager(),
                n::merged_entries() = p.merged_entries(),
                n::no_chown() = ! getenv_with_default(env_vars::no_chown, "").empty(),
                n::options() = p.options(),
                n::parts() = make_null_shared_ptr(),
                n::permit_destination() = p.permit_destination(),
                n::root() = p.root()
            )),
    _imp(p)
{
}

VDBMerger::~VDBMerger()
{
}

Hook
VDBMerger::extend_hook(const Hook & h)
{
    std::shared_ptr<const FSPathSequence> bashrc_files(_imp->params.environment()->bashrc_files());

    if (_imp->params.package_id())
    {
        std::string cat(stringify(_imp->params.package_id()->name().category()));
        std::string pn(stringify(_imp->params.package_id()->name().package()));
        std::string pvr(stringify(_imp->params.package_id()->version()));
        std::string pv(stringify(_imp->params.package_id()->version().remove_revision()));
        std::string slot(_imp->params.package_id()->slot_key() ? stringify(_imp->params.package_id()->slot_key()->parse_value().raw_value()) : "");

        return FSMerger::extend_hook(h)
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
            ("CONFIG_PROTECT", _imp->params.config_protect())
            ("CONFIG_PROTECT_MASK", _imp->params.config_protect_mask())
            ("PALUDIS_BASHRC_FILES", join(bashrc_files->begin(), bashrc_files->end(), " "));
    }
    else
        return FSMerger::extend_hook(h)
            ("CONFIG_PROTECT", _imp->params.config_protect())
            ("CONFIG_PROTECT_MASK", _imp->params.config_protect_mask())
            ("PALUDIS_BASHRC_FILES", join(bashrc_files->begin(), bashrc_files->end(), " "));
}

void
VDBMerger::record_install_file(const FSPath & src, const FSPath & dst_dir, const std::string & dst_name, const FSMergerStatusFlags & flags)
{
    const auto file(dst_dir / src.basename());
    const auto renamed_file(dst_dir / dst_name);

    const std::string tidy(stringify(renamed_file.strip_leading(_imp->realroot))),
                      tidy_real(stringify(file.strip_leading(_imp->realroot)));
    const Timestamp timestamp(renamed_file.stat().mtim());

    SafeIFStream infile(renamed_file);
    if (! infile)
        throw FSMergerError("Cannot read '" + stringify(renamed_file) + "'");

    MD5 md5(infile);

    display_merge(et_file, renamed_file, flags,
                  src.basename() == dst_name ? "" : dst_name);

    *_imp->contents_file << "obj " << tidy_real << " " << md5.hexsum() << " " << timestamp.seconds() << std::endl;
}

void
VDBMerger::record_install_dir(const FSPath & src, const FSPath & dst_dir, const FSMergerStatusFlags & flags)
{
    const auto dir(dst_dir / src.basename());
    const std::string tidy(stringify(dir.strip_leading(_imp->realroot)));

    display_merge(et_dir, dir, flags);

    *_imp->contents_file << "dir " << tidy << std::endl;
}

void
VDBMerger::record_install_under_dir(const FSPath & dst_dir, const FSMergerStatusFlags & flags)
{
    const std::string tidy(stringify(dst_dir.strip_leading(_imp->realroot)));

    display_merge(et_dir, dst_dir, flags);

    *_imp->contents_file << "dir " << tidy << std::endl;
}

void
VDBMerger::record_install_sym(const FSPath & src, const FSPath & dst_dir, const FSMergerStatusFlags & flags)
{
    const auto sym(dst_dir / src.basename());
    const std::string tidy(stringify(sym.strip_leading(_imp->realroot)));
    const std::string target(sym.readlink());
    const Timestamp timestamp(sym.stat().mtim());

    display_merge(et_sym, sym, flags);

    *_imp->contents_file << "sym " << tidy << " -> " << target << " " << timestamp.seconds() << std::endl;
}

void
VDBMerger::on_error(bool is_check, const std::string & s)
{
    make_check_fail();

    if (is_check)
        _imp->params.output_manager()->stdout_stream() << "." << std::endl << "!!! " << s << std::endl;
    else
        throw FSMergerError(s);
}

void
VDBMerger::on_warn(bool is_check, const std::string & s)
{
    if (is_check)
        Log::get_instance()->message("e.vdb_merger.warning", ll_warning, lc_context) << s;
}

bool
VDBMerger::config_protected(const FSPath & src, const FSPath & dst_dir)
{
    std::string tidy(stringify((dst_dir / src.basename()).strip_leading(_imp->realroot)));

    bool result(false);
    for (std::list<std::string>::const_iterator c(_imp->config_protect.begin()),
            c_end(_imp->config_protect.end()) ; c != c_end && ! result ; ++c)
    {
        std::string cc(strip_trailing(*c, "/") + "/");
        if (tidy == *c || 0 == tidy.compare(0, cc.length(), cc))
            result = true;
    }
    if (result)
        for (std::list<std::string>::const_iterator c(_imp->config_protect_mask.begin()),
                c_end(_imp->config_protect_mask.end()) ; c != c_end && result ; ++c)
        {
            std::string cc(strip_trailing(*c, "/") + "/");
            if (tidy == *c || 0 == tidy.compare(0, cc.length(), cc))
                result = false;
        }

    return result;
}

std::string
VDBMerger::make_config_protect_name(const FSPath & src, const FSPath & dst)
{
    std::string result_name(src.basename());
    int n(0);

    SafeIFStream our_md5_file(src);
    if (! our_md5_file)
        throw FSMergerError("Could not get md5 for '" + stringify((dst / src.basename()).strip_leading(_imp->realroot)) + "'");
    MD5 our_md5(our_md5_file);

    while (true)
    {
        FSPath dst_result_name(dst / result_name);
        FSStat dst_result_name_stat(dst_result_name);
        if (! dst_result_name_stat.exists())
            break;

        if (dst_result_name_stat.is_regular_file_or_symlink_to_regular_file())
        {
            try
            {
                SafeIFStream other_md5_file(dst / result_name);
                MD5 other_md5(other_md5_file);
                if (our_md5.hexsum() == other_md5.hexsum())
                    break;
            }
            catch (const SafeIFStreamError &)
            {
            }
        }

        std::stringstream s;
        s << std::setw(4) << std::setfill('0') << std::right << n++;
        result_name = "._cfg" + s.str() + "_" + src.basename();
    }

    return result_name;
}

void
VDBMerger::merge()
{
    display_override(">>> Merging to " + stringify(_imp->params.root()));
    _imp->contents_file = std::make_shared<SafeOFStream>(_imp->params.contents_file(), -1, false);
    FSMerger::merge();
}

bool
VDBMerger::check()
{
    _imp->params.output_manager()->stdout_stream() << ">>> Checking whether we can merge to " << _imp->params.root() << " ";
    bool result(FSMerger::check());
    _imp->params.output_manager()->stdout_stream() << std::endl;
    return result;
}

void
VDBMerger::on_enter_dir(bool is_check, const FSPath)
{
    if (! is_check)
        return;

    _imp->params.output_manager()->stdout_stream() << "." << std::flush;
}

void
VDBMerger::on_file(bool is_check, const FSPath & src, const FSPath & dst)
{
    if (is_check && std::string::npos != src.basename().find('\n'))
        throw FSMergerError("File '" + stringify(src) + "' contains a newline in its name, which cannot be stored by VDB");
    FSMerger::on_file(is_check, src, dst);
}

void
VDBMerger::on_dir(bool is_check, const FSPath & src, const FSPath & dst)
{
    if (is_check && std::string::npos != src.basename().find('\n'))
        throw FSMergerError("Directory '" + stringify(src) + "' contains a newline in its name, which cannot be stored by VDB");
    FSMerger::on_dir(is_check, src, dst);
}

void
VDBMerger::on_sym(bool is_check, const FSPath & src, const FSPath & dst)
{
    if (is_check)
    {
        if (std::string::npos != src.basename().find('\n'))
            throw FSMergerError("Symlink '" + stringify(src) + "' contains a newline in its name, which cannot be stored by VDB");
        if (std::string::npos != src.readlink().find('\n'))
            throw FSMergerError("Symlink '" + stringify(src) + "' contains a newline in its target '" +
                              src.readlink() + "', which cannot be stored by VDB");
        if (std::string::npos != stringify(src).find(" -> "))
            throw FSMergerError("Symlink '" + stringify(src) + "' contains a ' -> ' in its name, which cannot be stored by VDB");
    }
    FSMerger::on_sym(is_check, src, dst);
}

void
VDBMerger::display_override(const std::string & message) const
{
    _imp->params.output_manager()->stdout_stream() << message << std::endl;
}

