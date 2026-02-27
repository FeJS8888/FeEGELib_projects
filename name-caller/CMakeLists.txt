cmake_minimum_required(VERSION 3.14)
project(NameCaller)

set(CMAKE_CXX_STANDARD 23)

# 配置 FeEGELib 子模块选项（必须在 add_subdirectory 之前设置）
set(EGE_BUILD_DEMO OFF CACHE BOOL "Do not build xege demos" FORCE)
set(EGE_BUILD_TEST OFF CACHE BOOL "Do not build xege tests" FORCE)
set(EGE_BUILD_TEMP OFF CACHE BOOL "Do not build xege temp" FORCE)

# 引入 FeEGELib 作为第三方库
add_subdirectory(3rdparty/FeEGELib)

# 允许多重定义（与 FeEGELib 保持一致）
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--allow-multiple-definition")

# 编译点名器主程序
add_executable(NameCaller WIN32 src/main.cpp)
target_link_libraries(NameCaller PRIVATE FeEGELib_core)
set_target_properties(NameCaller PROPERTIES WIN32_EXECUTABLE TRUE)
target_link_options(NameCaller PRIVATE -static)
