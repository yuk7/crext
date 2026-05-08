cmake_minimum_required(VERSION 3.10)
project(crext VERSION 2.6.0)

# C++17 Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files
set(SOURCES
    main.cpp
    ext2fs.cpp
    ext2read.cpp
    lvm.cpp
    log.c
    platform_dos.c
    platform_unix.c
    platform_win32.c
)

add_executable(crext ${SOURCES})

# Platform specific configurations
if(WIN32)
    # Ensure WIN32 is defined for our platform.h logic
    target_compile_definitions(crext PRIVATE WIN32 _WIN32)
    
    # Static linking for MinGW to remove DLL dependencies (libgcc, libstdc++, winpthread)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_link_options(crext PRIVATE "-static-libgcc" "-static-libstdc++" "-static")
    endif()
endif()

# Link necessary libraries for filesystem and threads
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # GCC/MinGW might need this for std::filesystem in C++17
    target_link_libraries(crext stdc++fs)
endif()

if(UNIX)
    target_link_libraries(crext pthread)
endif()
