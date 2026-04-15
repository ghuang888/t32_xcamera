wsdl2h -o out/onvif.h -c -s -t ./typemap.dat  wsdl/remotediscovery.wsdl  wsdl/devicemgmt.wsdl  wsdl/media.wsdl
#sed -r -i  's/d\.h/wsdd10.h/' gen/onvif.h
soapcpp2 -2 -c -S  out/onvif.h -x -I .\custom -I .\import -I .\import -dout

