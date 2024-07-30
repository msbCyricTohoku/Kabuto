#ifndef VIEWER_H
#define VIEWER_H

#include <gtk/gtk.h>
#include <string>

class Viewer
{
public:
    Viewer();
    ~Viewer();
    void show();
    void loadImage(const std::string &filePath);

private:
    static void on_open_image(GtkWidget *widget, gpointer data);
    GtkWidget *window;
    GtkWidget *image;
};

#endif // VIEWER_H
