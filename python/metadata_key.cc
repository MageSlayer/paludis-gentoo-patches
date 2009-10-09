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
#include <paludis/util/set.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/sequence.hh>

using namespace paludis;
using namespace paludis::python;
namespace bp = boost::python;

class MetadataKeySptrToPythonVisitor
{
    private:
        const std::tr1::shared_ptr<const MetadataKey> & _m_ptr;

    public:
        boost::python::object obj;

        MetadataKeySptrToPythonVisitor(const std::tr1::shared_ptr<const MetadataKey> & m_ptr) :
            _m_ptr(m_ptr)
        {
        }

        void visit(const MetadataValueKey<std::tr1::shared_ptr<const PackageID> > & k)
        {
            obj = bp::object(std::tr1::static_pointer_cast<const MetadataValueKey<std::tr1::shared_ptr<const PackageID> > >(_m_ptr));
        }

        void visit(const MetadataValueKey<std::string> & k)
        {
            obj = bp::object(std::tr1::static_pointer_cast<const MetadataValueKey<std::string> >(_m_ptr));
        }

        void visit(const MetadataValueKey<SlotName> & k)
        {
            obj = bp::object(std::tr1::static_pointer_cast<const MetadataValueKey<SlotName> >(_m_ptr));
        }

        void visit(const MetadataValueKey<long> & k)
        {
            obj = bp::object(std::tr1::static_pointer_cast<const MetadataValueKey<long> >(_m_ptr));
        }

        void visit(const MetadataValueKey<bool> & k)
        {
            obj = bp::object(std::tr1::static_pointer_cast<const MetadataValueKey<bool> >(_m_ptr));
        }

        void visit(const MetadataTimeKey & k)
        {
            obj = bp::object(std::tr1::static_pointer_cast<const MetadataTimeKey>(_m_ptr));
        }

        void visit(const MetadataValueKey<std::tr1::shared_ptr<const Contents> > & k)
        {
            obj = bp::object(std::tr1::static_pointer_cast<const MetadataValueKey<std::tr1::shared_ptr<const Contents> > >(_m_ptr));
        }

        void visit(const MetadataValueKey<std::tr1::shared_ptr<const Choices> > & k)
        {
            obj = bp::object(std::tr1::static_pointer_cast<const MetadataValueKey<std::tr1::shared_ptr<const Choices> > >(_m_ptr));
        }

        void visit(const MetadataValueKey<std::tr1::shared_ptr<const RepositoryMaskInfo> > & k)
        {
            obj = bp::object(std::tr1::static_pointer_cast<const MetadataValueKey<std::tr1::shared_ptr<const RepositoryMaskInfo> > >(_m_ptr));
        }

        void visit(const MetadataValueKey<FSEntry> & k)
        {
            obj = bp::object(std::tr1::static_pointer_cast<const MetadataValueKey<FSEntry> >(_m_ptr));
        }

        void visit(const MetadataCollectionKey<KeywordNameSet> & k)
        {
            obj = bp::object(std::tr1::static_pointer_cast<const MetadataCollectionKey<KeywordNameSet> >(_m_ptr));
        }

        void visit(const MetadataCollectionKey<Set<std::string> > & k)
        {
            obj = bp::object(std::tr1::static_pointer_cast<const MetadataCollectionKey<Set<std::string> > >(_m_ptr));
        }

        void visit(const MetadataCollectionKey<Sequence<std::string> > & k)
        {
            obj = bp::object(std::tr1::static_pointer_cast<const MetadataCollectionKey<Sequence<std::string> > >(_m_ptr));
        }

        void visit(const MetadataCollectionKey<FSEntrySequence> & k)
        {
            obj = bp::object(std::tr1::static_pointer_cast<const MetadataCollectionKey<FSEntrySequence> >(_m_ptr));
        }

        void visit(const MetadataSpecTreeKey<LicenseSpecTree> & k)
        {
            obj = bp::object(std::tr1::static_pointer_cast<const MetadataSpecTreeKey<LicenseSpecTree> >(_m_ptr));
        }

        void visit(const MetadataSpecTreeKey<ProvideSpecTree> & k)
        {
            obj = bp::object(std::tr1::static_pointer_cast<const MetadataSpecTreeKey<ProvideSpecTree> >(_m_ptr));
        }

        void visit(const MetadataSpecTreeKey<DependencySpecTree> & k)
        {
            obj = bp::object(std::tr1::static_pointer_cast<const MetadataSpecTreeKey<DependencySpecTree> >(_m_ptr));
        }

        void visit(const MetadataSpecTreeKey<PlainTextSpecTree> & k)
        {
            obj = bp::object(std::tr1::static_pointer_cast<const MetadataSpecTreeKey<PlainTextSpecTree> >(_m_ptr));
        }

        void visit(const MetadataSpecTreeKey<FetchableURISpecTree> & k)
        {
            obj = bp::object(std::tr1::static_pointer_cast<const MetadataSpecTreeKey<FetchableURISpecTree> >(_m_ptr));
        }

        void visit(const MetadataSpecTreeKey<SimpleURISpecTree> & k)
        {
            obj = bp::object(std::tr1::static_pointer_cast<const MetadataSpecTreeKey<SimpleURISpecTree> >(_m_ptr));
        }

        void visit(const MetadataCollectionKey<PackageIDSequence> & k)
        {
            obj = bp::object(std::tr1::static_pointer_cast<const MetadataCollectionKey<PackageIDSequence> >(_m_ptr));
        }

        void visit(const MetadataSectionKey & k)
        {
            obj = bp::object(std::tr1::static_pointer_cast<const MetadataSectionKey>(_m_ptr));
        }
};

struct MetadataKeySptrToPython
{
    MetadataKeySptrToPython()
    {
        bp::to_python_converter<std::tr1::shared_ptr<const MetadataKey>, MetadataKeySptrToPython>();
    }

    static PyObject *
    convert(const std::tr1::shared_ptr<const MetadataKey> & m)
    {
        MetadataKeySptrToPythonVisitor v(m);
        m->accept(v);
        return bp::incref(v.obj.ptr());
    }
};

struct MetadataPackageIDKeyWrapper :
    MetadataValueKey<std::tr1::shared_ptr<const PackageID> > ,
    bp::wrapper<MetadataValueKey<std::tr1::shared_ptr<const PackageID> > >
{
    virtual const std::tr1::shared_ptr<const PackageID> value() const
        PALUDIS_ATTRIBUTE((warn_unused_result))
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("value"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataPackageIDKey", "value");
    }

    virtual std::string pretty_print(const Formatter<PackageID> &) const
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("pretty_print"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataPackageIDKey", "pretty_print");
    }

    virtual const std::string raw_name() const
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("raw_name"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataPackageIDKey", "raw_name");
    }

    virtual const std::string human_name() const
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("human_name"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataPackageIDKey", "human_name");
    }

    virtual MetadataKeyType type() const
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("type"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataPackageIDKey", "type");
    }
};

struct MetadataStringKeyWrapper :
    MetadataValueKey<std::string> ,
    bp::wrapper<MetadataValueKey<std::string> >
{
    virtual const std::string value() const
        PALUDIS_ATTRIBUTE((warn_unused_result))
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("value"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataStringKey", "value");
    }

    virtual const std::string raw_name() const
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("raw_name"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataStringKey", "raw_name");
    }

    virtual const std::string human_name() const
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("human_name"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataStringKey", "human_name");
    }

    virtual MetadataKeyType type() const
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("type"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataStringKey", "type");
    }
};

struct MetadataSlotNameKeyWrapper :
    MetadataValueKey<SlotName> ,
    bp::wrapper<MetadataValueKey<SlotName> >
{
    virtual const SlotName value() const
        PALUDIS_ATTRIBUTE((warn_unused_result))
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("value"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataSlotNameKey", "value");
    }

    virtual const std::string raw_name() const
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("raw_name"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataSlotNameKey", "raw_name");
    }

    virtual const std::string human_name() const
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("human_name"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataSlotNameKey", "human_name");
    }

    virtual MetadataKeyType type() const
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("type"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataSlotNameKey", "type");
    }
};

struct MetadataSectionKeyWrapper :
    MetadataSectionKey,
    bp::wrapper<MetadataSectionKey>
{
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

    virtual const std::tr1::shared_ptr<const MetadataValueKey<std::string> > title_key() const
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("title_key"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataSectionKey", "title_key");
    }

    virtual const std::string raw_name() const
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("raw_name"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataSectionKey", "raw_name");
    }

    virtual const std::string human_name() const
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("human_name"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataSectionKey", "human_name");
    }

    virtual MetadataKeyType type() const
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("type"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataSectionKey", "type");
    }
};

struct MetadataTimeKeyWrapper :
    MetadataTimeKey,
    bp::wrapper<MetadataTimeKey>
{
    virtual time_t value() const
        PALUDIS_ATTRIBUTE((warn_unused_result))
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("value"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataTimeKey", "value");
    }

    virtual const std::string raw_name() const
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("raw_name"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataTimeKey", "raw_name");
    }

    virtual const std::string human_name() const
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("human_name"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataTimeKey", "human_name");
    }

    virtual MetadataKeyType type() const
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("type"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataTimeKey", "type");
    }
};

struct MetadataContentsKeyWrapper :
    MetadataValueKey<std::tr1::shared_ptr<const Contents> > ,
    bp::wrapper<MetadataValueKey<std::tr1::shared_ptr<const Contents> > >
{
    virtual const std::tr1::shared_ptr<const Contents> value() const
        PALUDIS_ATTRIBUTE((warn_unused_result))
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("value"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataContentsKey", "value");
    }

    virtual const std::string raw_name() const
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("raw_name"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataContentsKey", "raw_name");
    }

    virtual const std::string human_name() const
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("human_name"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataContentsKey", "human_name");
    }

    virtual MetadataKeyType type() const
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("type"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataContentsKey", "type");
    }
};

struct MetadataChoicesKeyWrapper :
    MetadataValueKey<std::tr1::shared_ptr<const Choices> > ,
    bp::wrapper<MetadataValueKey<std::tr1::shared_ptr<const Choices> > >
{
    virtual const std::tr1::shared_ptr<const Choices> value() const
        PALUDIS_ATTRIBUTE((warn_unused_result))
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("value"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataChoicesKey", "value");
    }

    virtual const std::string raw_name() const
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("raw_name"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataChoicesKey", "raw_name");
    }

    virtual const std::string human_name() const
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("human_name"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataChoicesKey", "human_name");
    }

    virtual MetadataKeyType type() const
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("type"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataChoicesKey", "type");
    }
};

struct MetadataFSEntryKeyWrapper :
    MetadataValueKey<FSEntry> ,
    bp::wrapper<MetadataValueKey<FSEntry> >
{
    virtual const FSEntry value() const
        PALUDIS_ATTRIBUTE((warn_unused_result))
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("value"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataValueKey<FSEntry> ", "value");
    }

    virtual const std::string raw_name() const
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("raw_name"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataFSEntryKey", "raw_name");
    }

    virtual const std::string human_name() const
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("human_name"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataFSEntryKey", "human_name");
    }

    virtual MetadataKeyType type() const
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("type"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataFSEntryKey", "type");
    }
};

struct MetadataRepositoryMaskInfoKeyWrapper :
    MetadataValueKey<std::tr1::shared_ptr<const RepositoryMaskInfo> > ,
    bp::wrapper<MetadataValueKey<std::tr1::shared_ptr<const RepositoryMaskInfo> > >
{
    virtual const std::tr1::shared_ptr<const RepositoryMaskInfo> value() const
        PALUDIS_ATTRIBUTE((warn_unused_result))
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("value"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataRepositoryMaskInfoKey", "value");
    }

    virtual const std::string raw_name() const
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("raw_name"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataRepositoryMaskInfoKey", "raw_name");
    }

    virtual const std::string human_name() const
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("human_name"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataRepositoryMaskInfoKey", "human_name");
    }

    virtual MetadataKeyType type() const
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("type"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataRepositoryMaskInfoKey", "type");
    }
};

template <typename C_>
struct MetadataCollectionKeyWrapper :
    MetadataCollectionKey<C_>,
    bp::wrapper<MetadataCollectionKey<C_> >
{
    virtual const std::tr1::shared_ptr<const C_> value() const
        PALUDIS_ATTRIBUTE((warn_unused_result))
    {
        Lock l(get_mutex());

        if (bp::override f = this->get_override("value"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataCollectionKey", "value");
    }

    std::string pretty_print_flat(const Formatter<
            typename std::tr1::remove_const<
                    typename RemoveSharedPtr<typename C_::value_type>::Type>::type> & formatter) const
        PALUDIS_ATTRIBUTE((warn_unused_result))
    {
        Lock l(get_mutex());

        if (bp::override f = this->get_override("pretty_print_flat"))
            return f(boost::cref(formatter));
        else
            throw PythonMethodNotImplemented("MetadataCollectionKey", "pretty_print_flat");
    }

    virtual const std::string raw_name() const
    {
        Lock l(get_mutex());

        if (bp::override f = this->get_override("raw_name"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataCollectionKey", "raw_name");
    }

    virtual const std::string human_name() const
    {
        Lock l(get_mutex());

        if (bp::override f = this->get_override("human_name"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataCollectionKey", "human_name");
    }

    virtual MetadataKeyType type() const
    {
        Lock l(get_mutex());

        if (bp::override f = this->get_override("type"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataCollectionKey", "type");
    }
};

template <typename C_>
struct MetadataSpecTreeKeyWrapper :
    MetadataSpecTreeKey<C_>,
    bp::wrapper<MetadataSpecTreeKey<C_> >
{
    virtual const std::tr1::shared_ptr<const C_> value() const
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

    virtual const std::string raw_name() const
    {
        Lock l(get_mutex());

        if (bp::override f = this->get_override("raw_name"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataSpecTreeKey", "raw_name");
    }

    virtual const std::string human_name() const
    {
        Lock l(get_mutex());

        if (bp::override f = this->get_override("human_name"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataSpecTreeKey", "human_name");
    }

    virtual MetadataKeyType type() const
    {
        Lock l(get_mutex());

        if (bp::override f = this->get_override("type"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataSpecTreeKey", "type");
    }
};

template <>
struct MetadataSpecTreeKeyWrapper<FetchableURISpecTree> :
    MetadataSpecTreeKey<FetchableURISpecTree>,
    bp::wrapper<MetadataSpecTreeKey<FetchableURISpecTree> >
{
    virtual const std::tr1::shared_ptr<const FetchableURISpecTree> value() const
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

    virtual const std::tr1::shared_ptr<const URILabel> initial_label() const
        PALUDIS_ATTRIBUTE((warn_unused_result))
    {
        Lock l(get_mutex());

        if (bp::override f = this->get_override("initial_label"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataSpecTreeKey", "initial_label");
    }

    virtual const std::string raw_name() const
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("raw_name"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataSpecTreeKey", "raw_name");
    }

    virtual const std::string human_name() const
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("human_name"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataSpecTreeKey", "human_name");
    }

    virtual MetadataKeyType type() const
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("type"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataSpecTreeKey", "type");
    }
};

template <>
struct MetadataSpecTreeKeyWrapper<DependencySpecTree> :
    MetadataSpecTreeKey<DependencySpecTree>,
    bp::wrapper<MetadataSpecTreeKey<DependencySpecTree> >
{
    virtual const std::tr1::shared_ptr<const DependencySpecTree> value() const
        PALUDIS_ATTRIBUTE((warn_unused_result))
    {
        Lock l(get_mutex());

        if (bp::override f = this->get_override("value"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataSpecTreeKey", "value");
    }

    virtual std::string pretty_print(const DependencySpecTree::ItemFormatter & formatter) const
        PALUDIS_ATTRIBUTE((warn_unused_result))
    {
        Lock l(get_mutex());

        if (bp::override f = this->get_override("pretty_print"))
            return f(boost::cref(formatter));
        else
            throw PythonMethodNotImplemented("MetadataSpecTreeKey", "pretty_print");
    }

    virtual std::string pretty_print_flat(const DependencySpecTree::ItemFormatter & formatter) const
        PALUDIS_ATTRIBUTE((warn_unused_result))
    {
        Lock l(get_mutex());

        if (bp::override f = this->get_override("pretty_print_flat"))
            return f(boost::cref(formatter));
        else
            throw PythonMethodNotImplemented("MetadataSpecTreeKey", "pretty_print_flat");
    }

    virtual const std::tr1::shared_ptr<const DependenciesLabelSequence> initial_labels() const
        PALUDIS_ATTRIBUTE((warn_unused_result))
    {
        Lock l(get_mutex());

        if (bp::override f = this->get_override("initial_labels"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataSpecTreeKey", "initial_labels");
    }

    virtual const std::string raw_name() const
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("raw_name"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataSpecTreeKey", "raw_name");
    }

    virtual const std::string human_name() const
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("human_name"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataSpecTreeKey", "human_name");
    }

    virtual MetadataKeyType type() const
    {
        Lock l(get_mutex());

        if (bp::override f = get_override("type"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataSpecTreeKey", "type");
    }
};

template <typename C_>
struct class_set_key :
    bp::class_<MetadataCollectionKeyWrapper<C_>, std::tr1::shared_ptr<MetadataCollectionKeyWrapper<C_> >,
        bp::bases<MetadataKey>, boost::noncopyable>
{
    class_set_key(const std::string & set) :
        bp::class_<MetadataCollectionKeyWrapper<C_>, std::tr1::shared_ptr<MetadataCollectionKeyWrapper<C_> >,
            bp::bases<MetadataKey>, boost::noncopyable>(
                    ("Metadata" + set + "Key").c_str(),
                    "A MetadataCollectionKey is a MetadataKey that holds a Set or Sequence of some kind of item\n"
                    "as its value.\n\n"

                    "This class can be subclassed in Python.",
                    bp::init<>(
                        "__init__()"
                        )
                    )
    {
        bp::register_ptr_to_python<std::tr1::shared_ptr<const MetadataCollectionKey<C_> > >();
        bp::implicitly_convertible<std::tr1::shared_ptr<MetadataCollectionKeyWrapper<C_> >,
                std::tr1::shared_ptr<MetadataKey> >();

        def("value", bp::pure_virtual(&MetadataCollectionKey<C_>::value),
                ("value() -> " + set + "\n"
                 "Fetch our value.").c_str()
           );

        def("pretty_print_flat", bp::pure_virtual(&MetadataCollectionKey<C_>::pretty_print_flat),
                ("pretty_print_flat(" + set +"Formatter) -> string\n"
                 "Return a single-line formatted version of our value, using the\n"
                 "supplied Formatter to format individual items.").c_str()
           );
    }
};

template <typename C_>
struct class_spec_tree_key :
    bp::class_<MetadataSpecTreeKeyWrapper<C_>, std::tr1::shared_ptr<MetadataSpecTreeKeyWrapper<C_> >,
        bp::bases<MetadataKey>, boost::noncopyable>
{
    class_spec_tree_key(const std::string & spec_tree) :
        bp::class_<MetadataSpecTreeKeyWrapper<C_>, std::tr1::shared_ptr<MetadataSpecTreeKeyWrapper<C_> >,
            bp::bases<MetadataKey>, boost::noncopyable>(
                    ("Metadata" + spec_tree + "Key").c_str(),
                    "A MetadataSpecTreeKey is a MetadataKey that holds a spec tree of some\n"
                    "kind as its value.\n\n"

                    "This class can be subclassed in Python.",
                    bp::init<>(
                        "__init__()"
                        )
                    )
    {
        bp::register_ptr_to_python<std::tr1::shared_ptr<const MetadataSpecTreeKey<C_> > >();
        bp::implicitly_convertible<std::tr1::shared_ptr<MetadataSpecTreeKeyWrapper<C_> >,
                std::tr1::shared_ptr<MetadataKey> >();

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
        std::tr1::shared_ptr<MetadataSpecTreeKeyWrapper<FetchableURISpecTree> >,
        bp::bases<MetadataKey>, boost::noncopyable>
{
    class_spec_tree_key(const std::string & spec_tree) :
        bp::class_<MetadataSpecTreeKeyWrapper<FetchableURISpecTree>, 
            std::tr1::shared_ptr<MetadataSpecTreeKeyWrapper<FetchableURISpecTree> >,
            bp::bases<MetadataKey>, boost::noncopyable>(
                    ("Metadata" + spec_tree + "Key").c_str(),
                    "A MetadataSpecTreeKey is a MetadataKey that holds a spec tree of some\n"
                    "kind as its value.\n\n"

                    "This class can be subclassed in Python.",
                    bp::init<>(
                        "__init__()"
                        )
                    )
    {
        bp::register_ptr_to_python<std::tr1::shared_ptr<const MetadataSpecTreeKey<FetchableURISpecTree> > >();
        bp::implicitly_convertible<std::tr1::shared_ptr<MetadataSpecTreeKeyWrapper<FetchableURISpecTree> >,
                std::tr1::shared_ptr<MetadataKey> >();

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

template <>
struct class_spec_tree_key<DependencySpecTree> :
    bp::class_<MetadataSpecTreeKeyWrapper<DependencySpecTree>,
        std::tr1::shared_ptr<MetadataSpecTreeKeyWrapper<DependencySpecTree> >,
        bp::bases<MetadataKey>, boost::noncopyable>
{
    class_spec_tree_key(const std::string & spec_tree) :
        bp::class_<MetadataSpecTreeKeyWrapper<DependencySpecTree>, 
            std::tr1::shared_ptr<MetadataSpecTreeKeyWrapper<DependencySpecTree> >,
            bp::bases<MetadataKey>, boost::noncopyable>(
                    ("Metadata" + spec_tree + "Key").c_str(),
                    "A MetadataSpecTreeKey is a MetadataKey that holds a spec tree of some\n"
                    "kind as its value.\n\n"

                    "This class can be subclassed in Python.",
                    bp::init<>(
                        "__init__()"
                        )
                    )
    {
        bp::register_ptr_to_python<std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > >();
        bp::implicitly_convertible<std::tr1::shared_ptr<MetadataSpecTreeKeyWrapper<DependencySpecTree> >,
                std::tr1::shared_ptr<MetadataKey> >();

        def("value", bp::pure_virtual(&MetadataSpecTreeKey<DependencySpecTree>::value),
                ("value() -> " + spec_tree + "\n"
                 "Fetch our value").c_str()
           );

        def("pretty_print", bp::pure_virtual(&MetadataSpecTreeKey<DependencySpecTree>::pretty_print),
                ("pretty_print(" + spec_tree + "Formatter) -> string\n"
                 "Return a multiline-line indented and formatted version of our\n"
                 "value, using the supplied Formatter to format individual items.").c_str()
           );

        def("pretty_print_flat", bp::pure_virtual(&MetadataSpecTreeKey<DependencySpecTree>::pretty_print_flat),
                ("pretty_print_flat(" + spec_tree + "Formatter) -> string\n"
                 "Return a single-line formatted version of our value, using the\n"
                 "supplied Formatter to format individual items.").c_str()
           );

        def("initial_labels", bp::pure_virtual(&MetadataSpecTreeKey<DependencySpecTree>::initial_labels),
                "initial_label() -> DependenciesLabelSequence\n"
                "Return a DependenciesLabelSequence that represents the initial labels to use when\n"
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
    bp::register_ptr_to_python<std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const PackageID> > > >();
    bp::implicitly_convertible<std::tr1::shared_ptr<MetadataPackageIDKeyWrapper>,
            std::tr1::shared_ptr<MetadataKey> >();
    bp::class_<MetadataPackageIDKeyWrapper, std::tr1::shared_ptr<MetadataPackageIDKeyWrapper>,
            bp::bases<MetadataKey>, boost::noncopyable>
        (
         "MetadataPackageIDKey",
         "A MetadataPackageIDKey is a MetadataKey that has a PackageID as its\n"
         "value.\n\n"

         "This class can be subclassed in Python.",
         bp::init<>(
             "__init__()"
             )
        )
        .def("value", bp::pure_virtual(&MetadataValueKey<std::tr1::shared_ptr<const PackageID> > ::value),
                "value() -> PackageID\n"
                "Fetch our value."
                )
        ;

    /**
     * MetadataStringKey
     */
    bp::register_ptr_to_python<std::tr1::shared_ptr<const MetadataValueKey<std::string> > >();
    bp::implicitly_convertible<std::tr1::shared_ptr<MetadataStringKeyWrapper>,
            std::tr1::shared_ptr<MetadataKey> >();
    bp::class_<MetadataStringKeyWrapper, std::tr1::shared_ptr<MetadataStringKeyWrapper>,
            bp::bases<MetadataKey>, boost::noncopyable>
        (
         "MetadataStringKey",
         "A MetadataStringKey is a MetadataKey that has a std::string as its\n"
         "value.\n\n"

         "This class can be subclassed in Python.",
         bp::init<>(
             "__init__()"
             )
        )
        .def("value", bp::pure_virtual(&MetadataValueKey<std::string> ::value),
                "value() -> string\n"
                "Fetch our value."
                )
        ;

    /**
     * MetadataSlotNameKey
     */
    bp::register_ptr_to_python<std::tr1::shared_ptr<const MetadataValueKey<SlotName> > >();
    bp::implicitly_convertible<std::tr1::shared_ptr<MetadataSlotNameKeyWrapper>,
            std::tr1::shared_ptr<MetadataKey> >();
    bp::class_<MetadataSlotNameKeyWrapper, std::tr1::shared_ptr<MetadataSlotNameKeyWrapper>,
            bp::bases<MetadataKey>, boost::noncopyable>
        (
         "MetadataSlotNameKey",
         "A MetadataStringKey is a MetadataKey that has a SlotName as its\n"
         "value.\n\n"

         "This class can be subclassed in Python.",
         bp::init<>(
             "__init__()"
             )
        )
        .def("value", bp::pure_virtual(&MetadataValueKey<SlotName> ::value),
                "value() -> SlotName\n"
                "Fetch our value."
                )
        ;

    /**
     * MetadataSectionKey
     */
    bp::register_ptr_to_python<std::tr1::shared_ptr<const MetadataSectionKey> >();
    bp::implicitly_convertible<std::tr1::shared_ptr<MetadataSectionKeyWrapper>,
            std::tr1::shared_ptr<MetadataKey> >();
    bp::class_<MetadataSectionKeyWrapper, std::tr1::shared_ptr<MetadataSectionKeyWrapper>,
            bp::bases<MetadataKey>, boost::noncopyable>
        (
         "MetadataSectionKey",
         "A MetadataSectionKey holds a number of other MetadataKey instances, and\n"
         "may have a title.\n\n"

         "This class can be subclassed in Python.",
         bp::init<>(
             "__init__()"
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
    bp::register_ptr_to_python<std::tr1::shared_ptr<const MetadataTimeKey> >();
    bp::implicitly_convertible<std::tr1::shared_ptr<MetadataTimeKeyWrapper>,
            std::tr1::shared_ptr<MetadataKey> >();
    bp::class_<MetadataTimeKeyWrapper, std::tr1::shared_ptr<MetadataTimeKeyWrapper>,
            bp::bases<MetadataKey>, boost::noncopyable>
        (
         "MetadataTimeKey",
         "A MetadataTimeKey is a MetadataKey that has a int(time_t) as its value.\n\n"

         "This class can be subclassed in Python.",
         bp::init<>(
             "__init__()"
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
    bp::register_ptr_to_python<std::tr1::shared_ptr<const MetadataValueKey<FSEntry> > >();
    bp::implicitly_convertible<std::tr1::shared_ptr<MetadataFSEntryKeyWrapper>,
            std::tr1::shared_ptr<MetadataKey> >();
    bp::class_<MetadataFSEntryKeyWrapper, std::tr1::shared_ptr<MetadataFSEntryKeyWrapper>,
            bp::bases<MetadataKey>, boost::noncopyable>
        (
         "MetadataFSEntryKey",
         "A MetadataFSEntryKey is a MetadataKey that has an string(FSEntry) as its value.\n\n"

         "This class can be subclassed in Python.",
         bp::init<>(
             "__init__()"
             )
        )
        .def("value", bp::pure_virtual(&MetadataValueKey<FSEntry> ::value),
                "value() -> FSEntry\n"
                "Fetch our value."
                )
        ;

    /**
     * MetadataContentsKey
     */
    bp::register_ptr_to_python<std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const Contents> > > >();
    bp::implicitly_convertible<std::tr1::shared_ptr<MetadataContentsKeyWrapper>,
            std::tr1::shared_ptr<MetadataKey> >();
    bp::class_<MetadataContentsKeyWrapper, std::tr1::shared_ptr<MetadataContentsKeyWrapper>,
            bp::bases<MetadataKey>, boost::noncopyable>
        (
         "MetadataContentsKey",
         "A MetadataContentsKey is a MetadataKey that holds a Contents heirarchy.\n\n"

         "This class can be subclassed in Python.",
         bp::init<>(
             "__init__()"
             )
        )
        .def("value", bp::pure_virtual(&MetadataValueKey<std::tr1::shared_ptr<const Contents> > ::value),
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
     * MetadataChoicesKey
     */
    bp::register_ptr_to_python<std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const Choices> > > >();
    bp::implicitly_convertible<std::tr1::shared_ptr<MetadataChoicesKeyWrapper>,
            std::tr1::shared_ptr<MetadataKey> >();
    bp::class_<MetadataChoicesKeyWrapper, std::tr1::shared_ptr<MetadataChoicesKeyWrapper>,
            bp::bases<MetadataKey>, boost::noncopyable>
        (
         "MetadataChoicesKey",
         "A MetadataChoicesKey is a MetadataKey that holds a Choices heirarchy.\n\n"

         "This class can be subclassed in Python.",
         bp::init<>(
             "__init__()"
             )
        )
        .def("value", bp::pure_virtual(&MetadataValueKey<std::tr1::shared_ptr<const Choices> > ::value),
                "value() -> Choices\n"
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
    bp::register_ptr_to_python<std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const RepositoryMaskInfo> > > >();
    bp::implicitly_convertible<std::tr1::shared_ptr<MetadataRepositoryMaskInfoKeyWrapper>,
            std::tr1::shared_ptr<MetadataKey> >();
    bp::class_<MetadataRepositoryMaskInfoKeyWrapper, std::tr1::shared_ptr<MetadataRepositoryMaskInfoKeyWrapper>,
            bp::bases<MetadataKey>, boost::noncopyable>
        (
         "MetadataRepositoryMaskInfoKey",
         "A MetadataRepositoryMaskInfoKey is a MetadataKey that holds\n"
         "RepositoryMaskInfo as its value.\n\n"

         "This class can be subclassed in Python.",
         bp::init<>(
             "__init__()"
             )
        )
        .def("value", bp::pure_virtual(&MetadataValueKey<std::tr1::shared_ptr<const RepositoryMaskInfo> > ::value),
                "value() -> RepositoryMaskInfo\n"
                "Fetch our value."
                )
        ;

    /**
     * MetadataCollectionKeys
     */
    class_set_key<KeywordNameSet>("KeywordNameIterable");
    class_set_key<Set<std::string> >("StringIterable");
    class_set_key<FSEntrySequence>("FSEntryIterable");
    class_set_key<PackageIDSequence>("PackageIDIterable");

    /**
     * MetadataSpecTreeKeys
     */
    class_spec_tree_key<LicenseSpecTree>("LicenseSpecTree");
    class_spec_tree_key<ProvideSpecTree>("ProvideSpecTree");
    class_spec_tree_key<DependencySpecTree>("DependencySpecTree");
    class_spec_tree_key<PlainTextSpecTree>("PlainTextSpecTree");
    class_spec_tree_key<SimpleURISpecTree>("SimpleURISpecTree");
    class_spec_tree_key<FetchableURISpecTree>("FetchableURISpecTree");
}

