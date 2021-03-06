# Python Plugin for OpenFlipper
__Note: With the Release of OpenFlipper 4.0, this Plugin got integrated into the OpenFlipper Core. This ensures a better performance and also enables some features, which are not possible to implement by just using a Plugin.__

Currently (2019-03-07), it seems that the Core Version does not support the manipulation of meshes via Python, but it looks like the OF/OM-team is working on it (upstream OpenMesh-Python got similar changes to the OpenMesh-Python version in this Repository) and the support should only be a matter of time.

Therefore, with the integration to the Core, __this Plugin should be obsolete and I will no longer support it.__

Run Python Scripts in OpenFlipper with OpenMesh-Python support.

This OpenFlipper Plugin runs any Python script and provides OpenFlipper support for meshes
created with the OpenMesh-Python bindings. 
The meshes (poly/tri) created by the script will be displayed in OpenFlipper as normal meshes.
Additionally, it provides an Openflipper module for your Python script, giving you access
to the OpenFlipper internal script engine.

This allows you to write geometry processing algorithms in Python
but also use all the OpenFlipper features.

## Requirements

Note: This project uses submodules, you may want to clone with --recursive.

- [OpenFlipper](https://www.openflipper.org) (4.0+)
- [Python](https://www.python.org) (tested >= 3.6)

Also make sure, that the OpenMesh requirements i.e. NumPy are installed.

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

If you develop your own OpenFlipper-Plugin, you can execute Python scripts using the [RPC Plugin Interface](http://openflipper.org/Documentation/latest/a00087.html) or by the [internal scripting engine](http://openflipper.org/Documentation/latest/a00119.html).
The corresponding functions are the following:

```cpp

// execute a python script file
bool runPyScriptFile(const QString& _filename, bool _clearPrevious);

// execute python code
bool runPyScript(const QString& _script, bool _clearPrevious);

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
ofp.meshes() : Dict[str, Mesh]

# returns a dict with {meshname: mesh} for all meshes which are tagged as targets
ofp.targets() :Dict[str, Mesh]

# returns a dict with {meshname: mesh} for all meshes which are tagged as sources
ofp.sources() : Dict[str, Mesh]
 
# returns a mesh from the given OpenFlipper Id. return None if no mesh with such an id was found
ofp.get_mesh(id : integer)

# returns the openflipper id of the given mesh
ofp.get_id(mesh : Mesh)

```

### OpenFlipper Scripting

The ofp module also provides a lot of function (~90%) which are provided by the OpenFlipper internal script.

```python
ofp.plugin.functionname(args)
```
For example the following call
```python
cube_id = ofp.primitivesgenerator.addCube()
```
or

```python
ofp.backup.createBackup(cube_id, "testing_backup", ofp.Update.GEOMETRY | ofp.Update.TOPOLOGY)
cube_id = ofp.core.deleteObject(cube_id)
```

__Note:__ Not all return types are supported (espacially, the function which return Qt Objects). They are not filtered yet.


## About Custom Properties

You can convert all properties with python types (all properties created in python script), to the corresponding C++ Types (i.e. for futher processing or saving in om file format).
Currently, only pod types and buffers (such as numpy arrays) are supported.

__note:__ The Plugin PropertyVis only supports double/ACG::Vec3d, so you may cannot inspect some properties. Ask the creator of this Plugin for more type support.
 
## About Python Object Lifetime

Lifetime of Meshes are owned by OpenFlipper.

## Known isses

- The Meshes in OpenFlipper have already face normals after creation (also in python), where the Meshes given back by pure OpenMesh-Python (not running in OpenFlipper) have no face normals
- Converting Python Props to C++ Props will destroy the backup history. At the moment, it is not possible to clear the backup history. Therefore, expect some undefined behavior if you undo to a state before the conversion

## Project Build Status
Project was created and tested on Windows VS2017 and Linux using GCC8.

## License
[BSD 3-Clause](./LICENSE) © Matthias Möller. Made with ♥ in Germany.
