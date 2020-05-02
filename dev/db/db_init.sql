PRAGMA encoding="UTF-8";
drop table if exists posts;
	CREATE TABLE posts (
	id INTEGER PRIMARY KEY autoincrement,
	title_HUN TEXT NOT NULL UNIQUE,
	title_EN TEXT NOT NULL UNIQUE,
	category text not null,
	content_HUN TEXT NOT NULL,
	content_EN TEXT NOT NULL,
	created_at DATE
);
