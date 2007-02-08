/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
 * Copyright (c) 2006 David Morgan <david.morgan@wadham.oxford.ac.uk>
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

#ifndef PALUDIS_GUARD_PALUDIS_QA_ENVIRONMENT_HH
#define PALUDIS_GUARD_PALUDIS_QA_ENVIRONMENT_HH 1

#include <paludis/environment.hh>
#include <paludis/environment/no_config/no_config_environment.hh>
#include <paludis/package_database.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/fs_entry.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>

/** \file
 * Declarations for the QAEnvironment class.
 */

namespace paludis
{
    namespace qa
    {
        /**
         * The QAEnvironment is an Environment that corresponds to the environment
         * used by Qualudis for QA checks.
         */
        class QAEnvironment :
            public NoConfigEnvironment
        {
            public:
                /**
                 * Constructor.
                 *
                 * \deprecated Use the three arg form
                 */
                QAEnvironment(const FSEntry & base,
                        const FSEntry & write_cache) PALUDIS_ATTRIBUTE((deprecated));

                QAEnvironment(const FSEntry & base,
                        const FSEntry & write_cache = FSEntry("/var/empty"),
                        const FSEntry & master_repository_dir = FSEntry("/var/empty"));

                ~QAEnvironment();

                virtual std::string paludis_command() const;
        };
    }
}

#endif
