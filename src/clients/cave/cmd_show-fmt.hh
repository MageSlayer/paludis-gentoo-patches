/* vim: set sw=4 sts=4 et foldmethod=syntax : */

const auto fs_repository_heading = make_format_string_fetcher("show/repository_heading", 1)
    << "* " << c::bold_blue_or_pink() << param<'s'>() << c::normal() << "\\n";

const auto fs_wildcard_heading = make_format_string_fetcher("show/wildcard_heading", 1)
    << "* " << c::bold_blue_or_pink() << param<'s'>() << c::normal() << "\\n";

const auto fs_wildcard_spec_installed = make_format_string_fetcher("show/wildcard_spec_installed", 1)
    << "    " << c::green_or_pink() << param<'s'>() << c::normal() << "\\n";

const auto fs_wildcard_spec_installable = make_format_string_fetcher("show/wildcard_spec_installable", 1)
    << "    " << param<'s'>() << c::normal() << "\\n";

const auto fs_wildcard_spec_unavailable = make_format_string_fetcher("show/wildcard_spec_unavailable", 1)
    << "    " << c::red() << param<'s'>() << c::normal() << "\\n";

const auto fs_set_heading = make_format_string_fetcher("show/set_heading", 1)
    << "* " << c::bold_blue_or_pink() << param<'s'>() << c::normal() << "\\n";

const auto fs_set_set = make_format_string_fetcher("show/set_set", 1)
    << param<'i'>() << param<'i'>() << param<'i'>() << param<'i'>() << c::blue_or_pink() << param<'s'>() << c::normal() << "\\n";

const auto fs_set_spec_installed = make_format_string_fetcher("show/set_spec_installed", 1)
    << param<'i'>() << param<'i'>() << param<'i'>() << param<'i'>() << c::bold_green_or_pink() << param<'s'>() << c::normal() << "\\n";

const auto fs_set_spec_installable = make_format_string_fetcher("show/set_spec_installable", 1)
    << param<'i'>() << param<'i'>() << param<'i'>() << param<'i'>() << c::green_or_pink() << param<'s'>() << c::normal() << "\\n";

const auto fs_set_spec_unavailable = make_format_string_fetcher("show/set_spec_unavailable", 1)
    << param<'i'>() << param<'i'>() << param<'i'>() << param<'i'>() << c::bold_red() << param<'s'>() << c::normal() << "\\n";

