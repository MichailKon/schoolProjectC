cmake_minimum_required(VERSION 3.19)
project(schoolProjectC)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(QT_VERSION 5)
set(REQUIRED_LIBS Core Gui Widgets)
set(REQUIRED_LIBS_QUALIFIED Qt5::Core Qt5::Gui Qt5::Widgets Qt5Sql)

include_directories(uis)
include_directories(databases)

configure_file(databases/students.sqlite3 databases/students.sqlite3 COPYONLY)

set(SOURCES main.cpp main_window.cpp add_edit_student.cpp students_review.cpp useful_funcs.cpp)
set(HEADERS add_edit_student.h main_window.h students_review.h useful_funcs.h edit_view_marks.cpp edit_view_marks.h)

add_executable(${PROJECT_NAME} main.cpp ${SOURCES} ${HEADERS})

if (NOT CMAKE_PREFIX_PATH)
    message(WARNING "CMAKE_PREFIX_PATH is not defined, you may need to set it "
            "(-DCMAKE_PREFIX_PATH=\"path/to/Qt/lib/cmake\" or -DCMAKE_PREFIX_PATH=/usr/include/{host}/qt{version}/ on Ubuntu)")
endif ()

find_package(Qt${QT_VERSION} COMPONENTS ${REQUIRED_LIBS} REQUIRED)
target_link_libraries(${PROJECT_NAME} ${REQUIRED_LIBS_QUALIFIED})
