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

#include "unmerger.hh"
#include <paludis/environment.hh>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

using namespace paludis;

#include <paludis/merger/unmerger-sr.cc>

UnmergerError::UnmergerError(const std::string & s) throw () :
    Exception(s)
{
}

Unmerger::Unmerger(const UnmergerOptions & o) :
    _options(o)
{
}

Unmerger::~Unmerger()
{
}

void
Unmerger::unlink_file(FSEntry d)
{
    if (0 != _options.environment->perform_hook(extend_hook(
                         Hook("unmerger_unlink_file_pre")
                         ("UNLINK_TARGET", stringify(d)))))
        throw UnmergerError("Unmerge of '" + stringify(d) + "' aborted by hook");

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
                         Hook("unmerger_unlink_file_post")
                         ("UNLINK_TARGET", stringify(d)))))
        throw UnmergerError("Unmerge of '" + stringify(d) + "' aborted by hook");
}

void
Unmerger::unlink_sym(FSEntry d)
{
    if (0 != _options.environment->perform_hook(extend_hook(
                         Hook("unmerger_unlink_sym_pre")
                         ("UNLINK_TARGET", stringify(d)))))
        throw UnmergerError("Unmerge of '" + stringify(d) + "' aborted by hook");

    d.unlink();

    if (0 != _options.environment->perform_hook(extend_hook(
                         Hook("unmerger_unlink_sym_post")
                         ("UNLINK_TARGET", stringify(d)))))
        throw UnmergerError("Unmerge of '" + stringify(d) + "' aborted by hook");
}

void
Unmerger::unlink_dir(FSEntry d)
{
    if (0 != _options.environment->perform_hook(extend_hook(
                         Hook("unmerger_unlink_dir_pre")
                         ("UNLINK_TARGET", stringify(d)))))
        throw UnmergerError("Unmerge of '" + stringify(d) + "' aborted by hook");

    d.rmdir();

    if (0 != _options.environment->perform_hook(extend_hook(
                         Hook("unmerger_unlink_dir_post")
                         ("UNLINK_TARGET", stringify(d)))))
        throw UnmergerError("Unmerge of '" + stringify(d) + "' aborted by hook");
}

void
Unmerger::unlink_misc(FSEntry d)
{
    if (0 != _options.environment->perform_hook(extend_hook(
                         Hook("unmerger_unlink_misc_pre")
                         ("UNLINK_TARGET", stringify(d)))))
        throw UnmergerError("Unmerge of '" + stringify(d) + "' aborted by hook");

    d.unlink();

    if (0 != _options.environment->perform_hook(extend_hook(
                         Hook("unmerger_unlink_misc_post")
                         ("UNLINK_TARGET", stringify(d)))))
        throw UnmergerError("Unmerge of '" + stringify(d) + "' aborted by hook");
}

Hook
Unmerger::extend_hook(const Hook & h)
{
    return h
        ("ROOT", stringify(_options.root));
}


