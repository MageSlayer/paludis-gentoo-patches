/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010 Ciaran McCreesh
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

#include <paludis/repositories/e/glsa.hh>
#include <paludis/repositories/e/xml_things_handle.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/about.hh>
#include <list>
#include <dlfcn.h>
#include <stdint.h>
#include "config.h"

using namespace paludis;

typedef std::list<std::string> Archs;
typedef std::list<erepository::GLSARange> Ranges;
typedef std::list<std::shared_ptr<const GLSAPackage> > Packages;

namespace paludis
{
    template<>
    struct Imp<GLSAPackage>
    {
        QualifiedPackageName name;
        Archs archs;
        Ranges unaffected;
        Ranges vulnerable;

        Imp(const QualifiedPackageName & n) :
            name(n)
        {
        }
    };

    template<>
    struct Imp<GLSA>
    {
        std::string id;
        std::string title;
        Packages packages;
    };

    template <>
    struct WrappedForwardIteratorTraits<GLSAPackage::ArchsConstIteratorTag>
    {
        typedef Archs::const_iterator UnderlyingIterator;
    };

    template <>
    struct WrappedForwardIteratorTraits<GLSAPackage::RangesConstIteratorTag>
    {
        typedef Ranges::const_iterator UnderlyingIterator;
    };

    template <>
    struct WrappedForwardIteratorTraits<GLSA::PackagesConstIteratorTag>
    {
        typedef IndirectIterator<Packages::const_iterator> UnderlyingIterator;
    };
}

GLSAPackage::GLSAPackage(const QualifiedPackageName & n) :
    Pimp<GLSAPackage>(n)
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
GLSAPackage::add_unaffected(const erepository::GLSARange & r)
{
    _imp->unaffected.push_back(r);
}

void
GLSAPackage::add_vulnerable(const erepository::GLSARange & r)
{
    _imp->vulnerable.push_back(r);
}

QualifiedPackageName
GLSAPackage::name() const
{
    return _imp->name;
}

GLSA::GLSA() :
    Pimp<GLSA>()
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
GLSA::add_package(const std::shared_ptr<const GLSAPackage> & p)
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

std::shared_ptr<GLSA>
GLSA::create_from_xml_file(const std::string & filename)
{
    if (! erepository::XMLThingsHandle::get_instance()->create_glsa_from_xml_file())
    {
#ifdef ENABLE_XML
        throw NotAvailableError("Cannot create GLSA from XML file '" + filename + "' because your XML libraries are unusable");
#else
        throw NotAvailableError("Cannot create GLSA from XML file '" + filename + "' because GLSA support was not enabled at compile time");
#endif
    }

    return erepository::XMLThingsHandle::get_instance()->create_glsa_from_xml_file()(filename);
}

GLSAError::GLSAError(const std::string & msg, const std::string & filename) throw () :
    ConfigurationError("GLSA error: " + msg + (filename.empty() ? "" : " in file " + filename))
{
}

template class WrappedForwardIterator<GLSAPackage::ArchsConstIteratorTag, const std::string>;
template class WrappedForwardIterator<GLSAPackage::RangesConstIteratorTag, const erepository::GLSARange>;
template class WrappedForwardIterator<GLSA::PackagesConstIteratorTag, const GLSAPackage>;

