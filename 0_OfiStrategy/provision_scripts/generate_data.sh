#!/bin/bash

# We use a downloader/parser for IEX pcap data and specify directory to download to
python3 /vagrant/iexdownloaderparser/src/download_iex_pcaps.py --start-date 2021-11-05 --end-date 2021-11-05 --download-dir /vagrant/iexdownloaderparser/data/iex_downloads

# Conditional statements to check if various versions of python are installed in different locations on the system

if [ -e /usr/bin/python3 ]; then
	PYTHON_INTERP=python3
fi

if [ -e /usr/bin/pypy3.7 ]; then
	PYTHON_INTERP=pypy3.7
fi

if [ -e /var/lib/snapd/snap/bin/pypy3 ]; then
	PYTHON_INTERP=pypy3
fi

if [ -e /snap/bin/pypy3 ]; then
	PYTHON_INTERP=pypy3
fi

# All the stock symbols we want to download data for
SYMBOLS="DIA,UNH,GS,HD,MSFT,CRM,MCD,HON,BA,V,AMGN,CAT,MMM,NKE,AXP,DIS,JPM,JNJ,TRV,AAPL,WMT,PG,IBM,CVX,MRK,DOW,CSCO,KO,VZ,INTC,WBA,KD"

# Loop iterating over files that match the specified pattern
for pcap in $(ls /vagrant/iexdownloaderparser/data/iex_downloads/DEEP/*gz);
do
	pcap_date=$(echo $pcap | sed -r 's/.*data_feeds_(.*)_(.*)_IEXTP.*/\1/')
	echo "PCAP_FILE=$pcap PCAP_DATE=$pcap_date"
	#gunzip -d -c $pcap | tcpdump -r - -w - -s 0 | $PYTHON_INTERP src/parse_iex_pcap.py /dev/stdin --symbols SPY
	cd /vagrant/iexdownloaderparser/ ; gunzip -d -c $pcap | tcpdump -r - -w - -s 0 | $PYTHON_INTERP /vagrant/iexdownloaderparser/src/parse_iex_pcap.py /dev/stdin --symbols $SYMBOLS --trade-date $pcap_date --output-deep-books-too
	# gunzip decompresses the pcap file ($pcap) and sends the output to stdout (/dev/stdout).
	# tcpdump reads the decompressed data from stdin (-r -), captures the packets, and writes them to stdout (-w -). The -s 0 flag sets the snap length to capture the entire packet.

done;
