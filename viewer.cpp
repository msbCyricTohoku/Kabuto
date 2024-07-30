#include "viewer.h"
#include <iostream>
#include <cstdlib>

Viewer::Viewer()
{
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "EPS Viewer");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

    image = gtk_image_new();
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(box), image, TRUE, TRUE, 0);

    GtkWidget *button = gtk_button_new_with_label("Open EPS File");
    gtk_box_pack_start(GTK_BOX(box), button, FALSE, FALSE, 0);

    g_signal_connect(button, "clicked", G_CALLBACK(Viewer::on_open_image), this);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_container_add(GTK_CONTAINER(window), box);
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
    std::string outputPath = filePath + ".png";
    std::string command = "gs -dNOPAUSE -dBATCH -sDEVICE=pngalpha -sOutputFile=" + outputPath + " " + filePath;

    int ret = std::system(command.c_str());
    if (ret != 0)
    {
        std::cerr << "Ghostscript command failed with error code " << ret << std::endl;
        return;
    }

    gtk_image_set_from_file(GTK_IMAGE(image), outputPath.c_str());
    gtk_widget_show(image);
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
