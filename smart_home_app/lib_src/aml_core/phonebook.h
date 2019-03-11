#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <string.h>

typedef int (* call_fun)(void *, int , char **, char**);
int update_contact(char * Json_string, sqlite3 * db);
void create_table(sqlite3 * db);
void insert_contact(char * name, char * number, char * contactId, char* version, sqlite3 * db);
void modify_contact(char * name, char * number, sqlite3 * db);
void delete_contact(char * name, sqlite3 * db);
void search_contact_number(char * number, sqlite3 * db, call_fun cb);
void search_contact_name(char * name, sqlite3 * db, call_fun cb);
void delete_all_contact(sqlite3 * db);
void search_contact_all(sqlite3 * db, call_fun cb);
