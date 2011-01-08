/* vim: set sw=4 sts=4 et foldmethod=syntax : */

const auto fs_package_id = make_format_string_fetcher("report/package_id", 1)
    << c::bold_blue() << param<'s'>() << c::normal() << "\\n";

const auto fs_package_no_origin = make_format_string_fetcher("report/no_origin", 1)
    << "    No longer exists in original repository " << param<'s'>() << "\\n";

const auto fs_package_origin = make_format_string_fetcher("report/origin", 1)
    << "    Origin " << param<'s'>() << ":\\n";

const auto fs_package_origin_masked = make_format_string_fetcher("report/origin_masked", 1)
    << "        Masked in original repository\\n";

const auto fs_package_insecure = make_format_string_fetcher("report/insecure", 1)
    << "    Marked as insecure\\n";

const auto fs_package_unused = make_format_string_fetcher("report/unused", 1)
    << "    Not used by any package in world\\n";

