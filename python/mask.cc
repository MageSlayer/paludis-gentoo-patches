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

#include <paludis/mask.hh>
#include <paludis/util/visitor-impl.hh>

using namespace paludis;
using namespace paludis::python;
namespace bp = boost::python;

class MaskSptrToPythonVisitor :
    public ConstVisitor<MaskVisitorTypes>
{
    private:
        const tr1::shared_ptr<const Mask> & _m_ptr;

    public:
        bp::object obj;

        MaskSptrToPythonVisitor(const tr1::shared_ptr<const Mask> & m_ptr) :
            _m_ptr(m_ptr)
        {
        }

        void visit(const UserMask & m)
        {
            obj = bp::object(tr1::static_pointer_cast<const UserMask>(_m_ptr));
        }

        void visit(const UnacceptedMask & m)
        {
            obj = bp::object(tr1::static_pointer_cast<const UnacceptedMask>(_m_ptr));
        }

        void visit(const RepositoryMask & m)
        {
            obj = bp::object(tr1::static_pointer_cast<const RepositoryMask>(_m_ptr));
        }

        void visit(const UnsupportedMask & m)
        {
            obj = bp::object(tr1::static_pointer_cast<const UnsupportedMask>(_m_ptr));
        }

        void visit(const AssociationMask & m)
        {
            obj = bp::object(tr1::static_pointer_cast<const AssociationMask>(_m_ptr));
        }
};

struct MaskSptrToPython
{
    MaskSptrToPython()
    {
        bp::to_python_converter<tr1::shared_ptr<const Mask>, MaskSptrToPython>();
    }

    static PyObject *
    convert(const tr1::shared_ptr<const Mask> & m)
    {
        MaskSptrToPythonVisitor v(m);
        m->accept(v);
        return bp::incref(v.obj.ptr());
    }
};

struct MaskWrapper :
    Mask,
    bp::wrapper<Mask>
{
    virtual const char key() const
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("key"))
            return f();
        else
            throw PythonMethodNotImplemented("Mask", "key");
    }

    virtual const std::string description() const
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("description"))
            return f();
        else
            throw PythonMethodNotImplemented("Mask", "description");
    }
};

struct UserMaskWrapper :
    UserMask,
    bp::wrapper<UserMask>
{
    virtual const char key() const
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("key"))
            return f();
        else
            throw PythonMethodNotImplemented("UserMask", "key");
    }

    virtual const std::string description() const
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("description"))
            return f();
        else
            throw PythonMethodNotImplemented("UserMask", "description");
    }
};

struct UnacceptedMaskWrapper :
    UnacceptedMask,
    bp::wrapper<UnacceptedMask>
{
    virtual const tr1::shared_ptr<const MetadataKey> unaccepted_key() const
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("unaccepted_key"))
            return f();
        else
            throw PythonMethodNotImplemented("UnacceptedMask", "unaccepted_key");
    }

    virtual const char key() const
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("key"))
            return f();
        else
            throw PythonMethodNotImplemented("UnacceptedMask", "key");
    }

    virtual const std::string description() const
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("description"))
            return f();
        else
            throw PythonMethodNotImplemented("UnacceptedMask", "description");
    }
};

struct RepositoryMaskWrapper :
    RepositoryMask,
    bp::wrapper<RepositoryMask>
{
    virtual const tr1::shared_ptr<const MetadataKey> mask_key() const
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("mask_key"))
            return f();
        else
            throw PythonMethodNotImplemented("RepositoryMask", "mask_key");
    }

    virtual const char key() const
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("key"))
            return f();
        else
            throw PythonMethodNotImplemented("RepositoryMask", "key");
    }

    virtual const std::string description() const
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("description"))
            return f();
        else
            throw PythonMethodNotImplemented("RepositoryMask", "description");
    }
};
struct UnsupportedMaskWrapper :
    UnsupportedMask,
    bp::wrapper<UnsupportedMask>
{
    virtual const std::string explanation() const
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("explanation"))
            return f();
        else
            throw PythonMethodNotImplemented("UnsupportedMask", "explanation");
    }

    virtual const char key() const
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("key"))
            return f();
        else
            throw PythonMethodNotImplemented("UnsupportedMask", "key");
    }

    virtual const std::string description() const
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("description"))
            return f();
        else
            throw PythonMethodNotImplemented("UnsupportedMask", "description");
    }
};

struct AssociationMaskWrapper :
    AssociationMask,
    bp::wrapper<AssociationMask>
{
    virtual const tr1::shared_ptr<const PackageID> associated_package() const
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("associated_package"))
            return f();
        else
            throw PythonMethodNotImplemented("AssociationMask", "associated_package");
    }

    virtual const char key() const
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("key"))
            return f();
        else
            throw PythonMethodNotImplemented("AssociationMask", "key");
    }

    virtual const std::string description() const
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("description"))
            return f();
        else
            throw PythonMethodNotImplemented("AssociationMask", "description");
    }
};


void expose_mask()
{
    /**
     * RepositoryMaskInfo
     */
    register_shared_ptrs_to_python<RepositoryMaskInfo>(rsp_const);
    bp::class_<RepositoryMaskInfo, tr1::shared_ptr<RepositoryMaskInfo> >
        (
         "RepositoryMaskInfo",
         "Information about a RepositoryMask.",
         bp::init<const FSEntry &, const tr1::shared_ptr<const Sequence<std::string> > &>(
             "__init__(path_str, list of string)"
             )
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
    MaskSptrToPython();
    register_shared_ptrs_to_python<Mask>(rsp_non_const);
    bp::class_<MaskWrapper, boost::noncopyable>
        (
         "Mask",
         "NEED_DOC",
         bp::no_init
        )
        .def("key", bp::pure_virtual(&Mask::key),
                "key() -> string\n"
                "NEED_DOC"
                )

        .def("description", bp::pure_virtual(&Mask::description),
                "description() -> string\n"
                "NEED_DOC"
                )
        ;

    /**
     * UserMask
     */
    bp::register_ptr_to_python<tr1::shared_ptr<const UserMask> >();
    bp::implicitly_convertible<tr1::shared_ptr<UserMaskWrapper>, tr1::shared_ptr<Mask> >();
    bp::class_<UserMaskWrapper, tr1::shared_ptr<UserMaskWrapper>,
                bp::bases<Mask>, boost::noncopyable>
        (
         "UserMask",
         "NEED_DOC",
         bp::init<>()
        )
        .def("key", bp::pure_virtual(&Mask::key),
                "[ro] str\n"
                "NEED_DOC"
                )

        .def("description", bp::pure_virtual(&Mask::description),
                "[ro] str\n"
                "NEED_DOC"
                )
        ;

    /**
     * UnacceptedMask
     */
    bp::register_ptr_to_python<tr1::shared_ptr<const UnacceptedMask> >();
    bp::implicitly_convertible<tr1::shared_ptr<UnacceptedMaskWrapper>, tr1::shared_ptr<Mask> >();
    bp::class_<UnacceptedMaskWrapper, tr1::shared_ptr<UnacceptedMaskWrapper>,
            bp::bases<Mask>, boost::noncopyable>
        (
         "UnacceptedMask",
         "NEED_DOC",
         bp::init<>()
        )
        .def("unaccepted_key", bp::pure_virtual(&UnacceptedMask::unaccepted_key),
                "[ro] MetadataKey\n"
                "NEED_DOC"
                )

        .def("key", bp::pure_virtual(&Mask::key),
                "[ro] str\n"
                "NEED_DOC"
                )

        .def("description", bp::pure_virtual(&Mask::description),
                "[ro] str\n"
                "NEED_DOC"
                )
        ;

    /**
     * RepositoryMask
     */
    bp::register_ptr_to_python<tr1::shared_ptr<const RepositoryMask> >();
    bp::implicitly_convertible<tr1::shared_ptr<RepositoryMaskWrapper>, tr1::shared_ptr<Mask> >();
    bp::class_<RepositoryMaskWrapper, tr1::shared_ptr<RepositoryMaskWrapper>,
            bp::bases<Mask>, boost::noncopyable>
        (
         "RepositoryMask",
         "NEED_DOC",
         bp::init<>()
        )
        .def("mask_key", bp::pure_virtual(&RepositoryMask::mask_key),
                "[ro] MetadataKey\n"
                "NEED_DOC"
                )

        .def("key", bp::pure_virtual(&Mask::key),
                "[ro] str\n"
                "NEED_DOC"
                )

        .def("description", bp::pure_virtual(&Mask::description),
                "[ro] str\n"
                "NEED_DOC"
                )
        ;

    /**
     * UnsupportedMask
     */
    bp::register_ptr_to_python<tr1::shared_ptr<const UnsupportedMask> >();
    bp::implicitly_convertible<tr1::shared_ptr<UnsupportedMaskWrapper>, tr1::shared_ptr<Mask> >();
    bp::class_<UnsupportedMaskWrapper, tr1::shared_ptr<UnsupportedMaskWrapper>,
            bp::bases<Mask>, boost::noncopyable>
        (
         "UnsupportedMask",
         "NEED_DOC",
         bp::init<>()
        )
        .def("explanation", bp::pure_virtual(&UnsupportedMask::explanation),
                "[ro] str\n"
                "NEED_DOC"
                )

        .def("key", bp::pure_virtual(&Mask::key),
                "[ro] str\n"
                "NEED_DOC"
                )

        .def("description", bp::pure_virtual(&Mask::description),
                "[ro] str\n"
                "NEED_DOC"
                )
        ;

    /**
     * AssociationMask
     */
    bp::register_ptr_to_python<tr1::shared_ptr<const AssociationMask> >();
    bp::implicitly_convertible<tr1::shared_ptr<AssociationMaskWrapper>, tr1::shared_ptr<Mask> >();
    bp::class_<AssociationMaskWrapper, tr1::shared_ptr<AssociationMaskWrapper>,
            bp::bases<Mask>, boost::noncopyable>
        (
         "AssociationMask",
         "NEED_DOC",
         bp::init<>()
        )
        .def("associated_package", bp::pure_virtual(&AssociationMask::associated_package),
                "[ro] PackageID\n"
                "NEED_DOC"
                )

        .def("key", bp::pure_virtual(&Mask::key),
                "[ro] str\n"
                "NEED_DOC"
                )

        .def("description", bp::pure_virtual(&Mask::description),
                "[ro] str\n"
                "NEED_DOC"
                )
        ;
}

