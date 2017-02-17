include (plugin)
set (_flags "")

################################
## Boost
if (MSVC AND NOT USE_STATIC_BOOST)
	set(USE_STATIC_BOOST OFF CACHE BOOL "Use Static libs of boost")
endif()
FIND_PACKAGE(Boost COMPONENTS python QUIET)


if (USE_STATIC_BOOST OR Boost_USE_STATIC_LIBS)
	set( _flags ${flags_} BOOST_PYTHON_STATIC_LIB)#already set by OpenMesh/Python/Bindings.cc
endif()

################################
## Python
FIND_PACKAGE(PythonLibs QUIET)
set (_flags ${_flags}
	#python uses members like "slots", nasty python
	#"QT_NO_KEYWORDS" disabled because OF Interface Headers dont support it
	)

################################
## OpenFlipper Plugin
openflipper_plugin (INSTALLDATA Icons 
	DEPS PythonLibs Boost #add additional findpacke, so OF can do the error handling
	CDEFINITIONSADD ${_flags}	
	INCDIRS  ${PYTHON_INCLUDE_DIRS} # give a strong hint about the slightly different name of python include dir/libs
	LIBRARIES ${PYTHON_LIBRARIES}
	TYPES TRIANGLEMESH POLYMESH 
	)

