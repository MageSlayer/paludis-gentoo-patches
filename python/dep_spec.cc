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

#include <paludis_python.hh>

#include <dep_spec.hh>
#include <paludis/dep_tag.hh>
#include <paludis/version_requirements.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/collection_concrete.hh>
#include <paludis/util/save.hh>
#include <paludis/util/stringify.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>
#include <libwrapiter/libwrapiter_output_iterator.hh>
#include <list>

using namespace paludis;
using namespace paludis::python;
namespace bp = boost::python;


PythonDepSpec::PythonDepSpec()
{
}

PythonDepSpec::~PythonDepSpec()
{
}

const PythonUseDepSpec *
PythonDepSpec::as_use_dep_spec() const
{
    return 0;
}

const PythonPackageDepSpec *
PythonDepSpec::as_package_dep_spec() const
{
    return 0;
}

namespace paludis
{
    template<>
    struct Implementation<PythonCompositeDepSpec>
    {
        std::list<tr1::shared_ptr<const PythonDepSpec> > children;
    };

    template<>
    struct Implementation<PythonPackageDepSpec>
    {
        tr1::shared_ptr<const QualifiedPackageName> package_ptr;
        tr1::shared_ptr<const CategoryNamePart> category_name_part_ptr;
        tr1::shared_ptr<const PackageNamePart> package_name_part_ptr;
        tr1::shared_ptr<VersionRequirements> version_requirements;
        VersionRequirementsMode version_requirements_mode;
        tr1::shared_ptr<const SlotName> slot;
        tr1::shared_ptr<const RepositoryName> repository;
        tr1::shared_ptr<const UseRequirements> use_requirements;
        tr1::shared_ptr<const DepTag> tag;
        const std::string str;

        Implementation(
                const tr1::shared_ptr<const QualifiedPackageName> & q,
                const tr1::shared_ptr<const CategoryNamePart> & c,
                const tr1::shared_ptr<const PackageNamePart> & p,
                const tr1::shared_ptr<VersionRequirements> & v,
                const VersionRequirementsMode m,
                const tr1::shared_ptr<const SlotName> & s,
                const tr1::shared_ptr<const RepositoryName> & r,
                const tr1::shared_ptr<const UseRequirements> & u,
                const tr1::shared_ptr<const DepTag> & t,
                const std::string & st) :
            package_ptr(q),
            category_name_part_ptr(c),
            package_name_part_ptr(p),
            version_requirements(v),
            version_requirements_mode(m),
            slot(s),
            repository(r),
            use_requirements(u),
            tag(t),
            str(st)
        {
        }
    };
}

PythonCompositeDepSpec::PythonCompositeDepSpec() :
    PrivateImplementationPattern<PythonCompositeDepSpec>(new Implementation<PythonCompositeDepSpec>)
{
}

PythonCompositeDepSpec::~PythonCompositeDepSpec()
{
}

void
PythonCompositeDepSpec::add_child(const tr1::shared_ptr<const PythonDepSpec> c)
{
    _imp->children.push_back(c);
}

PythonCompositeDepSpec::Iterator
PythonCompositeDepSpec::begin() const
{
    return Iterator(_imp->children.begin());
}

PythonCompositeDepSpec::Iterator
PythonCompositeDepSpec::end() const
{
    return Iterator(_imp->children.end());
}

PythonAnyDepSpec::PythonAnyDepSpec()
{
}

PythonAnyDepSpec::PythonAnyDepSpec(const AnyDepSpec &)
{
}

PythonAllDepSpec::PythonAllDepSpec()
{
}

PythonAllDepSpec::PythonAllDepSpec(const AllDepSpec &)
{
}

PythonUseDepSpec::PythonUseDepSpec(const UseFlagName & our_flag, bool is_inverse) :
    _flag(our_flag),
    _inverse(is_inverse)
{
}

PythonUseDepSpec::PythonUseDepSpec(const UseDepSpec & d) :
    _flag(d.flag()),
    _inverse(d.inverse())
{
}

const PythonUseDepSpec *
PythonUseDepSpec::as_use_dep_spec() const
{
    return this;
}

UseFlagName
PythonUseDepSpec::flag() const
{
    return _flag;
}

bool
PythonUseDepSpec::inverse() const
{
    return _inverse;
}

PythonStringDepSpec::PythonStringDepSpec(const std::string & s) :
    _str(s)
{
}

PythonStringDepSpec::PythonStringDepSpec(const StringDepSpec & d) :
    _str(d.text())
{
}

PythonStringDepSpec::~PythonStringDepSpec()
{
}

void
PythonStringDepSpec::set_text(const std::string & t)
{
    _str = t;
}

std::string
PythonStringDepSpec::text() const
{
    return _str;
}

template <typename T_>
tr1::shared_ptr<T_>
deep_copy(const tr1::shared_ptr<const T_> & x)
{
    if (x)
        return tr1::shared_ptr<T_>(new T_(*x));
    else
        return tr1::shared_ptr<T_>();
}

PythonPackageDepSpec::PythonPackageDepSpec(const PackageDepSpec & p) :
    PythonStringDepSpec(p.text()),
    PrivateImplementationPattern<PythonPackageDepSpec>(new Implementation<PythonPackageDepSpec>(
                deep_copy(p.package_ptr()),
                deep_copy(p.category_name_part_ptr()),
                deep_copy(p.package_name_part_ptr()),
                tr1::shared_ptr<VersionRequirements>(new VersionRequirements::Concrete),
                p.version_requirements_mode(),
                deep_copy(p.slot_ptr()),
                deep_copy(p.repository_ptr()),
                deep_copy(p.use_requirements_ptr()),
                tr1::shared_ptr<const DepTag>(p.tag()),
                stringify(p)))
{
    if (p.version_requirements_ptr())
    {
        std::copy(p.version_requirements_ptr()->begin(), p.version_requirements_ptr()->end(),
            _imp->version_requirements->inserter());
    }
}

PythonPackageDepSpec::PythonPackageDepSpec(const PythonPackageDepSpec & p) :
    PythonStringDepSpec(p.text()),
    PrivateImplementationPattern<PythonPackageDepSpec>(new Implementation<PythonPackageDepSpec>(
                deep_copy(p.package_ptr()),
                deep_copy(p.category_name_part_ptr()),
                deep_copy(p.package_name_part_ptr()),
                tr1::shared_ptr<VersionRequirements>(new VersionRequirements::Concrete),
                p.version_requirements_mode(),
                deep_copy(p.slot_ptr()),
                deep_copy(p.repository_ptr()),
                deep_copy(p.use_requirements_ptr()),
                tr1::shared_ptr<const DepTag>(p.tag()),
                p.py_str()))
{
    std::copy(p.version_requirements_ptr()->begin(), p.version_requirements_ptr()->end(),
            _imp->version_requirements->inserter());
}

PythonPackageDepSpec::~PythonPackageDepSpec()
{
}

tr1::shared_ptr<const PythonPackageDepSpec>
PythonPackageDepSpec::make_from_string(const std::string & ss, const PackageDepSpecParseMode p)
{
    return tr1::shared_ptr<PythonPackageDepSpec>(new PythonPackageDepSpec(PackageDepSpec(ss, p)));
}

const PythonPackageDepSpec *
PythonPackageDepSpec::as_package_dep_spec() const
{
    return this;
}

const tr1::shared_ptr<const PythonPackageDepSpec>
PythonPackageDepSpec::without_use_requirements() const
{
    PackageDepSpec p(
            deep_copy(package_ptr()),
            deep_copy(category_name_part_ptr()),
            deep_copy(package_name_part_ptr()),
            tr1::shared_ptr<VersionRequirements>(new VersionRequirements::Concrete),
            version_requirements_mode(),
            deep_copy(slot_ptr()),
            deep_copy(repository_ptr()),
            deep_copy(use_requirements_ptr()),
            tr1::shared_ptr<const DepTag>(tag())
            );

    if (version_requirements_ptr())
    {
        std::copy(version_requirements_ptr()->begin(), version_requirements_ptr()->end(),
                p.version_requirements_ptr()->inserter());
    }

    return tr1::shared_ptr<PythonPackageDepSpec>(new PythonPackageDepSpec(*p.without_use_requirements()));
}

tr1::shared_ptr<const QualifiedPackageName>
PythonPackageDepSpec::package_ptr() const
{
    return _imp->package_ptr;
}

tr1::shared_ptr<const PackageNamePart>
PythonPackageDepSpec::package_name_part_ptr() const
{
    return _imp->package_name_part_ptr;
}

tr1::shared_ptr<const CategoryNamePart>
PythonPackageDepSpec::category_name_part_ptr() const
{
    return _imp->category_name_part_ptr;
}

tr1::shared_ptr<const VersionRequirements>
PythonPackageDepSpec::version_requirements_ptr() const
{
    return _imp->version_requirements;
}

VersionRequirementsMode
PythonPackageDepSpec::version_requirements_mode() const
{
    return _imp->version_requirements_mode;
}

void
PythonPackageDepSpec::set_version_requirements_mode(const VersionRequirementsMode m)
{
    _imp->version_requirements_mode = m;
}

tr1::shared_ptr<const SlotName>
PythonPackageDepSpec::slot_ptr() const
{
    return _imp->slot;
}

tr1::shared_ptr<const RepositoryName>
PythonPackageDepSpec::repository_ptr() const
{
    return _imp->repository;
}

tr1::shared_ptr<const UseRequirements>
PythonPackageDepSpec::use_requirements_ptr() const
{
    return _imp->use_requirements;
}

tr1::shared_ptr<const DepTag>
PythonPackageDepSpec::tag() const
{
    return _imp->tag;
}

std::string
PythonPackageDepSpec::py_str() const
{
    return _imp->str;
}

void
PythonPackageDepSpec::set_tag(const tr1::shared_ptr<const DepTag> & s)
{
    _imp->tag = s;
}

PythonPlainTextDepSpec::PythonPlainTextDepSpec(const std::string & s) :
    PythonStringDepSpec(s)
{
}

PythonPlainTextDepSpec::PythonPlainTextDepSpec(const PlainTextDepSpec & d) :
    PythonStringDepSpec(d.text())
{
}

PythonBlockDepSpec::PythonBlockDepSpec(tr1::shared_ptr<const PythonPackageDepSpec> & a) :
    PythonStringDepSpec("!" + a->text()),
    _spec(a)
{
}

PythonBlockDepSpec::PythonBlockDepSpec(const BlockDepSpec & d) :
    PythonStringDepSpec(d.text()),
    _spec(tr1::shared_ptr<const PythonPackageDepSpec>(new PythonPackageDepSpec(*d.blocked_spec())))
{
}

tr1::shared_ptr<const PythonPackageDepSpec>
PythonBlockDepSpec::blocked_spec() const
{
    return _spec;
}

PythonURIDepSpec::PythonURIDepSpec(const std::string & s) :
    PythonStringDepSpec(s)
{
}

PythonURIDepSpec::PythonURIDepSpec(const URIDepSpec & d) :
    PythonStringDepSpec(d.text())
{
}

std::string
PythonURIDepSpec::original_url() const
{
    std::string::size_type p(text().find(" -> "));
    if (std::string::npos == p)
        return text();
    else
        return text().substr(0, p);
}

std::string
PythonURIDepSpec::renamed_url_suffix() const
{
    std::string::size_type p(text().find(" -> "));
    if (std::string::npos == p)
        return "";
    else
        return text().substr(p + 4);
}

DepSpecVisitor::DepSpecVisitor() :
    _current_parent(new PythonAllDepSpec())
{
}

DepSpecVisitor::~DepSpecVisitor()
{
}

void
DepSpecVisitor::visit_sequence(const AllDepSpec & d,
        GenericSpecTree::ConstSequenceIterator cur,
        GenericSpecTree::ConstSequenceIterator end)
{
    tr1::shared_ptr<PythonAllDepSpec> py_cds(new PythonAllDepSpec(d));
    _current_parent->add_child(py_cds);
    Save<tr1::shared_ptr<PythonCompositeDepSpec> > old_parent(&_current_parent, py_cds);
    std::for_each(cur, end, accept_visitor(*this));
}

void
DepSpecVisitor::visit_sequence(const AnyDepSpec & d,
        GenericSpecTree::ConstSequenceIterator cur,
        GenericSpecTree::ConstSequenceIterator end)
{
    tr1::shared_ptr<PythonAnyDepSpec> py_cds(new PythonAnyDepSpec(d));
    _current_parent->add_child(py_cds);
    Save<tr1::shared_ptr<PythonCompositeDepSpec> > old_parent(&_current_parent, py_cds);
    std::for_each(cur, end, accept_visitor(*this));
}

void
DepSpecVisitor::visit_sequence(const UseDepSpec & d,
        GenericSpecTree::ConstSequenceIterator cur,
        GenericSpecTree::ConstSequenceIterator end)
{
    tr1::shared_ptr<PythonUseDepSpec> py_cds(new PythonUseDepSpec(d));
    _current_parent->add_child(py_cds);
    Save<tr1::shared_ptr<PythonCompositeDepSpec> > old_parent(&_current_parent, py_cds);
    std::for_each(cur, end, accept_visitor(*this));
}

void
DepSpecVisitor::visit_leaf(const PackageDepSpec & d)
{
    _current_parent->add_child(tr1::shared_ptr<PythonPackageDepSpec>(new PythonPackageDepSpec(d)));
}

void
DepSpecVisitor::visit_leaf(const PlainTextDepSpec & d)
{
    _current_parent->add_child(tr1::shared_ptr<PythonPlainTextDepSpec>(new PythonPlainTextDepSpec(d)));
}

void
DepSpecVisitor::visit_leaf(const URIDepSpec & d)
{
    _current_parent->add_child(tr1::shared_ptr<PythonURIDepSpec>(new PythonURIDepSpec(d)));
}

void
DepSpecVisitor::visit_leaf(const BlockDepSpec & d)
{
    _current_parent->add_child(tr1::shared_ptr<PythonBlockDepSpec>(new PythonBlockDepSpec(d)));
}

const tr1::shared_ptr<const PythonDepSpec>
DepSpecVisitor::result() const
{
    return *_current_parent->begin();
}

template <typename N_>
struct tree_to_python
{
    static PyObject *
    convert(const N_ & n)
    {
        DepSpecVisitor v;
        n->accept(v);
        return bp::incref(bp::object(v.result()).ptr());
    }
};

template <typename T_>
void register_tree_to_python()
{
    bp::to_python_converter<tr1::shared_ptr<typename T_::ConstItem>,
            tree_to_python<tr1::shared_ptr<typename T_::ConstItem> > >();
}


struct PackageDepSpecFromPython
{
    PackageDepSpecFromPython()
    {
        bp::converter::registry::push_back(&convertible, &construct,
                boost::python::type_id<PackageDepSpec>());
    }

    static void *
    convertible(PyObject * obj_ptr)
    {
        return obj_ptr;
    }

    static void
    construct(PyObject * obj_ptr, bp::converter::rvalue_from_python_stage1_data * data)
    {
        typedef bp::converter::rvalue_from_python_storage<PythonPackageDepSpec> Storage;
        void * storage = reinterpret_cast<Storage *>(data)->storage.bytes;
        PythonPackageDepSpec p = bp::extract<PythonPackageDepSpec>(obj_ptr);
        new (storage) PackageDepSpec(
                    deep_copy(p.package_ptr()),
                    deep_copy(p.category_name_part_ptr()),
                    deep_copy(p.package_name_part_ptr()),
                    tr1::shared_ptr<VersionRequirements>(new VersionRequirements::Concrete),
                    p.version_requirements_mode(),
                    deep_copy(p.slot_ptr()),
                    deep_copy(p.repository_ptr()),
                    deep_copy(p.use_requirements_ptr()),
                    tr1::shared_ptr<const DepTag>(p.tag())
                );

        if (p.version_requirements_ptr())
        {
            std::copy(p.version_requirements_ptr()->begin(), p.version_requirements_ptr()->end(),
                    reinterpret_cast<PackageDepSpec *>(storage)->version_requirements_ptr()->inserter());
        }
        data->convertible = storage;
    }
};

struct PackageDepSpecSPFromPython
{
    PackageDepSpecSPFromPython()
    {
        bp::converter::registry::push_back(&convertible, &construct,
                boost::python::type_id<tr1::shared_ptr<const PackageDepSpec> >());
    }

    static void *
    convertible(PyObject * obj_ptr)
    {
        return obj_ptr;
    }

    static void
    construct(PyObject * obj_ptr, bp::converter::rvalue_from_python_stage1_data * data)
    {
        typedef bp::converter::rvalue_from_python_storage<PythonPackageDepSpec> Storage;
        void * storage = reinterpret_cast<Storage *>(data)->storage.bytes;

        PythonPackageDepSpec p = bp::extract<PythonPackageDepSpec>(obj_ptr);

        new (storage) tr1::shared_ptr<PackageDepSpec>(new PackageDepSpec(
                    deep_copy(p.package_ptr()),
                    deep_copy(p.category_name_part_ptr()),
                    deep_copy(p.package_name_part_ptr()),
                    tr1::shared_ptr<VersionRequirements>(new VersionRequirements::Concrete),
                    p.version_requirements_mode(),
                    deep_copy(p.slot_ptr()),
                    deep_copy(p.repository_ptr()),
                    deep_copy(p.use_requirements_ptr()),
                    tr1::shared_ptr<const DepTag>(p.tag())
                    ));
        if (p.version_requirements_ptr())
        {
            std::copy(p.version_requirements_ptr()->begin(), p.version_requirements_ptr()->end(),
                    (*reinterpret_cast<tr1::shared_ptr<PackageDepSpec> *>(storage))->version_requirements_ptr()->inserter());
        }
        data->convertible = storage;
    }
};

void PALUDIS_VISIBLE expose_dep_spec()
{
    ExceptionRegister::get_instance()->add_exception<PackageDepSpecError>
        ("PackageDepSpecError", "BaseException",
         "Thrown if an invalid package dep spec specification is encountered.");

    enum_auto("PackageDepSpecParseMode", last_pds_pm);

    register_tree_to_python<DependencySpecTree>();
    register_tree_to_python<ProvideSpecTree>();
    register_tree_to_python<RestrictSpecTree>();
    register_tree_to_python<URISpecTree>();
    register_tree_to_python<LicenseSpecTree>();

    register_shared_ptrs_to_python<PythonDepSpec>();
    bp::class_<PythonDepSpec, boost::noncopyable>
        ds("DepSpec",
                "Base class for a dependency spec.",
                bp::no_init
          );
    ds.def("as_use_dep_spec", &PythonDepSpec::as_use_dep_spec,
            bp::return_value_policy<bp::reference_existing_object>(),
            "as_use_dep_spec() -> UseDepSpec\n"
            "Return us as a UseDepSpec, or None if we are not a UseDepSpec."
          );
    ds.def("as_package_dep_spec", &PythonDepSpec::as_package_dep_spec,
            bp::return_value_policy<bp::reference_existing_object>(),
            "as_package_dep_spec() -> PackageDepSpec\n"
            "Return us as a PackageDepSpec, or None if we are not a PackageDepSpec."
          );

    register_shared_ptrs_to_python<PythonCompositeDepSpec>();
    bp::class_<PythonCompositeDepSpec, bp::bases<PythonDepSpec>, boost::noncopyable>
        cds("CompositeDepSpec",
                "Iterable class for dependency specs that have a number of child dependency specs.",
                bp::no_init
           );
    cds.def("__iter__", bp::range(&PythonCompositeDepSpec::begin, &PythonCompositeDepSpec::end));

    bp::class_<PythonAnyDepSpec, bp::bases<PythonCompositeDepSpec>, boost::noncopyable>
        anyds("AnyDepSpec",
                "Represents a \"|| ( )\" dependency block.",
                bp::no_init
          );

    bp::class_<PythonAllDepSpec, bp::bases<PythonCompositeDepSpec>, boost::noncopyable>
        allds("AllDepSpec",
                "Represents a ( first second third ) or top level group of dependency specs.",
                bp::no_init
          );

    bp::class_<PythonUseDepSpec, bp::bases<PythonCompositeDepSpec>, boost::noncopyable>
        useds("UseDepSpec",
                "Represents a use? ( ) dependency spec.",
                bp::no_init
          );
    useds.add_property("flag", &UseDepSpec::flag,
            "[ro] UseFlagName\n"
            "Our use flag name."
            );
    useds.add_property("inverse", &UseDepSpec::inverse,
            "[ro] bool\n"
            "Are we a ! flag?"
            );

    bp::class_<PythonStringDepSpec, bp::bases<PythonDepSpec>, boost::noncopyable>
        strds("StringDepSpec",
                "A StringDepSpec represents a non-composite dep spec with an associated piece of text.",
                bp::no_init
             );
    strds.add_property("text", &PythonStringDepSpec::text,
            "[ro] string\n"
            "Our text."
            );

    bp::to_python_converter<std::pair<const UseFlagName, UseFlagState>,
            pair_to_tuple<const UseFlagName, UseFlagState> >();

    register_shared_ptrs_to_python<UseRequirements>();
    bp::class_<UseRequirements>
        ur("UseRequirements",
                "A selection of USE flag requirements.",
                bp::no_init
          );
    ur.def("state", &UseRequirements::state,
            "state(UseFlagName) -> UseFlagState\n"
            "What state is desired for a particular use flag?"
          );
    ur.def("__iter__", bp::range(&UseRequirements::begin, &UseRequirements::end));


    PackageDepSpecFromPython();
    PackageDepSpecSPFromPython();
    bp::implicitly_convertible<PackageDepSpec, PythonPackageDepSpec>();

    bp::class_<PythonPackageDepSpec, tr1::shared_ptr<const PythonPackageDepSpec>, bp::bases<PythonStringDepSpec> >
        pkgds("PackageDepSpec",
                "A PackageDepSpec represents a package name (for example, 'app-editors/vim'),"
                " possibly with associated version and SLOT restrictions.",
                bp::no_init
           );
    pkgds.def("__init__", bp::make_constructor(&PythonPackageDepSpec::make_from_string),
            "__init__(string, PackageDepSpecParseMode)"
            );
    pkgds.add_property("package", &PythonPackageDepSpec::package_ptr,
            "[ro] QualifiedPackageName\n"
            "Qualified package name."
           );
    pkgds.add_property("package_name_part", &PythonPackageDepSpec::package_name_part_ptr,
            "[ro] PackageNamePart\n"
            "Package name part (may be None)"
           );
    pkgds.add_property("category_name_part", &PythonPackageDepSpec::category_name_part_ptr,
            "[ro] CategoryNamePart\n"
            "Category name part (may be None)."
           );
    pkgds.add_property("version_requirements", &PythonPackageDepSpec::version_requirements_ptr,
            "[ro] VersionRequirements\n"
            "Version requirements (may be None)."
           );
    pkgds.add_property("version_requirements_mode", &PythonPackageDepSpec::version_requirements_mode,
            "[ro] VersionRequirementsMode\n"
            "Version requirements mode."
           );
    pkgds.add_property("slot", &PythonPackageDepSpec::slot_ptr,
            "[ro] SlotName\n"
            "Slot name (may be None)."
           );
    pkgds.add_property("repository", &PythonPackageDepSpec::repository_ptr,
            "[ro] RepositoryName\n"
            "Repository name (may be None)."
           );
    pkgds.add_property("use_requirements", &PythonPackageDepSpec::use_requirements_ptr,
            "[ro] UseRequirements\n"
            "Use requirements (may be None)."
            );
    pkgds.def("without_use_requirements", &PythonPackageDepSpec::without_use_requirements,
            "without_use_requirements() -> PackageDepSpec\n"
            "Fetch a copy of ourself without the USE requirements."
            );
    pkgds.def("__str__", &PythonPackageDepSpec::py_str);

    bp::class_<PythonPlainTextDepSpec, bp::bases<PythonStringDepSpec>, boost::noncopyable>
        ptds("PlainTextDepSpec",
                "A PlainTextDepSpec represents a plain text entry (for example, a URI in SRC_URI).",
                bp::init<const std::string &>("__init__(string)")
           );
    ptds.def("__str__", &PythonPlainTextDepSpec::text);

    bp::class_<PythonURIDepSpec, bp::bases<PythonStringDepSpec>, boost::noncopyable>
        uds("URIDepSpec",
                "A URIDepSpec represents a URI part.",
                bp::init<const std::string &>("__init__(str)")
           );
    uds.add_property("original_url", &PythonURIDepSpec::original_url,
            "[ro] str"
           );
    uds.add_property("renamed_url_suffix", &PythonURIDepSpec::renamed_url_suffix,
            "[ro] str"
           );

    bp::class_<PythonBlockDepSpec, bp::bases<PythonStringDepSpec>, boost::noncopyable >
        bds("BlockDepSpec",
                "A BlockDepSpec represents a block on a package name (for example, 'app-editors/vim'), \n"
                "possibly with associated version and SLOT restrictions.",
                bp::init<tr1::shared_ptr<const PackageDepSpec> >("__init__(PackageDepSpec)")
           );
    bds.add_property("blocked_spec", &PythonBlockDepSpec::blocked_spec,
            "[ro] PackageDepSpec\n"
            "The spec we're blocking."
           );
    //Work around epydoc bug - http://sf.net/tracker/index.php?func=detail&aid=1738417&group_id=32455&atid=405618
    bds.add_property("text", &PythonBlockDepSpec::text,
            "[ro] string\n"
            "Our text."
            );
}
