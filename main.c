

/*
  Nexus encryption scheme update:

  The encryption method hasn't changed, however the generated key has.
  The key is generated on each packet, see the usage method below.

  Different keys are used for different packets depending on direction
  sent.

  Every packet now has an additional 3 bytes at the end, these 3 bytes
  should not be touched by anything but the encryption setup routines in this
  file and further, they should NOT actually be encrypted! Keep this in mind and
  update the encryption function accordingly as these 3 bytes are accounted
  for in the packet header's size field but should be ignored by the
  actual encryption routine.

  All other aspects of the encryption remain unchanged.

  - Travis

*/

#include <stdio.h>
#include <stdlib.h>
#include "md5calc.h"
#define SWAP16(x) (short)(((x)<<8)|((x)>>8))
/* USAGE:

  On character log in, call populate_table(cl->name, &(cl->tbl), sizeof(cl->tbl));
  where cl is your client structure, name is the name exactly as how the client
  typed it to log in and tbl is a char array with size at least 0x401 bytes.

  On sending a packet to the client (a type that must be encrypted), after the packet
  is filled and has its header attached with correct size, call set_packet_indexes(packetptr).
  Now if the packet is a packet type on the svkey1packets list then you just call your
  encryption like normal using the static key, the value of which is below (key1).

  If the packet is not on that list then you must call
  generate_key2(packetptr, &(cl->tbl), &keyout, 0);
  keyout is a buffer at least 10 bytes long to hold the encryption key.
  Then perform the encryption as normal, using the keyout instead of the static key.


  On receiving an encrypted packet from the client, if the packet type is on the
  clkey1packets list then you can just decrypt it like normal using the static key.

  If the packet is not on the list, then before decryption you must call
  generate_key2(packetptr, &(cl->tbl), &keyout, 1);
  then decrypt using the keyout key instead of the static one.

*/




char key1[] = "Urk#nI7ni";




char table[0x401];

/*
 These packet types must use the static key1 when being encrypted or decrypted.
 svkey1packets are all packet types sent out from server that must encrypt using
 key1.
 clkey1packets are all packet types coming in from client that must be decrypted
 using key1.
 All other encrypted packets not on the lists use the generated key2.
*/
const unsigned char svkey1packets[] = {2,10,68,94,96,98,102,111,125};
const unsigned char clkey1packets[] = {2,3,4,11,21,38,58,66,67,75,80,87,98,113,115,123,125,133};



/* test suite, not needed for encryption */

//official cap

//unsigned char refpacket[] = {0xAA, 0x00, 0x9B, 0x35, 0x91, 0xA9, 0xFB, 0xAB, 0xA9, 0xA8, 0x38, 0xF0, 0x9B, 0x80, 0xDF, 0x9E, 0xCA, 0xC5, 0x89, 0xDD, 0xCD, 0x9C, 0xD2, 0x8B, 0x98, 0xC7, 0xD2, 0x8A, 0xC9, 0xC9, 0x98, 0xC1, 0xCF, 0x86, 0xC9, 0x86, 0x8B, 0xDB, 0xCB, 0x92, 0xC2, 0xDE, 0x96, 0x80, 0xCC, 0xC0, 0xC0, 0xCF, 0x87, 0x84, 0xD8, 0x9A, 0xC4, 0x8C, 0xCF, 0xC8, 0xC0, 0x83, 0xC0, 0xDD, 0xD1, 0xCD, 0xC9, 0x8E, 0xDA, 0xCA, 0x9B, 0xD5, 0x8E, 0x80, 0xC2, 0xDD, 0xDC, 0x8F, 0xC2, 0x90, 0xC4, 0xC4, 0x8C, 0xDF, 0x81, 0xD4, 0xCF, 0x8C, 0x9D, 0xC4, 0xCC, 0xDE, 0xFE, 0xCF, 0xCF, 0xC9, 0xC4, 0xDD, 0xC8, 0xD1, 0x98, 0xCF, 0xD0, 0x82, 0xD1, 0xC1, 0xDE, 0xDE, 0xCA, 0x99, 0xD6, 0x82, 0xCE, 0xC2, 0xD6, 0xDF, 0xC8, 0xCA, 0x96, 0xD8, 0xC9, 0xC1, 0xD0, 0xCD, 0xD8, 0xCD, 0x84, 0x8E, 0xC8, 0xD7, 0xCE, 0x85, 0xE0, 0xD9, 0xC5, 0xC6, 0x8F, 0xCF, 0x87, 0xC1, 0xCF, 0xDC, 0x9F, 0xC0, 0x86, 0x8C, 0xC3, 0xC3, 0xCA, 0x89, 0xA2, 0xF2, 0xE4, 0xD8, 0xAA, 0xDD, 0xD7, 0xDF, 0xB1, 0xD7, 0x01, 0x06};

unsigned char refpacket[] = {0xAA, 0x00, 0x9B, 0x35, 0x86, 0xB7, 0xB2, 0xBA, 0xB6, 0xE0, 0x2F, 0xEA, 0xDF, 0xC2, 0xC1, 0xD7, 0xDB, 0xDA, 0xC1, 0xCA, 0xD7, 0xD8, 0x90, 0x95, 0xD1, 0xD6, 0xCD, 0xC2, 0xDE, 0xD3, 0xDC, 0x83, 0xD1, 0xCF, 0xD8, 0x99, 0xC3, 0xCC, 0xD1, 0xD6, 0x80, 0xC0, 0xDF, 0x91, 0xD3, 0x88, 0xD7, 0xD5, 0xC3, 0xC6, 0xC6, 0xD3, 0xD5, 0x93, 0x87, 0xDF, 0xDA, 0xC7, 0x82, 0xC3, 0x98, 0xDC, 0xD6, 0xC6, 0xCD, 0xD0, 0xDF, 0x97, 0x90, 0xC9, 0xD3, 0xC2, 0x94, 0x98, 0xD8, 0xD4, 0x86, 0xDA, 0xC5, 0xCE, 0x9E, 0x9C, 0xD8, 0x96, 0xD9, 0x86, 0xD2, 0x97, 0xEF, 0xD0, 0x87, 0xDE, 0xDE, 0x99, 0x8A, 0xCF, 0xD1, 0xDE, 0xCF, 0xCA, 0xC6, 0xDB, 0x9A, 0x9C, 0xD4, 0xD0, 0xC7, 0x9D, 0x86, 0xD5, 0xCC, 0x9B, 0x8A, 0xD4, 0xDF, 0xC9, 0xD6, 0x89, 0xC7, 0xD7, 0x9C, 0x8F, 0x9A, 0xC7, 0xD9, 0xC8, 0x86, 0x92, 0xFA, 0x9D, 0x87, 0xD8, 0xC6, 0xDE, 0x98, 0x89, 0xD8, 0xC6, 0xDB, 0x82, 0x98, 0xC5, 0xD2, 0xDC, 0x82, 0x9E, 0xB8, 0xB6, 0xA6, 0xC6, 0xE3, 0xCC, 0xC8, 0x97, 0xA6, 0xED, 0xDF, 0x4C};



char refkeysv[] = "114a93e36";
char refkeycl[] = "68a17df51";
char refname[] = "heroth";

/*
  generate_hashvalues:
  internal function.
*/

char *generate_hashvalues(const char *name, char *outbuffer, int buflen)
{
    struct cvs_MD5Context context;
    unsigned char checksum[16];
    int i;

    if(buflen < 33)
        return NULL;

    cvs_MD5Init(&context);
    cvs_MD5Update(&context, name, strlen(name));
    cvs_MD5Final(checksum, &context);

    for(i = 0; i < 16; i++)
    {
        sprintf(&outbuffer[i*2], "%02x", (unsigned int) checksum[i]);
    }

    outbuffer[32] = 0;
    return outbuffer;

}

/*
  populate_table
  name - string representing character's name exactly as how they logged in with it.
  table - the character specific table to hold the hash string.
  tabellen - length of table, must be at least 0x401 bytes large.

  returns - pointer to the table.
*/

char *populate_table(const char *name, char *table, int tablelen)
{
    char hash[64];
    int i;

    if(tablelen < 0x401)
        return NULL;

    if(!generate_hashvalues(name, &hash[0], sizeof(hash)))
        return NULL;

    table[0] = 0;
    sprintf(&table[0], "%s", &hash[0]);
    generate_hashvalues(&table[0], &hash[0], sizeof(hash));
    table[0] = 0;
    sprintf(&table[0], "%s", &hash[0]);

    for(i = 0; i < 32; i++)
    {
        generate_hashvalues(&table[0], &hash[0], sizeof(hash));
        sprintf(&table[0], "%s%s", &table[0], &hash[0]);
    }

    return &table[0];
}

#ifdef USE_RANDOM_INDEXES
#define rnd() (rand())
#else
#define rnd() (0x1337)
#endif

/*
  set_packet_indexes
  packet - pointer to the fully formed packet. Note that packet must
           have at least 3 more unused bytes or bad things will happen!

  returns - total size of the packet, including all header bytes.

  footnote: define USE_RANDOM_INDEXES if you want the key to be randomized each packet.
*/
int set_packet_indexes(unsigned char *packet)
{
    unsigned char k1 = ((rnd()&0x7FFF)%0x9B + 0x64)^0x21;
    unsigned short k2 = ((rnd()&0x7FFF) + 0x100)^0x7424;
    unsigned short psize = (unsigned short) (packet[1]<<8) + (packet[2]&0xFF);

    psize += 3;
    packet[psize] = (unsigned char) k2&0xFF;
    packet[psize+1] = k1;
    packet[psize+2] = (unsigned char) (k2>>8)&0xFF;
    packet[1] = (unsigned char) (psize>>8)&0xFF;
    packet[2] = (unsigned char) psize&0xFF;

    return psize + 3;
}
#undef rnd

/*
  generate_key2
  packet - pointer to full packet, either encrypted or not.
  table - the character specific table to hold the hash string.
  keyout - pointer to a buffer to hold the generated key. Buffer must
           be at least 10 bytes long (9 byte key + 0 terminator.)
  fromclient - 0 if packet is being encrypted on server and sent to client
               1 if packet came from client and is going to be decrypted.

  returns - pointer to the generated key.
*/

char *generate_key2(unsigned char *packet, char *table, char *keyout, int fromclient)
{
    unsigned short psize = (unsigned short) (packet[1]<<8) + (packet[2]&0xFF);
    unsigned int k1 = packet[psize+1];
    unsigned int k2 = (unsigned int) (packet[psize+2]<<8) + (packet[psize]&0xFF);
    int i;

    if(fromclient)
    {
        k1 ^= 0x25;
        k2 ^= 0x2361;
    }
    else
    {
        k1 ^= 0x21;
        k2 ^= 0x7424;
    }

    k1 *= k1;

    for(i = 0; i < 9; i++)
    {
        keyout[i] = table[(k1*i + k2)&0x3FF];
        k1 += 3;
    }
    keyout[9] = 0;

    return keyout;
}

/*
  test suite.
*/

int debug(unsigned char* stringthing, int len) {
	int i;

printf("Decrypted Packet\n");

	for (i = 0; i < len; i++) {
		printf("0x%02X, ", stringthing[i]);
	}
printf("\n");



printf("\n\n");
	for (i = 0; i < len; i++) {
		if (stringthing[i] < 16) { printf("%02X ", stringthing[i]);} 
		//else { printf("%02X ", stringthing[i]); }
}
	
	printf("\n\n");
	
	for (i = 0; i < len; i++) {
		if (stringthing[i] <= 32 || stringthing[i] > 126) {
			printf(" ");
		} else {
			printf("%c", stringthing[i]);
		}
	}
	printf("\n\n");

// ASCII LOCATIONS
printf("\nField:     Character:        Decimal:         Hex Value:\n");


for (i=0; i < len; i++) {
if (stringthing[i] >= 33 && stringthing[i] < 126) printf("    %i             %c                 %d                %02X\n", i, stringthing[i], stringthing[i], stringthing[i]);
else { printf("    %i                               %d              %02X\n", i, stringthing[i], stringthing[i]); }
}

printf("\n\n");









	return 0;
}

void crypt2(char *buff, char* key) {
	unsigned int Group=0;
	unsigned int GroupCount=0;
	unsigned int packet_len;
	unsigned char packet_inc;
	unsigned char KeyVal;
	int i;
	buff++;
	packet_len = SWAP16(*(unsigned short*)buff)-5;
	buff+=3;
	packet_inc = *buff;
	buff++;

	//buff now points to the first data byte
	if(packet_len>65535) return 0;

	for (i=0; i < packet_len; i++)
	{
		*(buff+i) ^= key[i%9];

		KeyVal = (unsigned char)(Group % 256); // Second Stage
		if (KeyVal != packet_inc)
		{
			*(buff+i) ^= KeyVal;
		}	

		*(buff+i) ^= packet_inc;

		GroupCount++;
		if (GroupCount == 9)
		{
			Group++;
			GroupCount = 0;
		}
	}
}
int main(int argc, char **argv)
{

	/*if (argc < 2 || argc > 3)
	{
		fprintf (stderr, "usage: %s name to gen table for\n", argv[0]);
		system("PAUSE");
		exit(1);
	}

    printf ("table for name (\"%s\") = ", argv[1]);
    printf("%s\n", populate_table(argv[1], &table[0], sizeof(table)));
    FILE *f = fopen("tbldump.dat", "w");
    fwrite(&table[0], sizeof(table[0]), sizeof(table)/sizeof(table[0]), f);
    fclose(f);*/

    char key[16];
    populate_table(&refname[0], &table[0], sizeof(table));
    generate_key2(&refpacket[0], &table[0], &key[0], 0);
    crypt2(&refpacket[0], &key[0]);

//printf("\n\n");
//printf("refkeysv: %s =?= genkey: %s\n", &refkeysv[0], &key[0]);



//    generate_key2(&refpacket[0], &table[0], &key[0], 1);
//printf("refkeycl: %s =?= genkey: %s\n", &refkeycl[0], &key[0]);
//printf("\n\n");



	debug(&refpacket[0], sizeof(refpacket));





	return 0;
}
