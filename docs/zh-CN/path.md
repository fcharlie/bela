# Windows Path Utility

## PathCat 函数

`PathCat` 函数借鉴了 `StringCat` 函数，将路径组件连接起来。例子如下：

```c++
  auto p =
      bela::PathCat(L"\\\\?\\C:\\Windows/System32", L"drivers/etc", L"hosts");
  bela::FPrintF(stderr, L"PathCat: %s\n", p);
  auto p2 = bela::PathCat(L"C:\\Windows/System32", L"drivers/../..");
  bela::FPrintF(stderr, L"PathCat: %s\n", p2);
  auto p3 = bela::PathCat(L"Windows/System32", L"drivers/./././.\\.\\etc");
  bela::FPrintF(stderr, L"PathCat: %s\n", p3);
  auto p4 = bela::PathCat(L".", L"test/pathcat/./pathcat_test.exe");
  bela::FPrintF(stderr, L"PathCat: %s\n", p4);
```
`PathCat` 的思路是先将 `UNC` 前缀和盘符记录并去除，然后将所有的参数使用 `PathSpilt` 函数以 Windows 路径分隔符和 Linux 路径分隔符拆分成 `std::wstring_view` 数组，当当前路径元素为 `..` 时，弹出字符串数组一个元素，如果为 `.` 则保持不变，否则将路径元素压入数组。拆分完毕后，遍历数组计算所需缓冲区大小，调整 `std::wstring` 容量，然后进行路径重组。

当第一个参数值为 `.` 时，`PathCat` 将解析第一个路径为当前目录，然后参与解析。

路径解析错误是很多软件的漏洞根源，合理的规范化路径非常有必要，而 `PathCat` 在规范化路径时，使用 C++17/C++20(Span) 的特性，减少内存分配，简化了规范化流程。

## PathExists 函数


## LookupRealPath 函数

## LookupAppExecTarget 函数
