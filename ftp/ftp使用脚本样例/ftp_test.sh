#!/bin/bash
        #连接服务器
        ftp -v -n 192.168.200.163 << EOF    1
        #用户名d5000   密码  root.123       2 3
        user d5000 root.123
        binary
        hash
        #服务器目录
        cd /home/d5000/tmp                  4
        #本地目录
        lcd /home/d5000/tmp/test            5
        prompt
        #获取文件名    发送用mput
        mget test.txt                       6
        bye
        EOF

使用： test.sh

        -----------------------------------

        带参数：

        #!/bin/bash

        ftp -v -n 192.168.200.163 << EOF
        user $1 $2
        binary
        hash
        cd /home/d5000/tmp
        lcd /home/d5000/tmp/test
        prompt
        mget test.txt
        bye
        EOF


使用： test.sh  用户名  密码


在shell脚本中，通常将EOF与 << 结合使用，表示后续的输入作为子命令或子Shell的输入，直到遇到EOF为止，再返回到主Shell。

EOF只是一个分界符，当然也可以用abcde替换。
当shell遇到<<时，它知道下一个词是一个分界符。在该分界符以后的内容都被当作输入，直到shell又看到该分界符(位于单独的一行)。
此分界符可以是所定义的任何字符串，其实，不一定要用EOF，只要是“内容段”中没有出现的字符串，都可以用来替代EOF，完全可以换成abcde之类的字符串，只是一个起始和结束的标志罢了。