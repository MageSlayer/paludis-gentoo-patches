/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010 Ciaran McCreesh
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

#include "vdb_merger.hh"
#include <paludis/util/log.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/system.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/join.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/options.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/enum_iterator.hh>
#include <paludis/util/timestamp.hh>
#include <paludis/output_manager.hh>
#include <paludis/util/safe_ofstream.hh>
#include <paludis/util/safe_ifstream.hh>
#include <paludis/hook.hh>
#include <paludis/package_id.hh>
#include <paludis/util/md5.hh>
#include <paludis/environment.hh>
#include <paludis/package_database.hh>
#include <paludis/metadata_key.hh>
#include <iomanip>
#include <list>

using namespace paludis;

namespace paludis
{
    template<>
    struct Implementation<VDBMerger>
    {
        VDBMergerParams params;
        FSEntry realroot;
        std::tr1::shared_ptr<SafeOFStream> contents_file;

        std::list<std::string> config_protect;
        std::list<std::string> config_protect_mask;

        Implementation(const VDBMergerParams & p) :
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
    get_new_ids_or_minus_one(const Environment * const env, const FSEntry & f)
    {
        uid_t uid = (f.owner() == env->reduced_uid()) ? 0 : -1;
        gid_t gid = (f.group() == env->reduced_gid()) ? 0 : -1;

        return std::make_pair(uid, gid);
    }
}

VDBMerger::VDBMerger(const VDBMergerParams & p) :
    Merger(make_named_values<MergerParams>(
            value_for<n::environment>(p.environment()),
            value_for<n::fix_mtimes_before>(p.fix_mtimes_before()),
            value_for<n::get_new_ids_or_minus_one>(std::tr1::bind(&get_new_ids_or_minus_one, p.environment(), std::tr1::placeholders::_1)),
            value_for<n::image>(p.image()),
            value_for<n::install_under>(FSEntry("/")),
            value_for<n::merged_entries>(p.merged_entries()),
            value_for<n::no_chown>(! getenv_with_default("PALUDIS_NO_CHOWN", "").empty()),
            value_for<n::options>(p.options()),
            value_for<n::root>(p.root())
            )),
    PrivateImplementationPattern<VDBMerger>(new Implementation<VDBMerger>(p)),
    _imp(PrivateImplementationPattern<VDBMerger>::_imp)
{
}

VDBMerger::~VDBMerger()
{
}

Hook
VDBMerger::extend_hook(const Hook & h)
{
    std::tr1::shared_ptr<const FSEntrySequence> bashrc_files(_imp->params.environment()->bashrc_files());

    if (_imp->params.package_id())
    {
        std::string cat(stringify(_imp->params.package_id()->name().category()));
        std::string pn(stringify(_imp->params.package_id()->name().package()));
        std::string pvr(stringify(_imp->params.package_id()->version()));
        std::string pv(stringify(_imp->params.package_id()->version().remove_revision()));
        std::string slot(_imp->params.package_id()->slot_key() ? stringify(_imp->params.package_id()->slot_key()->value()) : "");

        return Merger::extend_hook(h)
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
        return Merger::extend_hook(h)
            ("CONFIG_PROTECT", _imp->params.config_protect())
            ("CONFIG_PROTECT_MASK", _imp->params.config_protect_mask())
            ("PALUDIS_BASHRC_FILES", join(bashrc_files->begin(), bashrc_files->end(), " "));
}

void
VDBMerger::record_install_file(const FSEntry & src, const FSEntry & dst_dir, const std::string & dst_name, const MergeStatusFlags & flags)
{
    std::string tidy(stringify((dst_dir / dst_name).strip_leading(_imp->realroot))),
            tidy_real(stringify((dst_dir / src.basename()).strip_leading(_imp->realroot)));
    Timestamp timestamp((dst_dir / dst_name).mtim());

    SafeIFStream infile(FSEntry(dst_dir / dst_name));
    if (! infile)
        throw MergerError("Cannot read '" + stringify(FSEntry(dst_dir / dst_name)) + "'");

    MD5 md5(infile);

    std::string line(make_arrows(flags) + " [obj] " + tidy_real);
    if (tidy_real != tidy)
        line.append(" (" + FSEntry(tidy).basename() + ")");
    display_override(line);

    *_imp->contents_file << "obj " << tidy_real << " " << md5.hexsum() << " " << timestamp.seconds() << std::endl;
}

void
VDBMerger::record_install_dir(const FSEntry & src, const FSEntry & dst_dir, const MergeStatusFlags & flags)
{
    std::string tidy(stringify((dst_dir / src.basename()).strip_leading(_imp->realroot)));
    display_override(make_arrows(flags) + " [dir] " + tidy);

    *_imp->contents_file << "dir " << tidy << std::endl;
}

void
VDBMerger::record_install_under_dir(const FSEntry & dst_dir, const MergeStatusFlags & flags)
{
    std::string tidy(stringify(dst_dir.strip_leading(_imp->realroot)));
    display_override(make_arrows(flags) + " [dir] " + tidy);

    *_imp->contents_file << "dir " << tidy << std::endl;
}

void
VDBMerger::record_install_sym(const FSEntry & src, const FSEntry & dst_dir, const MergeStatusFlags & flags)
{
    std::string tidy(stringify((dst_dir / src.basename()).strip_leading(_imp->realroot)));
    std::string target((dst_dir / src.basename()).readlink());
    Timestamp timestamp((dst_dir / src.basename()).mtim());

    display_override(make_arrows(flags) + " [sym] " + tidy);

    *_imp->contents_file << "sym " << tidy << " -> " << target << " " << timestamp.seconds() << std::endl;
}

void
VDBMerger::on_error(bool is_check, const std::string & s)
{
    make_check_fail();

    if (is_check)
        _imp->params.output_manager()->stdout_stream() << "." << std::endl << "!!! " << s << std::endl;
    else
        throw MergerError(s);
}

void
VDBMerger::on_warn(bool is_check, const std::string & s)
{
    if (is_check)
        Log::get_instance()->message("e.vdb_merger.warning", ll_warning, lc_context) << s;
}

bool
VDBMerger::config_protected(const FSEntry & src, const FSEntry & dst_dir)
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
VDBMerger::make_config_protect_name(const FSEntry & src, const FSEntry & dst)
{
    std::string result_name(src.basename());
    int n(0);

    SafeIFStream our_md5_file(src);
    if (! our_md5_file)
        throw MergerError("Could not get md5 for '" + stringify((dst / src.basename()).strip_leading(_imp->realroot)) + "'");
    MD5 our_md5(our_md5_file);

    while (true)
    {
        if (! (dst / result_name).exists())
            break;

        if ((dst / result_name).is_regular_file_or_symlink_to_regular_file())
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
    _imp->contents_file.reset(new SafeOFStream(_imp->params.contents_file()));
    Merger::merge();
}

bool
VDBMerger::check()
{
    _imp->params.output_manager()->stdout_stream() << ">>> Checking whether we can merge to " << _imp->params.root() << " ";
    bool result(Merger::check());
    _imp->params.output_manager()->stdout_stream() << std::endl;
    return result;
}

void
VDBMerger::on_enter_dir(bool is_check, const FSEntry)
{
    if (! is_check)
        return;

    _imp->params.output_manager()->stdout_stream() << "." << std::flush;
}

void
VDBMerger::on_file(bool is_check, const FSEntry & src, const FSEntry & dst)
{
    if (is_check && std::string::npos != src.basename().find('\n'))
        throw MergerError("File '" + stringify(src) + "' contains a newline in its name, which cannot be stored by VDB");
    Merger::on_file(is_check, src, dst);
}

void
VDBMerger::on_dir(bool is_check, const FSEntry & src, const FSEntry & dst)
{
    if (is_check && std::string::npos != src.basename().find('\n'))
        throw MergerError("Directory '" + stringify(src) + "' contains a newline in its name, which cannot be stored by VDB");
    Merger::on_dir(is_check, src, dst);
}

void
VDBMerger::on_sym(bool is_check, const FSEntry & src, const FSEntry & dst)
{
    if (is_check)
    {
        if (std::string::npos != src.basename().find('\n'))
            throw MergerError("Symlink '" + stringify(src) + "' contains a newline in its name, which cannot be stored by VDB");
        if (std::string::npos != src.readlink().find('\n'))
            throw MergerError("Symlink '" + stringify(src) + "' contains a newline in its target '" +
                              src.readlink() + "', which cannot be stored by VDB");
        if (std::string::npos != stringify(src).find(" -> "))
            throw MergerError("Symlink '" + stringify(src) + "' contains a ' -> ' in its name, which cannot be stored by VDB");
    }
    Merger::on_sym(is_check, src, dst);
}

void
VDBMerger::display_override(const std::string & message) const
{
    _imp->params.output_manager()->stdout_stream() << message << std::endl;
}

std::string
VDBMerger::make_arrows(const MergeStatusFlags & flags) const
{
    std::string result(">>>");
    for (EnumIterator<MergeStatusFlag> m, m_end(last_msi) ;
            m != m_end ; ++m)
    {
        if (! flags[*m])
            continue;

        switch (*m)
        {
            case msi_unlinked_first:
                result[0] = '<';
                continue;

            case msi_used_existing:
                result[0] = '=';
                continue;

            case msi_parent_rename:
                result[1] = '^';
                continue;

            case msi_rename:
                result[1] = '-';
                continue;

            case msi_as_hardlink:
                result[1] = '&';
                continue;

            case msi_fixed_ownership:
                result[2] = '~';
                continue;

            case msi_setid_bits:
                result[2] = '*';
                continue;

            case msi_xattr:
                result[2] = '+';
                continue;

            case last_msi:
                break;
        }
        throw InternalError(PALUDIS_HERE, "Unhandled MergeStatusFlag '" + stringify(static_cast<long>(*m)) + "'");
    }

    return result;
}

