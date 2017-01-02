# OpenMesh Python Plugin for OpenFlipper
This Plugin runs a given Python Script with OpenMesh Bindings.
Created Meshes will be displayed in OpenFlipper.
This allows you to write Geometry Processing Algorithms in Python
and 

## Requierements

- [OpenFlipper](https://www.openflipper.org)(Qt >= 5.0)
- [OpenMesh](https://www.openmesh.org)* (build with Python Bindings is __not__ required)
- [Python](https://www.python.org) (tested >= 3.6) 
- [Boost.Python](https://www.boost.org)

*Already included in OpenFLipper


## How-To-Use
	1. Write a Python script with OpenMesh bindings
	2. Start OpenFlipper
	3. Select your saved Script in PyMesh's Toolbox
	4. Press "Run Script"
	5. See your result in the log window and/or the viewer

An Example Python Script is included.

You can also use the Script language included in OpenFlipper, search for
`PyMesh.runPyScript` or `PyMesh.runPyScriptFile`.

## Python Object Lifetime
Keep in mind, that the embedded python interpreter does not reset!
Also, don't reference on in OpenFlipper deleted meshes in your Python Script,
your program can crash.

## Project Status
Project was created on windows and is not well tested. Maybe doesnt compile with Linux/MacOS.
Please test and give some feedback.

Maybe I will add some Python Bindings for OpenFlipper functions like "getTargets".

## License
[GPLv3 License](./LICENSE) © Matthias Möller. Made with ♥ in Germany.
