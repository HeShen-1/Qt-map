# 2025.4.12
- 实现了代码编辑区的行号实现
- 代码的高亮实现
# 2025.4.13
- 实现栅格地图保存为json文件的功能
- 存在很多报错；需要实现创建地图时修改栅格属性，
# 2025.4.14
- 今天完成的任务：
    - 重新修改了栅格地图绘制的函数，给每个栅格添加了属性，默认创建的栅格均为可通行
    - 实现了栅格属性修改的功能，修复了存在多个起点、终点的BUG
    - 修复了无法保存栅格地图的BUG，实现保存栅格地图为json文件的功能
    - 实现记忆化文件打开与保存：再次打开或保存文件会定位到之前打开的文件夹
- 明天的任务：
    - 为栅格属性设置添加快捷键
    - 实现识别json文件为栅格地图
    - 实现代码与栅格地图的联动
    - 实现其他菜单功能
# 2025.4.17
- 补充
    - 添加小车图标，添加小车的运动轨迹
    - 动态添加障碍、起始点，验证算法的可行性和及时性
    - 添加寻路算法的模板
    - 添加SLAM的算法
- 重庆理工大学图书馆-数字资源
- 实现功能
    - 实现创建起始点出现小车和旗帜，修复删除起点、终点时图标没有删除的bug
    - 为栅格属性设置添加快捷键（未实现）
- 为函数添加注释，明确各模块的功能，优化结构

- 优化代码高亮




------

# First Level Heading

Paragraph.

## Second Level Heading

Paragraph.

- bullet
+ other bullet
* another bullet
    * child bullet

1. ordered
2. next ordered

### Third Level Heading

Some *italic* and **bold** text and `inline code`.

An empty line starts a new paragraph.

Use two spaces at the end  
to force a line break.

A horizontal ruler follows:

---

Add links inline like [this link to the Qt homepage](https://www.qt.io),
or with a reference like [this other link to the Qt homepage][1].

    Add code blocks with
    four spaces at the front.

> A blockquote
> starts with >
>
> and has the same paragraph rules as normal text.

First Level Heading in Alternate Style
======================================

Paragraph.

Second Level Heading in Alternate Style
---------------------------------------

Paragraph.

[1]: https://www.qt.io
