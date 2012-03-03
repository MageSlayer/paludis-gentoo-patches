/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Piotr Jaroszyński
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
#include <paludis/util/make_named_values.hh>
#include <paludis/dep_spec.hh>

using namespace paludis;
using namespace paludis::python;
namespace bp = boost::python;

class MaskSptrToPythonVisitor
{
    private:
        const std::shared_ptr<const Mask> & _m_ptr;

    public:
        bp::object obj;

        MaskSptrToPythonVisitor(const std::shared_ptr<const Mask> & m_ptr) :
            _m_ptr(m_ptr)
        {
        }

        void visit(const UserMask &)
        {
            obj = bp::object(std::static_pointer_cast<const UserMask>(_m_ptr));
        }

        void visit(const UnacceptedMask &)
        {
            obj = bp::object(std::static_pointer_cast<const UnacceptedMask>(_m_ptr));
        }

        void visit(const RepositoryMask &)
        {
            obj = bp::object(std::static_pointer_cast<const RepositoryMask>(_m_ptr));
        }

        void visit(const UnsupportedMask &)
        {
            obj = bp::object(std::static_pointer_cast<const UnsupportedMask>(_m_ptr));
        }
};

struct MaskSptrToPython
{
    MaskSptrToPython()
    {
        bp::to_python_converter<std::shared_ptr<const Mask>, MaskSptrToPython>();
    }

    static PyObject *
    convert(const std::shared_ptr<const Mask> & m)
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
    virtual char key() const
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
    virtual char key() const
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
    virtual const std::string unaccepted_key_name() const
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("unaccepted_key_name"))
            return f();
        else
            throw PythonMethodNotImplemented("UnacceptedMask", "unaccepted_key_name");
    }

    virtual char key() const
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
    virtual const std::string mask_key_name() const
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("mask_key_name"))
            return f();
        else
            throw PythonMethodNotImplemented("RepositoryMask", "mask_key_name");
    }

    virtual char key() const
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

    virtual const std::string comment() const
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("comment"))
            return f();
        else
            throw PythonMethodNotImplemented("RepositoryMask", "comment");
    }

    virtual const std::string token() const
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("token"))
            return f();
        else
            throw PythonMethodNotImplemented("RepositoryMask", "token");
    }

    virtual const FSPath mask_file() const
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("mask_file"))
            return f();
        else
            throw PythonMethodNotImplemented("RepositoryMask", "mask_file");
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

    virtual char key() const
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

void expose_mask()
{
    /**
     * Mask
     */
    MaskSptrToPython();
    register_shared_ptrs_to_python<Mask>(rsp_non_const);
    bp::class_<MaskWrapper, boost::noncopyable>
        (
         "Mask",
         "A Mask represents one reason why a PackageID is masked (not available to\n"
         "be installed).\n\n"

         "A basic Mask has:\n\n"

         "- A single character key, which can be used by clients if they need a\n"
         "  very compact way of representing a mask.\n\n"

         "- A description.\n\n"

         "Subclasses provide additional information.",
         bp::no_init
        )
        .def("key", bp::pure_virtual(&Mask::key),
                "key() -> string\n"
                "A single character key, which can be used by clients if they need\n"
                "a very compact way of representing a mask."
                )

        .def("description", bp::pure_virtual(&Mask::description),
                "description() -> string\n"
                "A description of the mask."
                )
        ;

    /**
     * UserMask
     */
    bp::register_ptr_to_python<std::shared_ptr<const UserMask> >();
    bp::implicitly_convertible<std::shared_ptr<UserMaskWrapper>, std::shared_ptr<Mask> >();
    bp::class_<UserMaskWrapper, std::shared_ptr<UserMaskWrapper>,
                bp::bases<Mask>, boost::noncopyable>
        (
         "UserMask",
         "A UserMask is a Mask due to user configuration.\n\n"

         "Can be subclassed in Python.",
         bp::init<>()
        )
        .def("key", bp::pure_virtual(&Mask::key),
                "key() -> string\n"
                "A single character key, which can be used by clients if they need\n"
                "a very compact way of representing a mask."
                )

        .def("description", bp::pure_virtual(&Mask::description),
                "description() -> string\n"
                "A description of the mask."
                )
        ;

    /**
     * UnacceptedMask
     */
    bp::register_ptr_to_python<std::shared_ptr<const UnacceptedMask> >();
    bp::implicitly_convertible<std::shared_ptr<UnacceptedMaskWrapper>, std::shared_ptr<Mask> >();
    bp::class_<UnacceptedMaskWrapper, std::shared_ptr<UnacceptedMaskWrapper>,
            bp::bases<Mask>, boost::noncopyable>
        (
         "UnacceptedMask",
         "An UnacceptedMask is a Mask that signifies that a particular value or\n"
         "combination of values in (for example) a MetadataSetKey or\n"
         "MetadataSpecTreeKey is not accepted by user configuration.\n\n"

         "Can be subclassed in Python.",
         bp::init<>()
        )
        .def("unaccepted_key_name", bp::pure_virtual(&UnacceptedMask::unaccepted_key_name),
                "unaccepted_key_name() -> string\n"
                "Fetch the name of the metadata key that is not accepted."
                )

        .def("key", bp::pure_virtual(&Mask::key),
                "key() -> string\n"
                "A single character key, which can be used by clients if they need\n"
                "a very compact way of representing a mask."
                )

        .def("description", bp::pure_virtual(&Mask::description),
                "description() -> string\n"
                "A description of the mask."
                )
        ;

    /**
     * RepositoryMask
     */
    bp::register_ptr_to_python<std::shared_ptr<const RepositoryMask> >();
    bp::implicitly_convertible<std::shared_ptr<RepositoryMaskWrapper>, std::shared_ptr<Mask> >();
    bp::class_<RepositoryMaskWrapper, std::shared_ptr<RepositoryMaskWrapper>,
            bp::bases<Mask>, boost::noncopyable>
        (
         "RepositoryMask",
         "A RepositoryMask is a Mask that signifies that a PackageID has been\n"
         "marked as masked by a Repository.\n\n"

         "Can be subclassed in Python.",
         bp::init<>()
        )
        .def("key", bp::pure_virtual(&Mask::key),
                "key() -> string\n"
                "A single character key, which can be used by clients if they need\n"
                "a very compact way of representing a mask."
                )

        .def("description", bp::pure_virtual(&Mask::description),
                "description() -> string\n"
                "A description of the mask."
                )
        ;

    /**
     * UnsupportedMask
     */
    bp::register_ptr_to_python<std::shared_ptr<const UnsupportedMask> >();
    bp::implicitly_convertible<std::shared_ptr<UnsupportedMaskWrapper>, std::shared_ptr<Mask> >();
    bp::class_<UnsupportedMaskWrapper, std::shared_ptr<UnsupportedMaskWrapper>,
            bp::bases<Mask>, boost::noncopyable>
        (
         "UnsupportedMask",
         "An UnsupportedMask is a Mask that signifies that a PackageID is not\n"
         "supported, for example because it is broken or because it uses an\n"
         "unrecognised EAPI.\n\n"

         "Can be subclassed in Python.",
         bp::init<>()
        )
        .def("explanation", bp::pure_virtual(&UnsupportedMask::explanation),
                "explanation() -> str\n"
                "An explanation of why we are unsupported."
                )

        .def("key", bp::pure_virtual(&Mask::key),
                "key() -> string\n"
                "A single character key, which can be used by clients if they need\n"
                "a very compact way of representing a mask."
                )

        .def("description", bp::pure_virtual(&Mask::description),
                "description() -> string\n"
                "A description of the mask."
                )
        ;
}

