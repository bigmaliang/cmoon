<?php

$m = mevent_init_plugin('mgbench');
mevent_add_str($m,'dns','epiml.cm');
mevent_add_int($m,'query.integral','0');
echo $r = mevent_trigger($m, "", 1003, 2);

$b = mevent_result($m);
print_r($b);
mevent_free($m);
