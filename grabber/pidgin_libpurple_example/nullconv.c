#include <glib.h>
#include "purple.h"
#include "nullconv.h"

#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

static void null_write_conv(PurpleConversation *conv, const char *who, const char *alias,
                            const char *message, PurpleMessageFlags flags, time_t mtime)
{
	const char *name;
	if (alias && *alias)
		name = alias;
	else if (who && *who)
		name = who;
	else
		name = NULL;

	printf("(%s) %s %s: %s\n", purple_conversation_get_name(conv),
           purple_utf8_strftime("(%H:%M:%S)", localtime(&mtime)),
           name, message);
}

static PurpleConversationUiOps null_conv_uiops = {
    NULL,                      /* create_conversation  */
    NULL,                      /* destroy_conversation */
    NULL,                      /* write_chat           */
    NULL,                      /* write_im             */
    null_write_conv,           /* write_conv           */
    NULL,                      /* chat_add_users       */
    NULL,                      /* chat_rename_user     */
    NULL,                      /* chat_remove_users    */
    NULL,                      /* chat_update_user     */
    NULL,                      /* present              */
    NULL,                      /* has_focus            */
    NULL,                      /* custom_smiley_add    */
    NULL,                      /* custom_smiley_write  */
    NULL,                      /* custom_smiley_close  */
    NULL,                      /* send_confirm         */
    NULL,
    NULL,
    NULL,
    NULL
};

PurpleConversationUiOps* null_conv_get_ui_ops()
{
    return &null_conv_uiops;
}
