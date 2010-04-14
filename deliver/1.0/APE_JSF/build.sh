#!/bin/sh

DIR_CORE=Source
DIR_CLIENT=Clients

FILE_CORE="mootools-core Core/APE Core/Events Core/Core Pipe/Pipe Pipe/PipeProxy Pipe/PipeMulti Pipe/PipeSingle Request/Request Request/Request.Stack Request/Request.CycledStack Transport/Transport.longPolling Transport/Transport.SSE Transport/Transport.XHRStreaming Transport/Transport.JSONP Core/Utility Core/Session"
FILE_CLIENT="JavaScript config"

DST_CORE=Build/mpsCore.js
DST_CLIENT=Build/mpsClient.js

echo -n > $DST_CORE
echo -n > $DST_CLIENT

for file in $FILE_CORE
do
    cat $DIR_CORE/${file}.js >> $DST_CORE
done

for file in $FILE_CLIENT
do
    cat $DIR_CLIENT/${file}.js >> $DST_CLIENT
done
