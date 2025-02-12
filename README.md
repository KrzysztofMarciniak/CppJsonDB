```bash
make -j8
```

ex. startup command:
```bash
.login krzysztof password// .use baza// select name FROM users
```
wynik:
```bash
[krzysztofmarciniak@mainframe:~/Documents/git/dbms]$ ./dbms .login krzysztof password// .use baza// select name FROM users
Logged in as: krzysztof
Switched to database: baza
Table: users
+----------------+
|name            |
+----------------+
|"nfenfun"       |
|"mfimfeimfemf"  |
|"fjefeifmeim"   |
+----------------+
```
