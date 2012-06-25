/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2012 Ciaran McCreesh
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

#include <iostream>
#include <sstream>
#include <string>
#include <cstdlib>

int main(int, char * argv[])
{
    char buf[512];
    uint64_t records_to_skip(0);
    int zero_records(0);
    while (std::cin.read(buf, 512))
    {
        int bufn(std::cin.gcount());
        if (bufn < 512)
        {
            std::cout.write(buf, bufn);
            std::cerr << argv[0] << ": Unexpectedly short tar file" << std::endl;
            return EXIT_FAILURE;
        }

        if (records_to_skip > 0)
        {
            --records_to_skip;
            std::cout.write(buf, 512);
            continue;
        }

        if (std::string::npos == std::string(buf, 512).find_first_not_of(std::string("\0", 1)))
        {
            if (zero_records < 2)
                std::cout.write(buf, 512);
            ++zero_records;
            continue;
        }

        if (zero_records > 0)
        {
            if (zero_records >= 2)
            {
                if (std::string(buf, 8) == std::string("\xa6\x4c\x32\x2e\xd6\x14\xae\xdb"))
                    std::cerr << argv[0] << ": Found trailing corruption from pixz after "
                        << zero_records << " zero records, discarding it" << std::endl;
                else
                    std::cerr << argv[0] << ": Found trailing corruption in an unrecognised format after "
                        << zero_records << " zero records, discarding it" << std::endl;
            }
            else
                std::cerr << argv[0] << ": Found trailing corruption after a single zero record, discarding it" << std::endl;

            while (std::cin.read(buf, 512))
            {
                /* nothing */
            }

            return EXIT_SUCCESS;
        }

        zero_records = 0;

        switch (buf[156])
        {
            case '2': case '3': case '4': case '5': case '6':
                records_to_skip = 0;
                break;

            case 'S':
                {
                    std::string s(&buf[124], 12);
                    std::stringstream ss(s);
                    if (! (ss >> std::oct >> records_to_skip))
                    {
                        std::cerr << argv[0] << ": Unable to determine how many records to skip from '" << s << "'" << std::endl;
                        return EXIT_FAILURE;
                    }

                    if (buf[482]) /* isextended */
                    {
                        do
                        {
                            std::cout.write(buf, 512);
                            std::cin.read(buf, 512);
                        } while (buf[504]); /* isextended */
                    }

                    records_to_skip = (records_to_skip + 511) / 512;
                }

            default:
                {
                    std::string s(&buf[124], 12);
                    std::stringstream ss(s);
                    if (! (ss >> std::oct >> records_to_skip))
                    {
                        std::cerr << argv[0] << ": Unable to determine how many records to skip from '" << s << "'" << std::endl;
                        return EXIT_FAILURE;
                    }

                    records_to_skip = (records_to_skip + 511) / 512;
                }
        }

        std::cout.write(buf, 512);
    }

    if (zero_records < 2)
        std::cerr << argv[0] << ": tar file does not include an EOF marker" << std::endl;

    return EXIT_SUCCESS;
}


