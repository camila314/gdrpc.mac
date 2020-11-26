main:
	colorgcc rpc.cpp discord-rpc/lib/libdiscord-rpc.a -std=c++11 -lstdc++ -lc++ -lgdstdlib -lGDML -g -dynamiclib -I/usr/local/include/cocos2dx -I/usr/local/include/cocos2dext -o gdrpc.dylib -lSystem -static -framework Foundation -framework CoreServices 

inj:
	sudo osxinj "Geometry Dash" gdrpc.dylib