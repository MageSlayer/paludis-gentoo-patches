/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "is_file_with_extension.hh"

using namespace paludis;

bool
IsFileWithExtension::operator() (const FSEntry & f) const
{
    const std::string filename(f.basename());

    if (filename.length() < _ext.length() + _prefix.length())
        return false;
    if (0 != filename.compare(filename.length() - _ext.length(),
                _ext.length(), _ext))
        return false;
    if (0 != filename.compare(0, _prefix.length(), _prefix))
        return false;
    return f.is_regular_file();
}
