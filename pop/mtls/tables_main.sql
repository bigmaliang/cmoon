CREATE TABLE appinfo (
	   aid int PRIMARY KEY,
	   aname varchar(256) NOT NULL DEFAULT '',
	   state int NOT NULL DEFAULT 0,
	   intime timestamp DEFAULT now(),
	   uptime timestamp DEFAULT now()
);

CREATE INDEX app_index ON appinfo (state);

CREATE TRIGGER tg_uptime_app BEFORE UPDATE ON appinfo FOR EACH ROW EXECUTE PROCEDURE update_time();
