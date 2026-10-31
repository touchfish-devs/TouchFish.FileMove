> 本项目已迁移至 https://github.com/touchfish-devs/TouchFish.FileMove

# TouchFish 文件移动器
![Platform](https://img.shields.io/badge/platform-Windows-lightgrey.svg)
![Language](https://img.shields.io/badge/language-C++-orange.svg)



一个高效的 Windows 文件移动工具，支持符号链接创建，界面美观，操作流畅，使用 C++ 开发！

## Feature
- **非阻塞操作** - 使用多线程处理，界面永不卡顿
- **实时进度显示** - 清晰显示移动进度、文件大小和速度
- **符号链接支持** - 自动在原位置创建符号链接，保持软件兼容性

## 工作原理

1. **移动文件** - 将源文件夹的所有内容移动到目标位置
2. **创建链接** - 在原位置创建符号链接指向新位置
3. **透明访问** - 任何访问原位置的程序都会自动重定向到新位置

这样可以将安装在C盘的程序移动到其他盘，节省系统盘空间，同时不会破坏程序运行。

## 下载

从 Releases 页面下载最新版本。

## 使用方法

1. **以管理员身份运行** 程序（创建符号链接需要管理员权限）
2. 选择要移动的**源文件夹**
3. 选择**目标位置**
4. 勾选"在原位置创建符号链接"（推荐）
5. 点击"开始移动"按钮

> **提示**: 移动过程中会显示进度对话框，可以随时取消操作

## 注意事项

### 不建议移动的目录

以下系统关键目录**不应该**被移动，可能导致系统功能异常：

- `C:\Windows`
- `C:\Program Files`（整个文件夹）
- `C:\Program Files (x86)`（整个文件夹）
- `C:\Users`
- `C:\Documents and Settings`

### 可以安全移动的目录

以下目录可以安全移动：

- `C:\Program Files\某个特定程序`
- `C:\Program Files (x86)\某个特定游戏`
- 用户文档、下载、视频等个人文件夹
- 大型软件或游戏的独立安装目录

## 许可证

本项目采用 GPL-3.0 许可证 - 详见 [LICENSE](LICENSE) 文件
