/*
Tests for the BencEntity class
*/

#undef _M_CEE_PURE
#undef new

#include <iostream>

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "bencoding.h"


TEST(BencEntityClassTest, TestParseWithRegion)
{
	const int numTestStr = 5;
	std::pair<unsigned char*, unsigned char*> region;
	std::string front("d1:rd2:id20:AAAABBBBCCCCDDDDEEEE5:nodes78:abcdefghij0101010101zzzzxxabcdefghij0123456789zzzzxx20_byte_dhtid_val_00zzzzxx5:token20:ÖÀE–vXŸRè◊œ∏YÔN‡1:v");
	std::string back("e1:t2:aa1:v4:UTn˜1:y1:re");
	std::string testText[numTestStr];
	std::string output;

	testText[0] = "i5e";    // bencoded integer
	testText[1] = "4:TEXT"; // bencoded text
	testText[2] = "l4:spam4:eggse"; // bencoded list of text
	testText[3] = "d3:cow3:moo4:spam4:eggse"; // bencoded dictionary
	testText[4] = "d4:spaml1:a1:bee"; // bencoded dictionary with embeded list

	for(int x=0; x<numTestStr; ++x){
		// assemble the message
		std::string bencMessage = front + testText[x] + back;

		// parse and get the region for 'v'
		BencEntity bEntity;
		BencEntity::Parse((const byte *)bencMessage.c_str(), bEntity, (const byte *)(bencMessage.c_str() + bencMessage.length()), "r\0v\0", &region);

		BencodedDict *dict = BencodedDict::AsDict(&bEntity);
		EXPECT_TRUE(dict) << "FAILED to extract a valid dictionary";
		if (!dict) {
			continue;
		}

		// did we get the correct length region
		EXPECT_EQ(testText[x].size(), region.second - region.first);

		// look at the region we received
		output.clear();
		for(int i=0; i<(region.second - region.first); ++i){
			output += region.first[i];
		}
		// do they match?
		EXPECT_TRUE(testText[x] == output) << "ERROR:  Expected and Actual regions do not match";

		// if there is a problem, output some information
		if(testText[x].size() != output.size() || testText[x] != output){
			std::cout << "\n\n*****\n  Expected ---> " << testText[x] << "\n";
			std::cout << "  Actual   ---> " << output << "\n*****\n";
		}
	}
}


TEST(BencEntityClassTest, TestParseInPlaceWithRegion)
{
	const int numTestStr = 5;
	std::pair<unsigned char*, unsigned char*> region;
	std::string front("d1:rd2:id20:AAAABBBBCCCCDDDDEEEE5:nodes78:abcdefghij0101010101zzzzxxabcdefghij0123456789zzzzxx20_byte_dhtid_val_00zzzzxx5:token20:ÖÀE–vXŸRè◊œ∏YÔN‡1:v");
	std::string back("e1:t2:aa1:v4:UTn˜1:y1:re");
	std::string testText[numTestStr];
	std::string output;

	testText[0] = "i5e";    // bencoded integer
	testText[1] = "4:TEXT"; // bencoded text
	testText[2] = "l4:spam4:eggse"; // bencoded list of text
	testText[3] = "d3:cow3:moo4:spam4:eggse"; // bencoded dictionary
	testText[4] = "d4:spaml1:a1:bee"; // bencoded dictionary with embeded list

	for(int x=0; x<numTestStr; ++x){
		// assemble the message
		std::string bencMessage = front + testText[x] + back;

		// parse and get the region for 'v'
		BencEntity bEntity;
		BencEntity::ParseInPlace((const byte *)bencMessage.c_str(), bEntity, (const byte *)(bencMessage.c_str() + bencMessage.length()), "r\0v\0", &region);

		BencodedDict *dict = BencodedDict::AsDict(&bEntity);
		EXPECT_TRUE(dict) << "FAILED to extract a valid dictionary";
		if (!dict) {
			continue;
		}

		// did we get the correct length region
		EXPECT_EQ(testText[x].size(), region.second - region.first);

		// look at the region we received
		output.clear();
		for(int i=0; i<(region.second - region.first); ++i){
			output += region.first[i];
		}
		// do they match?
		EXPECT_TRUE(testText[x] == output) << "ERROR:  Expected and Actual regions do not match";

		// if there is a problem, output some information
		if(testText[x].size() != output.size() || testText[x] != output){
			std::cout << "\n\n*****\n  Expected ---> " << testText[x] << "\n";
			std::cout << "  Actual   ---> " << output << "\n*****\n";
		}
	}
}

/*
When parsing this buffer:

0x04203B38  64 31 3a 61 64 32 3a 69 64 32 30 3a 61 62 63 64 65 66 67 68 69 6a 30 31 32 33 34 35 36 37 38 39  d1:ad2:id20:abcdefghij0123456789
0x04203B58  31 3a 6b 32 37 30 3a 30 82 01 0a 02 82 01 01 00 ca 68 15 99 36 19 d6 20 96 62 97 6c 0f 41 6a a8  1:k270:0........ h.ô6.÷ ñból.Aj®
0x04203B78  ce 81 4e 91 c2 3e a2 15 c1 08 53 20 85 6f 3a 45 4e 45 61 7f fe 5a 9a d5 ea c9 f5 b6 b5 1e 67 9c  Œ.Në¬>¢.¡.S .o:ENEa.˛Zö’Í…ı∂µ.gú
0x04203B98  85 68 1e 3d ae 7d 69 5b 9e c3 3c 2e 2b e9 25 1b bf 43 c0 4e 7e ea 01 cc dc 14 75 7e cd 18 7e 45  .h.=Æ}i[û√<.+È%.øC¿N~Í.Ã‹.u~Õ.~E
0x04203BB8  f0 d7 11 f9 b3 72 04 b7 49 ac 47 02 5f d5 91 a8 0c 4c 7e d5 72 1d 4e 39 af 61 6a 5c fc b3 fd a3  ◊.˘.r.∑I¨G._’ë®.L~’r.N9Øaj\¸.˝£
0x04203BD8  51 5e 0f 3b 9b f8 4c 0e 20 12 f3 aa 08 6d 83 e5 b2 12 97 35 ad fe 20 da cd a2 f4 68 4f f3 de ef  Q^.;.¯L. .Û™.mÉÂ..ó5.˛ ⁄Õ¢ÙhOÛﬁÔ
0x04203BF8  80 05 a3 60 c7 70 a0 6d aa f3 77 28 e0 47 50 81 73 0d 9c 1c a2 09 24 24 40 e5 cf 4d 70 c4 45 b4  Ä.£`«p†m™Ûw(‡GP.s.ú.¢.$$@ÂœMpƒE¥
0x04203C18  79 7d f1 f7 38 83 3a c3 cc 97 06 96 7a c5 aa 74 90 da 97 ed c4 03 2a a7 47 50 5f 6b a3 7c b4 c1  y}Ò˜8É:√Ãó.ñz≈™t.⁄óÌƒ.*ßGP_k£|¥¡
0x04203C38  1e e4 49 c4 8a 70 06 99 76 29 f0 4d c8 44 87 43 c6 1e 21 4d 2b 9f d0 62 fd 2e 01 57 ac 60 ba cb  .‰Iƒäp.ôv)M»D.C∆.!M+ü–b˝..W¨`∫À
0x04203C58  1d 18 f5 2f 43 09 64 ff 8e e8 67 a3 3b 6e db 61 02 03 01 00 01 33 3a 73 65 71 69 35 35 65 33 3a  ..ı/C.dˇéËg£;n€a.....3:seqi55e3:
0x04203C78  73 69 67 32 35 36 3a 76 29 2e 57 a9 d8 e0 7e 98 76 81 64 5e d1 ea 18 26 f3 eb 7d 45 9a a2 80 fd  sig256:v).W©ÿ‡~òv.d^—Í.&ÛÎ}Eö¢Ä˝
0x04203C98  b8 e8 87 4f 0a 48 12 47 fe 95 6e 5e 61 05 fb f6 4c a4 db d1 44 35 00 47 3d b1 3c 82 82 d9 02 31  ∏Ë.O.H.G˛.n^a.˚ˆL§€—D5.G=±<..Ÿ.1
0x04203CB8  35 0a 2e 73 9a b9 45 5c a2 87 20 21 a3 46 5e d4 e9 22 83 0c 0c df c5 4b dd 2b e5 87 9b 0d bd 6f  5..sö.E\¢. !£F^‘È"É..ﬂ≈K›+Â....o
0x04203CD8  46 22 bf 46 63 a4 42 8a 1c a8 c4 1e f9 e8 e1 51 b9 be 85 bd 43 41 02 9d d6 2a a8 e1 e5 ed c2 95  F"øFc§Bä.®ƒ.˘Ë·Q....CA..÷*®·ÂÌ¬.
0x04203CF8  d3 a5 08 9c 0d 52 d3 da 07 6b a4 4f 74 84 17 c1 2d ba ed 68 bb 43 83 9a fc f1 c3 0e 3f 48 d1 38  ”•.ú.R”⁄.k§Ot..¡-∫ÌhªCÉö¸Ò√.?H—8
0x04203D18  76 fd 49 52 78 64 02 d8 c2 22 5e 7b dd c7 98 53 f4 9f d9 74 35 8e fa ad 9d aa d4 b7 ec e6 48 71  v˝IRxd.ÿ¬"^{›«òSÙüŸt5é˙..™‘∑ÏÊHq
0x04203D38  aa 1c 7c f1 7b 8b f6 0e fa e7 21 07 ae 24 79 01 6c 47 bd d0 5a 05 1e 21 b7 ca d4 5a 9f 98 e1 48  ™.|Ò{.ˆ.˙Á!.Æ$y.lG.–Z..!∑ ‘Züò·H
0x04203D58  de 51 12 4f b8 af db db 50 83 e6 84 1a c3 4d 77 07 92 86 0c 03 08 c1 33 54 d4 8c f3 16 51 fe 8e  ﬁQ.O∏Ø€€PÉÊ..√Mw.í....¡3T‘åÛ.Q˛é
0x04203D78  53 ac 60 3b 2e 36 f3 35 3a 74 6f 6b 65 6e 32 30 3a 49 80 6c fe 05 fc 7b fd 2b 46 0e 58 2f 27 78  S¨`;.6Û5:token20:IÄl˛.¸{˝+F.X/'x
0x04203D98  d3 a9 60 eb 89 31 3a 76 31 36 3a 4d 75 74 61 62 6c 65 20 47 45 54 20 54 65 73 74 65 31 3a 71 33  ”©`Î.1:v16:Mutable GET Teste1:q3
0x04203DB8  3a 70 75 74 31 3a 74 32 3a 61 61 31 3a 79 31 3a 71 65 cd cd cd cd cd cd cd cd cd cd cd cd cd cd  :put1:t2:aa1:y1:qeÕÕÕÕÕÕÕÕÕÕÕÕÕÕ

Our benc entity is looking for "v" in the keys of this dictionary, but it doesn't differentiate between values and keys, or depth.
So in the above data, it's fooled by the signature "sig256:v).W©ÿ", because it starts with v (a 1 in 256 chance).  Therefore, it
assumes that the next element it parses is the 'value' of v, which is the key 5:token.

Even worse, inserting a dictionary "1:_d1:v3:bade" at the beginning of this dictionary causes us to identify the 'v' key in the
subdict as the 'v' key for the whole dictionary, again causing us to identify the wrong object (3:bad here).

BencEntity::ParseInPlace via BencParserElement::ParseNext needs to differentiate between keys and values and depth to get this test
right.
*/

static const byte *baddict = (const byte *)("\x64" "1:_d1:v3:bade" "\x31\x3a\x61\x64\x32\x3a\x69\x64\x32\x30\x3a\x61\x62\x63\x64\x65\x66\x67\x68\x69\x6a\x30\x31\x32\x33\x34\x35\x36\x37\x38\x39\x31\x3a\x6b\x32\x37\x30\x3a\x30\x82\x01\x0a\x02\x82\x01\x01\x00\xca\x68\x15\x99\x36\x19\xd6\x20\x96\x62\x97\x6c\x0f\x41\x6a\xa8\xce\x81\x4e\x91\xc2\x3e\xa2\x15\xc1\x08\x53\x20\x85\x6f\x3a\x45\x4e\x45\x61\x7f\xfe\x5a\x9a\xd5\xea\xc9\xf5\xb6\xb5\x1e\x67\x9c\x85\x68\x1e\x3d\xae\x7d\x69\x5b\x9e\xc3\x3c\x2e\x2b\xe9\x25\x1b\xbf\x43\xc0\x4e\x7e\xea\x01\xcc\xdc\x14\x75\x7e\xcd\x18\x7e\x45\xf0\xd7\x11\xf9\xb3\x72\x04\xb7\x49\xac\x47\x02\x5f\xd5\x91\xa8\x0c\x4c\x7e\xd5\x72\x1d\x4e\x39\xaf\x61\x6a\x5c\xfc\xb3\xfd\xa3\x51\x5e\x0f\x3b\x9b\xf8\x4c\x0e\x20\x12\xf3\xaa\x08\x6d\x83\xe5\xb2\x12\x97\x35\xad\xfe\x20\xda\xcd\xa2\xf4\x68\x4f\xf3\xde\xef\x80\x05\xa3\x60\xc7\x70\xa0\x6d\xaa\xf3\x77\x28\xe0\x47\x50\x81\x73\x0d\x9c\x1c\xa2\x09\x24\x24\x40\xe5\xcf\x4d\x70\xc4\x45\xb4\x79\x7d\xf1\xf7\x38\x83\x3a\xc3\xcc\x97\x06\x96\x7a\xc5\xaa\x74\x90\xda\x97\xed\xc4\x03\x2a\xa7\x47\x50\x5f\x6b\xa3\x7c\xb4\xc1\x1e\xe4\x49\xc4\x8a\x70\x06\x99\x76\x29\xf0\x4d\xc8\x44\x87\x43\xc6\x1e\x21\x4d\x2b\x9f\xd0\x62\xfd\x2e\x01\x57\xac\x60\xba\xcb\x1d\x18\xf5\x2f\x43\x09\x64\xff\x8e\xe8\x67\xa3\x3b\x6e\xdb\x61\x02\x03\x01\x00\x01\x33\x3a\x73\x65\x71\x69\x35\x35\x65\x33\x3a\x73\x69\x67\x32\x35\x36\x3a\x76\x29\x2e\x57\xa9\xd8\xe0\x7e\x98\x76\x81\x64\x5e\xd1\xea\x18\x26\xf3\xeb\x7d\x45\x9a\xa2\x80\xfd\xb8\xe8\x87\x4f\x0a\x48\x12\x47\xfe\x95\x6e\x5e\x61\x05\xfb\xf6\x4c\xa4\xdb\xd1\x44\x35\x00\x47\x3d\xb1\x3c\x82\x82\xd9\x02\x31\x35\x0a\x2e\x73\x9a\xb9\x45\x5c\xa2\x87\x20\x21\xa3\x46\x5e\xd4\xe9\x22\x83\x0c\x0c\xdf\xc5\x4b\xdd\x2b\xe5\x87\x9b\x0d\xbd\x6f\x46\x22\xbf\x46\x63\xa4\x42\x8a\x1c\xa8\xc4\x1e\xf9\xe8\xe1\x51\xb9\xbe\x85\xbd\x43\x41\x02\x9d\xd6\x2a\xa8\xe1\xe5\xed\xc2\x95\xd3\xa5\x08\x9c\x0d\x52\xd3\xda\x07\x6b\xa4\x4f\x74\x84\x17\xc1\x2d\xba\xed\x68\xbb\x43\x83\x9a\xfc\xf1\xc3\x0e\x3f\x48\xd1\x38\x76\xfd\x49\x52\x78\x64\x02\xd8\xc2\x22\x5e\x7b\xdd\xc7\x98\x53\xf4\x9f\xd9\x74\x35\x8e\xfa\xad\x9d\xaa\xd4\xb7\xec\xe6\x48\x71\xaa\x1c\x7c\xf1\x7b\x8b\xf6\x0e\xfa\xe7\x21\x07\xae\x24\x79\x01\x6c\x47\xbd\xd0\x5a\x05\x1e\x21\xb7\xca\xd4\x5a\x9f\x98\xe1\x48\xde\x51\x12\x4f\xb8\xaf\xdb\xdb\x50\x83\xe6\x84\x1a\xc3\x4d\x77\x07\x92\x86\x0c\x03\x08\xc1\x33\x54\xd4\x8c\xf3\x16\x51\xfe\x8e\x53\xac\x60\x3b\x2e\x36\xf3\x35\x3a\x74\x6f\x6b\x65\x6e\x32\x30\x3a\x49\x80\x6c\xfe\x05\xfc\x7b\xfd\x2b\x46\x0e\x58\x2f\x27\x78\xd3\xa9\x60\xeb\x89\x31\x3a\x76\x31\x36\x3a\x4d\x75\x74\x61\x62\x6c\x65\x20\x47\x45\x54\x20\x54\x65\x73\x74\x65\x31\x3a\x71\x33\x3a\x70\x75\x74\x31\x3a\x74\x32\x3a\x61\x61\x31\x3a\x79\x31\x3a\x71\x65");
static int badlen = 0x04203DB8 - 0x04203B38 + 18 + 13;

TEST(BencEntityClassTest, TestDontBeFooledByNonKeys) 
{
	BencodedDict _bDict;
	const byte *baddict = (const byte *)("\x64" "1:_d1:v3:bade" "\x31\x3a\x61\x64\x32\x3a\x69\x64\x32\x30\x3a\x61\x62\x63\x64\x65\x66\x67\x68\x69\x6a\x30\x31\x32\x33\x34\x35\x36\x37\x38\x39\x31\x3a\x6b\x32\x37\x30\x3a\x30\x82\x01\x0a\x02\x82\x01\x01\x00\xca\x68\x15\x99\x36\x19\xd6\x20\x96\x62\x97\x6c\x0f\x41\x6a\xa8\xce\x81\x4e\x91\xc2\x3e\xa2\x15\xc1\x08\x53\x20\x85\x6f\x3a\x45\x4e\x45\x61\x7f\xfe\x5a\x9a\xd5\xea\xc9\xf5\xb6\xb5\x1e\x67\x9c\x85\x68\x1e\x3d\xae\x7d\x69\x5b\x9e\xc3\x3c\x2e\x2b\xe9\x25\x1b\xbf\x43\xc0\x4e\x7e\xea\x01\xcc\xdc\x14\x75\x7e\xcd\x18\x7e\x45\xf0\xd7\x11\xf9\xb3\x72\x04\xb7\x49\xac\x47\x02\x5f\xd5\x91\xa8\x0c\x4c\x7e\xd5\x72\x1d\x4e\x39\xaf\x61\x6a\x5c\xfc\xb3\xfd\xa3\x51\x5e\x0f\x3b\x9b\xf8\x4c\x0e\x20\x12\xf3\xaa\x08\x6d\x83\xe5\xb2\x12\x97\x35\xad\xfe\x20\xda\xcd\xa2\xf4\x68\x4f\xf3\xde\xef\x80\x05\xa3\x60\xc7\x70\xa0\x6d\xaa\xf3\x77\x28\xe0\x47\x50\x81\x73\x0d\x9c\x1c\xa2\x09\x24\x24\x40\xe5\xcf\x4d\x70\xc4\x45\xb4\x79\x7d\xf1\xf7\x38\x83\x3a\xc3\xcc\x97\x06\x96\x7a\xc5\xaa\x74\x90\xda\x97\xed\xc4\x03\x2a\xa7\x47\x50\x5f\x6b\xa3\x7c\xb4\xc1\x1e\xe4\x49\xc4\x8a\x70\x06\x99\x76\x29\xf0\x4d\xc8\x44\x87\x43\xc6\x1e\x21\x4d\x2b\x9f\xd0\x62\xfd\x2e\x01\x57\xac\x60\xba\xcb\x1d\x18\xf5\x2f\x43\x09\x64\xff\x8e\xe8\x67\xa3\x3b\x6e\xdb\x61\x02\x03\x01\x00\x01\x33\x3a\x73\x65\x71\x69\x35\x35\x65\x33\x3a\x73\x69\x67\x32\x35\x36\x3a\x76\x29\x2e\x57\xa9\xd8\xe0\x7e\x98\x76\x81\x64\x5e\xd1\xea\x18\x26\xf3\xeb\x7d\x45\x9a\xa2\x80\xfd\xb8\xe8\x87\x4f\x0a\x48\x12\x47\xfe\x95\x6e\x5e\x61\x05\xfb\xf6\x4c\xa4\xdb\xd1\x44\x35\x00\x47\x3d\xb1\x3c\x82\x82\xd9\x02\x31\x35\x0a\x2e\x73\x9a\xb9\x45\x5c\xa2\x87\x20\x21\xa3\x46\x5e\xd4\xe9\x22\x83\x0c\x0c\xdf\xc5\x4b\xdd\x2b\xe5\x87\x9b\x0d\xbd\x6f\x46\x22\xbf\x46\x63\xa4\x42\x8a\x1c\xa8\xc4\x1e\xf9\xe8\xe1\x51\xb9\xbe\x85\xbd\x43\x41\x02\x9d\xd6\x2a\xa8\xe1\xe5\xed\xc2\x95\xd3\xa5\x08\x9c\x0d\x52\xd3\xda\x07\x6b\xa4\x4f\x74\x84\x17\xc1\x2d\xba\xed\x68\xbb\x43\x83\x9a\xfc\xf1\xc3\x0e\x3f\x48\xd1\x38\x76\xfd\x49\x52\x78\x64\x02\xd8\xc2\x22\x5e\x7b\xdd\xc7\x98\x53\xf4\x9f\xd9\x74\x35\x8e\xfa\xad\x9d\xaa\xd4\xb7\xec\xe6\x48\x71\xaa\x1c\x7c\xf1\x7b\x8b\xf6\x0e\xfa\xe7\x21\x07\xae\x24\x79\x01\x6c\x47\xbd\xd0\x5a\x05\x1e\x21\xb7\xca\xd4\x5a\x9f\x98\xe1\x48\xde\x51\x12\x4f\xb8\xaf\xdb\xdb\x50\x83\xe6\x84\x1a\xc3\x4d\x77\x07\x92\x86\x0c\x03\x08\xc1\x33\x54\xd4\x8c\xf3\x16\x51\xfe\x8e\x53\xac\x60\x3b\x2e\x36\xf3\x35\x3a\x74\x6f\x6b\x65\x6e\x32\x30\x3a\x49\x80\x6c\xfe\x05\xfc\x7b\xfd\x2b\x46\x0e\x58\x2f\x27\x78\xd3\xa9\x60\xeb\x89\x31\x3a\x76\x31\x36\x3a\x4d\x75\x74\x61\x62\x6c\x65\x20\x47\x45\x54\x20\x54\x65\x73\x74\x65\x31\x3a\x71\x33\x3a\x70\x75\x74\x31\x3a\x74\x32\x3a\x61\x61\x31\x3a\x79\x31\x3a\x71\x65");
	int badlen = 0x04203DB8 - 0x04203B38 + 18 + 13;
	std::pair<unsigned char*, unsigned char*> rgn;
	
	ASSERT_TRUE(BencEntity::ParseInPlace(baddict, _bDict, baddict + badlen, "a\0v\0", &rgn));
	ASSERT_EQ(rgn.second - rgn.first, 19);
}


// 723a3164 763a3164 3331696c 35696530 36696539  d1:rd1:vli130ei59ei6
// 37696532 32696536 32653134 3264693a 299b3a30  2ei76ei241e2:id20:.)
// 80284845 d164d311 37521b79 50f0bdbf 3a322031  EH(Ä.”d—y.R7ø.P1 2:
// 3a347069 52d9d16b 743a3165 0a053a34 3a310000  ip4:k—ŸRe1:t4:....1:
// 553a3476 31536454 3a31793a cccc6572 cccccccc  v4:UTdS1:y1:reÃÃÃÃÃÃ

TEST(BencEntityClassTest, TestBadDHTResponse1) 
{
	BencodedDict _bDict;
	const byte *baddict = (const byte *)("d1:rd1:vli130ei59ei62ei76ei241e2:id20:.)EH(Ä.”d—y.R7ø.P1 2:ip4:k—ŸRe1:t4:....1:v4:UTdS1:y1:re");
	int badlen = strlen((char*) baddict);
	std::pair<unsigned char*, unsigned char*> rgn;

	std::vector<const char*> keys;
	keys.push_back("a\0v\0");
	keys.push_back("r\0v\0");

	// We expect this one to fail parsing
	ASSERT_FALSE(BencEntity::ParseInPlace(baddict, _bDict, baddict + badlen, keys, &rgn));
}

// Positive test.
TEST(BencEntityClassTest, TestParseDict)
{
	BencodedDict dhtFN;
	// Typical DHT find_node query: http://www.bittorrent.org/beps/bep_0005.html
	std::string fn_str("d1:ad2:id20:abcdefghij01234567896:target20:mnopqrstuvwxyz123456e1:q9:find_node1:t2:aa1:y1:qe");

	const byte* ret = BencEntity::ParseInPlace((byte*)fn_str.c_str(),
		dhtFN,
		(const byte*)(fn_str.c_str() + fn_str.length()));

	ASSERT_TRUE(ret);

	// Check that the whole dict was parsed
	ASSERT_EQ('\0', ret[0]) << "Failed to fully parse a valid DHT find_node query!";

	// There should be a dict in here
	BencodedDict *adict = dhtFN.GetDict("a");
	ASSERT_TRUE(adict);

	// Fetch the id value
	EXPECT_STREQ("abcdefghij0123456789", adict->GetString("id"));

	// Fetch the target value
	EXPECT_STREQ("mnopqrstuvwxyz123456", adict->GetString("target"));

	// Fetch the t value
	EXPECT_STREQ("aa", dhtFN.GetString("t"));

	// Fetch the q value
	EXPECT_STREQ("find_node", dhtFN.GetString("q"));

	// Fetch the y value
	EXPECT_STREQ("q", dhtFN.GetString("y"));
}

// Test for large invalid lengths
TEST(BencEntityClassTest, TestBadStringLenInDict)
{
	BencodedDict badDhtFN;
	// Invalid DHT query. q key length is too large
	std::string bfn_str("d1:ad2:id20:abcdefghij01234567896:target20:mnopqrstuvwxyz123456e4294967285:q9:find_node1:t2:aa1:y1:qe");

	const byte* ret_bad = BencEntity::ParseInPlace((byte*)bfn_str.c_str(),
		badDhtFN,
		(const byte*)(bfn_str.c_str() + bfn_str.length()));

	// Parser will abort when trying to parse the length of key 'q'
	ASSERT_FALSE(ret_bad);
}

// Test for miscellaneous large/invalid string lengths
TEST(BencEntityClassTest, TestBadStringLenInStr)
{
	BencodedList badList;

	// Large length. Abort parse.
	std::string bl_str("l999:AA2:BBe");
	const byte* ret_bad2 = BencEntity::ParseInPlace((byte*)bl_str.c_str(), badList, (const byte*)(bl_str.c_str() + bl_str.length()));
	ASSERT_FALSE(ret_bad2);

	// In-range. Can't parse.
	bl_str = "l3:AA2:BBe";

	ret_bad2 = BencEntity::ParseInPlace((byte*)bl_str.c_str(), badList, (const byte*)(bl_str.c_str() + bl_str.length()));
	ASSERT_FALSE(ret_bad2);

	// In-range. Can't parse.
	bl_str = "l7:AA2:BBe";

	ret_bad2 = BencEntity::ParseInPlace((byte*)bl_str.c_str(), badList, (const byte*)(bl_str.c_str() + bl_str.length()));
	ASSERT_FALSE(ret_bad2);

	// Invalid length. Can't parse.
	bl_str = "l-2:AA2:BBe";

	ret_bad2 = BencEntity::ParseInPlace((byte*)bl_str.c_str(), badList, (const byte*)(bl_str.c_str() + bl_str.length()));
	ASSERT_FALSE(ret_bad2);
}
