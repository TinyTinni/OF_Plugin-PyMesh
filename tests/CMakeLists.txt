
# Include Testing package
if(BUILD_TESTING)

  include( ${CMAKE_SOURCE_DIR}/OpenFlipper/tests/testGenerators.cmake )
  
  set(OPENMESH_PYTHON_TEST_DIR "${CMAKE_SOURCE_DIR}/Plugin-PyMesh/OpenMesh-Python/tests/")

  # ========================================================================
  # ========================================================================
  # Tests
  # ========================================================================
  # ========================================================================
  run_local_script_test("om_tests.ofs")
  
endif()
