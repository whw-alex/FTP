# FTP client 运行方式

## 命令行client

如果要连作业完成的```server```，要先进入```server/src/```目录，运行```make```指令，得到```server```可执行文件，再输```./server -root {root} -port {port}```（参数可选、顺序均可，默认值为端口21，根目录```/tmp```）。

之后到```client/src/```目录下，运行```python client.py --no_gui --root {root} --port {port}```指令，必须有```--no_gui```参数，而```--root```参数和```--port```参数需与运行的服务器的信息一样。

## GUI client

到```client/src/```目录下，运行```python client.py ```指令，此时会启动图形界面，在图形界面中输入server的ip，port，用户名与密码，之后点击```connect```，则会自动连接。

![Screenshot 2023-10-19 at 08.12.14](/Users/alex/Library/Application Support/typora-user-images/Screenshot 2023-10-19 at 08.12.14.png)

之后可以点击右侧的按钮进行操作。有的按钮需要进一步输入完整的指令，比如点击```RETR```按钮后还需要输入```RETR  {文件信息}```

![Screenshot 2023-10-15 at 16.21.54](/Users/alex/Library/Application Support/typora-user-images/Screenshot 2023-10-19 at 08.13.01.png)

点击```QUIT```指令，会收到服务器发送的goodbye信息，之后可以点击关闭窗口。

![Screenshot 2023-10-19 at 08.13.29](/Users/alex/Library/Application Support/typora-user-images/Screenshot 2023-10-19 at 08.13.29.png)