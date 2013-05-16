/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2013 Ciaran McCreesh
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

#include <paludis/repositories/e/parse_uri_label.hh>
#include <paludis/repositories/e/dep_parser.hh>
#include <paludis/repositories/e/eapi.hh>
#include <paludis/util/log.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/singleton-impl.hh>
#include <paludis/dep_label.hh>
#include <paludis/dep_spec.hh>
#include <map>
#include <set>

using namespace paludis;
using namespace paludis::erepository;

namespace
{
    typedef std::tuple<std::string, std::string, std::string> URILabelsIndex;

    struct URILabelsStore :
        Singleton<URILabelsStore>
    {
        std::mutex mutex;
        std::map<URILabelsIndex, std::shared_ptr<URILabel> > store;

        std::shared_ptr<URILabel> make(const std::string & class_name, const std::string & text)
        {
            if (class_name == "URIMirrorsThenListedLabel")
                return std::make_shared<URIMirrorsThenListedLabel>(text);
            else if (class_name == "URIMirrorsOnlyLabel")
                return std::make_shared<URIMirrorsOnlyLabel>(text);
            else if (class_name == "URIListedOnlyLabel")
                return std::make_shared<URIListedOnlyLabel>(text);
            else if (class_name == "URIListedThenMirrorsLabel")
                return std::make_shared<URIListedThenMirrorsLabel>(text);
            else if (class_name == "URILocalMirrorsOnlyLabel")
                return std::make_shared<URILocalMirrorsOnlyLabel>(text);
            else if (class_name == "URIManualOnlyLabel")
                return std::make_shared<URIManualOnlyLabel>(text);
            else
                throw EDepParseError(text, "Label '" + text + "' maps to unknown class '" + class_name + "'");
        }

        std::shared_ptr<URILabel> get(const std::string & eapi_name, const std::string & class_name, const std::string & text)
        {
            std::unique_lock<std::mutex> lock(mutex);
            URILabelsIndex x{eapi_name, class_name, text};

            auto i(store.find(x));
            if (i == store.end())
                i = store.insert(std::make_pair(x, make(class_name, text))).first;
            return i->second;
        }
    };
}

std::shared_ptr<URILabelsDepSpec>
paludis::erepository::parse_uri_label(const std::string & s, const EAPI & e)
{
    Context context("When parsing label string '" + s + "' using EAPI '" + e.name() + "':");

    if (s.empty())
        throw EDepParseError(s, "Empty label");

    std::string c(e.supported()->uri_labels()->class_for_label(s.substr(0, s.length() - 1)));
    if (c.empty())
        throw EDepParseError(s, "Unknown label");

    std::shared_ptr<URILabelsDepSpec> l(std::make_shared<URILabelsDepSpec>());
    l->add_label(URILabelsStore::get_instance()->get(e.name(), c, s.substr(0, s.length() - 1)));

    return l;
}


