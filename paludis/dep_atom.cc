/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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

CompositeDepAtom::CompositeDepAtom()
{
}

void
CompositeDepAtom::add_child(DepAtom::ConstPointer c)
{
    _children.push_back(c);
}

AnyDepAtom::AnyDepAtom()
{
}

AllDepAtom::AllDepAtom()
{
}

UseDepAtom::UseDepAtom(const UseFlagName & flag, bool inverse) :
    _flag(flag),
    _inverse(inverse)
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

PackageDepAtom::PackageDepAtom(const QualifiedPackageName & package) :
    StringDepAtom(stringify(package)),
    _package(package),
    _version_operator("="),
    _version_spec(0),
    _slot(0)
{
}

PackageDepAtom::PackageDepAtom(const std::string & ss) :
    StringDepAtom(ss),
    _package(CategoryNamePart("later"), PackageNamePart("later")),
    _version_operator("="),
    _version_spec(0),
    _slot(0)
{
    Context context("When parsing package dep atom '" + ss + "':");

    try
    {
        std::string s(ss);

        if (s.empty())
            throw PackageDepAtomError("Got empty dep atom");

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
                if (s.at(q) >= '0' && s.at(q) <= '9')
                    break;
            }
            _package = QualifiedPackageName(s.substr(p, q - p - 1));

            if ('*' == s.at(s.length() - 1))
            {
                if (_version_operator != vo_equal)
                    throw PackageDepAtomError("Package dep atom '" + ss + "' uses * but not equals operator");
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
        s << *a.slot_ptr();
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

PlainTextDepAtom::PlainTextDepAtom(const std::string & s) :
    StringDepAtom(s)
{
}

