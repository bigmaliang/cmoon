-- mn_tjt (the joshua tree)
-- csc's id=5, and all of it's childs's aid is 5 too, so, we put all csc's tjt item in tjt_5
-- you can create tjt_x for any other tjt_like item as you wish
CREATE FUNCTION update_time() RETURNS TRIGGER AS $$ BEGIN NEW.uptime=now(); RETURN NEW; END; $$ LANGUAGE plpgsql;

CREATE TABLE tjt_5 (
	   id SERIAL,
       aid int NOT NULL DEFAULT 1, --anchor file's id
       fid int NOT NULL DEFAULT 1, --which file this item belongs to
	   uid int NOT NULL DEFAULT 0, --who created this item
	   img varchar(256) NOT NULL DEFAULT '', --image file name, without path
	   exp text NOT NULL DEFAULT '', --explanation text, present by <pre></pre>
	   intime timestamp DEFAULT now(),
	   uptime timestamp DEFAULT now(),
	   PRIMARY KEY (id)
);
CREATE TRIGGER tg_uptime_tjt_5 BEFORE UPDATE ON tjt_5 FOR EACH ROW EXECUTE PROCEDURE update_time();
