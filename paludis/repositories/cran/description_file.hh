/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_CRAN_DESCRIPTION_FILE_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_CRAN_DESCRIPTION_FILE_HH 1

#include <paludis/util/config_file.hh>
#include <paludis/util/attributes.hh>

namespace paludis
{
    namespace cranrepository
    {
        class PALUDIS_VISIBLE DescriptionFile :
            public ConfigFile,
            private Pimp<DescriptionFile>
        {
            private:
                void _line(const std::string &);

            public:
                DescriptionFile(const Source &);
                ~DescriptionFile();

                /**
                 * Fetch the value for a particular key, or an empty string.
                 */
                std::string get(const std::string &) const PALUDIS_ATTRIBUTE((warn_unused_result));
        };
    }
}

#endif
