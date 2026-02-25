cursedvga is a command line program that uses NCurses to decode and display TGA images. In adherence to NCurses' limitations (and because I think its neat) colors are mapped to their nearest match within an 8-bit color palette. Future plans include implementing the remaining color quantization methods, expanding support to other image formats (including GIFs), and adding an LED simulation.

Currently, cursedvga only supports a subset of TGA image types, that being run-length encoded RGB images (without a color map).

Requires ncurses 6.5 and primarily tested on kitty.
