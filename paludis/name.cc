/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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
#include <paludis/util/wrapped_value-impl.hh>
#include <ostream>
#include <utility>

using namespace paludis;

namespace paludis
{
    template class WrappedValue<RepositoryNameTag>;
    template class WrappedValue<CategoryNamePartTag>;
    template class WrappedValue<PackageNamePartTag>;
    template class WrappedValue<SlotNameTag>;
    template class WrappedValue<KeywordNameTag>;
    template class WrappedValue<SetNameTag>;

    template class PALUDIS_VISIBLE Sequence<RepositoryName>;
    template class PALUDIS_VISIBLE WrappedForwardIterator<Sequence<RepositoryName>::ConstIteratorTag, const RepositoryName>;

    template class PALUDIS_VISIBLE Set<RepositoryName>;
    template class PALUDIS_VISIBLE WrappedForwardIterator<Set<RepositoryName>::ConstIteratorTag, const RepositoryName>;
    template class PALUDIS_VISIBLE WrappedOutputIterator<Set<RepositoryName>::InserterTag, RepositoryName>;

    template class PALUDIS_VISIBLE Set<PackageNamePart>;
    template class PALUDIS_VISIBLE WrappedForwardIterator<Set<PackageNamePart>::ConstIteratorTag, const PackageNamePart>;
    template class PALUDIS_VISIBLE WrappedOutputIterator<Set<PackageNamePart>::InserterTag, PackageNamePart>;

    template class PALUDIS_VISIBLE Set<CategoryNamePart>;
    template class PALUDIS_VISIBLE WrappedForwardIterator<Set<CategoryNamePart>::ConstIteratorTag, const CategoryNamePart>;
    template class PALUDIS_VISIBLE WrappedOutputIterator<Set<CategoryNamePart>::InserterTag, CategoryNamePart>;

    template class Set<QualifiedPackageName>;
    template class WrappedForwardIterator<Set<QualifiedPackageName>::ConstIteratorTag, const QualifiedPackageName>;
    template class WrappedOutputIterator<Set<QualifiedPackageName>::InserterTag, QualifiedPackageName>;

    template class PALUDIS_VISIBLE Set<KeywordName>;
    template class PALUDIS_VISIBLE WrappedForwardIterator<Set<KeywordName>::ConstIteratorTag, const KeywordName>;
    template class PALUDIS_VISIBLE WrappedOutputIterator<Set<KeywordName>::InserterTag, KeywordName>;

    template class PALUDIS_VISIBLE Set<SetName>;
    template class PALUDIS_VISIBLE WrappedForwardIterator<Set<SetName>::ConstIteratorTag, const SetName>;
    template class PALUDIS_VISIBLE WrappedOutputIterator<Set<SetName>::InserterTag, SetName>;

    template class PALUDIS_VISIBLE WrappedForwardIterator<Set<std::string>::ConstIteratorTag, const std::string>;
    template class PALUDIS_VISIBLE WrappedOutputIterator<Set<std::string>::InserterTag, std::string>;
}

template PALUDIS_VISIBLE std::ostream & paludis::operator<< (std::ostream &, const WrappedValue<RepositoryNameTag> &);
template PALUDIS_VISIBLE std::ostream & paludis::operator<< (std::ostream &, const WrappedValue<CategoryNamePartTag> &);
template PALUDIS_VISIBLE std::ostream & paludis::operator<< (std::ostream &, const WrappedValue<PackageNamePartTag> &);
template PALUDIS_VISIBLE std::ostream & paludis::operator<< (std::ostream &, const WrappedValue<SlotNameTag> &);
template PALUDIS_VISIBLE std::ostream & paludis::operator<< (std::ostream &, const WrappedValue<KeywordNameTag> &);
template PALUDIS_VISIBLE std::ostream & paludis::operator<< (std::ostream &, const WrappedValue<SetNameTag> &);

std::ostream &
paludis::operator<< (std::ostream & s, const QualifiedPackageName & q)
{
    s << q.category() << "/" << q.package();
    return s;
}

SlotNameError::SlotNameError(const std::string & name) throw () :
    NameError(name, "slot name")
{
}

bool
WrappedValueTraits<SlotNameTag>::validate(const std::string & s)
{
    static const std::string allowed_chars(
            "abcdefghijklmnopqrstuvwxyz"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "0123456789-+_.");

    if (s.empty())
        return false;

    if ('-' == s[0] || '.' == s[0])
        return false;

    if (std::string::npos != s.find_first_not_of(allowed_chars))
        return false;

    return true;
}

PackageNamePartError::PackageNamePartError(const std::string & name) throw () :
    NameError(name, "package name part")
{
}

bool
WrappedValueTraits<PackageNamePartTag>::validate(const std::string & s)
{
    static const std::string allowed_chars(
            "abcdefghijklmnopqrstuvwxyz"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "0123456789-+_");

    static const std::string number_chars(
            "0123456789");

    if (s.empty() || '-' == s[0])
        return false;

    for (std::string::size_type p(0) ; p < s.length() ; ++p)
    {
        if (std::string::npos == allowed_chars.find(s[p]))
            return false;

        if ((p + 1 < s.length()) && (s[p] == '-') &&
                (std::string::npos != number_chars.find(s[p + 1])))
            if (std::string::npos == s.find_first_not_of(number_chars, p + 1))
                return false;
    }

    return true;
}

bool
WrappedValueTraits<CategoryNamePartTag>::validate(const std::string & s)
{
    // Allow . because crossdev can create, for example,
    // cross-i686-unknown-freebsd6.0   --spb
    static const std::string allowed_chars(
            "abcdefghijklmnopqrstuvwxyz"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "0123456789-+_.");

    if (s.empty())
        return false;

    if ('-' == s[0] || '.' == s[0])
        return false;

    if (std::string::npos != s.find_first_not_of(allowed_chars))
        return false;

    return true;
}

CategoryNamePartError::CategoryNamePartError(const std::string & name) throw () :
    NameError(name, "category name part")
{
}

bool
WrappedValueTraits<RepositoryNameTag>::validate(const std::string & s)
{
    static const std::string allowed_chars(
            "abcdefghijklmnopqrstuvwxyz"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "0123456789-_");

    if (s.empty())
        return false;

    if ('-' == s[0])
        return false;

    if (std::string::npos != s.find_first_not_of(allowed_chars))
        return false;

    return true;
}

RepositoryNameError::RepositoryNameError(const std::string & name) throw () :
    NameError(name, "repository")
{
}

KeywordNameError::KeywordNameError(const std::string & name) throw () :
    NameError(name, "keyword name")
{
}

bool
WrappedValueTraits<KeywordNameTag>::validate(const std::string & s)
{
    static const std::string allowed_chars(
            "abcdefghijklmnopqrstuvwxyz"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "0123456789-_");

    switch (s.length())
    {
        case 0:
            return false;;

        case 1:
            if ("*" == s)
                return true;
            return false;

        case 2:
            if ("-*" == s)
                return true;

            /* fall through */
        default:
            if (std::string::npos != s.find_first_not_of(allowed_chars,
                        ('~' == s[0] ? 1 : 0)))
                return false;
    }

    return true;
}

namespace
{
    CategoryNamePart
    get_category_name_part(const std::string & s)
    {
        Context c("When splitting out category and package names from '" + s + "':");

        std::string::size_type p(s.find('/'));
        if (std::string::npos == p)
            throw CategoryNamePartError("/" + s);

        return CategoryNamePart(s.substr(0, p));

    }

    PackageNamePart
    get_package_name_part(const std::string & s)
    {
        Context c("When splitting out category and package names from '" + s + "':");

        std::string::size_type p(s.find('/'));
        if (std::string::npos == p)
            throw PackageNamePartError("/" + s);

        return PackageNamePart(s.substr(p + 1));

    }
}

QualifiedPackageName::QualifiedPackageName(const std::string & s) :
    _cat(get_category_name_part(s)),
    _pkg(get_package_name_part(s))
{
}

QualifiedPackageName::QualifiedPackageName(const CategoryNamePart & c, const PackageNamePart & p) :
    _cat(c),
    _pkg(p)
{
}

bool
QualifiedPackageName::operator< (const QualifiedPackageName & other) const
{
    if (category() < other.category())
        return true;
    if (category() > other.category())
        return false;

    return package() < other.package();
}

bool
QualifiedPackageName::operator== (const QualifiedPackageName & other) const
{
    return category() == other.category() && package() == other.package();
}

bool
WrappedValueTraits<SetNameTag>::validate(const std::string & s)
{
    static const std::string allowed_chars(
            "abcdefghijklmnopqrstuvwxyz"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "0123456789-+_:");

    if (s.empty())
        return false;

    if (s.length() > 1 && '*' == s[s.length() - 1] && '*' != s[s.length() - 2])
    {
        Context c("When validating set name '" + s + "':");
        return validate(s.substr(0, s.length() - 1));
    }

    if ('-' == s[0] || '.' == s[0])
        return false;

    if (std::string::npos != s.find_first_not_of(allowed_chars))
        return false;

    std::string::size_type p(s.find(':'));
    if (std::string::npos != p)
    {
        if (++p >= s.length())
            return false;
        if (s[p] != ':')
            return false;

        if (++p >= s.length())
            return false;
        if (std::string::npos != s.find(':', p))
            return false;
    }

    return true;
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

