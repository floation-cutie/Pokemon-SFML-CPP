对于SFML图形库的学习整体参考SFML开发者撰写的<<SFML GAME DEVELOPMENT BOOK>> 和SFML官方文档
此为一个公开的[网络链接](https://miphworld.ru/wp-content/uploads/Books/%D0%9F%D1%80%D0%BE%D0%B3%D1%80%D0%B0%D0%BC%D0%BC%D0%B8%D1%80%D0%BE%D0%B2%D0%B0%D0%BD%D0%B8%D0%B5/SFML%20Game%20Development.pdf)

**** game loop
To make the movement *frame-independent*, we shouldn't just add a distance vector directly.
Instead, we reuse the *distance = speed * Delta.asTime()*. Obviously, the speed should revelant to how many pixels it travels per second.

* the first way: we actually assign last frame time to the assumed frame processing time.
the problem is causing the frame time badly longer (every frame is unique)
physcis engines expect the delta time to be fixed,otherwise it collide with bugs

* the secend way: fixed time steps aka fixed FPS
that cause the frame "chases" the real time 

* third way: vertical synchronization ==V-sync== 
adapts the rate of graphical update rate to the refresh rate of the monitor(not fixed)


**** Real-time rendering

Some programs use a render-on-demand approach, minimizing the performance cost,which is not natural.
But this approach is actually redundant, as we request new frames all the time.

> The fact that our eyes can't perceive more frames than 30 - 60 FPS.

Now the game always render again and again, independently of what changed since last frame.

*double buffering* defines two virtual screens, one for display and one for draw
> we use the SFML's sf::RenderWindow::display()
> function to put the contents of the back buffer on the screen

and the front buffer previously will be the back buffer to draw on.
So we won't be entrapped into concerning whether the front buffer is destoryed..

*** Resource management

load a texture && display a sprite over and over again

Defining *Resource* and *asset* : 
mostly they are multimedia resources(heavyweight)

> those occupying a lot of memory, operations on them (especially copying) perform slowly

but broadly speaking, those non-multimedia items such as 
scripts that describe the in-game world, menu content, 
or artificial intelligence are also considered resources. 

`bool loadFromFile (const std::string& filename);` 
`bool loadFromMemory()` `bool loadFromStream()` 

****texture

**it is stored as an array of pixels in the graphics card's video memory, not reside in the RAM**
texture represents a graphical image, and the sprite can have multiple mapping to the same texture
while the texture itself is not affected. 
Why we choose to separate the sprite and texture?? 
We have a simple possibility to deal with graphics using `sf::Sprite`,while the heavyweight `sf::Texture` need not be modified.

****Images
stores in the RAM Instead of the video memory,can manipulate single pixels 

==Texture use intermediate Images==

**** Fonts
fonts -> text
**** Shaders
着色器apply effects to rendered objects, building on OpenGL,whose instance use GLSL

****Audio
What is sound effects and background music? 音效 和 背景乐

the former one is appropriate for short segments(explosions, button clicks and so on...)
the latter are designed to handle music themes.They stays longer and require more memory.

*****Sound Buffers
Sound buffer holds an array of 16 bit audio of sample,but can't play audio.

Sound refers to a sound buffer containing the audio samples.

`sf::Sound` use `sf::SoundBuffer`, and during its life time the sound buffers must remain alive.


****Try

decide How long and by whom those resources are used : preload the resources
so we need use a Resource Identifier ,we recommend use enumerator instead of hardcoding strings

`std::map` performed as associative container is the good choice to store Resource

***** RAII design and generalization

*****Error handling

***Different entity design
1. Class hierarchy, requiring virtual inheritance
2. component-based design

relative coordinates相对坐标用于渲染

C++中的explicit关键字用法

指定的构造函数在被explicit修饰后,不能再被隐式调用,即不允许做隐式的类型转换

To describe the drawable property of these entities, one pratical way is to use scene graph and each entity represents each scene node.

to reuse `std::unique_ptr<SceneNode>` quite often, we create a `SceneNode` Class

Why use pointer? For the usage of polymorphism and extension

由于对每个node来说,父节点只有一个,对于每个Node,有类型为ptr的parent域和类型为vector<ptr>的children域


5.9 实现升级功能 实现技能的特化

如何动态引入texture呢?

5.14 解决了智能指针方面的私有继承的问题,

how to give the player the power to manipulate the world?

5.16 实现了玩家对象的移动,以及所有键盘输入对游戏画面的影响
发现输入,接收事件,确认事件类型,放入事件队列 
事件有四大类:
Window --> 关闭,聚焦..
Joystick --> 摇杆
Keyboard --> 键盘 KeyEvent 数据结构, code域... event.key.code, 以及输入框...
Mouse

同时实时获得输入状态

Trade off: To use events or real-time input? 二选一

处理过程中要避免硬编码 和 将input and logic code in one place

方法: 通过命令和接收器来撰写良好的代码

构造函数对象代替普通的函数指针--> std::function<> action; 

***定义状态 -- Game Flow的迁移
我们可以将状态视为游戏软件中的独立屏幕

VideoState --> title screen --> main menu (some state may work in parallel)
在此,我们将FSM的概念进行扩展,因为游戏过程中可能出现多个状态被同时激活,扩展到state stack

最初在堆栈中register所有可能的状态,通过工厂函数按需创建一个新状态

通过反向迭代器模拟栈的特性:: 正向填入,反向处理,状态堆栈中被实例化的都是活跃状态

绘制过程会先绘制堆栈中的旧状态,这样可以观察到旧状态的屏幕

状态本身可以发送push pop请求, 管理的栈进行实际push pop 操作

****状态转移的延迟操作
向一个正在更新或渲染的状态栈(即正处于迭代中)进行元素的删除和添加,会导致迭代器失效,从而产生矛盾

故而将request信号填入pending队列中

维护状态上下文,作为所有状态之间的共享对象的所有者,避免内存浪费

***扩展游戏--> 应用

构建Application Class 在菜单页面提供用户登录和注册能力

在加载状态下,资源会被导入进屏幕中,但是从硬盘将资源加载到内存上会发生阻塞

故而我们选择加入并行的线程 parallel threading

当多个线程试图访问同一数据时以实现线程之间的通信

我们要保证多个线程轮流对共享数据进行操作,进行**锁**操作

***设计GUI层次结构 hierarchy
sfml借鉴了Java中很多工具包的设计思路,但"without pumping the state full of boilerplate GUI code"

构造基本组件 component 在GUI的命名空间中

进一步实现  容器container <--> 按钮button <--> 标签label

5.17 解决了从this指针扩展到共享指针的资源丢失问题

*** multiPlayer
多人游戏范畴包含 本地合作 和 实际互联网游戏

实际关注的场景为socket编程网络通信

socket本质上是数据的网关

TCP协议保证Packet ordering, Packet Restructuring , Reliability

一方进入监听状态,一方进入连接状态

UDP协议发送数据速度很快

实际编程中往往会在这两个协议的基础上自定义协议

P2P模式架构会导致作弊的严重问题,而C/S架构可以进行避免

5.18 设计网络协议 选择数据库Sqlite3

建表 查看表 插入数据 查询数据 修改数据 删除数据 

内存数据库 进行网络设置 -- 
5.20 处理字符串流的时候修正了流的读取位置在重新赋值时的问题，发现了不同并发事件对键位码值的不同处理

5.21 服务器如何处理多用户登录问题呢?

通过Singleton单例设计模式,确保Server端只有一个实例,也可通过继承SFML的Noncopyable类

使用Server 进行登录和注册的短连接，使用endPoint进行长连接

TIP::在VScode中可以使用#pragma region 和 #endregion 进行预指令中代码块折叠

一些改进思路 --> 将socket和Sqlite3接口设置为RAII

多线程分为监听线程和中断线程

网络通信到达的数据，是否满足数据库内已存储的用户的信息，在登录注册的模块内部实现

上述方法可提高耦合度

用户名和密码的硬性格式要求
1. 不能以_开头
2. 只能是由数字和字母和_组成的字符串

对Sqlite3的查询操作可通过exec统一执行

同时，Sqlite3常用于嵌入式，查询效率较高,业务逻辑使用上偏向于同步接口。

5.21 细节：`sqlite3_get_table()`查询项如果为空，返回值仍为OK,只代表没有相应的匹配的数据行

5.22 endPoint模块提供timer和listen两个线程

通过计时器记录用户是否意外离线，通过监听处理用户操作

5.22 对于Server模块增加monitor资源管理线程，用于监督对应endPoint

有关线程划分的一些原则

何时用detach或join

分离状态的线程通常用于执行一些不需要等待其完成的工作，
如后台任务或短期的独立任务。

结合状态的线程通常用于执行一些需要等待其完成才能继续执行的任务。

Server模块在登录成功后开启monitor线程

修正误区:

C++中的volatile并不是用来解决多线程竞争问题的，
而是用来修饰一些因为程序不可控因素导致变化的变量，
比如访问底层硬件设备的变量，
以提醒编译器不要对该变量的访问擅自进行优化。

多线程编程:
Instead of using `detach()` method to let `monitor` thread observe `endpoint` separately

we can create a global threadPool in c++20 `fthread`

`condition_variable` is used for `timerFunc` thread in the module `endpoint`

5.22 考虑后续添加记录用户首次登录的查找表
5.22 完成服务器的数据库处理，网络接口通信，开始转向客户端

5.23 实现客户端和服务器之间通信，实现登录注册功能,开始界面优化

5.24 完成题目二的所有要求

5.25 需要记录首次登录 和 玩家对战次数 和 玩家获胜次数

方法: 可以选择在第一次注册的时候分发初始化, 也可以在登录时动态分配

5.28 清晰地了解到CPU 与 外设之间的 程序查询 Polling方式，即本项目使用到的`pollevent`

5.29 注意到SQL注入风险，考虑后续扩展 还有四个通信功能需要实现
5.30 考虑玩家移动动画的扩展,  实现了玩家移动时背景视图的变化

6.1 对战过程不需要通信，结果进行通信 进行数据交换即可
在游戏主界面栈上添加战斗界面，战斗界面上添加玩家和对手的信息，胜负结果由socket通信

用户选择自己的精灵也选择对手的精灵，然后进行对战, 对手等级大小由系统决定

当玩家走到World的草坪时，触发对战事件，对战结束后，返回主界面 

草坪的位置是固定的 

6.2 解决了玩家Sprite位置边界动态变化的问题
局部变换矩阵需要在全局变换矩阵的基础上进行变换

当玩家走到草坪时，如何避免重复触发对战事件呢？ 通过设置一个标志位

-[x] 已解决

6.2 实现了对战界面的基本设计，为了布局美观，重构了Button代码，未来考虑整合

借助Category枚举类型，实现玩家移动时的不同朝向动画效果

6.3 Endpoint类玩家信息添加决斗信息

通过按钮信号触发事件, 考虑后续GUI生成时可以通过工厂模式进行生成

6.4 完成了选择对战界面的设计，完成了对战界面的设计

使用了Lambda表达式给HelperClass类中的static静态函数起了别名，方便使用

如何将对战界面的信息传递给对战界面呢？通过msg传递 

注意到了在Lambda表达式中使用shared_ptr指针的问题,如果通过引用传递,则会导致shared_ptr的引用计数不变，
可能在Lambda表达式执行过程中shared_ptr指针被释放，导致程序崩溃

发送了切换界面时，键盘输入没有被释放的问题，导致会进行一次自动攻击

发现了在对战过程中判断胜利条件是在下次攻击时才生效的问题,解决方法，在发送消息前先进行胜利判断

玩家失败时，需要下次尝试攻击的时候才发现自己已经失败 

while循环的时候满足条件后需要break，退出循环

6.5 完成所有设计 进行最后测试阶段

游戏特性: 玩家的精灵id只允许输入数字 玩家可以在对战失败后丢弃任意自己的精灵,但系统会有推荐

解决了丢弃不存在的精灵会游戏崩溃的问题，解决了蓝量属性的问题, 解决了用户强行输入字符串以匹配精灵id的问题

最后添加了用户宠物个数评价和宠物等级评价
