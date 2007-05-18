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

#include <paludis/name.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/operators.hh>
#include <paludis/util/log.hh>
#include <ostream>

/** \file
 * Implementation of name.hh things.
 *
 * \ingroup grpnames
 */

using namespace paludis;

#include <paludis/name-sr.cc>
#include <paludis/name-se.cc>

QualifiedPackageNameError::QualifiedPackageNameError(const std::string & s) throw () :
    NameError(s, "qualified package name")
{
}

std::ostream &
paludis::operator<< (std::ostream & s, const QualifiedPackageName & q)
{
    s << q.category << "/" << q.package;
    return s;
}

void
UseFlagNameValidator::validate(const std::string & s)
{
    static const std::string allowed_chars(
            "abcdefghijklmnopqrstuvwxyz"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "0123456789-+_@");

    static const std::string alphanum_chars(
            "abcdefghijklmnopqrstuvwxyz"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "0123456789");

    do
    {
        if (s.empty())
            break;

        if (std::string::npos == alphanum_chars.find(s[0]))
            break;

        if (std::string::npos != s.find_first_not_of(allowed_chars))
            break;

        return;

    } while (false);

    Context c("When validating use flag name '" + s + "':");

    throw UseFlagNameError(s);
}

UseFlagNameError::UseFlagNameError(const std::string & name) throw () :
    NameError(name, "use flag name")
{
}

SlotNameError::SlotNameError(const std::string & name) throw () :
    NameError(name, "slot name")
{
}

void
SlotNameValidator::validate(const std::string & s)
{
    static const std::string allowed_chars(
            "abcdefghijklmnopqrstuvwxyz"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "0123456789-+_.");

    do
    {
        if (s.empty())
            break;

        if ('-' == s[0] || '.' == s[0])
            break;

        if (std::string::npos != s.find_first_not_of(allowed_chars))
            break;

        return;

    } while (false);

    Context c("When validating slot name '" + s + "':");

    throw SlotNameError(s);
}

PackageNamePartError::PackageNamePartError(const std::string & name) throw () :
    NameError(name, "package name part")
{
}

void
PackageNamePartValidator::validate(const std::string & s)
{
    /* this gets called a lot, make it fast */

    static const std::string allowed_chars(
            "abcdefghijklmnopqrstuvwxyz"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "0123456789-+_");

    static const std::string number_chars(
            "0123456789");

    if (s.empty() || '-' == s[0])
    {
        Context c("When validating package name part '" + s + "':");
        throw PackageNamePartError(s);
    }

    for (std::string::size_type p(0) ; p < s.length() ; ++p)
    {
        if (std::string::npos == allowed_chars.find(s[p]))
        {
            Context c("When validating package name part '" + s + "':");
            throw PackageNamePartError(s);
        }

        if ((p + 1 < s.length()) && (s[p] == '-') &&
                (std::string::npos != number_chars.find(s[p + 1])))
            if (std::string::npos == s.find_first_not_of(number_chars, p + 1))
                throw PackageNamePartError(s);
    }
}

void
CategoryNamePartValidator::validate(const std::string & s)
{
    /* this gets called a lot, make it fast */

    // Allow . because crossdev can create, for example,
    // cross-i686-unknown-freebsd6.0   --spb
    static const std::string allowed_chars(
            "abcdefghijklmnopqrstuvwxyz"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "0123456789-+_.");

    do
    {
        if (s.empty())
            break;

        if ('-' == s[0] || '.' == s[0])
            break;

        if (std::string::npos != s.find_first_not_of(allowed_chars))
            break;

        return;

    } while (false);

    Context c("When validating category name '" + s + "':");
    throw CategoryNamePartError(s);
}

CategoryNamePartError::CategoryNamePartError(const std::string & name) throw () :
    NameError(name, "category name part")
{
}

void
RepositoryNameValidator::validate(const std::string & s)
{
    static const std::string allowed_chars(
            "abcdefghijklmnopqrstuvwxyz"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "0123456789-_");

    do
    {
        if (s.empty())
            break;

        if ('-' == s[0])
            break;

        if (std::string::npos != s.find_first_not_of(allowed_chars))
            break;

        return;

    } while (false);

    Context c("When validating repository name '" + s + "':");
    throw RepositoryNameError(s);
}

RepositoryNameError::RepositoryNameError(const std::string & name) throw () :
    NameError(name, "repository")
{
}

KeywordNameError::KeywordNameError(const std::string & name) throw () :
    NameError(name, "keyword name")
{
}

void
KeywordNameValidator::validate(const std::string & s)
{
    static const std::string allowed_chars(
            "abcdefghijklmnopqrstuvwxyz"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "0123456789-_");

    do
    {
        switch (s.length())
        {
            case 0:
                continue;

            case 1:
                if ("*" == s)
                    return;
                continue;

            case 2:
                if ("-*" == s)
                    return;

                /* fall through */
            default:
                if (std::string::npos != s.find_first_not_of(allowed_chars,
                            ('~' == s[0] ? 1 : 0)))
                    continue;
        }

        return;

    } while (false);

    Context c("When validating keyword name '" + s + "':");
    throw KeywordNameError(s);
}

namespace
{
    CategoryNamePart
    get_category_name_part(const std::string & s)
    {
        Context c("When splitting out category and package names from '" + s + "':");

        std::string::size_type p(s.find('/'));
        if (std::string::npos == p)
            throw QualifiedPackageNameError(s);

        return CategoryNamePart(s.substr(0, p));

    }

    PackageNamePart
    get_package_name_part(const std::string & s)
    {
        Context c("When splitting out category and package names from '" + s + "':");

        std::string::size_type p(s.find('/'));
        if (std::string::npos == p)
            throw QualifiedPackageNameError(s);

        return PackageNamePart(s.substr(p + 1));

    }
}

QualifiedPackageName::QualifiedPackageName(const std::string & s) :
    category(get_category_name_part(s)),
    package(get_package_name_part(s))
{
}

void
SetNameValidator::validate(const std::string & s)
{
    static const std::string allowed_chars(
            "abcdefghijklmnopqrstuvwxyz"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "0123456789-+_");

    do
    {
        if (s.empty())
            break;

        if ('-' == s[0] || '.' == s[0])
            break;

        if (std::string::npos != s.find_first_not_of(allowed_chars))
            break;

        return;

    } while (false);

    Context c("When validating set name '" + s + "':");
    throw SetNameError(s);
}

SetNameError::SetNameError(const std::string & name) throw () :
    NameError(name, "set")
{
}

namespace
{
    UseFlagName get_flag(const std::string & s)
    {
        Context c("When extracting USE flag name from IUSE flag '" + s + "':");
        if (s.empty() || ('-' != s[0] && '+' != s[0]))
            return UseFlagName(s);
        else
            return UseFlagName(s.substr(1));
    }

    UseFlagState get_state(const std::string & s, IUseFlagParseMode m)
    {
        Context c("When extracting USE flag state from IUSE flag '" + s + "':");

        if (s.empty())
            return use_unspecified;
        if ('-' == s[0] || '+' == s[0])
        {
            switch (m)
            {
                case iuse_pm_eapi_0:
                    Log::get_instance()->message(ll_warning, lc_context,
                            "+/- prefixed IUSE flag names not allowed in this EAPI");

                    /* fall through */

                case iuse_pm_permissive:
                    return '-' == s[0] ? use_disabled : use_enabled;

                case iuse_pm_eapi_0_strict:
                    throw IUseFlagNameError(s, "+/- prefixed IUSE flag names not allowed in this EAPI");

                case last_iuse_pm:
                    ;
            }
        }
        return use_unspecified;
    }
}

IUseFlag::IUseFlag(const std::string & s, IUseFlagParseMode m) try:
    flag(get_flag(s)),
    state(get_state(s, m))
{
}
catch (const UseFlagNameError &)
{
    throw IUseFlagNameError(s);
}

IUseFlagNameError::IUseFlagNameError(const std::string & s) throw () :
    NameError(s, "IUse flag")
{
}

IUseFlagNameError::IUseFlagNameError(const std::string & s, const std::string & m) throw () :
    NameError(s, "IUse flag", m)
{
}

std::ostream &
paludis::operator<< (std::ostream & s, const IUseFlag & i)
{
    switch (i.state)
    {
        case use_enabled:
            s << "+";
            break;

        case use_disabled:
            s << "-";
            break;

        case use_unspecified:
        case last_use:
            break;
    }

    s << i.flag;

    return s;
}

