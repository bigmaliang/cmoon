CREATE TABLE test (
	   id SERIAL,
	   type int NOT NULL DEFAULT 0,
	   count int NOT NULL DEFAULT 0,
	   intime timestamp DEFAULT now(),
	   uptime timestamp DEFAULT now(),
	   PRIMARY KEY (id)
);

CREATE INDEX app_test ON appinfo (type, count);

CREATE TRIGGER tg_uptime_test BEFORE UPDATE ON test FOR EACH ROW EXECUTE PROCEDURE update_time();
