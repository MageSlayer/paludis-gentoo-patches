/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh
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

#include <paludis/util/exception.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/stringify.hh>

namespace paludis
{
    std::string strip_leading_string(const std::string & s, const std::string & prefix)
    {
        try
        {
            if (0 == s.compare(0, prefix.length(), prefix))
                return s.substr(prefix.length());
            else
                return s;
        }
        catch (const std::exception & e)
        {
            throw InternalError(PALUDIS_HERE, "Caught unexpected exception " +
                    stringify(e.what()));
        }
    }

    std::string strip_leading(const std::string & s, const std::string & remove)
    {
        try
        {
            std::string::size_type p(s.find_first_not_of(remove));
            if (std::string::npos == p)
                return std::string();
            else
                return s.substr(p);
        }
        catch (const std::exception & e)
        {
            throw InternalError(PALUDIS_HERE, "Caught unexpected exception " +
                    stringify(e.what()));
        }
    }

    std::string strip_trailing_string(const std::string & s, const std::string & suffix)
    {
        try
        {
            if (suffix.length() > s.length())
                return s;
            else if (0 == s.compare(s.length() - suffix.length(), suffix.length(), suffix))
                return s.substr(0, s.length() - suffix.length());
            else
                return s;
        }
        catch (const std::exception & e)
        {
            throw InternalError(PALUDIS_HERE, "Caught unexpected exception " +
                    stringify(e.what()) + " with s='" + s + "', suffix='" + suffix + "'");
        }
    }

    std::string strip_trailing(const std::string & s, const std::string & remove)
    {
        try
        {
            std::string::size_type p(s.find_last_not_of(remove));
            if (std::string::npos == p)
                return std::string();
            else
                return s.substr(0, p + 1);
        }
        catch (const std::exception & e)
        {
            throw InternalError(PALUDIS_HERE, "Caught unexpected exception " +
                    stringify(e.what()));
        }
    }
}

