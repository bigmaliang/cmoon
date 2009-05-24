#!/bin/bash
#-- mn_sys
#CREATE TABLE user0_99 (
#       uin int CHECK (Uin > 998),
#       uname varchar(64) NOT NULL DEFAULT "",
#       usn varchar(64) NOT NULL,
#       male boolean DEFAULT true,
#       email varchar(64) NOT NULL DEFAULT "",
#       intime timestamp DEFAULT now(),
#       uptime timestamp DEFAULT now(),
#       PRIMARY KEY (Uin)
#);

# integer type in -2147483648 to +2147483647, min 3 pos, max 9 pos uin
for i in `seq 3 9`;
do
    psql -U mner -d mn_sys -c "DROP TABLE rls_user_$i;";
    psql -U mner -d mn_sys -c "DROP INDEX rls_user_index_$i;";
    psql -U mner -d mn_sys -c "DROP TRIGGER tg_uptime_rls_user_$i;";
done

for i in `seq 0 99`
do
    psql -U mner -d mn_sys -c "DROP TABLE user_$i;";
    psql -U mner -d mn_sys -c "DROP INDEX user_index_$i;";
    psql -U mner -d mn_sys -c "DROP TRIGGER tg_uptime_user_$i;";
done
