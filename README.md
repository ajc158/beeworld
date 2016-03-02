# beeworld
A raytraced virtual environment for simulating a bee's field of view suing the Giger algorithm (http://andygiger.com/science/beye/beyehome.html)

The program is written using Qt 5, and should work with most versions of Qt. 

This is research code, so unfortunately comments are sparse to non-existent. The main file
of import is beeworldwindow.h/cpp, as this is the main hub of the program. It initialises 
the raytracer (beeworldgiger.h/cpp) and loads in config xml files. There are a few examples 
in the 'examples' folder. The program communicates with external SpineML models (see user
SpineML on GitHub, or http://bimpa.group.shef.ac.uk/SpineML/index.php/Home for more details)
using a tcp/ip interface - the xml file defines the ports by which the program will communicate, 
allowing the model to alter aspects of the beeworld 'in flight' (e.g. object locations/rotation, 
texture colors/scalings, bee location/attitude). 

The world consists of a set of configurable primitives (sphere, plane, cylinder) which can be textured with a set
of configurable textures (check, radial stripes). These are all subclasses of the master sceneobject and scenetexture
classes, so can be extended if need be.

If you are interested then have a play and add Issues to the tracker if there are any questions!
