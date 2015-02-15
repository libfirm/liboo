#!/bin/sh

set -eu

FIRM_HOME="../../libfirm"
JFIRM_HOME="../../jFirm"
GENERATOR="python ${FIRM_HOME}/scripts/gen_ir.py -I${JFIRM_HOME}/binding_generator -I. javaspec.py"

#$GENERATOR templates/Node.java
echo "Create: Nodes.java"
$GENERATOR templates/Nodes.java > ../src/firm/oo/nodes/Nodes.java

$GENERATOR templates/nodelist > /tmp/nodelist
for n in $(cat /tmp/nodelist); do
	GOAL="../src/firm/oo/nodes/$n.java"
	echo "GEN $GOAL"
	$GENERATOR -DNODENAME=$n templates/Node.java > $GOAL
done
