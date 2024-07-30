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
    void setBackgroundColor(GdkRGBA color);
    void rotateImage(int angle);
    void saveImage();
    void quit();

private:
    static void on_open_image(GtkWidget *widget, gpointer data);
    static void on_change_bg_color(GtkWidget *widget, gpointer data);
    static void on_rotate_left(GtkWidget *widget, gpointer data);
    static void on_rotate_right(GtkWidget *widget, gpointer data);
    static void on_save_image(GtkWidget *widget, gpointer data);
    static void on_quit(GtkWidget *widget, gpointer data);

    cairo_surface_t* createSurfaceWithBackground(GdkPixbuf *pixbuf, int width, int height);

    GtkWidget *window;
    GtkWidget *image;
    GdkRGBA bgColor;
    int rotationAngle;
    std::string currentFilePath;
    GdkPixbuf *pixbuf; // To store the loaded image
};

#endif // VIEWER_H
