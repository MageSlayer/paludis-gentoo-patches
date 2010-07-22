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
#include <paludis/util/make_shared_ptr.hh>

using namespace paludis;
using namespace paludis::python;
namespace bp = boost::python;

namespace
{
    std::shared_ptr<RepositoryMaskInfo>  make_repository_mask_info(
            const std::shared_ptr<const Sequence<std::string> > & s, const FSEntry & f)
    {
        return make_shared_ptr(new RepositoryMaskInfo(make_named_values<RepositoryMaskInfo>(
                        n::comment() = s,
                        n::mask_file() = f
                        )));
    }
}

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

        void visit(const UserMask & m)
        {
            obj = bp::object(std::static_pointer_cast<const UserMask>(_m_ptr));
        }

        void visit(const UnacceptedMask & m)
        {
            obj = bp::object(std::static_pointer_cast<const UnacceptedMask>(_m_ptr));
        }

        void visit(const RepositoryMask & m)
        {
            obj = bp::object(std::static_pointer_cast<const RepositoryMask>(_m_ptr));
        }

        void visit(const UnsupportedMask & m)
        {
            obj = bp::object(std::static_pointer_cast<const UnsupportedMask>(_m_ptr));
        }

        void visit(const AssociationMask & m)
        {
            obj = bp::object(std::static_pointer_cast<const AssociationMask>(_m_ptr));
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
    virtual const std::shared_ptr<const MetadataKey> unaccepted_key() const
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("unaccepted_key"))
            return f();
        else
            throw PythonMethodNotImplemented("UnacceptedMask", "unaccepted_key");
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
    virtual const std::shared_ptr<const MetadataKey> mask_key() const
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("mask_key"))
            return f();
        else
            throw PythonMethodNotImplemented("RepositoryMask", "mask_key");
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

struct AssociationMaskWrapper :
    AssociationMask,
    bp::wrapper<AssociationMask>
{
    virtual const std::shared_ptr<const PackageID> associated_package() const
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("associated_package"))
            return f();
        else
            throw PythonMethodNotImplemented("AssociationMask", "associated_package");
    }

    virtual char key() const
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
    bp::class_<RepositoryMaskInfo, std::shared_ptr<RepositoryMaskInfo> >
        (
         "RepositoryMaskInfo",
         "Information about a RepositoryMask.",
         bp::no_init
        )

        .def("__init__",
                bp::make_constructor(&make_repository_mask_info),
                "__init__(list of string, path_str)"
            )

        .add_property("mask_file",
                &named_values_getter<RepositoryMaskInfo, n::mask_file, FSEntry, &RepositoryMaskInfo::mask_file>,
                &named_values_setter<RepositoryMaskInfo, n::mask_file, FSEntry, &RepositoryMaskInfo::mask_file>,
                "[ro] str\n"
                "Holds the file whence the mask originates."
                )

        .add_property("comment",
                &named_values_getter<RepositoryMaskInfo, n::comment, std::shared_ptr<const Sequence<std::string> >, &RepositoryMaskInfo::comment>,
                &named_values_setter<RepositoryMaskInfo, n::comment, std::shared_ptr<const Sequence<std::string> >, &RepositoryMaskInfo::comment>,
                "[ro] Iterable of str\n"
                "Sequence of lines explaining the mask."
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
        .def("unaccepted_key", bp::pure_virtual(&UnacceptedMask::unaccepted_key),
                "unaccepted_key() -> MetadataKey\n"
                "Fetch the metadata key that is not accepted."
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
        .def("mask_key", bp::pure_virtual(&RepositoryMask::mask_key),
                "mask_key() -> MetadataKey\n"
                "Fetch a metadata key explaining the mask. May return None,\n"
                "if no more information is available."
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

    /**
     * AssociationMask
     */
    bp::register_ptr_to_python<std::shared_ptr<const AssociationMask> >();
    bp::implicitly_convertible<std::shared_ptr<AssociationMaskWrapper>, std::shared_ptr<Mask> >();
    bp::class_<AssociationMaskWrapper, std::shared_ptr<AssociationMaskWrapper>,
            bp::bases<Mask>, boost::noncopyable>
        (
         "AssociationMask",
         "An AssociationMask is a Mask that signifies that a PackageID is masked\n"
         "because of its association with another PackageID that is itself masked.\n\n"

         "This is used by old-style virtuals. If the provider of a virtual is\n"
         "masked then the virtual itself is masked by association.\n\n"

         "Can be subclassed in Python.",
         bp::init<>()
        )
        .def("associated_package", bp::pure_virtual(&AssociationMask::associated_package),
                "associated_package() -> PackageID\n"
                "Fetch the associated package."
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

