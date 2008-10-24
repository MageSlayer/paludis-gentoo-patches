/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008 Ciaran McCreesh
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

#include "glsa.hh"
#include <paludis/util/stringify.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/about.hh>
#include <list>
#include <dlfcn.h>
#include <stdint.h>
#include "config.h"

#define STUPID_CAST(type, val) reinterpret_cast<type>(reinterpret_cast<uintptr_t>(val))

using namespace paludis;

template class WrappedForwardIterator<GLSAPackage::ArchsConstIteratorTag, const std::string>;
template class WrappedForwardIterator<GLSAPackage::RangesConstIteratorTag, const GLSARange>;
template class WrappedForwardIterator<GLSA::PackagesConstIteratorTag, const GLSAPackage>;

#include "glsa-sr.cc"

#ifdef MONOLITHIC
#  include <paludis/repositories/e/xml_things.hh>
#endif

namespace paludis
{
    template<>
    struct Implementation<GLSAPackage>
    {
        QualifiedPackageName name;
        std::list<std::string> archs;
        std::list<GLSARange> unaffected;
        std::list<GLSARange> vulnerable;

        Implementation(const QualifiedPackageName & n) :
            name(n)
        {
        }
    };

    template<>
    struct Implementation<GLSA>
    {
        std::string id;
        std::string title;
        std::list<std::tr1::shared_ptr<const GLSAPackage> > packages;
    };
}

GLSAPackage::GLSAPackage(const QualifiedPackageName & n) :
    PrivateImplementationPattern<GLSAPackage>(new Implementation<GLSAPackage>(n))
{
}

GLSAPackage::~GLSAPackage()
{
}

GLSAPackage::ArchsConstIterator
GLSAPackage::begin_archs() const
{
    return ArchsConstIterator(_imp->archs.begin());
}

GLSAPackage::ArchsConstIterator
GLSAPackage::end_archs() const
{
    return ArchsConstIterator(_imp->archs.end());
}

void
GLSAPackage::add_arch(const std::string & n)
{
    _imp->archs.push_back(n);
}

GLSAPackage::RangesConstIterator
GLSAPackage::begin_unaffected() const
{
    return RangesConstIterator(_imp->unaffected.begin());
}

GLSAPackage::RangesConstIterator
GLSAPackage::end_unaffected() const
{
    return RangesConstIterator(_imp->unaffected.end());
}

GLSAPackage::RangesConstIterator
GLSAPackage::begin_vulnerable() const
{
    return RangesConstIterator(_imp->vulnerable.begin());
}

GLSAPackage::RangesConstIterator
GLSAPackage::end_vulnerable() const
{
    return RangesConstIterator(_imp->vulnerable.end());
}

void
GLSAPackage::add_unaffected(const GLSARange & r)
{
    _imp->unaffected.push_back(r);
}

void
GLSAPackage::add_vulnerable(const GLSARange & r)
{
    _imp->vulnerable.push_back(r);
}

QualifiedPackageName
GLSAPackage::name() const
{
    return _imp->name;
}

GLSA::GLSA() :
    PrivateImplementationPattern<GLSA>(new Implementation<GLSA>)
{
}

GLSA::~GLSA()
{
}

GLSA::PackagesConstIterator
GLSA::begin_packages() const
{
    return PackagesConstIterator(indirect_iterator(_imp->packages.begin()));
}

GLSA::PackagesConstIterator
GLSA::end_packages() const
{
    return PackagesConstIterator(indirect_iterator(_imp->packages.end()));
}

void
GLSA::add_package(const std::tr1::shared_ptr<const GLSAPackage> & p)
{
    _imp->packages.push_back(p);
}

void
GLSA::set_id(const std::string & s)
{
    _imp->id = s;
}

void
GLSA::set_title(const std::string & s)
{
    _imp->title = s;
}

std::string
GLSA::id() const
{
    return _imp->id;
}

std::string
GLSA::title() const
{
    return _imp->title;
}

#ifndef MONOLITHIC

namespace
{
    struct LibXMLHandle
    {
        Mutex mutex;
        void * handle;
        std::tr1::shared_ptr<GLSA> (* create_glsa_from_xml_file_handle)(const std::string &);

        LibXMLHandle() :
            handle(0),
            create_glsa_from_xml_file_handle(0)
        {
        }

        ~LibXMLHandle()
        {
            if (0 != handle)
                dlclose(handle);
        }

    } libxmlhandle;
}

#endif

std::tr1::shared_ptr<GLSA>
GLSA::create_from_xml_file(const std::string & filename)
{
#if ENABLE_GLSA
#  ifdef MONOLITHIC

#  else
    {
        Lock lock(libxmlhandle.mutex);

        if (0 == libxmlhandle.handle)
            libxmlhandle.handle = dlopen(("libpaludiserepositoryxmlthings_" + stringify(PALUDIS_PC_SLOT) + ".so").c_str(),
                    RTLD_NOW | RTLD_GLOBAL);
        if (0 == libxmlhandle.handle)
            throw NotAvailableError("Cannot create GLSA from XML file '" + filename + "' due to error '"
                    + stringify(dlerror()) + "' when dlopen(libpaludiserepositoryxmlthings.so)");

        if (0 == libxmlhandle.create_glsa_from_xml_file_handle)
            libxmlhandle.create_glsa_from_xml_file_handle = STUPID_CAST(std::tr1::shared_ptr<GLSA> (*)(const std::string &),
                    dlsym(libxmlhandle.handle, "create_glsa_from_xml_file"));
        if (0 == libxmlhandle.create_glsa_from_xml_file_handle)
            throw NotAvailableError("Cannot create GLSA from XML file '" + filename + "' due to error '"
                    + stringify(dlerror()) + "' when dlsym(libpaludisgentoorepositoryxmlthings.so, create_glsa_from_xml_file)");
    }

#  endif
#else
#  ifndef MONOLITHIC
    /* avoid noreturn warning */
    if (0 == libxmlhandle.handle)
        throw NotAvailableError("Cannot create GLSA from XML file '" + filename + "' because Paludis was built "
                "without GLSA support");
#  endif
#endif

#ifdef MONOLITHIC
    return create_glsa_from_xml_file(filename);
#else
    return (*libxmlhandle.create_glsa_from_xml_file_handle)(filename);
#endif
}

GLSAError::GLSAError(const std::string & msg, const std::string & filename) throw () :
    ConfigurationError("GLSA error: " + msg + (filename.empty() ? "" : " in file " + filename))
{
}

