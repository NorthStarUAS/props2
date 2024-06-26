# props2 - PropertyTree version 2

**Structured shared in-memory data repository.**

props2 is a "property tree" system for convenient organization and
sharing of data between code modules and even for sharing data among
hybrid python/C++ application modules.

This core functionality is written in C++ using rapidjson as the tree
management backend.  A matching python interface is included (using
pybind11) so that the property tree can also serve as a quick/simple
mechanism for sharing data between hybrid Python and C++ applications.

Traditionally code modules pass data through a rigid (and often
brittle or clunky) API's that are define by each module.  The property
tree establishes an organized tree of data that is shared and
accessible by all modules in the application.  It assumes some basic
cooperation and rule following between modules, but there are many
very nice outcomes to this approach.

## Quick Installation Guide

Please notice there is two parts to the install process: the python
install and the C library install.  (I use the property tree to share
program state between python and C++ modules within the same hybrid
app.  This simplifies dealing with module api's and interfacing and
managing the data flow.

## Install the python module

    ./setup.py build
    sudo ./setup.py install

## Install the C++ library

    mkdir build
    cd build
    cmake ..
    make
    sudo make install

## Install the C++ library with MinGW/cmake on windows

    mkdir build
    cd build
    cmake .. -G "MinGW Makefiles"
    make
    make DESTDIR=~/mingw64 install

## Special hybrid (python and C++) application API note

For sharing the PropertyTree between python-wrapped C++ modules that
are imported into a python script and the main python interpreter,
there is a simple API setup to inform the C++ module of the shared
proprty tree pointer:

In your main python script call:

    # Python script
    from PropertyTree import PropertyNode
    root = PropertyNode("/")
    doc = root.get_Document()

In your C++ (wrapped) module code include something like this
(presumes the init() function will be wrapped/exported with pybind11
or something similar):

    #include <props2.h>
    void init(DocPointerWrapper d) {
        PropertyNode("/").set_Document(d);
    }

Once this step is complete, both python and C++ modules can freely
create and access properties from the same shared tree to exchange
program state.

## Background

Every non-trivial program needs a way to share data between modules.
Traditional data encapsulation involves hiding and protecting a
module's data so that other modules cannot directly touch it.  Instead
any interaction with the module's data is through the module's
carefully crafted API.  This is called "data encapsulation" and it is
at the foundation of object oriented programming.

Constructing an application's modules and data in a carefully packaged
way is a great idea.  It is a very robust way to structure your
applications and avoid a large class of bugs that arise out of
careless data sharing.  However, every ideology has to eventually
survive on it's own in the real world.  There is a reason C++ classes
have static (shared) members and friend functions.  There is a reason
C++ supports global variable space.  Real world programming challenges
are often messier than the book examples.

There are times when it makes the most sense to share data globally
within your application. It just does.  (Sometimes it still makes
sense to use a goto.)

rc-props provides a way to create shared data space within your
application in a friendly and well structured way.  With rc-props as
the backbone of your application, you receive many additional nice
services and structures.

The original idea for the 'Property System' probably dates back to
long ago, but the first implementation that I am aware of grew up
within the FlightGear ecosystem.  It has proved to be so convenient
and nice that I have used it in several other large projects.

Now, here, I have re-imagined the property system, stripped down to
it's essentials, simplified, and rewritten entirely in python.

## The Property System

The word "System" is carefully chosen.  The Property System is an
interwoven network of concepts that can bring huge value to your
application.  It may not be the right choice for every application,
but in the right context it is super awesome.

* It provides structured data sharing between modules within your
  application.  (I.e. when your app needs global data shared, don't
  hack it and hide it, embrace it with a real structure that avoids
  the bad side of global variables.)

* It provides a way to organize the important data structures within
  your application (the property tree.)

* The new python implementation now enables rich data sharing between
  C++ and Python without forcing the C++ coder to wade through an
  obtuse Python C++ API.

* The property tree maps well to xml files enabling sophisticated
  configuration file support with a few easy function calls.

* The property tree can be exposed to external interfaces (i.e. via a
  network socket) to enable allowed external programs to conveniently
  access and even change items in the property tree.  This can be
  useful for debugging or for simple (low bandwidth) interaction
  between separate applications.

## The Property Tree

The rc-props module enables an application to easily build a tree of
important data to be shared throughout the program.  There is a single
shared 'root' node that is automatically created when the props module
is imported.  From there an application can start filling in the tree
very quickly.  For example, consider a reader/writer example of a
simple autopilot that reads an external gps and uses that data to
navigate.

A hierarchical tree structure is easy to understand, easy to organize,
easy to use, and keeps like data near each other.

### Example: writer module

The gps driver module could include the following (python) code to
publish gps tstate to the shared property tree so other modules can
quickly access this information:

    from PropertyTree import PropertyNode
    (lat, lon, alt, speed, ground_track) = read_gps()
    gps_node = PropertyNode("/sensors/gps")
    gps_node.setDouble("latitude_deg", lat)
    gps_node.setDouble("longitude_deg", lon)
    gps_node.setDouble("altitude_m", alt)
    gps_node.setDouble("speed_mps", speed)
    gps_node.setDouble("ground_track_deg", ground_track)

With this simple bit of code we have constructed our property tree,
read the gps, and shared the values with the rest of our application.
Done!

The PropretyNode() constructor will find the specified path in the property
tree and return that node to you.

### Example: reader module

The navigation module needs the gps information from the property tree
and do something with it.  The code starts out very similar, but watch
what we can do:

    from PropertyTree import PropertyNode
    gps_node = PropertyNode("/sensors/gps")
    wpt_node = PropertyNode("/navigation/route/target")
    ap_node = PropertyNode("/autopilot/settings")
    heading = great_circle_route([gps_node.getDouble("latitude_deg"), gps_node.getDouble("longitude_deg")],
                                 [wpt_node.getDouble("latitude_deg"), wpt_node.getDouble("longitude_deg")])
    ap_node.setDouble("target_heading_deg", heading)

Did you see what happened there?  We first grabbed a pointer the
shared gps node, then we grabbed the shared target waypoint node, and
we grabbed the autopilot settings node.  We called a function to
compute the heading from our current location to our target waypoint
and we wrote that back into the autopilot configuration node.

This approach to sharing data between program modules is a bit unique.
But consider the alternatives: many applications grow their
inter-module communication ad-hoc as the code evolves and some of the
interfaces can become inconsistent or awkward as real world data gets
incrementally shoved into existing C++ class api's.  The result of the
ideological approach is often messy and clunky.

The property system provides an alternative for intra-application data
sharing that is simple, easy to understand, and just works.  It is a
different philosophy of programming from what many people are used to,
but the benefits and convenience of the property system quickly
becomes a way of life.

### Module initialization order.

Please notice that both the reader and writer modules in the above
example call getNode() with the create flag set to true.  The property
tree system allows initialization order independence among modules.
With respect to the property tree (ignoring other higher level
application specific dependencies) the modules can be initialized in
any order.  The first module to initialize and request the specific
property nodes will trigger their creation, and subsequent modules
will find the nodes already there.

### Sharing data between mixed C++ and Python applications

A duplicate python interface to the C++ property tree is provided (via
pybind11).  This allows hybrid python/C++ apps to share data between
modules without needing to worry about complex API calls, data flow,
initialization order, etc.

### Script features for C++

For the C++ developer: incorporating the Property Tree into your
application brings several conveniences of scripting languages to your
application.  One big convenience is automatic type conversion.  For
example, an application can write a string value into a field of the
property tree, but read it back out as a double.  Watch carefully:

    #include <props2.h>
    int main() {
        PropertyNode gps_node("/sensors/gps");
        // write the value as a string
        gps_node.setString("lat", "-45.235");
        // read the value back as a double
        double lat = gps_node.getDouble("lat");
    }

Did you see how the value of "lat" is written as a string constant,
but can be read back out as a double?  Often it is easy to keep your
types consistent, but it's nice to just let the back end system convert
types for you as needed.

### Performance considerations

The property tree uses rapidjson as the backend for all dynamic tree
management functionality.  This provides a nice balance between memory
usage and performance.

Notice the the PropretyNode("/path") constructor and getChild("path")
method both can traverse parts of the tree using string compares.  For
this reason, it is recommended to do the name lookup in an init
function and cache the result for later use in your update function,
perhaps like this:

    #include <props2.h>
    class gps_reader {
        PropertyNode gps_node;
    public:
        void init() {
            gps_node = PropertyNode("/sensors/gps");
        }
        void update() {
            gps_node.getDouble("latitude_deg");
        }
    };

As with anything related to performance, the performance you see with
your application running on your propcessor is the only benchmark that
matters.  In many cases it is fine to do the name lookup each time
through a loop if that simplifies your code and makes it more
readable.

### Easy I/O for reading and writing configuration files

The hierarchical structure of the property tree (and the fact that
it's built using rapidjson as the backened) means the property tree
maps directly json.  Loading a json file into the property tree can be
a great way to add config file support to your application with a
single function call.  Then the config values are available in the
property tree for the various modules to use as needed.  I often do
something like this:

    #include <props2.h>
    main() {
        PropertyNode config_node("/config");
        config_node.load("config.json");
    }

Now your application configuration settings are available under the
/config node in the property tree.

Notice that key/value style configuration files can often simplify
upgrading to new versions of your app or supporting older config
variants.  Binary config files can be more brittle to deal with.

### A note on threaded applications

The Property Tree system is *not* thread safe.  If you wish to use
this system in a threaded application, you will have to manage
exclusive access to the property tree through your own higher level
application mutual exclusion mechanisms.
