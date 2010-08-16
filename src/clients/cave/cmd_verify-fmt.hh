/* vim: set sw=4 sts=4 et foldmethod=syntax : */

const auto fs_package = make_format_string_fetcher("verify/package", 1)
    << c::bold_blue_or_pink() << param<'s'>() << c::normal() << "\\n";

const auto fs_error = make_format_string_fetcher("verify/error", 1)
    << c::bold_red() << "    " << param<'p'>() << c::normal() << "%{column 32}" << param<'t'>() << "\\n";

