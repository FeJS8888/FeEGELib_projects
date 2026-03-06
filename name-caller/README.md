# 点名器 (Name Caller)

基于 [FeEGELib](https://github.com/FeJS8888/FeEGELib) 制作的点名器示例程序。

## 功能特点

- **随机点名**：点击"开始点名"按钮，以滚动动画随机从成员列表中选取一人
- **可配置成员**：点击"配置成员"按钮，在记事本中编辑 `config/names.txt` 文件，保存后自动重载
- **UTF-8 支持**：配置文件支持 UTF-8 及 ANSI 编码的中文姓名

## 配置成员

编辑 `config/names.txt`，每行写一个成员姓名：

```
张三
李四
王五
# 以 # 开头的行为注释，会被忽略
```

## 构建方式

### 前置要求

- CMake >= 3.14
- MinGW-w64 (Windows) 或 x86_64-w64-mingw32 工具链 (Linux 交叉编译)
- Git（用于子模块初始化）

### 步骤

```bash
# 克隆仓库并初始化所有子模块
git clone --recurse-submodules https://github.com/FeJS8888/FeEGELib_projects.git
cd FeEGELib_projects/name-caller

# 配置并构建
mkdir build && cd build
cmake .. -G "Ninja" -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

构建完成后，`build/NameCaller.exe` 即为可执行文件。运行时，请确保 `config/names.txt` 与可执行文件位于同一目录下。

## CI/CD

本项目通过 GitHub Actions 自动构建，每次推送或 PR 时自动触发 Linux 交叉编译，产物 `NameCaller.exe` 可在 Actions 页面下载。
