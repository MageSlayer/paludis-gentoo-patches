/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <paludis/dep_atom.hh>
#include <paludis/util/log.hh>

#include <list>
#include <map>

/** \file
 * Implementation for dep_atom.hh things.
 *
 * \ingroup grpdepatoms
 */

using namespace paludis;

DepAtom::DepAtom()
{
}

DepAtom::~DepAtom()
{
}

const UseDepAtom *
DepAtom::as_use_dep_atom() const
{
    return 0;
}

namespace paludis
{
    /**
     * Implementation data for CompositeDepAtom.
     *
     * \ingroup grpdepatoms
     */
    template<>
    struct Implementation<CompositeDepAtom> :
        InternalCounted<Implementation<CompositeDepAtom> >
    {
        std::list<DepAtom::ConstPointer> children;
    };
}

CompositeDepAtom::CompositeDepAtom() :
    PrivateImplementationPattern<CompositeDepAtom>(new Implementation<CompositeDepAtom>)
{
}

CompositeDepAtom::~CompositeDepAtom()
{
}

void
CompositeDepAtom::add_child(DepAtom::ConstPointer c)
{
    _imp->children.push_back(c);
}

CompositeDepAtom::Iterator
CompositeDepAtom::begin() const
{
    return Iterator(_imp->children.begin());
}

CompositeDepAtom::Iterator
CompositeDepAtom::end() const
{
    return Iterator(_imp->children.end());
}

AnyDepAtom::AnyDepAtom()
{
}

AllDepAtom::AllDepAtom()
{
}

UseDepAtom::UseDepAtom(const UseFlagName & our_flag, bool is_inverse) :
    _flag(our_flag),
    _inverse(is_inverse)
{
}

const UseDepAtom *
UseDepAtom::as_use_dep_atom() const
{
    return this;
}

BlockDepAtom::BlockDepAtom(PackageDepAtom::ConstPointer a) :
    StringDepAtom("!" + a->text()),
    _atom(a)
{
}

PackageDepAtom::PackageDepAtom(const QualifiedPackageName & our_package) :
    StringDepAtom(stringify(our_package)),
    _package(our_package),
    _version_operator("="),
    _version_spec(0),
    _slot(0),
    _repository(0),
    _use_requirements(0),
    _tag(0)
{
}

PackageDepAtom::PackageDepAtom(const PackageDepAtom & other) :
    VisitableInterface<DepAtomVisitorTypes>(other),
    StringDepAtom(stringify(other)),
    Visitable<PackageDepAtom, DepAtomVisitorTypes>(other),
    _package(other._package),
    _version_operator(other._version_operator),
    _version_spec(other._version_spec),
    _slot(other._slot),
    _repository(other._repository),
    _use_requirements(other._use_requirements),
    _tag(other._tag)
{
}

PackageDepAtom::PackageDepAtom(const std::string & ss) :
    StringDepAtom(ss),
    _package(CategoryNamePart("later"), PackageNamePart("later")),
    _version_operator("="),
    _version_spec(0),
    _slot(0),
    _repository(0),
    _use_requirements(0),
    _tag(0)
{
    Context context("When parsing package dep atom '" + ss + "':");

    try
    {
        std::string s(ss);

        if (s.empty())
            throw PackageDepAtomError("Got empty dep atom");

        std::string::size_type use_group_p;
        while (std::string::npos != ((use_group_p = s.rfind('['))))
        {
            if (s.at(s.length() - 1) != ']')
                throw PackageDepAtomError("Mismatched []");

            std::string flag(s.substr(use_group_p + 1));
            UseFlagState state(use_enabled);
            if (flag.length() < 2)
                throw PackageDepAtomError("Invalid [] contents");
            flag.erase(flag.length() - 1);
            if ('-' == flag.at(0))
            {
                state = use_disabled;
                flag.erase(0, 1);
                if (flag.empty())
                    throw PackageDepAtomError("Invalid [] contents");
            }
            UseFlagName name(flag);
            if (0 == _use_requirements)
                _use_requirements.assign(new UseRequirements);
            if (! _use_requirements->insert(name, state))
                throw PackageDepAtomError("Conflicting [] contents");

            s.erase(use_group_p);
        }

        std::string::size_type repo_p;
        if (std::string::npos != ((repo_p = s.rfind("::"))))
        {
            _repository.assign(new RepositoryName(s.substr(repo_p + 2)));
            s.erase(repo_p);
        }

        std::string::size_type slot_p;
        if (std::string::npos != ((slot_p = s.rfind(':'))))
        {
            _slot.assign(new SlotName(s.substr(slot_p + 1)));
            s.erase(slot_p);
        }

        if (std::string::npos != std::string("<>=~").find(s.at(0)))
        {
            std::string::size_type p(1);
            if (s.length() > 1 && std::string::npos != std::string("<>=~").find(s.at(1)))
                ++p;
            _version_operator = s.substr(0, p);

            std::string::size_type q(p);

            while (true)
            {
                if (p >= s.length())
                    throw PackageDepAtomError("Couldn't parse dep atom '" + ss + "'");
                q = s.find('-', q + 1);
                if ((std::string::npos == q) || (++q >= s.length()))
                    throw PackageDepAtomError("Couldn't parse dep atom '" + ss + "'");
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

            _package = QualifiedPackageName(s.substr(p, q - p - 1));

            if ('*' == s.at(s.length() - 1))
            {
                if (_version_operator != vo_equal)
                    Log::get_instance()->message(ll_qa, lc_context,
                            "Package dep atom '" + ss + "' uses * "
                            "with operator '" + stringify(_version_operator) +
                            "', pretending it uses the equals operator instead");
                _version_operator = vo_equal_star;
                _version_spec = CountedPtr<VersionSpec, count_policy::ExternalCountTag>(
                        new VersionSpec(s.substr(q, s.length() - q - 1)));
            }
            else
                _version_spec = CountedPtr<VersionSpec, count_policy::ExternalCountTag>(
                        new VersionSpec(s.substr(q)));
        }
        else
            _package = QualifiedPackageName(s);
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

PackageDepAtom::~PackageDepAtom()
{
}

std::ostream &
paludis::operator<< (std::ostream & s, const PlainTextDepAtom & a)
{
    s << a.text();
    return s;
}

std::ostream &
paludis::operator<< (std::ostream & s, const PackageDepAtom & a)
{
    if (a.version_spec_ptr())
    {
        if (a.version_operator() == vo_equal_star)
            s << "=";
        else
           s << a.version_operator();
    }

    s << a.package();

    if (a.version_spec_ptr())
    {
        s << "-" << *a.version_spec_ptr();

        if (a.version_operator() == vo_equal_star)
            s << "*";
    }

    if (a.slot_ptr())
        s << ":" << *a.slot_ptr();
    if (a.repository_ptr())
        s << "::" << *a.repository_ptr();

    if (a.use_requirements_ptr())
    {
        for (UseRequirements::Iterator u(a.use_requirements_ptr()->begin()),
                u_end(a.use_requirements_ptr()->end()) ; u != u_end ; ++u)
            s << "[" << (u->second == use_disabled ? "-" + stringify(u->first) :
                    stringify(u->first)) << "]";
    }

    return s;
}

PackageDepAtomError::PackageDepAtomError(const std::string & msg) throw () :
    Exception(msg)
{
}

StringDepAtom::StringDepAtom(const std::string & s) :
    _str(s)
{
}

StringDepAtom::~StringDepAtom()
{
}


PlainTextDepAtom::PlainTextDepAtom(const std::string & s) :
    StringDepAtom(s)
{
}

namespace paludis
{
    /**
     * Implementation data for UseRequirements.
     *
     * \ingroup grpdepatoms
     */
    template<>
    struct Implementation<UseRequirements> :
        InternalCounted<Implementation<UseRequirements> >
    {
        std::map<UseFlagName, UseFlagState> reqs;
    };
}

UseRequirements::UseRequirements() :
    PrivateImplementationPattern<UseRequirements>(new Implementation<UseRequirements>)
{
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

PackageDepAtom::Pointer
PackageDepAtom::without_use_requirements() const
{
    std::string s(text());
    if (std::string::npos != s.find('['))
        s.erase(s.find('['));
    return Pointer(new PackageDepAtom(s));
}

