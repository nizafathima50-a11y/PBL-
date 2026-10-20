#ifndef GUI_H
#define GUI_H

#include <gtk/gtk.h>
#include "scheduler.h"

/*
 * GTK4 activation callback.
 * The GUI will be created when the application starts.
 */
void gui_activate(
    GtkApplication *app,
    gpointer user_data
);

#endif

