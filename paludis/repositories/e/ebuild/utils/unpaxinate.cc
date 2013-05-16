/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include <iostream>
#include <string>
#include <cstdlib>

#include <archive.h>
#include <archive_entry.h>
#include <unistd.h>

#include "config.h"

int main(int argc, char *argv[])
{
    if (argc != 4)
        return EXIT_FAILURE;

    bool want_env(false);
    if (argv[1] == std::string("env"))
        want_env = true;
    else if (argv[1] == std::string("img"))
        want_env = false;
    else
        return EXIT_FAILURE;

    std::string archive_file(argv[2]);

    if (0 != chdir(argv[3]))
        return EXIT_FAILURE;

    archive * archive(archive_read_new());
    archive_read_support_filter_all(archive);
    archive_read_support_format_all(archive);

    int x;

    if (ARCHIVE_OK != ((x = archive_read_open_filename(archive, archive_file.c_str(), 10240))))
    {
        std::cerr << "Could not open '" << archive_file << "': libarchive returned " <<
            x << ", archive_errno " << archive_errno(archive) << ": " << archive_error_string(archive) << std::endl;
        return EXIT_FAILURE;
    }

    archive_entry * entry;
    bool done_any(false);

    while (ARCHIVE_OK == archive_read_next_header(archive, &entry))
    {
        std::string archive_filename(archive_entry_pathname(entry));
        std::string out_filename;
        int flags = 0;

        if (archive_filename == "PBIN/environment" && want_env)
        {
            out_filename = "environment";
            flags = ARCHIVE_EXTRACT_TIME;
        }
        else if ((! want_env) && (0 != archive_filename.compare(0, 5, "PBIN/", 0, 5)))
        {
            out_filename = archive_filename;
            flags = ARCHIVE_EXTRACT_OWNER | ARCHIVE_EXTRACT_PERM | ARCHIVE_EXTRACT_TIME | ARCHIVE_EXTRACT_ACL |
                ARCHIVE_EXTRACT_FFLAGS | ARCHIVE_EXTRACT_XATTR | ARCHIVE_EXTRACT_SECURE_NODOTDOT;
        }
        else
        {
            archive_read_data_skip(archive);
            continue;
        }

        std::cout << out_filename << std::endl;
        archive_entry_set_pathname(entry, out_filename.c_str());

        if (ARCHIVE_OK != ((x = archive_read_extract(archive, entry, flags))))
        {
            std::cerr << "Could not extract '" << out_filename << "' from '" << archive_file << "': libarchive returned " <<
                 x << ", archive_errno " << archive_errno(archive) << ": " << archive_error_string(archive) << std::endl;
            return EXIT_FAILURE;
        }
        done_any = true;
    }

    if (ARCHIVE_OK != ((x = archive_read_free(archive))))
    {
        std::cerr << "Could not finish reading '" << archive_file << "': libarchive returned " <<
            x << ", archive_errno " << archive_errno(archive) << ": " << archive_error_string(archive) << std::endl;
        return EXIT_FAILURE;
    }

    if (want_env && ! done_any)
    {
        std::cerr << "Could not find any environment in '" << archive_file << "'" << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

