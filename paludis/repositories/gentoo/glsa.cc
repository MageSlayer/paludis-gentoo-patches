/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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
#include <paludis/util/iterator.hh>
#include <paludis/util/stringify.hh>
#include <list>
#include <dlfcn.h>
#include <stdint.h>
#include "config.h"

#define STUPID_CAST(type, val) reinterpret_cast<type>(reinterpret_cast<uintptr_t>(val))

using namespace paludis;

#include "glsa-sr.cc"

#ifdef MONOLITHIC
#  include <paludis/repositories/portage/xml_things.hh>
#endif

namespace paludis
{
    template<>
    struct Implementation<GLSAPackage>
    {
        QualifiedPackageName name;
        std::list<UseFlagName> archs;
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
        std::list<tr1::shared_ptr<const GLSAPackage> > packages;
    };
}

GLSAPackage::GLSAPackage(const QualifiedPackageName & n) :
    PrivateImplementationPattern<GLSAPackage>(new Implementation<GLSAPackage>(n))
{
}

GLSAPackage::~GLSAPackage()
{
}

GLSAPackage::ArchsIterator
GLSAPackage::begin_archs() const
{
    return ArchsIterator(_imp->archs.begin());
}

GLSAPackage::ArchsIterator
GLSAPackage::end_archs() const
{
    return ArchsIterator(_imp->archs.end());
}

void
GLSAPackage::add_arch(const UseFlagName & n)
{
    _imp->archs.push_back(n);
}

GLSAPackage::RangesIterator
GLSAPackage::begin_unaffected() const
{
    return RangesIterator(_imp->unaffected.begin());
}

GLSAPackage::RangesIterator
GLSAPackage::end_unaffected() const
{
    return RangesIterator(_imp->unaffected.end());
}

GLSAPackage::RangesIterator
GLSAPackage::begin_vulnerable() const
{
    return RangesIterator(_imp->vulnerable.begin());
}

GLSAPackage::RangesIterator
GLSAPackage::end_vulnerable() const
{
    return RangesIterator(_imp->vulnerable.end());
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

GLSA::PackagesIterator
GLSA::begin_packages() const
{
    return PackagesIterator(indirect_iterator(_imp->packages.begin()));
}

GLSA::PackagesIterator
GLSA::end_packages() const
{
    return PackagesIterator(indirect_iterator(_imp->packages.end()));
}

void
GLSA::add_package(tr1::shared_ptr<const GLSAPackage> p)
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
        void * handle;
        tr1::shared_ptr<GLSA> (* create_glsa_from_xml_file_handle)(const std::string &);

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

tr1::shared_ptr<GLSA>
GLSA::create_from_xml_file(const std::string & filename)
{
#if ENABLE_GLSA
#  ifdef MONOLITHIC

#  else
    if (0 == libxmlhandle.handle)
        libxmlhandle.handle = dlopen("libpaludisgentoorepositoryxmlthings.so",
                RTLD_NOW | RTLD_GLOBAL);
    if (0 == libxmlhandle.handle)
        throw NotAvailableError("Cannot create GLSA from XML file '" + filename + "' due to error '"
                + stringify(dlerror()) + "' when dlopen(libpaludisgentoorepositoryxmlthings.so)");

    if (0 == libxmlhandle.create_glsa_from_xml_file_handle)
        libxmlhandle.create_glsa_from_xml_file_handle = STUPID_CAST(tr1::shared_ptr<GLSA> (*)(const std::string &),
                dlsym(libxmlhandle.handle, "create_glsa_from_xml_file"));
    if (0 == libxmlhandle.create_glsa_from_xml_file_handle)
        throw NotAvailableError("Cannot create GLSA from XML file '" + filename + "' due to error '"
                + stringify(dlerror()) + "' when dlsym(libpaludisgentoorepositoryxmlthings.so, create_glsa_from_xml_file)");

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

