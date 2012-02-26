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

#include <paludis/tar_extras.hh>
#include <paludis/merger.hh>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <archive.h>
#include <archive_entry.h>

#include "config.h"

using namespace paludis;

struct PaludisTarExtras
{
    struct archive * archive;
    struct archive_entry_linkresolver * linkresolver;
};

extern "C"
PaludisTarExtras *
paludis_tar_extras_init(const std::string & f, const std::string & compress)
{
    auto extras(new PaludisTarExtras);
    extras->archive = archive_write_new();

    if (! extras->archive)
        throw MergerError("archive_write_new returned null");

    if (compress == "bz2")
        archive_write_set_compression_bzip2(extras->archive);
    else
        archive_write_set_compression_none(extras->archive);

#ifdef LIBARCHIVE_DOES_GNUTAR
    archive_write_set_format_gnutar(extras->archive);
#else
    archive_write_set_format_pax(extras->archive);
#endif

    if (ARCHIVE_OK != archive_write_open_filename(extras->archive, f.c_str()))
        throw MergerError("archive_write_open_filename failed");

    extras->linkresolver = archive_entry_linkresolver_new();

    if (extras->linkresolver == NULL)
        throw MergerError("archive_entry_linkresolver_new failed");

    archive_entry_linkresolver_set_strategy(extras->linkresolver, archive_format(extras->archive));

    return extras;
}

extern "C"
void
paludis_tar_extras_add_file(PaludisTarExtras * const extras, const std::string & from, const std::string & path)
{
    struct archive * disk_archive(archive_read_disk_new());
    if (! disk_archive)
        throw MergerError("archive_read_disk_new failed");

    archive_read_disk_set_standard_lookup(disk_archive);

    struct archive_entry * entry(archive_entry_new());
    struct archive_entry * sparse(archive_entry_new());

    int fd(open(from.c_str(), O_RDONLY));

    archive_entry_copy_pathname(entry, path.c_str());
    if (ARCHIVE_OK != archive_read_disk_entry_from_file(disk_archive, entry, fd, 0))
        throw MergerError("archive_read_disk_entry_from_file failed");

    archive_entry_linkify(extras->linkresolver, &entry, &sparse);

    if (ARCHIVE_OK != archive_write_header(extras->archive, entry))
        throw MergerError("archive_write_header failed");

    if (archive_entry_size(entry) > 0)
    {
        int bytes_read;
        char buf[4096];
        while ((bytes_read = read(fd, buf, sizeof(buf))) > 0)
            if (bytes_read != archive_write_data(extras->archive, buf, bytes_read))
                throw MergerError("archive_write_data failed");

        close(fd);

        if (ARCHIVE_OK != archive_write_finish_entry(extras->archive))
            throw MergerError("archive_write_finish_entry failed");

        if (ARCHIVE_OK != archive_read_finish(disk_archive))
            throw MergerError("archive_read_finish failed");
    }

    archive_entry_free(entry);
}

extern "C"
void
paludis_tar_extras_add_sym(PaludisTarExtras * const extras, const std::string & from, const std::string & path, const std::string & dest)
{
    struct archive_entry * entry(archive_entry_new());
    if (! entry)
        throw MergerError("archive_entry_new returned null");

    struct stat st;
    if (0 != lstat(from.c_str(), &st))
        throw MergerError("lstat failed");

    archive_entry_copy_pathname(entry, path.c_str());
    archive_entry_copy_stat(entry, &st);
    archive_entry_copy_symlink(entry, dest.c_str());
    archive_entry_set_size(entry, dest.length());
    if (ARCHIVE_OK != archive_write_header(extras->archive, entry))
        throw MergerError("archive_write_header failed");

    if (ARCHIVE_OK != archive_write_finish_entry(extras->archive))
        throw MergerError("archive_write_finish_entry failed");

    archive_entry_free(entry);
}

extern "C"
void
paludis_tar_extras_cleanup(PaludisTarExtras * const extras)
{
    if (ARCHIVE_OK != archive_write_close(extras->archive))
        throw MergerError("archive_write_close failed");
    if (ARCHIVE_OK != archive_write_finish(extras->archive))
        throw MergerError("archive_write_finish failed");
    archive_entry_linkresolver_free(extras->linkresolver);
    delete extras;
}

