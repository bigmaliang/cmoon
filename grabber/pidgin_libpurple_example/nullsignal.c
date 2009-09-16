#include <glib.h>
#include "purple.h"
#include "nullsignal.h"

#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

static void signed_on(PurpleConnection *gc, gpointer null)
{
	PurpleAccount *account = purple_connection_get_account(gc);
	printf("Account connected: %s %s\n", account->username, account->protocol_id);
}

void null_signals_connect(void)
{
	static int handle;
	purple_signal_connect(purple_connections_get_handle(), "signed-on", &handle,
                          PURPLE_CALLBACK(signed_on), NULL);
}
