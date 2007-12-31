/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh
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

#include <paludis/dep_spec.hh>
#include <paludis/version_operator.hh>
#include <paludis/version_spec.hh>
#include <paludis/version_requirements.hh>
#include <paludis/use_requirements.hh>
#include <paludis/util/clone-impl.hh>
#include <paludis/util/log.hh>
#include <paludis/util/join.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/iterator_funcs.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/options.hh>
#include <paludis/util/tr1_functional.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/visitor-impl.hh>
#include <list>
#include <map>

using namespace paludis;

#include <paludis/dep_spec-se.cc>

DepSpec::DepSpec()
{
}

DepSpec::~DepSpec()
{
}

const UseDepSpec *
DepSpec::as_use_dep_spec() const
{
    return 0;
}

const PackageDepSpec *
DepSpec::as_package_dep_spec() const
{
    return 0;
}

AnyDepSpec::AnyDepSpec()
{
}

tr1::shared_ptr<DepSpec>
AnyDepSpec::clone() const
{
    return tr1::shared_ptr<AnyDepSpec>(new AnyDepSpec());
}

AllDepSpec::AllDepSpec()
{
}

tr1::shared_ptr<DepSpec>
AllDepSpec::clone() const
{
    return tr1::shared_ptr<AllDepSpec>(new AllDepSpec());
}

UseDepSpec::UseDepSpec(const UseFlagName & our_flag, bool is_inverse) :
    _flag(our_flag),
    _inverse(is_inverse)
{
}

const UseDepSpec *
UseDepSpec::as_use_dep_spec() const
{
    return this;
}

UseFlagName
UseDepSpec::flag() const
{
    return _flag;
}

bool
UseDepSpec::inverse() const
{
    return _inverse;
}

tr1::shared_ptr<DepSpec>
UseDepSpec::clone() const
{
    return tr1::shared_ptr<UseDepSpec>(new UseDepSpec(_flag, _inverse));
}

std::string
StringDepSpec::text() const
{
    return _str;
}

NamedSetDepSpec::NamedSetDepSpec(const SetName & n) :
    StringDepSpec(stringify(n)),
    _name(n)
{
}

const SetName
NamedSetDepSpec::name() const
{
    return _name;
}

tr1::shared_ptr<DepSpec>
NamedSetDepSpec::clone() const
{
    return tr1::shared_ptr<NamedSetDepSpec>(new NamedSetDepSpec(_name));
}

const PackageDepSpec *
PackageDepSpec::as_package_dep_spec() const
{
    return this;
}

BlockDepSpec::BlockDepSpec(tr1::shared_ptr<const PackageDepSpec> a) :
    StringDepSpec("!" + a->text()),
    _spec(a)
{
}

std::ostream &
paludis::operator<< (std::ostream & s, const PlainTextDepSpec & a)
{
    s << a.text();
    return s;
}

std::ostream &
paludis::operator<< (std::ostream & s, const LicenseDepSpec & a)
{
    s << a.text();
    return s;
}

std::ostream &
paludis::operator<< (std::ostream & s, const NamedSetDepSpec & a)
{
    s << a.name();
    return s;
}

std::ostream &
paludis::operator<< (std::ostream & s, const BlockDepSpec & a)
{
    s << "!" << *a.blocked_spec();
    return s;
}

std::ostream &
paludis::operator<< (std::ostream & s, const UseDepSpec & a)
{
    if (a.inverse())
        s << "!";
    s << a.flag() << "?";
    return s;
}

std::ostream &
paludis::operator<< (std::ostream & s, const FetchableURIDepSpec & p)
{
    if (! p.renamed_url_suffix().empty())
        s << p.original_url() << " -> " << p.renamed_url_suffix();
    else
        s << p.original_url();

    return s;
}

std::ostream &
paludis::operator<< (std::ostream & s, const SimpleURIDepSpec & p)
{
    s << p.text();
    return s;
}

std::ostream &
paludis::operator<< (std::ostream & s, const PackageDepSpec & a)
{
    s << a._as_string();
    return s;
}

std::ostream &
paludis::operator<< (std::ostream & s, const URILabelsDepSpec & l)
{
    s << join(indirect_iterator(l.begin()), indirect_iterator(l.end()), "+") << ":";
    return s;
}

std::ostream &
paludis::operator<< (std::ostream & s, const DependencyLabelsDepSpec & l)
{
    s << join(indirect_iterator(l.begin()), indirect_iterator(l.end()), ",") << ":";
    return s;
}

PackageDepSpecError::PackageDepSpecError(const std::string & msg) throw () :
    Exception(msg)
{
}

StringDepSpec::StringDepSpec(const std::string & s) :
    _str(s)
{
}

StringDepSpec::~StringDepSpec()
{
}


PlainTextDepSpec::PlainTextDepSpec(const std::string & s) :
    StringDepSpec(s)
{
}

tr1::shared_ptr<DepSpec>
PlainTextDepSpec::clone() const
{
    return tr1::shared_ptr<DepSpec>(new PlainTextDepSpec(text()));
}

LicenseDepSpec::LicenseDepSpec(const std::string & s) :
    StringDepSpec(s)
{
}

tr1::shared_ptr<DepSpec>
LicenseDepSpec::clone() const
{
    return tr1::shared_ptr<DepSpec>(new LicenseDepSpec(text()));
}

SimpleURIDepSpec::SimpleURIDepSpec(const std::string & s) :
    StringDepSpec(s)
{
}

tr1::shared_ptr<DepSpec>
SimpleURIDepSpec::clone() const
{
    return tr1::shared_ptr<DepSpec>(new SimpleURIDepSpec(text()));
}

tr1::shared_ptr<const PackageDepSpec>
BlockDepSpec::blocked_spec() const
{
    return _spec;
}

tr1::shared_ptr<DepSpec>
BlockDepSpec::clone() const
{
    return tr1::shared_ptr<DepSpec>(new BlockDepSpec(tr1::static_pointer_cast<PackageDepSpec>(_spec->clone())));
}

FetchableURIDepSpec::FetchableURIDepSpec(const std::string & s) :
    StringDepSpec(s)
{
}

std::string
FetchableURIDepSpec::original_url() const
{
    std::string::size_type p(text().find(" -> "));
    if (std::string::npos == p)
        return text();
    else
        return text().substr(0, p);
}

std::string
FetchableURIDepSpec::renamed_url_suffix() const
{
    std::string::size_type p(text().find(" -> "));
    if (std::string::npos == p)
        return "";
    else
        return text().substr(p + 4);
}

std::string
FetchableURIDepSpec::filename() const
{
    std::string rus = renamed_url_suffix();
    if (! rus.empty())
        return rus;

    std::string orig = original_url();
    std::string::size_type p(orig.rfind('/'));

    if (std::string::npos == p)
        return orig;
    return orig.substr(p+1);
}

tr1::shared_ptr<DepSpec>
FetchableURIDepSpec::clone() const
{
    return tr1::shared_ptr<FetchableURIDepSpec>(new FetchableURIDepSpec(text()));
}

namespace paludis
{
#ifndef PALUDIS_NO_DOUBLE_TEMPLATE
    template <>
#endif
    template <typename T_>
    struct Implementation<LabelsDepSpec<T_ > >
    {
        std::list<tr1::shared_ptr<const typename T_::BasicNode> > items;
    };
}

template <typename T_>
LabelsDepSpec<T_>::LabelsDepSpec() :
    PrivateImplementationPattern<LabelsDepSpec<T_> >(new Implementation<LabelsDepSpec<T_> >)
{
}

template <typename T_>
LabelsDepSpec<T_>::~LabelsDepSpec()
{
}

template <typename T_>
tr1::shared_ptr<DepSpec>
LabelsDepSpec<T_>::clone() const
{
    return tr1::shared_ptr<LabelsDepSpec<T_> >(new LabelsDepSpec<T_>);
}

template <typename T_>
typename LabelsDepSpec<T_>::ConstIterator
LabelsDepSpec<T_>::begin() const
{
    return ConstIterator(_imp->items.begin());
}

template <typename T_>
typename LabelsDepSpec<T_>::ConstIterator
LabelsDepSpec<T_>::end() const
{
    return ConstIterator(_imp->items.end());
}

template <typename T_>
void
LabelsDepSpec<T_>::add_label(const tr1::shared_ptr<const typename T_::BasicNode> & item)
{
    _imp->items.push_back(item);
}

PackageDepSpecData::~PackageDepSpecData()
{
}

namespace paludis
{
    template <>
    struct Implementation<PackageDepSpec>
    {
        const tr1::shared_ptr<const PackageDepSpecData> data;
        tr1::shared_ptr<const DepTag> tag;

        Implementation(const tr1::shared_ptr<const PackageDepSpecData> & d, const tr1::shared_ptr<const DepTag> & t) :
            data(d),
            tag(t)
        {
        }
    };
}

PackageDepSpec::PackageDepSpec(const tr1::shared_ptr<const PackageDepSpecData> & d) :
    Cloneable<DepSpec>(),
    StringDepSpec(d->as_string()),
    PrivateImplementationPattern<PackageDepSpec>(new Implementation<PackageDepSpec>(d, tr1::shared_ptr<const DepTag>()))
{
}

PackageDepSpec::~PackageDepSpec()
{
}

PackageDepSpec::PackageDepSpec(const PackageDepSpec & d) :
    Cloneable<DepSpec>(d),
    StringDepSpec(d._imp->data->as_string()),
    PrivateImplementationPattern<PackageDepSpec>(new Implementation<PackageDepSpec>(d._imp->data, d._imp->tag)),
    CloneUsingThis<DepSpec, PackageDepSpec>(d)
{
}

tr1::shared_ptr<const QualifiedPackageName>
PackageDepSpec::package_ptr() const
{
    return _imp->data->package_ptr();
}

tr1::shared_ptr<const PackageNamePart>
PackageDepSpec::package_name_part_ptr() const
{
    return _imp->data->package_name_part_ptr();
}

tr1::shared_ptr<const CategoryNamePart>
PackageDepSpec::category_name_part_ptr() const
{
    return _imp->data->category_name_part_ptr();
}

tr1::shared_ptr<const VersionRequirements>
PackageDepSpec::version_requirements_ptr() const
{
    return _imp->data->version_requirements_ptr();
}

const VersionRequirementsMode
PackageDepSpec::version_requirements_mode() const
{
    return _imp->data->version_requirements_mode();
}

tr1::shared_ptr<const SlotName>
PackageDepSpec::slot_ptr() const
{
    return _imp->data->slot_ptr();
}

tr1::shared_ptr<const RepositoryName>
PackageDepSpec::repository_ptr() const
{
    return _imp->data->repository_ptr();
}

tr1::shared_ptr<const UseRequirements>
PackageDepSpec::use_requirements_ptr() const
{
    return _imp->data->use_requirements_ptr();
}

tr1::shared_ptr<PackageDepSpec>
PackageDepSpec::without_use_requirements() const
{
    using namespace tr1::placeholders;

    PartiallyMadePackageDepSpec result;

    if (package_ptr())
        result.package(*package_ptr());

    if (package_name_part_ptr())
        result.package_name_part(*package_name_part_ptr());

    if (category_name_part_ptr())
        result.category_name_part(*category_name_part_ptr());

    if (version_requirements_ptr())
        std::for_each(version_requirements_ptr()->begin(), version_requirements_ptr()->end(),
                tr1::bind(&PartiallyMadePackageDepSpec::version_requirement, &result, _1));

    result.version_requirements_mode(version_requirements_mode());

    if (slot_ptr())
        result.slot(*slot_ptr());

    if (repository_ptr())
        result.repository(*repository_ptr());

    return make_shared_ptr(new PackageDepSpec(result));
}

tr1::shared_ptr<const DepTag>
PackageDepSpec::tag() const
{
    return _imp->tag;
}

void
PackageDepSpec::set_tag(const tr1::shared_ptr<const DepTag> & s)
{
    _imp->tag = s;
}

std::string
PackageDepSpec::_as_string() const
{
    return _imp->data->as_string();
}

PackageDepSpec
paludis::parse_user_package_dep_spec(const std::string & ss, const UserPackageDepSpecOptions & options)
{
    Context context("When parsing package dep spec '" + ss + "':");

    if (ss.empty())
        throw PackageDepSpecError("Got empty dep spec");

    std::string s(ss);
    PartiallyMadePackageDepSpec result;
    bool had_bracket_version_requirements(false);

    std::string::size_type use_group_p;
    while (std::string::npos != ((use_group_p = s.rfind('['))))
    {
        if (s.at(s.length() - 1) != ']')
            throw PackageDepSpecError("Mismatched []");

        std::string flag(s.substr(use_group_p + 1));
        if (flag.length() < 2)
            throw PackageDepSpecError("Invalid [] contents");

        flag.erase(flag.length() - 1);

        switch (flag.at(0))
        {
            case '<':
            case '>':
            case '=':
            case '~':
                {
                    char needed_mode(0);

                    while (! flag.empty())
                    {
                        Context cc("When parsing [] segment '" + flag + "':");

                        std::string op;
                        std::string::size_type opos(0);
                        while (opos < flag.length())
                            if (std::string::npos == std::string("><=~").find(flag.at(opos)))
                                break;
                            else
                                ++opos;

                        op = flag.substr(0, opos);
                        flag.erase(0, opos);

                        if (op.empty())
                            throw PackageDepSpecError("Missing operator inside []");

                        VersionOperator vop(op);

                        std::string ver;
                        opos = flag.find_first_of("|&");
                        if (std::string::npos == opos)
                        {
                            ver = flag;
                            flag.clear();
                        }
                        else
                        {
                            if (0 == needed_mode)
                                needed_mode = flag.at(opos);
                            else if (needed_mode != flag.at(opos))
                                throw PackageDepSpecError("Mixed & and | inside []");

                            result.version_requirements_mode(flag.at(opos) == '|' ? vr_or : vr_and);
                            ver = flag.substr(0, opos++);
                            flag.erase(0, opos);
                        }

                        if (ver.empty())
                            throw PackageDepSpecError("Missing version after operator '" + stringify(vop) + " inside []");

                        if ('*' == ver.at(ver.length() - 1))
                        {
                            ver.erase(ver.length() - 1);
                            if (vop == vo_equal)
                                vop = vo_equal_star;
                            else
                                throw PackageDepSpecError("Invalid use of * with operator '" + stringify(vop) + " inside []");
                        }

                        VersionSpec vs(ver);
                        result.version_requirement(VersionRequirement(vop, vs));
                        had_bracket_version_requirements = true;
                    }
                }
                break;

            default:
                {
                    tr1::shared_ptr<UseRequirement> req;
                    if ('-' == flag.at(0))
                    {
                        flag.erase(0, 1);
                        if (flag.empty())
                            throw PackageDepSpecError("Invalid [] contents");
                        req.reset(new DisabledUseRequirement(UseFlagName(flag)));
                    }
                    else
                        req.reset(new EnabledUseRequirement(UseFlagName(flag)));

                    result.use_requirement(req);
                }
                break;
        };

        s.erase(use_group_p);
    }

    std::string::size_type repo_p;
    if (std::string::npos != ((repo_p = s.rfind("::"))))
    {
        result.repository(RepositoryName(s.substr(repo_p + 2)));
        s.erase(repo_p);
    }

    std::string::size_type slot_p;
    if (std::string::npos != ((slot_p = s.rfind(':'))))
    {
        result.slot(SlotName(s.substr(slot_p + 1)));
        s.erase(slot_p);
    }

    if (std::string::npos != std::string("<>=~").find(s.at(0)))
    {
        if (had_bracket_version_requirements)
            throw PackageDepSpecError("Cannot mix [] and traditional version specifications");

        std::string::size_type p(1);
        if (s.length() > 1 && std::string::npos != std::string("<>=~").find(s.at(1)))
            ++p;
        VersionOperator op(s.substr(0, p));
        std::string::size_type q(p);

        while (true)
        {
            if (p >= s.length())
                throw PackageDepSpecError("Couldn't parse dep spec '" + ss + "'");
            q = s.find('-', q + 1);
            if ((std::string::npos == q) || (++q >= s.length()))
                throw PackageDepSpecError("Couldn't parse dep spec '" + ss + "'");
            if ((s.at(q) >= '0' && s.at(q) <= '9') || (0 == s.compare(q, 3, "scm")))
                break;
        }

        std::string::size_type new_q(q);
        while (true)
        {
            if (new_q >= s.length())
                break;
            new_q = s.find('-', new_q + 1);
            if ((std::string::npos == new_q) || (++new_q >= s.length()))
                break;
            if (s.at(new_q) >= '0' && s.at(new_q) <= '9')
                q = new_q;
        }

        std::string t(s.substr(p, q - p - 1));
        if (t.length() >= 3 && (0 == t.compare(0, 2, "*/")))
        {
            if (! options[updso_allow_wildcards])
                throw PackageDepSpecError("Wildcard '*' not allowed in '" + stringify(ss) + "'");

            if (0 != t.compare(t.length() - 2, 2, "/*"))
                result.package_name_part(PackageNamePart(t.substr(2)));
        }
        else if (t.length() >= 3 && (0 == t.compare(t.length() - 2, 2, "/*")))
        {
            if (! options[updso_allow_wildcards])
                throw PackageDepSpecError("Wildcard '*' not allowed in '" + stringify(ss) + "'");

            result.category_name_part(CategoryNamePart(t.substr(0, t.length() - 2)));
        }
        else
            result.package(QualifiedPackageName(t));

        if ('*' == s.at(s.length() - 1))
        {
            if (op != vo_equal)
                Log::get_instance()->message(ll_qa, lc_context,
                        "Package dep spec '" + ss + "' uses * "
                        "with operator '" + stringify(op) +
                        "', pretending it uses the equals operator instead");
            op = vo_equal_star;

            result.version_requirement(VersionRequirement(op, VersionSpec(s.substr(q, s.length() - q - 1))));
        }
        else
            result.version_requirement(VersionRequirement(op, VersionSpec(s.substr(q))));
    }
    else
    {
        if (s.length() >= 3 && (0 == s.compare(0, 2, "*/")))
        {
            if (! options[updso_allow_wildcards])
                throw PackageDepSpecError("Wildcard '*' not allowed in '" + stringify(ss) + "'");

            if (0 != s.compare(s.length() - 2, 2, "/*"))
                result.package_name_part(PackageNamePart(s.substr(2)));
        }
        else if (s.length() >= 3 && (0 == s.compare(s.length() - 2, 2, "/*")))
        {
            if (! options[updso_allow_wildcards])
                throw PackageDepSpecError("Wildcard '*' not allowed in '" + stringify(ss) + "'");

            result.category_name_part(CategoryNamePart(s.substr(0, s.length() - 2)));
        }
        else
            result.package(QualifiedPackageName(s));
    }

    return result;
}

PartiallyMadePackageDepSpec
paludis::make_package_dep_spec()
{
    return PartiallyMadePackageDepSpec();
}

namespace
{
    struct UseRequirementPrinter :
        ConstVisitor<UseRequirementVisitorTypes>
    {
        std::ostringstream s;

        void visit(const EnabledUseRequirement & r)
        {
            s << "[" << r.flag() << "]";
        }

        void visit(const DisabledUseRequirement & r)
        {
            s << "[-" << r.flag() << "]";
        }

        void visit(const IfMineThenUseRequirement & r)
        {
            s << "[" << r.flag() << "?]";
        }

        void visit(const IfNotMineThenUseRequirement & r)
        {
            s << "[" << r.flag() << "!?]";
        }

        void visit(const IfMineThenNotUseRequirement & r)
        {
            s << "[-" << r.flag() << "?]";
        }

        void visit(const IfNotMineThenNotUseRequirement & r)
        {
            s << "[-" << r.flag() << "!?]";
        }

        void visit(const EqualUseRequirement & r)
        {
            s << "[" << r.flag() << "=]";
        }

        void visit(const NotEqualUseRequirement & r)
        {
            s << "[" << r.flag() << "!=]";
        }
    };

    struct PartiallyMadePackageDepSpecData :
        PackageDepSpecData
    {
        tr1::shared_ptr<QualifiedPackageName> package;
        tr1::shared_ptr<PackageNamePart> package_name_part;
        tr1::shared_ptr<CategoryNamePart> category_name_part;
        tr1::shared_ptr<VersionRequirements> version_requirements;
        VersionRequirementsMode version_requirements_mode_v;
        tr1::shared_ptr<SlotName> slot;
        tr1::shared_ptr<RepositoryName> repository;
        tr1::shared_ptr<UseRequirements> use_requirements;

        PartiallyMadePackageDepSpecData() :
            PackageDepSpecData(),
            version_requirements_mode_v(vr_and)
        {
        }

        PartiallyMadePackageDepSpecData(const PartiallyMadePackageDepSpecData & other) :
            PackageDepSpecData(other),
            package(other.package),
            package_name_part(other.package_name_part),
            category_name_part(other.category_name_part),
            version_requirements(other.version_requirements),
            version_requirements_mode_v(other.version_requirements_mode_v),
            slot(other.slot),
            repository(other.repository),
            use_requirements(other.use_requirements)
        {
        }

        virtual std::string as_string() const
        {
            std::ostringstream s;

            if (version_requirements_ptr())
            {
                if (version_requirements_ptr()->begin() == version_requirements_ptr()->end())
                {
                }
                else if (next(version_requirements_ptr()->begin()) == version_requirements_ptr()->end())
                {
                    if (version_requirements_ptr()->begin()->version_operator == vo_equal_star)
                        s << "=";
                    else
                        s << version_requirements_ptr()->begin()->version_operator;
                }
            }

            if (package_ptr())
                s << *package_ptr();
            else
            {
                if (category_name_part_ptr())
                    s << *category_name_part_ptr();
                else
                    s << "*";

                s << "/";

                if (package_name_part_ptr())
                    s << *package_name_part_ptr();
                else
                    s << "*";
            }

            if (version_requirements_ptr())
            {
                if (version_requirements_ptr()->begin() == version_requirements_ptr()->end())
                {
                }
                else if (next(version_requirements_ptr()->begin()) == version_requirements_ptr()->end())
                {
                    s << "-" << version_requirements_ptr()->begin()->version_spec;
                    if (version_requirements_ptr()->begin()->version_operator == vo_equal_star)
                        s << "*";
                }
            }

            if (slot_ptr())
                s << ":" << *slot_ptr();
            if (repository_ptr())
                s << "::" << *repository_ptr();

            if (version_requirements_ptr())
            {
                if (version_requirements_ptr()->begin() == version_requirements_ptr()->end())
                {
                }
                else if (next(version_requirements_ptr()->begin()) == version_requirements_ptr()->end())
                {
                }
                else
                {
                    bool need_op(false);
                    s << "[";
                    for (VersionRequirements::ConstIterator r(version_requirements_ptr()->begin()),
                            r_end(version_requirements_ptr()->end()) ; r != r_end ; ++r)
                    {
                        if (need_op)
                        {
                            do
                            {
                                switch (version_requirements_mode())
                                {
                                    case vr_and:
                                        s << "&";
                                        continue;

                                    case vr_or:
                                        s << "|";
                                        continue;

                                    case last_vr:
                                        ;
                                }
                                throw InternalError(PALUDIS_HERE, "Bad version_requirements_mode");
                            } while (false);
                        }

                        if (r->version_operator == vo_equal_star)
                            s << "=";
                        else
                            s << r->version_operator;

                        s << r->version_spec;

                        if (r->version_operator == vo_equal_star)
                            s << "*";

                        need_op = true;
                    }
                    s << "]";
                }
            }

            if (use_requirements_ptr())
            {
                for (UseRequirements::ConstIterator u(use_requirements_ptr()->begin()),
                        u_end(use_requirements_ptr()->end()) ; u != u_end ; ++u)
                {
                    UseRequirementPrinter p;
                    (*u)->accept(p);
                    s << p.s.str();
                }
            }

            return s.str();
        }

        virtual tr1::shared_ptr<const QualifiedPackageName> package_ptr() const
        {
            return package;
        }

        virtual tr1::shared_ptr<const PackageNamePart> package_name_part_ptr() const
        {
            return package_name_part;
        }

        virtual tr1::shared_ptr<const CategoryNamePart> category_name_part_ptr() const
        {
            return category_name_part;
        }

        virtual tr1::shared_ptr<const VersionRequirements> version_requirements_ptr() const
        {
            return version_requirements;
        }

        virtual const VersionRequirementsMode version_requirements_mode() const
        {
            return version_requirements_mode_v;
        }

        virtual tr1::shared_ptr<const SlotName> slot_ptr() const
        {
            return slot;
        }

        virtual tr1::shared_ptr<const RepositoryName> repository_ptr() const
        {
            return repository;
        }

        virtual tr1::shared_ptr<const UseRequirements> use_requirements_ptr() const
        {
            return use_requirements;
        }
    };
}

namespace paludis
{
    template <>
    struct Implementation<PartiallyMadePackageDepSpec>
    {
        tr1::shared_ptr<PartiallyMadePackageDepSpecData> data;

        Implementation() :
            data(new PartiallyMadePackageDepSpecData)
        {
        }

        Implementation(const Implementation & other) :
            data(new PartiallyMadePackageDepSpecData(*other.data))
        {
        }
    };
}

PartiallyMadePackageDepSpec::PartiallyMadePackageDepSpec() :
    PrivateImplementationPattern<PartiallyMadePackageDepSpec>(new Implementation<PartiallyMadePackageDepSpec>)
{
}

PartiallyMadePackageDepSpec::PartiallyMadePackageDepSpec(const PartiallyMadePackageDepSpec & other) :
    PrivateImplementationPattern<PartiallyMadePackageDepSpec>(new Implementation<PartiallyMadePackageDepSpec>(*other._imp.get()))
{
}

PartiallyMadePackageDepSpec::~PartiallyMadePackageDepSpec()
{
}

PartiallyMadePackageDepSpec &
PartiallyMadePackageDepSpec::package(const QualifiedPackageName & name)
{
    _imp->data->package.reset(new QualifiedPackageName(name));
    return *this;
}

PartiallyMadePackageDepSpec &
PartiallyMadePackageDepSpec::slot(const SlotName & s)
{
    _imp->data->slot.reset(new SlotName(s));
    return *this;
}

PartiallyMadePackageDepSpec &
PartiallyMadePackageDepSpec::repository(const RepositoryName & repo)
{
   _imp->data->repository.reset(new RepositoryName(repo));
   return *this;
}

PartiallyMadePackageDepSpec &
PartiallyMadePackageDepSpec::package_name_part(const PackageNamePart & part)
{
    _imp->data->package_name_part.reset(new PackageNamePart(part));
    return *this;
}

PartiallyMadePackageDepSpec &
PartiallyMadePackageDepSpec::category_name_part(const CategoryNamePart & part)
{
    _imp->data->category_name_part.reset(new CategoryNamePart(part));
    return *this;
}

PartiallyMadePackageDepSpec &
PartiallyMadePackageDepSpec::version_requirement(const VersionRequirement & req)
{
    if (! _imp->data->version_requirements)
        _imp->data->version_requirements.reset(new VersionRequirements);
    _imp->data->version_requirements->push_back(req);
    return *this;
}

PartiallyMadePackageDepSpec &
PartiallyMadePackageDepSpec::version_requirements_mode(const VersionRequirementsMode & mode)
{
    _imp->data->version_requirements_mode_v = mode;
    return *this;
}

PartiallyMadePackageDepSpec &
PartiallyMadePackageDepSpec::use_requirement(const tr1::shared_ptr<const UseRequirement> & req)
{
    if (! _imp->data->use_requirements)
        _imp->data->use_requirements.reset(new UseRequirements);
    _imp->data->use_requirements->insert(req);
    return *this;
}

PartiallyMadePackageDepSpec::operator const PackageDepSpec() const
{
    return PackageDepSpec(_imp->data);
}

const PackageDepSpec
PartiallyMadePackageDepSpec::to_package_dep_spec() const
{
    return operator const PackageDepSpec();
}

template class WrappedForwardIterator<UseRequirements::ConstIteratorTag, const std::pair<const UseFlagName, UseFlagState> >;

template class LabelsDepSpec<URILabelVisitorTypes>;
template class WrappedForwardIterator<LabelsDepSpec<URILabelVisitorTypes>::ConstIteratorTag,
         const tr1::shared_ptr<const URILabelVisitorTypes::BasicNode> >;

template class LabelsDepSpec<DependencyLabelVisitorTypes>;
template class WrappedForwardIterator<LabelsDepSpec<DependencyLabelVisitorTypes>::ConstIteratorTag,
         const tr1::shared_ptr<const DependencyLabelVisitorTypes::BasicNode> >;

