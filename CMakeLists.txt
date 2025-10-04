cmake_minimum_required(VERSION 3.13)

project(pdfool
  VERSION 0.1.0
  DESCRIPTION "PDF tooling playground"
  LANGUAGES C CXX)

# Language standards
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Options
option(PDFOOL_WITH_PODOFO "Build/link PoDoFo from 3rd/podofo" ON)

# -----------------------------------------------------------------------------
# pdfio (prebuilt static library in 3rd/pdfio/build/Release/libpdfio.a)
# Creates imported target: PDFIO::pdfio
# -----------------------------------------------------------------------------
set(PDFIO_ROOT           ${CMAKE_SOURCE_DIR}/3rd/pdfio)
set(PDFIO_BUILD_RELEASE  ${PDFIO_ROOT}/build/Release)
set(PDFIO_LIB            ${PDFIO_BUILD_RELEASE}/libpdfio.a)
set(PDFIO_INC            ${PDFIO_BUILD_RELEASE}/usr/local/include)
set(PDFIO_INC_CANDIDATES
  ${PDFIO_BUILD_RELEASE}/usr # headers generated/installed by Xcode build
  ${PDFIO_ROOT})             # fallback where pdfio.h lives

if(EXISTS ${PDFIO_LIB})
  add_library(PDFIO::pdfio STATIC IMPORTED GLOBAL)
  set_target_properties(PDFIO::pdfio PROPERTIES
    IMPORTED_LOCATION "${PDFIO_LIB}"
    INTERFACE_INCLUDE_DIRECTORIES "${PDFIO_INC_CANDIDATES}")
else()
  message(STATUS "pdfio static lib not found at ${PDFIO_LIB}. Skipping PDFIO::pdfio target.")
endif()

# -----------------------------------------------------------------------------
# PoDoFo (CMake-based in 3rd/podofo). Prefer using the subproject.
# Provides either target PoDoFo::podofo or podofo depending on version.
# -----------------------------------------------------------------------------
set(PODOFO_ROOT ${CMAKE_SOURCE_DIR}/3rd/podofo)
set(PODOFO_TARGET_NAME "")

if(PDFOOL_WITH_PODOFO AND EXISTS ${PODOFO_ROOT}/CMakeLists.txt)
  # Build as subproject to get targets exported into this build.
  add_subdirectory(${PODOFO_ROOT} ${CMAKE_BINARY_DIR}/_podofo EXCLUDE_FROM_ALL)
  # Try to detect the canonical target name
  if(TARGET PoDoFo::podofo)
    set(PODOFO_TARGET_NAME PoDoFo::podofo)
  elseif(TARGET podofo)
    set(PODOFO_TARGET_NAME podofo)
  endif()
else()
  # Fallback: try system package if available
  find_package(PoDoFo QUIET)
  if(TARGET PoDoFo::podofo)
    set(PODOFO_TARGET_NAME PoDoFo::podofo)
  elseif(TARGET podofo)
    set(PODOFO_TARGET_NAME podofo)
  endif()
endif()

# -----------------------------------------------------------------------------
# poppler (prebuilt static library in 3rd/poppler-25.09.1/build/Release/libpoppler.a)
# -----------------------------------------------------------------------------
set(POPPLER_ROOT           ${CMAKE_SOURCE_DIR}/3rd/poppler-25.09.1)
set(POPPLER_BUILD_RELEASE  ${POPPLER_ROOT}/build/darwin)
set(POPPLER_LIB            ${POPPLER_BUILD_RELEASE}/libpoppler.dylib)
set(POPPLER_INC            ${POPPLER_ROOT})
set(POPPLER_GLIB_LIB       ${POPPLER_BUILD_RELEASE}/glib)
set(POPPLER_GLIB_INC       "${POPPLER_ROOT}/glib"
                           "${POPPLER_ROOT}/build/darwin/glib")
set(GLIB_LIB               "/opt/homebrew/Cellar/glib/2.86.0/lib"
                           "/opt/homebrew/Cellar/cairo/1.18.4/lib")
set(GLIB_INC               "/opt/homebrew/Cellar/glib/2.86.0/include/glib-2.0" 
                           "/opt/homebrew/Cellar/glib/2.86.0/lib/glib-2.0/include"
                           "/opt/homebrew/Cellar/cairo/1.18.4/include/cairo")

add_executable(pdfool-crop 
  src/pdfool-crop.c
)

target_include_directories(pdfool-crop PRIVATE
  ${PDFIO_INC}
  ${GLIB_INC}
  ${POPPLER_INC}
  ${POPPLER_GLIB_INC}
)

target_link_directories(pdfool-crop PRIVATE
  ${PDFIO_BUILD_RELEASE}
  ${POPPLER_BUILD_RELEASE}
  ${GLIB_LIB}
  ${POPPLER_GLIB_LIB}
)

target_link_libraries(pdfool-crop PRIVATE
  pdfio
  glib-2.0
  gobject-2.0
  poppler-glib
  cairo
  poppler
)
