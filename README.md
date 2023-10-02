
```sqlite
CREATE TABLE password (
id INTEGER PRIMARY KEY AUTOINCREMENT,
site_name TEXT NOT NULL,
site_url TEXT,
username TEXT NOT NULL,
password TEXT NOT NULL,
auth_text TEXT,
recovery_code TEXT,
comment TEXT,
sqltime DATETIME DEFAULT (datetime('now', '+9 hours')),
updatetime DATETIME
);

CREATE TRIGGER update_time_trigger
AFTER UPDATE ON password
FOR EACH ROW
BEGIN
UPDATE password SET updatetime = datetime('now', '+9 hours') WHERE id = OLD.id;
END;
```

