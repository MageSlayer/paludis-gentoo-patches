/* vim: set sw=4 sts=4 et foldmethod=syntax : */

const auto fs_file = make_format_string_fetcher("contents/file", 1)
    << c::bold_blue_or_pink() << param<'s'>() << c::normal();

const auto fs_dir = make_format_string_fetcher("contents/dir", 1)
    << c::blue_or_pink() << param<'s'>() << c::normal();

const auto fs_sym = make_format_string_fetcher("contents/sym", 1)
    << c::bold_green_or_pink() << param<'s'>() << c::normal() << " -> " << param<'t'>();

const auto fs_other = make_format_string_fetcher("contents/other", 1)
    << c::bold_yellow() << param<'s'>() << c::normal();

