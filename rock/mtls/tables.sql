--mn_sys

CREATE TABLE fileinfo (
	   id SERIAL,
	   pid int NOT NULL DEFAULT 1,
	   uid int NOT NULL DEFAULT 0,
	   gid int NOT NULL DEFAULT 0,
	   mode int NOT NULL DEFAULT 0,	   -- ofile.h
	   reqtype int NOT NULL DEFAULT 0,	  -- request type
	   lmttype int NOT NULL DEFAULT 0,	  -- action type (need some extra limit to access this file)
	   name varchar(256) NOT NULL DEFAULT '', --file name present in url
	   remark varchar(256) NOT NULL DEFAULT '', --description remark
	   uri varchar(1024) NOT NULL DEFAULT '/',	--consider as file's absolute path
	   dataer varchar(256) NOT NULL DEFAULT '',
	   render varchar(256) NOT NULL DEFAULT '',
	   intime timestamp DEFAULT now(),
	   uptime timestamp DEFAULT now(),
	   PRIMARY KEY (id)
);

CREATE INDEX file_index ON fileinfo (pid, uid, gid, mode, name);
CREATE TRIGGER tg_uptime_file BEFORE UPDATE ON fileinfo FOR EACH ROW EXECUTE PROCEDURE update_time();

CREATE TABLE groupinfo (
	   uid int NOT NULL DEFAULT 0,
	   gid int NOT NULL DEFAULT 0,
	   mode int NOT NULL DEFAULT 0, -- lnum.h
	   PRIMARY KEY (uid, gid)
);

CREATE RULE filerule AS ON INSERT TO fileinfo
	DO ( UPDATE fileinfo SET uri= (SELECT uri FROM fileinfo WHERE id=NEW.pid) || '/' || NEW.name WHERE id=NEW.id;
	INSERT INTO groupinfo (uid, gid, mode) VALUES (NEW.uid, NEW.id, 255); );



CREATE FUNCTION update_file() RETURNS TRIGGER AS $$
BEGIN
	SET NEW.uri= (SELECT uri FROM fileinfo WHERE id=NEW.pid) || '/' || NEW.name;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER tg_upuri_file AFTER INSERT ON fileinfo FOR EACH ROW EXECUTE PROCEDURE update_file();
