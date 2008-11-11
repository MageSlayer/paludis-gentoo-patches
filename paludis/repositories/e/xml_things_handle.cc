/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008 Ciaran McCreesh
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

#include <paludis/repositories/e/xml_things_handle.hh>
#include <paludis/about.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/log.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/instantiation_policy-impl.hh>
#include "config.h"
#include <dlfcn.h>
#include <stdint.h>

using namespace paludis;
using namespace paludis::erepository;

#define STUPID_CAST(type, val) reinterpret_cast<type>(reinterpret_cast<uintptr_t>(val))

namespace paludis
{
    template <>
    struct Implementation<XMLThingsHandle>
    {
        void * handle;

        bool available;
        std::tr1::shared_ptr<GLSA> (* create_glsa_from_xml_file) (const std::string &);
        std::tr1::shared_ptr<MetadataXML> (* create_metadata_xml_from_xml_file) (const FSEntry &);
        void (* init) ();
        void (* cleanup) ();

        Implementation() :
            handle(0),
            available(false),
            create_glsa_from_xml_file(0),
            create_metadata_xml_from_xml_file(0),
            init(0),
            cleanup(0)
        {
#if defined(ENABLE_GLSA) || defined(ENABLE_METADATA_XML)
            available = true;

            handle = ::dlopen(("libpaludiserepositoryxmlthings_" + stringify(PALUDIS_PC_SLOT) + ".so").c_str(), RTLD_NOW | RTLD_GLOBAL);
            if (! handle)
            {
                Log::get_instance()->message("e.xml_things.dlopen_failed", ll_warning, lc_context) << "Got error '"
                    << ::dlerror() << "' from dlopen for XML things";
                available = false;
            }

            if (available)
            {
                init = STUPID_CAST(void (*)(), ::dlsym(handle, "paludis_xml_things_init"));
                if (! init)
                {
                    Log::get_instance()->message("e.xml_things.dlsym_failed", ll_warning, lc_context) << "Got error '"
                        << ::dlerror() << "' from dlsym for init";
                    available = false;
                }
            }

            if (available)
            {
                cleanup = STUPID_CAST(void (*)(), ::dlsym(handle, "paludis_xml_things_cleanup"));
                if (! cleanup)
                {
                    Log::get_instance()->message("e.xml_things.dlsym_failed", ll_warning, lc_context) << "Got error '"
                        << ::dlerror() << "' from dlsym for cleanup";
                    available = false;
                }
            }

#  ifdef ENABLE_GLSA
            if (available)
            {
                create_glsa_from_xml_file = STUPID_CAST(std::tr1::shared_ptr<GLSA> (*)(const std::string &),
                        ::dlsym(handle, "paludis_xml_things_create_glsa_from_xml_file"));
                if (! create_glsa_from_xml_file)
                {
                    Log::get_instance()->message("e.xml_things.dlsym_failed", ll_warning, lc_context) << "Got error '"
                        << ::dlerror() << "' from dlsym for GLSA things";
                    available = false;
                }
            }
#  endif

#  ifdef ENABLE_METADATA_XML
            if (available)
            {
                create_metadata_xml_from_xml_file = STUPID_CAST(std::tr1::shared_ptr<MetadataXML> (*)(const FSEntry &),
                        ::dlsym(handle, "paludis_xml_things_create_metadata_xml_from_xml_file"));
                if (! create_metadata_xml_from_xml_file)
                {
                    Log::get_instance()->message("e.xml_things.dlsym_failed", ll_warning, lc_context) << "Got error '"
                        << ::dlerror() << "' from dlsym for metadata.xml things";
                    available = false;
                }
            }
#  endif

            if (available)
            {
                init();
            }
#endif
        }
    };
}

XMLThingsHandle::XMLThingsHandle() :
    PrivateImplementationPattern<XMLThingsHandle>(new Implementation<XMLThingsHandle>)
{
}

XMLThingsHandle::~XMLThingsHandle()
{
    if (0 != _imp->cleanup)
        _imp->cleanup();

    // don't dlclose, gcc does something dumb on cleanup
    // if (0 != _imp->handle)
    //    ::dlclose(_imp->handle);
}

bool
XMLThingsHandle::available() const
{
    return _imp->available;
}

std::tr1::shared_ptr<GLSA>
(* XMLThingsHandle::create_glsa_from_xml_file() const) (const std::string &)
{
    return _imp->create_glsa_from_xml_file;
}

std::tr1::shared_ptr<MetadataXML>
(* XMLThingsHandle::create_metadata_xml_from_xml_file() const) (const FSEntry &)
{
    return _imp->create_metadata_xml_from_xml_file;
}

template class PrivateImplementationPattern<XMLThingsHandle>;
template class InstantiationPolicy<XMLThingsHandle, instantiation_method::SingletonTag>;

