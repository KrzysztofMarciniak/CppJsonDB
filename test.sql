---login i use
.login krzysztof password; .use baza;


--- 3 tabelki, users, posts i user_posts.
CREATE TABLE users (id int {PRIMARY KEY, AUTO_INCREMENT}, name string, email string {UNIQUE}) posts (id int {PRIMARY KEY, AUTO_INCREMENT}, title string, content string)  user_posts (user_id int {FOREIGN KEY REFERENCES users id}, post_id int {FOREIGN KEY REFERENCES posts id})

--- insert
insert into users (name) values ("imie1");
insert into users (name, email) values ("imie1", "fmeifmef");

--- select
select name FROM users;
SELECT users.id, users.name, posts.title, posts.content FROM users JOIN user_posts ON users.id = user_posts.user_id;
SELECT users.id, users.name, posts.title, posts.content FROM users JOIN user_posts ON users.id = user_posts.user_id JOIN posts ON user_posts.post_id = posts.id;
