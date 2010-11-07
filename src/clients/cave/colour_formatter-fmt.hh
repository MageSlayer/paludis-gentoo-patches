/* vim: set sw=4 sts=4 et foldmethod=syntax : */

const auto fs_format_keyword_name_plain = make_format_string_fetcher("format-keyword-name/plain", 1)
    << param<'s'>();

const auto fs_format_keyword_name_accepted = make_format_string_fetcher("format-keyword-name/accepted", 1)
    << c::green_or_pink() << param<'s'>() << c::normal();

const auto fs_format_keyword_name_unaccepted = make_format_string_fetcher("format-keyword-name/unaccepted", 1)
    << c::red() << param<'s'>() << c::normal();

const auto fs_format_string_plain = make_format_string_fetcher("format-string/plain", 1)
    << param<'s'>();

const auto fs_format_string_string_plain = make_format_string_fetcher("format-string-string/plain", 1)
    << param<'k'>() << "=" << param<'v'>();

const auto fs_format_package_id_plain = make_format_string_fetcher("format-package-id/plain", 1)
    << param<'s'>();

const auto fs_format_package_id_installed = make_format_string_fetcher("format-package-id/installed", 1)
    << c::bold_green_or_pink() << param<'s'>() << c::normal();

const auto fs_format_package_id_installable = make_format_string_fetcher("format-package-id/installable", 1)
    << c::green_or_pink() << param<'s'>() << c::normal();

const auto fs_format_license_dep_spec_plain = make_format_string_fetcher("format-license-dep-spec/plain", 1)
    << param<'s'>();

const auto fs_format_license_dep_spec_accepted = make_format_string_fetcher("format-license-dep-spec/accepted", 1)
    << c::green_or_pink() << param<'s'>() << c::normal();

const auto fs_format_license_dep_spec_unaccepted = make_format_string_fetcher("format-license-dep-spec/unaccepted", 1)
    << c::red() << param<'s'>() << c::normal();

const auto fs_format_choice_value_plain = make_format_string_fetcher("format-choice-value/plain", 1)
    << param<'s'>();

const auto fs_format_choice_value_enabled = make_format_string_fetcher("format-choice-value/enabled", 1)
    << c::green_or_pink() << param<'k'>() << c::normal() << param<'v'>();

const auto fs_format_choice_value_disabled = make_format_string_fetcher("format-choice-value/disabled", 1)
    << c::red() << "-" << param<'s'>() << c::normal();

const auto fs_format_choice_value_forced = make_format_string_fetcher("format-choice-value/forced", 1)
    << c::green_or_pink() << "(" << param<'k'>() << param<'v'>() << ")" << c::normal();

const auto fs_format_choice_value_masked = make_format_string_fetcher("format-choice-value/masked", 1)
    << c::red() << "(-" << param<'s'>() << ")" << c::normal();

const auto fs_format_conditional_dep_spec_plain = make_format_string_fetcher("format-conditional-dep-spec/plain", 1)
    << param<'s'>();

const auto fs_format_conditional_dep_spec_enabled = make_format_string_fetcher("format-conditional-dep-spec/enabled", 1)
    << c::green_or_pink() << param<'s'>() << c::normal();

const auto fs_format_conditional_dep_spec_disabled = make_format_string_fetcher("format-conditional-dep-spec/disabled", 1)
    << c::red() << param<'s'>() << c::normal();

const auto fs_format_conditional_dep_spec_forced = make_format_string_fetcher("format-conditional-dep-spec/forced", 1)
    << c::green_or_pink() << "(" << param<'s'>() << ")" << c::normal();

const auto fs_format_conditional_dep_spec_masked = make_format_string_fetcher("format-conditional-dep-spec/masked", 1)
    << c::red() << "(" << param<'s'>() << ")" << c::normal();

const auto fs_format_plain_text_dep_spec_plain = make_format_string_fetcher("format-plain-text-dep-spec/plain", 1)
    << param<'s'>();

const auto fs_format_simple_uri_dep_spec_plain = make_format_string_fetcher("format-simple-uri-dep-spec/plain", 1)
    << param<'s'>();

const auto fs_format_fetchable_uri_dep_spec_plain = make_format_string_fetcher("format-fetchable-uri-dep-spec/plain", 1)
    << param<'s'>();

const auto fs_format_uri_labels_dep_spec_plain = make_format_string_fetcher("format-uri-labels-dep-spec/plain", 1)
    << param<'s'>();

const auto fs_format_package_dep_spec_plain = make_format_string_fetcher("format-package-dep-spec/plain", 1)
    << param<'s'>();

const auto fs_format_package_dep_spec_installed = make_format_string_fetcher("format-package-dep-spec/installed", 1)
    << c::bold_green_or_pink() << param<'s'>() << c::normal();

const auto fs_format_package_dep_spec_installable = make_format_string_fetcher("format-package-dep-spec/installable", 1)
    << c::green_or_pink() << param<'s'>() << c::normal();

const auto fs_format_dependency_labels_dep_spec_plain = make_format_string_fetcher("format-dependency-labels-dep-spec/plain", 1)
    << param<'s'>();

const auto fs_format_block_dep_spec_plain = make_format_string_fetcher("format-block-dep-spec/plain", 1)
    << param<'s'>();

const auto fs_format_named_set_dep_spec_plain = make_format_string_fetcher("format-named-set-dep-spec/plain", 1)
    << c::blue_or_pink() << param<'s'>() << c::normal();

const auto fs_format_fsentry_plain = make_format_string_fetcher("format-fsentry/plain", 1)
    << param<'s'>();

const auto fs_format_indent = make_format_string_fetcher("format/indent", 1)
    << "%{column 30}" << param<'i'>() << param<'i'>() << param<'i'>() << param<'i'>();

