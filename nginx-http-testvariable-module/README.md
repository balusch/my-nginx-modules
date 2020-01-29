# Introduction

这个模块是在学习 Nginx 的内部变量时写的，基本上是抄《深入》这本书上的。

## Usage

添加了一个`allow_in`指令来控制是否允许访问，比如`allow $remote_addr 10.69.50.199`，只有当对端地址与目标地址匹配时才允许访问。

```nginx
http {
    server {
        listen       9877;
        server_name  localhost;

        location /myspace {
            allow_in $remote_addr 9.69.50.199;
        }
    }
}
```
