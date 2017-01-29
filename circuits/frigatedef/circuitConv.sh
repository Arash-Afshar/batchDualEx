#! /bin/bash

/media/linuxstorage/git/frigaterelease/src/frigate "$1".wir -i -i_output "$1".out; java -ea -cp . FrigateConvert . "$1" $2
