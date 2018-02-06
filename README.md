# OpenMesh Python Plugin for OpenFlipper
OpenFlipper Plugin which runs any Python script with OpenMesh Python bindings.
The meshes (poly/tri) created by the script will be displayed in OpenFlipper as normal meshes.

This allows you to write geometry processing algorithms in Python
and use all the OpenFlipper features for your mesh.

The Plugin provides an interface to some OpenFlipper functions.
But they are not necessary for visualizing the mesh.
Just import your already existing Python Code without any change and
the mesh(es) will be there.

## Requirements

(Same as OpenMesh Python Bindings + OpenFlipper)

Note: This project uses submodules, you may want to clone with --recursive.
- [OpenFlipper](https://www.openflipper.org) (version in gitrepo)
- [OpenMesh](https://www.openmesh.org)* (build with Python Bindings is __not__ required)
- [Python](https://www.python.org) (tested >= 3.6, builds with 2.7) 
- [Boost.Python](https://www.boost.org)

*Already included in OpenFlipper

## Build Instructions
- Place (e.g. via git clone --recursive) the files in a directory called "Plugin-PyMesh" into your OpenFLipper Source dir.
- Run CMake
- add Python & Boost Python, if CMake couldn't find python or boost, have a look at ["OpenMesh Python Bindings Tutorial"](http://openmesh.org/Daily-Builds/Doc/a03957.html)
- you don't have to enable "OPENMESH_BUILD_PYTHON_BINDINGS"
- Build, using your selected toolchain

## How-To-Use
	1. Write a Python script with OpenMesh bindings (nothing else required)
	2. Start OpenFlipper
	3. Select your saved Script in PyMesh's Toolbox
	4. Press "Run Script"
	5. See your result in the log window and/or the viewer

Also, have a look at the [example python script.](./python_example_script.py)

You can also use the Script language included in OpenFlipper, search for
`PyMesh.runPyScript` or `PyMesh.runPyScriptFile`.

## Execute Python Script from a Plugin
If you develop your own OpenFlipper-Plugin, you can execute Python scripts using the [RPC Plugin Interface](http://openflipper.org/Daily-Builds/Doc/Free/Developer/a14371.html) or using the [internal scripting engine](http://openflipper.org/Daily-Builds/Doc/Free/Developer/a14403.html).
The correspoding functions are the following:
```cpp

void runPyScriptFile(const QString& _filename, bool _clearPrevious);
void runPyScriptFileAsync(const QString& _filename, bool _clearPrevious);

void runPyScript(const QString& _script, bool _clearPrevious);
void runPyScriptAsync(const QString& _script, bool _clearPrevious);

// try to convert all properties from PyObject to the corresponding C++ type
void convertPropsPyToCpp(const IdList& _list);

```


## OpenFlipper Python Module
Your script has also access to the underlying OpenFlipper module which is automatically
loaded as a module called "openflipper"

Currently, following functions are supported:
```python
openflipper.meshes() # returns a dict with {meshname: mesh} for all meshes
openflipper.targets() # returns a dict with {meshname: mesh} for all meshes which are tagged as targets
openflipper.sources() # returns a dict with {meshname: mesh} for all meshes which are tagged as sources
```

### _(experimental functions)_

```python
openflipper.get_tri_mesh(int id)# returns a mesh from the given OpenFlipper Id 
openflipper.get_poly_mesh(int id)# returns a mesh from the given OpenFLipper Id
```
__note:__ no error handling is implemented at the current state, so make sure the Id exists and points to a correct classified mesh.

### RPC
Use the following function to communicate through the [RPC Interface](http://openflipper.org/Documentation/latest/a00087.html).
```python
openflipper.rpc_call(plugin, functionname)
```
For example the following call
```python
cube_id = openflipper.rpc_call("primitivesgenerator","addCube")
```
creates a cube using the PrimitivesGenerator Plugin.
Theoretically, you should be able to call every script function which is provided by the internal OpenFLipper Script
functionality.

__Note__: Currently, every function which does not take any parameters are supported. 
The type conversion QScriValue <-> PythonType is not implemented yet.


## About Custom Properties
Custom properties created with python are supported. Remind, that the holding type of these properties
are always `PyObject`. After script execution, PyMesh tries to convert properties
from `PyObject` to the corresponding C/C++ type. Remind, that this process can be a huge performance issue.

## About Python Object Lifetime

The Interpreter is does not reset after each run.
Lifetime of Meshes are owned by OpenFlipper.

## Project Build Status
Project was created and tested on Windows with VS2015 and VS2017 using Python 3.6.

## License
[GPLv3 License](./LICENSE) © Matthias Möller. Made with ♥ in Germany.
