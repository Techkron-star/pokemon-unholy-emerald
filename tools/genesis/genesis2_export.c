#include "genesis2/world.h"
#include "genesis2/assets.generated.h"
#include <stdio.h>
#include <stdlib.h>
int main(int argc,char **argv)
{
    if(argc!=4)return 2;
    uint32_t seed=(uint32_t)strtoul(argv[1],NULL,0),mid=(uint32_t)strtoul(argv[2],NULL,0);
    FILE *out=fopen(argv[3],"wb");
    if(!out)return 3;
    for(size_t i=0;i<sizeof(sG2Maps)/sizeof(sG2Maps[0]);++i)
        if(sG2Maps[i].mapId==mid)
        {
            const struct G2_Map *m=&sG2Maps[i];
            for(uint32_t k=0;k<(uint32_t)m->width*m->height;++k)
            {
                uint16_t v=G2_RenderCell(m,seed,k);
                if(fputc(v&255,out)==EOF || fputc(v>>8,out)==EOF)return 4;
            }
            return fclose(out)!=0;
        }
    fclose(out);
    return 5;
}
