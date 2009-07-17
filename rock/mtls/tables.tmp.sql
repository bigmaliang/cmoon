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






-- file rule deprecated, use file triggle instead
CREATE RULE filerule AS ON INSERT TO fileinfo
	DO ( UPDATE fileinfo SET uri= (SELECT uri FROM fileinfo WHERE id=NEW.pid) || '/' || NEW.name WHERE id=NEW.id;
	INSERT INTO groupinfo (uid, gid, mode) VALUES (NEW.uid, NEW.id, 255); );
