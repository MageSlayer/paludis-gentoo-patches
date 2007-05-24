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

#include "merger.hh"
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/fd_holder.hh>
#include <paludis/util/log.hh>
#include <paludis/selinux/security_context.hh>
#include <paludis/environment.hh>
#include <paludis/hook.hh>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

using namespace paludis;

#include <paludis/merger/merger-sr.cc>

MergerError::MergerError(const std::string & s) throw () :
    Exception(s)
{
}

Merger::Merger(const MergerOptions & o) :
    _options(o),
    _result(true),
    _skip_dir(false)
{
}

Merger::~Merger()
{
}

bool
Merger::check()
{
    Context context("When checking merge from '" + stringify(_options.image) + "' to '"
            + stringify(_options.root) + "':");

    if (0 != _options.environment->perform_hook(extend_hook(
                         Hook("merger_check_pre")
                         ("INSTALL_SOURCE", stringify(_options.image))
                         ("INSTALL_DESTINATION", stringify(_options.root)))).max_exit_status)
        make_check_fail();

    do_dir_recursive(true, _options.image, _options.root);

    if (0 != _options.environment->perform_hook(extend_hook(
                         Hook("merger_check_post")
                         ("INSTALL_SOURCE", stringify(_options.image))
                         ("INSTALL_DESTINATION", stringify(_options.root)))).max_exit_status)
        make_check_fail();

    return _result;
}

void
Merger::make_check_fail()
{
    _result = false;
}

void
Merger::merge()
{
    Context context("When performing merge from '" + stringify(_options.image) + "' to '"
            + stringify(_options.root) + "':");

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

    if (0 != _options.environment->perform_hook(extend_hook(
                         Hook("merger_install_pre")
                         ("INSTALL_SOURCE", stringify(_options.image))
                         ("INSTALL_DESTINATION", stringify(_options.root)))).max_exit_status)
        Log::get_instance()->message(ll_warning, lc_context,
                "Merge of '" + stringify(_options.image) + "' to '" + stringify(_options.root) + "' pre hooks returned non-zero");

    do_dir_recursive(false, _options.image, _options.root);

    if (0 != _options.environment->perform_hook(extend_hook(
                         Hook("merger_install_post")
                         ("INSTALL_SOURCE", stringify(_options.image))
                         ("INSTALL_DESTINATION", stringify(_options.root)))).max_exit_status)
        Log::get_instance()->message(ll_warning, lc_context,
                "Merge of '" + stringify(_options.image) + "' to '" + stringify(_options.root) + "' post hooks returned non-zero");
}

EntryType
Merger::entry_type(const FSEntry & f)
{
    Context context("When checking type of '" + stringify(f) + "':");

    if (! f.exists())
        return et_nothing;

    if (f.is_symbolic_link())
        return et_sym;

    if (f.is_regular_file())
        return et_file;

    if (f.is_directory())
        return et_dir;

    return et_misc;
}

void
Merger::do_dir_recursive(bool is_check, const FSEntry & src, const FSEntry & dst)
{
    Context context("When " + stringify(is_check ? "checking" : "performing") + " merge from '" +
            stringify(src) + "' to '" + stringify(dst) + "':");

    if (! src.is_directory())
        throw MergerError("Source directory '" + stringify(src) + "' is not a directory");
    if ((! is_check) && (! dst.is_directory()))
        throw MergerError("Destination directory '" + stringify(src) + "' is not a directory");

    on_enter_dir(is_check, src);

    for (DirIterator d(src, false), d_end ; d != d_end ; ++d)
    {
        EntryType m(entry_type(*d));
        switch (m)
        {
            case et_sym:
                on_sym(is_check, *d, dst);
                continue;

            case et_file:
                on_file(is_check, *d, dst);
                continue;

            case et_dir:
                on_dir(is_check, *d, dst);
                if (_result)
                {
                    if (! _skip_dir)
                        do_dir_recursive(is_check, *d,
                                is_check ? (dst / d->basename()) : (dst / d->basename()).realpath());
                    else
                        _skip_dir = false;
                }
                continue;

            case et_misc:
                on_misc(is_check, *d, dst);
                continue;

            case et_nothing:
            case last_et:
                ;
        }

        throw InternalError(PALUDIS_HERE, "Unexpected entry_type '" + stringify(m) + "'");
    }

    on_leave_dir(is_check, src);
}

void
Merger::on_file(bool is_check, const FSEntry & src, const FSEntry & dst)
{
    Context context("When handling file '" + stringify(src) + "' to '" + stringify(dst) + "':");

    EntryType m(entry_type(dst / src.basename()));

    if (is_check &&
        0 != _options.environment->perform_hook(extend_hook(
                         Hook("merger_check_file_pre")
                         ("INSTALL_SOURCE", stringify(src))
                         ("INSTALL_DESTINATION", stringify(dst / src.basename())))).max_exit_status)
        make_check_fail();

    if (! is_check)
    {
        HookResult hr(_options.environment->perform_hook(extend_hook(
                        Hook("merger_install_file_override")
                        ("INSTALL_SOURCE", stringify(src))
                        ("INSTALL_DESTINATION", stringify(dst / src.basename()))
                        .grab_output(Hook::AllowedOutputValues()("skip")))));

        if (hr.max_exit_status != 0)
            Log::get_instance()->message(ll_warning, lc_context) << "Merge of '"
                << stringify(src) << "' to '" << stringify(dst) << "' skip hooks returned non-zero";
        else if (hr.output == "skip")
        {
            std::string tidy(stringify((dst / src.basename()).strip_leading(_options.root)));
            display_override("--- [skp] " + tidy);
            return;
        }
    }

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

    if (is_check &&
        0 != _options.environment->perform_hook(extend_hook(
                         Hook("merger_check_file_post")
                         ("INSTALL_SOURCE", stringify(src))
                         ("INSTALL_DESTINATION", stringify(dst / src.basename())))).max_exit_status)
        make_check_fail();
}

void
Merger::on_dir(bool is_check, const FSEntry & src, const FSEntry & dst)
{
    Context context("When handling dir '" + stringify(src) + "' to '" + stringify(dst) + "':");

    EntryType m(entry_type(dst / src.basename()));

    if (is_check &&
        0 != _options.environment->perform_hook(extend_hook(
                         Hook("merger_check_dir_pre")
                         ("INSTALL_SOURCE", stringify(src))
                         ("INSTALL_DESTINATION", stringify(dst / src.basename())))).max_exit_status)
        make_check_fail();

    if (! is_check)
    {
        HookResult hr(_options.environment->perform_hook(extend_hook(
                        Hook("merger_install_dir_override")
                        ("INSTALL_SOURCE", stringify(src))
                        ("INSTALL_DESTINATION", stringify(dst / src.basename()))
                        .grab_output(Hook::AllowedOutputValues()("skip")))));

        if (hr.max_exit_status != 0)
            Log::get_instance()->message(ll_warning, lc_context) << "Merge of '"
                << stringify(src) << "' to '" << stringify(dst) << "' skip hooks returned non-zero";
        else if (hr.output == "skip")
        {
            std::string tidy(stringify((dst / src.basename()).strip_leading(_options.root)));
            display_override("--- [skp] " + tidy);
            _skip_dir = true;
            return;
        }
    }

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

    if (is_check &&
        0 != _options.environment->perform_hook(extend_hook(
                         Hook("merger_check_dir_post")
                         ("INSTALL_SOURCE", stringify(src))
                         ("INSTALL_DESTINATION", stringify(dst / src.basename())))).max_exit_status)
        make_check_fail();
}

void
Merger::on_sym(bool is_check, const FSEntry & src, const FSEntry & dst)
{
    Context context("When handling sym '" + stringify(src) + "' to '" + stringify(dst) + "':");

    EntryType m(entry_type(dst / src.basename()));

    if (is_check &&
        0 != _options.environment->perform_hook(extend_hook(
                         Hook("merger_check_sym_post")
                         ("INSTALL_SOURCE", stringify(src))
                         ("INSTALL_DESTINATION", stringify(dst / src.basename())))).max_exit_status)
        make_check_fail();

    if (! is_check)
    {
        HookResult hr(_options.environment->perform_hook(extend_hook(
                        Hook("merger_install_sym_override")
                        ("INSTALL_SOURCE", stringify(src))
                        ("INSTALL_DESTINATION", stringify(dst / src.basename()))
                        .grab_output(Hook::AllowedOutputValues()("skip")))));

        if (hr.max_exit_status != 0)
            Log::get_instance()->message(ll_warning, lc_context) << "Merge of '"
                << stringify(src) << "' to '" << stringify(dst) << "' skip hooks returned non-zero";
        else if (hr.output == "skip")
        {
            std::string tidy(stringify((dst / src.basename()).strip_leading(_options.root)));
            display_override("--- [skp] " + tidy);
            return;
        }
    }

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

    if (is_check &&
        0 != _options.environment->perform_hook(extend_hook(
                         Hook("merger_check_sym_post")
                         ("INSTALL_SOURCE", stringify(src))
                         ("INSTALL_DESTINATION", stringify(dst / src.basename())))).max_exit_status)
        make_check_fail();
}

void
Merger::on_misc(bool is_check, const FSEntry & src, const FSEntry & dst)
{
    Context context("When handling misc '" + stringify(src) + "' to '" + stringify(dst) + "':");

    on_error(is_check, "Cannot write '" + stringify(src) + "' to '" + stringify(dst) +
            "' because it is not a recognised file type");
}

void
Merger::on_enter_dir(bool, const FSEntry)
{
}

void
Merger::on_leave_dir(bool, const FSEntry)
{
}

void
Merger::on_file_over_nothing(bool is_check, const FSEntry & src, const FSEntry & dst)
{
    if (is_check)
        return;

    install_file(src, dst, src.basename());
    record_install_file(src, dst, src.basename());
}

void
Merger::on_file_over_file(bool is_check, const FSEntry & src, const FSEntry & dst)
{
    if (is_check)
        return;

    if (config_protected(src, dst))
    {
        std::string cfgpro_name(make_config_protect_name(src, dst));
        install_file(src, dst, cfgpro_name);
        record_install_file(src, dst, cfgpro_name);
    }
    else
    {
        unlink_file(dst / src.basename());
        install_file(src, dst, src.basename());
        record_install_file(src, dst, src.basename());
    }
}

void
Merger::on_file_over_dir(bool is_check, const FSEntry & src, const FSEntry & dst)
{
    on_error(is_check, "Cannot overwrite directory '" + stringify(dst / src.basename()) + "' with file '"
            + stringify(src) + "'");
}

void
Merger::on_file_over_sym(bool is_check, const FSEntry & src, const FSEntry & dst)
{
    if (is_check)
        return;

    unlink_sym(dst / src.basename());
    install_file(src, dst, src.basename());
    record_install_file(src, dst, src.basename());
}

void
Merger::on_file_over_misc(bool is_check, const FSEntry & src, const FSEntry & dst)
{
    if (is_check)
        return;

    unlink_misc(dst / src.basename());
    install_file(src, dst, src.basename());
    record_install_file(src, dst, src.basename());
}

void
Merger::on_dir_over_nothing(bool is_check, const FSEntry & src, const FSEntry & dst)
{
    if (is_check)
        return;

    install_dir(src, dst);
    record_install_dir(src, dst);
}

void
Merger::on_dir_over_file(bool is_check, const FSEntry & src, const FSEntry & dst)
{
    on_error(is_check, "Cannot overwrite file '" + stringify(dst / src.basename()) + "' with directory '"
            + stringify(src) + "'");
}

void
Merger::on_dir_over_dir(bool is_check, const FSEntry & src, const FSEntry & dst)
{
    if (is_check)
        return;

    record_install_dir(src, dst);
}

void
Merger::on_dir_over_sym(bool is_check, const FSEntry & src, const FSEntry & dst)
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
        on_warn(is_check, "Expected '" + stringify(dst / src.basename()) +
                "' to be a directory but found a symlink to a directory");
        if (! is_check)
            record_install_dir(src, dst);
    }
    else
        on_error(is_check, "Expected '" + stringify(dst / src.basename()) +
                "' to be a directory but found a symlink to a non-directory");
}

void
Merger::on_dir_over_misc(bool is_check, const FSEntry & src, const FSEntry & dst)
{
    if (is_check)
        return;

    unlink_misc(dst / src.basename());
    install_dir(src, dst);
    record_install_dir(src, dst);
}

void
Merger::on_sym_over_nothing(bool is_check, const FSEntry & src, const FSEntry & dst)
{
    if (is_check)
        return;

    install_sym(src, dst);
    record_install_sym(src, dst);
}

void
Merger::on_sym_over_file(bool is_check, const FSEntry & src, const FSEntry & dst)
{
    if (is_check)
        return;

    unlink_file(dst / src.basename());
    install_sym(src, dst);
    record_install_sym(src, dst);
}

void
Merger::on_sym_over_dir(bool is_check, const FSEntry & src, const FSEntry & dst)
{
    on_error(is_check, "Cannot overwrite directory '" + stringify(dst / src.basename()) + "' with symlink '"
            + stringify(src) + "'");
}

void
Merger::on_sym_over_sym(bool is_check, const FSEntry & src, const FSEntry & dst)
{
    if (is_check)
        return;

    unlink_sym(dst / src.basename());
    install_sym(src, dst);
    record_install_sym(src, dst);
}

void
Merger::on_sym_over_misc(bool is_check, const FSEntry & src, const FSEntry & dst)
{
    if (is_check)
        return;

    unlink_misc(dst / src.basename());
    install_sym(src, dst);
    record_install_sym(src, dst);
}

void
Merger::install_file(const FSEntry & src, const FSEntry & dst_dir, const std::string & dst_name)
{
    Context context("When installing file '" + stringify(src) + "' to '" + stringify(dst_dir) + "' with protection '"
            + stringify(dst_name) + "':");

    if (0 != _options.environment->perform_hook(extend_hook(
                         Hook("merger_install_file_pre")
                        ("INSTALL_SOURCE", stringify(src))
                        ("INSTALL_DESTINATION", stringify(dst_dir / src.basename())))).max_exit_status)
        Log::get_instance()->message(ll_warning, lc_context,
                "Merge of '" + stringify(src) + "' to '" + stringify(dst_dir) + "' pre hooks returned non-zero");

    FSCreateCon createcon(MatchPathCon::get_instance()->match(stringify(dst_dir/dst_name), src.permissions()));
    FDHolder input_fd(::open(stringify(src).c_str(), O_RDONLY), false);
    if (-1 == input_fd)
        throw MergerError("Cannot read '" + stringify(src) + "'");

    FDHolder output_fd(::open(stringify(dst_dir / dst_name).c_str(), O_WRONLY | O_CREAT,
                src.permissions()), false);
    if (-1 == output_fd)
        throw MergerError("Cannot write '" + stringify(dst_dir / dst_name) + "'");

    if (! _options.no_chown)
        if (0 != ::fchown(output_fd,
                    src.owner() == _options.environment->reduced_uid() ? 0 : src.owner(),
                    src.group() == _options.environment->reduced_gid() ? 0 : src.group()))
            throw MergerError("Cannot fchown '" + stringify(dst_dir / dst_name) + "'");

    /* set*id bits */
    if (0 != ::fchmod(output_fd, src.permissions()))
        throw MergerError("Cannot fchmod '" + stringify(dst_dir / dst_name) + "'");

    char buf[4096];
    ssize_t count;
    while ((count = read(input_fd, buf, 4096)) > 0)
        write(output_fd, buf, count);

    if (0 != _options.environment->perform_hook(extend_hook(
                         Hook("merger_install_file_post")
                         ("INSTALL_SOURCE", stringify(src))
                         ("INSTALL_DESTINATION", stringify(dst_dir / src.basename())))).max_exit_status)
        Log::get_instance()->message(ll_warning, lc_context,
                "Merge of '" + stringify(src) + "' to '" + stringify(dst_dir) + "' post hooks returned non-zero");
}

void
Merger::install_dir(const FSEntry & src, const FSEntry & dst_dir)
{
    Context context("When installing dir '" + stringify(src) + "' to '" + stringify(dst_dir) + "':");

    if (0 != _options.environment->perform_hook(extend_hook(
                         Hook("merger_install_dir_pre")
                         ("INSTALL_SOURCE", stringify(src))
                         ("INSTALL_DESTINATION", stringify(dst_dir / src.basename())))).max_exit_status)
        Log::get_instance()->message(ll_warning, lc_context,
                "Merge of '" + stringify(src) + "' to '" + stringify(dst_dir) + "' pre hooks returned non-zero");

    mode_t mode(src.permissions());
    FSEntry dst(dst_dir / src.basename());
    FSCreateCon createcon(MatchPathCon::get_instance()->match(stringify(dst), mode));
    dst.mkdir(mode);
    if (! _options.no_chown)
        dst.chown(
                src.owner() == _options.environment->reduced_uid() ? 0 : src.owner(),
                src.group() == _options.environment->reduced_gid() ? 0 : src.group());
    /* pick up set*id bits */
    dst.chmod(src.permissions());

    if (0 != _options.environment->perform_hook(extend_hook(
                         Hook("merger_install_dir_post")
                         ("INSTALL_SOURCE", stringify(src))
                         ("INSTALL_DESTINATION", stringify(dst_dir / src.basename())))).max_exit_status)
        Log::get_instance()->message(ll_warning, lc_context,
                "Merge of '" + stringify(src) + "' to '" + stringify(dst_dir) + "' post hooks returned non-zero");
}

void
Merger::install_sym(const FSEntry & src, const FSEntry & dst_dir)
{
    Context context("When installing sym '" + stringify(src) + "' to '" + stringify(dst_dir) + "':");

    if (0 != _options.environment->perform_hook(extend_hook(
                         Hook("merger_install_sym_pre")
                         ("INSTALL_SOURCE", stringify(src))
                         ("INSTALL_DESTINATION", stringify(dst_dir / src.basename())))).max_exit_status)
        Log::get_instance()->message(ll_warning, lc_context,
                "Merge of '" + stringify(src) + "' to '" + stringify(dst_dir) + "' pre hooks returned non-zero");

    FSCreateCon createcon(MatchPathCon::get_instance()->match(stringify(dst_dir / src.basename()), S_IFLNK));
    if (0 != ::symlink(stringify(src.readlink()).c_str(), stringify(dst_dir / src.basename()).c_str()))
        throw MergerError("Couldn't create symlink at '" + stringify(dst_dir / src.basename()) + "'");

    if (0 != _options.environment->perform_hook(extend_hook(
                         Hook("merger_install_sym_post")
                         ("INSTALL_SOURCE", stringify(src))
                         ("INSTALL_DESTINATION", stringify(dst_dir / src.basename())))).max_exit_status)
        Log::get_instance()->message(ll_warning, lc_context,
                "Merge of '" + stringify(src) + "' to '" + stringify(dst_dir) + "' post hooks returned non-zero");
}

void
Merger::unlink_file(FSEntry d)
{
    if (0 != _options.environment->perform_hook(extend_hook(
                         Hook("merger_unlink_file_pre")
                         ("UNLINK_TARGET", stringify(d)))).max_exit_status)
        Log::get_instance()->message(ll_warning, lc_context,
                "Unmerge of '" + stringify(d) + "' pre hooks returned non-zero");

    if (d.is_regular_file())
    {
        mode_t mode(d.permissions());
        if ((mode & S_ISUID) || (mode & S_ISGID))
        {
            mode &= 0400;
            d.chmod(mode);
        }
    }

    d.unlink();

    if (0 != _options.environment->perform_hook(extend_hook(
                         Hook("merger_unlink_file_post")
                         ("UNLINK_TARGET", stringify(d)))).max_exit_status)
        Log::get_instance()->message(ll_warning, lc_context,
                "Unmerge of '" + stringify(d) + "' post hooks returned non-zero");
}

void
Merger::unlink_sym(FSEntry d)
{
    if (0 != _options.environment->perform_hook(extend_hook(
                         Hook("merger_unlink_sym_pre")
                         ("UNLINK_TARGET", stringify(d)))).max_exit_status)
        Log::get_instance()->message(ll_warning, lc_context,
                "Unmerge of '" + stringify(d) + "' pre hooks returned non-zero");

    d.unlink();

    if (0 != _options.environment->perform_hook(extend_hook(
                         Hook("merger_unlink_sym_post")
                         ("UNLINK_TARGET", stringify(d)))).max_exit_status)
        Log::get_instance()->message(ll_warning, lc_context,
                "Unmerge of '" + stringify(d) + "' post hooks returned non-zero");
}

void
Merger::unlink_dir(FSEntry d)
{
    if (0 != _options.environment->perform_hook(extend_hook(
                         Hook("merger_unlink_dir_pre")
                         ("UNLINK_TARGET", stringify(d)))).max_exit_status)
        Log::get_instance()->message(ll_warning, lc_context,
                "Unmerge of '" + stringify(d) + "' pre hooks returned non-zero");

    d.rmdir();

    if (0 != _options.environment->perform_hook(extend_hook(
                         Hook("merger_unlink_dir_post")
                         ("UNLINK_TARGET", stringify(d)))).max_exit_status)
        Log::get_instance()->message(ll_warning, lc_context,
                "Unmerge of '" + stringify(d) + "' post hooks returned non-zero");
}

void
Merger::unlink_misc(FSEntry d)
{
    if (0 != _options.environment->perform_hook(extend_hook(
                         Hook("merger_unlink_misc_pre")
                         ("UNLINK_TARGET", stringify(d)))).max_exit_status)
        Log::get_instance()->message(ll_warning, lc_context,
                "Unmerge of '" + stringify(d) + "' pre hooks returned non-zero");

    d.unlink();

    if (0 != _options.environment->perform_hook(extend_hook(
                         Hook("merger_unlink_misc_post")
                         ("UNLINK_TARGET", stringify(d)))).max_exit_status)
        Log::get_instance()->message(ll_warning, lc_context,
                "Unmerge of '" + stringify(d) + "' post hooks returned non-zero");
}

Hook
Merger::extend_hook(const Hook & h)
{
    return h
        ("ROOT", stringify(_options.root))
        ("IMAGE", stringify(_options.image));
}

