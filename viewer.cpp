//Kabuto EPS image viewer and converter to png 
//Feel free to change your image background and enjoy simple and free and open-source program
#include "viewer.h"
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <cstdio>

Viewer::Viewer() : rotationAngle(0), pixbuf(nullptr)
{
    GError *error = NULL;
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Kabuto EPS Viewer/Converter");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

    //gtk_window_set_icon(GTK_WINDOW(window), create_pixbuf("icon/icon.png"));
    //gtk_window_set_icon_from_file(GTK_WINDOW(window), "icon/icon.png", NULL);

    //here the Kabuto icon is added, it also checks if the icon is missing
    if (!gtk_window_set_icon_from_file(GTK_WINDOW(window), "icon/icon.png", &error)) {
        g_printerr("Error loading icon file: %s\n", error->message);
        g_clear_error(&error);
    }


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
    if (pixbuf)
    {
        g_object_unref(pixbuf);
    }
}

void Viewer::show()
{
    gtk_widget_show_all(window);
}

void Viewer::loadImage(const std::string &filePath)
{
    currentFilePath = filePath;

    //create a temporary file for the PNG output -- this will be removed
    char tempPath[] = "/tmp/epsviewer_XXXXXX.png";
    int fd = mkstemps(tempPath, 4); //4 is the length of ".png"
    if (fd == -1)
    {
        std::cerr << "Failed to create temporary file for PNG output." << std::endl;
        return;
    }
    close(fd); //close the file descriptor as we don't need it

    std::string command = "gs -dNOPAUSE -dBATCH -sDEVICE=pngalpha -dEPSCrop -sOutputFile=\"" + std::string(tempPath) + "\" \"" + filePath + "\"";

    int ret = std::system(command.c_str());
    if (ret != 0)
    {
        std::cerr << "Ghostscript command failed with error code 555" << ret << std::endl; //note error code 555 refers to missing ghostscript on user PC
        std::remove(tempPath); //remove the temporary file
        return;
    }

    GError *error = NULL;
    pixbuf = gdk_pixbuf_new_from_file(tempPath, &error);
    if (error)
    {
        std::cerr << "Failed to load image: " << error->message << std::endl;
        g_error_free(error);
        std::remove(tempPath); //remove the temporary file important
        return;
    }

    gtk_image_set_from_pixbuf(GTK_IMAGE(image), pixbuf);
    gtk_widget_show(image);

    std::remove(tempPath); //remove the temporary file important to avoid flooding the dir
}

//void Viewer::setBackgroundColor(GdkRGBA color)
//{
//    bgColor = color;
//    gtk_widget_override_background_color(window, GTK_STATE_FLAG_NORMAL, &bgColor);
//}

//gtk_widget_override_background_color is deprecated so now we need to use css style
void Viewer::setBackgroundColor(GdkRGBA color)
{
    bgColor = color;

    //convert GdkRGBA color to CSS string
    char css[256];
    snprintf(css, sizeof(css),
             "window { background-color: rgba(%d, %d, %d, %f); }",
             static_cast<int>(color.red * 255),
             static_cast<int>(color.green * 255),
             static_cast<int>(color.blue * 255),
             color.alpha);

    //here we create a new CSS provider
    GtkCssProvider* cssProvider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(cssProvider, css, -1, NULL);

    //here gets the screen and apply the CSS provider
    GdkScreen* screen = gdk_screen_get_default();
    gtk_style_context_add_provider_for_screen(screen, GTK_STYLE_PROVIDER(cssProvider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    //clean the object
    g_object_unref(cssProvider);
}


void Viewer::rotateImage(int angle)
{
    rotationAngle = (rotationAngle + angle) % 360;
    if (rotationAngle < 0)
    {
        rotationAngle += 360;
    }

    if (!pixbuf)
    {
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
        rotatedPixbuf = gdk_pixbuf_copy(pixbuf); //no rotation
        break;
    }

    if (!rotatedPixbuf)
    {
        std::cerr << "Failed to rotate image." << std::endl;
        return;
    }

    gtk_image_set_from_pixbuf(GTK_IMAGE(image), rotatedPixbuf);
    g_object_unref(rotatedPixbuf);
}

cairo_surface_t* Viewer::createSurfaceWithBackground(GdkPixbuf *pixbuf, int width, int height)
{
    cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    cairo_t *cr = cairo_create(surface);

    //we set the background color
    cairo_set_source_rgba(cr, bgColor.red, bgColor.green, bgColor.blue, bgColor.alpha);
    cairo_paint(cr);

    //here draw the pixbuf on the surface
    gdk_cairo_set_source_pixbuf(cr, pixbuf, 0, 0);
    cairo_paint(cr);

    cairo_destroy(cr);
    return surface;
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

        //this is important to automatically add .png extension to the end of filepath 
        if (outputPath.substr(outputPath.find_last_of(".") + 1) != "png") {
            outputPath += ".png";
        }

        if (!pixbuf)
        {
            std::cerr << "No EPS image loaded to save." << std::endl;
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
            std::cerr << "Failed to rotate image for saving." << std::endl;
            return;
        }

        int width = gdk_pixbuf_get_width(rotatedPixbuf);
        int height = gdk_pixbuf_get_height(rotatedPixbuf);
        cairo_surface_t *surface = createSurfaceWithBackground(rotatedPixbuf, width, height);

        cairo_surface_write_to_png(surface, outputPath.c_str());
        cairo_surface_destroy(surface);
        g_object_unref(rotatedPixbuf);
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
    GtkFileFilter *filter;

    GtkWidget *dialog = gtk_file_chooser_dialog_new("Open an EPS image File",
                                                    GTK_WINDOW(viewer->window),
                                                    GTK_FILE_CHOOSER_ACTION_OPEN,
                                                    "_Cancel", GTK_RESPONSE_CANCEL,
                                                    "_Open", GTK_RESPONSE_ACCEPT,
                                                    NULL);

    filter = gtk_file_filter_new();
    gtk_file_filter_add_pattern(filter, "*.eps");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
    //gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), "test.cpp");
    gtk_widget_show_all(dialog);

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
