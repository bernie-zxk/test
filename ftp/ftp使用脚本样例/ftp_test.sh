#!/bin/bash
        #���ӷ�����
        ftp -v -n 192.168.200.163 << EOF    1
        #�û���d5000   ����  root.123       2 3
        user d5000 root.123
        binary
        hash
        #������Ŀ¼
        cd /home/d5000/tmp                  4
        #����Ŀ¼
        lcd /home/d5000/tmp/test            5
        prompt
        #��ȡ�ļ���    ������mput
        mget test.txt                       6
        bye
        EOF

ʹ�ã� test.sh

        -----------------------------------

        ��������

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


ʹ�ã� test.sh  �û���  ����


��shell�ű��У�ͨ����EOF�� << ���ʹ�ã���ʾ������������Ϊ���������Shell�����룬ֱ������EOFΪֹ���ٷ��ص���Shell��

EOFֻ��һ���ֽ������ȻҲ������abcde�滻��
��shell����<<ʱ����֪����һ������һ���ֽ�����ڸ÷ֽ���Ժ�����ݶ����������룬ֱ��shell�ֿ����÷ֽ��(λ�ڵ�����һ��)��
�˷ֽ����������������κ��ַ�������ʵ����һ��Ҫ��EOF��ֻҪ�ǡ����ݶΡ���û�г��ֵ��ַ������������������EOF����ȫ���Ի���abcde֮����ַ�����ֻ��һ����ʼ�ͽ����ı�־���ˡ�