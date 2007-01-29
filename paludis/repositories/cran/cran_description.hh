/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Danny van Dyk <kugelfang@gentoo.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_CRAN_DESCRIPTION_HH
#define PALUDIS_GUARD_PALUDIS_CRAN_DESCRIPTION_HH 1

#include <paludis/version_metadata.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/log.hh>

namespace paludis
{
    /**
     * CRANDescription as used by CRANRepository and CRANInstalledRepository
     */
    struct PALUDIS_VISIBLE CRANDescription
    {
        /// Our package name.
        QualifiedPackageName name;

        /// Our package version.
        VersionSpec version;

        /// Our metadata, may be zero.
        std::tr1::shared_ptr<VersionMetadata> metadata;

        /// Our installed date.
        time_t installed_time;

        /// Turn a CRAN package name into a paludis package name.
        static void normalise_name(std::string & s)
        {
            std::replace_if(s.begin(), s.end(), std::bind2nd(std::equal_to<char>(), '.'), '-');
        }

        /// Turn a paludis package name into a CRAN package name.
        static void denormalise_name(std::string & s)
        {
            std::replace_if(s.begin(), s.end(), std::bind2nd(std::equal_to<char>(), '-'), '.');
        }

        /// Turn a CRAN package version into a paludis package version.
        static void normalise_version(std::string & s)
        {
            std::replace_if(s.begin(), s.end(), std::bind2nd(std::equal_to<char>(), '-'), '.');
        }

        /// Constructor
        CRANDescription(const std::string & n, const FSEntry & f, bool installed);

        /// Comparison operator
        bool operator< (const CRANDescription & other) const
        {
            if (name < other.name)
                return true;
            if (name > other.name)
                return false;
            if (version < other.version)
                return true;
            return false;
        }

        /**
         * Compare a CRANDescription by name only.
         *
         * \ingroup grpcrandesc
         */
        struct PALUDIS_VISIBLE ComparePackage
        {
            ///\name Comparison operators
            ///\{

            bool operator() (const QualifiedPackageName & c, const CRANDescription & d) const
            {
                return c < d.name;
            }

            bool operator() (const CRANDescription & d, const QualifiedPackageName & c) const
            {
                return d.name < c;
            }

            ///\}
        };

        /**
         * Compare a CRANIDescription by name and version.
         *
         * \ingroup grpcrandesc
         */
        struct PALUDIS_VISIBLE CompareVersion
        {
            ///\name Comparison operators
            ///\{

            bool operator() (const std::pair<QualifiedPackageName, VersionSpec> & c,
                    const CRANDescription & d) const
            {
                if (c.first < d.name)
                    return true;
                else if (c.first > d.name)
                    return false;
                else if (c.second < d.version)
                    return true;
                else
                    return false;
            }

            bool operator() (const CRANDescription & d,
                    const std::pair<QualifiedPackageName, VersionSpec> & c) const
            {
                if (d.name < c.first)
                    return true;
                else if (d.name > c.first)
                    return false;
                else if (d.version < c.second)
                    return true;
                else
                    return false;
            }

            ///\}
        };
    };
}

#endif
