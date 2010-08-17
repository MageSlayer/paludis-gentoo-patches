/* vim: set sw=4 sts=4 et foldmethod=syntax : */

const auto fs_file = make_format_string_fetcher("executables/file", 1)
    << c::bold_blue_or_pink() << param<'s'>() << c::normal() << "\\n";

