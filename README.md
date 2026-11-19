# SoyoCC

Nick 学习编译原理的产物，基本是 [chibicc](https://github.com/rui314/chibicc) 的翻版。

## 编译

soyocc 使用 [nob.h](https://github.com/tsoding/nob.h) 作为构建系统

```bash
$ cc -o nob nob.c && ./nob -h
Usage: ./nob [-h | --help] [-d | --debug]
  这个程序被用于便捷地编译 soyocc 以及编译运行其测试
  -h, --help    显示帮助信息
  -d, --debug   编译 soyocc 二进制时附加调试信息

子命令:
  run           编译 soyocc 并运行
  test          编译 soyocc 及其测试并运行测试
```

只需要编译一次，后续向 `nob.c` 写入更改后，运行 `./nob` 会自动重编译自身。

## 为什么叫 SoyoCC

![soyo](.github/readme/soyo.jpg)

## [为什么学编译](https://pku-minic.github.io/online-doc/#/misc-app-ref/why)

> TODO: 待补充.

## 参考资料

- chibicc: <https://github.com/rui314/chibicc>
- chibicc 作者的书：<https://www.sigbus.info/compilerbook>
- MaxXing 的北大编译实践 lab: <https://pku-minic.github.io/online-doc/#/>
- 如何避免所有内存泄漏 | Tsoding: <https://www.bilibili.com/video/BV1e2UgBrEPY/>
- <https://github.com/tsoding/nob.h>
- <https://satgo1546.github.io/archives/pku-minic/>
- <https://godbolt.org/>

## 67
