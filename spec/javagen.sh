#!/bin/sh

set -eu

rm -f *.java
python javagen.py oo_nodes_spec.py templates/Node.java -nodes
echo "Create: Nodes.java"
python javagen.py oo_nodes_spec.py templates/Nodes.java > Nodes.java
mv *.java ../src/firm/oo/nodes
