set(SG_MODULE_${SG_MODULE_NAME}_TEST_DEPENDS
  ${SG_MODULE_${SG_MODULE_NAME}_LIBRARY}
  ${SG_MODULE_${SG_MODULE_NAME}_DEPENDS}
  ${GTEST_LIBRARIES})
set(qt_tests "")
if(SG_MODULE_VISUALIZE_ENABLE_QT)
    set(qt_tests test_visualize_histograms.cpp)
endif()
set(SG_MODULE_${SG_MODULE_NAME}_TESTS
  test_visualize_spatial_graph.cpp
  ${qt_tests}
  )

SG_add_gtests()