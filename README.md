# lua-tablelib

tablelib是一个使用C++实现的动态库，提供给lua使用。

tablelib is a dynamic library implemented in C++ for lua.

# Installation

将tablelib.dll或tablelib.so文件放到你的lua项目的目录下，或者添加到你的环境变量中。

Put the tablelib.dll or tablelib.so file in your lua project directory, or add it to your environment variables.

# Usage
```lua
local tablelib = require "tablelib"

-- tostring
local s = tablelib.tostring({value = 1})
print(s)
-- {value = 1}

-- tojson
local tb = {["value"]=1,["array"]={[1]=1,[2]=2,},["hash"]={[100]="string",},}
local json = tablelib.tojson(tb)
print(json)
-- {"array":{"1":1,"2":2,"_int_keys_":["1","2"]},"value":1,"hash":{"100":"string","_int_keys_":["100"]}}

-- loadjson
local tb = tablelib.loadjson(json)
print(tablelib.tostring(tb))
-- {["value"]=1,["array"]={[1]=1,[2]=2,},["hash"]={[100]="string",},}
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
- `tojson`函数会将一个`table`转为`json`格式的字符串, 为了解决`table`数值`key`过于稀疏的问题, 会将所有的数值`key`(包括浮点数)转为`string`, 并添加`_int_keys_`字段, 它是一个数组, 会记录有哪些数值`key`被转换, 例如 : `{["value"]=1,["array"]={[1]=1,[2]=2,},["hash"]={[100]=999,},}`将会被转为:`{"array":{"1":1,"2":2,"_int_keys_":["1","2"]},"value":1,"hash":{"100":999,"_int_keys_":["100"]}}`
- The `tojson` function will convert a `table` to a `json` format string. To solve the problem of sparse numeric key in `table`, it will convert all numeric `key` (including floating point numbers) to `string`, and add `_int_keys_` field. It is an array that records which numeric `key` are converted1. For example : `{["value"]=1,["array"]={[1]=1,[2]=2,},["hash"]={[100]=999,},}` conver to : `{"array":{"1":1,"2":2,"_int_keys_":["1","2"]},"value":1,"hash":{"100":999,"_int_keys_":["100"]}}`
- `loadjson`函数会将`json`格式的字符串转为`table`, 如果存在`_int_keys_`字段, 会遍历`_int_keys_`, 将对应的`key`转为`number`
- The `loadjson` function will convert a `json` format string to a `table`. If the `_int_keys_` field exists, it will iterate through `_int_keys_` and convert the corresponding `key` to `number`.
