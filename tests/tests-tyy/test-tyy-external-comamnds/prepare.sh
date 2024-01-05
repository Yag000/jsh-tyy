rm -rdf ./*

mkdir -p .data

cat > .data/true.sh <<EOF
#!/bin/bash
exit 0
EOF

cat > .data/false.sh <<EOF
#!/bin/bash
exit 1
EOF

chmod u+x .data/true.sh
chmod u+x .data/false.sh
