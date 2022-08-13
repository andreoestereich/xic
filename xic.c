#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <errno.h>
#include <gd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char uchar;

void
window_name (Display *display, Window window)
{
    int i;

    XTextProperty text;
    XGetWMName (display, window, &text);
    printf ("id=0x%lx, XGetWMName=\"%s\"\n", window, text.value);
}

void
die (const char *errstr, ...)
{
    va_list ap;

    va_start (ap, errstr);
    vfprintf (stderr, errstr, ap);
    va_end (ap);
    exit (1);
}

void
change_window_icon (Display *dpy, Window *win, FILE *incon_file)
{
    Atom request;
    Atom netwmicon;
    // pid_t thispid = getpid();

    request = XInternAtom (dpy, "_NET_ACTIVE_WINDOW", False);

    /* use a png-image to set _NET_WM_ICON */
    /* load image in rgba-format */
    const gdImagePtr icon_rgba = gdImageCreateFromPng (incon_file);
    if (!icon_rgba)
        {
            fclose (incon_file);
            die ("Faled to load image from file.\n");
        }
    /* declare icon-variable which will store the image in argb-format */
    const int width = gdImageSX (icon_rgba);
    const int height = gdImageSY (icon_rgba);
    const int icon_n = width * height + 2;
    long icon_argb[icon_n];
    /* set width and height of the icon */
    int i = 0;
    icon_argb[i++] = width;
    icon_argb[i++] = height;
    /* rgba -> argb */
    for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
                {
                    const int pixel_rgba = gdImageGetPixel (icon_rgba, x, y);
                    unsigned char *pixel_argb = (unsigned char *)&icon_argb[i++];
                    pixel_argb[0] = gdImageBlue (icon_rgba, pixel_rgba);
                    pixel_argb[1] = gdImageGreen (icon_rgba, pixel_rgba);
                    pixel_argb[2] = gdImageRed (icon_rgba, pixel_rgba);
                    /* scale alpha from 0-127 to 0-255 */
                    const unsigned char alpha
                            = 127 - gdImageAlpha (icon_rgba, pixel_rgba);
                    pixel_argb[3] = alpha == 127 ? 255 : alpha * 2;
                }
        }
    gdImageDestroy (icon_rgba);
    /* set _NET_WM_ICON */
    netwmicon = XInternAtom (dpy, "_NET_WM_ICON", False);
    XChangeProperty (dpy, *win, netwmicon, XA_CARDINAL, 32, PropModeReplace,
                                     (uchar *)icon_argb, icon_n);

    XSync (dpy, False);
}

int
main (int argc, char *argv[])
{
    if (argc != 2)
        die ("A single argument is expected.\n");
    if (strcmp ("-h", argv[1]) == 0)
        {
            printf ("xic ICON_FILE\n");
            return 0;
        }

    FILE *incon_file = fopen (argv[1], "r");
    if (!incon_file)
        die ("Argument is not a file.\n");

    Display *dpy;
    Window win;
    int revert_to;

    if (!(dpy = XOpenDisplay (NULL)))
        die ("can't open display\n");

    XGetInputFocus (dpy, &win, &revert_to);
    if (win == None)
        die ("No focused window.\n");

    change_window_icon (dpy, &win, incon_file);

    fclose (incon_file);
}
