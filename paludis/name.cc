/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008 Ciaran McCreesh
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
#include <paludis/util/sequence-impl.hh>
#include <paludis/util/set-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/wrapped_output_iterator-impl.hh>
#include <paludis/util/options.hh>
#include <paludis/util/hashes.hh>
#include <ostream>
#include <utility>

using namespace paludis;

#include <paludis/name-sr.cc>

template struct Sequence<RepositoryName>;
template struct WrappedForwardIterator<Sequence<RepositoryName>::ConstIteratorTag, const RepositoryName>;

template struct Set<RepositoryName, RepositoryNameComparator>;
template struct WrappedForwardIterator<Set<RepositoryName, RepositoryNameComparator>::ConstIteratorTag, const RepositoryName>;
template struct WrappedOutputIterator<Set<RepositoryName, RepositoryNameComparator>::InserterTag, RepositoryName>;

template struct Set<PackageNamePart>;
template struct WrappedForwardIterator<Set<PackageNamePart>::ConstIteratorTag, const PackageNamePart>;
template struct WrappedOutputIterator<Set<PackageNamePart>::InserterTag, PackageNamePart>;

template struct Set<CategoryNamePart>;
template struct WrappedForwardIterator<Set<CategoryNamePart>::ConstIteratorTag, const CategoryNamePart>;
template struct WrappedOutputIterator<Set<CategoryNamePart>::InserterTag, CategoryNamePart>;

template struct Set<QualifiedPackageName>;
template struct WrappedForwardIterator<Set<QualifiedPackageName>::ConstIteratorTag, const QualifiedPackageName>;
template struct WrappedOutputIterator<Set<QualifiedPackageName>::InserterTag, QualifiedPackageName>;

template struct Set<KeywordName>;
template struct WrappedForwardIterator<Set<KeywordName>::ConstIteratorTag, const KeywordName>;
template struct WrappedOutputIterator<Set<KeywordName>::InserterTag, KeywordName>;

template struct Set<SetName>;
template struct WrappedForwardIterator<Set<SetName>::ConstIteratorTag, const SetName>;
template struct WrappedOutputIterator<Set<SetName>::InserterTag, SetName>;

template struct Set<std::string>;
template struct WrappedForwardIterator<Set<std::string>::ConstIteratorTag, const std::string>;
template struct WrappedOutputIterator<Set<std::string>::InserterTag, std::string>;

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

bool
KeywordNameComparator::operator() (const std::string & a, const std::string & b) const
{
    char a_prefix('~' == a[0] || '-' == a[0] ? a[0] : '\0');
    char b_prefix('~' == b[0] || '-' == b[0] ? b[0] : '\0');
    const std::string a_keyword(a_prefix ? a.substr(1) : a);
    const std::string b_keyword(b_prefix ? b.substr(1) : b);

    if (a_keyword == b_keyword)
    {
        if ('\0' == a_prefix && '\0' != b_prefix)
            return true;
        if ('~' == a_prefix && '-' == b_prefix)
            return true;
        return false;
    }
    else
        return a_keyword < b_keyword;
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

        if (s.length() > 1 && '*' == s[s.length() - 1] && '*' != s[s.length() - 2])
        {
            Context c("When validating set name '" + s + "':");
            validate(s.substr(0, s.length() - 1));
            return;
        }

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

std::size_t
QualifiedPackageName::hash() const
{
    return Hash<std::string>()(stringify(*this));
}

