include (plugin)

add_subdirectory(pybind11)

################################
## OpenFlipper Plugin
openflipper_plugin (INSTALLDATA Icons 
	TYPES TRIANGLEMESH POLYMESH 
	DIRS "PyModules" 
	#INCDIRS "OpenMesh-Python/src/"
	)
	
	
if(TARGET Plugin-PyMesh)	
	target_link_libraries(Plugin-PyMesh pybind11::module pybind11::embed)

	if(MSVC)
		target_compile_options(Plugin-PyMesh PUBLIC "/bigobj" )
	endif()
endif()
