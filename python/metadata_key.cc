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
#include <python/exception.hh>

#include <paludis/metadata_key.hh>
#include <paludis/name.hh>
#include <paludis/formatter.hh>
#include <paludis/dep_label.hh>
#include <paludis/environment.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/set.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/sequence.hh>

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

        void visit(const MetadataSetKey<FSEntrySequence> & k)
        {
            obj = bp::object(tr1::static_pointer_cast<const MetadataSetKey<FSEntrySequence> >(_m_ptr));
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

        void visit(const MetadataSpecTreeKey<FetchableURISpecTree> & k)
        {
            obj = bp::object(tr1::static_pointer_cast<const MetadataSpecTreeKey<FetchableURISpecTree> >(_m_ptr));
        }

        void visit(const MetadataSpecTreeKey<SimpleURISpecTree> & k)
        {
            obj = bp::object(tr1::static_pointer_cast<const MetadataSpecTreeKey<SimpleURISpecTree> >(_m_ptr));
        }

        void visit(const MetadataSetKey<PackageIDSequence> & k)
        {
            obj = bp::object(tr1::static_pointer_cast<const MetadataSetKey<PackageIDSequence> >(_m_ptr));
        }

        void visit(const MetadataSectionKey & k)
        {
            obj = bp::object(tr1::static_pointer_cast<const MetadataSectionKey>(_m_ptr));
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

struct MetadataSectionKeyWrapper :
    MetadataSectionKey,
    bp::wrapper<MetadataSectionKey>
{
    MetadataSectionKeyWrapper(const std::string & r, const std::string & h, const MetadataKeyType t) :
        MetadataSectionKey(r, h, t)
    {
    }

    static PyObject *
    find_metadata(const MetadataSectionKey & self, const std::string & key)
    {
        MetadataSectionKey::MetadataConstIterator i(self.find_metadata(key));
        if (i != self.end_metadata())
            return bp::incref(bp::object(*i).ptr());
        else
            return Py_None;
    }

    virtual void need_keys_added() const
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("need_keys_added"))
            f();
        else
            throw PythonMethodNotImplemented("MetadataSectionKey", "need_keys_added");
    }

    virtual const tr1::shared_ptr<const MetadataStringKey> title_key() const
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("title_key"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataSectionKey", "title_key");
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

    virtual time_t value() const
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

    std::string pretty_print_flat(const Formatter<typename C_::value_type> & formatter) const
        PALUDIS_ATTRIBUTE((warn_unused_result))
    {
        Lock l(get_mutex());

        if (bp::override f = this->get_override("pretty_print_flat"))
            return f(boost::cref(formatter));
        else
            throw PythonMethodNotImplemented("MetadataSetKey", "pretty_print_flat");
    }
};

template <>
struct MetadataSetKeyWrapper<IUseFlagSet> :
    MetadataSetKey<IUseFlagSet>,
    bp::wrapper<MetadataSetKey<IUseFlagSet> >
{
    MetadataSetKeyWrapper(const std::string & r, const std::string & h, const MetadataKeyType t) :
        MetadataSetKey<IUseFlagSet>(r, h, t)
    {
    }

    virtual const tr1::shared_ptr<const IUseFlagSet> value() const
        PALUDIS_ATTRIBUTE((warn_unused_result))
    {
        Lock l(get_mutex());

        if (bp::override f = this->get_override("value"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataSetKey", "value");
    }

    std::string pretty_print_flat(const Formatter<IUseFlag> & formatter) const
        PALUDIS_ATTRIBUTE((warn_unused_result))
    {
        Lock l(get_mutex());

        if (bp::override f = this->get_override("pretty_print_flat"))
            return f(boost::cref(formatter));
        else
            throw PythonMethodNotImplemented("MetadataSetKey", "pretty_print_flat");
    }

    std::string pretty_print_flat_with_comparison(
            const Environment * const e,
            const tr1::shared_ptr<const PackageID> & pid,
            const Formatter<IUseFlag> & formatter) const
        PALUDIS_ATTRIBUTE((warn_unused_result))
    {
        Lock l(get_mutex());

        if (bp::override f = this->get_override("pretty_print_flat_with_comparison"))
            return f(boost::cref(e), pid, boost::cref(formatter));
        else
            throw PythonMethodNotImplemented("MetadataSetKey", "pretty_print_flat_with_comparison");
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

    virtual std::string pretty_print(const typename C_::ItemFormatter & formatter) const
        PALUDIS_ATTRIBUTE((warn_unused_result))
    {
        Lock l(get_mutex());

        if (bp::override f = this->get_override("pretty_print"))
            return f(boost::cref(formatter));
        else
            throw PythonMethodNotImplemented("MetadataSpecTreeKey", "pretty_print");
    }

    virtual std::string pretty_print_flat(const typename C_::ItemFormatter & formatter) const
        PALUDIS_ATTRIBUTE((warn_unused_result))
    {
        Lock l(get_mutex());

        if (bp::override f = this->get_override("pretty_print_flat"))
            return f(boost::cref(formatter));
        else
            throw PythonMethodNotImplemented("MetadataSpecTreeKey", "pretty_print_flat");
    }
};

template <>
struct MetadataSpecTreeKeyWrapper<FetchableURISpecTree> :
    MetadataSpecTreeKey<FetchableURISpecTree>,
    bp::wrapper<MetadataSpecTreeKey<FetchableURISpecTree> >
{
    MetadataSpecTreeKeyWrapper(const std::string & r, const std::string & h, const MetadataKeyType t) :
        MetadataSpecTreeKey<FetchableURISpecTree>(r, h, t)
    {
    }

    virtual const tr1::shared_ptr<const FetchableURISpecTree::ConstItem> value() const
        PALUDIS_ATTRIBUTE((warn_unused_result))
    {
        Lock l(get_mutex());

        if (bp::override f = this->get_override("value"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataSpecTreeKey", "value");
    }

    virtual std::string pretty_print(const FetchableURISpecTree::ItemFormatter & formatter) const
        PALUDIS_ATTRIBUTE((warn_unused_result))
    {
        Lock l(get_mutex());

        if (bp::override f = this->get_override("pretty_print"))
            return f(boost::cref(formatter));
        else
            throw PythonMethodNotImplemented("MetadataSpecTreeKey", "pretty_print");
    }

    virtual std::string pretty_print_flat(const FetchableURISpecTree::ItemFormatter & formatter) const
        PALUDIS_ATTRIBUTE((warn_unused_result))
    {
        Lock l(get_mutex());

        if (bp::override f = this->get_override("pretty_print_flat"))
            return f(boost::cref(formatter));
        else
            throw PythonMethodNotImplemented("MetadataSpecTreeKey", "pretty_print_flat");
    }

    virtual const tr1::shared_ptr<const URILabel> initial_label() const
        PALUDIS_ATTRIBUTE((warn_unused_result))
    {
        Lock l(get_mutex());

        if (bp::override f = this->get_override("initial_label"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataSpecTreeKey", "initial_label");
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
                    "A MetadataSetKey is a MetadataKey that holds a Set of some kind of item\n"
                    "as its value.\n\n"

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
                ("value() -> " + set + "\n"
                 "Fetch our value.").c_str()
           );

        def("pretty_print_flat", bp::pure_virtual(&MetadataSetKey<C_>::pretty_print_flat),
                ("pretty_print_flat(" + set +"Formatter) -> string\n"
                 "Return a single-line formatted version of our value, using the\n"
                 "supplied Formatter to format individual items.").c_str()
           );
    }
};

template <>
struct class_set_key<IUseFlagSet> :
    bp::class_<MetadataSetKeyWrapper<IUseFlagSet>, tr1::shared_ptr<MetadataSetKeyWrapper<IUseFlagSet> >,
        bp::bases<MetadataKey>, boost::noncopyable>
{
    class_set_key(const std::string & set) :
        bp::class_<MetadataSetKeyWrapper<IUseFlagSet>, tr1::shared_ptr<MetadataSetKeyWrapper<IUseFlagSet> >,
            bp::bases<MetadataKey>, boost::noncopyable>(
                    ("Metadata" + set + "Key").c_str(),
                    "A MetadataSetKey is a MetadataKey that holds a Set of some kind of item\n"
                    "as its value.\n\n"

                    "This class can be subclassed in Python.",
                    bp::init<const std::string &, const std::string &, MetadataKeyType>(
                        "__init__(raw_name, human_name, MetadataKeyType)"
                        )
                    )
    {
        bp::register_ptr_to_python<tr1::shared_ptr<const MetadataSetKey<IUseFlagSet> > >();
        bp::implicitly_convertible<tr1::shared_ptr<MetadataSetKeyWrapper<IUseFlagSet> >,
                tr1::shared_ptr<MetadataKey> >();

        def("value", bp::pure_virtual(&MetadataSetKey<IUseFlagSet>::value),
                ("value() -> " + set + "\n"
                 "Fetch our value.").c_str()
           );

        def("pretty_print_flat", bp::pure_virtual(&MetadataSetKey<IUseFlagSet>::pretty_print_flat),
                ("pretty_print_flat(" + set +"Formatter) -> string\n"
                 "Return a single-line formatted version of our value, using the\n"
                 "supplied Formatter to format individual items.").c_str()
           );

        def("pretty_print_flat_with_comparison",
                bp::pure_virtual(&MetadataSetKey<IUseFlagSet>::pretty_print_flat_with_comparison),
                ("pretty_print_flat_with_comparison(Environment, PackageID, " + set +"Formatter) -> string\n"
                 "Return a single-line formatted version of our value, using the\n"
                 "supplied Formatter to format individual items, and the supplied\n"
                 "PackageID to decorate using format::Added and format::Changed as\n"
                 "appropriate.").c_str()
           );
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
                    "A MetadataSpecTreeKey is a MetadataKey that holds a spec tree of some\n"
                    "kind as its value.\n\n"

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
                ("value() -> " + spec_tree + "\n"
                 "Fetch our value").c_str()
           );

        def("pretty_print", bp::pure_virtual(&MetadataSpecTreeKey<C_>::pretty_print),
                ("pretty_print(" + spec_tree + "Formatter) -> string\n"
                 "Return a multiline-line indented and formatted version of our\n"
                 "value, using the supplied Formatter to format individual items.").c_str()
           );

        def("pretty_print_flat", bp::pure_virtual(&MetadataSpecTreeKey<C_>::pretty_print_flat),
                ("pretty_print_flat(" + spec_tree + "Formatter) -> string\n"
                 "Return a single-line formatted version of our value, using the\n"
                 "supplied Formatter to format individual items.").c_str()
           );
    }
};

template <>
struct class_spec_tree_key<FetchableURISpecTree> :
    bp::class_<MetadataSpecTreeKeyWrapper<FetchableURISpecTree>,
        tr1::shared_ptr<MetadataSpecTreeKeyWrapper<FetchableURISpecTree> >,
        bp::bases<MetadataKey>, boost::noncopyable>
{
    class_spec_tree_key(const std::string & spec_tree) :
        bp::class_<MetadataSpecTreeKeyWrapper<FetchableURISpecTree>, 
            tr1::shared_ptr<MetadataSpecTreeKeyWrapper<FetchableURISpecTree> >,
            bp::bases<MetadataKey>, boost::noncopyable>(
                    ("Metadata" + spec_tree + "Key").c_str(),
                    "A MetadataSpecTreeKey is a MetadataKey that holds a spec tree of some\n"
                    "kind as its value.\n\n"

                    "This class can be subclassed in Python.",
                    bp::init<const std::string &, const std::string &, MetadataKeyType>(
                        "__init__(raw_name, human_name, MetadataKeyType)"
                        )
                    )
    {
        bp::register_ptr_to_python<tr1::shared_ptr<const MetadataSpecTreeKey<FetchableURISpecTree> > >();
        bp::implicitly_convertible<tr1::shared_ptr<MetadataSpecTreeKeyWrapper<FetchableURISpecTree> >,
                tr1::shared_ptr<MetadataKey> >();

        def("value", bp::pure_virtual(&MetadataSpecTreeKey<FetchableURISpecTree>::value),
                ("value() -> " + spec_tree + "\n"
                 "Fetch our value").c_str()
           );

        def("pretty_print", bp::pure_virtual(&MetadataSpecTreeKey<FetchableURISpecTree>::pretty_print),
                ("pretty_print(" + spec_tree + "Formatter) -> string\n"
                 "Return a multiline-line indented and formatted version of our\n"
                 "value, using the supplied Formatter to format individual items.").c_str()
           );

        def("pretty_print_flat", bp::pure_virtual(&MetadataSpecTreeKey<FetchableURISpecTree>::pretty_print_flat),
                ("pretty_print_flat(" + spec_tree + "Formatter) -> string\n"
                 "Return a single-line formatted version of our value, using the\n"
                 "supplied Formatter to format individual items.").c_str()
           );

        def("initial_label", bp::pure_virtual(&MetadataSpecTreeKey<FetchableURISpecTree>::initial_label),
                "initial_label() -> URILabel\n"
                "Return a URILabel that represents the initial label to use when\n"
                "deciding the behaviour of individual items in the heirarchy."
           );
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
         "A MetadataKey is a generic key that contains a particular piece of\n"
         "information about a PackageID instance.\n\n"

         "A basic MetadataKey has:\n\n"

         "- A raw name. This is in a repository-defined format designed to closely\n"
         "  represent the internal name. For example, ebuilds and VDB IDs use\n"
         "  raw names like 'DESCRIPTION' and 'KEYWORDS', whereas CRAN uses names\n"
         "  like 'Title' and 'BundleDescription'. The raw name is unique in a\n"
         "  PackageID.\n\n"

         "- A human name. This is the name that should be used when outputting\n"
         "  normally for a human to read.\n\n"

         "- A MetadataKeyType. This is a hint to clients as to whether the key\n"
         "  should be displayed when outputting information about a package ID.\n\n"

         "Subclasses provide additional information, including the 'value' of the\n"
         "key.",
         bp::no_init
        )
        .def("raw_name", bp::pure_virtual(&MetadataKey::raw_name),
                "raw_name() -> string\n"
                "Fetch our raw name."
                )

        .def("human_name", bp::pure_virtual(&MetadataKey::human_name),
                "human_name() -> string\n"
                "Fetch our human name."
                )

        .def("type", bp::pure_virtual(&MetadataKey::type),
                "type() -> MetadataKeyType\n"
                "Fetch our key type."
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
         "A MetadataPackageIDKey is a MetadataKey that has a PackageID as its\n"
         "value.\n\n"

         "This class can be subclassed in Python.",
         bp::init<const std::string &, const std::string &, MetadataKeyType>(
             "__init__(raw_name, human_name, MetadataKeyType)"
             )
        )
        .def("value", bp::pure_virtual(&MetadataPackageIDKey::value),
                "value() -> PackageID\n"
                "Fetch our value."
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
         "A MetadataStringKey is a MetadataKey that has a std::string as its\n"
         "value.\n\n"

         "This class can be subclassed in Python.",
         bp::init<const std::string &, const std::string &, MetadataKeyType>(
             "__init__(raw_name, human_name, MetadataKeyType)"
             )
        )
        .def("value", bp::pure_virtual(&MetadataStringKey::value),
                "value() -> string\n"
                "Fetch our value."
                )
        ;

    /**
     * MetadataSectionKey
     */
    bp::register_ptr_to_python<tr1::shared_ptr<const MetadataSectionKey> >();
    bp::implicitly_convertible<tr1::shared_ptr<MetadataSectionKeyWrapper>,
            tr1::shared_ptr<MetadataKey> >();
    bp::class_<MetadataSectionKeyWrapper, tr1::shared_ptr<MetadataSectionKeyWrapper>,
            bp::bases<MetadataKey>, boost::noncopyable>
        (
         "MetadataSectionKey",
         "A MetadataSectionKey holds a number of other MetadataKey instances, and\n"
         "may have a title.\n\n"

         "This class can be subclassed in Python.",
         bp::init<const std::string &, const std::string &, MetadataKeyType>(
             "__init__(raw_name, human_name, MetadataKeyType)"
             )
        )
        .add_property("metadata", bp::range(&MetadataSectionKey::begin_metadata, &MetadataSectionKey::end_metadata),
                "[ro] Iterable of MetadataKey\n"
                "NEED_DOC"
                )

        .def("find_metadata", &MetadataSectionKeyWrapper::find_metadata,
                "find_metadata(string) -> MetadataKey\n"
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
         "A MetadataTimeKey is a MetadataKey that has a int(time_t) as its value.\n\n"

         "This class can be subclassed in Python.",
         bp::init<const std::string &, const std::string &, MetadataKeyType>(
             "__init__(raw_name, human_name, MetadataKeyType)"
             )
        )
        .def("value", bp::pure_virtual(&MetadataTimeKey::value),
                "value() -> int\n"
                "Fetch our value."
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
         "A MetadataFSEntryKey is a MetadataKey that has an string(FSEntry) as its value.\n\n"

         "This class can be subclassed in Python.",
         bp::init<const std::string &, const std::string &, MetadataKeyType>(
             "__init__(raw_name, human_name, MetadataKeyType)"
             )
        )
        .def("value", bp::pure_virtual(&MetadataFSEntryKey::value),
                "value() -> FSEntry\n"
                "Fetch our value."
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
         "A MetadataContentsKey is a MetadataKey that holds a Contents heirarchy.\n\n"

         "This class can be subclassed in Python.",
         bp::init<const std::string &, const std::string &, MetadataKeyType>(
             "__init__(raw_name, human_name, MetadataKeyType)"
             )
        )
        .def("value", bp::pure_virtual(&MetadataContentsKey::value),
                "value() -> Contents\n"
                "Fetch our value."
                )

        //Work around epydoc bug
        .def("raw_name", bp::pure_virtual(&MetadataKey::raw_name),
                "raw_name() -> string\n"
                "Fetch our raw name."
                )

        //Work around epydoc bug
        .def("human_name", bp::pure_virtual(&MetadataKey::human_name),
                "human_name() -> string\n"
                "Fetch our human name."
                )

        //Work around epydoc bug
        .def("type", bp::pure_virtual(&MetadataKey::type),
                "type() -> MetadataKeyType\n"
                "Fetch our key type."
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
         "A MetadataRepositoryMaskInfoKey is a MetadataKey that holds\n"
         "RepositoryMaskInfo as its value.\n\n"

         "This class can be subclassed in Python.",
         bp::init<const std::string &, const std::string &, MetadataKeyType>(
             "__init__(raw_name, human_name, MetadataKeyType)"
             )
        )
        .def("value", bp::pure_virtual(&MetadataRepositoryMaskInfoKey::value),
                "value() -> RepositoryMaskInfo\n"
                "Fetch our value."
                )
        ;

    /**
     * MetadataSetKeys
     */
    class_set_key<KeywordNameSet>("KeywordNameIterable");
    class_set_key<UseFlagNameSet>("UseFlagNameIterable");
    class_set_key<IUseFlagSet>("IUseFlagIterable");
    class_set_key<Set<std::string> >("StringIterable");
    class_set_key<FSEntrySequence>("FSEntryIterable");

    /**
     * MetadataSpecTreeKeys
     */
    class_spec_tree_key<LicenseSpecTree>("LicenseSpecTree");
    class_spec_tree_key<ProvideSpecTree>("ProvideSpecTree");
    class_spec_tree_key<DependencySpecTree>("DependencySpecTree");
    class_spec_tree_key<RestrictSpecTree>("RestrictSpecTree");
    class_spec_tree_key<SimpleURISpecTree>("SimpleURISpecTree");
    class_spec_tree_key<FetchableURISpecTree>("FetchableURISpecTree");
}

