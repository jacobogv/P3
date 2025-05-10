#!/bin/bash

# Establecemos que el código de retorno de un pipeline sea el del último programa con código de retorno
# distinto de cero, o cero si todos devuelven cero.
set -o pipefail

# Put here the program (maybe with path)
GETF0="get_pitch"

# Añadir opciones
LOW_PASS_FILTER=""
MEDIAN_FILTER=""
WINDOW_TYPE="RECT"  # Por defecto

# Procesado de las opciones en la línea de comandos
while [[ "$#" -gt 0 ]]; do
    case "$1" in
        --low-pass-filter) LOW_PASS_FILTER="--low-pass-filter" ;;
        --median-filter) MEDIAN_FILTER="--median-filter" ;;
        --window=*) WINDOW_TYPE="${1#*=}" ;;
        *) echo "Opció desconeguda: $1"; exit 1 ;;
    esac
    shift
done

# Iterar sobre los ficheros .wav
for fwav in pitch_db/train/*.wav; do
    ff0=${fwav/.wav/.f0}
    echo "Executant $GETF0 $fwav $ff0 ----"
    
    CMD="$GETF0 $LOW_PASS_FILTER $MEDIAN_FILTER --window=$WINDOW_TYPE $fwav $ff0"
    echo "Comanda: $CMD"

    $CMD > /dev/null || { 
        echo -e "\nError en $GETF0 $fwav $ff0"; 
        continue
    }
done

pitch_evaluate pitch_db/train/*.f0ref

exit 0