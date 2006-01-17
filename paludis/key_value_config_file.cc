/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "key_value_config_file.hh"

using namespace paludis;

KeyValueConfigFile::KeyValueConfigFile(std::istream * const s) :
    ConfigFile(s)
{
    need_lines();
}

void
KeyValueConfigFile::accept_line(const std::string & line) const
{
    std::string::size_type p(line.find('='));
    if (std::string::npos == p)
        _entries[line] = "";
    else
    {
        std::string key(line.substr(0, p)), value(line.substr(p + 1));
        normalise_line(key);
        normalise_line(value);
        _entries[key] = value;
    }
}
