/************
*  airkiss  *
*************/
1: 在手机装上 com.example.airkissdemo-v0.7.1.apk(android 4.3 以上)

2: 手机连上 wifi 并启动应用。显示第一项为当前wifi名 ，第二项为wifi密钥 ，第三项为airkiss匹配密钥。默认airkiss密钥为"0123456789123456"，
   可以通过更改ramdisk 根目录 sbin下的 startsta.sh 中的/sbin/akiss -k 后面的参数进行配置。

3: 启动开发板 点击发送 在串口输出 "state: 2(listening)" 后点击发送。

4: 在应用显示通知bingo之后即联网完成，可以通过ping www.baidu.com或者网关来检测。

5: 在wifi信号比较好的地方进行测试（建议三米以内），由于目前wifi频偏还没有校准，如果链接不上的话可以尝试更改路由的信道。

