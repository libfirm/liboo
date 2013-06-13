#!/bin/sh
for file in gen_ir.py filters.py spec_util.py templates/gen_irnode.c templates/gen_irnode.h templates/nodes.h; do
	cp -pvu ../../libfirm/scripts/$file $file
done
for file in javagen.py templates/Node.java templates/Nodes.java; do
	cp -pvu ../../jFirm/binding_generator/$file $file
done
