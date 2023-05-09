# lua-table2string

tableop是一个使用C++实现的动态库，提供给lua使用，作用是将lua的table转换为字符串。

tableop is a dynamic library implemented in C++ for lua, which can convert lua tables to strings.

# Installation

将tableop.dll或tableop.so文件放到你的lua项目的目录下，或者添加到你的环境变量中。

Put the tableop.dll or tableop.so file in your lua project directory, or add it to your environment variables.

# Usage
在你的lua代码中，使用require "tableop"来加载这个库，然后你就可以使用tableop.tostring函数来将一个table转换为字符串了。 例如：

In your lua code, use require “tableop” to load this library, and then you can use the tableop.tostring function to convert a table to a string. For example:

```lua
local tableop = require "tableop"
local s = tableop.tostring({value = 1})
print(s)
-- 输出：{value = 1}
```

# Notes
- tableop.tostring函数可以接收任意参数。如果参数不是一个table，它会返回参数的字符串表示。
- tableop.tostring函数可以安全地处理table中的循环引用。它会标记已访问过的元素，避免无限递归。
- The tableop.tostring function can accept any argument. If the argument is not a table, it will return the string representation of the argument.
- The tableop.tostring function can handle circular references in tables safely. It will mark the visited elements and avoid infinite recursion.

