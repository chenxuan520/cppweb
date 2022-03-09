openssl genrsa -des3 -out privkey.pem 2048 
openssl req -new -key privkey.pem -out cert.csr 
openssl req -new -x509 -key privkey.pem -out cacert.pem -days 1095   
