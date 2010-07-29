/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010 Ciaran McCreesh
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

#include <paludis/tar_extras.hh>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <archive.h>
#include <archive_entry.h>

struct PaludisTarExtras
{
    struct archive * archive;
};

extern "C"
PaludisTarExtras *
paludis_tar_extras_init(const std::string & f)
{
    auto extras(new PaludisTarExtras);
    extras->archive = archive_write_new();
    archive_write_set_compression_none(extras->archive);
    archive_write_set_format_pax(extras->archive);
    archive_write_open_filename(extras->archive, f.c_str());
    return extras;
}

extern "C"
void
paludis_tar_extras_add_file(PaludisTarExtras * const extras, const std::string & from, const std::string & path)
{
    struct archive * disk_archive(archive_read_disk_new());
    archive_read_disk_set_standard_lookup(disk_archive);

    struct archive_entry * entry(archive_entry_new());

    int fd(open(from.c_str(), O_RDONLY));

    archive_entry_copy_pathname(entry, path.c_str());
    archive_read_disk_entry_from_file(disk_archive, entry, fd, 0);
    archive_write_header(extras->archive, entry);

    int bytes_read;
    char buf[4096];
    while ((bytes_read = read(fd, buf, sizeof(buf))) > 0)
        archive_write_data(extras->archive, buf, bytes_read);

    close(fd);

    archive_write_finish_entry(extras->archive);
    archive_read_finish(disk_archive);
    archive_entry_free(entry);
}

extern "C"
void
paludis_tar_extras_add_sym(PaludisTarExtras * const extras, const std::string & from, const std::string & path, const std::string & dest)
{
    struct archive_entry * entry(archive_entry_new());

    struct stat st;
    lstat(from.c_str(), &st);

    archive_entry_copy_pathname(entry, path.c_str());
    archive_entry_copy_stat(entry, &st);
    archive_entry_copy_symlink(entry, dest.c_str());
    archive_write_header(extras->archive, entry);

    archive_write_finish_entry(extras->archive);
    archive_entry_free(entry);
}

extern "C"
void
paludis_tar_extras_cleanup(PaludisTarExtras * const extras)
{
    archive_write_close(extras->archive);
    archive_write_finish(extras->archive);
    delete extras;
}

