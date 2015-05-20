#!/bin/bash
#
# You need a new version cparser (with jna backend) and firm headers
set -eu

. ./config

for i in oo nodes rta ; do
	RES="../src/firm/bindings/binding_$i.java"
	TMP="/tmp/tmp.java"
	TMP2="/tmp/tmp2.java"
	OO_INC="../include"
	echo " * Creating $RES"
	CMD="cparser --print-jna --jna-libname oo -I${FIRM_INC} -I${FIRM_INC2} -I${OO_INC} ${OO_INC}/liboo/$i.h --jna-limit ${OO_INC}/liboo/$i.h"
	echo "$CMD"
	$CMD > $TMP || exit $?
	sed -e "s/class binding/class binding_$i/g" -i $TMP
	echo "package firm.bindings;" > $TMP2
	echo "" >> $TMP2
	cat $TMP2 $TMP > $RES
done

