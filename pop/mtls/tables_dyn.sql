CREATE TABLE lcsjoin (
	   id SERIAL,
	   aid int NOT NULL DEFAULT 0,
	   aname varchar(256) NOT NULL DEFAULT '',
	   uid int NOT NULL DEFAULT 0,
	   uname varchar(256) NOT NULL DEFAULT '',
	   unamea varchar(256) NOT NULL DEFAULT '',
	   ip varchar(256) NOT NULL DEFAULT '',
	   refer varchar(256) NOT NULL DEFAULT '',
	   url varchar(256) NOT NULL DEFAULT '',
	   title varchar(256) NOT NULL DEFAULT '',
	   retcode int NOT NULL DEFAULT 0,
	   intime timestamp DEFAULT now(),
	   uptime timestamp DEFAULT now(),
	   PRIMARY KEY (id)
);

CREATE INDEX app_index ON lcsjoin (aid, uid, retcode);

CREATE TRIGGER tg_uptime_lcsjoin BEFORE UPDATE ON lcsjoin FOR EACH ROW EXECUTE PROCEDURE update_time();



CREATE TABLE visit (
	   id SERIAL,
	   jid int NOT NULL DEFAULT 0,
	   url varchar(256) NOT NULL DEFAULT '',
	   title varchar(256) NOT NULL DEFAULT '',
	   intime timestamp DEFAULT now(),
	   uptime timestamp DEFAULT now(),
	   PRIMARY KEY (id)
);

CREATE INDEX app_index ON visit (jid);

CREATE TRIGGER tg_uptime_visit BEFORE UPDATE ON visit FOR EACH ROW EXECUTE PROCEDURE update_time();



CREATE TABLE counter (
	   id SERIAL,
	   type int NOT NULL DEFAULT 0,
	   count int NOT NULL DEFAULT 0,
	   intime timestamp DEFAULT now(),
	   uptime timestamp DEFAULT now(),
	   PRIMARY KEY (id)
);

CREATE INDEX app_counter ON counter (type, count);

CREATE TRIGGER tg_uptime_counter BEFORE UPDATE ON counter FOR EACH ROW EXECUTE PROCEDURE update_time();
