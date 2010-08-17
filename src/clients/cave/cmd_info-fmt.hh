/* vim: set sw=4 sts=4 et foldmethod=syntax : */

const auto fs_metadata = make_format_string_fetcher("info/metadata", 1)
    << param<'i'>() << param<'i'>() << param<'i'>() << param<'i'>() << param<'h'>() << "%{column 30}" << param<'s'>() << "\\n";

const auto fs_metadata_subsection = make_format_string_fetcher("info/metadata_subsection", 1)
    << param<'i'>() << param<'i'>() << param<'i'>() << param<'i'>() << param<'s'>() << "\\n";

const auto fs_repository_heading = make_format_string_fetcher("info/repository_heading", 1)
    << "Repository " << c::blue_or_pink() << param<'s'>() << c::normal() << ":\\n";

const auto fs_id_heading = make_format_string_fetcher("info/id_heading", 1)
    << "Extra Information for " << c::blue_or_pink() << param<'s'>() << c::normal() << ":\\n";

const auto fs_heading = make_format_string_fetcher("info/heading", 1)
    << c::blue_or_pink() << param<'s'>() << c::normal() << ":\\n";

