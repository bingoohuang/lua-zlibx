# lua zlibx

## build

1. To use this library, you need [zlib](http://www.gzip.org/zlib/)
   1. centos `rpm -qa zlib`
   2. ubunto `dpkg -l | grep zlib`
2. To build this library, you can use [CMake](http://www.cmake.org), or you can use GNU Make: `make <platform>`
   1. download [cmake binnary](https://github.com/Kitware/CMake/releases/download/v3.28.1/cmake-3.28.1-linux-x86_64.tar.gz)
   2. e.g. `cmake -DLUA_INCLUDE_DIR=/home/app/luajit/include/luajit-2.1 -DLUA_LIBRARIES=/home/app/luajit/lib -DUSE_LUAJIT=ON -DUSE_LUA=OFF`  thanks [lua-zlib模块安装、使用](https://blog.51cto.com/u_5650011/5394910)
3. Use Luajit, download [openresty/luajit2](https://github.com/openresty/luajit2), [install](https://luajit.org/install.html) `make install PREFIX=/home/app/luajit`

## usage

Loading the library， If you built the library as a loadable package

`local zlib = require 'zlib'`

If you compiled the package statically into your application, call the function `luaopen_zlib(L)`. It will create a table with the zlib
functions and leave it on the stack.


`int major, int minor, int patch = zlib.version()`

returns numeric zlib version for the major, minor, and patch levels of the version dynamically linked in.

`function stream = zlib.deflate([ int compression_level ], [ int window_size ])`

If no compression_level is provided uses Z_DEFAULT_COMPRESSION (6), compression level is a number from 1-9 where zlib.BEST_SPEED is 1 and zlib.BEST_COMPRESSION is 9.

Returns a "stream" function that compresses (or deflates) all strings passed in.  Specifically, use it as such:

```lua
string deflated, bool eof, int bytes_in, int bytes_out = stream(string input [, 'sync' | 'full' | 'finish'])
```

Takes input and deflates and returns a portion of it,
optionally forcing a flush.

A 'sync' flush will force all pending output to be flushed to the return value and the output is aligned on a byte boundary,
so that the decompressor can get all input data available so far.  Flushing may degrade compression for some compression
algorithms and so it should be used only when necessary.

A 'full' flush will flush all output as with 'sync', and the compression state is reset so that decompression can restart
from this point if previous compressed data has been damaged or if random access is desired. Using Z_FULL_FLUSH too often
can seriously degrade the compression. 

A 'finish' flush will force all pending output to be processed and results in the stream become unusable.  Any future
attempts to print anything other than the empty string will result in an error that begins with IllegalState.

The eof result is true if 'finish' was specified, otherwise it is false.

The bytes_in is how many bytes of input have been passed to stream, and bytes_out is the number of bytes returned in deflated string chunks.

- `function stream = zlib.inflate([int windowBits])`

Returns a "stream" function that decompresses (or inflates) all strings passed in.  Optionally specify a windowBits argument
that is passed to inflateInit2(), see zlib.h for details about this argument.  By default, gzip header detection is done, and
the max window size is used.

The "stream" function should be used as such:

```lua
string inflated, bool eof, int bytes_in, int bytes_out = stream(string input)
```

Takes input and inflates and returns a portion of it.  If it detects the end of a deflation stream, then total will be the
total number of bytes read from input and all future calls to stream() with a non empty string will result in an error that
begins with IllegalState.

No flush options are provided since the maximal amount of input is always processed.

eof will be true when the input string is determined to be at the "end of the file".

The bytes_in is how many bytes of input have been passed to stream, and bytes_out is the number of bytes returned in inflated string chunks.

For data created by merging multiple compressed streams (#41), you want to continue in inflating until there is no more data.

Example:

```lua
local source = io.open"allparts.gz"
local dest = io.tmpfile()

local inflate = zlib.inflate()
local shift = 0
while true do
    local data = source:read(4096) -- read in 4K chunks
    if not data then break end     -- end if no more data
    local inflated, eos, bytes_in, bytes_out = inflate(data)
    dest:write(inflated)           -- write inflated data
    if eos then                    -- end of current stream
        source:seek("set", shift + bytes_in)
        shift = shift + bytes_in   -- update shift
        inflate = zlib.inflate()     -- new inflate per stream
    end
end
```

- `function compute_checksum = zlib.adler32()`
- `function compute_checksum = zlib.crc32()`

Create a new checksum computation function using either the adler32 or crc32 algorithms.  This resulting function should be used as such:

```lua
int checksum = compute_checksum(string input |
                                function compute_checksum)
```

The compute_checksum function takes as input either a string that is logically getting appended to or another
compute_checksum function that is logically getting appended.

The result is the updated checksum.

For example, these uses will all result in the same checksum:

```lua
-- All in one call:
local csum = zlib.crc32()("one two")

-- Multiple calls:
local compute = zlib.crc32()
compute("one")
assert(csum == compute(" two"))

-- Multiple compute_checksums joined:
local compute1, compute2 = zlib.crc32(), zlib.crc32()
compute1("one")
compute2(" two")
assert(csum == compute1(compute2))
```

NOTE: This library ships with an "lzlib" compatibility shim. However, the following things are not compatible:

* `zlib.version()` in lzlib returns a string, but this library returns a numeric tuple (see above).
* `zlib.{adler,crc}32()` in lzlib returns the `{adler,crc}32` initial value, however if this value is used with calls to adler32 it works in compatibility mode.

To use this shim add the `-DLZLIB_COMPAT` compiler flag.

## sm3/sm3hmac

- `local sm3 = zlib.sm3("lqlq666lqlq946")`
- `local sm3_base64 = zlib.sm3_base64("lqlq666lqlq946")`
- `local sm3_base64_url = zlib.sm3_base64_url("lqlq666lqlq946")`

- `local sm3hmac = zlib.sm3hmac("lqlq666lqlq946", "123")`
- `local sm3hmac_base64 = zlib.sm3hmac_base64("lqlq666lqlq946", "123")`
- `local sm3hmac_base64_url = zlib.sm3hmac_base64_url("lqlq666lqlq946", "123")`

- `local encoded = zlib.base64_encode(raw)`
- `local raw = zlib.base64_decode(encoded)`

- `local encoded = zlib.base64_encode_url(raw)`
- `local raw = zlib.base64_decode_url(encoded)`

Note: In openresty, base64 is already in lua-resty-core as following:

[ngx encode_base64](https://github.com/openresty/lua-nginx-module#ngxencode_base64)

```lua
local newstr = ngx.encode_base64(str, no_padding?)
local newstr = ngx.decode_base64(str)
```

[ngx.base64](https://github.com/openresty/lua-resty-core/blob/master/lib/ngx/base64.md) - urlsafe base64 encode/decode functions OpenResty/ngx_lua.

```lua
local b64 = require("ngx.base64")
local res, err

res = b64.encode_base64url("foo")

res, err = b64.decode_base64url(res)
if not res then
    -- invalid input
    ngx.log(ngx.ERR, err)
end

assert(res == "foo")
```

examples:

```lua
function tohex(str)
    return (str:gsub('.', function (c)
        return string.format('%02X', string.byte(c))
    end))
end

local sm3 = zlib.sm3_base64("lqlq666lqlq946")
print(sm3)
-- 5k/Xb0B45R3KQoMj0/rb1dUnI7vxN5GEZQ2lzmACsr8=
 
local sm3hmac = zlib.sm3hmac_base64("lqlq666lqlq946", "123")
print(sm3hmac)
-- +7Z/yTZ3cBGqcDNvDwtjBdUpqXqH2OyoiARyzSwwpyE=
```

use ggt to check it

```sh
$ ggt sm3 -i lqlq666lqlq946 --base64
sm3 lqlq666lql... => 5k_Xb0B45R3KQoMj0_rb1dUnI7vxN5GEZQ2lzmACsr8 len:43

$ KEY=123 ggt sm3 -i lqlq666lqlq946 --base64
sm3 lqlq666lql... => -7Z_yTZ3cBGqcDNvDwtjBdUpqXqH2OyoiARyzSwwpyE len:43
```

- thanks [sm3 c](https://github.com/guanzhi/GmSSL)
- thanks [sm3-lua-bind](https://github.com/openLuat/LuatOS/tree/master/components/gmssl)
- thanks [base64 c](https://github.com/sniperHW/chuck/blob/master/src/util/base64.c) [base64 lua-bind](https://github.com/sniperHW/chuck/blob/master/src/luabind/base64.h)

### sm3hmac_base64 performance

checkit in [test.lua](./test.lua#L267)

```sh
#a:     332     #b:     32      sm3hmac_base64(a, b):   kVQjPJwA1JZmrDSNsE/ALI01f1okExR4WCeGFJQBn0E=
Total execution 1000000 times in 3.062802 seconds
Benchmark results: 1000000 function calls | 3088145.00 μs elapsed | 3.09 μs avg execution time.
```

Thanks:

- [Some Tests. Lua Performance 一些测试。Lua 性能](https://forum.defold.com/t/some-tests-lua-performance/70782)
- [Lua脚本性能优化指南](https://github.com/flily/lua-performance/blob/master/Guide.zh.md)
