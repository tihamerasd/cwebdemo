drop table if exists posts;
	create table posts (
	id integer primary key autoincrement,
	tittle text not null,
	category text not null,
	content text not null
);
