set(CMAKE_C_FLAGS "-Wall -Wextra -Wno-missing-field-initializers -Wno-empty-body $ENV{CFLAGS}")
set(CMAKE_C_FLAGS_DEBUG "-O0 -g3")
set(CMAKE_C_FLAGS_RELEASE "-Werror -O3 -DNDEBUG")
