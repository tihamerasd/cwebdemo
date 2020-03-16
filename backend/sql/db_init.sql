drop table if exists posts;
	create table posts (
	id integer primary key autoincrement,
	title_HUN text not null,
	title_EN text not null,
	category text not null,
	content_HUN text not null,
	content_EN text not null,
	created_at date
);
