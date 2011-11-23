<?php

$m = mevent_init_plugin('search');
mevent_add_str($m,'type','1');
mevent_add_str($m,'q','GG');
echo $r = mevent_trigger($m, "", 1001, 2);

$b = mevent_result($m);
print_r($b);
mevent_free($m);
