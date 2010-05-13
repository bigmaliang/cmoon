<?php




$m = mevent_init_plugin('uic',1001,2);
mevent_add_str($m,'','uin','20');
echo $r = mevent_trigger($m);
 
$b = mevent_result($m);
print_r($b);
mevent_free($m);

