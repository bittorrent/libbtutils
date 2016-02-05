#include <math.h>
//#undef _M_CEE_PURE
//#undef new

#include <iostream>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "bencoding.h"
#include "sockaddr.h"
#include "ut_bencoding_data.h"

#define utlogf(fmt,...) string_fmt(fmt ,##__VA_ARGS__)
//#define utassert_failmsg(x,log) EXPECT_TRUE(x) << log

namespace unittest {
#define SETTINGS_LENGTH 0x50dc


void Parse(const unsigned char *p, uint len, BencodedDict &base)
{
	ASSERT_TRUE(p != NULL);
	ASSERT_NE(0, len);

	std::pair<unsigned char*, unsigned char*> rgn(NULL,NULL);

	// parse the torrent file
//	EXPECT_TRUE(base.GetType() == BENC_VOID);
	ASSERT_TRUE(BencEntity::Parse(p, base, p + len, "info\0", &rgn) != NULL);
	ASSERT_EQ(BENC_DICT, base.GetType());

	EXPECT_GE(rgn.first, p);
	EXPECT_GE(rgn.second, rgn.first);

	uint info_base = rgn.first - p;
	uint info_size = rgn.second - rgn.first;

	EXPECT_EQ(static_cast<uint>(132), info_base);
	EXPECT_EQ(static_cast<uint>(16982), info_size);

	// compute the hash of the info region
	// TODO
	/*
	SHA1 sha;
	const sha1_hash * digest = sha.Hash(rgn.first, info_size);

	EXPECT_TRUE(memcmp(string_fmt("%h", digest->value).c_str(),
			"b545029b7cc7d1b1f4d0b4707ae44bd35403a0b9",
			40) == 0);
	*/
}
}

TEST(Bencoding, DHT) {
	static unsigned char msg[] = {
		0x64, 0x31, 0x3a, 0x61, 0x64, 0x32, 0x3a, 0x69,
		0x64, 0x32, 0x30, 0x3a, 0xa7, 0x41, 0x00, 0x00,
		0xf1, 0x3a, 0xd6, 0x10, 0xd9, 0xac, 0xb7, 0x60,
		0x2a, 0x0c, 0xb5, 0x3a, 0x82, 0xb7, 0x31, 0x44,
		0x36, 0x3a, 0x74, 0x61, 0x72, 0x67, 0x65, 0x74,
		0x32, 0x30, 0x3a, 0xa7, 0x41, 0x00, 0x00, 0xf1,
		0x3a, 0xd6, 0x10, 0xd9, 0xac, 0xb7, 0x60, 0x2a,
		0x0c, 0xb5, 0x3a, 0x82, 0xb7, 0x31, 0x45, 0x65,
		0x31, 0x3a, 0x71, 0x39, 0x3a, 0x66, 0x69, 0x6e,
		0x64, 0x5f, 0x6e, 0x6f, 0x64, 0x65, 0x31, 0x3a,
		0x74, 0x34, 0x3a, 0x17, 0x39, 0xe8, 0x55, 0x31,
		0x3a, 0x76, 0x34, 0x3a, 0x63, 0x74, 0x00, 0x01,
		0x31, 0x3a, 0x79, 0x31, 0x3a, 0x71, 0x65
	};

	BencEntity dict;
	std::pair<unsigned char*, unsigned char*> region;
	bool ok = (BencEntity::ParseInPlace(msg, dict, msg + sizeof(msg), "a\0v\0", &region)) ? true : false;
	EXPECT_TRUE(ok);
	ASSERT_EQ(BENC_DICT, dict.GetType());

	std::string command = BencEntity::AsDict(&dict)->GetString("q");
	EXPECT_EQ(command, "find_node");
}

TEST(Bencoding, Multikey) {
	static const char msg[] = "d1:b1:c1:rd1:s1:t1:v6:potatoee";

	BencEntity dict;
	std::pair<unsigned char*, unsigned char*> region;
	std::vector<const char*> keys;
	keys.push_back("a\0v\0");
	keys.push_back("r\0v\0");
	bool ok = (BencEntity::ParseInPlace(reinterpret_cast<const unsigned char*>(msg), dict, reinterpret_cast<const unsigned char*>(msg) + sizeof(msg), keys, &region)) ? true : false;
	EXPECT_TRUE(ok);
	ASSERT_EQ(BENC_DICT, dict.GetType());
	EXPECT_GE(reinterpret_cast<const char*>(region.first), msg);
	EXPECT_GE(region.second, region.first);
	EXPECT_EQ(0, memcmp(region.first, "6:potato", region.second - region.first));
	keys.clear();
	keys.push_back("r\0v\0");
	keys.push_back("a\0v\0");
	ok = (BencEntity::ParseInPlace(reinterpret_cast<const unsigned char*>(msg), dict, reinterpret_cast<const unsigned char*>(msg) + sizeof(msg), keys, &region)) ? true : false;
	EXPECT_TRUE(ok);
	ASSERT_EQ(BENC_DICT, dict.GetType());
	EXPECT_EQ(0, memcmp(region.first, "6:potato", region.second - region.first));
	keys.clear();
	keys.push_back("r\0v\0");
	keys.push_back("b\0");
	ok = (BencEntity::ParseInPlace(reinterpret_cast<const unsigned char*>(msg), dict, reinterpret_cast<const unsigned char*>(msg) + sizeof(msg), keys, &region)) ? true : false;
	EXPECT_TRUE(ok);
	ASSERT_EQ(BENC_DICT, dict.GetType());
	EXPECT_EQ(0, memcmp(region.first, "1:c", region.second - region.first));
}

TEST(Bencoding, Basics) {
	static const char sample_scrape[] = "d5:filesd20:7\247!\365'\221\335\320\352\305Q\343\200\344qf\220\316\346\371d8:completei8e10:downloadedi16838e10:incompletei5e4:name17:Fedora-8-Live-ppce20:]\341\022\010E\230\356k\223\363\260`$w\327\357\321\264v2d8:completei36e10:downloadedi45649e10:incompletei8e4:name22:Fedora-8-Live-KDE-i686e20:\301\234\352<DC\367\273\017\342\1\353\306\303\221\257tJ\254\251d8:completei24e10:downloadedi29004e10:incompletei2e4:name20:Fedora-8-Live-x86_64eee";
	static const char* keys[] = {
		"7\247!\365'\221\335\320\352\305Q\343\200\344qf\220\316\346\371",
		"]\341\022\010E\230\356k\223\363\260`$w\327\357\321\264v2",
		"\301\234\352<DC\367\273\017\342\1\353\306\303\221\257tJ\254\251"
	};

	const char* name;
	unsigned char* tmp = static_cast<unsigned char*>(malloc(sizeof(sample_scrape)));
	EXPECT_TRUE(tmp != NULL);
	memcpy(tmp, sample_scrape, sizeof(sample_scrape));
	BencodedDict dict;
	bool ok = (BencEntity::ParseInPlace(tmp, dict, tmp + sizeof(sample_scrape))) ? true : false;
	EXPECT_TRUE(ok);

	ASSERT_EQ(BENC_DICT, dict.GetType());
	BencodedDict* files = dict.GetDict("files");
	EXPECT_TRUE(files != NULL);
	EXPECT_EQ(static_cast<size_t>(3), files->GetCount());
	for(int i = 0; i < 3; i++) {
		EXPECT_TRUE(files->HasKey(keys[i]));
	}
	if (files) {
		BencodedEntityMap::iterator it = files->dict->begin();
		BencodedDict* d = (BencodedDict*) &it->second;
		EXPECT_TRUE(d != NULL);
		EXPECT_EQ(BENC_DICT, d->bencType);
		EXPECT_EQ(8, d->GetInt("complete", -99));
		EXPECT_EQ(16838, d->GetInt("downloaded", -99));
		EXPECT_EQ(5, d->GetInt("incomplete", -99));
		name = "Fedora-8-Live-ppc";
		EXPECT_EQ(0, memcmp(name, d->GetString("name"), sizeof(name) - 1));

		it++;
		d = (BencodedDict*) (BencodedDict*) &it->second;
		EXPECT_TRUE(d != NULL);
		EXPECT_EQ(BENC_DICT, d->bencType);
		EXPECT_EQ(36, d->GetInt("complete", -99));
		EXPECT_EQ(45649, d->GetInt("downloaded", -99));
		EXPECT_EQ(8, d->GetInt("incomplete", -99));
		name = "Fedora-8-Live-KDE-i686";
		EXPECT_EQ(0, memcmp(name, d->GetString("name"), sizeof(name) -1));

		it++;
		d = (BencodedDict*) (BencodedDict*) &it->second;
		EXPECT_TRUE(d != NULL);
		EXPECT_EQ(BENC_DICT, d->bencType);
		EXPECT_EQ(24, d->GetInt("complete", -99));
		EXPECT_EQ(29004, d->GetInt("downloaded", -99));
		EXPECT_EQ(2, d->GetInt("incomplete", -99));
		name = "Fedora-8-Live-x86_64";
		EXPECT_EQ(0, memcmp(name, d->GetString("name"), sizeof(name) -1));
	}

	free(tmp);

	static const char sample_tracker[] = "d8:completei36e10:incompletei10e8:intervali1800e5:peers108:E\0210\236\032\342J)K\5'\315D\222\216\026\030:\230\3\334\245\032\341:\010F+N/\301\331}\245\363\235\022\7\031\241\032\341O\317\232\203\260\017Y\365b\022&\232\325\263\344\6\267\"\200\257\015c\032\341U\036\2329\032\341W\3+S\303V\303\270\311\n\032\342N^8CA\361Xz\375Nd\325\246R\t,\032\337Ov\013$\314\377e";

	tmp = static_cast<unsigned char*>(malloc(sizeof(sample_tracker) * sizeof(char)));
	ASSERT_TRUE(tmp != NULL);
	memcpy(tmp, sample_tracker, sizeof(sample_tracker));
	BencodedDict dict2;
	ok = (BencEntity::ParseInPlace(tmp, dict2, tmp + sizeof(sample_tracker))) ? true : false;
	ASSERT_TRUE(ok);

	ASSERT_EQ(BENC_DICT, dict2.GetType());
	EXPECT_EQ(1800, dict2.GetInt("interval", -99));
	EXPECT_EQ(36, dict2.GetInt("complete", -99));
	EXPECT_EQ(10, dict2.GetInt("incomplete", -99));
	EXPECT_TRUE(!dict2.GetList("peers"));
	size_t len;
	cstr pbin = dict2.GetString("peers", &len);
	EXPECT_EQ(static_cast<size_t>(108), len);
	EXPECT_TRUE(pbin != NULL);

	static const char* peer_addrs[] = {
		"69.17.48.158:6882", "74.41.75.5:10189", "68.146.142.22:6202",
		"152.3.220.165:6881", "58.8.70.43:20015", "193.217.125.165:62365",
		"18.7.25.161:6881", "79.207.154.131:45071", "89.245.98.18:9882",
		"213.179.228.6:46882", "128.175.13.99:6881", "85.30.154.57:6881",
		"87.3.43.83:50006", "195.184.201.10:6882", "78.94.56.67:16881",
		"88.122.253.78:25813", "166.82.9.44:6879", "79.118.11.36:52479"
	};

	EXPECT_EQ(static_cast<size_t>(0), len % 6);
	const uint n = len / 6;
	EXPECT_EQ(sizeof(peer_addrs)/sizeof(peer_addrs[0]), n); //gack
	for (uint i = 0; i < n; ++i, pbin += 6) {
		const SockAddr ip((unsigned char*)pbin, 6, NULL);
		SockAddr ip2 = SockAddr::parse_addr(peer_addrs[i]);
		EXPECT_EQ(ip, ip2);
	}

	free(tmp);
}

TEST(Bencoding, String1) {
	static const char sample_data[] = "d7:astring6:avalue5:aznumi10e6:newstr10:0123456789e";

	unsigned char* tmp = static_cast<unsigned char*>(malloc(sizeof(sample_data)));
	ASSERT_TRUE(tmp != NULL);
	memcpy(tmp, sample_data, sizeof(sample_data));
	BencodedDict dictinplace;
	BencodedDict dict;
	bool ok = (BencEntity::Parse(tmp, dict, tmp + sizeof(sample_data))) ? true : false;
	ASSERT_TRUE(ok);
	tstring value(dict.GetStringT("astring"));
	EXPECT_FALSE(value.empty());
	if (value.size()) {
		size_t valuelen = value.size();
		EXPECT_EQ(static_cast<size_t>(6), valuelen);
		/*utassert_failmsg(6 == valuelen,
			utlogf("Value and length expected %s %d actual %s %d",
				"avalue", 6, value.c_str(), valuelen));*/
	}
	ok = (BencEntity::ParseInPlace(tmp, dictinplace, tmp + sizeof(sample_data))) ? true : false;
	ASSERT_TRUE(ok);
	value = dictinplace.GetStringT("astring");
	EXPECT_FALSE(value.empty());
	if (value.size()) {
		size_t valuelen = value.size();
		EXPECT_EQ(static_cast<size_t>(6), valuelen);
		/*utassert_failmsg(6 == valuelen,
			utlogf("Value and length expected %s %d actual %s %d",
				"avalue", 6, value.c_str(), valuelen));*/
	}
	free(tmp);
}

TEST(Bencoding, String2) {
	static const char sample_data[] = "d7:astring6:avalue5:aznumi10e6:newstr10:0123456789e";

	unsigned char* tmp = static_cast<unsigned char*>(malloc(sizeof(sample_data)));
	ASSERT_TRUE(tmp != NULL);
	memcpy(tmp, sample_data, sizeof(sample_data));
	BencodedDict dictinplace;
	BencodedDict dict;
	bool ok = (BencEntity::Parse(tmp, dict, tmp + sizeof(sample_data))) ? true : false;
	ASSERT_TRUE(ok);
	tstring value = dict.GetStringT("astring");
	EXPECT_FALSE(value.empty());
	if (value.size()) {
		size_t valuelen = value.size();
		EXPECT_EQ(static_cast<size_t>(6), valuelen);
		/*utassert_failmsg(6 == valuelen,
			utlogf("Value and length expected %s %d actual %s %d",
				"avalue", 6, value.c_str(), valuelen));*/
	}
	ok = (BencEntity::ParseInPlace(tmp, dictinplace, tmp + sizeof(sample_data))) ? true : false;
	ASSERT_TRUE(ok);
	value = dictinplace.GetStringT("astring");
	EXPECT_FALSE(value.empty());
	if (value.size()) {
		size_t valuelen = value.size();
		EXPECT_EQ(static_cast<size_t>(6), valuelen);
		/*utassert_failmsg(6 == valuelen,
			utlogf("Value and length expected %s %d actual %s %d",
				"avalue", 6, value.c_str(), valuelen));*/
	}
	free(tmp);
}

TEST(Bencoding, TwoFiles) {
	unsigned char buffer[] = "ld6:lengthi73773662e4:pathl18:13 Silverfuck.flaceed6:lengthi68006760e4:pathl15:11 Oceania.flaceee";
	BencEntity b;
	ASSERT_TRUE(BencEntity::Parse(buffer, b, buffer + sizeof(buffer)-1) != NULL);
	EXPECT_EQ(BENC_LIST, b.GetType());
}

TEST(Bencoding, AllFiles) {
	unsigned char buffer[] = "ld6:lengthi73773662e4:pathl18:13 Silverfuck.flaceed6:lengthi68006760e4:pathl15:11 Oceania.flaceed6:lengthi53151088e4:pathl49:16 Thru the Eyes of Ruby (w: I Am One tease).flaceed6:lengthi51403387e4:pathl14:04 Starla.flaceed6:lengthi51282991e4:pathl18:20 For Martha.flaceed6:lengthi45949439e4:pathl12:10 Siva.flaceed6:lengthi43531708e4:pathl35:23 Bullet With Butterfly Wings.flaceed6:lengthi38384832e4:pathl12:09 Soma.flaceed6:lengthi35563659e4:pathl20:07 Window Paine.flaceed6:lengthi34611179e4:pathl14:02 Quasar.flaceed6:lengthi33650038e4:pathl18:05 Geek U.S.A.flaceed6:lengthi33056735e4:pathl25:08 Lightning Strikes.flaceed6:lengthi31249410e4:pathl19:17 Cherub Rock.flaceed6:lengthi29052468e4:pathl18:15 Pale Horse.flaceed6:lengthi28567588e4:pathl13:18 Owata.flaceed6:lengthi27877903e4:pathl25:19 My Love Is Winter.flaceed6:lengthi27128747e4:pathl20:21 Encore Break.flaceed6:lengthi24447297e4:pathl14:06 Muzzle.flaceed6:lengthi23627619e4:pathl18:03 Panopticon.flaceed6:lengthi21799823e4:pathl16:14 Obscured.flaceed6:lengthi19913776e4:pathl27:12 Frail and Bedazzled.flaceed6:lengthi16869841e4:pathl15:22 Pissant.flaceed6:lengthi5643269e4:pathl13:01 Intro.flaceed6:lengthi1321e4:pathl10:md5sum.md5eed6:lengthi1302e4:pathl19:fingerprint.ffp.txteed6:lengthi638e4:pathl8:info.txteee";
	BencEntity b;
	ASSERT_TRUE(BencEntity::Parse(buffer, b, buffer + sizeof(buffer)-1) != NULL);
	EXPECT_EQ(BENC_LIST, b.GetType());
}

TEST(Bencoding, NestedList) {
	unsigned char buffer[] = "l1:al1:b1:ce1:de";
	BencEntity b;
	ASSERT_TRUE(BencEntity::Parse(buffer, b, buffer + sizeof(buffer)-1) != NULL);
	EXPECT_EQ(BENC_LIST, b.GetType());
}

TEST(Bencoding, MixedDictionary) {
	unsigned char buffer[] = "d1:a1:b1:cd1:dl1:e1:feee";
	BencEntity b;
	ASSERT_TRUE(BencEntity::Parse(buffer, b, buffer + sizeof(buffer)-1) != NULL);
	EXPECT_EQ(BENC_DICT, b.GetType());
}

TEST(Bencoding, MoreNestedDictionary) {
	unsigned char buffer[] = "d1:a1:b1:c1:d1:ed1:fld2:k12:v12:k22:v2ed2:k32:v32:k42:v4eeee";
	BencodedDict b;
	ASSERT_TRUE(BencEntity::Parse(buffer, b, buffer + sizeof(buffer)-1) != NULL);
	ASSERT_EQ(BENC_DICT, b.GetType());
	std::string serialized = b.Serialize();
	EXPECT_EQ(sizeof(buffer) - 1, serialized.size());
	EXPECT_EQ(0, memcmp(serialized.c_str(), buffer, sizeof(buffer) - 1));
}

// out bdecoder doesn't support this!
TEST(Bencoding, DISABLED_plain_string) {
	unsigned char buffer[] = "4:abab";
	BencodedDict b;
	ASSERT_TRUE(BencEntity::Parse(buffer, b, buffer + sizeof(buffer)-1) != NULL);
	ASSERT_EQ(BENC_STR, b.GetType());
	std::string serialized = b.Serialize();
	EXPECT_EQ(sizeof(buffer) - 1, serialized.size());
	EXPECT_EQ(0, memcmp(serialized.c_str(), buffer, sizeof(buffer) - 1));
}

TEST(Bencoding, DISABLED_InfoSection) {
	unsigned char buffer[32768];
	memset(buffer, 0, 32768);
	FILE* fp = fopen("./unittests/info.benc", "rb");
	assert(fp);
	size_t read = fread(buffer, sizeof(char), 32768, fp);
	assert(read < 32768);
	BencodedDict b;
	ASSERT_TRUE(BencEntity::Parse(buffer, b, buffer + read) != NULL);
	ASSERT_EQ(BENC_DICT, b.GetType());
	std::string serialized = b.Serialize();
	EXPECT_EQ(read, serialized.size());
	EXPECT_EQ(0, memcmp(serialized.c_str(), buffer, read));
}

TEST(Bencoding, Torrent) {
	BencodedDict dict;
	ASSERT_TRUE(BencEntity::Parse((const unsigned char*)unittest::torrent_data,
		dict, (const unsigned char*)(unittest::torrent_data + 17115)) != NULL);
	ASSERT_EQ(BENC_DICT, dict.GetType());
	ASSERT_EQ(static_cast<size_t>(17115), sizeof(unittest::torrent_data));
	std::string serialized = dict.Serialize();
	EXPECT_EQ(static_cast<size_t>(17115), serialized.size());
	EXPECT_EQ(0, memcmp(serialized.c_str(), unittest::torrent_data, serialized.size()));
}

TEST(Bencoding, TorrentWithRegion) {
	BencodedDict dict;
	unittest::Parse((const unsigned char*)unittest::torrent_data, 17115, dict);
}

TEST(Bencoding, NestedDictionarySerialize) {
	unsigned char buffer[] = "d1:ad1:b1:cee";
	BencodedDict b;
	ASSERT_TRUE(BencEntity::Parse(buffer, b, buffer + sizeof(buffer)-1) != NULL);
	ASSERT_EQ(BENC_DICT, b.GetType());
	std::string serialized = b.Serialize();
	EXPECT_EQ(sizeof(buffer) - 1, serialized.size());
	EXPECT_EQ(0, memcmp(buffer, serialized.c_str(), serialized.size()));
}

TEST(Bencoding, LongInt) {
	BencodedDict ent;
	const unsigned char dict[] = "d1:ki12970376315ee";
	ASSERT_TRUE(BencEntity::Parse(dict, ent, dict + sizeof(dict)-1) != NULL);
	BencEntity* value = ent.Get("k");
	EXPECT_EQ(BENC_BIGINT, value->GetType());
	EXPECT_EQ(12970376315, value->GetInt64(0));
	std::string p = ent.Serialize();
	EXPECT_EQ(p.size(), sizeof(dict) - 1);
	EXPECT_EQ(0, memcmp(dict, p.c_str(), sizeof(dict) - 1));
}

TEST(Bencoding, Settings1) {
	// Follows general parsing
	BencodedDict dict;

	std::pair<unsigned char*, unsigned char*> rgn(NULL,NULL);
	// parse the settings
	ASSERT_TRUE(BencEntity::Parse((const unsigned char*)unittest::settings_data,
		dict, ((const unsigned char*)unittest::settings_data) + SETTINGS_LENGTH, "info", &rgn) != NULL);
	//the below fails.  why???
//	ASSERT_EQ(static_cast<unsigned long>(SETTINGS_LENGTH), sizeof(unittest::settings_data));
	ASSERT_EQ(BENC_DICT, dict.GetType());

	std::string b = dict.Serialize();
	EXPECT_EQ(static_cast<size_t>(SETTINGS_LENGTH), b.size());
	//EXPECT_EQ(sizeof(unittest::settings_data), len);
	EXPECT_EQ(0, memcmp(unittest::settings_data, b.c_str(), SETTINGS_LENGTH));
	//EXPECT_EQ(0, memcmp(unittest::settings_data, b, sizeof(unittest::settings_data)));
}

TEST(Bencoding, Settings2) {
	// Follows general parsing
	BencodedDict dict;

	std::pair<unsigned char*, unsigned char*> rgn(NULL,NULL);
	// parse the settings
	ASSERT_TRUE(BencEntity::Parse((const unsigned char*)unittest::settings_data, dict,
		((const unsigned char*)unittest::settings_data) + SETTINGS_LENGTH, "info", &rgn) != NULL);
	ASSERT_TRUE(dict.GetType() == BENC_DICT);
	std::string b = dict.Serialize();
	EXPECT_EQ(static_cast<size_t>(SETTINGS_LENGTH), b.size());
	EXPECT_EQ(0, memcmp(unittest::settings_data, b.c_str(), SETTINGS_LENGTH));
	//for(int i = 0; i < SETTINGS_LENGTH; i++)
	//	if( unittest::settings_data[i] != b[i] )
	//		break;
}

TEST(Bencoding, Settings3) {
	// Follows general parsing
	BencodedDict dict;

	// parse the settings
	ASSERT_TRUE(BencEntity::ParseInPlace((unsigned char*)unittest::settings_data, dict,
		((const unsigned char*)unittest::settings_data) + SETTINGS_LENGTH) != NULL);
	ASSERT_TRUE(dict.GetType() == BENC_DICT);
	std::string b = dict.Serialize();
	EXPECT_EQ(static_cast<size_t>(SETTINGS_LENGTH), b.size());
	EXPECT_EQ(0, memcmp(unittest::settings_data, b.c_str(), SETTINGS_LENGTH));
}

TEST(Bencoding, Copy) {
	static const char sample_data[] = "d7:astring6:avalue5:aznumi10e6:newstr10:0123456789e";
	// sizeof(sample_data) includes the terminating zero byte
	static const size_t sample_len = sizeof(sample_data) - 1;
	unsigned char* tmp = static_cast<unsigned char*>(malloc(sample_len));
	ASSERT_TRUE(tmp != NULL);
	memcpy(tmp, sample_data, sample_len);

	// Copy and verify a dict
	BencodedDict dict;
	bool ok = (BencEntity::Parse(tmp, dict, tmp + sample_len)) ? true : false;
	ASSERT_TRUE(ok);
	tstring value = dict.GetStringT("astring");
	EXPECT_FALSE(value.empty());
	BencodedDict* d2 = new BencodedDict();
	ASSERT_TRUE(d2 != NULL);
	d2->CopyFrom(dict);
//	EXPECT_TRUE(d2->inplace == false);
	std::string bytes = d2->Serialize();
	EXPECT_EQ(sample_len, bytes.size());
	/*utassert_failmsg(serialized_len == sample_len,
		utlogf("serialized_len %d sample_len %d",
		serialized_len, sample_len));*/
	EXPECT_EQ(0, memcmp(sample_data, bytes.c_str(), sample_len));
	/*utassert_failmsg(memcmp(sample_data, (const char*)bytes, sample_len) == 0,
		utlogf("sample_data %s bytes %s", sample_data, bytes));*/
	delete d2;
	d2 = NULL;

	// Copy and verify a string
	static cstr testString = "foobar";
	size_t testStringLength = strlen(testString);
	BencEntityMem s;
	s.SetStr(testString);
	BencEntity* s2 = new BencEntity();
	s2->CopyFrom(s);
	EXPECT_EQ(BENC_STR, s2->GetType());
	size_t len;
	const char* copied = ((BencEntityMem *) s2)->GetString(&len);
	EXPECT_EQ(testStringLength, len);
	EXPECT_EQ(0, strncmp(testString, copied, len));
	delete s2;
	s2 = NULL;

	free((void*)tmp);
}

TEST(Bencoding, DeleteStringsFromList) {
	BencodedList list;
	static const unsigned char sample_data[] = "l1:a1:b1:c1:de";
	static const size_t sample_len = sizeof(sample_data) - 1;
	bool ok = (BencEntity::Parse(sample_data, list, sample_data + sample_len)) ? true : false;
	ASSERT_TRUE(ok);
	while(list.GetCount() > 0) {
		EXPECT_EQ(BENC_STR, list.Get(0)->GetType());
		list.Delete(0);
	}
}
