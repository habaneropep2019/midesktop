/*
 * MiDesktop Welcome/First run screen
 * (C) Copyright 2025 Alec Bloss.
 * Licensed under the MIT license.
 *
 * SPDX-License-Identifier: MIT
 */

#include <qapplication.h>
#include <qmessagebox.h>

int main ( int argc, char **argv )
{

    QApplication a( argc, argv );

    QMessageBox::information( NULL, "Welcome to MiDesktop!",
                             "Welcome to MiDesktop! MiDesktop is a fork of KDE version 1.\n"
                             "WARNING: This is experimental software. You are likely to run\n"
                             "into potentially serious bugs or security issues.\n"
                             "\n"
                             "Please report bugs at https://centre.libranext.com/libranext/mide/issues\n"
                             "\n"
                             "Have fun!");

}
