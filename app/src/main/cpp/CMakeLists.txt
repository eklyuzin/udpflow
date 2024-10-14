# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html.
# For more examples on how to use CMake, see https://github.com/android/ndk-samples.

# Sets the minimum CMake version required for this project.
cmake_minimum_required(VERSION 3.22.1)

set(CMAKE_CXX_STANDARD 17)

# Declares the project name. The project name can be accessed via ${ PROJECT_NAME},
# Since this is the top level CMakeLists.txt, the project name is also accessible
# with ${CMAKE_PROJECT_NAME} (both CMake variables are in-sync within the top level
# build script scope).
project("udpflow")

if (0)
set(CMAKE_FIND_DEBUG_MODE TRUE)

#set(BOOST_ROOT /home/ek/AndroidStudioProjects/Boost-for-Android/build/out/arm64-v8a/lib/)
set(CMAKE_PREFIX_PATH             /home/ek/AndroidStudioProjects/Boost-for-Android/build/out/arm64-v8a/)
set(Boost_DEBUG 1)

set(Boost_ADDITIONAL_VERSIONS "1.85")
set(Boost_USE_STATIC_LIBS ON)
set(USE_STATIC_BOOST ON)  # Not sure if you really need this later.  If not, delete.
set(Boost_USE_STATIC_RUNTIME OFF)
#find_package(Boost REQUIRED)
find_package(Boost CONFIG COMPONENTS thread date_time REQUIRED
        PATHS /home/ek/AndroidStudioProjects/Boost-for-Android/build/out/arm64-v8a/lib/cmake/Boost-1.85.0/)
endif()

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.
#
# In this top level CMakeLists.txt, ${CMAKE_PROJECT_NAME} is used to define
# the target library name; in the sub-module's CMakeLists.txt, ${PROJECT_NAME}
# is preferred for the same purpose.
#
# In order to load a library into your app from Java/Kotlin, you must call
# System.loadLibrary() and pass the name of the library defined here;
# for GameActivity/NativeActivity derived applications, the same library name must be
# used in the AndroidManifest.xml file.
add_library(${CMAKE_PROJECT_NAME} SHARED
        # List C/C++ source files with relative paths to this CMakeLists.txt.
        include/udpflow/CommandHandler.h
        include/udpflow/ICommandHandler.h
        include/udpflow/Server.h
        include/udpflow/Stat.h
        include/udpflow/StatCollector.h
        src/CommandHandler.cpp
        src/Sender.cpp
        src/Server.cpp
        src/Sender.h
        src/Stat.cpp
        src/StatCollector.cpp
        native-lib.cpp)

target_include_directories(${CMAKE_PROJECT_NAME}
        PRIVATE
            include)
# Specifies libraries CMake should link to your target library. You
# can link libraries from various origins, such as libraries defined in this
# build script, prebuilt third-party libraries, or Android system libraries.
target_link_libraries(${CMAKE_PROJECT_NAME}
        # List libraries link to the target library
        android
        log)