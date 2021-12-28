# 微雪10.3寸墨水屏作为显示器使用


硬件：  树霉派4b, 10.3寸微雪墨水屏

软件:   papertty

## 注意:

papertty使用最新版本
 
使用papertty的vnc功能

修改papertty中的vcom匹配墨水屏的vcom


## 如果vncserver不运行在树霉派

在linux主机上运行vncserver,display=:1

通过
DISPLAY=:1 qiv 1.jpg

方式，在墨水屏展示内容。
