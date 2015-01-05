Description
-----------
Compilation of various basic programs about computer vision. It uses [OpenCV](http://opencv.org/).


Installation
------------
* Linux: `sudo apt-get install libopencv-dev libcv-dev libcvaux-dev libhighgui-dev`.
    * for a manual installation, see [there](https://help.ubuntu.com/community/OpenCV).
* Mac OS X: `brew tap homebrew/science && brew install opencv`.


Compilation
-----------
Don't forget to link the OpenCV library: `` `pkg-config --cflags --libs opencv` ``.

Example: ``g++ `pkg-config --cflags --libs opencv` test.c -o test``.

