#!/bin/bash
chmod 777 skrypt.sh
cc Glodomor0.c -Wall -o g0
cc Glodomor1.c -Wall -o g1
cc Glodomor2.c -Wall -o g2
cc Glodomor3.c -Wall -o g3
cc Glodomor4.c -Wall -o g4
cc watki.c -Wall -pthread -o kolejka
