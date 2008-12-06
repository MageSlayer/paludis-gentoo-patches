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

#ifndef PALUDIS_GUARD_PALUDIS_QA_HH
#define PALUDIS_GUARD_PALUDIS_QA_HH 1

#include <paludis/qa-fwd.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key-fwd.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/sequence-fwd.hh>
#include <paludis/util/set-fwd.hh>
#include <paludis/util/named_value.hh>

/** \file
 * Declarations for Repository QA classes.
 *
 * \ingroup g_repository
 *
 * \section Examples
 *
 * - \ref example_repository.cc "example_repository.cc"
 */

namespace paludis
{
    namespace n
    {
        struct associated_ids;
        struct associated_keys;
        struct entry;
        struct level;
        struct message;
        struct name;
    }

    /**
     * Parameters for a QA message.
     *
     * \see RepositoryQAInterface
     * \ingroup g_repository
     * \since 0.26
     * \nosubgrouping
     */
    struct PALUDIS_VISIBLE QAMessage
    {
        typedef Sequence<std::pair<std::tr1::shared_ptr<const PackageID>, std::tr1::shared_ptr<const MetadataKey> > > KeysSequence;

        NamedValue<n::associated_ids, std::tr1::shared_ptr<PackageIDSet> > associated_ids;
        NamedValue<n::associated_keys, std::tr1::shared_ptr<KeysSequence> > associated_keys;
        NamedValue<n::entry, FSEntry> entry;
        NamedValue<n::level, QAMessageLevel> level;
        NamedValue<n::message, std::string> message;
        NamedValue<n::name, std::string> name;

        static std::tr1::shared_ptr<PackageIDSet> default_associated_ids();
        static std::tr1::shared_ptr<KeysSequence> default_associated_keys();
        QAMessage & with_associated_id(const std::tr1::shared_ptr<const PackageID> &);
        QAMessage & with_associated_key(const std::tr1::shared_ptr<const PackageID> &, const std::tr1::shared_ptr<const MetadataKey> &);

        QAMessage(const FSEntry &, const QAMessageLevel &, const std::string & name, const std::string & message);
    };

    /**
     * A QAReporter subclass is passed to RepositoryQAInterface::check_qa to do
     * the reporting.
     *
     * \ingroup g_repository
     */
    class PALUDIS_VISIBLE QAReporter
    {
        public:
            virtual ~QAReporter() = 0;

            /**
             * Report a QA message.
             */
            virtual void message(const QAMessage &) = 0;

            /**
             * Update status.
             */
            virtual void status(const std::string &) = 0;
    };
}

#endif
