/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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

#include "check_metadata.hh"
#include "qa_notice.hh"

using namespace paludis;

bool
check_metadata(const Environment * const env, const PackageDatabaseEntry & e)
{
    bool ok(true);
    VersionMetadata::ConstPointer metadata(0);

    do
    {
        try
        {
            metadata = env->package_database()->fetch_metadata(e);
            if ("UNKNOWN" == metadata->get(vmk_eapi))
            {
                *QANotices::get_instance() << QANotice(qal_fatal, stringify(e),
                        "Couldn't generate metadata");
                ok = false;
                break;
            }
        }
        catch (const paludis::Exception & x)
        {
            *QANotices::get_instance() << QANotice(qal_fatal, stringify(e),
                    "Error generating metadata: " + x.message());
            ok = false;
            break;
        }

        try
        {
            DepParser::parse(metadata->get(vmk_depend));
        }
        catch (const paludis::Exception & x)
        {
            *QANotices::get_instance() << QANotice(qal_fatal, stringify(e),
                    "Error parsing DEPEND: " + x.message());
            ok = false;
        }

        try
        {
            DepParser::parse(metadata->get(vmk_rdepend));
        }
        catch (const paludis::Exception & x)
        {
            *QANotices::get_instance() << QANotice(qal_fatal, stringify(e),
                    "Error parsing RDEPEND: " + x.message());
            ok = false;
        }

        try
        {
            DepParser::parse(metadata->get(vmk_pdepend));
        }
        catch (const paludis::Exception & x)
        {
            *QANotices::get_instance() << QANotice(qal_fatal, stringify(e),
                    "Error parsing PDEPEND: " + x.message());
            ok = false;
        }

        if (! ok)
            break;

        try
        {
            DepParserOptions opts;
            opts.set(dpo_qualified_package_names);
            DepParser::parse(metadata->get(vmk_provide), opts);
        }
        catch (const paludis::Exception & x)
        {
            *QANotices::get_instance() << QANotice(qal_fatal, stringify(e),
                    "Error parsing PROVIDE: " + x.message());
            ok = false;
        }

        try
        {
            std::set<UseFlagName> iuse(metadata->begin_iuse(), metadata->end_iuse());
        }
        catch (const paludis::Exception & x)
        {
            *QANotices::get_instance() << QANotice(qal_fatal, stringify(e),
                    "Error parsing IUSE: " + x.message());
            ok = false;
        }

        try
        {
            std::set<KeywordName> keywords(metadata->begin_keywords(), metadata->end_keywords());

            if (keywords.empty())
                *QANotices::get_instance() << QANotice(qal_minor, stringify(e),
                        "KEYWORDS is empty");
        }
        catch (const paludis::Exception & x)
        {
            *QANotices::get_instance() << QANotice(qal_fatal, stringify(e),
                    "Error parsing KEYWORDS: " + x.message());
            ok = false;
        }

    } while (false);

    return ok;
}

