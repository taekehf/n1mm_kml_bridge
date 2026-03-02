# N1MM → QRZ → Google Earth KML Bridge

A lightweight program running in the background. It
- listens to N1MM UDP logging messages
- grabs the call
- queries QRZ.com for the Maidenhead grid
- converts this to map coordinates
- writes them to a KML-file for Google Earth Pro

The program is written with the help of Claude Sonnet 4.6 and tinkering by myself in C++ for QT6.10 It requires GCC v15+ and CMake 4.0+ to compile.
A compiled version for WIndows 64 has been included. Currently v 0.1

The program is a command-line program, so should be run from the terminal. Unpack it, right-click the directory and open that in the terminal and start with .\n1mm_kml_bridge
If there is no config-file, it will create one for you. Or you download the example in the config-directory, adjust it to your liking and save it in the program's directory, then start it. Please remember the QRZ-password is stored unencrypted, but if you leave the password-field empty, you can enter the password every time you start the program.

Inspiration for this program is the contest-station OT5A where the IT-geeks created a similar program.

Hope you enjoy it.

73, Martin - PE1EEC
