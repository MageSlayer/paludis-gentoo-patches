/* vim: set sw=4 sts=4 et foldmethod=syntax : */

const auto fs_x_of_y_title = make_format_string_fetcher("perform/x_of_y_title", 1)
    << "\\e]2;" << param<'c'>() << param<'x'>() << " " << param<'a'>() << " " << param<'i'>() << "\\a";

