targ=hmw3

all: $(targ).cc
	export NS_LOG="$(targ)=level_all"
	./ns3 run $(targ) 

dump:
	@./pcapproc.sh
	cat $(targ).tr

%.cc:
	echo begins

grep:
	ls | grep .pcap

clean:
	rm *.pcap

