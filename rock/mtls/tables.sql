--mn_sys

CREATE TABLE fileinfo (
	   id SERIAL,
	   pid int NOT NULL DEFAULT 1,
	   uid int NOT NULL DEFAULT 0,
	   gid int NOT NULL DEFAULT 0,
	   mode int NOT NULL DEFAULT 0,	   -- ofile.h
	   reqtype int NOT NULL DEFAULT 0,	  -- request type 0 html; 1 ajax
	   lmttype int NOT NULL DEFAULT 0,	  -- action type (need some extra limit to access this file)
	   name varchar(256) NOT NULL DEFAULT '', --file name present in url
	   remark varchar(256) NOT NULL DEFAULT '', --description remark
	   uri varchar(1024) NOT NULL DEFAULT '',	--consider as file's absolute path
	   dataer varchar(256) NOT NULL DEFAULT '',
	   render varchar(256) NOT NULL DEFAULT '',
	   intime timestamp DEFAULT now(),
	   uptime timestamp DEFAULT now(),
	   PRIMARY KEY (id)
);

CREATE TABLE groupinfo (
	   uid int NOT NULL DEFAULT 0,
	   gid int NOT NULL DEFAULT 0,
	   mode int NOT NULL DEFAULT 0, -- lnum.h
	   status int NOT NULL DEFAULT 0, -- lnum.h
	   intime timestamp DEFAULT now(),
	   uptime timestamp DEFAULT now(),
	   PRIMARY KEY (uid, gid)
);

CREATE TABLE accountinfo (
	uin int CHECK (Uin > 998),
	uname varchar(64) NOT NULL DEFAULT '',
	status smallint NOT NULL DEFAULT 0,
	intime timestamp DEFAULT now(),
	uptime timestamp DEFAULT now(),
	PRIMARY KEY (Uin)
);

-- insert root row before trigger create
INSERT INTO fileinfo (pid, uid, mode, reqtype, lmttype, name, remark) VALUES (0, 1001, 1, 0, 0, '/', '首页'); -- can't direct access

CREATE INDEX file_index ON fileinfo (pid, uid, gid, mode, name);
--done in after_file_insert()
CREATE TRIGGER tg_uptime_file BEFORE UPDATE ON fileinfo FOR EACH ROW EXECUTE PROCEDURE update_time();
CREATE TRIGGER tg_uptime_group BEFORE UPDATE ON groupinfo FOR EACH ROW EXECUTE PROCEDURE update_time();

CREATE OR REPLACE FUNCTION after_file_insert() RETURNS TRIGGER AS $file_insert$
	BEGIN
		UPDATE fileinfo SET
		gid = NEW.id,
		uri = (SELECT uri FROM fileinfo WHERE id=NEW.pid) || '/' || NEW.name
		WHERE id=NEW.id;

		IF NEW.pid = 1 THEN
			UPDATE fileinfo SET
			dataer = NEW.name, render = NEW.name WHERE id=NEW.id;
		ELSE
			UPDATE fileinfo SET
			--dataer = SUBSTRING((SELECT dataer FROM fileinfo WHERE id=NEW.pid) || '_' || NEW.name FROM '^[^_]*_[^_]*')
			dataer = (SELECT dataer FROM fileinfo WHERE id=NEW.pid),
			render = (SELECT render FROM fileinfo WHERE id=NEW.pid)
			WHERE id=NEW.id;
		END IF;

		INSERT INTO groupinfo (uid, gid, mode, status) VALUES (NEW.uid, NEW.id, 255, 4);
		RETURN NULL;
	END;
$file_insert$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION after_file_delete() RETURNS TRIGGER AS $$
	BEGIN
		DELETE FROM groupinfo WHERE gid=OLD.gid;
		RETURN NULL;
	END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER tg_suf_fileinfo_insert AFTER INSERT ON fileinfo FOR EACH ROW EXECUTE PROCEDURE after_file_insert();
CREATE TRIGGER tg_suf_fileinfo_delete AFTER DELETE ON fileinfo FOR EACH ROW EXECUTE PROCEDURE after_file_delete();

