﻿# OpenMesh Python Plugin for OpenFlipper
OpenFlipper Plugin which runs any Python script with OpenMesh Python bindings.
The meshes (poly/tri) created by the script will be displayed in OpenFlipper as normal meshes.

This allows you to write geometry processing algorithms in Python.

The Plugin does not define an Interface for Python, so
no changes are required to already existing scripts.

## Requierements

- [OpenFlipper](https://www.openflipper.org) (version in gitrepo since 28.12.2016, new type system)
- [OpenMesh](https://www.openmesh.org)* (build with Python Bindings is __not__ required)
- [Python](https://www.python.org) (tested >= 3.6, builds with 2.7) 
- [Boost.Python](https://www.boost.org)

*Already included in OpenFlipper


## How-To-Use
	1. Write a Python script with OpenMesh bindings (nothing else required)
	2. Start OpenFlipper
	3. Select your saved Script in PyMesh's Toolbox
	4. Press "Run Script"
	5. See your result in the log window and/or the viewer

Also, have a look on the [example python script.](./python_example_script.py)

You can also use the Script language included in OpenFlipper, search for
`PyMesh.runPyScript` or `PyMesh.runPyScriptFile`.

## OpenFlipper Python Module
Your script has also access to the underlying OpenFlipper module which is automatically
loaded as a module called "openflipper"

Currently, following functions are supported:
```python
openflipper.meshes() # returns a dict with (meshname, mesh) for all meshes
openflipper.targets() # returns a dict with (meshname, mesh) for all meshes which are tagged as targets
openflipper.sources() # returns a dict with (meshname, mesh) for all meshes which are tagged as sources
```


## About Custom Properties
Custom properties created with python are supported. Remind, that the holding type of these properties
are always `PyObject`. After script execution, PyMesh tries to convert properties
from `PyObject` to the corresponding C/C++ type. Remind, that this process can be a huge performance issue.

## About Python Object Lifetime
Keep in mind, that the embedded python interpreter does not reset!!

OpenFlipper owns the underlying mesh ressources. Therefore, don't 
access on in OpenFlipper deleted meshes in your Python script, e.g. DON'T
```python
mesh = TriMesh()
...
# end of the python run
# delete mesh in OpenFlipper
# start new python run
mesh.update_normals() # still exists in the python env
```
Your program can and probably will crash.

## Project Status
Project was created and tested on windows.
Compiles under windows, maybe also under MacOs.
Please test and give some feedback.

Plugin can change, but I guess the interface (slot, for OF Script and the python binding interface) should be stable.

## License
[GPLv3 License](./LICENSE) © Matthias Möller. Made with ♥ in Germany.
