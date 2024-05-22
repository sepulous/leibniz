#ifndef WINDOW_TITLE_H
#define WINDOW_TITLE_H

#include "version.h"
#include <string>

std::string getWindowTitle()
{
    std::string title;

    title += "Leibniz ";
    if (LEIBNIZ_BUILD == ALPHA)
        title += u8"α-";
    else if (LEIBNIZ_BUILD == BETA)
        title += u8"β-";
    else
        title += "v";
    title += LEIBNIZ_VERSION;

    return title;
}

#endif
