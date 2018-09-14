
# Include Testing package
if(BUILD_TESTING)

  include( ${CMAKE_SOURCE_DIR}/OpenFlipper/tests/testGenerators.cmake )
  
  set(OPENMESH_PYTHON_TEST_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../openmesh-python/tests/")

  # ========================================================================
  # ========================================================================
  # Tests
  # ========================================================================
  # ========================================================================
  run_local_script_test("om_tests.ofs")
  run_local_script_test("test_example.ofs")
  run_local_script_test("test_reset.ofs")
  run_local_script_test("test_return_values.ofs")
  
endif()
