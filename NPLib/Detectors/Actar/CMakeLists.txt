add_custom_command(OUTPUT TActarPhysicsDict.cxx COMMAND ../../scripts/build_dict.sh TActarPhysics.h TActarPhysicsDict.cxx TActarPhysics.rootmap libNPActar.dylib DEPENDS TActarPhysics.h)
add_custom_command(OUTPUT TActarDataDict.cxx COMMAND ../../scripts/build_dict.sh TActarData.h TActarDataDict.cxx TActarData.rootmap libNPActar.dylib DEPENDS TActarData.h)
add_library(NPActar SHARED TActarSpectra.cxx TActarData.cxx TActarPhysics.cxx TActarDataDict.cxx TActarPhysicsDict.cxx )
target_link_libraries(NPActar ${ROOT_LIBRARIES} NPCore) 
install(FILES TActarData.h TActarPhysics.h TActarSpectra.h DESTINATION ${CMAKE_INCLUDE_OUTPUT_DIRECTORY})

