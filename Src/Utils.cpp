/*
 * Utils.c
 *
 *  Created on: Aug 12, 2020
 *      Author: Tanguy
 */

#include "Utils.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <string>
#include <vector>
#include <cctype>
#include <cstring>
#include <sys/time.h>
#include "AncvPrintTicket.hpp"
#include "YesNoWindow.hpp"

using namespace cib;
using namespace cib::communication::http;
using namespace cib::json;

// ---------- Utils ----------
static std::string toHex(const std::vector<unsigned char>& data) {
    static const char* hex = "0123456789abcdef";
    std::string out; out.reserve(data.size()*2);
    for (size_t i=0;i<data.size();++i){ unsigned char b=data[i];
        out.push_back(hex[b>>4]); out.push_back(hex[b&0x0F]); }
    return out;
}
static std::string ivToSignedDecimalCSV(const std::vector<unsigned char>& iv) {
    std::ostringstream oss;
    for (size_t i=0;i<iv.size();++i) {
        int v = (iv[i] <= 127) ? int(iv[i]) : int(iv[i]) - 256;
        if (i) oss << ",";
        oss << v;
    }
    return oss.str();
}
static std::vector<unsigned char> pkcs7PadToBlock(const std::vector<unsigned char>& in, size_t blockSize) {
    size_t rem = in.size() % blockSize;
    size_t padLen = (rem==0) ? blockSize : (blockSize-rem);
    std::vector<unsigned char> out(in);
    out.resize(in.size()+padLen, static_cast<unsigned char>(padLen));
    return out;
}

// ---------- AES-256 (ECB block) + CBC glue (no padding) ----------
// Implémentation minimale inspirée des descriptions FIPS-197 (S-Box, Rcon, MixColumns).

namespace aes256_min {

static const unsigned char sbox[256] = {
0x63,0x7c,0x77,0x7b,0xf2,0x6b,0x6f,0xc5,0x30,0x01,0x67,0x2b,0xfe,0xd7,0xab,0x76,
0xca,0x82,0xc9,0x7d,0xfa,0x59,0x47,0xf0,0xad,0xd4,0xa2,0xaf,0x9c,0xa4,0x72,0xc0,
0xb7,0xfd,0x93,0x26,0x36,0x3f,0xf7,0xcc,0x34,0xa5,0xe5,0xf1,0x71,0xd8,0x31,0x15,
0x04,0xc7,0x23,0xc3,0x18,0x96,0x05,0x9a,0x07,0x12,0x80,0xe2,0xeb,0x27,0xb2,0x75,
0x09,0x83,0x2c,0x1a,0x1b,0x6e,0x5a,0xa0,0x52,0x3b,0xd6,0xb3,0x29,0xe3,0x2f,0x84,
0x53,0xd1,0x00,0xed,0x20,0xfc,0xb1,0x5b,0x6a,0xcb,0xbe,0x39,0x4a,0x4c,0x58,0xcf,
0xd0,0xef,0xaa,0xfb,0x43,0x4d,0x33,0x85,0x45,0xf9,0x02,0x7f,0x50,0x3c,0x9f,0xa8,
0x51,0xa3,0x40,0x8f,0x92,0x9d,0x38,0xf5,0xbc,0xb6,0xda,0x21,0x10,0xff,0xf3,0xd2,
0xcd,0x0c,0x13,0xec,0x5f,0x97,0x44,0x17,0xc4,0xa7,0x7e,0x3d,0x64,0x5d,0x19,0x73,
0x60,0x81,0x4f,0xdc,0x22,0x2a,0x90,0x88,0x46,0xee,0xb8,0x14,0xde,0x5e,0x0b,0xdb,
0xe0,0x32,0x3a,0x0a,0x49,0x06,0x24,0x5c,0xc2,0xd3,0xac,0x62,0x91,0x95,0xe4,0x79,
0xe7,0xc8,0x37,0x6d,0x8d,0xd5,0x4e,0xa9,0x6c,0x56,0xf4,0xea,0x65,0x7a,0xae,0x08,
0xba,0x78,0x25,0x2e,0x1c,0xa6,0xb4,0xc6,0xe8,0xdd,0x74,0x1f,0x4b,0xbd,0x8b,0x8a,
0x70,0x3e,0xb5,0x66,0x48,0x03,0xf6,0x0e,0x61,0x35,0x57,0xb9,0x86,0xc1,0x1d,0x9e,
0xe1,0xf8,0x98,0x11,0x69,0xd9,0x8e,0x94,0x9b,0x1e,0x87,0xe9,0xce,0x55,0x28,0xdf,
0x8c,0xa1,0x89,0x0d,0xbf,0xe6,0x42,0x68,0x41,0x99,0x2d,0x0f,0xb0,0x54,0xbb,0x16 };

static const unsigned char Rcon[15] = {
0x00,0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1b,0x36,0x6c,0xd8,0xab,0x4d
};

static inline unsigned char xtime(unsigned char x){ return (unsigned char)((x<<1) ^ ((x & 0x80)?0x1b:0x00)); }
static inline unsigned char mul(unsigned char x, unsigned char y){
    unsigned char r=0; while(y){ if(y&1) r^=x; x=xtime(x); y>>=1; } return r;
}

struct AES256Ctx {
    unsigned char roundKey[240]; // 15*16 max, we use 14 rounds => 240 bytes key schedule
};

static void keyExpansion256(const unsigned char key[32], AES256Ctx& ctx) {
    unsigned char* rk = ctx.roundKey;
    for (int i=0;i<32;++i) rk[i]=key[i];
    int bytesGen = 32;
    int rconIter = 1;
    unsigned char temp[4];
    while (bytesGen < 240) {
        for (int i=0;i<4;++i) temp[i]=rk[bytesGen-4+i];
        if (bytesGen % 32 == 0) {
            // RotWord
            unsigned char t = temp[0]; temp[0]=temp[1]; temp[1]=temp[2]; temp[2]=temp[3]; temp[3]=t;
            // SubWord
            temp[0]=sbox[temp[0]]; temp[1]=sbox[temp[1]]; temp[2]=sbox[temp[2]]; temp[3]=sbox[temp[3]];
            // Rcon
            temp[0] ^= Rcon[rconIter++];
        } else if (bytesGen % 32 == 16) {
            temp[0]=sbox[temp[0]]; temp[1]=sbox[temp[1]]; temp[2]=sbox[temp[2]]; temp[3]=sbox[temp[3]];
        }
        for (int i=0;i<4;++i) {
            rk[bytesGen] = rk[bytesGen-32] ^ temp[i];
            ++bytesGen;
        }
    }
}

static void addRoundKey(unsigned char state[16], const unsigned char* rk) {
    for (int i=0;i<16;++i) state[i]^=rk[i];
}

static void subBytes(unsigned char s[16]) {
    for (int i=0;i<16;++i) s[i]=sbox[s[i]];
}

static void shiftRows(unsigned char s[16]) {
    unsigned char t;

    // row1 shift 1
    t=s[1]; s[1]=s[5]; s[5]=s[9]; s[9]=s[13]; s[13]=t;
    // row2 shift 2
    t=s[2]; s[2]=s[10]; s[10]=t; t=s[6]; s[6]=s[14]; s[14]=t;
    // row3 shift 3
    t=s[3]; s[3]=s[15]; s[15]=s[11]; s[11]=s[7]; s[7]=t;
}

static void mixColumns(unsigned char s[16]) {
    for (int c=0;c<4;++c) {
        int i=4*c;
        unsigned char a0=s[i], a1=s[i+1], a2=s[i+2], a3=s[i+3];
        s[i]   = (unsigned char)(mul(a0,2)^mul(a1,3)^a2^a3);
        s[i+1] = (unsigned char)(a0^mul(a1,2)^mul(a2,3)^a3);
        s[i+2] = (unsigned char)(a0^a1^mul(a2,2)^mul(a3,3));
        s[i+3] = (unsigned char)(mul(a0,3)^a1^a2^mul(a3,2));
    }
}

static void encryptBlock(const AES256Ctx& ctx, const unsigned char in[16], unsigned char out[16]) {
    unsigned char s[16];
    for (int i=0;i<16;++i) s[i]=in[i];
    const int Nr = 14;
    const unsigned char* rk = ctx.roundKey;

    addRoundKey(s, rk); // first round key (bytes 0..15)
    rk += 16;

    for (int round=1; round<Nr; ++round) {
        subBytes(s);
        shiftRows(s);
        mixColumns(s);
        addRoundKey(s, rk);
        rk += 16;
    }
    subBytes(s);
    shiftRows(s);
    addRoundKey(s, rk);

    for (int i=0;i<16;++i) out[i]=s[i];
}

// CBC no-padding: in.size() multiple de 16
static std::vector<unsigned char> encryptCBC_noPad(const AES256Ctx& ctx,
                                                   const std::vector<unsigned char>& in,
                                                   const unsigned char iv[16]) {
    if (in.size()==0 || (in.size()%16)!=0) throw std::runtime_error("CBC input not multiple of 16");
    std::vector<unsigned char> out(in.size());
    unsigned char prev[16];
    for (int i=0;i<16;++i) prev[i]=iv[i];
    for (size_t off=0; off<in.size(); off+=16) {
        unsigned char blk[16];
        for (int i=0;i<16;++i) blk[i] = (unsigned char)(in[off+i] ^ prev[i]);
        unsigned char enc[16];
        encryptBlock(ctx, blk, enc);
        for (int i=0;i<16;++i){ out[off+i]=enc[i]; prev[i]=enc[i]; }
    }
    return out;
}

} // namespace aes256_min

// ---------- API demandée ----------
// serialNumber: string, shopIdStr: string numérique
static std::string buildJbSurfAuthorize_noSSL(const std::string& serialNumber,
                                              const std::string& shopIdStr)
{
    // shopId numérique -> on laisse tel quel lors de l’assemblage final
    // clé 32 octets: 0..31
    unsigned char key[32];
    for (int i=0;i<32;++i) key[i]=(unsigned char)i;

    // IV 16 octets "aléatoire"
    std::vector<unsigned char> iv(16);
    for (size_t i=0;i<iv.size();++i) iv[i]=(unsigned char)(std::rand()%256);
    std::string ivStr = ivToSignedDecimalCSV(iv);

    // Texte clair: "jbancv38" + serialNumber
    const std::string prefix("jbancv38");
    std::vector<unsigned char> plain(prefix.begin(), prefix.end());
    plain.insert(plain.end(), serialNumber.begin(), serialNumber.end());

    // Padding 1: PKCS7 jusqu’à multiple de 16
    std::vector<unsigned char> padded = pkcs7PadToBlock(plain, 16);
    // Padding 2: ajouter un bloc complet de 16 octets valeur 0x10 (16) pour émuler PKCS5Padding Java
    padded.resize(padded.size()+16, (unsigned char)16);

    // AES-256 key schedule
    aes256_min::AES256Ctx ctx;
    aes256_min::keyExpansion256(key, ctx);

    // CBC encryption (no padding, déjà géré)
    std::vector<unsigned char> ct = aes256_min::encryptCBC_noPad(ctx, padded, &iv[0]);

    std::string encHex = toHex(ct);

    // shopId%ivCSV_hex
    std::ostringstream oss;
    oss << shopIdStr << "%" << ivStr << "_" << encHex;
    return oss.str();
}


static bool parseDdMmYyyy(const std::string& s, std::tm& outTm)
{
    if (s.size() != 10 || s[2] != '/' || s[5] != '/')
        return false;

    // Vérification manuelle des caractères
    static const size_t positions[] = {0,1,3,4,6,7,8,9};
    for (int j = 0; j < 8; ++j) {
        size_t i = positions[j];
        if (!std::isdigit(static_cast<unsigned char>(s[i])))
            return false;
    }

    // Conversion manuelle en int (pas de stoi en C++98)
    int d = (s[0]-'0')*10 + (s[1]-'0');
    int m = (s[3]-'0')*10 + (s[4]-'0');
    int y = (s[6]-'0')*1000 + (s[7]-'0')*100 + (s[8]-'0')*10 + (s[9]-'0');

    if (y < 1900 || m < 1 || m > 12 || d < 1 || d > 31)
        return false;

    // Initialisation du tm (pas de "{}" en C++98)
    std::tm tm;
    std::memset(&tm, 0, sizeof(std::tm));

    tm.tm_mday = d;
    tm.tm_mon  = m - 1;   // 0..11
    tm.tm_year = y - 1900;
    tm.tm_hour = 12;      // pour éviter les effets d'heure d'été

    // Validation avec mktime
    std::tm copy = tm;
    if (std::mktime(&copy) == -1)
        return false;

    // Vérifie que la date n’a pas été "corrigée" (ex: 31/11)
    if (copy.tm_mday != tm.tm_mday ||
        copy.tm_mon  != tm.tm_mon  ||
        copy.tm_year != tm.tm_year)
        return false;

    outTm = tm;
    return true;
}

// Formate en "YYYY-MM-DDTHH:MM:SS"
static std::string formatIso(const std::tm& tm, bool endOfDay) {
    std::tm t = tm;
    t.tm_hour = endOfDay ? 23 : 0;
    t.tm_min  = endOfDay ? 59 : 0;
    t.tm_sec  = endOfDay ? 59 : 0;

    std::ostringstream oss;
    oss << std::setfill('0')
        << std::setw(4) << (t.tm_year + 1900) << "-"
        << std::setw(2) << (t.tm_mon + 1)     << "-"
        << std::setw(2) <<  t.tm_mday         << "T"
        << std::setw(2) <<  t.tm_hour         << ":"
        << std::setw(2) <<  t.tm_min          << ":"
        << std::setw(2) <<  t.tm_sec;
    return oss.str();
}


//// --- helper: base64 char -> valeur 0..63, -1 sinon
static int b64idx(unsigned char c)
{
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    return -1;
}

// --- base64 decode (C++98)
static std::vector<unsigned char> base64Decode(const std::string& in)
{
    std::vector<unsigned char> out;
    out.reserve(in.size() * 3 / 4);

    int val = 0;       // accumulateur 24 bits
    int valb = -8;     // combien de bits valides dans val (-8, -2, 4, ...)
    int pad = 0;

    for (size_t i = 0; i < in.size(); ++i)
    {
        unsigned char c = (unsigned char)in[i];
        if (isspace(c)) continue; // ignore espaces/retours ligne

        if (c == '=') { pad++; continue; }

        int d = b64idx(c);
        if (d < 0) {
            // ignore les caractères non base64 (utile si header data:...)
            continue;
        }

        val = (val << 6) | d;
        valb += 6;
        if (valb >= 0) {
            unsigned char byte = (unsigned char)((val >> valb) & 0xFF);
            out.push_back(byte);
            valb -= 8;
        }
    }

    // gérer padding (=) : retirer 1 ou 2 bytes si nécessaire
    if (pad) {
        // Le padding signifie que les derniers octets sortis sont en trop
        // pad==1 -> enlever 1 octet ; pad==2 -> enlever 2 octets (si présents)
        while (pad-- > 0 && !out.empty()) out.pop_back();
    }
    return out;
}
//


// --- Sauvegarde le PNG à partir d'un base64 (retourne true si OK) ---
static bool saveImageBase64ToPng(const std::string& imageBase64, const char* outPath)
{
	std::vector<unsigned char> png = base64Decode(imageBase64);

	T_GL_HFILE myfile = GL_File_Open(outPath, GL_FILE_CREATE_ALWAYS, GL_FILE_ACCESS_READ_WRITE);
	if (!myfile) return false;

	// 3) écrire (attention si l’API peut écrire partiellement)
	const unsigned char* p = png.data();
	int remaining = static_cast<int>(png.size());
	int totalWritten = 0;

	while (remaining > 0) {
		int n = GL_File_Write(myfile, p + totalWritten, remaining);
		if (n <= 0) break; // erreur d'écriture
		totalWritten += n;
		remaining    -= n;
	}

	// 4) flush & close
	if (totalWritten > 0) {
		GL_File_Flush(myfile);
	}
	GL_File_Close(myfile);

	return totalWritten > 0;
}

static long loadDataParam(const string &filePath, void *bufferOut, const unsigned long bufferSize)
{
	long ret = 0;

	// Check parameters
	if (!filePath.empty() && bufferOut && bufferSize > 0)
	{
		if (File::isFileExists(filePath))
		{
			File file;
			// Open the file with read access only
			if (file.open(filePath, File::FILE_OPEN_EXISTING, File::FILE_ACCESS_READ))
			{
				size_t fileSize = file.getSize();

				ret = file.read(bufferOut, fileSize > bufferSize ? bufferSize : fileSize);
				file.close();
			}
		}
	}
	else
	{
		// Invalid parameters
		ret = -1;
	}

	return ret;
}

Utils::Utils()
{
}


int Utils::connect(bool isANCV)
{
	int iRet = LL_ERROR_OK;

	if(LL_GetStatus(m_session.getSessionHandle()) != LL_STATUS_CONNECTED)
	{
		string ssl = "ANCV";
		if(m_host == HOST_DEV)
		{
			ssl = "ANCVSTAG";
		}
		m_session.setSessionData(m_host, 443, m_cntType, ssl, m_gprs);
		if ((iRet = m_session.configAndConnect()) != LL_ERROR_OK && !isANCV)
		{
			SGL::ref().dialogMessage("Erreur", "Connexion impossible", GL_ICON_ERROR, GL_BUTTON_VALID, GL_TIME_SECOND);
		}
	}

	return iRet;
}


void Utils::resetTerminal(const string &host)
{
	this->loadData();
	cib::json::Document jsonParam;
	loadDataAsJson(FIC_PARAM, jsonParam);
	jsonParam["host"] = host;
	jsonParam["connectionType"] = (int)(connectionType::IP);
	jsonParam["ANCVOnly"] = false;
	jsonParam["shopId"] = "";
	saveDataAsJson(FIC_PARAM, jsonParam);
	m_host = host;
	SGL::ref().dialogMessage("Paramétrage", "Application remise à zéro", GL_ICON_INFORMATION, GL_BUTTON_VALID, GL_TIME_INFINITE);
}

bool Utils::getCertificate()
{
	return settings::certificate::UpdateCertificate(&m_session);
}

void Utils::disconnect()
{
	if (LL_GetStatus(m_session.getSessionHandle()) == LL_STATUS_CONNECTED)
	{
		m_session.disconnect();
	}
}

void Utils::loadData()
{
	cib::json::Document jsonParam;
	loadDataAsJson(FIC_PARAM, jsonParam);

	m_host = (string)jsonParam["host"].as_string();;
	m_cntType = (connectionType::EconnectionType)(int)jsonParam["connectionType"];
	m_gprs.apn = (string)jsonParam["gprs"]["apn"].as_string();
	m_gprs.login = (string)jsonParam["gprs"]["login"].as_string();
	m_gprs.pwd = (string)jsonParam["gprs"]["password"].as_string();

	Utils::ref().copyFromResources("/Icons/ca1.pem", "/CVCONNECT/ca1.pem");
	Utils::ref().copyFromResources("/Icons/ca2.pem", "/CVCONNECT/ca2.pem");
	Utils::ref().createSSLProfile("ANCV", 2, "/CVCONNECT/ca1.pem", "/CVCONNECT/ca2.pem");

	Utils::ref().copyFromResources("/Icons/stagec1.pem", "/CVCONNECT/stagec1.pem");
	Utils::ref().copyFromResources("/Icons/stagec2.pem", "/CVCONNECT/stagec2.pem");
	Utils::ref().createSSLProfile("ANCVSTAG", 2, "/CVCONNECT/stagec1.pem", "/CVCONNECT/stagec2.pem");
}



std::string Utils::makeId(int length)
{
	std::srand(static_cast<unsigned int>(std::time(0)));

    static const char characters[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789";

    const int charCount = sizeof(characters) - 1; // -1 pour ignorer '\0'
    std::string result;
    result.reserve(length);

    for (int i = 0; i < length; ++i)
    {
        int index = rand() % charCount;
        result += characters[index];
    }

    return result;
}

std::string Utils::makePaymentIdWithMs()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    // conversion en date/heure locale
    struct tm* tm_info = localtime(&tv.tv_sec);

    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(4) << (tm_info->tm_year + 1900)
        << std::setfill('0') << std::setw(2) << (tm_info->tm_mon + 1)
        << std::setfill('0') << std::setw(2) << tm_info->tm_mday
		<< std::setfill('0') << std::setw(2) << tm_info->tm_mday+1
        << std::setfill('0') << std::setw(2) << tm_info->tm_hour
        << std::setfill('0') << std::setw(2) << tm_info->tm_min
        << std::setfill('0') << std::setw(2) << tm_info->tm_sec
        << std::setfill('0') << std::setw(3) << (tv.tv_usec / 1000); // ms

    return oss.str();
}

std::string Utils::getTodayDate()
{
    std::time_t now = std::time(0);
    struct tm localTime;
#if defined(_WIN32)
    localtime_s(&localTime, &now);
#else
    localtime_r(&now, &localTime);
#endif

    std::ostringstream oss;
    oss << std::setw(2) << std::setfill('0') << localTime.tm_mday << "/"
        << std::setw(2) << std::setfill('0') << (localTime.tm_mon + 1) << "/"
        << (1900 + localTime.tm_year);

    return oss.str();
}


std::string Utils::getCaptureDate()
{
    // Obtenir le temps actuel
    std::time_t now = std::time(NULL);

    // Ajouter 1 jour (24h * 3600s)
    now += 24 * 60 * 60;

    // Convertir en UTC
    struct tm gmt;
#if defined(_WIN32)
    gmtime_s(&gmt, &now); // Windows
#else
    gmtime_r(&now, &gmt); // POSIX
#endif

    // Formatter manuellement
    static const char* DAYS[]   = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    static const char* MONTHS[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                   "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

    std::ostringstream oss;
    oss << DAYS[gmt.tm_wday] << ", "
        << std::setw(2) << std::setfill('0') << gmt.tm_mday << " "
        << MONTHS[gmt.tm_mon] << " "
        << (1900 + gmt.tm_year) << " "
        << std::setw(2) << std::setfill('0') << gmt.tm_hour << ":"
        << std::setw(2) << std::setfill('0') << gmt.tm_min  << ":"
        << std::setw(2) << std::setfill('0') << gmt.tm_sec
        << " GMT";

    return oss.str();
}


Response Utils::createRequest(string path, eMethod method, const string &body, bool isANCVRequest)
{
	Request request = m_session.getRequest();
	cib::json::Document jsonParam;
	loadDataAsJson(FIC_PARAM, jsonParam);
	if (this->connect(isANCVRequest) == LL_ERROR_OK)
	{
		request.editRequest(method, path, body);
		if (method == _POST)
		{
			request.addHeader("Content-Type", "application/json");

			if((!((string)jsonParam["shopId"].as_string()).empty()))
			{
				request.addHeader("jbsurfauthorize", buildJbSurfAuthorize_noSSL(Terminal::ref().SerialNumber.substr(Terminal::ref().SerialNumber.size() - 8), (string)jsonParam["shopId"].as_string()));
			}
		}

		m_session.setRequest(request);
		m_session.sendReceive();

		switch (m_session.getResponse().getStatusCode())
		{
		case 200:
			break;
		case 401:
			break;
		default:
			if(!isANCVRequest)
			{
				SGL::ref().dialogMessage("Communication", "Une erreur est survenue", GL_ICON_WARNING, GL_BUTTON_VALID, GL_TIME_SECOND);
			}
			break;
		}
	}

	return m_session.getResponse();
}

bool Utils::initQrCodePayment(long long int amount)
{
	bool bRet = false;
	Response response;
	cib::json::Document jsonBody;
	cib::json::Document jsonParam;
	cib::json::Document jsonResponse;
	string date = getCaptureDate();

	loadDataAsJson(FIC_PARAM, jsonParam);
	if(jsonParam["shopId"] != "")
	{
		waitingWindow->drawing("Génération du QRCode", Waiting);
		string id = makeId();

		jsonBody["shopId"] = (string)jsonParam["shopId"].as_string();
		jsonBody["expirationDate"] = date;
		jsonBody["serialnumber"] = Terminal::ref().SerialNumber.substr(Terminal::ref().SerialNumber.size() - 8);
		jsonBody["order"] = json::Document(json::VALUE_IS_OBJECT);
		jsonBody["order"]["id"] = id;
		jsonBody["order"]["paymentId"] = makePaymentIdWithMs();
		jsonBody["order"]["amount"] = json::Document(json::VALUE_IS_OBJECT);
		jsonBody["order"]["amount"]["total"] = amount;
		jsonBody["order"]["amount"]["currency"] = "978";
		jsonBody["paymentMethod"] = json::Document(json::VALUE_IS_OBJECT);
		jsonBody["paymentMethod"]["tspdMode"] = "001";
		jsonBody["paymentMethod"]["captureMode"] = "NORMAL";
		jsonBody["paymentMethod"]["captureTerm"] = 1;
		// Effectuer la requête POST pour obtenir un nouveau token
		response = createRequest("/Webservices/rest/FO/wcfANCVPeriph.svc/InitAndGetQRPreTransac", _POST, jsonBody.serialize());
		if (response.getStatusCode() == 200)
		{
			jsonResponse.parse(response.getContent().data());
			if((string)jsonResponse["InitAndGetQRPreTransacResult"]["ImageBase64"].as_string() != "")
			{
				jsonParam["orderId"] = (string)jsonResponse["InitAndGetQRPreTransacResult"]["TransactionId"].as_string();
//				jsonParam["orderId"] = id;
				jsonParam["paymentId"] = (string)jsonBody["order"]["paymentId"].as_string();
				jsonParam["orderDate"] = date;
				saveDataAsJson(FIC_PARAM, jsonParam);
				bRet = saveImageBase64ToPng((string)jsonResponse["InitAndGetQRPreTransacResult"]["ImageBase64"].as_string(), "file://flash/HOST/QRCODE.png");
			}
		}
		if(!bRet)
		{
			string msg = "QrCode invalide";
			if(response.getStatusCode() != 200)
			{
				string msg = "Erreur réseau";
			}
			waitingWindow->drawing(msg, Cancel);
		}
		else
		{
			waitingWindow->hidding();
		}
	}

	return bRet;
}

bool Utils::sendMiseEnPaiementTransac(string beneficiaryId, long long int amount)
{
	bool bRet = false;
	Response response;
	cib::json::Document jsonBody;
	cib::json::Document jsonParam;
	cib::json::Document jsonResponse;

	loadDataAsJson(FIC_PARAM, jsonParam);

	if(jsonParam["shopId"] != "")
	{
		Utils::waitingWindow->drawing("OPERATION EN COURS", Waiting);
		string id = makeId();
		string date = getCaptureDate();
		// Construction du corps de la requête avec les informations d'authentification
		jsonBody["beneficiaryId"] = beneficiaryId;
		jsonBody["shopId"] = (string)jsonParam["shopId"].as_string();
		jsonBody["serialnumber"] = Terminal::ref().SerialNumber.substr(Terminal::ref().SerialNumber.size() - 8);
		jsonBody["order"] = json::Document(json::VALUE_IS_OBJECT);
		jsonBody["order"]["id"] = id;
		jsonBody["order"]["paymentId"] = makePaymentIdWithMs();
		jsonBody["order"]["amount"] = json::Document(json::VALUE_IS_OBJECT);
		jsonBody["order"]["amount"]["total"] = amount;
		jsonBody["order"]["amount"]["currency"] = "978";
		jsonBody["paymentMethod"] = json::Document(json::VALUE_IS_OBJECT);
		jsonBody["paymentMethod"]["tspdMode"] = "001";
		jsonBody["paymentMethod"]["captureMode"] = "NORMAL";
		jsonBody["paymentMethod"]["captureDate"] = date;
		// Effectuer la requête POST pour obtenir un nouveau token
		response = createRequest("/Webservices/rest/FO/wcfANCVPeriph.svc/InitAndMiseEnPaiementTransac", _POST, jsonBody.serialize());
		if (response.getStatusCode() == 200)
		{
			jsonResponse.parse(response.getContent().data());
			if(!(((string)jsonResponse["InitAndMiseEnPaiementTransacResult"].as_string()).empty()))
			{
				jsonParam["orderId"] = (string)jsonResponse["InitAndMiseEnPaiementTransacResult"].as_string();
				jsonParam["orderDate"] = date;
				jsonParam["paymentId"] = (string)jsonBody["order"]["paymentId"].as_string();
				saveDataAsJson(FIC_PARAM, jsonParam);
				bRet = true;
			}
		}
	}

	if(!bRet)
	{
		string msg = "Une erreur réseau est survenue";
		if(response.getStatusCode() == 200)
		{
			msg = "ShopID incorrect";
			jsonParam["shopId"] = "";
			isConnected = false;
			saveDataAsJson(FIC_PARAM, jsonParam);
		}

		Utils::waitingWindow->drawing(msg, Cancel);
	}

	return bRet;
}

int Utils::pollingPreTransacResult()
{
	int iRet = 0;
	Response response;
	cib::json::Document jsonBody;
	cib::json::Document jsonParam;
	cib::json::Document jsonResponse;

	loadDataAsJson(FIC_PARAM, jsonParam);
	if(jsonParam["shopId"] != "")
	{

		jsonBody["id"] = jsonParam["orderId"];

		// Effectuer la requête POST pour obtenir un nouveau token
		response = createRequest("/Webservices/rest/FO/wcfANCVPeriph.svc/PollingPreTransac", _POST, jsonBody.serialize());
		if (response.getStatusCode() == 200)
		{
			jsonResponse.parse(response.getContent().data());
			string etat = (string)jsonResponse["PollingPreTransacResult"]["etat"].as_string();
			string beneficiaryId = (string)jsonResponse["PollingPreTransacResult"]["beneficiary_id"].as_string();

			if(etat != "error")
			{
				jsonParam["beneficiaryId"] = beneficiaryId;
				jsonParam["lastState"] = etat;
				saveDataAsJson(FIC_PARAM, jsonParam);
				if(etat == "SCANNED")
				{
					iRet = 201;
				}
				else if(etat == "VALIDATED")
				{
					iRet = 202;
					if((int)jsonResponse["PollingPreTransacResult"]["total"].as_int() < (int)jsonParam["amountToPay"].as_int()
										&& (bool)!jsonParam["ANCVOnly"].as_bool() )
					{
						int valTotal = (int)jsonParam["amountToPay"].as_int();
						int valPaid = (int)jsonResponse["PollingPreTransacResult"]["total"].as_int();
						jsonParam["toComplete"] = valTotal - valPaid;
						saveDataAsJson(FIC_PARAM, jsonParam);
					}
					else if((int)jsonResponse["PollingPreTransacResult"]["total"].as_int() < (int)jsonParam["amountToPay"].as_int())
					{
						iRet = 203;
					}
				}
				else if(etat == "ABORTED" || etat == "REJECTED")
				{
					iRet = 203;
				}
				else if(etat == "TIMEOUT")
				{
					iRet = 204;
				}
				else if(etat == "ERROR")
				{
					iRet = 205;
				}
			}

		}
		else
		{
			iRet = 205;
		}
	}

	return iRet;
}

int Utils::pollingTransacResult()
{
	int iRet = 0;
	Response response;
	cib::json::Document jsonBody;
	cib::json::Document jsonParam;
	cib::json::Document jsonResponse;

	loadDataAsJson(FIC_PARAM, jsonParam);
	if(jsonParam["shopId"] != "")
	{

		jsonBody["id"] = jsonParam["orderId"];

		// Effectuer la requête POST pour obtenir un nouveau token
		response = createRequest("/Webservices/rest/FO/wcfANCVPeriph.svc/PollingTransac", _POST, jsonBody.serialize());
		if (response.getStatusCode() == 200)
		{
			jsonResponse.parse(response.getContent().data());
			if((cib::json::Document)jsonResponse["PollingTransacResult"].as_document())
			{
				string etat = (string)jsonResponse["PollingTransacResult"]["etat"].as_string();
				string beneficiaryId = (string)jsonResponse["PollingTransacResult"]["beneficiary_id"].as_string();

				if(etat != "error")
				{

					jsonParam["beneficiaryId"] = beneficiaryId;
					jsonParam["lastState"] = etat;
					saveDataAsJson(FIC_PARAM, jsonParam);
					if(etat == "SCANNED")
					{
						iRet = 201;
					}
					else if(etat == "VALIDATED")
					{
						iRet = 202;
						if((int)jsonResponse["PollingTransacResult"]["total"].as_int() < (int)jsonParam["amountToPay"].as_int()
											&& (bool)!jsonParam["ANCVOnly"].as_bool() )
						{
							int valTotal = (int)jsonParam["amountToPay"].as_int();
							int valPaid = (int)jsonResponse["PollingTransacResult"]["total"].as_int();
							jsonParam["toComplete"] = valTotal - valPaid;

							saveDataAsJson(FIC_PARAM, jsonParam);
						}
						else if((int)jsonResponse["PollingTransacResult"]["total"].as_int() < (int)jsonParam["amountToPay"].as_int())
						{
							iRet = 203;
						}
					}
					else if(etat == "ABORTED" || etat == "REJECTED")
					{
						iRet = 203;
					}
					else if(etat == "TIMEOUT")
					{
						iRet = 204;
					}
					else if(etat == "ERROR")
					{
						iRet = 205;
					}

				}
			}
			else
			{
				iRet = 205;
			}
		}
	}

	return iRet;
}

bool Utils::terminateTransac(bool isValid, int isPre, int paidWithCB)
{
	bool bRet = false;
	Response response;
	cib::json::Document jsonBody;
	cib::json::Document jsonParam;
	cib::json::Document jsonResponse;

	loadDataAsJson(FIC_PARAM, jsonParam);
	if(jsonParam["shopId"] != "")
	{
		if(isValid)
		{
			jsonBody["id"] = jsonParam["orderId"];
			jsonBody["pre_or_transac"] = isPre;
			jsonBody["paidWithCB"] = paidWithCB;

			response = createRequest("/Webservices/rest/FO/wcfANCVPeriph.svc/ValidateTransaction", _POST, jsonBody.serialize());
		}
		else
		{
			jsonBody["id"] = jsonParam["orderId"];
			jsonBody["pre_or_transac"] = isPre;
			jsonBody["reason"] = "ABORTED_MERCHANT";
			jsonBody["cancelOrigine"] = 1;

			response = createRequest("/Webservices/rest/FO/wcfANCVPeriph.svc/AnnuleTransacPreTransac", _POST, jsonBody.serialize());
		}

		if (response.getStatusCode() == 200)
		{
			jsonResponse.parse(response.getContent().data());
			if(isValid)
			{
				if((bool)jsonResponse["ValidateTransactionResult"].as_bool() && jsonResponse["beneficiary_id"] == (string)jsonParam["beneficiaryId"].as_string())
				{
					bRet = (bool)jsonResponse["ValidateTransactionResult"].as_bool();
				}
			}
			else
			{
				bRet = (bool)jsonResponse["AnnuleTransacPreTransacResult"].as_bool();
			}
		}

	}
	return bRet;
}

bool Utils::connectWithShopId(string shopId)
{
	bool bRet = false;
	Response response;
	cib::json::Document jsonBody;
	cib::json::Document jsonParam;
	cib::json::Document jsonResponse;


	loadDataAsJson(FIC_PARAM, jsonParam);

	Utils::waitingWindow->drawing("Opération en cours", Waiting);
	// Construction du corps de la requête avec les informations d'authentification
	jsonBody["serialNumber"] = Terminal::ref().SerialNumber.substr(Terminal::ref().SerialNumber.size() - 8);
	jsonBody["typePeriph"] = Terminal::ref().TerminalType;
//	jsonBody["RequestHasLicense"]["serialNumber"] = "23136053";
//	jsonBody["RequestHasLicense"]["typePeriph"] = "M71";
	jsonBody["typeAppli"] = 0;
	jsonBody["shopId"] = shopId;

	// Effectuer la requête POST pour obtenir un nouveau token
	response = createRequest("/Webservices/rest/FO/wcfANCVPeriph.svc/checkLicence", _POST, jsonBody.serialize(), true);
	if (response.getStatusCode() == 200)
	{
		jsonResponse.parse(response.getContent().data());
		if((string)jsonResponse["checkLicenceResult"].as_string() != "")
		{
			jsonParam["shopId"] = shopId;
			saveDataAsJson(FIC_PARAM, jsonParam);
			bRet = true;
		}
	}


	if(!bRet)
	{
		string msg = "Une erreur réseau est survenue";
		if(response.getStatusCode() == 200)
		{
			msg = "ShopID incorrect";
			jsonParam["shopId"] = "";
			isConnected = false;
			saveDataAsJson(FIC_PARAM, jsonParam);
		}

		Utils::waitingWindow->drawing(msg, Cancel);
	}


	return bRet;
}

bool Utils::checkLicense()
{
	bool bRet = false;
	Response response;
	cib::json::Document jsonBody;
	cib::json::Document jsonParam;
	cib::json::Document jsonResponse;

	loadDataAsJson(FIC_PARAM, jsonParam);


	if(jsonParam["shopId"] != "")
	{
		Utils::waitingWindow->drawing("Opération en cours", Waiting);

		//TODO REMOVE
	//	response = createRequest("/Webservices/rest/FO/wcfANCVPeriph.svc/GetShopId/23136053", _GET, "", true);
		response = createRequest("/Webservices/rest/FO/wcfANCVPeriph.svc/GetShopId/"+Terminal::ref().SerialNumber.substr(Terminal::ref().SerialNumber.size() - 8), _GET, "", true);

		if (response.getStatusCode() == 200)
		{
			jsonResponse.parse(response.getContent().data());
			if( (m_host == HOST_PROD && (int)jsonResponse["GetShopIdResult"].as_int() > 0) || (m_host == HOST_DEV && !((string)jsonResponse["GetShopIdResult"].as_string()).empty()))
			{

				if(m_host == HOST_PROD)
				{
					std::ostringstream oss;
					oss << (int)jsonResponse["GetShopIdResult"].as_int();
					std::string s = oss.str();
					jsonParam["shopId"] = s;
				}
				else
				{
					jsonParam["shopId"] = (string)jsonResponse["GetShopIdResult"].as_string();
				}
				isConnected = true;
				bRet = true;
			}
		}
		saveDataAsJson(FIC_PARAM, jsonParam);
		string msg = "ID client valide";
		WaitingStep step = Valid;
		if(!bRet)
		{
			step = Cancel;
			msg = "Id Inconnu";
			if(response.getStatusCode() != 200)
			{
				msg = "Une erreur réseau est survenue";
			}
		}
		Utils::waitingWindow->drawing(msg, step);
	}
	return bRet;
}

bool Utils::getHistoric(string dateFrom, string dateTo)
{
	bool bRet = false;
	Response response;
	cib::json::Document jsonBody;
	cib::json::Document jsonParam;
	cib::json::Document jsonResponse;

	loadDataAsJson(FIC_PARAM, jsonParam);

	std::tm tmFrom;
	std::tm tmTo;
	std::memset(&tmFrom, 0, sizeof(std::tm));
	std::memset(&tmTo,   0, sizeof(std::tm));

	if(jsonParam["shopId"] != "")
	{
		Utils::waitingWindow->drawing("Opération en cours", Waiting);
		parseDdMmYyyy(dateFrom, tmFrom);
		parseDdMmYyyy(dateTo, tmTo);

		jsonBody["serialNumber"] = Terminal::ref().SerialNumber.substr(Terminal::ref().SerialNumber.size() - 8);
		jsonBody["dateFrom"] = formatIso(tmFrom,false);
		jsonBody["dateTo"] = formatIso(tmTo,true);

		response = createRequest("/Webservices/rest/FO/wcfANCVPeriph.svc/GetRevenue", _POST, jsonBody.serialize(), true);

		if (response.getStatusCode() == 200)
		{
			jsonResponse.parse(response.getContent().data());
			if((int)jsonResponse["Total"].as_int() >= 0 )
			{
				jsonParam["Historic"] = json::Document(json::VALUE_IS_OBJECT);
				jsonParam["Historic"]["Total"] = (int)jsonResponse["Total"].as_int();
				jsonParam["Historic"]["TotalANCV"] = (int)jsonResponse["TotalANCV"].as_int();
				jsonParam["Historic"]["TotalCB"] = (int)jsonResponse["TotalCB"].as_int();
				saveDataAsJson(FIC_PARAM, jsonParam);
				Utils::waitingWindow->hidding();
				bRet = true;
			}
		}
		saveDataAsJson(FIC_PARAM, jsonParam);
		if(!bRet)
		{
			WaitingStep step = Cancel;
			string msg = "Une erreur est survenue";
			if(response.getStatusCode() != 200)
			{
				msg = "Une erreur réseau est survenue";
			}
			Utils::waitingWindow->drawing(msg, step);
		}

	}

	if(bRet && YesNoWindow(glib, "IMPRIMER TOTAUX",0).drawing())
	{
		CvConnectTicket* t = new CvConnectTicket();
		t->printPaymentSummary(dateFrom, dateTo);
	}
	return bRet;
}

ulong Utils::displayDualScreen(const std::string & title, const std::string & text, T_GL_DIALOG_ICON icon, T_GL_BUTTONS buttons, T_GL_DURATION duration)
{
	ulong ulRet;

	SGL::ref().openApplicationWindow();
	if (getClientGL().getViewer() == GL_VIEWER_PINPAD)
	{
		PadSGL::ref().openApplicationWindow();
		PadSGL::ref().dialogMessage(title, text, icon, buttons, duration);
		PadSGL::ref().closeApplicationWindow();
	}
	ulRet = SGL::ref().dialogMessage(title, text, icon, buttons, duration);
	SGL::ref().closeApplicationWindow();

	return ulRet;
}



string Utils::getIconsPath(string name)
{
	std::ostringstream iconPath;
	iconPath << "file://param/" << "DATA54F9250"<< ".tar/Icons/" << name << ".png";
	return iconPath.str();
}



void Utils::copyLogoToPinPad()
{
	GL_File_Copy(Utils::ptr()->getIconsPath("valid").c_str(), "file://flash/HOST/valid.png");
	GL_File_Copy(Utils::ptr()->getIconsPath("waiting_icon").c_str(), "file://flash/HOST/waiting.png");
	GL_File_Copy(Utils::ptr()->getIconsPath("cancelicon").c_str(), "file://flash/HOST/cancel.png");
	GL_File_Copy(Utils::ptr()->getIconsPath("ancvlogomini").c_str(), "file://flash/HOST/logo.png");
	GL_File_Copy(Utils::ptr()->getIconsPath("backgroundtpe").c_str(), "file://flash/HOST/background.png");
//	GL_File_Copy("/CVCONNECT/QRCODE.png", "file://flash/HOST/QRCODE.png");

	if(Ppad_IsConnected(PPAD_ID_0))
	{
		SGL::ref().injectResource("file://flash/HOST/valid.png");
		PadSGL::ref().injectResource("file://flash/HOST/valid.png");

		SGL::ref().injectResource("file://flash/HOST/cancel.png");
		PadSGL::ref().injectResource("file://flash/HOST/cancel.png");

		SGL::ref().injectResource("file://flash/HOST/logo.png");
		PadSGL::ref().injectResource("file://flash/HOST/logo.png");

		SGL::ref().injectResource("file://flash/HOST/background.png");
		PadSGL::ref().injectResource("file://flash/HOST/background.png");
	}
}

void Utils::fillTicketTransacData(AncvConnectData &data)
{
	cib::json::Document jsonParam;
	loadDataAsJson(FIC_PARAM, jsonParam);

	int amountTotal = (int)jsonParam["amountToPay"].as_int();
	int toComplete = (int)jsonParam["toComplete"].as_int();
	int rest = amountTotal - toComplete;

	double total = amountTotal / 100.0;
	std::ostringstream oss;
	oss << std::fixed << std::setprecision(2) << total;
	std::string amountStr = oss.str();

	double eurosToComplete = toComplete / 100.0;
	std::ostringstream oss2;
	oss2 << std::fixed << std::setprecision(2) << eurosToComplete;
	std::string amountStr2 = oss2.str();

	double restDiv = rest / 100.0;
	std::ostringstream oss3;
	oss3 << std::fixed << std::setprecision(2) << restDiv;
	std::string amountStr3 = oss3.str();


	data.beneficiaryId = (string)jsonParam["beneficiaryId"].as_string();
	data.transactionId = (string)jsonParam["paymentId"].as_string();
	data.orderId = (string)jsonParam["orderId"].as_string();
	data.dateOverride = (string)jsonParam["orderDate"].as_string();
	data.total = amountStr;
	data.payments.push_back(std::make_pair(std::string("CB"),   amountStr2 + " EUR"));
	data.payments.push_back(std::make_pair(std::string("ANCV"),  amountStr3 + " EUR"));
}

int Utils::createSSLProfile(string nameProfile,int count, ... )
{
	ssllib_open();
	int iError;
	int iResult = false;
	va_list args;
	va_start(args, count);

	cib::tools::security::deleteProfile(nameProfile.c_str());

	SSL_DeleteProfile(nameProfile.c_str());

	SSL_PROFILE_HANDLE hProfile = SSL_NewProfile(nameProfile.c_str(), &iError);

	if (hProfile != NULL)
	{
		if (SSL_ProfileSetProtocol(hProfile, TLSv1_2) == SSL_PROFILE_EOK)
		{
			long cipher = (SSL_kECDHE | SSL_aRSA | SSL_AES | SSL_SHA256 | SSL_SHA384);
			long strength = (SSL_HIGH | SSL_NOT_EXP);

			if (SSL_ProfileSetCipher(hProfile, cipher, strength) == SSL_PROFILE_EOK)
			{
				for(int i = 0; i< count; ++i)
				{
					 const char* arg = va_arg(args, const char*);
					 SSL_ProfileAddCertificateCA(hProfile, arg);
				}
				va_end(args);
				if (SSL_SaveProfile(hProfile) == SSL_PROFILE_EOK)
				{
					iResult = true;
				}
			}
		}

		iResult = !SSL_UnloadProfile(hProfile);
	}
	ssllib_close();
	return iResult;
}
void Utils::copyFromResources(string filepathResources, string filepathToCopy)
{
	std::ostringstream path;
	path << "file://param/" << "DATA54F9250"<< ".tar" << filepathResources;

	int sizeCsr = File::getSizeFile(path.str().c_str());
	if (sizeCsr > 0)
	{
		char* buffer = (char*)malloc(sizeCsr);
		loadDataParam(path.str().c_str(), buffer, sizeCsr);

		disk::saveData(filepathToCopy, buffer, sizeCsr, 1) > 0;
	}

}

