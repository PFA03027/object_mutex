cmake_minimum_required(VERSION 3.16)

file(GLOB HEADER_FILES inc/*.hpp )

add_library(object_mutex INTERFACE)
# target_sources(
#     object_mutex
#     INTERFACE
#         FILE_SET HEADERS
#         BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/inc
#         FILES
#             ${HEADER_FILES}
# )
set_target_properties(object_mutex PROPERTIES PUBLIC_HEADER "${HEADER_FILES}")
target_include_directories(object_mutex INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
    $<INSTALL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
)

# install(
#     TARGETS object_mutex
#     EXPORT object_mutex
#     FILE_SET HEADERS
# )
include(GNUInstallDirs)
install(TARGETS object_mutex
        PUBLIC_HEADER
          DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/
)
