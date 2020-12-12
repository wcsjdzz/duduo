基于 Reactor 模式的 TCP 非阻塞式网络库系统。核心意义是将网络框架和业务代码相分离，让使用者从繁琐的底层网络接口中解脱出来

### 项目结构

- `duduo` : 源码部分，分为 Reactor 框架部分和 Server 部分。Client 部分待开发
- `testCase` : 一些网络库的使用例程

### 特性

- 基于 C++11 语言特性，以 RAII 进行资源管理
- 以`EventLoop`为核心框架，详细解析见[使用 C++实现简单的 Reactor 模式](https://wcsjdzz.github.io/2020/12/06/%E4%BD%BF%E7%94%A8C-%E5%AE%9E%E7%8E%B0%E7%AE%80%E5%8D%95%E7%9A%84Reactor%E6%A8%A1%E5%BC%8F/)
- 支持基于`timer fd`的定时器机制，详细解析见[TimerQueue 定时器](https://wcsjdzz.github.io/2020/12/06/TimerQueue%E2%80%94%E2%80%94%E5%9F%BA%E4%BA%8Etimer-fd%E7%9A%84%E5%AE%9A%E6%97%B6%E5%99%A8%E6%9C%BA%E5%88%B6/)
- 多线程：待开发

### 其他

时间戳部件`TimeStamp`和日志库部件`Logging`使用自`muduo`网络库
