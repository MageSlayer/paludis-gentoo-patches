/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2010 Ciaran McCreesh
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
#include <paludis/util/singleton-impl.hh>
#include <paludis/util/system.hh>
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
        void * paludis_handle;
        void * handle;

        typedef void (* InitPtr) ();
        typedef void (* CleanupPtr) ();

        XMLThingsHandle::CreateGLSAFromXMLFilePtr create_glsa_from_xml_file;
        XMLThingsHandle::CreateMetadataXMLFromXMLFilePtr create_metadata_xml_from_xml_file;
        InitPtr init;
        CleanupPtr cleanup;

        Implementation() :
            paludis_handle(0),
            handle(0),
            create_glsa_from_xml_file(0),
            create_metadata_xml_from_xml_file(0),
            init(0),
            cleanup(0)
        {
#if ENABLE_XML
            if (! getenv_with_default("PALUDIS_NO_XML", "").empty())
                return;

            paludis_handle = ::dlopen(("libpaludis_" + stringify(PALUDIS_PC_SLOT) + ".so").c_str(), RTLD_NOW | RTLD_GLOBAL);
            if (! paludis_handle)
            {
                Log::get_instance()->message("e.xml_things.dlopen_failed", ll_warning, lc_context) << "Got error '"
                    << ::dlerror() << "' from dlopen libpaludis";
                return;
            }

            handle = ::dlopen(("libpaludiserepositoryxmlthings_" + stringify(PALUDIS_PC_SLOT) + ".so").c_str(), RTLD_NOW | RTLD_GLOBAL);
            if (! handle)
            {
                Log::get_instance()->message("e.xml_things.dlopen_failed", ll_warning, lc_context) << "Got error '"
                    << ::dlerror() << "' from dlopen for XML things";
                return;
            }

            InitPtr i(STUPID_CAST(InitPtr, ::dlsym(handle, "paludis_xml_things_init")));
            if (! i)
            {
                Log::get_instance()->message("e.xml_things.dlsym_failed", ll_warning, lc_context) << "Got error '"
                                                                                                    << ::dlerror() << "' from dlsym for init";
                return;
            }

            CleanupPtr c(STUPID_CAST(CleanupPtr, ::dlsym(handle, "paludis_xml_things_cleanup")));
            if (! c)
            {
                Log::get_instance()->message("e.xml_things.dlsym_failed", ll_warning, lc_context) << "Got error '"
                    << ::dlerror() << "' from dlsym for cleanup";
                return;
            }

            XMLThingsHandle::CreateGLSAFromXMLFilePtr g(STUPID_CAST(XMLThingsHandle::CreateGLSAFromXMLFilePtr,
                    ::dlsym(handle, "paludis_xml_things_create_glsa_from_xml_file")));
            if (! g)
            {
                Log::get_instance()->message("e.xml_things.dlsym_failed", ll_warning, lc_context) << "Got error '"
                    << ::dlerror() << "' from dlsym for GLSA things";
                return;
            }

            XMLThingsHandle::CreateMetadataXMLFromXMLFilePtr x(STUPID_CAST(XMLThingsHandle::CreateMetadataXMLFromXMLFilePtr,
                        ::dlsym(handle, "paludis_xml_things_create_metadata_xml_from_xml_file")));
            if (! x)
            {
                Log::get_instance()->message("e.xml_things.dlsym_failed", ll_warning, lc_context) << "Got error '"
                    << ::dlerror() << "' from dlsym for metadata.xml things";
                return;
            }

            init = i;
            cleanup = c;
            create_glsa_from_xml_file = g;
            create_metadata_xml_from_xml_file = x;

            init();
#endif
        }
    };
}

XMLThingsHandle::XMLThingsHandle() :
    PrivateImplementationPattern<XMLThingsHandle>()
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

XMLThingsHandle::CreateGLSAFromXMLFilePtr
XMLThingsHandle::create_glsa_from_xml_file() const
{
    return _imp->create_glsa_from_xml_file;
}

XMLThingsHandle::CreateMetadataXMLFromXMLFilePtr
XMLThingsHandle::create_metadata_xml_from_xml_file() const
{
    return _imp->create_metadata_xml_from_xml_file;
}

template class PrivateImplementationPattern<XMLThingsHandle>;
template class Singleton<XMLThingsHandle>;

