cmake_minimum_required(VERSION 3.16)
project(hairdresser_app VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Sql)

set(PROJECT_SOURCES
    src/main.cpp
    src/controllers/DBManager.cpp
    src/views/LoginWindow.cpp
    src/views/MainWindow.cpp
    src/views/BranchDialog.cpp
    src/views/HaircutDialog.cpp
    src/views/ClientDialog.cpp
    src/views/VisitDialog.cpp
    src/views/UserDialog.cpp
    src/views/PasswordChangeDialog.cpp
)

set(PROJECT_HEADERS
    src/models/Role.h
    src/models/User.h
    src/models/Branch.h
    src/models/Employee.h
    src/models/Client.h
    src/models/Haircut.h
    src/models/PriceHistory.h
    src/models/Document.h
    src/models/ReportStats.h
    src/controllers/DBManager.h
    src/views/LoginWindow.h
    src/views/MainWindow.h
    src/views/BranchDialog.h
    src/views/HaircutDialog.h
    src/views/ClientDialog.h
    src/views/VisitDialog.h
    src/views/UserDialog.h
    src/views/PasswordChangeDialog.h
)

add_executable(hairdresser_app ${PROJECT_SOURCES} ${PROJECT_HEADERS})

target_link_libraries(hairdresser_app PRIVATE
    Qt6::Widgets
    Qt6::Sql
)
