#!/bin/sh
for file in gen_ir.py filters.py spec_util.py templates/gen_irnode.c templates/gen_irnode.h templates/nodes.h; do
	cp -pvu ../../libfirm/scripts/$file $file
done
