/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 David Leverton
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_VDB_CONTENTS_TOKENISER_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_VDB_CONTENTS_TOKENISER_HH

#include <string>

namespace paludis
{
    namespace erepository
    {
        class VDBContentsTokeniser
        {
            private:
                VDBContentsTokeniser();

            public:
                template <typename Iter_>
                static bool tokenise(const std::string &, Iter_);
        };

        template <typename Iter_>
        bool
        VDBContentsTokeniser::tokenise(const std::string & s, Iter_ iter)
        {
            static const std::string space(" \t\r\n");

            std::string::size_type type_begin(s.find_first_not_of(space));
            if (std::string::npos == type_begin)
                return false;

            std::string::size_type type_end(s.find_first_of(space, type_begin + 1));
            // need at least one character for the filename after the
            // whitespace
            if (std::string::npos == type_end || s.length() <= type_end + 1)
                return false;
            // skip the whitespace
            std::string::size_type filename_begin(type_end + 1);

            std::string type(s.substr(type_begin, type_end - type_begin));
            int extra_fields(0);
            if ("obj" == type)
                extra_fields = 2;
            else if ("sym" == type)
                extra_fields = 1;

            std::string::size_type filename_end(s.length());
            for (int x(0); x < extra_fields; ++x)
            {
                // filename_end is exclusive, but the second argument
                // to find_last_not_of is inclusive
                std::string::size_type extra_end(s.find_last_not_of(space, filename_end - 1));
                if (std::string::npos == extra_end || extra_end <= filename_begin)
                    return false;
                // filename_end will point /at/ the delimeter space,
                // which is fine because it's exclusive
                filename_end = s.find_last_of(space, extra_end);
                if (std::string::npos == filename_end || filename_end <= filename_begin)
                    return false;
            }

            if ("sym" == type)
            {
                // need at least one character each for the symlink
                // name itself and the target
                std::string::size_type arrow_begin(s.find(" -> ", filename_begin + 1));
                if (std::string::npos == arrow_begin || arrow_begin >= filename_end - 4)
                    return false;

                *iter++ = type;
                *iter++ = s.substr(filename_begin, arrow_begin - filename_begin);
                *iter++ = s.substr(arrow_begin + 4, filename_end - (arrow_begin + 4));
            }
            else
            {
                *iter++ = type;
                *iter++ = s.substr(filename_begin, filename_end - filename_begin);
            }

            // none of these finds should fail because we already
            // counted the extra fields above
            std::string::size_type pos(filename_end + 1);
            for (int x(0); x < extra_fields; ++x)
            {
                std::string::size_type extra_begin(s.find_first_not_of(space, pos));
                std::string::size_type extra_end(s.find_first_of(space, extra_begin + 1));
                *iter++ = s.substr(extra_begin, extra_end - extra_begin);
                pos = extra_end + 1;
            }

            return true;
        }
    }
}

#endif
