/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Piotr Jaroszyński <peper@gentoo.org>
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

#include <python/paludis_python.hh>
#include <python/iterable.hh>

#include <python/metadata_key.hh>

#include <paludis/mask.hh>
#include <paludis/util/visitor-impl.hh>

using namespace paludis;
using namespace paludis::python;
namespace bp = boost::python;

struct MaskToPython :
    ConstVisitor<MaskVisitorTypes>
{
    bp::object value;

    void visit(const UserMask & m)
    {
        value = bp::object(bp::ptr(&m));
    }

    void visit(const UnacceptedMask & m)
    {
        value = bp::object(bp::ptr(&m));
    }

    void visit(const RepositoryMask & m)
    {
        value = bp::object(bp::ptr(&m));
    }

    void visit(const UnsupportedMask & m)
    {
        value = bp::object(bp::ptr(&m));
    }

    void visit(const AssociationMask & m)
    {
        value = bp::object(bp::ptr(&m));
    }
};

struct mask_to_python
{
    static PyObject *
    convert(const Mask & m)
    {
        MaskToPython v;
        m.accept(v);
        return bp::incref(v.value.ptr());
    }
};

void register_mask_to_python()
{
    bp::to_python_converter<Mask, mask_to_python>();
}

struct UnacceptedMaskWrapper
{
    static PyObject *
    unaccepted_key(const UnacceptedMask & self)
    {
        MetadataKeyToPython v;
        self.unaccepted_key()->accept(v);
        return bp::incref(v.value.ptr());
    }
};

struct RepositoryMaskWrapper
{
    static PyObject *
    mask_key(const RepositoryMask & self)
    {
        MetadataKeyToPython v;
        self.mask_key()->accept(v);
        return bp::incref(v.value.ptr());
    }
};


void PALUDIS_VISIBLE expose_mask()
{
    /**
     * RepositoryMaskInfo
     */
    class_iterable<Sequence<std::string> >
        (
         "StringIterable",
         "Iterable of string"
        );
    register_shared_ptrs_to_python<RepositoryMaskInfo>();
    bp::class_<RepositoryMaskInfo>
        (
         "RepositoryMaskInfo",
         "Information about a RepositoryMask.",
         bp::no_init
        )
        .add_property("mask_file", bp::make_getter(&RepositoryMaskInfo::mask_file,
                    bp::return_value_policy<bp::return_by_value>()),
                "[ro] str\n"
                "NEED_DOC"
                )

        .add_property("comment", bp::make_getter(&RepositoryMaskInfo::comment,
                    bp::return_value_policy<bp::return_by_value>()),
                "[ro] Iterable of str\n"
                "NEED_DOC"
                )
        ;

    /**
     * Mask
     */
    register_mask_to_python();
    bp::class_<Mask, boost::noncopyable>
        (
         "Mask",
         "NEED_DOC",
         bp::no_init
        )
        .add_property("key", &Mask::key,
                "[ro] str\n"
                "NEED_DOC"
                )

        .add_property("description", &Mask::description,
                "[ro] str\n"
                "NEED_DOC"
                )
        ;

    /**
     * UserMask
     */
    bp::class_<UserMask, bp::bases<Mask>, boost::noncopyable>
        (
         "UserMask",
         "NEED_DOC",
         bp::no_init
        );

    /**
     * UnacceptedMask
     */
    bp::class_<UnacceptedMask, bp::bases<Mask>, boost::noncopyable>
        (
         "UnacceptedMask",
         "NEED_DOC",
         bp::no_init
        )
        .add_property("unaccepted_key", &UnacceptedMaskWrapper::unaccepted_key,
                "[ro] MetadataKey\n"
                "NEED_DOC"
                )
        ;

    /**
     * RepositoryMask
     */
    bp::class_<RepositoryMask, bp::bases<Mask>, boost::noncopyable>
        (
         "RepositoryMask",
         "NEED_DOC",
         bp::no_init
        )
        .add_property("mask_key", &RepositoryMaskWrapper::mask_key,
                "[ro] MetadataKey\n"
                "NEED_DOC"
                )
        ;

    /**
     * UnsupportedMask
     */
    bp::class_<UnsupportedMask, bp::bases<Mask>, boost::noncopyable>
        (
         "UnsupportedMask",
         "NEED_DOC",
         bp::no_init
        )
        .add_property("explanation", &UnsupportedMask::explanation,
                "[ro] str\n"
                "NEED_DOC"
                )
        ;

    /**
     * AssociationMask
     */
    bp::class_<AssociationMask, bp::bases<Mask>, boost::noncopyable>
        (
         "AssociationMask",
         "NEED_DOC",
         bp::no_init
        )
        .add_property("associated_package", &AssociationMask::associated_package,
                "[ro] PackageID\n"
                "NEED_DOC"
                )
        ;
}

