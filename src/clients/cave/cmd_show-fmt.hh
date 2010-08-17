/* vim: set sw=4 sts=4 et foldmethod=syntax : */

const auto fs_repository_heading = make_format_string_fetcher("show/repository_heading", 1)
    << "* " << c::bold_blue_or_pink() << param<'s'>() << c::normal() << "\\n";

