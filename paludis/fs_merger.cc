/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
 * Copyright (c) 2008 Fernando J. Pereda
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

#include <paludis/fs_merger.hh>
#include <paludis/util/enum_iterator.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/fd_holder.hh>
#include <paludis/util/log.hh>
#include <paludis/util/options.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/set.hh>
#include <paludis/util/timestamp.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/fs_stat.hh>
#include <paludis/util/fs_iterator.hh>
#include <paludis/util/fs_error.hh>
#include <paludis/selinux/security_context.hh>
#include <paludis/environment.hh>
#include <paludis/hook.hh>
#include <paludis/partitioning.hh>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <cstring>
#include <cstdio>
#include <list>
#include <set>
#include <unordered_map>

#include "config.h"

#ifdef HAVE_XATTRS
#  include <sys/xattr.h>
#endif

#ifdef HAVE_FALLOCATE
#  include <linux/falloc.h>
#endif

using namespace paludis;

#include <paludis/fs_merger-se.cc>

typedef std::unordered_map<std::pair<dev_t, ino_t>, std::string, Hash<std::pair<dev_t, ino_t> > > MergedMap;

namespace paludis
{
    template <>
    struct Imp<FSMerger>
    {
        MergedMap merged_ids;
        FSMergerParams params;
        std::set<FSPath, FSPathComparator> elided_paths;

        Imp(const FSMergerParams & p) :
            params(p)
        {
        }

        bool is_elided_directory(const FSPath & dir) const
        {
            for (FSIterator dentry(dir, { fsio_include_dotfiles }), invalid;
                    dentry != invalid; ++dentry)
            {
                const auto path = dentry->strip_leading(params.image());
                const auto part = params.parts()->classify(path);

                if (path.stat().is_directory())
                    if (! is_elided_directory(*dentry))
                        return false;

                if (params.should_merge()(path))
                    return false;
            }

            return true;
        }
    };
}

FSMergerError::FSMergerError(const std::string & s) noexcept :
    MergerError(s)
{
}

FSMergerError::FSMergerError(int error, const std::string & msg) noexcept :
    MergerError(msg + ": " + std::string(std::strerror(error)))
{
}

FSMerger::FSMerger(const FSMergerParams & p) :
    Merger(make_named_values<MergerParams>(
                n::environment() = p.environment(),
                n::fix_mtimes_before() = p.fix_mtimes_before(),
                n::get_new_ids_or_minus_one() = p.get_new_ids_or_minus_one(),
                n::image() = p.image(),
                n::install_under() = p.install_under(),
                n::maybe_output_manager() = p.maybe_output_manager(),
                n::merged_entries() = p.merged_entries(),
                n::no_chown() = p.no_chown(),
                n::options() = p.options(),
                n::permit_destination() = p.permit_destination(),
                n::root() = p.root()
                )),
    _imp(p)
{
}

FSMerger::~FSMerger() = default;

void
FSMerger::merge()
{
    struct SaveUmask
    {
        mode_t m;

        SaveUmask(mode_t mm) :
            m(mm)
        {
        }

        ~SaveUmask()
        {
            ::umask(m);
        }
    } old_umask(::umask(0000));

    Merger::merge();
}

void
FSMerger::prepare_install_under()
{
    Context context("When preparing install_under directory '" + stringify(_imp->params.install_under()) + "' under root '"
            + stringify(_imp->params.root()) + "':");

    std::list<FSPath> dd;
    for (FSPath d(_imp->params.root().realpath() / _imp->params.install_under()), d_end(_imp->params.root().realpath()) ;
            d != d_end ; d = d.dirname())
        dd.push_front(d);
    for (auto & d : dd)
        if (! d.stat().exists())
        {
            d.mkdir(0755, { });
            track_install_under_dir(d, FSMergerStatusFlags());
        }
        else
            track_install_under_dir(d, FSMergerStatusFlags() + msi_used_existing);
}

void
FSMerger::on_file_over_nothing(bool is_check, const FSPath & src, const FSPath & dst)
{
    if (is_check)
        return;

    track_install_file(src, dst, src.basename(), install_file(src, dst, src.basename()));
}

void
FSMerger::on_file_over_file(bool is_check, const FSPath & src, const FSPath & dst)
{
    if (is_check)
        return;

    if (config_protected(src, dst))
    {
        std::string cfgpro_name(make_config_protect_name(src, dst));
        track_install_file(src, dst, cfgpro_name, install_file(src, dst, cfgpro_name));
    }
    else
        track_install_file(src, dst, src.basename(), install_file(src, dst, src.basename()) + msi_unlinked_first);
}

void
FSMerger::on_file_over_dir(bool is_check, const FSPath & src, const FSPath & dst)
{
    on_error(is_check, "Cannot overwrite directory '" + stringify(dst / src.basename()) + "' with file '"
            + stringify(src) + "'");
}

void
FSMerger::on_file_over_sym(bool is_check, const FSPath & src, const FSPath & dst)
{
    if (is_check)
        return;

    track_install_file(src, dst, src.basename(), install_file(src, dst, src.basename()) + msi_unlinked_first);
}

void
FSMerger::on_file_over_misc(bool is_check, const FSPath & src, const FSPath & dst)
{
    if (is_check)
        return;

    track_install_file(src, dst, src.basename(), install_file(src, dst, src.basename()) + msi_unlinked_first);
}

void
FSMerger::on_dir_over_nothing(bool is_check, const FSPath & src, const FSPath & dst)
{
    if (is_check)
        return;

    track_install_dir(src, dst, install_dir(src, dst));
}

void
FSMerger::on_dir_over_file(bool is_check, const FSPath & src, const FSPath & dst)
{
    on_error(is_check, "Cannot overwrite file '" + stringify(dst / src.basename()) + "' with directory '"
            + stringify(src) + "'");
}

void
FSMerger::on_dir_over_dir(bool is_check, const FSPath & src, const FSPath & dst)
{
    if (is_check)
        return;

    track_install_dir(src, dst, { msi_used_existing });
}

void
FSMerger::on_dir_over_sym(bool is_check, const FSPath & src, const FSPath & dst)
{
    EntryType m;
    try
    {
        m = entry_type((dst / src.basename()).realpath());
    }
    catch (const FSError &)
    {
        m = et_nothing;
    }

    if (m == et_dir)
    {
        if (_imp->params.fs_merger_options()[fsmo_dir_over_sym_dir_is_error])
            on_error(is_check, "Expected '" + stringify(dst / src.basename()) +
                    "' to be a directory but found a symlink to a directory");
        else
            on_warn(is_check, "Expected '" + stringify(dst / src.basename()) +
                    "' to be a directory but found a symlink to a directory");

        if (! is_check)
            track_install_dir(src, dst, { msi_used_existing });
    }
    else
        on_error(is_check, "Expected '" + stringify(dst / src.basename()) +
                "' to be a directory but found a symlink to a non-directory");
}

void
FSMerger::on_dir_over_misc(bool is_check, const FSPath & src, const FSPath & dst)
{
    if (is_check)
        return;

    unlink_misc(dst / src.basename());
    track_install_dir(src, dst, install_dir(src, dst) + msi_unlinked_first);
}

void
FSMerger::on_sym_over_nothing(bool is_check, const FSPath & src, const FSPath & dst)
{
    if (is_check)
        return;

    track_install_sym(src, dst, install_sym(src, dst));
}

void
FSMerger::on_sym_over_file(bool is_check, const FSPath & src, const FSPath & dst)
{
    if (is_check)
        return;

    unlink_file(dst / src.basename());
    track_install_sym(src, dst, install_sym(src, dst) + msi_unlinked_first);
}

void
FSMerger::on_sym_over_dir(bool is_check, const FSPath & src, const FSPath & dst)
{
    on_error(is_check, "Cannot overwrite directory '" + stringify(dst / src.basename()) + "' with symlink '"
            + stringify(src) + "'");
}

void
FSMerger::on_sym_over_sym(bool is_check, const FSPath & src, const FSPath & dst)
{
    if (is_check)
        return;

    unlink_sym(dst / src.basename());
    track_install_sym(src, dst, install_sym(src, dst) + msi_unlinked_first);
}

void
FSMerger::on_sym_over_misc(bool is_check, const FSPath & src, const FSPath & dst)
{
    if (is_check)
        return;

    unlink_misc(dst / src.basename());
    track_install_sym(src, dst, install_sym(src, dst) + msi_unlinked_first);
}

FSMergerStatusFlags
FSMerger::install_file(const FSPath & src, const FSPath & dst_dir, const std::string & dst_name)
{
    Context context("When installing file '" + stringify(src) + "' to '" + stringify(dst_dir) + "' with protection '"
            + stringify(dst_name) + "':");

    FSPath dst_real(dst_dir / dst_name);

    if (_imp->params.should_merge() &&
            ! _imp->params.should_merge()(dst_real.strip_leading(_imp->params.root().realpath())))
        return { msi_unselected_part };

    FSMergerStatusFlags result;
    FSStat src_stat(src);
    FSStat dst_real_stat(dst_real);
    FSPath dst(dst_dir / (stringify(dst_name) + "|paludis-midmerge"));

    if (dst_real_stat.is_regular_file())
        dst_real.chmod(0);

    if (0 != _imp->params.environment()->perform_hook(extend_hook(
                         Hook("merger_install_file_pre")
                        ("INSTALL_SOURCE", stringify(src))
                        ("INSTALL_DESTINATION", stringify(dst_dir / src.basename()))
                        ("REAL_DESTINATION", stringify(dst_real))),
                _imp->params.maybe_output_manager()).max_exit_status())
        Log::get_instance()->message("merger.file.pre_hooks.failure", ll_warning, lc_context) <<
                "Merge of '" << src << "' to '" << dst_dir << "' pre hooks returned non-zero";

    std::shared_ptr<const SecurityContext> secctx(MatchPathCon::get_instance()->match(stringify(dst_real), src_stat.permissions()));
    FSCreateCon createcon(secctx);
    if (0 != paludis::setfilecon(src, secctx))
        throw FSMergerError(errno, "Could not set SELinux context on '"
                + stringify(src) + "'");

    mode_t src_perms(src_stat.permissions());
    if (0 != (src_perms & (S_ISVTX | S_ISUID | S_ISGID)))
        result += msi_setid_bits;

    bool do_copy(false);

    if ((! _imp->params.options()[mo_nondestructive]) &&
            0 == std::rename(stringify(src).c_str(), stringify(dst_real).c_str()))
    {
        result += msi_rename;

        bool touch(_imp->merged_ids.end() == _imp->merged_ids.find(src_stat.lowlevel_id()));
        _imp->merged_ids.insert(make_pair(src_stat.lowlevel_id(), stringify(dst_real)));

        FSPath d(stringify(dst_real));
        if (touch && ! _imp->params.options()[mo_preserve_mtimes])
            if (! d.utime(Timestamp::now()))
                throw FSMergerError(errno, "utime(" + stringify(dst_real) + ", 0) failed");

        /* set*id bits get partially clobbered on a rename on linux */
        dst_real.chmod(src_perms);
    }
    else
    {
        do_copy = true;
        std::pair<MergedMap::const_iterator, MergedMap::const_iterator> ii(_imp->merged_ids.equal_range(src_stat.lowlevel_id()));
        for (MergedMap::const_iterator i = ii.first ; i != ii.second ; ++i)
        {
            if (0 == ::link(i->second.c_str(), stringify(dst).c_str()))
            {
                if (0 != std::rename(stringify(dst).c_str(), stringify(dst_real).c_str()))
                    throw FSMergerError(errno, "rename(" + stringify(dst) + ", " + stringify(dst_real) + ") failed");
                do_copy = false;
                result += msi_as_hardlink;
                break;
            }
            Log::get_instance()->message("merger.file.link_failed", ll_debug, lc_context)
                    << "link(" << i->second << ", " << dst_real << ") failed: "
                    << std::strerror(errno);
        }
    }

    if (do_copy)
    {
        Log::get_instance()->message("merger.file.will_copy", ll_debug, lc_context) <<
            "rename/link failed: " << std::strerror(errno) << ". Falling back to regular read/write copy";

        FDHolder input_fd(::open(stringify(src).c_str(), O_RDONLY), false);
        if (-1 == input_fd)
            throw FSMergerError(errno, "Cannot read '" + stringify(src) + "'");

        FDHolder output_fd(::open(stringify(dst).c_str(), O_WRONLY | O_CREAT, src_perms), false);
        if (-1 == output_fd)
            throw FSMergerError(errno, "Cannot write '" + stringify(dst) + "'");

        if (! _imp->params.no_chown())
            if (0 != ::fchown(output_fd, src_stat.owner(), src_stat.group()))
                throw FSMergerError(errno, "Cannot fchown '" + stringify(dst) + "'");

#ifdef HAVE_FALLOCATE
        if (0 != ::fallocate(output_fd, FALLOC_FL_KEEP_SIZE, 0, src_stat.file_size()))
            switch (errno)
            {
                case EOPNOTSUPP:
                case ENOSYS:
                    break;

                case ENOSPC:
                    throw FSMergerError(errno, "fallocate '" + stringify(dst) + "' failed");

                default:
                    Log::get_instance()->message("merger.file.fallocate_failed", ll_debug, lc_context) <<
                        "fallocate '" + stringify(dst) + "' returned " + stringify(std::strerror(errno));
                    break;
            }
#endif

        /* set*id bits, after fallocate because xfs is weird */
        if (0 != ::fchmod(output_fd, src_perms))
            throw FSMergerError(errno, "Cannot fchmod '" + stringify(dst) + "'");
        try_to_copy_xattrs(src, output_fd, result);

        char buf[4096];
        ssize_t count;
        while ((count = read(input_fd, buf, 4096)) > 0)
            if (-1 == write(output_fd, buf, count))
                throw FSMergerError(errno, "write failed");
        if (-1 == count)
            throw FSMergerError(errno, "read failed");

        /* might need to copy mtime */
        if (_imp->params.options()[mo_preserve_mtimes])
        {
            Timestamp timestamp(src_stat.mtim());
            struct timespec ts[2];
            ts[0] = ts[1] = timestamp.as_timespec();
            if (0 != ::futimens(output_fd, ts))
                throw FSMergerError(errno, "Cannot futimens '" + stringify(dst) + "'");
        }

        if (0 != std::rename(stringify(dst).c_str(), stringify(dst_real).c_str()))
            throw FSMergerError(
                    "rename(" + stringify(dst) + ", " + stringify(dst_real) + ") failed: " + stringify(std::strerror(errno)));

        _imp->merged_ids.insert(make_pair(src_stat.lowlevel_id(), stringify(dst_real)));
    }

    if (fixed_ownership_for(src))
        result += msi_fixed_ownership;

    if (0 != _imp->params.environment()->perform_hook(extend_hook(
                         Hook("merger_install_file_post")
                         ("INSTALL_SOURCE", stringify(src))
                         ("INSTALL_DESTINATION", stringify(dst_dir / src.basename()))
                         ("REAL_DESTINATION", stringify(dst_real))),
                _imp->params.maybe_output_manager()).max_exit_status())
        Log::get_instance()->message("merger.file.post_hooks.failed", ll_warning, lc_context) <<
            "Merge of '" << src << "' to '" << dst_dir << "' post hooks returned non-zero";

    return result;
}

void
FSMerger::track_renamed_dir_recursive(const FSPath & dst)
{
    for (FSIterator d(dst, { fsio_include_dotfiles, fsio_inode_sort }), d_end ; d != d_end ; ++d)
    {
        FSMergerStatusFlags merged_how({ msi_parent_rename });
        if (fixed_ownership_for(_imp->params.image() / *d))
            merged_how += msi_fixed_ownership;
        EntryType m(entry_type(*d));
        switch (m)
        {
            case et_sym:
                rewrite_symlink_as_needed(*d, dst);
                track_install_sym(*d, dst, merged_how);
                _imp->merged_ids.insert(make_pair(d->stat().lowlevel_id(), stringify(*d)));
                continue;

            case et_file:
                {
                    FSStat d_star_stat(*d);
                    bool touch(_imp->merged_ids.end() == _imp->merged_ids.find(d_star_stat.lowlevel_id()));
                    _imp->merged_ids.insert(make_pair(d_star_stat.lowlevel_id(), stringify(*d)));

                    if (touch && ! _imp->params.options()[mo_preserve_mtimes])
                        if (! d->utime(Timestamp::now()))
                            throw FSMergerError(errno, "utime(" + stringify(*d) + ", 0) failed");
                    track_install_file(*d, dst, stringify(d->basename()), merged_how);
                }
                continue;

            case et_dir:
                track_install_dir(*d, d->dirname(), merged_how);
                track_renamed_dir_recursive(*d);
                continue;

            case et_misc:
                throw FSMergerError("Unexpected 'et_misc' entry found at: " + stringify(*d));

            case et_nothing:
            case last_et:
                break;
        }

        throw InternalError(PALUDIS_HERE, "Unexpected entry_type '" + stringify(m) + "'");
    }
}

void
FSMerger::relabel_dir_recursive(const FSPath & src, const FSPath & dst)
{
    for (FSIterator d(src, { fsio_include_dotfiles, fsio_inode_sort }), d_end ; d != d_end ; ++d)
    {
        FSStat d_star_stat(*d);

        mode_t mode(d_star_stat.permissions());
        std::shared_ptr<const SecurityContext> secctx(
                MatchPathCon::get_instance()->match(stringify(dst / d->basename()), mode));
        if (0 != paludis::setfilecon(*d, secctx))
            throw FSMergerError(errno, "Could not set SELinux context on '"
                    + stringify(*d) + "'");
        if (d_star_stat.is_directory())
            relabel_dir_recursive(*d, dst / d->basename());
    }
}

FSMergerStatusFlags
FSMerger::install_dir(const FSPath & src, const FSPath & dst_dir)
{
    Context context("When installing dir '" + stringify(src) + "' to '" + stringify(dst_dir) + "':");

    const FSPath dst(dst_dir / src.basename());
    FSMergerStatusFlags result;
    FSStat src_stat(src);

    if (! _imp->elided_paths.empty())
    {
        const auto path = dst.strip_leading(_imp->params.root());
        if (_imp->elided_paths.find(path) != _imp->elided_paths.end())
        {
            set_skipped_dir(true);
            return { msi_unselected_part };
        }
    }

    if (0 != _imp->params.environment()->perform_hook(extend_hook(
                         Hook("merger_install_dir_pre")
                         ("INSTALL_SOURCE", stringify(src))
                         ("INSTALL_DESTINATION", stringify(dst_dir / src.basename()))),
                _imp->params.maybe_output_manager()).max_exit_status())
        Log::get_instance()->message("merger.dir.pre_hooks.failure", ll_warning, lc_context)
            << "Merge of '" << src << "' to '" << dst_dir << "' pre hooks returned non-zero";

    mode_t mode(src_stat.permissions());
    if (0 != (mode & (S_ISVTX | S_ISUID | S_ISGID)))
        result += msi_setid_bits;

    std::shared_ptr<const SecurityContext> secctx(MatchPathCon::get_instance()->match(stringify(dst), mode));
    FSCreateCon createcon(secctx);
    if (0 != paludis::setfilecon(src, secctx))
        throw FSMergerError(errno, "Could not set SELinux context on '"
                + stringify(src) + "'");

    if (is_selinux_enabled())
        relabel_dir_recursive(src, dst);

    const bool partitioned = _imp->params.parts() &&
        _imp->params.parts()->is_partitioned(dst.strip_leading(_imp->params.root()));

    if (! partitioned && ! _imp->params.options()[mo_nondestructive] &&
            0 == std::rename(stringify(src).c_str(), stringify(dst).c_str()))
    {
        result += msi_rename;
        track_renamed_dir_recursive(dst);
        set_skipped_dir(true);
    }
    else
    {
        if (! partitioned && ! _imp->params.options()[mo_nondestructive])
            Log::get_instance()->message("merger.dir.rename_failed", ll_debug, lc_context)
                << "rename failed. Falling back to recursive copy.";

        dst.mkdir(mode, { fspmkdo_ok_if_exists });
        FDHolder dst_fd(::open(stringify(dst).c_str(), O_RDONLY));
        struct stat sb;
        if (-1 == dst_fd)
            throw FSMergerError(errno, "Could not get an FD for the directory '"
                    + stringify(dst) + "' that we just created");
        if (-1 == ::fstat(dst_fd, &sb))
            throw FSMergerError(errno, "Could not fstat the directory '"
                    + stringify(dst) + "' that we just created");
        if ( !S_ISDIR(sb.st_mode))
            throw FSMergerError("The directory that we just created is not a directory anymore");
        if (! _imp->params.no_chown())
            if (-1 == ::fchown(dst_fd, src_stat.owner(), src_stat.group()))
                throw FSMergerError(errno, "Could not fchown the directory '"
                        + stringify(dst) + "' that we just created");
        /* pick up set*id bits */
        ::fchmod(dst_fd, mode);
        try_to_copy_xattrs(src, dst_fd, result);
    }

    if (fixed_ownership_for(src))
        result += msi_fixed_ownership;

    if (0 != _imp->params.environment()->perform_hook(extend_hook(
                         Hook("merger_install_dir_post")
                         ("INSTALL_SOURCE", stringify(src))
                         ("INSTALL_DESTINATION", stringify(dst_dir / src.basename()))),
                _imp->params.maybe_output_manager()).max_exit_status())
        Log::get_instance()->message("merger.dir.post_hooks.failure", ll_warning, lc_context)
            << "Merge of '" << src << "' to '" << dst_dir << "' post hooks returned non-zero";

    return result;
}

FSMergerStatusFlags
FSMerger::install_sym(const FSPath & src, const FSPath & dst_dir)
{
    Context context("When installing sym '" + stringify(src) + "' to '" + stringify(dst_dir) + "':");

    FSPath dst(dst_dir / src.basename());

    if (_imp->params.should_merge() &&
            ! _imp->params.should_merge()(dst.strip_leading(_imp->params.root().realpath())))
        return { msi_unselected_part };

    FSMergerStatusFlags result;
    FSStat src_stat(src);

    if (0 != _imp->params.environment()->perform_hook(extend_hook(
                         Hook("merger_install_sym_pre")
                         ("INSTALL_SOURCE", stringify(src))
                         ("INSTALL_DESTINATION", stringify(dst))),
                _imp->params.maybe_output_manager()).max_exit_status())
        Log::get_instance()->message("merger.sym.pre_hooks.failure", ll_warning, lc_context)
            << "Merge of '" << src << "' to '" << dst_dir << "' pre hooks returned non-zero";

    if (0 != (src_stat.permissions() & (S_ISVTX | S_ISUID | S_ISGID)))
        result += msi_setid_bits;

    bool do_sym(true);

    FSCreateCon createcon(MatchPathCon::get_instance()->match(stringify(dst), S_IFLNK));
    std::pair<MergedMap::const_iterator, MergedMap::const_iterator> ii(
            _imp->merged_ids.equal_range(src_stat.lowlevel_id()));
    for (MergedMap::const_iterator i = ii.first ; i != ii.second ; ++i)
    {
        if (0 == ::link(i->second.c_str(), stringify(dst).c_str()))
        {
            do_sym = false;
            result += msi_as_hardlink;
            break;
        }
        Log::get_instance()->message("merger.sym.link_failed", ll_debug, lc_context)
            << "link(" << i->second + ", " << stringify(dst) << ") failed: "
            << std::strerror(errno);
    }

    if (do_sym)
    {
        if (0 != ::symlink(stringify(src.readlink()).c_str(), stringify(dst).c_str()))
            throw FSMergerError(errno, "Couldn't create symlink at '" + stringify(dst) + "'");
        _imp->merged_ids.insert(make_pair(src_stat.lowlevel_id(), stringify(dst)));
    }

    if (! _imp->params.no_chown())
    {
        dst.lchown(src_stat.owner(), src_stat.group());
        if (fixed_ownership_for(src))
            result += msi_fixed_ownership;
    }

    if (0 != _imp->params.environment()->perform_hook(extend_hook(
                         Hook("merger_install_sym_post")
                         ("INSTALL_SOURCE", stringify(src))
                         ("INSTALL_DESTINATION", stringify(dst))),
                _imp->params.maybe_output_manager()).max_exit_status())
        Log::get_instance()->message("merger.sym.post_hooks.failure", ll_warning, lc_context) <<
            "Merge of '" << src << "' to '" << dst_dir << "' post hooks returned non-zero";

    return result;
}

void
FSMerger::unlink_file(FSPath d)
{
    if (0 != _imp->params.environment()->perform_hook(extend_hook(
                         Hook("merger_unlink_file_pre")
                         ("UNLINK_TARGET", stringify(d))),
                _imp->params.maybe_output_manager()).max_exit_status())
        Log::get_instance()->message("merger.unlink_file.pre_hooks.failure", ll_warning, lc_context) <<
            "Unmerge of '" << d << "' pre hooks returned non-zero";

    d.chmod(0);
    d.unlink();

    if (0 != _imp->params.environment()->perform_hook(extend_hook(
                         Hook("merger_unlink_file_post")
                         ("UNLINK_TARGET", stringify(d))),
                _imp->params.maybe_output_manager()).max_exit_status())
        Log::get_instance()->message("merger.unlink_file.post_hooks.failure", ll_warning, lc_context) <<
            "Unmerge of '" << d << "' post hooks returned non-zero";
}

void
FSMerger::unlink_sym(FSPath d)
{
    if (0 != _imp->params.environment()->perform_hook(extend_hook(
                         Hook("merger_unlink_sym_pre")
                         ("UNLINK_TARGET", stringify(d))),
                _imp->params.maybe_output_manager()).max_exit_status())
        Log::get_instance()->message("merger.unlink_sym.pre_hooks.failure", ll_warning, lc_context) <<
            "Unmerge of '" << d << "' pre hooks returned non-zero";

    d.unlink();

    if (0 != _imp->params.environment()->perform_hook(extend_hook(
                         Hook("merger_unlink_sym_post")
                         ("UNLINK_TARGET", stringify(d))),
                _imp->params.maybe_output_manager()).max_exit_status())
        Log::get_instance()->message("merger.unlink_sym.post_hooks.failure", ll_warning, lc_context) <<
            "Unmerge of '" << d << "' post hooks returned non-zero";
}

void
FSMerger::unlink_dir(FSPath d)
{
    if (0 != _imp->params.environment()->perform_hook(extend_hook(
                         Hook("merger_unlink_dir_pre")
                         ("UNLINK_TARGET", stringify(d))),
                _imp->params.maybe_output_manager()).max_exit_status())
        Log::get_instance()->message("merger.unlink_dir.pre_hooks.failure", ll_warning, lc_context) <<
            "Unmerge of '" << d << "' pre hooks returned non-zero";

    d.rmdir();

    if (0 != _imp->params.environment()->perform_hook(extend_hook(
                         Hook("merger_unlink_dir_post")
                         ("UNLINK_TARGET", stringify(d))),
                _imp->params.maybe_output_manager()).max_exit_status())
        Log::get_instance()->message("merger.unlink_dir.post_hooks.failure", ll_warning, lc_context) <<
            "Unmerge of '" << d << "' post hooks returned non-zero";
}

void
FSMerger::unlink_misc(FSPath d)
{
    if (0 != _imp->params.environment()->perform_hook(extend_hook(
                         Hook("merger_unlink_misc_pre")
                         ("UNLINK_TARGET", stringify(d))),
                _imp->params.maybe_output_manager()).max_exit_status())
        Log::get_instance()->message("merger.unlink_misc.pre_hooks.failure", ll_warning, lc_context) <<
            "Unmerge of '" << d << "' pre hooks returned non-zero";

    d.unlink();

    if (0 != _imp->params.environment()->perform_hook(extend_hook(
                         Hook("merger_unlink_misc_post")
                         ("UNLINK_TARGET", stringify(d))),
                _imp->params.maybe_output_manager()).max_exit_status())
        Log::get_instance()->message("merger.unlink_misc.post_hooks.failure", ll_warning, lc_context) <<
            "Unmerge of '" << d << "' post hooks returned non-zero";
}

Hook
FSMerger::extend_hook(const Hook & h)
{
    return h
        ("ROOT", stringify(_imp->params.root()))
        ("IMAGE", stringify(_imp->params.image()));
}

#ifdef HAVE_XATTRS

void
FSMerger::try_to_copy_xattrs(const FSPath & src, int dst_fd, FSMergerStatusFlags & flags)
{
    FDHolder src_fd(::open(stringify(src).c_str(), O_RDONLY));

    ssize_t list_sz(flistxattr(src_fd, nullptr, 0));
    if (-1 == list_sz)
    {
        if (ENOTSUP != errno)
            Log::get_instance()->message("merger.xattrs.failure", ll_warning, lc_context) <<
                "Got error '" << std::strerror(errno) << "' when trying to find extended attributes size for '" << src << "'";
        return;
    }
    else if (list_sz > (2 << 29))
    {
        Log::get_instance()->message("merger.xattrs.your_fs_sucks", ll_warning, lc_context) <<
            "flistxattr returned " << list_sz << ", which clearly isn't right. Are you using some crazy "
            "ricer filesystem or kernel?";
        return;
    }

    std::shared_ptr<char> list_holder(static_cast<char *>(::operator new(list_sz)));
    list_sz = flistxattr(src_fd, list_holder.get(), list_sz);
    if (-1 == list_sz)
    {
        Log::get_instance()->message("merger.xattrs.failure", ll_warning, lc_context) <<
            "Got error '" << std::strerror(errno) << "' when trying to find extended attributes for '" << src << "'";
        return;
    }

    for (int offset(0) ; list_sz > 0 ; )
    {
        std::string key(list_holder.get() + offset);
        do
        {
            ssize_t value_sz(fgetxattr(src_fd, key.c_str(), nullptr, 0));
            if (-1 == value_sz)
            {
                Log::get_instance()->message("merger.xattrs.failure", ll_warning, lc_context) <<
                    "Got error '" << std::strerror(errno) << "' when trying to read size of extended attribute '" <<
                    key << "' for '" << src << "'";
                break;
            }

            std::shared_ptr<char> value_holder(static_cast<char *>(::operator new(value_sz)));

            if (key == "security.selinux")
            {
                /* we handle selinux stuff specially */
                continue;
            }

            value_sz = fgetxattr(src_fd, key.c_str(), value_holder.get(), value_sz);
            if (-1 == value_sz)
            {
                Log::get_instance()->message("merger.xattrs.failure", ll_warning, lc_context) <<
                    "Got error '" << std::strerror(errno) << "' when trying to read extended attribute '" <<
                    key << "' for '" << src << "'";
            }

            if (-1 == fsetxattr(dst_fd, key.c_str(), value_holder.get(), value_sz, 0))
            {
                if (ENOTSUP == errno)
                {
                    Log::get_instance()->message("merger.xattrs.failure", ll_warning, lc_context) <<
                        "Could not copy extended attributes from source file '" << src << "', discarding attribute '" <<
                        key << "' = '" << std::string(value_holder.get(), value_sz) << "'";
                    continue;
                }
                else
                    Log::get_instance()->message("merger.xattrs.failure", ll_warning, lc_context) <<
                        "Got error '" << std::strerror(errno) << "' when trying to set extended attribute '" <<
                        key << "' taken from source file '" << src << "'";
            }

            flags += msi_xattr;

        } while (false);

        list_sz -= (key.length() + 1);
        offset += (key.length() + 1);
    }
}

#else

void
FSMerger::try_to_copy_xattrs(const FSPath &, int, FSMergerStatusFlags &)
{
}

#endif

void
FSMerger::track_install_file(const FSPath & src, const FSPath & dst_dir, const std::string & dst_name, const FSMergerStatusFlags & flags)
{
    if (flags[msi_unselected_part])
        return display_merge(et_file, dst_dir / src.basename(), flags,
                             src.basename() == dst_name ? "" : dst_name);

    _imp->params.merged_entries()->insert(dst_dir / dst_name);
    record_install_file(src, dst_dir, dst_name, flags);
}

void
FSMerger::track_install_dir(const FSPath & src, const FSPath & dst_dir, const FSMergerStatusFlags & flags)
{
    if (flags[msi_unselected_part])
        return display_merge(et_dir, dst_dir / src.basename(), flags);

    _imp->params.merged_entries()->insert(dst_dir / src.basename());
    record_install_dir(src, dst_dir, flags);
}

void
FSMerger::track_install_under_dir(const FSPath & dst, const FSMergerStatusFlags & flags)
{
    _imp->params.merged_entries()->insert(dst);
    record_install_under_dir(dst, flags);
}

void
FSMerger::track_install_sym(const FSPath & src, const FSPath & dst_dir, const FSMergerStatusFlags & flags)
{
    if (flags[msi_unselected_part])
        return display_merge(et_sym, dst_dir / src.basename(), flags);

    _imp->params.merged_entries()->insert(dst_dir / src.basename());
    record_install_sym(src, dst_dir, flags);
}

void
FSMerger::on_file_main(bool is_check, const FSPath & src, const FSPath & dst)
{
    EntryType m(entry_type(dst / src.basename()));

    do
    {
        switch (m)
        {
            case et_nothing:
                on_file_over_nothing(is_check, src, dst);
                continue;

            case et_sym:
                on_file_over_sym(is_check, src, dst);
                continue;

            case et_dir:
                on_file_over_dir(is_check, src, dst);
                continue;

            case et_misc:
                on_file_over_misc(is_check, src, dst);
                continue;

            case et_file:
                on_file_over_file(is_check, src, dst);
                continue;

            case last_et:
                ;
        }

        throw InternalError(PALUDIS_HERE, "Unexpected entry_type '" + stringify(m) + "'");
    } while (false);
}

void
FSMerger::on_dir_main(bool is_check, const FSPath & src, const FSPath & dst)
{
    EntryType m(entry_type(dst / src.basename()));

    do
    {
        switch (m)
        {
            case et_nothing:
                on_dir_over_nothing(is_check, src, dst);
                continue;

            case et_sym:
                on_dir_over_sym(is_check, src, dst);
                continue;

            case et_dir:
                on_dir_over_dir(is_check, src, dst);
                continue;

            case et_misc:
                on_dir_over_misc(is_check, src, dst);
                continue;

            case et_file:
                on_dir_over_file(is_check, src, dst);
                continue;

            case last_et:
                ;
        }

        throw InternalError(PALUDIS_HERE, "Unexpected entry_type '" + stringify(m) + "'");

    } while (false);
}

void
FSMerger::on_enter_dir(bool is_check, const FSPath src)
{
    if (is_check || ! _imp->params.parts() || ! _imp->params.should_merge())
        return;

    for (FSIterator dentry(src, { fsio_want_directories }), invalid;
            dentry != invalid; ++dentry)
        if (_imp->is_elided_directory(*dentry))
            _imp->elided_paths.insert(dentry->strip_leading(_imp->params.image()));
}

void
FSMerger::on_sym_main(bool is_check, const FSPath & src, const FSPath & dst)
{
    EntryType m(entry_type(dst / src.basename()));

    do
    {
        switch (m)
        {
            case et_nothing:
                on_sym_over_nothing(is_check, src, dst);
                continue;

            case et_sym:
                on_sym_over_sym(is_check, src, dst);
                continue;

            case et_dir:
                on_sym_over_dir(is_check, src, dst);
                continue;

            case et_misc:
                on_sym_over_misc(is_check, src, dst);
                continue;

            case et_file:
                on_sym_over_file(is_check, src, dst);
                continue;

            case last_et:
                ;
        }

        throw InternalError(PALUDIS_HERE, "Unexpected entry_type '" + stringify(m) + "'");
    } while (false);
}

FSPath
FSMerger::canonicalise_root_path(const FSPath & f)
{
    return f.realpath();
}

void
FSMerger::do_dir_recursive(bool is_check, const FSPath & src, const FSPath & dst)
{
    FSStat dst_stat(dst);

    if ((! is_check) && (! dst_stat.is_directory()))
        throw MergerError("Destination directory '" + stringify(dst) + "' is not a directory");

    Merger::do_dir_recursive(is_check, src, dst);
}

std::string
FSMerger::make_arrows(const FSMergerStatusFlags & flags) const
{
    std::string result(">>>");

    for (EnumIterator<FSMergerStatusFlag> m, m_end(last_msi); m != m_end; ++m)
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

            case msi_unselected_part:
                result[0] = '%';
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

void
FSMerger::display_merge(const EntryType & type, const FSPath & path,
                        const FSMergerStatusFlags & flags,
                        const std::string & renamed) const
{
    const auto real_path(path.strip_leading(_imp->params.root().realpath()));
    std::ostringstream message;

    message << make_arrows(flags);

    switch (type)
    {
        case et_dir:
            message << " [dir] ";
            break;
        case et_file:
            message << " [obj] ";
            break;
        case et_sym:
            message << " [sym] ";
            break;

        case et_misc:
        case et_nothing:
        case last_et:
            throw FSMergerError("Unexpected entry type merged at: " + stringify(real_path));
    }

    message << stringify(real_path);
    if (!renamed.empty())
        message << " (" << renamed << ")";

    display_override(message.str());
}

