#!/bin/bash

if [ "$#" -ne 1 ]; then
    echo "Utilizare: $0 <caracter>"
    exit 1
fi

caracter=$1
contor=0

while IFS= read -r linie || [[ -n "$linie" ]]; do

    if [[ $linie == *"$caracter"* ]] && 
       [[ $linie =~ ^[A-Z] ]] && 
       [[ $linie =~ [?.!]$ ]] && 
       [[ $linie =~ [a-zA-Z0-9\ \?\!\.]* ]] &&
       ! [[ $linie =~ ,\ *si ]]; then
        ((contor++))
    fi
done

echo $contor
