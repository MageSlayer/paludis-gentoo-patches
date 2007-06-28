/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <paludis/digests/md5.hh>
#include <iostream>
#include <cstdlib>
#include <fstream>

int
main(int argc, char * argv[])
{
    if (argc >= 3)
    {
        std::cerr << "usage: " << argv[0] << " [filename]" << std::endl;
        return EXIT_FAILURE;
    }

    if (argc == 2)
    {
        std::ifstream f(argv[1]);
        if (! f)
        {
            std::cerr << argv[0] << ": could not open '" << argv[1] << "'" << std::endl;
            return EXIT_FAILURE;
        }
        paludis::MD5 sum(f);
        std::cout << sum.hexsum() << std::endl;
    }
    else
    {
        paludis::MD5 sum(std::cin);
        std::cout << sum.hexsum() << std::endl;
    }

}



