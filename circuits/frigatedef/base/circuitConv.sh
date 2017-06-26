#! /bin/bash


min=$1
max=$2
ddd=$3
dataFile="gate.dat"
cp /dev/null ${dataFile}

for i in `seq ${min} ${max}`; do
    n=`echo "scale=0; 2^${i}" | bc -l`
    lg=`echo "a=l(${n})/l(2); scale=0; a/1" | bc -l`
    hg=`echo "a=l(${n})/l(2); scale=0; (a/1)+1" | bc -l`
    tmpFile="tmp.dat"
    logFile="compile.log"

    cp /dev/null ${tmpFile}
    cp /dev/null ${logFile}

    function create(){
        circ=$1
        cat ${circ}.base | sed "s/QQQ/${n}/g" | sed "s/LGQ/${lg}/g" | sed "s/HQQ/${hg}/g" | sed "s/DDD/${ddd}/g" > ${circ}.wir
        /media/linuxstorage/git/frigaterelease/src/frigate "${circ}".wir -i -i_output "${circ}".out >> ${logFile}; java -ea -cp .. FrigateConvert . "${circ}" "-${n}-${ddd}" "nigel" "head"

        cat ${circ}.base | sed "s/QQQ/${n}/g" | sed "s/LGQ/${lg}/g" | sed "s/HQQ/${hg}/g" | sed "s/DDD/${lg}/g" > ${circ}.wir
        /media/linuxstorage/git/frigaterelease/src/frigate "${circ}".wir -i -i_output "${circ}".out >> ${logFile}; java -ea -cp .. FrigateConvert . "${circ}" "-${n}-${lg}" "nigel" "head"

        #echo ${circ} `cat ${circ}.circ | wc -l`  `cat ${circ}.circ | grep XOR | wc -l`
        #echo `cat ${circ}.circ | wc -l` >> ${tmpFile}
        #echo `cat ${circ}.circ | grep XOR | wc -l` >> ${tmpFile}
    }

    #create "lookup"
    #create "lookup"
    #create "lookup"
    #create "lookup"
    create "readWrite"
    #create "readWrite"
    #create "readWrite"
    #create "readWrite"
    create "universalInstruction"
    create "evict"
    #create "evict"

    #echo "all" `cat *.circ | wc -l`  `cat *.circ | grep XOR | wc -l`
    #echo `cat *.circ | wc -l` >> ${tmpFile}


    #lookup=`cat lookup.circ | wc -l`
    #readWrite=`cat readWrite.circ | wc -l`
    #ui=`cat universalInstruction.circ | wc -l`
    #evict=`cat evict.circ | wc -l`

    #estiimate=`echo "n=${n}; lg=l(n)/l(2); a=8000 *(lg^3); b=(l(20 * lg)/l(2)); scale=0; a*b/1" | bc -l`
    #total=`echo "l=${lookup}; rw=${readWrite}; ui=${ui}; evict=${evict}; n=${n}; lg=l(n)/l(2); m=20*lg; total=(4*rw+2*evict+ui)*m; scale=0; total/1" | bc -l`
    #echo "${n}" "${total}" "${estiimate}" >> ${dataFile}
    #echo "estiimate" ${estiimate}

    #echo "${n}" `cat tmp.dat | python -c "import sys; print('\n'.join(' '.join(c) for c in zip(*(l.split() for l in sys.stdin.readlines() if l.strip()))))"` "${estiimate}" >> ${dataFile}

    echo $n

done
