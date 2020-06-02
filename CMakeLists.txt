cmake_minimum_required(VERSION 3.0.0)
project(NightcoreBot VERSION 0.1.0)

# use, i.e. don't skip the full RPATH for the build tree
SET(CMAKE_SKIP_BUILD_RPATH  FALSE)

# when building, don't use the install RPATH already
# (but later on when installing)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

# the RPATH to be used when installing
SET(CMAKE_INSTALL_RPATH "./")

# don't add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

add_subdirectory(externals/tiny-process-library)

include_directories("${PROJECT_SOURCE_DIR}/externals/CJSON"
                    "${PROJECT_SOURCE_DIR}/externals/tiny-process-library"
                    "${PROJECT_SOURCE_DIR}/externals/sts"
                    "${PROJECT_SOURCE_DIR}/externals/tinyformat"
                    "${PROJECT_SOURCE_DIR}/externals/CLog"
                    "${PROJECT_SOURCE_DIR}/externals/dr_libs"
                    "${PROJECT_SOURCE_DIR}/externals/libDiscordBot/include"
                    "${PROJECT_SOURCE_DIR}/include")

link_directories("${PROJECT_SOURCE_DIR}/externals/libDiscordBot/lib/")

file(GLOB SRC 
    "${PROJECT_SOURCE_DIR}/src/services/*.cpp"
    "${PROJECT_SOURCE_DIR}/src/controllers/*.cpp"
    "${PROJECT_SOURCE_DIR}/src/commands/*.cpp")

add_executable(${PROJECT_NAME} main.cpp ${SRC})
target_link_libraries(${PROJECT_NAME} tiny-process-library discordbot)

file(GLOB LIBS "${PROJECT_SOURCE_DIR}/externals/libDiscordBot/lib/*${CMAKE_SHARED_LIBRARY_SUFFIX}*")

install(FILES "${PROJECT_SOURCE_DIR}/settings.json" DESTINATION "${CMAKE_INSTALL_PREFIX}/")
install(FILES ${LIBS} DESTINATION "${CMAKE_INSTALL_PREFIX}/")
install(DIRECTORY DESTINATION "cache")
install(DIRECTORY DESTINATION "log")

install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}
  RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}
)