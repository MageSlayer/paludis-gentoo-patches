/* vim: set sw=4 sts=4 et foldmethod=syntax : */

const auto fs_pretty_print_enabled = make_format_string_fetcher("pretty-print/enabled", 1)
    << c::green_or_pink() << param<'s'>() << c::normal();

const auto fs_pretty_print_disabled = make_format_string_fetcher("pretty-print/disabled", 1)
    << c::red() << param<'s'>() << c::normal();

const auto fs_pretty_print_installed = make_format_string_fetcher("pretty-print/installed", 1)
    << c::bold_blue_or_pink() << param<'s'>() << c::normal();

const auto fs_pretty_print_installable = make_format_string_fetcher("pretty-print/installable", 1)
    << c::blue_or_pink() << param<'s'>() << c::normal();

const auto fs_pretty_print_masked = make_format_string_fetcher("pretty-print/masked", 1)
    << c::red() << param<'s'>() << c::normal();

const auto fs_pretty_print_plain = make_format_string_fetcher("pretty-print/plain", 1)
    << param<'s'>();

const auto fs_pretty_print_indent = make_format_string_fetcher("pretty-print/indent", 1)
    << "%{column 30}" << param<'i'>() << param<'i'>() << param<'i'>() << param<'i'>();

const auto fs_pretty_print_choice_value_enabled = make_format_string_fetcher("pretty-print-choice-value/enabled", 1)
    << c::green_or_pink() << param<'k'>() << c::normal() << param<'v'>();

const auto fs_pretty_print_choice_value_disabled = make_format_string_fetcher("pretty-print-choice-value/disabled", 1)
    << c::red() << "-" << param<'s'>() << c::normal();

const auto fs_pretty_print_choice_value_forced = make_format_string_fetcher("pretty-print-choice-value/forced", 1)
    << c::green_or_pink() << "(" << param<'k'>() << param<'v'>() << ")" << c::normal();

const auto fs_pretty_print_choice_value_masked = make_format_string_fetcher("pretty-print-choice-value/masked", 1)
    << c::red() << "(-" << param<'s'>() << ")" << c::normal();

