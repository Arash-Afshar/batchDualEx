 #! /bin/bash


name=$1
n=$2
ddd=$3

lg=`echo "a=l(${n})/l(2); scale=0; a/1" | bc -l`
hg=`echo "a=l(${n})/l(2); scale=0; (a/1)+1" | bc -l`

logFile="/tmp/frigate.log"
cat /dev/null > ${logFile}
function create(){
    circ=$1
    cat ${circ}.base | sed "s/QQQ/${n}/g" | sed "s/LGQ/${lg}/g" | sed "s/HQQ/${hg}/g" | sed "s/DDD/${ddd}/g" > ${circ}.wir
    /media/linuxstorage/git/frigaterelease/src/frigate "${circ}".wir -i -i_output "${circ}".out >> ${logFile}; java -ea  -cp .. FrigateConvert . "${circ}" "-${n}-${ddd}" "nigel"
}

create ${name}
