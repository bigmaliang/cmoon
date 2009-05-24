--mn_sys

CREATE TABLE fileinfo (
	   id SERIAL,
	   pid int NOT NULL DEFAULT 0,
	   uid int NOT NULL DEFAULT 0,
	   gid int NOT NULL DEFAULT 0,
	   mode int NOT NULL DEFAULT 0,
	   name varchar(256) NOT NULL DEFAULT '', --file name present in url
	   remark varchar(256) NOT NULL DEFAULT '', --description remark
	   intime timestamp DEFAULT now(),
	   uptime timestamp DEFAULT now(),
	   PRIMARY KEY (id)
);

CREATE INDEX file_index ON fileinfo (pid, uid, gid, mode, name);
CREATE TRIGGER tg_uptime_file BEFORE UPDATE ON fileinfo FOR EACH ROW EXECUTE PROCEDURE update_time();

CREATE TABLE groupinfo (
	   uid int NULL NULL DEFAULT 0,
	   gid int NULL NULL DEFAULT 0,
	   PRIMARY KEY (uid, gid)
);
