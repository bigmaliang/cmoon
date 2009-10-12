#!/bin/bash
#-- mn_sys

#trigger function for uptime update
psql -U mner -d mn_sys -c "CREATE FUNCTION update_time() RETURNS TRIGGER AS $$ BEGIN NEW.uptime=now(); RETURN NEW; END; $$ LANGUAGE plpgsql";
psql -U mner -d mn_csc -c "CREATE FUNCTION update_time() RETURNS TRIGGER AS $$ BEGIN NEW.uptime=now(); RETURN NEW; END; $$ LANGUAGE plpgsql";

for i in `seq 0 99`
do
    psql -U mner -d mn_sys -c "
    CREATE TABLE user_$i (
	uin int CHECK (Uin > 998),
	uname varchar(64) NOT NULL DEFAULT '',
	usn varchar(64) NOT NULL DEFAULT '',
	musn varchar(64) NOT NULL DEFAULT '',
	male boolean DEFAULT true,
	email varchar(64) NOT NULL DEFAULT '',
	intime timestamp DEFAULT now(),
	uptime timestamp DEFAULT now(),
	status smallint NOT NULL DEFAULT 0,
	confirmkey varchar(64) NOT NULL DEFAULT '',
	PRIMARY KEY (Uin)
    );";
    psql -U mner -d mn_sys -c "CREATE INDEX user_index_$i ON user_$i (status);";
    psql -U mner -d mn_sys -c "CREATE TRIGGER tg_uptime_user_$i BEFORE UPDATE ON user_$i FOR EACH ROW EXECUTE PROCEDURE update_time();";
done

# integer type in -2147483648 to +2147483647, min 3 pos, max 9 pos uin
for i in `seq 3 9`
do
    psql -U mner -d mn_sys -c "
    CREATE TABLE rls_user_$i (
	uin int CHECK (Uin > 998),
	uname varchar(64) NOT NULL DEFAULT '',
	usn varchar(64) NOT NULL DEFAULT '',
	musn varchar(64) NOT NULL DEFAULT '',
	male boolean DEFAULT true,
	email varchar(64) NOT NULL DEFAULT '',
	intime timestamp DEFAULT now(),
	uptime timestamp DEFAULT now(),
	status smallint NOT NULL DEFAULT 0,
	confirmkey varchar(64) NOT NULL DEFAULT '',
	PRIMARY KEY (Uin)
    );";
    psql -U mner -d mn_sys -c "CREATE INDEX rls_user_index_$i ON rls_user_$i (status);";
    psql -U mner -d mn_sys -c "CREATE TRIGGER tg_uptime_rls_user_$i BEFORE UPDATE ON rls_user_$i FOR EACH ROW EXECUTE PROCEDURE update_time();";
done

for i in `seq 998 999`; do psql -U mner -d mn_sys -c "INSERT INTO rls_user_3 (uin) VALUES ($i);"; done
for i in `seq 1000 9999`; do psql -U mner -d mn_sys -c "INSERT INTO rls_user_4 (uin) VALUES ($i);"; done
#for i in `seq 10000 99999`; do psql -U mner -d mn_sys -c "INSERT INTO rls_user_5 (uin) VALUES ($i);"; done
#for i in `seq 100000 999999`; do psql -U mner -d mn_sys -c "INSERT INTO rls_user_6 (uin) VALUES ($i);"; done
#for i in `seq 1000000 9999999`; do psql -U mner -d mn_sys -c "INSERT INTO rls_user_7 (uin) VALUES ($i);"; done
#for i in `seq 10000000 99999999`; do psql -U mner -d mn_sys -c "INSERT INTO rls_user_8 (uin) VALUES ($i);"; done
#for i in `seq 100000000 999999999`; do psql -U mner -d mn_sys -c "INSERT INTO rls_user_9 (uin) VALUES ($i);"; done
