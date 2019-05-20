# StrFormat

Bela 目前提供了一个类型安全简单的 `StrFormat`, `StrFormat` 基于 C++ 变参模板，使用 `union` 记录参数类型，在解析时按照输入的占位符将其他类型转换为字符串连接在一起，从而实现格式化功能。

支持的类型和响应的占位符如下表所示：

|类型|占位符|备注|
|---|---|---|
|char|`%c`|ASCII 字符，会被提升为 wchar_t|
|unsigned char|`%c`|ASCII 字符，会被提升为 wchar_t|
|wchar_t|`%c`|UTF-16 字符|
|char16_t|`%c`|UTF-16 字符|
|char32_t|`%c`|UTF-32 Unicode 字符，会转变为 UTF-16 字符，这意味着可以使用 emoji Unicode 码点使用 %c 的方式显示 emoji。|
|short|`%d`|16位整型|
|unsigned short|`%d`|16位无符号整型|
|int|`%d`|32位整型|
|unsigned int|`%d`|32位无符号整型|
|long|`%d`|32位整型|
|unsigned long|`%d`|32位无符号整型|
|long long|`%d`|64位整型|
|unsigned long long|`%d`|64位无符号整型|
|float|`%f`|会被提升为 `double`|
|double|`%f`|64位浮点|
|const char *|`%s`|UTF-8 字符串，会被转换成 UTF-16 字符串|
|char *|`%s`|UTF-8 字符串，会被转换成 UTF-16 字符串|
|std::string|`%s`|UTF-8 字符串，会被转换成 UTF-16 字符串|
|std::string_view|`%s`|UTF-8 字符串，会被转换成 UTF-16 字符串|
|const wchar_t *|`%s`|UTF-16 字符串|
|wchar_t *|`%s`|UTF-16 字符串|
|std::wstring|`%s`|UTF-16 字符串|
|std::wstring_view|`%s`|UTF-16 字符串|
|const char16_t *|`%s`|UTF-16 字符串|
|char16_t *|`%s`|UTF-16 字符串|
|std::u16string|`%s`|UTF-16 字符串|
|std::u16string_view|`%s`|UTF-16 字符串|
|void *|`%p`|指针类型，会格式化成 `0xffff00000` 这样的字符串|

如果不格式化 UTF-8 字符串，且拥有固定大小内存缓冲区，可以使用 `StrFormat` 的如下重载，此重载可以轻松的移植到 POSIX 系统并支持异步信号安全:

```c++
template <typename... Args>
ssize_t StrFormat(wchar_t *buf, size_t N, const wchar_t *fmt, Args... args)
```
