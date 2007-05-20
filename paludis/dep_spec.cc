/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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
#include <paludis/util/log.hh>
#include <paludis/util/collection_concrete.hh>
#include <paludis/util/iterator.hh>

#include <list>
#include <map>

/** \file
 * Implementation for dep_spec.hh things.
 *
 * \ingroup grpdepspecs
 */

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

namespace paludis
{
    /**
     * Implementation data for CompositeDepSpec.
     *
     * \ingroup grpdepspecs
     */
    template<>
    struct Implementation<CompositeDepSpec>
    {
        std::list<tr1::shared_ptr<const DepSpec> > children;
    };

    template<>
    struct Implementation<PackageDepSpec>
    {
        bool unique;

        tr1::shared_ptr<QualifiedPackageName> package_ptr;
        tr1::shared_ptr<CategoryNamePart> category_name_part_ptr;
        tr1::shared_ptr<PackageNamePart> package_name_part_ptr;
        tr1::shared_ptr<VersionRequirements> version_requirements;
        VersionRequirementsMode version_requirements_mode;
        tr1::shared_ptr<SlotName> slot;
        tr1::shared_ptr<RepositoryName> repository;
        tr1::shared_ptr<UseRequirements> use_requirements;
        tr1::shared_ptr<const DepTag> tag;

        Implementation(
                tr1::shared_ptr<QualifiedPackageName> q,
                tr1::shared_ptr<CategoryNamePart> c,
                tr1::shared_ptr<PackageNamePart> p,
                tr1::shared_ptr<VersionRequirements> v,
                VersionRequirementsMode m,
                tr1::shared_ptr<SlotName> s,
                tr1::shared_ptr<RepositoryName> r,
                tr1::shared_ptr<UseRequirements> u,
                tr1::shared_ptr<const DepTag> t) :
            unique(false),
            package_ptr(q),
            category_name_part_ptr(c),
            package_name_part_ptr(p),
            version_requirements(v),
            version_requirements_mode(m),
            slot(s),
            repository(r),
            use_requirements(u),
            tag(t)
        {
        }
    };
}

CompositeDepSpec::CompositeDepSpec() :
    PrivateImplementationPattern<CompositeDepSpec>(new Implementation<CompositeDepSpec>)
{
}

CompositeDepSpec::~CompositeDepSpec()
{
}

void
CompositeDepSpec::add_child(tr1::shared_ptr<const DepSpec> c)
{
    _imp->children.push_back(c);
}

CompositeDepSpec::Iterator
CompositeDepSpec::begin() const
{
    return Iterator(_imp->children.begin());
}

CompositeDepSpec::Iterator
CompositeDepSpec::end() const
{
    return Iterator(_imp->children.end());
}

AnyDepSpec::AnyDepSpec()
{
}

AllDepSpec::AllDepSpec()
{
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

std::string
StringDepSpec::text() const
{
    return _str;
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

PackageDepSpec::PackageDepSpec(const QualifiedPackageName & our_package) :
    StringDepSpec(stringify(our_package)),
    PrivateImplementationPattern<PackageDepSpec>(new Implementation<PackageDepSpec>(
                tr1::shared_ptr<QualifiedPackageName>(new QualifiedPackageName(our_package)),
                tr1::shared_ptr<CategoryNamePart>(),
                tr1::shared_ptr<PackageNamePart>(),
                tr1::shared_ptr<VersionRequirements>(),
                vr_and,
                tr1::shared_ptr<SlotName>(),
                tr1::shared_ptr<RepositoryName>(),
                tr1::shared_ptr<UseRequirements>(),
                tr1::shared_ptr<const DepTag>()))
{
}

PackageDepSpec::PackageDepSpec(const PackageDepSpec & other) :
    VisitableInterface<DepSpecVisitorTypes>(other),
    StringDepSpec(stringify(other)),
    Visitable<PackageDepSpec, DepSpecVisitorTypes>(other),
    PrivateImplementationPattern<PackageDepSpec>(new Implementation<PackageDepSpec>(
                other._imp->package_ptr,
                other._imp->category_name_part_ptr,
                other._imp->package_name_part_ptr,
                other._imp->version_requirements,
                other._imp->version_requirements_mode,
                other._imp->slot,
                other._imp->repository,
                other._imp->use_requirements,
                other._imp->tag))
{
}

PackageDepSpec::PackageDepSpec(const std::string & ss) :
    StringDepSpec(ss),
    PrivateImplementationPattern<PackageDepSpec>(new Implementation<PackageDepSpec>(
                tr1::shared_ptr<QualifiedPackageName>(),
                tr1::shared_ptr<CategoryNamePart>(),
                tr1::shared_ptr<PackageNamePart>(),
                tr1::shared_ptr<VersionRequirements>(),
                vr_and,
                tr1::shared_ptr<SlotName>(),
                tr1::shared_ptr<RepositoryName>(),
                tr1::shared_ptr<UseRequirements>(),
                tr1::shared_ptr<const DepTag>()))
{
    _do_parse(ss, pds_pm_permissive);
    _imp->unique = true;
}

PackageDepSpec::PackageDepSpec(const std::string & ss, const PackageDepSpecParseMode p) :
    StringDepSpec(ss),
    PrivateImplementationPattern<PackageDepSpec>(new Implementation<PackageDepSpec>(
                tr1::shared_ptr<QualifiedPackageName>(),
                tr1::shared_ptr<CategoryNamePart>(),
                tr1::shared_ptr<PackageNamePart>(),
                tr1::shared_ptr<VersionRequirements>(),
                vr_and,
                tr1::shared_ptr<SlotName>(),
                tr1::shared_ptr<RepositoryName>(),
                tr1::shared_ptr<UseRequirements>(),
                tr1::shared_ptr<const DepTag>()))
{
    _do_parse(ss, p);
    _imp->unique = true;
}

PackageDepSpec::PackageDepSpec(
        tr1::shared_ptr<QualifiedPackageName> q,
        tr1::shared_ptr<CategoryNamePart> c,
        tr1::shared_ptr<PackageNamePart> p,
        tr1::shared_ptr<VersionRequirements> v,
        VersionRequirementsMode m,
        tr1::shared_ptr<SlotName> s,
        tr1::shared_ptr<RepositoryName> r,
        tr1::shared_ptr<UseRequirements> u,
        tr1::shared_ptr<const DepTag> t) :
    StringDepSpec(""),
    PrivateImplementationPattern<PackageDepSpec>(new Implementation<PackageDepSpec>(
                q, c, p, v, m, s, r, u, t))
{
    set_text(stringify(*this));
}

void
StringDepSpec::set_text(const std::string & t)
{
    _str = t;
}

void
PackageDepSpec::_do_parse(const std::string & ss, const PackageDepSpecParseMode mode)
{
    Context context("When parsing package dep spec '" + ss + "' with parse mode '" + stringify(mode) + "':");

    try
    {
        std::string s(ss);

        if (s.empty())
            throw PackageDepSpecError("Got empty dep spec");

        std::string::size_type use_group_p;
        while (std::string::npos != ((use_group_p = s.rfind('['))))
        {
            switch (mode)
            {
                case pds_pm_unspecific:
                case pds_pm_permissive:
                case last_pds_pm:
                    break;

                case pds_pm_eapi_0:
                    Log::get_instance()->message(ll_warning, lc_context, "[] dependencies not safe for use with this EAPI");
                    break;

                case pds_pm_eapi_0_strict:
                    throw PackageDepSpecError("[] dependencies not safe for use with this EAPI");
            }

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
                        _imp->version_requirements.reset(new VersionRequirements::Concrete);
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

                                _imp->version_requirements_mode = (flag.at(opos) == '|' ? vr_or : vr_and);
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
                            _imp->version_requirements->push_back(VersionRequirement(vop, vs));
                        }
                    }
                    break;

                default:
                    {
                        UseFlagState state(use_enabled);
                        if ('-' == flag.at(0))
                        {
                            state = use_disabled;
                            flag.erase(0, 1);
                            if (flag.empty())
                                throw PackageDepSpecError("Invalid [] contents");
                        }
                        UseFlagName name(flag);
                        if (! _imp->use_requirements)
                            _imp->use_requirements.reset(new UseRequirements);
                        if (! _imp->use_requirements->insert(name, state))
                            throw PackageDepSpecError("Conflicting [] contents");
                    }
                    break;
            };

            s.erase(use_group_p);
        }

        std::string::size_type repo_p;
        if (std::string::npos != ((repo_p = s.rfind("::"))))
        {
            switch (mode)
            {
                case pds_pm_unspecific:
                case pds_pm_permissive:
                case last_pds_pm:
                    break;

                case pds_pm_eapi_0:
                    Log::get_instance()->message(ll_warning, lc_context, "Repository dependencies not safe for use with this EAPI");
                    break;

                case pds_pm_eapi_0_strict:
                    throw PackageDepSpecError("Repository dependencies not safe for use with this EAPI");
            }

            _imp->repository.reset(new RepositoryName(s.substr(repo_p + 2)));
            s.erase(repo_p);
        }

        std::string::size_type slot_p;
        if (std::string::npos != ((slot_p = s.rfind(':'))))
        {
            switch (mode)
            {
                case pds_pm_unspecific:
                case pds_pm_permissive:
                case last_pds_pm:
                    break;

                case pds_pm_eapi_0:
                    Log::get_instance()->message(ll_warning, lc_context, "SLOT dependencies not safe for use with this EAPI");
                    break;

                case pds_pm_eapi_0_strict:
                    throw PackageDepSpecError("SLOT dependencies not safe for use with this EAPI");
            }

            _imp->slot.reset(new SlotName(s.substr(slot_p + 1)));
            s.erase(slot_p);
        }

        if (std::string::npos != std::string("<>=~").find(s.at(0)))
        {
            if (_imp->version_requirements)
                throw PackageDepSpecError("Cannot mix [] and traditional version specifications");

            std::string::size_type p(1);
            if (s.length() > 1 && std::string::npos != std::string("<>=~").find(s.at(1)))
                ++p;
            VersionOperator op(s.substr(0, p));

            if (op == vo_tilde_greater)
                switch (mode)
                {
                    case pds_pm_unspecific:
                    case pds_pm_permissive:
                    case last_pds_pm:
                        break;

                    case pds_pm_eapi_0:
                        Log::get_instance()->message(ll_warning, lc_context, "~> dependencies not safe for use with this EAPI");
                        break;

                    case pds_pm_eapi_0_strict:
                        throw PackageDepSpecError("~> dependencies not safe for use with this EAPI");
                }

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
                if (pds_pm_unspecific != mode)
                    throw PackageDepSpecError("Wildcard '*' not allowed in '" + stringify(ss) + "' with parse mode '"
                            + stringify(mode) + "'");

                if (0 != t.compare(t.length() - 2, 2, "/*"))
                    _imp->package_name_part_ptr.reset(new PackageNamePart(t.substr(2)));
            }
            else if (t.length() >= 3 && (0 == t.compare(t.length() - 2, 2, "/*")))
            {
                if (pds_pm_unspecific != mode)
                    throw PackageDepSpecError("Wildcard '*' not allowed in '" + stringify(ss) + "' with parse mode '"
                            + stringify(mode) + "'");

                _imp->category_name_part_ptr.reset(new CategoryNamePart(t.substr(0, t.length() - 2)));
            }
            else
                _imp->package_ptr.reset(new QualifiedPackageName(t));

            _imp->version_requirements.reset(new VersionRequirements::Concrete);

            if ('*' == s.at(s.length() - 1))
            {
                if (op != vo_equal)
                {
                    switch (mode)
                    {
                        case pds_pm_unspecific:
                        case pds_pm_permissive:
                        case last_pds_pm:
                        case pds_pm_eapi_0:
                            Log::get_instance()->message(ll_qa, lc_context,
                                    "Package dep spec '" + ss + "' uses * "
                                    "with operator '" + stringify(op) +
                                    "', pretending it uses the equals operator instead");
                            break;

                        case pds_pm_eapi_0_strict:
                            throw PackageDepSpecError(
                                    "Package dep spec '" + ss + "' uses * "
                                    "with operator '" + stringify(op) + "'");
                    }
                }
                op = vo_equal_star;

                _imp->version_requirements->push_back(VersionRequirement(op, VersionSpec(s.substr(q, s.length() - q - 1))));
            }
            else
                _imp->version_requirements->push_back(VersionRequirement(op, VersionSpec(s.substr(q))));
        }
        else
        {
            if (s.length() >= 3 && (0 == s.compare(0, 2, "*/")))
            {
                if (pds_pm_unspecific != mode)
                    throw PackageDepSpecError("Wildcard '*' not allowed in '" + stringify(ss) + "' with parse mode '"
                            + stringify(mode) + "'");

                if (0 != s.compare(s.length() - 2, 2, "/*"))
                    _imp->package_name_part_ptr.reset(new PackageNamePart(s.substr(2)));
            }
            else if (s.length() >= 3 && (0 == s.compare(s.length() - 2, 2, "/*")))
            {
                if (pds_pm_unspecific != mode)
                    throw PackageDepSpecError("Wildcard '*' not allowed in '" + stringify(ss) + "' with parse mode '"
                            + stringify(mode) + "'");

                _imp->category_name_part_ptr.reset(new CategoryNamePart(s.substr(0, s.length() - 2)));
            }
            else
                _imp->package_ptr.reset(new QualifiedPackageName(s));
        }
    }
    catch (Exception &)
    {
        throw;
    }
    catch (const std::exception & e)
    {
        throw InternalError(PALUDIS_HERE, "caught std::exception '"
                + stringify(e.what()) + "'");
    }
}

PackageDepSpec::~PackageDepSpec()
{
}

std::ostream &
paludis::operator<< (std::ostream & s, const PlainTextDepSpec & a)
{
    s << a.text();
    return s;
}

std::ostream &
paludis::operator<< (std::ostream & s, const PackageDepSpec & a)
{
    if (a.version_requirements_ptr())
    {
        if (a.version_requirements_ptr()->begin() == a.version_requirements_ptr()->end())
        {
        }
        else if (next(a.version_requirements_ptr()->begin()) == a.version_requirements_ptr()->end())
        {
            if (a.version_requirements_ptr()->begin()->version_operator == vo_equal_star)
                s << "=";
            else
               s << a.version_requirements_ptr()->begin()->version_operator;
        }
    }

    if (a.package_ptr())
        s << *a.package_ptr();
    else
    {
        if (a.category_name_part_ptr())
            s << *a.category_name_part_ptr();
        else
            s << "*";

        s << "/";

        if (a.package_name_part_ptr())
            s << *a.package_name_part_ptr();
        else
            s << "*";
    }

    if (a.version_requirements_ptr())
    {
        if (a.version_requirements_ptr()->begin() == a.version_requirements_ptr()->end())
        {
        }
        else if (next(a.version_requirements_ptr()->begin()) == a.version_requirements_ptr()->end())
        {
            s << "-" << a.version_requirements_ptr()->begin()->version_spec;
            if (a.version_requirements_ptr()->begin()->version_operator == vo_equal_star)
                s << "*";
        }
    }

    if (a.slot_ptr())
        s << ":" << *a.slot_ptr();
    if (a.repository_ptr())
        s << "::" << *a.repository_ptr();

    if (a.version_requirements_ptr())
    {
        if (a.version_requirements_ptr()->begin() == a.version_requirements_ptr()->end())
        {
        }
        else if (next(a.version_requirements_ptr()->begin()) == a.version_requirements_ptr()->end())
        {
        }
        else
        {
            bool need_op(false);
            s << "[";
            for (VersionRequirements::Iterator r(a.version_requirements_ptr()->begin()),
                    r_end(a.version_requirements_ptr()->end()) ; r != r_end ; ++r)
            {
                if (need_op)
                {
                    do
                    {
                        switch (a.version_requirements_mode())
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

    if (a.use_requirements_ptr())
    {
        for (UseRequirements::Iterator u(a.use_requirements_ptr()->begin()),
                u_end(a.use_requirements_ptr()->end()) ; u != u_end ; ++u)
            s << "[" << (u->second == use_disabled ? "-" + stringify(u->first) :
                    stringify(u->first)) << "]";
    }

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

namespace paludis
{
    /**
     * Implementation data for UseRequirements.
     *
     * \ingroup grpdepspecs
     */
    template<>
    struct Implementation<UseRequirements>
    {
        std::map<UseFlagName, UseFlagState> reqs;
    };
}

UseRequirements::UseRequirements() :
    PrivateImplementationPattern<UseRequirements>(new Implementation<UseRequirements>)
{
}

UseRequirements::UseRequirements(const UseRequirements & c) :
    PrivateImplementationPattern<UseRequirements>(new Implementation<UseRequirements>)
{
    _imp->reqs = c._imp->reqs;
}

UseRequirements::~UseRequirements()
{
}

UseRequirements::Iterator
UseRequirements::begin() const
{
    return Iterator(_imp->reqs.begin());
}

UseRequirements::Iterator
UseRequirements::end() const
{
    return Iterator(_imp->reqs.end());
}

UseRequirements::Iterator
UseRequirements::find(const UseFlagName & u) const
{
    return Iterator(_imp->reqs.find(u));
}

bool
UseRequirements::insert(const UseFlagName & u, UseFlagState s)
{
    return _imp->reqs.insert(std::make_pair(u, s)).second;
}

UseFlagState
UseRequirements::state(const UseFlagName & u) const
{
    Iterator i(find(u));
    if (end() == i)
        return use_unspecified;
    return i->second;
}

tr1::shared_ptr<PackageDepSpec>
PackageDepSpec::without_use_requirements() const
{
    tr1::shared_ptr<PackageDepSpec> result(new PackageDepSpec(*this));
    result->_make_unique();
    result->_imp->use_requirements.reset();
    return result;
}

tr1::shared_ptr<const QualifiedPackageName>
PackageDepSpec::package_ptr() const
{
    return _imp->package_ptr;
}

tr1::shared_ptr<const PackageNamePart>
PackageDepSpec::package_name_part_ptr() const
{
    return _imp->package_name_part_ptr;
}

tr1::shared_ptr<const CategoryNamePart>
PackageDepSpec::category_name_part_ptr() const
{
    return _imp->category_name_part_ptr;
}

tr1::shared_ptr<const VersionRequirements>
PackageDepSpec::version_requirements_ptr() const
{
    return _imp->version_requirements;
}

tr1::shared_ptr<VersionRequirements>
PackageDepSpec::version_requirements_ptr()
{
    _make_unique();
    return _imp->version_requirements;
}

VersionRequirementsMode
PackageDepSpec::version_requirements_mode() const
{
    return _imp->version_requirements_mode;
}

void
PackageDepSpec::set_version_requirements_mode(const VersionRequirementsMode m)
{
    _imp->version_requirements_mode = m;
}

tr1::shared_ptr<const SlotName>
PackageDepSpec::slot_ptr() const
{
    return _imp->slot;
}

tr1::shared_ptr<const RepositoryName>
PackageDepSpec::repository_ptr() const
{
    return _imp->repository;
}

tr1::shared_ptr<const UseRequirements>
PackageDepSpec::use_requirements_ptr() const
{
    return _imp->use_requirements;
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

void
PackageDepSpec::_make_unique()
{
    if (_imp->unique)
        return;

    if (_imp->package_ptr && ! _imp->package_ptr.unique())
        _imp->package_ptr.reset(new QualifiedPackageName(*_imp->package_ptr));

    if (_imp->category_name_part_ptr && ! _imp->category_name_part_ptr.unique())
        _imp->category_name_part_ptr.reset(new CategoryNamePart(*_imp->category_name_part_ptr));

    if (_imp->package_name_part_ptr && ! _imp->package_name_part_ptr.unique())
        _imp->package_name_part_ptr.reset(new PackageNamePart(*_imp->package_name_part_ptr));

    if (_imp->version_requirements && ! _imp->version_requirements.unique())
    {
        tr1::shared_ptr<VersionRequirements> v(new VersionRequirements::Concrete);
        std::copy(_imp->version_requirements->begin(), _imp->version_requirements->end(), v->inserter());
        _imp->version_requirements = v;
    }

    if (_imp->slot && ! _imp->slot.unique())
        _imp->slot.reset(new SlotName(*_imp->slot));

    if (_imp->repository && ! _imp->repository.unique())
        _imp->repository.reset(new RepositoryName(*_imp->repository));

    if (_imp->use_requirements && ! _imp->use_requirements.unique())
        _imp->use_requirements.reset(new UseRequirements(*_imp->use_requirements));

    _imp->unique = true;
}

tr1::shared_ptr<const PackageDepSpec>
BlockDepSpec::blocked_spec() const
{
    return _spec;
}

