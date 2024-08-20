SFML的[Linux环境配置参考](https://www.sfml-dev.org/tutorials/2.5/start-linux.php)

SQLite的[Linux环境配置参考](https://www.runoob.com/sqlite/sqlite-installation.html)

在主目录下执行`make`

在bin目录下执行`server`与`client`可执行文件

注：C++这门课的踩分点导致SFML库非常不适用于进行课程设计，若后人希望参考本份代码，需要进行重构

()内为作者吐槽

重构点1. 将所有控制台交互转换至图形化界面输出(设计的是本地程序,无意义...)

重构点2. 设计Pokemon子类时需要针对不同类型设计虚方法重写(课程设计的工程量是不可能扩展的，无意义...)

总结：这门课不看重工程量，而更在乎踩分点，需要按照要求逐一对照，不在乎是否使用现代C++，最无语的一门课
