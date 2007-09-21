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
#include <python/exception.hh>

#include <paludis/metadata_key.hh>
#include <paludis/name.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/visitor-impl.hh>

using namespace paludis;
using namespace paludis::python;
namespace bp = boost::python;

class MetadataKeySptrToPythonVisitor :
    public ConstVisitor<MetadataKeyVisitorTypes>
{
    private:
        const tr1::shared_ptr<const MetadataKey> & _m_ptr;

    public:
        boost::python::object obj;

        MetadataKeySptrToPythonVisitor(const tr1::shared_ptr<const MetadataKey> & m_ptr) :
            _m_ptr(m_ptr)
        {
        }

        void visit(const MetadataPackageIDKey & k)
        {
            obj = bp::object(tr1::static_pointer_cast<const MetadataPackageIDKey>(_m_ptr));
        }

        void visit(const MetadataStringKey & k)
        {
            obj = bp::object(tr1::static_pointer_cast<const MetadataStringKey>(_m_ptr));
        }

        void visit(const MetadataTimeKey & k)
        {
            obj = bp::object(tr1::static_pointer_cast<const MetadataTimeKey>(_m_ptr));
        }

        void visit(const MetadataContentsKey & k)
        {
            obj = bp::object(tr1::static_pointer_cast<const MetadataContentsKey>(_m_ptr));
        }

        void visit(const MetadataRepositoryMaskInfoKey & k)
        {
            obj = bp::object(tr1::static_pointer_cast<const MetadataRepositoryMaskInfoKey>(_m_ptr));
        }

        void visit(const MetadataFSEntryKey & k)
        {
            obj = bp::object(tr1::static_pointer_cast<const MetadataFSEntryKey>(_m_ptr));
        }

        void visit(const MetadataSetKey<KeywordNameSet> & k)
        {
            obj = bp::object(tr1::static_pointer_cast<const MetadataSetKey<KeywordNameSet> >(_m_ptr));
        }

        void visit(const MetadataSetKey<UseFlagNameSet> & k)
        {
            obj = bp::object(tr1::static_pointer_cast<const MetadataSetKey<UseFlagNameSet> >(_m_ptr));
        }

        void visit(const MetadataSetKey<IUseFlagSet> & k)
        {
            obj = bp::object(tr1::static_pointer_cast<const MetadataSetKey<IUseFlagSet> >(_m_ptr));
        }

        void visit(const MetadataSetKey<Set<std::string> > & k)
        {
            obj = bp::object(tr1::static_pointer_cast<const MetadataSetKey<Set<std::string> > >(_m_ptr));
        }

        void visit(const MetadataSpecTreeKey<LicenseSpecTree> & k)
        {
            obj = bp::object(tr1::static_pointer_cast<const MetadataSpecTreeKey<LicenseSpecTree> >(_m_ptr));
        }

        void visit(const MetadataSpecTreeKey<ProvideSpecTree> & k)
        {
            obj = bp::object(tr1::static_pointer_cast<const MetadataSpecTreeKey<ProvideSpecTree> >(_m_ptr));
        }

        void visit(const MetadataSpecTreeKey<DependencySpecTree> & k)
        {
            obj = bp::object(tr1::static_pointer_cast<const MetadataSpecTreeKey<DependencySpecTree> >(_m_ptr));
        }

        void visit(const MetadataSpecTreeKey<RestrictSpecTree> & k)
        {
            obj = bp::object(tr1::static_pointer_cast<const MetadataSpecTreeKey<RestrictSpecTree> >(_m_ptr));
        }

        void visit(const MetadataSpecTreeKey<URISpecTree> & k)
        {
            obj = bp::object(tr1::static_pointer_cast<const MetadataSpecTreeKey<URISpecTree> >(_m_ptr));
        }

        void visit(const MetadataSetKey<PackageIDSequence> & k)
        {
            obj = bp::object(tr1::static_pointer_cast<const MetadataSetKey<PackageIDSequence> >(_m_ptr));
        }
};

struct MetadataKeySptrToPython
{
    MetadataKeySptrToPython()
    {
        bp::to_python_converter<tr1::shared_ptr<const MetadataKey>, MetadataKeySptrToPython>();
    }

    static PyObject *
    convert(const tr1::shared_ptr<const MetadataKey> & m)
    {
        MetadataKeySptrToPythonVisitor v(m);
        m->accept(v);
        return bp::incref(v.obj.ptr());
    }
};

struct MetadataPackageIDKeyWrapper :
    MetadataPackageIDKey,
    bp::wrapper<MetadataPackageIDKey>
{
    MetadataPackageIDKeyWrapper(const std::string & r, const std::string & h, const MetadataKeyType t) :
        MetadataPackageIDKey(r, h, t)
    {
    }

    virtual const tr1::shared_ptr<const PackageID> value() const
        PALUDIS_ATTRIBUTE((warn_unused_result))
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("value"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataPackageIDKey", "value");
    }
};

struct MetadataStringKeyWrapper :
    MetadataStringKey,
    bp::wrapper<MetadataStringKey>
{
    MetadataStringKeyWrapper(const std::string & r, const std::string & h, const MetadataKeyType t) :
        MetadataStringKey(r, h, t)
    {
    }

    virtual const std::string value() const
        PALUDIS_ATTRIBUTE((warn_unused_result))
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("value"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataStringKey", "value");
    }
};

struct MetadataTimeKeyWrapper :
    MetadataTimeKey,
    bp::wrapper<MetadataTimeKey>
{
    MetadataTimeKeyWrapper(const std::string & r, const std::string & h, const MetadataKeyType t) :
        MetadataTimeKey(r, h, t)
    {
    }

    virtual const time_t value() const
        PALUDIS_ATTRIBUTE((warn_unused_result))
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("value"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataTimeKey", "value");
    }
};

struct MetadataContentsKeyWrapper :
    MetadataContentsKey,
    bp::wrapper<MetadataContentsKey>
{
    MetadataContentsKeyWrapper(const std::string & r, const std::string & h, const MetadataKeyType t) :
        MetadataContentsKey(r, h, t)
    {
    }

    virtual const tr1::shared_ptr<const Contents> value() const
        PALUDIS_ATTRIBUTE((warn_unused_result))
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("value"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataContentsKey", "value");
    }
};

struct MetadataFSEntryKeyWrapper :
    MetadataFSEntryKey,
    bp::wrapper<MetadataFSEntryKey>
{
    MetadataFSEntryKeyWrapper(const std::string & r, const std::string & h, const MetadataKeyType t) :
        MetadataFSEntryKey(r, h, t)
    {
    }

    virtual const FSEntry value() const
        PALUDIS_ATTRIBUTE((warn_unused_result))
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("value"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataFSEntryKey", "value");
    }
};

struct MetadataRepositoryMaskInfoKeyWrapper :
    MetadataRepositoryMaskInfoKey,
    bp::wrapper<MetadataRepositoryMaskInfoKey>
{
    MetadataRepositoryMaskInfoKeyWrapper(const std::string & r, const std::string & h, const MetadataKeyType t) :
        MetadataRepositoryMaskInfoKey(r, h, t)
    {
    }

    virtual const tr1::shared_ptr<const RepositoryMaskInfo> value() const
        PALUDIS_ATTRIBUTE((warn_unused_result))
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("value"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataRepositoryMaskInfoKey", "value");
    }
};

template <typename C_>
struct MetadataSetKeyWrapper :
    MetadataSetKey<C_>,
    bp::wrapper<MetadataSetKey<C_> >
{
    MetadataSetKeyWrapper(const std::string & r, const std::string & h, const MetadataKeyType t) :
        MetadataSetKey<C_>(r, h, t)
    {
    }

    virtual const tr1::shared_ptr<const C_> value() const
        PALUDIS_ATTRIBUTE((warn_unused_result))
    {
        Lock l(get_mutex());

        if (bp::override f = this->get_override("value"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataSetKey", "value");
    }
};

template <typename C_>
struct MetadataSpecTreeKeyWrapper :
    MetadataSpecTreeKey<C_>,
    bp::wrapper<MetadataSpecTreeKey<C_> >
{
    MetadataSpecTreeKeyWrapper(const std::string & r, const std::string & h, const MetadataKeyType t) :
        MetadataSpecTreeKey<C_>(r, h, t)
    {
    }

    virtual const tr1::shared_ptr<const typename C_::ConstItem> value() const
        PALUDIS_ATTRIBUTE((warn_unused_result))
    {
        Lock l(get_mutex());

        if (bp::override f = this->get_override("value"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataSpecTreeKey", "value");
    }

    virtual std::string pretty_print() const
        PALUDIS_ATTRIBUTE((warn_unused_result))
    {
        Lock l(get_mutex());

        if (bp::override f = this->get_override("pretty_print"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataSpecTreeKey", "pretty_print");
    }

    virtual std::string pretty_print_flat() const
        PALUDIS_ATTRIBUTE((warn_unused_result))
    {
        Lock l(get_mutex());

        if (bp::override f = this->get_override("pretty_print_flat"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataSpecTreeKey", "pretty_print_flat");
    }
};

template <typename C_>
struct class_set_key :
    bp::class_<MetadataSetKeyWrapper<C_>, tr1::shared_ptr<MetadataSetKeyWrapper<C_> >,
        bp::bases<MetadataKey>, boost::noncopyable>
{
    class_set_key(const std::string & set) :
        bp::class_<MetadataSetKeyWrapper<C_>, tr1::shared_ptr<MetadataSetKeyWrapper<C_> >,
            bp::bases<MetadataKey>, boost::noncopyable>(
                    ("Metadata" + set + "Key").c_str(),
                    "NEED_DOC\n"
                    "This class can be subclassed in Python.",
                    bp::init<const std::string &, const std::string &, MetadataKeyType>(
                        "__init__(raw_name, human_name, MetadataKeyType)"
                        )
                    )
    {
        bp::register_ptr_to_python<tr1::shared_ptr<const MetadataSetKey<C_> > >();
        bp::implicitly_convertible<tr1::shared_ptr<MetadataSetKeyWrapper<C_> >,
                tr1::shared_ptr<MetadataKey> >();

        def("value", bp::pure_virtual(&MetadataSetKey<C_>::value),
                ("[ro] " + set + "\n").c_str());
    }
};

template <typename C_>
struct class_spec_tree_key :
    bp::class_<MetadataSpecTreeKeyWrapper<C_>, tr1::shared_ptr<MetadataSpecTreeKeyWrapper<C_> >,
        bp::bases<MetadataKey>, boost::noncopyable>
{
    class_spec_tree_key(const std::string & spec_tree) :
        bp::class_<MetadataSpecTreeKeyWrapper<C_>, tr1::shared_ptr<MetadataSpecTreeKeyWrapper<C_> >,
            bp::bases<MetadataKey>, boost::noncopyable>(
                    ("Metadata" + spec_tree + "Key").c_str(),
                    "NEED_DOC\n"
                    "This class can be subclassed in Python.",
                    bp::init<const std::string &, const std::string &, MetadataKeyType>(
                        "__init__(raw_name, human_name, MetadataKeyType)"
                        )
                    )
    {
        bp::register_ptr_to_python<tr1::shared_ptr<const MetadataSpecTreeKey<C_> > >();
        bp::implicitly_convertible<tr1::shared_ptr<MetadataSpecTreeKeyWrapper<C_> >,
                tr1::shared_ptr<MetadataKey> >();

        def("value", bp::pure_virtual(&MetadataSpecTreeKey<C_>::value),
                ("[ro] " + spec_tree + "\n").c_str());
        def("pretty_print", bp::pure_virtual(&MetadataSpecTreeKey<C_>::pretty_print));
        def("pretty_print_flat", bp::pure_virtual(&MetadataSpecTreeKey<C_>::pretty_print_flat));
    }
};

void expose_metadata_key()
{
    /**
     * Enums
     */
    enum_auto("MetadataKeyType", last_mkt,
            "The significance of a MetadataKey to a user.");

    /**
     * MetadataKey
     */
    MetadataKeySptrToPython();
    register_shared_ptrs_to_python<MetadataKey>(rsp_non_const);
    bp::class_<MetadataKey, boost::noncopyable>
        (
         "MetadataKey",
         bp::no_init
        )
        .def("raw_name", &MetadataKey::raw_name,
                "raw_name() -> string\n"
                "NEED_DOC"
                )

        .def("human_name", &MetadataKey::human_name,
                "human_name() -> string\n"
                "NEED_DOC"
                )
        ;

    /**
     * MetadataPackageIDKey
     */
    bp::register_ptr_to_python<tr1::shared_ptr<const MetadataPackageIDKey> >();
    bp::implicitly_convertible<tr1::shared_ptr<MetadataPackageIDKeyWrapper>,
            tr1::shared_ptr<MetadataKey> >();
    bp::class_<MetadataPackageIDKeyWrapper, tr1::shared_ptr<MetadataPackageIDKeyWrapper>,
            bp::bases<MetadataKey>, boost::noncopyable>
        (
         "MetadataPackageIDKey",
         "NEED_DOC\n"
         "This class can be subclassed in Python.",
         bp::init<const std::string &, const std::string &, MetadataKeyType>(
             "__init__(raw_name, human_name, MetadataKeyType)"
             )
        )
        .def("value", bp::pure_virtual(&MetadataPackageIDKey::value),
                "value() -> PackageID\n"
                "NEED_DOC"
                )
        ;

    /**
     * MetadataStringKey
     */
    bp::register_ptr_to_python<tr1::shared_ptr<const MetadataStringKey> >();
    bp::implicitly_convertible<tr1::shared_ptr<MetadataStringKeyWrapper>,
            tr1::shared_ptr<MetadataKey> >();
    bp::class_<MetadataStringKeyWrapper, tr1::shared_ptr<MetadataStringKeyWrapper>,
            bp::bases<MetadataKey>, boost::noncopyable>
        (
         "MetadataStringKey",
         "NEED_DOC\n"
         "This class can be subclassed in Python.",
         bp::init<const std::string &, const std::string &, MetadataKeyType>(
             "__init__(raw_name, human_name, MetadataKeyType)"
             )
        )
        .def("value", &MetadataStringKey::value,
                "value() -> string\n"
                "NEED_DOC"
                )
        ;

    /**
     * MetadataTimeKey
     */
    bp::register_ptr_to_python<tr1::shared_ptr<const MetadataTimeKey> >();
    bp::implicitly_convertible<tr1::shared_ptr<MetadataTimeKeyWrapper>,
            tr1::shared_ptr<MetadataKey> >();
    bp::class_<MetadataTimeKeyWrapper, tr1::shared_ptr<MetadataTimeKeyWrapper>,
            bp::bases<MetadataKey>, boost::noncopyable>
        (
         "MetadataTimeKey",
         "NEED_DOC\n"
         "This class can be subclassed in Python.",
         bp::init<const std::string &, const std::string &, MetadataKeyType>(
             "__init__(raw_name, human_name, MetadataKeyType)"
             )
        )
        .def("value", bp::pure_virtual(&MetadataTimeKey::value),
                "value() -> int\n"
                )
        ;

    /**
     * MetadataFSEntryKey
     */
    bp::register_ptr_to_python<tr1::shared_ptr<const MetadataFSEntryKey> >();
    bp::implicitly_convertible<tr1::shared_ptr<MetadataFSEntryKeyWrapper>,
            tr1::shared_ptr<MetadataKey> >();
    bp::class_<MetadataFSEntryKeyWrapper, tr1::shared_ptr<MetadataFSEntryKeyWrapper>,
            bp::bases<MetadataKey>, boost::noncopyable>
        (
         "MetadataFSEntryKey",
         "NEED_DOC\n"
         "This class can be subclassed in Python.",
         bp::init<const std::string &, const std::string &, MetadataKeyType>(
             "__init__(raw_name, human_name, MetadataKeyType)"
             )
        )
        .def("value", bp::pure_virtual(&MetadataFSEntryKey::value),
                "value() -> FSEntry\n"
                )
        ;

    /**
     * MetadataContentsKey
     */
    bp::register_ptr_to_python<tr1::shared_ptr<const MetadataContentsKey> >();
    bp::implicitly_convertible<tr1::shared_ptr<MetadataContentsKeyWrapper>,
            tr1::shared_ptr<MetadataKey> >();
    bp::class_<MetadataContentsKeyWrapper, tr1::shared_ptr<MetadataContentsKeyWrapper>,
            bp::bases<MetadataKey>, boost::noncopyable>
        (
         "MetadataContentsKey",
         "NEED_DOC\n"
         "This class can be subclassed in Python.",
         bp::init<const std::string &, const std::string &, MetadataKeyType>(
             "__init__(raw_name, human_name, MetadataKeyType)"
             )
        )
        .def("value", bp::pure_virtual(&MetadataContentsKey::value),
                "value() -> Contents\n"
                "NEED_DOC"
                )

        //Work around epydoc bug
        .def("raw_name", &MetadataKey::raw_name,
                "raw_name() -> string\n"
                "NEED_DOC"
                )

        //Work around epydoc bug
        .def("human_name", &MetadataKey::human_name,
                "human_name() -> string\n"
                "NEED_DOC"
                )
        ;

    /**
     * MetadataRepositoryMaskInfoKey
     */
    bp::register_ptr_to_python<tr1::shared_ptr<const MetadataRepositoryMaskInfoKey> >();
    bp::implicitly_convertible<tr1::shared_ptr<MetadataRepositoryMaskInfoKeyWrapper>,
            tr1::shared_ptr<MetadataKey> >();
    bp::class_<MetadataRepositoryMaskInfoKeyWrapper, tr1::shared_ptr<MetadataRepositoryMaskInfoKeyWrapper>,
            bp::bases<MetadataKey>, boost::noncopyable>
        (
         "MetadataRepositoryMaskInfoKey",
         "NEED_DOC\n"
         "This class can be subclassed in Python.",
         bp::init<const std::string &, const std::string &, MetadataKeyType>(
             "__init__(raw_name, human_name, MetadataKeyType)"
             )
        )
        .def("value", bp::pure_virtual(&MetadataRepositoryMaskInfoKey::value),
                "value() -> RepositoryMaskInfo\n"
                )
        ;

    /**
     * MetadataSetKeys
     */
    class_set_key<KeywordNameSet>("KeywordNameIterable");
    class_set_key<UseFlagNameSet>("UseFlagNameIterable");
    class_set_key<IUseFlagSet>("IUseFlagIterable");
    class_set_key<Set<std::string> >("StringIterable");

    /**
     * MetadataSpecTreeKeys
     */
    class_spec_tree_key<LicenseSpecTree>("LicenseSpecTree");
    class_spec_tree_key<ProvideSpecTree>("ProvideSpecTree");
    class_spec_tree_key<DependencySpecTree>("DependencySpecTree");
    class_spec_tree_key<RestrictSpecTree>("RestrictSpecTree");
    class_spec_tree_key<URISpecTree>("URISpecTree");
}

