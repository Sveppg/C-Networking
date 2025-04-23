CC= g++ -o
CXXFLAGS= -Wall -W -Werror -std=c++17

gethosts: gethosts.cpp

lilnbigendian: lilnbigendian.cpp

clean: 
	$(RM) *.o lilnbigendian gethosts
