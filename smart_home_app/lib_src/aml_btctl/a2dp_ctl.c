#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <gio/gio.h>
#include <glib.h>
#include <dbus/dbus.h>
#include <pthread.h>
#include "a2dp_ctl.h"
#include "aml_log.h"

#define LOG_TAG "A2DP"
//#define INFO(fmt, args...) \
//	LOG(LEVEL_WARN, fmt, ##args)
//printf("[CT][%s] " fmt, __func__, ##args)


#define TRANSPORT_INTERFACE "org.bluez.MediaTransport1"
static char TRANSPORT_OBJECT[128] = {0};

#define PLAYER_INTERFACE "org.bluez.MediaPlayer1"
static char PLAYER_OBJECT[128] = {0};

#define CONTROL_INTERFACE "org.bluez.MediaControl1"
//static char CONTROL_OBJECT[128] = {0};

static GMainLoop *main_loop;
static pthread_t thread_id;
static GDBusConnection *conn;
static gboolean A2dpConnected = FALSE;
static gboolean initflag = FALSE;
static a2dp_connect_status_cb connect_status_cb = NULL;
static a2dp_play_status_cb  play_status_cb = NULL;


static int call_player_method(char *method);
static int modify_tansport_volume_property(gboolean up);
static void *dbus_thread(void *user_data);
static int call_objManager_method(void);
static void subscribe_signals(void);
static void unsubscribe_signals(void);

gint ifa_signal_handle = 0;
gint ifr_signal_handle = 0;
gint pch_signal_handle = 0;

void a2dp_set_connect_status_callback(a2dp_connect_status_cb callback)
{
	connect_status_cb = callback;
}
void a2dp_set_play_status_callback(a2dp_play_status_cb callback)
{
	play_status_cb = callback;
}
gboolean a2dp_is_connected()
{
	return A2dpConnected;
}
int a2dp_player_init(void)
{
	gchar *address;
	GError *err = NULL;
	if(initflag)
		return 0;
	INFO("\n");
	address = g_dbus_address_get_for_bus_sync(G_BUS_TYPE_SYSTEM, NULL, NULL);
	if (( conn = g_dbus_connection_new_for_address_sync(address,
	             G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT |
	             G_DBUS_CONNECTION_FLAGS_MESSAGE_BUS_CONNECTION,
	             NULL, NULL, &err)) == NULL) {
		INFO("Couldn't obtain D-Bus connection: %s", err->message);
		return -1;
	}

	//check status
	call_objManager_method();
	//monitor
	subscribe_signals();

	if (pthread_create(&thread_id, NULL, dbus_thread, NULL)) {
		INFO("thread create failed\n");
		unsubscribe_signals();
		return -1;
	}
	initflag = TRUE;
	return 0;
}

void a2dp_player_delinit(void)
{
	INFO("\n");
	unsubscribe_signals();
	conn = NULL;

	if (NULL != main_loop) {
		g_main_loop_quit(main_loop);
		main_loop = NULL;
	}
	initflag = FALSE;
}

int a2dp_start(void)
{
	return call_player_method("Play");
}

int a2dp_stop(void)
{
	return call_player_method("Stop");
}

int a2dp_pause(void)
{
	return call_player_method("Pause");
}

int a2dp_next(void)
{
	return call_player_method("Next");
}

int a2dp_previous(void)
{
	return call_player_method("Previous");
}

int a2dp_volume_up()
{
	INFO("\n");
	return modify_tansport_volume_property(TRUE);
}

int a2dp_volume_down()
{
	INFO("\n");
	return modify_tansport_volume_property(FALSE);
}

void connect_call_back(gboolean connected)
{
	if (TRUE == connected) {
		INFO("A2dp Connected\n");
		/*works when a2dp connected*/

	} else {
		INFO("A2dp Disconnected\n");
		/*works when a2dp disconnected*/
	}
	if(connect_status_cb)
		connect_status_cb(connected);
}

void play_call_back(char *status)
{

	/*Possible status: "playing", "stopped", "paused"*/
	if (strcmp("playing", status) == 0) {
		INFO("Media_Player is now playing\n");
		/*works when playing*/

	} else if (strcmp("stopped", status) == 0) {
		INFO("Media_Player stopped\n");
		/*works when stopped*/

	} else if (strcmp("paused", status) == 0) {
		INFO("Media_Player paused\n");
		/*works when paused*/

	}
	if(play_status_cb)
		play_status_cb(status);
}

static int call_player_method(char *method)
{

	GVariant *result;
	GError *error = NULL;
	int ret = -1;
	if (NULL == conn) {
		INFO("No connection!! Please init first\n");
		return ret;
	}

	if (NULL == method) {
		INFO("Invalid args!!\n") ;
		return ret;
	}

	if (FALSE == A2dpConnected) {
		INFO("A2dp not connected yet!\n") ;
		return ret;
	}

	if (NULL == method) {
		INFO("Invalid args!!\n") ;
		return ret;
	}

	INFO("args: %s\n", method);

	result = g_dbus_connection_call_sync(conn,
	                                     "org.bluez",
	                                     PLAYER_OBJECT,
	                                     PLAYER_INTERFACE,
	                                     method,
	                                     NULL,
	                                     NULL,
	                                     G_DBUS_CALL_FLAGS_NONE,
	                                     -1,
	                                     NULL,
	                                     &error);

	if (result == NULL) {
		INFO("Error: %s\n", error->message);
		g_error_free (error);
		return ret;
	} else
		ret = 0;


	g_variant_unref(result);

	return ret;
}

static int modify_tansport_volume_property(gboolean up)
{

	GVariant *result = NULL, *child = NULL, *parameters = NULL;
	int value = 0, temp = 0, ret = -1;
	GError *error = NULL;

	if (NULL == conn) {
		INFO("No connection!! Please init first\n");
		return ret;
	}

	if (FALSE == A2dpConnected) {
		INFO("A2dp not connected yet!\n");
		return ret;
	}

	/*------------------read volume-----------------------------------------------*/
	result = g_dbus_connection_call_sync(conn,
	                                     "org.bluez",
	                                     TRANSPORT_OBJECT,
	                                     "org.freedesktop.DBus.Properties",
	                                     "Get",
	                                     g_variant_new("(ss)", TRANSPORT_INTERFACE, "Volume"),
	                                     NULL,
	                                     G_DBUS_CALL_FLAGS_NONE,
	                                     -1,
	                                     NULL,
	                                     &error);

	if (result == NULL) {
		INFO("Error: %s\n", error->message);
		INFO("volume read failed\n");
		g_error_free (error);
		return ret;
	}

	//INFO("result: %s\n", g_variant_print(result, TRUE));
	//INFO("result type : %s\n", g_variant_get_type_string(result));
	g_variant_get(result, "(v)", &child);
	g_variant_get(child, "q", &value);

	/*-------------------modify value--------------------------------------------*/
	temp = value;

	if (TRUE == up)
		value += 10;
	else
		value -= 10;

	//volume rang from 0~127
	value = value > 127 ? 127 : value;
	value = value > 0   ? value : 0;

	INFO("volume set: %u->%u\n", temp, value);

	g_variant_unref(child);
	child = g_variant_new_uint16(value);
	parameters = g_variant_new("(ssv)",
	                           TRANSPORT_INTERFACE,
	                           "Volume",
	                           child);

	/*------------------set volume-----------------------------------------------*/
	g_variant_unref(result);
	result = g_dbus_connection_call_sync(conn,
	                                     "org.bluez",
	                                     TRANSPORT_OBJECT,
	                                     "org.freedesktop.DBus.Properties",
	                                     "Set",
	                                     parameters,
	                                     NULL,
	                                     G_DBUS_CALL_FLAGS_NONE,
	                                     -1,
	                                     NULL,
	                                     &error);

	if (result == NULL) {
		INFO("Error: %s\n", error->message);
		INFO("volume set failed\n");
		g_error_free (error);
		return ret;
	} else
		ret = 0;

	g_variant_unref(result);

	return ret;
}

static void signal_interfaces_added(GDBusConnection *conn,
                                    const gchar *sender,
                                    const gchar *path,
                                    const gchar *interface,
                                    const gchar *signal,
                                    GVariant *params,
                                    void *userdata)
{
	GVariantIter *interfaces, *interface_content;
	char *object, *interface_name;


	g_variant_get(params, "(oa{sa{sv}})", &object, &interfaces);

	while (g_variant_iter_next(interfaces, "{sa{sv}}", &interface_name, &interface_content)) {
		if (strcmp(interface_name, TRANSPORT_INTERFACE) == 0) {
			memset(TRANSPORT_OBJECT, 0, sizeof(TRANSPORT_OBJECT));
			INFO("Media_Transport registerd: %s\n", object);
			memcpy(TRANSPORT_OBJECT, object, strlen(object));
		} else if (strcmp(interface_name, PLAYER_INTERFACE) == 0) {
			memset(PLAYER_OBJECT, 0, sizeof(PLAYER_OBJECT));
			INFO("Media_Player registerd: %s\n", object);
			memcpy(PLAYER_OBJECT, object, strlen(object));
		} else if (strcmp(interface_name, CONTROL_INTERFACE) == 0) {
			INFO("Media_Control registerd: %s\n", object);
		}

		g_free(interface_name);
		g_variant_iter_free(interface_content);
	}

	g_variant_iter_free(interfaces);
	g_free(object);


}

static void signal_interfaces_removed(GDBusConnection *conn,
                                      const gchar *sender,
                                      const gchar *path,
                                      const gchar *interface,
                                      const gchar *signal,
                                      GVariant *params,
                                      void *userdata)
{
	GVariantIter *interfaces;
	char *object, *interface_name;

	//	INFO("params type : %s\n", g_variant_get_type_string(params));
	//	INFO("params: %s\n", g_variant_print(params, TRUE));

	g_variant_get(params, "(oas)", &object, &interfaces);

	while (g_variant_iter_next(interfaces, "s", &interface_name)) {
		if (strcmp(interface_name, TRANSPORT_INTERFACE) == 0) {
			INFO("Media_Transport unregisterd\n");
			//memset(TRANSPORT_OBJECT, 0, sizeof(TRANSPORT_OBJECT));
		} else if (strcmp(interface_name, PLAYER_INTERFACE) == 0) {
			INFO("Media_Player unregisterd\n");
			//memset(PLAYER_OBJECT, 0, sizeof(PLAYER_OBJECT));
		} else if (strcmp(interface_name, CONTROL_INTERFACE) == 0) {
			//INFO("Media_Control unregisterd\n");
		}

		g_free(interface_name);
	}

	g_variant_iter_free(interfaces);
	g_free(object);

}

static void signal_properties_changed(GDBusConnection *conn,
                                      const gchar *sender,
                                      const gchar *path,
                                      const gchar *interface,
                                      const gchar *signal,
                                      GVariant *params,
                                      void *userdata)
{
	GVariantIter *properties;
	GVariant *value;
	char *property, *interface_name, *status;

	g_variant_get(params, "(sa{sv}as)", &interface_name, &properties);

	//Media_Control properies handler
	if (strcmp(interface_name, CONTROL_INTERFACE) == 0) {
		while (g_variant_iter_next(properties, "{sv}", &property, &value)) {
			if (strcmp(property, "Connected") == 0) {
				g_variant_get(value, "b", &A2dpConnected);
				/*Possible value: TRUE, FALSE*/
				connect_call_back(A2dpConnected);
			}
			g_free(property);
			g_variant_unref(value);
		}
	}

	//Media_Player properies handler
	if (strcmp(interface_name, PLAYER_INTERFACE) == 0) {
		while (g_variant_iter_next(properties, "{sv}", &property, &value)) {
			if (strcmp(property, "Status") == 0) {
				g_variant_get(value, "s", &status);
				play_call_back(status);
			}
			g_free(property);
			g_variant_unref(value);
		}
	}

	g_free(interface_name);
	g_variant_iter_free(properties);
}

static int call_objManager_method(void)
{

	GVariant *result, *value;
	GError *error = NULL;

	GVariantIter *iter1, *iter2, *iter3;
	char *object, *interface_name, *property;
	gboolean status = FALSE;

	int ret = -1;
	if (NULL == conn) {
		INFO("No connection!! Please init first\n");
		return ret;
	}

	result = g_dbus_connection_call_sync(conn,
	                                     "org.bluez",
	                                     "/",
	                                     "org.freedesktop.DBus.ObjectManager",
	                                     "GetManagedObjects",
	                                     NULL,
	                                     NULL,
	                                     G_DBUS_CALL_FLAGS_NONE,
	                                     -1,
	                                     NULL,
	                                     &error);

	if (result == NULL) {
		INFO("Error: %s\n", error->message);
		g_error_free (error);
		return ret;
	} else
		ret = 0;

	g_variant_get(result, "(a{oa{sa{sv}}})", &iter1);
	//g_variant_iter_init(result, &iter1);
	while (g_variant_iter_next(iter1, "{oa{sa{sv}}}", &object, &iter2)) {
		while (g_variant_iter_next(iter2, "{sa{sv}}", &interface_name, &iter3)) {
			//get connected status
			if (strcmp(interface_name, CONTROL_INTERFACE) == 0) {
				while (g_variant_iter_next(iter3, "{sv}", &property, &value)) {
					if (strcmp(property, "Connected") == 0) {
						g_variant_get(value, "b", &status);
						//this is the beginng check, only 'TRUE' would be reported
						if (TRUE == status) {
							connect_call_back(TRUE);
							A2dpConnected = TRUE;
						}
					}
					g_free(property);
					g_variant_unref(value);
				}
				//get object path
			} else if (strcmp(interface_name, TRANSPORT_INTERFACE) == 0) {
				memset(TRANSPORT_OBJECT, 0, sizeof(TRANSPORT_OBJECT));
				memcpy(TRANSPORT_OBJECT, object, strlen(object));
				INFO("Media_Transport registerd: %s\n", object);
			} else if (strcmp(interface_name, PLAYER_INTERFACE) == 0) {
				memset(PLAYER_OBJECT, 0, sizeof(PLAYER_OBJECT));
				memcpy(PLAYER_OBJECT, object, strlen(object));
				INFO("Media_Player registerd: %s\n", object);
			}
			g_free(interface_name);
			g_variant_iter_free(iter3);
		}
		g_free(object);
		g_variant_iter_free(iter2);
	}

	g_variant_iter_free(iter1);
	g_variant_unref(result);
	return ret;
}

static void subscribe_signals(void)
{
	//we monitor interface added here
	ifa_signal_handle = g_dbus_connection_signal_subscribe(conn, "org.bluez", "org.freedesktop.DBus.ObjectManager",
	                    "InterfacesAdded", NULL, NULL, G_DBUS_SIGNAL_FLAGS_NONE,
	                    signal_interfaces_added, NULL, NULL);

	//we monitor interface remove here
	ifr_signal_handle = g_dbus_connection_signal_subscribe(conn, "org.bluez", "org.freedesktop.DBus.ObjectManager",
	                    "InterfacesRemoved", NULL, NULL, G_DBUS_SIGNAL_FLAGS_NONE,
	                    signal_interfaces_removed, NULL, NULL);

	//we monitor propertes changed here
	pch_signal_handle = g_dbus_connection_signal_subscribe(conn, "org.bluez", "org.freedesktop.DBus.Properties",
	                    "PropertiesChanged", NULL, NULL, G_DBUS_SIGNAL_FLAGS_NONE,
	                    signal_properties_changed, NULL, NULL);

}

static void unsubscribe_signals(void)
{
	g_dbus_connection_signal_unsubscribe(conn, ifa_signal_handle);
	g_dbus_connection_signal_unsubscribe(conn, ifr_signal_handle);
	g_dbus_connection_signal_unsubscribe(conn, pch_signal_handle);

}

static void *dbus_thread(void *user_data)
{
	main_loop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(main_loop);
}
/*
//test function
void my_a2dp_connect_status_cb(gboolean connected)
{
	INFO(" a2dp：%s\n", connected ? "connected" : "disconnected");
}
void my_a2dp_play_status_cb(char *status)
{
	INFO("a2dp  play event：%s\n", status);
}

void a2dp_test()
{
	int i;
	a2dp_set_play_status_callback(my_a2dp_play_status_cb);
	a2dp_set_connect_status_callback(my_a2dp_connect_status_cb);

	if (a2dp_player_init())
		return;

	while (!a2dp_is_connected()) {
		sleep(1);
		INFO("waiting for connection\n");
	}

	INFO("Device connected, start playing\n");
	a2dp_start();
	sleep(5);
	INFO("pause\n");
	a2dp_pause();
	sleep(5);
	INFO("next\n");
	a2dp_next();
	sleep(5);
	INFO("previous\n");
	a2dp_previous();
	sleep(5);
	a2dp_stop();
	sleep(5);
	a2dp_start();

	for (i = 0 ; i < 5; i++) {
		a2dp_volume_down();
		sleep(1);
	}

	for (i = 0 ; i < 5; i++) {
		a2dp_volume_up();
		sleep(1);
	}

	a2dp_stop();
	sleep(2);

	a2dp_player_delinit();

}
*/

