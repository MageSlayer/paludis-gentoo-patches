/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2010, 2011, 2013 Ciaran McCreesh
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

#include <paludis/repositories/e/metadata_xml.hh>
#include <paludis/repositories/e/xml_things_handle.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/singleton-impl.hh>
#include <paludis/util/map-impl.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/log.hh>
#include <paludis/util/fs_path.hh>
#include <paludis/util/fs_stat.hh>
#include <paludis/literal_metadata_key.hh>
#include <paludis/choice.hh>
#include <unordered_map>

using namespace paludis;
using namespace paludis::erepository;

typedef std::unordered_map<FSPath, std::shared_ptr<MetadataXML>, Hash<FSPath> > Store;

namespace paludis
{
    template <>
    struct Imp<MetadataXMLPool>
    {
        mutable std::mutex mutex;
        mutable Store store;
    };
}

MetadataXMLPool::MetadataXMLPool() :
    _imp()
{
}

MetadataXMLPool::~MetadataXMLPool() = default;

const std::shared_ptr<const MetadataXML>
MetadataXMLPool::metadata_if_exists(const FSPath & f) const
{
    Context context("When handling metadata.xml file '" + stringify(f) + "':");

    FSPath f_real(f.realpath_if_exists());
    std::unique_lock<std::mutex> lock(_imp->mutex);
    Store::const_iterator i(_imp->store.find(f_real));
    if (i != _imp->store.end())
        return i->second;
    else
    {
        std::shared_ptr<MetadataXML> metadata_xml;
        if (f_real.stat().is_regular_file_or_symlink_to_regular_file())
        {
            try
            {
                if (XMLThingsHandle::get_instance()->create_metadata_xml_from_xml_file())
                    metadata_xml = XMLThingsHandle::get_instance()->create_metadata_xml_from_xml_file()(f);
            }
            catch (const Exception & e)
            {
                Log::get_instance()->message("e.metadata_xml.bad", ll_warning, lc_context) << "Got exception '"
                    << e.message() << "' (" << e.what() << "), ignoring metadata.xml file '" << f_real << "'";
            }
        }
        return _imp->store.insert(std::make_pair(f_real, metadata_xml)).first->second;
    }
}

namespace paludis
{
    template class PALUDIS_VISIBLE Map<ChoiceNameWithPrefix, std::string>;
    template class Pimp<MetadataXMLPool>;
    template class Singleton<MetadataXMLPool>;
}
