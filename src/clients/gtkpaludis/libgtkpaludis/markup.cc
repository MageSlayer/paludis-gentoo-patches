/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "markup.hh"

using namespace gtkpaludis;

std::string
gtkpaludis::markup_escape(const std::string & s)
{
    std::string result;
    for (std::string::size_type p(0) ; p < s.length() ; ++p)
    {
        switch (s[p])
        {
            case '<':
                result.append("&lt;");
                break;
            case '>':
                result.append("&gt;");
                break;
            case '&':
                result.append("&amp;");
                break;
            default:
                result.append(1, s[p]);
                break;
        }
    }
    return result;
}

std::string
gtkpaludis::markup_bold(const std::string & s)
{
    return "<span weight=\"bold\">" + s + "</span>";
}

std::string
gtkpaludis::markup_italic(const std::string & s)
{
    return "<span style=\"italic\">" + s + "</span>";
}

std::string
gtkpaludis::markup_foreground(const std::string & c, const std::string & s)
{
    return "<span foreground=\"" + c + "\">" + s + "</span>";
}

