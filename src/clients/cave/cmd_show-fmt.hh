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

const auto fs_contents_file = make_format_string_fetcher("show/contents_file", 1)
    << param_if<'b'>() << "%{column 30}" << param_endif<'b'>() << param<'r'>() << param_if<'b'>() << "\\n" << param_else<'b'>() << " " << param_endif<'b'>();

const auto fs_contents_other = make_format_string_fetcher("show/contents_other", 1)
    << param_if<'b'>() << "%{column 30}" << param_endif<'b'>() << param<'r'>() << param_if<'b'>() << "\\n" << param_else<'b'>() << " " << param_endif<'b'>();

const auto fs_contents_dir = make_format_string_fetcher("show/contents_dir", 1)
    << param_if<'b'>() << "%{column 30}" << param_endif<'b'>() << param<'r'>() << param_if<'b'>() << "\\n" << param_else<'b'>() << " " << param_endif<'b'>();

const auto fs_contents_sym = make_format_string_fetcher("show/contents_sym", 1)
    << param_if<'b'>() << "%{column 30}" << param_endif<'b'>() << param<'r'>() << " -> " << param<'v'>()
    << param_if<'b'>() << "\\n" << param_else<'b'>() << " " << param_endif<'b'>();

const auto fs_choice_forced_enabled = make_format_string_fetcher("show/choice_forced_enabled", 1)
    << c::green_or_pink() << "(" << param<'s'>() << ")" << c::normal() << param<'r'>();

const auto fs_choice_enabled = make_format_string_fetcher("show/choice_enabled", 1)
    << c::green_or_pink() << param<'s'>() << c::normal() << param<'r'>();

const auto fs_choice_forced_disabled = make_format_string_fetcher("show/choice_forced_disabled", 1)
    << c::red() << "(-" << param<'s'>() << ")" << c::normal() << param<'r'>();

const auto fs_choice_disabled = make_format_string_fetcher("show/choice_disabled", 1)
    << c::red() << "-" << param<'s'>() << c::normal() << param<'r'>();

const auto fs_choice_parameter = make_format_string_fetcher("show/choice_parameter", 1)
    << "=" << param<'v'>();

const auto fs_permitted_choice_value_int = make_format_string_fetcher("show/permitted_choice_value_int", 1)
    << "%{column 30}Should be an integer" << param_if<'r'>() << " " << param<'r'>() << param_endif<'r'>() << "\\n";

const auto fs_permitted_choice_value_enum_values = make_format_string_fetcher("show/permitted_choice_value_enum_values", 1)
    << "%{column 30}Permitted values:" << "\\n";

const auto fs_permitted_choice_value_enum_value = make_format_string_fetcher("show/permitted_choice_value_enum_value", 1)
    << "%{column 34}" << param<'v'>() << param_if<'d'>() << ": " << "%{column 45}" << param<'d'>() << param_endif<'d'>() << "\\n";

const auto fs_metadata_value_raw = make_format_string_fetcher("show/metadata_value_raw", 2)
    << "    " << param<'i'>() << param<'i'>() << param<'i'>() << param<'i'>()
    << param_if<'b'>() << c::bold_normal() << param_endif<'b'>() << param<'s'>() << c::normal()
    << param_if<'p'>() << "=" << param<'p'>() << param_endif<'p'>()
    << "%{column 30}" << param<'v'>() << "\\n";

const auto fs_metadata_value_human = make_format_string_fetcher("show/metadata_value_human", 2)
    << "    " << param<'i'>() << param<'i'>() << param<'i'>() << param<'i'>()
    << param_if<'b'>() << c::bold_normal() << param_endif<'b'>() << param<'s'>() << c::normal()
    << param_if<'p'>() << "=" << param<'p'>() << param_endif<'p'>()
    << "%{column 30}" << param<'v'>() << "\\n";

const auto fs_metadata_continued_value = make_format_string_fetcher("show/metadata_continued_value", 1)
    << "    " << "%{column 30}" << param<'i'>() << param<'i'>() << param<'v'>() << "\\n";

const auto fs_metadata_subsection_human = make_format_string_fetcher("show/metadata_subsection_human", 1)
    << "    " << param<'i'>() << param<'i'>() << param<'i'>() << param<'i'>()
    << c::bold_blue_or_pink() << param<'s'>() << c::normal() << "\\n";

const auto fs_metadata_subsection_raw = make_format_string_fetcher("show/metadata_subsection_raw", 1)
    << "    " << param<'i'>() << param<'i'>() << param<'i'>() << param<'i'>()
    << c::bold_blue_or_pink() << param<'s'>() << c::normal() << "\\n";

const auto fs_package_id_heading = make_format_string_fetcher("show/package_id_heading", 1)
    << "    " << c::bold_blue_or_pink() << param<'s'>() << c::normal() << "\\n";

const auto fs_package_id_masks = make_format_string_fetcher("show/package_id_masks", 1)
    << "        " << c::bold_red() << param<'s'>() << c::normal() << "\\n";

const auto fs_package_id_masks_overridden = make_format_string_fetcher("show/package_id_masks_overridden", 1)
    << "        " << c::green_or_pink() << param<'s'>() << c::normal() << "\\n";

const auto fs_package_heading = make_format_string_fetcher("show/package_heading", 1)
    << "* " << c::bold_blue_or_pink() << param<'s'>() << c::normal() << "\\n";

const auto fs_package_version_installed = make_format_string_fetcher("show/package_version_installed", 1)
    << c::bold_green_or_pink() << param<'s'>() << c::normal();

const auto fs_package_version_installable = make_format_string_fetcher("show/package_version_installable", 2)
    << c::green_or_pink() << param<'s'>() << param<'r'>() << c::normal();

const auto fs_package_version_unavailable = make_format_string_fetcher("show/package_version_unavailable", 1)
    << c::red() << "(" << param<'s'>() << ")" << param<'r'>() << c::normal();

const auto fs_package_repository = make_format_string_fetcher("show/package_repository", 1)
    << "    ::" << param<'s'>() << "%{column 30}";

const auto fs_package_best = make_format_string_fetcher("show/package_best", 1)
    << "*";

const auto fs_package_slot = make_format_string_fetcher("show/package_slot", 1)
    << " {:" << param<'s'>() << "}";

const auto fs_package_no_slot = make_format_string_fetcher("show/package_no_slot", 1)
    << " {no slot}";

