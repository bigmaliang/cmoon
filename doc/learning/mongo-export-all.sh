#!/bin/bash

LOADING=false

usage()
{
    cat << EOF
    usage: $0 [options] dbname

    OPTIONS:
        -h      Show this help.
        -l      Load instead of export
        -u      Mongo username
        -p      Mongo password
        -H      Mongo host string (ex. localhost:27017)
EOF
}

while getopts "hlu:p:H:" opt; do
    MAXOPTIND=$OPTIND

    case $opt in
        h)
            usage
            exit
            ;;
        l)
            LOADING=true
            ;;
        u)
            USERNAME="$OPTARG"
            ;;
        p)
            PASSWORD="$OPTARG"
            ;;
        H)
            HOST="$OPTARG"
            ;;
        \?)
            echo "Invalid option $opt"
            exit 1
            ;;
    esac
done

shift $(($MAXOPTIND-1))

if [ -z "$1" ]; then
    echo "Usage: export-mongo [opts] <dbname>"
    exit 1
fi

DB="$1"
if [ -z "$HOST" ]; then
    CONN="localhost:27017/$DB"
else
    CONN="$HOST/$DB"
fi

ARGS=""
if [ -n "$USERNAME" ]; then
    ARGS="-u $USERNAME"
fi
if [ -n "$PASSWORD" ]; then
    ARGS="$ARGS -p $PASSWORD"
fi

echo "*************************** Mongo Export ************************"
echo "**** Host:      $HOST"
echo "**** Database:  $DB"
echo "**** Username:  $USERNAME"
echo "**** Password:  $PASSWORD"
echo "**** Loading:   $LOADING"
echo "*****************************************************************"

if $LOADING ; then
    echo "Loading into $CONN"
    tar -xzf $DB.tar.gz
    pushd $DB >/dev/null

    for path in *.json; do
        collection=${path%.json}
        echo "Loading into $DB/$collection from $path"
        mongoimport $ARGS -d $DB -c $collection $path
    done

    popd >/dev/null
    rm -rf $DB
else
    DATABASE_COLLECTIONS=$(mongo $CONN $ARGS --quiet --eval 'db.getCollectionNames()' | sed 's/,/ /g')

    mkdir /tmp/$DB
    pushd /tmp/$DB 2>/dev/null

    for collection in $DATABASE_COLLECTIONS; do
        mongoexport --host $HOST $ARGS -db $DB -c $collection --jsonArray -o $collection.json >/dev/null
    done

    pushd /tmp 2>/dev/null
    tar -czf "$DB.tar.gz" $DB 2>/dev/null
    popd 2>/dev/null
    popd 2>/dev/null
    mv /tmp/$DB.tar.gz ./ 2>/dev/null
    rm -rf /tmp/$DB 2>/dev/null
fi
