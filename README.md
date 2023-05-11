# lua-tablelib

tablelib是一个使用C++实现的动态库，提供给lua使用。

tablelib is a dynamic library implemented in C++ for lua.

# Installation

将tablelib.dll或tablelib.so文件放到你的lua项目的目录下，或者添加到你的环境变量中。

Put the tablelib.dll or tablelib.so file in your lua project directory, or add it to your environment variables.

# Usage
在你的lua代码中，使用require "tablelib"来加载这个库，然后你就可以使用`tablelib.tostring`函数来将一个table转换为字符串了。 例如：

In your lua code, use require "tablelib" to load this library, and then you can use the `tablelib.tostring` function to convert a table to a string. For example:

```lua
local tablelib = require "tablelib"
local s = tablelib.tostring({value = 1})
print(s)
-- {value = 1}
```

# Notes
- `tostring`函数可以接收任意参数。如果参数不是一个`table`，它会返回参数的字符串表示。
- The `tostring` function can accept any argument. If the argument is not a `table`, it will return the string representation of the argument.
- `tostring`函数可以安全地处理table中的循环引用。它会标记已访问过的元素，避免无限递归。
- The `tostring` function can handle circular references in tables safely. It will mark the visited elements and avoid infinite recursion.
- 如果你在编译库时添加了`-D_OPEN_HEX_`选项，它会把数值类型转为16进制字符串，性能优于10进制字符串。
- If you compile the library with the `-D_OPEN_HEX_` option, it will convert numeric values to hexadecimal strings, which have better performance than decimal strings.
- 如果你的`table`有`__pairs`元方法，那么`tostring`可能无法按照你的预期工作，它不会调用`__pairs`元方法来遍历table。
- If your table has a `__pairs` metamethod, then tostring may not work as you expect. It will not call the `__pairs` metamethod to iterate over the table.
