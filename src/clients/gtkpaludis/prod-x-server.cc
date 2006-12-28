/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <cstdlib>
#include <iostream>

int main(int, char *[])
{
    Display * display(XOpenDisplay(0));
    if (display)
    {
        std::cerr << "X server is available" << std::endl;
        XCloseDisplay(display);
        return EXIT_SUCCESS;
    }
    else
    {
        std::cerr << "X server is not available" << std::endl;
        return EXIT_FAILURE;
    }
}


