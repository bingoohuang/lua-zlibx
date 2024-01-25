# lua zlibx

## build

1. To use this library, you need [zlib](http://www.gzip.org/zlib/)
   1. centos `rpm -qa zlib`
   2. ubunto `dpkg -l | grep zlib`
2. To build this library, you can use [CMake](http://www.cmake.org), or you can use GNU Make: `make <platform>`
   1. download [cmake binnary](https://github.com/Kitware/CMake/releases/download/v3.28.1/cmake-3.28.1-linux-x86_64.tar.gz)
   2. e.g. `cmake -DLUA_INCLUDE_DIR=/home/app/bingoohuang/luajit2/include/luajit-2.1 -DLUA_LIBRARIES=/home/app/bingoohuang/luajit2/lib -DUSE_LUAJIT=ON -DUSE_LUA=OFF`

## usage

Loading the library:

    If you built the library as a loadable package

```lua
    [local] zlib = require 'zlib'
```

If you compiled the package statically into your application, call
the function `luaopen_zlib(L)`. It will create a table with the zlib
functions and leave it on the stack.

### -- zlib functions --

`int major, int minor, int patch = zlib.version()`

returns numeric zlib version for the major, minor, and patch
levels of the version dynamically linked in.

`function stream = zlib.deflate([ int compression_level ], [ int window_size ])`

If no compression_level is provided uses Z_DEFAULT_COMPRESSION (6),
compression level is a number from 1-9 where zlib.BEST_SPEED is 1
and zlib.BEST_COMPRESSION is 9.

Returns a "stream" function that compresses (or deflates) all
strings passed in.  Specifically, use it as such:

```lua
string deflated, bool eof, int bytes_in, int bytes_out =
        stream(string input [, 'sync' | 'full' | 'finish'])
```

Takes input and deflates and returns a portion of it,
optionally forcing a flush.

A 'sync' flush will force all pending output to be flushed to
the return value and the output is aligned on a byte boundary,
so that the decompressor can get all input data available so
far.  Flushing may degrade compression for some compression
algorithms and so it should be used only when necessary.

A 'full' flush will flush all output as with 'sync', and the
compression state is reset so that decompression can restart
from this point if previous compressed data has been damaged
or if random access is desired. Using Z_FULL_FLUSH too often
can seriously degrade the compression. 

A 'finish' flush will force all pending output to be processed
and results in the stream become unusable.  Any future
attempts to print anything other than the empty string will
result in an error that begins with IllegalState.

The eof result is true if 'finish' was specified, otherwise
it is false.

The bytes_in is how many bytes of input have been passed to
stream, and bytes_out is the number of bytes returned in
deflated string chunks.

`function stream = zlib.inflate([int windowBits])`

Returns a "stream" function that decompresses (or inflates) all
strings passed in.  Optionally specify a windowBits argument
that is passed to inflateInit2(), see zlib.h for details about
this argument.  By default, gzip header detection is done, and
the max window size is used.

The "stream" function should be used as such:

```lua
string inflated, bool eof, int bytes_in, int bytes_out =
        stream(string input)
```

Takes input and inflates and returns a portion of it.  If it
detects the end of a deflation stream, then total will be the
total number of bytes read from input and all future calls to
stream() with a non empty string will result in an error that
begins with IllegalState.

No flush options are provided since the maximal amount of
input is always processed.

eof will be true when the input string is determined to be at
the "end of the file".

The bytes_in is how many bytes of input have been passed to
stream, and bytes_out is the number of bytes returned in
inflated string chunks.

For data created by merging multiple compressed streams (#41),
you want to continue in inflating until there is no more data.
Example:

```lua
local source = io.open"allparts.gz"
local dest = io.tmpfile()

local inflate = lz.inflate()
local shift = 0
while true do
    local data = source:read(4096) -- read in 4K chunks
    if not data then break end     -- end if no more data
    local inflated, eos, bytes_in, bytes_out = inflate(data)
    dest:write(inflated)           -- write inflated data
    if eos then                    -- end of current stream
        source:seek("set", shift + bytes_in)
        shift = shift + bytes_in   -- update shift
        inflate = lz.inflate()     -- new inflate per stream
    end
end
```

`function compute_checksum = zlib.adler32()`

`function compute_checksum = zlib.crc32()`

Create a new checksum computation function using either the
adler32 or crc32 algorithms.  This resulting function should be
used as such:

```lua
int checksum = compute_checksum(string input |
                                function compute_checksum)
```

The compute_checksum function takes as input either a string
that is logically getting appended to or another
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

NOTE: This library ships with an "lzlib" compatibility shim. However, the
following things are not compatible:

* `zlib.version()` in lzlib returns a string, but this library returns a
numeric tuple (see above).

* `zlib.{adler,crc}32()` in lzlib returns the `{adler,crc}32` initial value,
however if this value is used with calls to adler32 it works in
compatibility mode.

To use this shim add the `-DLZLIB_COMPAT` compiler flag.

## sm3/sm3hmac

- `local sm3 = lz.sm3("lqlq666lqlq946")`
- `local sm3hmac = lz.sm3hmac("lqlq666lqlq946", "123")`


examples:

```lua
function tohex(str)
    return (str:gsub('.', function (c)
        return string.format('%02X', string.byte(c))
    end))
end

local sm3 = lz.sm3("lqlq666lqlq946")
print(tohex(sm3))
-- E64FD76F4078E51DCA428323D3FADBD5D52723BBF1379184650DA5CE6002B2BF
 
local sm3hmac = lz.sm3hmac("lqlq666lqlq946", "123")
print(tohex(sm3hmac))
-- FBB67FC936777011AA70336F0F0B6305D529A97A87D8ECA8880472CD2C30A721
```

use ggt to check it

```sh
$ ggt sm3 -i lqlq666lqlq946    
sm3 lqlq666lql... => E64FD76F4078E51DCA428323D3FADBD5D52723BBF1379184650DA5CE6002B2BF len:64

$ KEY=123 ggt sm3 -i lqlq666lqlq946
sm3 lqlq666lql... => FBB67FC936777011AA70336F0F0B6305D529A97A87D8ECA8880472CD2C30A721 len:64
```

- thanks [sm3 c](https://github.com/guanzhi/GmSSL)
- thanks [sm3-lua-bind](https://github.com/openLuat/LuatOS/tree/master/components/gmssl)
