/* vim: set sw=4 sts=4 et foldmethod=syntax : */

const auto fs_fixing = make_format_string_fetcher("fix-cache/fixing", 1)
    << "Fixing cache for " << c::bold_blue_or_pink() << param<'s'>() << c::normal() << "...\\n";

