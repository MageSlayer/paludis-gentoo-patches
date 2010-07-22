/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_ENVIRONMENTS_PALUDIS_MIRRORS_CONF_HH
#define PALUDIS_GUARD_PALUDIS_ENVIRONMENTS_PALUDIS_MIRRORS_CONF_HH 1

#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/config_file.hh>
#include <paludis/environment.hh>
#include <paludis/name.hh>

namespace paludis
{
    class PaludisEnvironment;

    namespace paludis_environment
    {
        /**
         * Represents the mirrors.conf file, which may be composed of multiple 'real' files.
         *
         * \ingroup grppaludisenvironment
         * \nosubgrouping
         */
        class MirrorsConf :
            private PrivateImplementationPattern<MirrorsConf>
        {
            public:
                ///\name Basic operations
                ///\{

                MirrorsConf(const PaludisEnvironment * const);
                ~MirrorsConf();

                MirrorsConf(const MirrorsConf &) = delete;
                MirrorsConf & operator= (const MirrorsConf &) = delete;

                ///\}

                /**
                 * Add another file.
                 */
                void add(const FSEntry &);

                /**
                 * Query a mirror.
                 */
                std::tr1::shared_ptr<const MirrorsSequence> query(const std::string &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));
        };
    }
}

#endif
