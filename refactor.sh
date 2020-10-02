#!/bin/bash

PPWD="$(cd "$(dirname "$0")" ; pwd)"

TMPD="$(mktemp -d /tmp/$(basename $0).XXXXXX)"
trap cleanup EXIT
cleanup()
{
    rm -rf $TMPD
}

do_search()
{
    cd "$PPWD"
    fgrep -n -r "$1" src testcases 2>/dev/null
}

[ "$#" != "2" ] && echo "expected two arguments, aborting" && exit 1

SEARCH="$1"
REPLACE="$2"

do_search $SEARCH | awk -F':' '{ print $1 }' | sort | uniq | tee $TMPD/files

cat $TMPD/files | while read F ; do
    sed -i "s,$SEARCH,$REPLACE,g" "$PPWD/$F"
done

