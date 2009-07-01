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
	   uri varchar(1024) NOT NULL DEFAULT '/',	--consider as file's absolute path
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
	   stat int NOT NULL DEFAULT 0, -- lnum.h
	   intime timestamp DEFAULT now(),
	   uptime timestamp DEFAULT now(),
	   PRIMARY KEY (uid, gid)
);

CREATE INDEX file_index ON fileinfo (pid, uid, gid, mode, name);
--done in after_file_insert()
CREATE TRIGGER tg_uptime_file BEFORE UPDATE ON fileinfo FOR EACH ROW EXECUTE PROCEDURE update_time();
CREATE TRIGGER tg_uptime_group BEFORE UPDATE ON groupinfo FOR EACH ROW EXECUTE PROCEDURE update_time();

CREATE OR REPLACE FUNCTION after_file_insert() RETURNS TRIGGER AS $$
BEGIN
	NEW.uri := (SELECT uri FROM fileinfo WHERE id=NEW.pid;) || '/' || NEW.name;
	NEW.gid := NEW.id;
	
	IF NEW.pid = 1 THEN
		NEW.dataer := NEW.name
	ELSE
		NEW.dataer := (SELECT dataer FROM fileinfo WHERE id=pid;) || '_' || NEW.name;
		--NEW.dataer := SPLIT_PART(NEW.dataer, '_', 1) || '_' || SPLIT_PART(NEW.dataer, '_', 2)
		NEW.dataer := SUBSTRING(NEW.dataer FROM '^[^_].*_[^_].*');
	END IF;
	-- render is chosed by user
	
	INSERT INTO groupinfo (uid, gid, mode) VALUES (NEW.uid, NEW.gid, 255);
	RETURN NEW;
END;
$$ LANGUAGE plpgsql;
-- after insert return NEW make no sense, so update by hand
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

		INSERT INTO groupinfo (uid, gid, mode) VALUES (NEW.uid, NEW.id, 255);
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






-- file rule deprecated, use file triggle instead
CREATE RULE filerule AS ON INSERT TO fileinfo
	DO ( UPDATE fileinfo SET uri= (SELECT uri FROM fileinfo WHERE id=NEW.pid) || '/' || NEW.name WHERE id=NEW.id;
	INSERT INTO groupinfo (uid, gid, mode) VALUES (NEW.uid, NEW.id, 255); );

