#!/bin/sh

cd /sys/class/atm/ || exit 1

for PORT in solos-pci* ; do

    RXRATE=`cat $PORT/parameters/RxBitRate`
    TXRATE=`cat $PORT/parameters/TxBitRate`
    RXSNR=`cat $PORT/parameters/LocalSNRMargin | sed "s/ dB//"`
    TXSNR=`cat $PORT/parameters/RemoteSNRMargin | sed "s/ dB//"`
    RXERR=`cat $PORT/parameters/RSUnCorrectedErrorsDn`
    TXERR=`cat $PORT/parameters/RSUnCorrectedErrorsUp`
    RXFEC=`cat $PORT/parameters/RSCorrectedErrorsDn`
    TXFEC=`cat $PORT/parameters/RSCorrectedErrorsUp`

    echo "$RXRATE $RXSNR $RXERR $RXFEC / $TXRATE $TXSNR $TXERR $TXFEC" |
       logger -t $PORT
done

