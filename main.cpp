#include <gtk/gtk.h>
#include "viewer.h"

int main(int argc, char *argv[])
{
    gtk_init(&argc, &argv);

    Viewer viewer;
    viewer.show();

    gtk_main();

    return 0;
}
