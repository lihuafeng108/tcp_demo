server:
	arm-linux-gnueabihf-gcc server.c -std=gnu99 -o server
	
client:
	arm-linux-gnueabihf-gcc client.c -std=gnu99 -o client
	
sends:
	chmod 755 server
	zftp_g2n server /home/root ${ip}
	
sendc:
	chmod 755 client
	zftp_g2n client /home/root ${ip}
	
clean:
	rm -rf server client
	