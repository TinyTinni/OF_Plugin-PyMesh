# Python Plugin for OpenFlipper
This OpenFlipper Plugin runs any Python script and provides OpenFlipper support for meshes
created with the OpenMesh-Python bindings. 
The meshes (poly/tri) created by the script will be displayed in OpenFlipper as normal meshes.
Additionally, it provides an Openflipper module for your Python script, giving you access
to the OpenFlipper internal script engine.

This allows you to write geometry processing algorithms in Python
but also use all the OpenFlipper features.

## Requirements
Note: This project uses submodules, you may want to clone with --recursive.

- [OpenFlipper](https://www.openflipper.org) (version in gitrepo)
- [Python](https://www.python.org) (tested >= 3.6, builds with 2.7)

Already included as submodules:
- [pybind11](https://github.com/pybind/pybind11)
- [OpenMesh-Python](https://www.graphics.rwth-aachen.de:9000/OpenMesh/openmesh-python)


## Build Instructions
- Place (e.g. via git clone --recursive) the files in a directory called "Plugin-PyMesh" into your OpenFlipper source dir.
- Run CMake
- Build, using your selected toolchain

## How-To-Use
	1. Write a Python script with OpenMesh bindings (nothing else required)
	2. Start OpenFlipper
	3. Select your saved Script in PyMesh's Toolbox
	4. Press "Run Script"
	5. See your result in the log window and/or the viewer

Also, have a look at the [example python script.](./python_example_script.py)

You can also use the script language included in OpenFlipper, search for
`PyMesh.runPyScript` or `PyMesh.runPyScriptFile`.

## Execute Python Script from a Plugin
If you develop your a OpenFlipper-Plugin, you can execute Python scripts using the [RPC Plugin Interface](http://openflipper.org/Daily-Builds/Doc/Free/Developer/a14371.html) or using the [internal scripting engine](http://openflipper.org/Daily-Builds/Doc/Free/Developer/a14403.html).
The correspoding functions are the following:
```cpp

bool runPyScriptFile(const QString& _filename, bool _clearPrevious);
void runPyScriptFileAsync(const QString& _filename, bool _clearPrevious);

bool runPyScript(const QString& _script, bool _clearPrevious);
void runPyScriptAsync(const QString& _script, bool _clearPrevious);

// try to convert all properties from PyObject to the corresponding C++ type
void convertPropsPyToCpp(const IdList& _list);

// resets the interpreter
void resetInterpreter();

```


## OpenFlipper Python Module
Your script has also access to the underlying OpenFlipper module which is automatically
loaded as a module called "openflipper" into "ofp" namespace (OpenFlipperPython)

Currently, following functions are supported:
```python
Mesh = Union[TriMesh,PolyMesh]

# returns a dict with {meshname: mesh} for all meshes
ofp.meshes() : List[str, Mesh]

# returns a dict with {meshname: mesh} for all meshes which are tagged as targets
ofp.targets() :List[str, Mesh]

# returns a dict with {meshname: mesh} for all meshes which are tagged as sources
ofp.sources() : List[(str, Mesh]
 
# returns a mesh from the given OpenFlipper Id. return None if no mesh with such an id was found
ofp.get_mesh(id : integer)

# returns the openflipper id of the given mesh
ofp.get_id(mesh : Mesh)

```

### RPC (_(experimental)_)
Use the following function to communicate through the [RPC Interface](http://openflipper.org/Documentation/latest/a00087.html).
```python
ofp.rpc_call(plugin, functionname)
```
For example the following call
```python
cube_id = ofp.rpc_call("primitivesgenerator","addCube")
```
creates a cube using the PrimitivesGenerator Plugin.
Theoretically, you should be able to call every script function which is provided by the internal OpenFlipper Script
functionality.

Call using parameters (currently, needs the type encoded. dict pattern [type, variable_name, type, variable_name, ...])
No all types are support (especially all Qt type except QString)
```python
cube_id = ofp.rpc_call(core","deleteObject",["int",cube_id])
```
Currently supported types:
- QString
- int
- uint
- bool
- IdList
- Vector
- Vector4
- UpdateType (for constants e.g. UPDATE_ALL use openflipper.Update.ALL)


## About Custom Properties
After script execution, the plugin tries to convert all python properties into c++ properties.
Currently, only pod types and the vectors by ACG are supported.
Not supported yet are properties added as numpy array.

## About Python Object Lifetime

Lifetime of Meshes are owned by OpenFlipper.

## Known isses
- Python Mesh in OpenFlipper has already face normals after creation, where the Mesh given back by pure OpenMesh-Python has no face normals

## Project Build Status
Project was created and tested on Windows VS2017 and Linux using GCC7.

## License
[GPLv3 License](./LICENSE) © Matthias Möller. Made with ♥ in Germany.
