#include <math.h>
//#undef _M_CEE_PURE
//#undef new

#include <iostream>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#ifdef ANDROID
#include "../util.h"
#else
#include "util.h"
#endif // ANDROID
#include "bencoding.h"
//#include "btstr.h"
#include "sockaddr.h"
//#include "strfmt.h"
//#include "sha.h"
#include "ut_bencoding_data.h"

#define utlogf(fmt,...) string_fmt(fmt ,##__VA_ARGS__)
//#define utassert_failmsg(x,log) EXPECT_TRUE(x) << log

std::ostream &operator << (std::ostream &stream, const std::string &other) {
	stream << other;
	return stream;
}

namespace unittest {
#define SETTINGS_LENGTH 0x50dc


bool Parse(const unsigned char *p, uint len, BencodedDict &base)
{
	EXPECT_TRUE(p);
	EXPECT_TRUE(len);

	std::pair<unsigned char*, unsigned char*> rgn = {NULL,NULL};

	// parse the torrent file
//	EXPECT_TRUE(base.GetType() == BENC_VOID);
	EXPECT_TRUE(BencEntity::Parse(p, base, p + len, "info", &rgn));
	EXPECT_EQ(BENC_DICT, base.GetType());

	// Bail out of the test here if the parsing went wrong
	if( base.GetType() != BENC_DICT )
		return false;

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
	return true;
}
}

TEST(Bencoding, Basics) {
	static const char sample_scrape[] = "d5:filesd20:7\247!\365'\221\335\320\352\305Q\343\200\344qf\220\316\346\371d8:completei8e10:downloadedi16838e10:incompletei5e4:name17:Fedora-8-Live-ppce20:]\341\022\010E\230\356k\223\363\260`$w\327\357\321\264v2d8:completei36e10:downloadedi45649e10:incompletei8e4:name22:Fedora-8-Live-KDE-i686e20:\301\234\352<DC\367\273\017\342\1\353\306\303\221\257tJ\254\251d8:completei24e10:downloadedi29004e10:incompletei2e4:name20:Fedora-8-Live-x86_64eee";
	static const char* keys[] = {
		"7\247!\365'\221\335\320\352\305Q\343\200\344qf\220\316\346\371",
		"]\341\022\010E\230\356k\223\363\260`$w\327\357\321\264v2",
		"\301\234\352<DC\367\273\017\342\1\353\306\303\221\257tJ\254\251"
	};

	unsigned char* tmp = static_cast<unsigned char*>(malloc(sizeof(sample_scrape)));
	EXPECT_TRUE(tmp);
	memcpy(tmp, sample_scrape, sizeof(sample_scrape));
	BencodedDict dict;
	bool ok = (BencEntity::ParseInPlace(tmp, dict, tmp + sizeof(sample_scrape))) ? true : false;
	EXPECT_TRUE(ok);

	EXPECT_EQ(BENC_DICT, dict.GetType());
	BencodedDict* files = dict.GetDict("files");
	EXPECT_TRUE(files);
	EXPECT_EQ(static_cast<size_t>(3), files->GetCount());
	for(int i = 0; i < 3; i++) {
		EXPECT_TRUE(files->HasKey(keys[i]));
	}
	if (files) {
		BencodedEntityMap::iterator it = files->dict->begin();
		BencodedDict* d = (BencodedDict*) &it->second;
		EXPECT_TRUE(d);
		EXPECT_EQ(BENC_DICT, d->bencType);
		EXPECT_EQ(8, d->GetInt("complete", -99));
		EXPECT_EQ(16838, d->GetInt("downloaded", -99));
		EXPECT_EQ(5, d->GetInt("incomplete", -99));
		EXPECT_STREQ("Fedora-8-Live-ppc", d->GetString("name"));

		it++;
		d = (BencodedDict*) (BencodedDict*) &it->second;
		EXPECT_TRUE(d);
		EXPECT_EQ(BENC_DICT, d->bencType);
		EXPECT_EQ(36, d->GetInt("complete", -99));
		EXPECT_EQ(45649, d->GetInt("downloaded", -99));
		EXPECT_EQ(8, d->GetInt("incomplete", -99));
		EXPECT_STREQ("Fedora-8-Live-KDE-i686", d->GetString("name"));

		it++;
		d = (BencodedDict*) (BencodedDict*) &it->second;
		EXPECT_TRUE(d);
		EXPECT_EQ(BENC_DICT, d->bencType);
		EXPECT_EQ(24, d->GetInt("complete", -99));
		EXPECT_EQ(29004, d->GetInt("downloaded", -99));
		EXPECT_EQ(2, d->GetInt("incomplete", -99));
		EXPECT_STREQ("Fedora-8-Live-x86_64", d->GetString("name"));
	}

	free(tmp);

	static const char sample_tracker[] = "d8:completei36e10:incompletei10e8:intervali1800e5:peers108:E\0210\236\032\342J)K\5'\315D\222\216\026\030:\230\3\334\245\032\341:\010F+N/\301\331}\245\363\235\022\7\031\241\032\341O\317\232\203\260\017Y\365b\022&\232\325\263\344\6\267\"\200\257\015c\032\341U\036\2329\032\341W\3+S\303V\303\270\311\n\032\342N^8CA\361Xz\375Nd\325\246R\t,\032\337Ov\013$\314\377e";

	tmp = static_cast<unsigned char*>(malloc(sizeof(sample_tracker) * sizeof(char)));
	EXPECT_TRUE(tmp);
	memcpy(tmp, sample_tracker, sizeof(sample_tracker));
	BencodedDict dict2;
	ok = (BencEntity::ParseInPlace(tmp, dict2, tmp + sizeof(sample_tracker))) ? true : false;
	EXPECT_TRUE(ok);

	EXPECT_EQ(BENC_DICT, dict2.GetType());
	EXPECT_EQ(1800, dict2.GetInt("interval", -99));
	EXPECT_EQ(36, dict2.GetInt("complete", -99));
	EXPECT_EQ(10, dict2.GetInt("incomplete", -99));
	EXPECT_TRUE(!dict2.GetList("peers"));
	size_t len;
	cstr pbin = dict2.GetString("peers", &len);
	EXPECT_EQ(static_cast<size_t>(108), len);
	EXPECT_TRUE(pbin);

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
	EXPECT_TRUE(tmp);
	memcpy(tmp, sample_data, sizeof(sample_data));
	BencodedDict dictinplace;
	BencodedDict dict;
	bool ok = (BencEntity::Parse(tmp, dict, tmp + sizeof(sample_data))) ? true : false;
	EXPECT_TRUE(ok);
	tstring value(dict.GetStringT("astring"));
	EXPECT_TRUE(value.size());
	if (value.size()) {
		size_t valuelen = value.size();
		EXPECT_EQ(static_cast<size_t>(6), valuelen);
		/*utassert_failmsg(6 == valuelen,
			utlogf("Value and length expected %s %d actual %s %d",
				"avalue", 6, value.c_str(), valuelen));*/
	}
	ok = (BencEntity::ParseInPlace(tmp, dictinplace, tmp + sizeof(sample_data))) ? true : false;
	EXPECT_TRUE(ok);
	value = dictinplace.GetStringT("astring");
	EXPECT_TRUE(value.size());
	if (value.size()) {
		size_t valuelen = value.size();
		EXPECT_EQ(static_cast<size_t>(6), valuelen);
		/*utassert_failmsg(6 == valuelen,
			utlogf("Value and length expected %s %d actual %s %d",
				"avalue", 6, value.c_str(), valuelen));*/
	}
}

TEST(Bencoding, String2) {
	static const char sample_data[] = "d7:astring6:avalue5:aznumi10e6:newstr10:0123456789e";

	unsigned char* tmp = static_cast<unsigned char*>(malloc(sizeof(sample_data)));
	EXPECT_TRUE(tmp);
	memcpy(tmp, sample_data, sizeof(sample_data));
	BencodedDict dictinplace;
	BencodedDict dict;
	bool ok = (BencEntity::Parse(tmp, dict, tmp + sizeof(sample_data))) ? true : false;
	EXPECT_TRUE(ok);
	tstring value = dict.GetStringT("astring");
	EXPECT_TRUE(value.size());
	if (value.size()) {
		size_t valuelen = value.size();
		EXPECT_EQ(static_cast<size_t>(6), valuelen);
		/*utassert_failmsg(6 == valuelen,
			utlogf("Value and length expected %s %d actual %s %d",
				"avalue", 6, value.c_str(), valuelen));*/
	}
	ok = (BencEntity::ParseInPlace(tmp, dictinplace, tmp + sizeof(sample_data))) ? true : false;
	EXPECT_TRUE(ok);
	value = dictinplace.GetStringT("astring");
	EXPECT_TRUE(value.size());
	if (value.size()) {
		size_t valuelen = value.size();
		EXPECT_EQ(static_cast<size_t>(6), valuelen);
		/*utassert_failmsg(6 == valuelen,
			utlogf("Value and length expected %s %d actual %s %d",
				"avalue", 6, value.c_str(), valuelen));*/
	}
}

/*
TEST(Bencoding, TwoFiles) {
	unsigned char buffer[32768];
	FILE* fp = fopen("./unittests/files2.benc", "rb");
	assert(fp);
}

TEST(Bencoding, AllFiles) {
}

TEST(Bencoding, InfoSection) {
}
*/

TEST(Bencoding, Torrent) {
	BencodedDict dict;
	bool parsed = unittest::Parse((const unsigned char*)unittest::torrent_data, 17115, dict);
	EXPECT_TRUE(parsed);
}

TEST(Bencoding, Settings1) {
	// Follows general parsing
	BencodedDict dict;

	std::pair<unsigned char*, unsigned char*> rgn = {NULL,NULL};
	// parse the settings
	EXPECT_TRUE(BencEntity::Parse((const unsigned char*)unittest::settings_data,
											dict,
											((const unsigned char*) unittest::settings_data) + SETTINGS_LENGTH, "info", &rgn));
	EXPECT_EQ(BENC_DICT, dict.GetType());
	if (dict.GetType() != BENC_DICT)
		return;

	size_t len = 0;
	unsigned char *b = dict.Serialize(&len);
	EXPECT_EQ(static_cast<size_t>(SETTINGS_LENGTH), len);
	EXPECT_EQ(0, memcmp(unittest::settings_data, b, SETTINGS_LENGTH));
	free( b );
}

TEST(Bencoding, Settings2) {
	// Follows general parsing
	BencodedDict dict;

	std::pair<unsigned char*, unsigned char*> rgn = {NULL,NULL};
	// parse the settings
	EXPECT_TRUE(BencEntity::Parse((const unsigned char*)unittest::settings_data,
											dict,
 											((const unsigned char*) unittest::settings_data) + SETTINGS_LENGTH, "info", &rgn));
//	EXPECT_TRUE(dict.GetType() == BENC_DICT);
	if (dict.GetType() != BENC_DICT)
		return;
	size_t len = 0;
	unsigned char *b = dict.Serialize(&len);
	EXPECT_EQ(static_cast<size_t>(SETTINGS_LENGTH), len);
	EXPECT_EQ(0, memcmp(unittest::settings_data, b, SETTINGS_LENGTH));
	//for(int i = 0; i < SETTINGS_LENGTH; i++)
	//	if( unittest::settings_data[i] != b[i] )
	//		break;
	free( b );
}

TEST(Bencoding, Settings3) {
	// Follows general parsing
	BencodedDict dict;

	// parse the settings
	EXPECT_TRUE(BencEntity::ParseInPlace((unsigned char*)unittest::settings_data,
											dict,
 											((const unsigned char*) unittest::settings_data) + SETTINGS_LENGTH));
//	EXPECT_TRUE(dict.GetType() == BENC_DICT);
	if (dict.GetType() != BENC_DICT)
		return;
	size_t len = 0;
	unsigned char *b = dict.Serialize(&len);
	EXPECT_EQ(static_cast<size_t>(SETTINGS_LENGTH), len);
	EXPECT_EQ(0, memcmp(unittest::settings_data, b, SETTINGS_LENGTH));
	//for(int i = 0; i < SETTINGS_LENGTH; i++)
	//	if( unittest::settings_data[i] != b[i] )
	//		break;
	free( b );
}

TEST(Bencoding, Copy) {
	static const char sample_data[] = "d7:astring6:avalue5:aznumi10e6:newstr10:0123456789e";
	// sizeof(sample_data) includes the terminating zero byte
	static const size_t sample_len = sizeof(sample_data) - 1;
	unsigned char* tmp = static_cast<unsigned char*>(malloc(sample_len));
	EXPECT_TRUE(tmp);
	memcpy(tmp, sample_data, sample_len);

	// Copy and verify a dict
	BencodedDict dict;
	bool ok = (BencEntity::Parse(tmp, dict, tmp + sample_len)) ? true : false;
	EXPECT_TRUE(ok);
	tstring value = dict.GetStringT("astring");
	EXPECT_TRUE(value.size());
	BencodedDict* d2 = new BencodedDict();
	EXPECT_TRUE(d2);
	d2->CopyFrom(dict);
//	EXPECT_TRUE(d2->inplace == false);
	size_t serialized_len;
	unsigned char* bytes = d2->Serialize(&serialized_len);
	EXPECT_EQ(sample_len, serialized_len);
	/*utassert_failmsg(serialized_len == sample_len,
		utlogf("serialized_len %d sample_len %d",
		serialized_len, sample_len));*/
	EXPECT_EQ(0, memcmp(sample_data, (const char*)bytes, sample_len));
	/*utassert_failmsg(memcmp(sample_data, (const char*)bytes, sample_len) == 0,
		utlogf("sample_data %s bytes %s", sample_data, bytes));*/
	delete d2;
	d2 = NULL;

	// Copy and verify a string
	BencEntityMem s;
	s.SetStr("foobar");
	BencEntity* s2 = new BencEntity();
	s2->CopyFrom(s);
	EXPECT_EQ(BENC_STR, s2->GetType());
	size_t len;
	EXPECT_EQ(0, strcmp("foobar", ((BencEntityMem *) s2)->GetString(&len)));
	delete s2;
	s2 = NULL;

	free((void*)tmp);
}
