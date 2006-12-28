dnl $Id: convert.m4,v 1.1.1.1 2003/01/21 13:41:24 murrayc Exp $

# Other libraries, such as libgnomeuimm, can provide their own convert.m4 files,
# Maybe choosing to include the same files as this one.

include(convert_gtkmm.m4)

_CONV_ENUM(Vte,TerminalEraseBinding)
_CONV_ENUM(Vte,TerminalAntiAlias)

_EQUAL(glong, long)
_EQUAL(guint, unsigned)

_CONVERSION(int&, int*, &($3))
_CONVERSION(long&, glong*, &($3))
_CONVERSION(return-char*, char*, ($3))

# Gtk conversions
_CONVERSION(const Gtk::MenuShell&, GtkMenuShell*, (const_cast<Gtk::MenuShell&>($3)).gobj())

# Gdk conversions
_CONVERSION(const Gdk::Color&, const GdkColor*, ($3).gobj())
_CONVERSION(Gdk::Color&, GdkColor*, ($3).gobj())
_CONVERSION(Gdk::Cursor&, GdkCursor*, ($3).gobj())

# Pango conversions
_CONVERSION(const Pango::FontDescription&, const PangoFontDescription*, ($3).gobj())
_CONVERSION(const PangoFontDescription*, const Pango::FontDescription, Glib::wrap(const_cast<PangoFontDescription*>(($3)), true))


