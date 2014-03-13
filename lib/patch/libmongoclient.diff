diff --git a/src/bson.c b/src/bson.c
index f73bbc9..f91c3f3 100644
--- a/src/bson.c
+++ b/src/bson.c
@@ -270,7 +270,7 @@ bson_new_from_data (const guint8 *data, gint32 size)
 {
   bson *b;
 
-  if (!data || size <= 0)
+  if (!data || size <= 0 || size > MAX_DATA_LEN)
     return NULL;
 
   b = g_new0 (bson, 1);
diff --git a/src/libmongo-macros.h b/src/libmongo-macros.h
index 644fbe8..9ca6f6b 100644
--- a/src/libmongo-macros.h
+++ b/src/libmongo-macros.h
@@ -19,6 +19,9 @@
 
 #include <glib.h>
 
+//#define MAX_DATA_LEN 32505856
+#define MAX_DATA_LEN 3290287
+
 inline static gdouble
 GDOUBLE_SWAP_LE_BE(gdouble in)
 {
diff --git a/src/mongo-client.c b/src/mongo-client.c
index 0226621..e8a1a82 100644
--- a/src/mongo-client.c
+++ b/src/mongo-client.c
@@ -23,6 +23,7 @@
 #include "bson.h"
 #include "mongo-wire.h"
 #include "libmongo-private.h"
+#include "libmongo-macros.h"
 
 #include <glib.h>
 
@@ -266,6 +267,7 @@ mongo_packet_recv (mongo_connection *conn)
     }
 
   size = h.length - sizeof (mongo_packet_header);
+  if (size < 0 || size > MAX_DATA_LEN) return NULL;
   data = g_new0 (guint8, size);
   if ((guint32)recv (conn->fd, data, size, MSG_NOSIGNAL | MSG_WAITALL) != size)
     {
diff --git a/src/mongo-sync.c b/src/mongo-sync.c
index 57110f3..901edd7 100644
--- a/src/mongo-sync.c
+++ b/src/mongo-sync.c
@@ -506,6 +506,7 @@ _mongo_sync_packet_recv (mongo_sync_connection *conn, gint32 rid, gint32 flags)
   mongo_packet_header h;
   mongo_reply_packet_header rh;
 
+rerecv:
   p = mongo_packet_recv ((mongo_connection *)conn);
   if (!p)
     return NULL;
@@ -522,6 +523,8 @@ _mongo_sync_packet_recv (mongo_sync_connection *conn, gint32 rid, gint32 flags)
   if (h.resp_to != rid)
     {
       mongo_wire_packet_free (p);
+      if (h.resp_to < rid) goto rerecv;
+
       errno = EPROTO;
       return NULL;
     }
