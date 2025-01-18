cmake_minimum_required(VERSION 3.28)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.15 CACHE STRING "Build for 10.15")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

## Temporarily use a date driven project version. Move to 1.0 when we ship 1.0
string(TIMESTAMP DAY_OF_YEAR "%j")
string(TIMESTAMP YEAR "%Y")
math(EXPR PART0 "${YEAR}-2023 + 2")
math(EXPR PART1 "${DAY_OF_YEAR}/2")
message(STATUS "Using date-driven version starting with '0'. 0.${PART0}.${PART1}.0")


project(sapphire-plugins VERSION 0.${PART0}.${PART1}.0 LANGUAGES C CXX)
set(PRODUCT_NAME "Sapphire")


option(USE_SANITIZER "Build and link with ASAN" FALSE)
option(COPY_AFTER_BUILD "Will copy after build" TRUE)
include(cmake/compile-options.cmake)

## New version
include(libs/sst/sst-plugininfra/cmake/git-version-functions.cmake)
version_from_versionfile_or_git()
message(STATUS "DISPLAY_VERSION=${GIT_IMPLIED_DISPLAY_VERSION}; COMMIT_HASH=${GIT_COMMIT_HASH}; BRANCH=${GIT_BRANCH}")

set(JUCE_PATH "${CMAKE_SOURCE_DIR}/libs/JUCE")
add_subdirectory(libs)

include(cmake/CmakeRC.cmake)

file(GLOB_RECURSE GRAPHICS  "res/knob_graphics/*" "libs/sapphire/res/*.svg")
list(LENGTH GRAPHICS glen)
message(STATUS "Adding ${glen} graphics files to the binary image")
cmrc_add_resource_library(${PROJECT_NAME}-graphics NAMESPACE sapphire_graphics ${GRAPHICS})

set(ELASTIKA_DIR libs/sapphire/src)
add_library(elastika-dsp STATIC
        ${ELASTIKA_DIR}/mesh_physics.cpp
        ${ELASTIKA_DIR}/elastika_mesh.cpp
        ${ELASTIKA_DIR}/sapphire_panel.cpp
)
target_include_directories(elastika-dsp PUBLIC ${ELASTIKA_DIR} libs/simde)
target_compile_definitions(elastika-dsp PUBLIC NO_RACK_DEPENDENCY)
if (WIN32)
    target_compile_definitions(elastika-dsp PUBLIC _USE_MATH_DEFINES)
endif()

add_library(${PROJECT_NAME}-impl STATIC
        src/clap/sapphire-clap-entry-impl.cpp

        src/elastika/processor.cpp
        src/elastika/editor.cpp

        src/tube_unit/processor.cpp
        src/tube_unit/editor.cpp

        src/shared/graphics_resources.cpp
        src/shared/sapphire_lnf.cpp
)
target_include_directories(${PROJECT_NAME}-impl PUBLIC src)
target_compile_definitions(${PROJECT_NAME}-impl PRIVATE
        PRODUCT_NAME="${PRODUCT_NAME}"
)
target_link_libraries(${PROJECT_NAME}-impl PUBLIC
        clap
)
target_link_libraries(${PROJECT_NAME}-impl PRIVATE
        elastika-dsp
        ${PROJECT_NAME}-graphics
        simde
        clap-helpers clap-wrapper-extensions
        fmt-header-only
        sst-basic-blocks  sst-cpputils
        sst-plugininfra
        sst-plugininfra::filesystem
        sst-plugininfra::tinyxml
        sst-plugininfra::strnatcmp
        sst-plugininfra::patchbase
        sst-plugininfra::version_information

        sst::clap_juce_shim sst::clap_juce_shim_headers
        juce::juce_gui_basics
)

make_clapfirst_plugins(
        TARGET_NAME ${PROJECT_NAME}
        IMPL_TARGET ${PROJECT_NAME}-impl

        OUTPUT_NAME "${PRODUCT_NAME}"

        ENTRY_SOURCE src/clap/sapphire-clap-entry.cpp

        BUNDLE_IDENTIFER "org.cosinkitty.sapphire"
        BUNDLE_VERSION ${PROJECT_VERSION}

        COPY_AFTER_BUILD ${COPY_AFTER_BUILD}

        PLUGIN_FORMATS CLAP VST3 AUV2

        # You can add a target-per-standalone you want. Syntax here is
        #   target-postfix output-name clap-id
        # This allows you to make multiple standalones from a multi-plugin clap
        #STANDALONE_CONFIGURATIONS
        #    standalone "${PRODUCT_NAME}" "org.baconpaul.six-sines"
)




