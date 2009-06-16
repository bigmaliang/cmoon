--mn_sys

CREATE TABLE fileinfo (
	   id SERIAL,
	   pid int NOT NULL DEFAULT 0,
	   uid int NOT NULL DEFAULT 0,
	   gid int NOT NULL DEFAULT 0,
	   mode int NOT NULL DEFAULT 0,	   -- ofile.h
	   mark int NOT NULL DEFAULT 0,		  -- bit1: 0 html, 1 ajax; bit2: 0 left nav, 1 action
	   name varchar(256) NOT NULL DEFAULT '', --file name present in url
	   remark varchar(256) NOT NULL DEFAULT '', --description remark
	   uri varchar(1024) NOT NULL DEFAULT '/',	--consider as file's absolute path
	   dataname varchar(256) NOT NULL DEFAULT '',
	   rendname varchar(256) NOT NULL DEFAULT '',
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
