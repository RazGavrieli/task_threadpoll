# bash script that runs test.bash 1000 times and check if there is a difference in files/file.txt and files/file3.txt
make
mkdir files
for i in {1..100}
do
    echo $RANDOM > files/hugefile.txt
    for j in {1..5}
    do
        head /dev/urandom | LC_ALL=C tr -dc A-Za-z0-9 >> files/hugefile.txt
    done 
    randomKey=$RANDOM
    while [ $randomKey -gt 32 ]
    do
        randomKey=$RANDOM
    done
    echo "testing with key " $randomKey
    ./tester $randomKey -e < files/hugefile.txt > files/hugefile2.txt
    ./tester $randomKey -d < files/hugefile2.txt > files/hugefile3.txt
    DIFF=$(diff files/hugefile.txt files/hugefile3.txt)
    if [ "$DIFF" == "" ] 
    then
        echo "Succsess"
    else
        echo "Fail:" $DIFF 
        exit 1
    fi
done