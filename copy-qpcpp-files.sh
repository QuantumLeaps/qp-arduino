for FILE in $(cat ./qpcpp-file-list.txt)
do
    cp ${FILE} ./libraries/qpcpp_arm-cm/src
done
