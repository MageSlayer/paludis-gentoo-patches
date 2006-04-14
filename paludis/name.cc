/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

using namespace paludis;

QualifiedPackageNameError::QualifiedPackageNameError(const std::string & s) throw () :
    NameError(s, "qualified package name")
{
}

std::ostream &
paludis::operator<< (std::ostream & s, const QualifiedPackageName & q)
{
    s << q.get<qpn_category>() << "/" << q.get<qpn_package>();
    return s;
}

MakeSmartRecord<QualifiedPackageNameTag>::Type
QualifiedPackageName::_make_parent(
        const std::string & s)
{
    Context c("When splitting out category and package names from '" + s + "':");

    std::string::size_type p(s.find('/'));
    if (std::string::npos == p)
        throw QualifiedPackageNameError(s);

    return MakeSmartRecord<QualifiedPackageNameTag>::Type(
            CategoryNamePart(s.substr(0, p)),
            PackageNamePart(s.substr(p + 1)));
}

void
UseFlagNameValidator::validate(const std::string & s)
{
    static const std::string allowed_chars(
            "abcdefghijklmnopqrstuvwxyz"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "0123456789-+_:@");

    do
    {
        if (s.empty())
            break;

        if ('-' == s.at(0))
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

        if ('-' == s.at(0))
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

    if (s.empty() || '-' == s.at(0))
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

    static const std::string allowed_chars(
            "abcdefghijklmnopqrstuvwxyz"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "0123456789-+_");

    do
    {
        if (s.empty())
            break;

        if ('-' == s.at(0))
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
            "0123456789-+_");

    do
    {
        if (s.empty())
            break;

        if ('-' == s.at(0))
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
            "0123456789-+_");

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

                /* fall throuth */
            default:
                if (std::string::npos != s.find_first_not_of(allowed_chars,
                            ('~' == s.at(0) ? 1 : 0)))
                    continue;
        }

        return;

    } while (false);

    Context c("When validating keyword name '" + s + "':");
    throw KeywordNameError(s);
}

