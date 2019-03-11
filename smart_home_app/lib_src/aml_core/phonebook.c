#include "phonebook.h"
#include "cJSON.h"
void create_table(sqlite3 * db) {
	char *sql = NULL;
	char *errmsg = NULL;
	int ret;

	sql = "CREATE TABLE IF NOT EXISTS CONTACT(" \
		   "nickname	TEXT	NOT NULL," \
		   "number		TEXT	NOT NULL," \
		   "contactId		TEXT	NOT NULL," \
		   "version			TEXT    NOT NULL);";

	if(SQLITE_OK != sqlite3_exec(db, sql, NULL, NULL, &errmsg)) {
		printf("create table error: %s\n", errmsg);
		return;
		//exit(1);
	}
}

void insert_contact(char * name, char * number, char * contactId, char * version, sqlite3 * db) {
	char * errmsg = NULL;
	char sql[128];

	sprintf(sql, "INSERT INTO CONTACT VALUES ('%s', '%s', '%s', '%s')",name,  number,contactId,  version);
	if(SQLITE_OK != sqlite3_exec(db, sql, NULL, NULL, &errmsg)) {
		printf("insert record error: %s\n", errmsg);
		return ;
		//exit(1);
	}
}

void search_contact_name(char * name, sqlite3 * db, call_fun cb) {
	char sql[128];
	char * errmsg = NULL;
	const char * data = "name search";

	sprintf(sql, "SELECT * from CONTACT where nickname='%s'", name);
	if(SQLITE_OK != sqlite3_exec(db, sql, cb, (void*)data, &errmsg)) {
		printf("search contact error: %s\n", errmsg);
		return;
		//exit(1);
	}
}

void search_contact_number(char * number, sqlite3 * db, call_fun cb) {
	char sql[128];
	char * errmsg = NULL;
	const char * data = "number search";

	sprintf(sql, "SELECT * from CONTACT where number='%s'", number);
	if(SQLITE_OK != sqlite3_exec(db, sql, cb, (void*)data, &errmsg)) {
		printf("search contact error: %s\n", errmsg);
		return;
		//exit(1);
	}
}

void delete_contact(char * name, sqlite3 * db) {
	char sql[128];
	char * errmsg = NULL;
	sprintf(sql, "DELETE from CONTACT where nickname='%s'", name);
	if(SQLITE_OK != sqlite3_exec(db, sql, NULL, NULL, &errmsg)) {
		printf("delete contact error: %s\n", errmsg);
		return;
		//exit(1);
	}
}

void modify_contact(char * name, char * number, sqlite3 * db) {
	char sql[128];
	char * errmsg = NULL;

	sprintf(sql, "UPDATE CONTACT set number='%s' where nickname='%s'", number, name);

	if(SQLITE_OK != sqlite3_exec(db, sql, NULL, NULL, &errmsg)) {
		printf("update table error: %s\n", errmsg);
		return;
		//exit(1);
	}
}

void delete_all_contact(sqlite3 * db) {
	char * sql;
	char * errmsg = NULL;
	sql = "DELETE from CONTACT";

	if(SQLITE_OK != sqlite3_exec(db, sql, NULL, NULL, &errmsg)) {
		printf("delete contact error: %s\n", errmsg);
		return;
		//exit(1);
	}
}

//for debug or other useful func
void search_contact_all(sqlite3 * db, call_fun cb) {
	char * sql;
	char * errmsg = NULL;
	const char * data = "search all";

	sql =  "SELECT * from CONTACT";
	if(SQLITE_OK != sqlite3_exec(db, sql, cb, (void*)data, &errmsg)) {
		printf("search contact error: %s\n", errmsg);
		return;
		//exit(1);
	}
}
int update_contact(char * Json_string, sqlite3 * db) {
	char *contactId = NULL;
	char *nickname = NULL;
	char *number = NULL;
	char *version = NULL;

	//printf("%s:%d\n",__func__,__LINE__);

	delete_all_contact(db);

	cJSON *jsonroot = cJSON_Parse(Json_string);
	if(jsonroot == NULL)
	{
		printf("jsonroot is NULL!\n");
		return -1;
	}

	cJSON *taskArry=cJSON_GetObjectItem(jsonroot,"persons");//get array
	if(taskArry == NULL)
	{
		printf("taskArry is NULL!\n");
		return -1;
	}

	cJSON *tasklist=taskArry->child;//sub object
	while(tasklist!=NULL) {
		nickname = cJSON_GetObjectItem(tasklist,"nickname")->valuestring;
		if (nickname==NULL)
		{
			printf("nickname NULL!\n");
			return -1;
		}
		if(strcmp(nickname, "")==0)
		{
			printf("get nickname error!\n");
			return -1;
		}
		#if 0
		number = cJSON_GetObjectItem(tasklist,"number")->valuestring;
		if (number==NULL)
		{
			printf("number NULL!\n");
			return -1;
		}
		if(strcmp(number, "")==0)
		{
			printf("get number error!\n");
			return -1;
		}
		#endif
		contactId = cJSON_GetObjectItem(tasklist,"contactId")->valuestring;
		if (contactId==NULL)
		{
			printf("contactId NULL!\n");
			return -1;
		}
		if(strcmp(contactId, "")==0)
		{
			printf("get contactId error!\n");
			return -1;
		}
		version = cJSON_GetObjectItem(tasklist,"version")->valuestring;
		if (version==NULL)
		{
			printf("version NULL!\n");
			return -1;
		}
		if(strcmp(version, "")==0)
		{
			printf("get version error!\n");
			return -1;
		}
		insert_contact(nickname, number, contactId, version, db);
		tasklist=tasklist->next;
	}
	cJSON_Delete(jsonroot);

	printf("update done!\n");
	return 0;
}
