#include "viewer.h"
#include <iostream>
#include <cstdlib>
#include <cmath>

Viewer::Viewer() : rotationAngle(0)
{
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "EPS Viewer");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

    image = gtk_image_new();
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(box), image, TRUE, TRUE, 0);

    GtkWidget *buttonBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(box), buttonBox, FALSE, FALSE, 0);

    GtkWidget *openButton = gtk_button_new_with_label("Open EPS File");
    gtk_box_pack_start(GTK_BOX(buttonBox), openButton, FALSE, FALSE, 0);

    GtkWidget *colorButton = gtk_button_new_with_label("Change Background Color");
    gtk_box_pack_start(GTK_BOX(buttonBox), colorButton, FALSE, FALSE, 0);

    GtkWidget *rotateLeftButton = gtk_button_new_with_label("Rotate Left");
    gtk_box_pack_start(GTK_BOX(buttonBox), rotateLeftButton, FALSE, FALSE, 0);

    GtkWidget *rotateRightButton = gtk_button_new_with_label("Rotate Right");
    gtk_box_pack_start(GTK_BOX(buttonBox), rotateRightButton, FALSE, FALSE, 0);

    GtkWidget *saveButton = gtk_button_new_with_label("Save as PNG");
    gtk_box_pack_start(GTK_BOX(buttonBox), saveButton, FALSE, FALSE, 0);

    GtkWidget *quitButton = gtk_button_new_with_label("Quit");
    gtk_box_pack_start(GTK_BOX(buttonBox), quitButton, FALSE, FALSE, 0);

    g_signal_connect(openButton, "clicked", G_CALLBACK(Viewer::on_open_image), this);
    g_signal_connect(colorButton, "clicked", G_CALLBACK(Viewer::on_change_bg_color), this);
    g_signal_connect(rotateLeftButton, "clicked", G_CALLBACK(Viewer::on_rotate_left), this);
    g_signal_connect(rotateRightButton, "clicked", G_CALLBACK(Viewer::on_rotate_right), this);
    g_signal_connect(saveButton, "clicked", G_CALLBACK(Viewer::on_save_image), this);
    g_signal_connect(quitButton, "clicked", G_CALLBACK(Viewer::on_quit), this);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_container_add(GTK_CONTAINER(window), box);

    bgColor.red = 1.0;
    bgColor.green = 1.0;
    bgColor.blue = 1.0;
    bgColor.alpha = 1.0;
}

Viewer::~Viewer()
{
}

void Viewer::show()
{
    gtk_widget_show_all(window);
}

void Viewer::loadImage(const std::string &filePath)
{
    currentFilePath = filePath;
    std::string outputPath = filePath + ".png";
    std::string command = "gs -dNOPAUSE -dBATCH -sDEVICE=pngalpha -dEPSCrop -sOutputFile=\"" + outputPath + "\" \"" + filePath + "\"";

    int ret = std::system(command.c_str());
    if (ret != 0)
    {
        std::cerr << "Ghostscript command failed with error code " << ret << std::endl;
        return;
    }

    gtk_image_set_from_file(GTK_IMAGE(image), outputPath.c_str());
    gtk_widget_show(image);
}

void Viewer::setBackgroundColor(GdkRGBA color)
{
    bgColor = color;
    gtk_widget_override_background_color(window, GTK_STATE_FLAG_NORMAL, &bgColor);
}

void Viewer::rotateImage(int angle)
{
    rotationAngle = (rotationAngle + angle) % 360;
    if (rotationAngle < 0)
    {
        rotationAngle += 360;
    }

    std::string outputPath = currentFilePath + ".png";
    GError *error = NULL;
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(outputPath.c_str(), &error);
    if (error)
    {
        std::cerr << "Failed to load image: " << error->message << std::endl;
        g_error_free(error);
        return;
    }

    GdkPixbuf *rotatedPixbuf = NULL;
    switch (rotationAngle)
    {
    case 90:
        rotatedPixbuf = gdk_pixbuf_rotate_simple(pixbuf, GDK_PIXBUF_ROTATE_CLOCKWISE);
        break;
    case 180:
        rotatedPixbuf = gdk_pixbuf_rotate_simple(pixbuf, GDK_PIXBUF_ROTATE_UPSIDEDOWN);
        break;
    case 270:
        rotatedPixbuf = gdk_pixbuf_rotate_simple(pixbuf, GDK_PIXBUF_ROTATE_COUNTERCLOCKWISE);
        break;
    default:
        rotatedPixbuf = gdk_pixbuf_copy(pixbuf); // No rotation
        break;
    }

    if (!rotatedPixbuf)
    {
        std::cerr << "Failed to rotate image." << std::endl;
        g_object_unref(pixbuf);
        return;
    }

    gtk_image_set_from_pixbuf(GTK_IMAGE(image), rotatedPixbuf);
    g_object_unref(rotatedPixbuf);
    g_object_unref(pixbuf);
}

void Viewer::saveImage()
{
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Save Image",
                                                    GTK_WINDOW(window),
                                                    GTK_FILE_CHOOSER_ACTION_SAVE,
                                                    "_Cancel", GTK_RESPONSE_CANCEL,
                                                    "_Save", GTK_RESPONSE_ACCEPT,
                                                    NULL);

    gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
    {
        char *filePath = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        std::string outputPath(filePath);
        g_free(filePath);

        std::string inputPath = currentFilePath + ".png";
        GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(inputPath.c_str(), NULL);
        if (pixbuf)
        {
            gdk_pixbuf_save(pixbuf, outputPath.c_str(), "png", NULL, NULL);
            g_object_unref(pixbuf);
        }
        else
        {
            std::cerr << "Failed to load image for saving." << std::endl;
        }
    }

    gtk_widget_destroy(dialog);
}

void Viewer::quit()
{
    gtk_main_quit();
}

void Viewer::on_open_image(GtkWidget *widget, gpointer data)
{
    Viewer *viewer = static_cast<Viewer *>(data);

    GtkWidget *dialog = gtk_file_chooser_dialog_new("Open EPS File",
                                                    GTK_WINDOW(viewer->window),
                                                    GTK_FILE_CHOOSER_ACTION_OPEN,
                                                    "_Cancel", GTK_RESPONSE_CANCEL,
                                                    "_Open", GTK_RESPONSE_ACCEPT,
                                                    NULL);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
    {
        char *filePath = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        viewer->loadImage(filePath);
        g_free(filePath);
    }

    gtk_widget_destroy(dialog);
}

void Viewer::on_change_bg_color(GtkWidget *widget, gpointer data)
{
    Viewer *viewer = static_cast<Viewer *>(data);

    GtkWidget *dialog = gtk_color_chooser_dialog_new("Select Background Color", GTK_WINDOW(viewer->window));

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK)
    {
        gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(dialog), &viewer->bgColor);
        viewer->setBackgroundColor(viewer->bgColor);
    }

    gtk_widget_destroy(dialog);
}

void Viewer::on_rotate_left(GtkWidget *widget, gpointer data)
{
    Viewer *viewer = static_cast<Viewer *>(data);
    viewer->rotateImage(-90);
}

void Viewer::on_rotate_right(GtkWidget *widget, gpointer data)
{
    Viewer *viewer = static_cast<Viewer *>(data);
    viewer->rotateImage(90);
}

void Viewer::on_save_image(GtkWidget *widget, gpointer data)
{
    Viewer *viewer = static_cast<Viewer *>(data);
    viewer->saveImage();
}

void Viewer::on_quit(GtkWidget *widget, gpointer data)
{
    Viewer *viewer = static_cast<Viewer *>(data);
    viewer->quit();
}
