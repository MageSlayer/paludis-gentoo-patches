/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <paludis/package_database_entry.hh>
#include <paludis/qa/qa_environment.hh>
#include <paludis/util/collection_concrete.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/log.hh>
#include <paludis/config_file.hh>
#include <map>

using namespace paludis;
using namespace paludis::qa;

namespace paludis
{
    namespace qa
    {
#include <paludis/qa/qa_environment-sr.hh>
#include <paludis/qa/qa_environment-sr.cc>
    }

    template<>
    struct Implementation<QAEnvironmentBase> :
        InternalCounted<QAEnvironmentBase>
    {
        std::vector<PackageDatabasesEntry> package_databases;

        Implementation(const FSEntry & base, const Environment * const env)
        {
            Context context("When creating package databases from profiles.desc under '"
                    + stringify(base / "profiles") + "':");

            LineConfigFile profiles_desc(base / "profiles" / "profiles.desc");
            for (LineConfigFile::Iterator line(profiles_desc.begin()), line_end(profiles_desc.end()) ;
                    line != line_end ; ++line)
            {
                std::vector<std::string> tokens;
                WhitespaceTokeniser::get_instance()->tokenise(*line, std::back_inserter(tokens));

                if (tokens.size() != 3)
                {
                    Log::get_instance()->message(ll_warning, lc_context, "Skipping invalid line '"
                            + *line + "'");
                    continue;
                }

                PackageDatabase::Pointer db(new PackageDatabase(env));

                /* create our portage repository */
                AssociativeCollection<std::string, std::string>::Pointer keys(
                        new AssociativeCollection<std::string, std::string>::Concrete);

                keys->insert("format", "portage");
                keys->insert("importace", "1");
                keys->insert("location", stringify(base));
                keys->insert("cache", "/var/empty");
                keys->insert("profiles", stringify(base / "profiles" / tokens.at(1)));

                db->add_repository(RepositoryMaker::get_instance()->find_maker("portage")(env,
                            db.raw_pointer(), keys));

                /* create our virtuals repository */
                db->add_repository(RepositoryMaker::get_instance()->find_maker("virtuals")(env,
                            db.raw_pointer(), AssociativeCollection<std::string, std::string>::Pointer(0)));

                /* make the entry */
                package_databases.push_back(PackageDatabasesEntry(PackageDatabasesEntry::create()
                            .arch(UseFlagName(tokens.at(0)))
                            .location(base / "profiles" / tokens.at(1))
                            .status(tokens.at(2))
                            .package_database(db)));
            }

            if (package_databases.empty())
                throw ProfilesDescError("No profiles.desc entries found");
        }
    };
}

QAEnvironmentBase::QAEnvironmentBase(const FSEntry & b, const Environment * const env) :
    PrivateImplementationPattern<QAEnvironmentBase>(new Implementation<QAEnvironmentBase>(b, env))
{
}

QAEnvironmentBase::~QAEnvironmentBase()
{
}

QAEnvironment::QAEnvironment(const FSEntry & base) :
    QAEnvironmentBase(base, this),
    Environment(_imp->package_databases.begin()->package_database)
{
}

QAEnvironment::~QAEnvironment()
{
}

std::string
QAEnvironment::paludis_command() const
{
    return "diefunc 'qa_environment.cc' 'QAEnvironment::paludis_command()' "
        "'paludis_command called from within QAEnvironment'";
}

ProfilesDescError::ProfilesDescError(const std::string & our_message) throw () :
    ConfigurationError("Bad profiles.desc: " + our_message)
{
}

