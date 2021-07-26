# Quick install instructions

## Install the python wrapper modules

I use the property tree to share program state between python and C++
modules within the same hybrid app.  This majorly simplifies
interfacing and data flow in a hybrid app.

    $ ./setup.py build
    $ sudo ./setup.py install

## Install the C++ library

    $ mkdir build
    $ cd build
    $ cmake ..
    $ make
    $ sudo make install