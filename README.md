# OpenMesh Python Plugin for OpenFlipper
OpenFlipper Plugin which runs any Python script with OpenMesh Python bindings.
The meshes (poly/tri) created by the script will be displayed in OpenFlipper as normal meshes.

This allows you to write geometry processing algorithms in Python.

The Plugin does not define an Interface for Python, so
no changes are required to already existing scripts.

## Requierements

- [OpenFlipper](https://www.openflipper.org)(Qt >= 5.0)
- [OpenMesh](https://www.openmesh.org)* (build with Python Bindings is __not__ required)
- [Python](https://www.python.org) (tested >= 3.6) 
- [Boost.Python](https://www.boost.org)

*Already included in OpenFLipper


## How-To-Use
	1. Write a Python script with OpenMesh bindings (nothing else required)
	2. Start OpenFlipper
	3. Select your saved Script in PyMesh's Toolbox
	4. Press "Run Script"
	5. See your result in the log window and/or the viewer

An example Python script is included.

You can also use the Script language included in OpenFlipper, search for
`PyMesh.runPyScript` or `PyMesh.runPyScriptFile`.

## About Custom Properties
Custom properties created with python are supported. Remind, that the holding type of these properties
are always `PyObject`. Usual OpenFlipper Plugins like the PropertyVis-Plugin does not support this type.
At the current state, a transformation from a property holding a `PyObject` type to the corresponding
C/C++ (pod only) type is __not__ supported.

## About Python Object Lifetime
Keep in mind, that the embedded python interpreter does not reset!!

OpenFlipper owns the underlying mesh ressources. Therefore, don't 
access on in OpenFlipper deleted meshes in your Python script, e.g. a python variable references on a
mesh from a previous run, which was deleted in OpenFlipper.
Your program can and probably will crash.

## Project Status
Project was created on windows. Maybe doesnt compile with Linux/MacOS.
Please test and give some feedback.

Extensions:
- Maybe I will add some Python bindings for OpenFlipper functions like "getTargets".

## License
[GPLv3 License](./LICENSE) © Matthias Möller. Made with ♥ in Germany.
