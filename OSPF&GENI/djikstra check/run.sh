g++ ospf.cpp -o ospf
./ospf -f infile.txt -o out -i 0 2> o0.txt 1>&2 &
./ospf -f infile.txt -o out -i 1 2> o1.txt 1>&2 &
./ospf -f infile.txt -o out -i 2 2> o2.txt 1>&2 &
./ospf -f infile.txt -o out -i 3 2> o3.txt 1>&2 &
./ospf -f infile.txt -o out -i 4 2> o4.txt 1>&2 &
./ospf -f infile.txt -o out -i 5 2> o5.txt 1>&2 &
./ospf -f infile.txt -o out -i 6 2> o6.txt 1>&2 



