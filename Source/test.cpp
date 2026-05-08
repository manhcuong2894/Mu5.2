#include <iostream>
typedef unsigned char BYTE;
typedef unsigned short WORD;
struct PSBMSG_HEAD { BYTE type; BYTE size; BYTE head; BYTE subh; };

#pragma pack(push, 1)
struct PMSG_CUSTOM_RANKING_RECV { PSBMSG_HEAD header; BYTE type; };
#pragma pack(pop)

int main() {
    std::cout << "Offset of type: " << offsetof(PMSG_CUSTOM_RANKING_RECV, type) << std::endl;
    return 0;
}
