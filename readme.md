# cds学习nanopb的教程文档



## 1、 环境搭建

1. 通过源码配置后 [GitHub下载链接](https://github.com/sexjun/nanopb) 需要下载下面这些库文件。

> 1. 创建文件：`requirement.txt`
> 2. 将下方代码写入上述文件
> 3. 执行：`pip install -r requirements.txt`
```python
pip uninstall protobuf
pip uninstall google
pip uninstall grpcio-tools

pip install google
pip install protobuf
pip install grpcio-tools
```

2. 下载之后就可以在

- 编码需要
`pb_encode.h` `pb_encode.c` (needed for encoding messages)

- 解码需要
`pb_decode.h` `pb_decode.c` (needed for decoding messages)

- 编解码都需要
`pb_encode.h`  `pb_encode.c` (needed for encoding messages)

- 用户自己的文件
person.proto (just an example)
person.pb.c (autogenerated, contains message descriptors)
person.pb.h (autogenerated, contains type declarations and macros)

通过以上文件就可以组成对nanopb的使用。加入到自己的编译系统里。加入规则可以参考我这个工程的cmake编译规则。



**nanopb Feature:**
c c++的流工具




## 2、 库使用流程梳理
1. 首先下载nanopb，搭建环境，将nanopb纳入自己的工程编译系统。
2. 根据自己需求编写`Foo.proto`文件和`Foo.options`
3. 调用用函数编译生成`message.pb.h`和`message.pb.c`文件，这二个就是需要参加编译系统参加编解码的规则文件。
```shell
user@host:~$ nanopb/generator/nanopb_generator.py message.proto
Writing to message.pb.h and message.pb.c
```

4. 编写代码根据上一步生成的文件进行编解码，完成流传输。



## 3. 基本概念

### 3.1 流的特性

Nanopb 使用流来访问编码格式的数据。流抽象非常轻量级，由一个结构（`pb_ostream_t`或`pb_istream_t`）组成，其中包含一个指向回调函数的指针。

回调函数有一些通用规则：

1. IO 错误返回 false。编码或解码过程将立即中止。
2. 使用 state 来存储您自己的数据，例如文件描述符。
3. `bytes_written`并`bytes_left`通过pb_write和pb_read更新。
4. 您的回调可以与子流一起使用。在这种情况下`bytes_left`，`bytes_written`并且`max_size`具有比原始流更小的值。不要使用这些值来计算指针。
5. 始终读取或写入请求的完整数据长度。例如，POSIX`recv()`需要`MSG_WAITALL`参数来完成此操作。

输入流有一个特殊的特点：

6. 您不需要提前知道消息的长度。读取时出现EOF错误后，设置`bytes_left`为0并返回`false`。`pb_decode()`将检测到这一点，如果 EOF 处于正确位置，它将返回 true。

- 输出流
```c
struct _pb_ostream_t
{
    // 编码的回调函数
   bool (*callback)(pb_ostream_t *stream, const uint8_t *buf, size_t count);
   void *state;
   // 编码的最大长度
   size_t max_size;
   // 已经编码了多大
   size_t bytes_written;
};
```

- 输入流
```c
struct pb_istream_s
{
    // 解码的回调函数
    bool (*callback)(pb_istream_t *stream, pb_byte_t *buf, size_t count);
    void *state; 
    // 是否解码了所欲字符
    size_t bytes_left;  
#ifndef PB_NO_ERRMSG
    const char *errmsg;
#endif
};

```


- 输入流


### 3.2 **数据类型：**

**简单整数字段：**
.proto: `int32 age = 1;`
.pb.h:`int32_t age;`

**长度未知的字符串：**
.proto：`string name = 1;`
.pb.h：`pb_callback_t name;`

**已知最大长度的字符串：**
.proto: `string name = 1 [(nanopb).max_length = 40];`
.pb.h:`char name[41];`

**计数未知的重复字符串：**
.proto: `repeated string names = 1;`
.pb.h:`pb_callback_t names;`

**已知最大计数和大小的重复字符串：**
.proto: `repeated string names = 1 [(nanopb).max_length = 40, (nanopb).max_count = 5];`
.pb.h:`size_t names_count;` `char names[5][41];`

**具有已知最大大小的字节字段：**
.proto: `bytes data = 1 [(nanopb).max_size = 16];`
.pb.h: `PB_BYTES_ARRAY_T(16) data;`，其中结构包含`{pb_size_t size; pb_byte_t bytes[n];}`

**固定长度的字节字段：**
.proto: `bytes data = 1 [(nanopb).max_size = 16, (nanopb).fixed_length = true];`
.pb.h:`pb_byte_t data[16];`

**已知最大大小的重复整数数组：**
.proto：`repeated int32 numbers = 1 [(nanopb).max_count = 5];`
.pb.h：`pb_size_t numbers_count;` `int32_t numbers[5];`

**具有固定计数的重复整数数组：**
.proto: `repeated int32 numbers = 1 [(nanopb).max_count = 5, (nanopb).fixed_count = true];`
.pb.h:`int32_t numbers[5];`

在运行时检查最大长度。如果字符串/字节/数组超过分配的长度，`pb_decode()`将返回 false。

> **注意：**对于`bytes`数据类型，字段长度检查可能不准确。编译器可能会向`pb_bytes_t`结构中添加一些填充，而 nanopb 运行时不知道填充了多少结构大小。因此它使用整个结构的长度来存储数据，这不是很聪明，但应该不会引起问题。实际上，这意味着如果您`(nanopb).max_size=5`在一个`bytes`字段上指定，您可能能够在那里存储 6 个字节。对于`string`字段类型，长度限制是准确的。

> **注意：**解码器一次只跟踪一个`fixed_count`重复的字段。通常这不是问题，因为重复字段的所有元素都是端到端的。几个`fixed_count`重复字段的交错数组元素将是有效的 protobuf 消息，但会被 nanopb 解码器拒绝并显示错误`"wrong size for fixed count field"`。



## 参考教程
- [nanopb关于string类型的处理](https://www.cnblogs.com/smartlife/articles/12443908.html)





## 编码教程

> 以下内容转载自：[StackOverflow](https://stackoverflow.com/questions/57569586/how-to-encode-a-string-when-it-is-a-pb-callback-t-type)

The `pb_callback_t` struct contains a `void* arg` field which you can use to pass any custom data to the encode/decode function through the `arg` parameter.

In this case you could do:

```csharp
int main()
{
    ... 
    featurefile.features.Id.arg = "something";
    featurefile.features.Id.funcs.encode = &encode_string;
}
```

And note that the `arg` parameter is a *pointer* to `void * const`, so you will always have to dereference it:

```cpp
bool encode_string(pb_ostream_t* stream, const pb_field_t* field, void* const* arg)
{
    const char* str = (const char*)(*arg);

    if (!pb_encode_tag_for_field(stream, field))
        return false;

    return pb_encode_string(stream, (uint8_t*)str, strlen(str));
}
```

Note that you can pass a pointer to any struct, i.e. you can easily create a sort of a "parsing context" struct and pass it around so that you don't need to care how the parsing func will use it.

In this case, it could be something like:

```cpp
typedef struct
{
    const char * something;
    const char * whatever;
    ...
}
callback_context_t;

int main()
{
    callback_context_t ctx = { .something = "something" };

    // this way you always pass the same pointer type
    featurefile.features.Id.arg = &ctx;
    featurefile.features.Id.funcs.encode = &encode_string;
}

bool encode_string(pb_ostream_t* stream, const pb_field_t* field, void* const* arg)
{
    // ...and you always cast to the same pointer type, reducing
    // the chance of mistakes
    callback_context_t * ctx = (callback_context_t *)(*arg);

    if (!pb_encode_tag_for_field(stream, field))
        return false;

    return pb_encode_string(stream, (uint8_t*)ctx->something, strlen(ctx->something));
}
```
