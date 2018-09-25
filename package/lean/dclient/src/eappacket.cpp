
#include <memory.h>
#include "global.h"
#include "eappacket.h"
#include "encrypt.h"
#include "conv/conv.h"

EapPacket::EapPacket()
{
    //ZTE认证服务器多播地址
    muticast_mac[0] = 0x01;
    muticast_mac[1] = 0x80;
    muticast_mac[2] = 0xc2;
    muticast_mac[3] = 0x00;
    muticast_mac[4] = 0x00;
    muticast_mac[5] = 0x03;


    // 初始化各个报文数组
    memset(eapol_start, 0, sizeof(eapol_start));
    memset(eapol_logoff, 0, sizeof(eapol_logoff));
    memset(eap_response_ident, 0, sizeof(eap_response_ident));
    memset(eap_response_md5ch, 0, sizeof(eap_response_md5ch));
    memset(eap_life_keeping, 0, sizeof(eap_life_keeping));


    // EAPOL Start
    uint8_t start_data[4] = {0x01, 0x01, 0x00, 0x00};
    memcpy(eapol_start + OFFSET_EAPOL, start_data, sizeof(start_data));

    // EAPOL Logoff
    uint8_t logoff_data[4] = {0x01, 0x02, 0x00, 0x00};
    memcpy(eapol_logoff + OFFSET_EAPOL, logoff_data, sizeof(logoff_data));

    // EAP RESPONSE IDENTITY
    uint8_t resp_iden_head[9] =
    {   0x01,           // Version: 802.1X-2001 (1)
        0x00,           // Type: EAP Packet (0)
        0x00, 0x00,     // Length: 00 (eapol_length, 以后填充)
        0x02,           // Code: Response (2)
        0x00,           // Id: 0 (以后填充, 可不填)
        0x00, 0x00,     // Length: 00 (eap_length, 以后填充)
        0x01            // Type: Identity (1)
    };
    memcpy(eap_response_ident + SIZE_ETHERNET, resp_iden_head, 9);

    // EAP RESPONSE MD5 Challenge
    uint8_t resp_md5_head[10] =
    {
        0x01,           // Version: 802.1X-2001 (1)
        0x00,           // Type: EAP Packet (0)
		0x00, 0x00,     // Length: 00 (eapol_length, 以后填充)
		0x02,           // Code: Response (2)
		0x00,           // Id: 0 (以后填充, 可不填)
		0x00, 0x00,     // Length: 00 (eap_length, 以后填充)
		0x04, 0x10      // Expert Info (以后填充)
    };
    memcpy(eap_response_md5ch + SIZE_ETHERNET, resp_md5_head, 10);


	// EAP Key
    uint8_t keep_alive[52] = {
		0x01,           // Version: 802.1X-2001 (1)
		0x03,           // Type: Key (3)
		0x00, 0x30,     // Length: 30
		0x01,           // Key Descriptor Type: RC4 Descriptor (1)
		0x00, 0x04,     // Key Length: 4
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// Replay Counter: 8字节
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// 16字节的Key IV 前8字节
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// 16字节的Key IV 后8字节
		0x00,			                                // Key Index: 0x00
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// 16字节的Key Signature 前8字节
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// 16字节的Key Signature 后8字节
		0x00, 0x00, 0x00, 0x00							// 4字节的Key
	};
	memcpy(eap_life_keeping + SIZE_ETHERNET, keep_alive, 52);
}

EapPacket::~EapPacket()
{

}

void EapPacket::init(const std::string & user, const std::string & passwd, uint8_t* mac)
{
    // 初始化用户名和密码
    this->user = user;
    this->passwd = passwd;

    // 初始化mac
    memset(local_mac, 0, ETHER_ADDR_LEN);
    memcpy(local_mac, mac, ETHER_ADDR_LEN);

    // 初始化Ethernet II Header
    uint8_t eapol_eth_header[SIZE_ETHERNET];
    struct ether_header * eth = (struct ether_header*)eapol_eth_header;
    memcpy(eth->ether_dhost, muticast_mac, ETHER_ADDR_LEN);     // 填充中兴认证多播地址
    memcpy(eth->ether_shost, local_mac, ETHER_ADDR_LEN);        // 填充本地MAC
    eth->ether_type = htons(0x888e);                            // 填充协议类型，0x888e是EAP协议

    // 各个报文填入Ethernet II Header
    memcpy(eapol_start, eapol_eth_header, SIZE_ETHERNET);
    memcpy(eapol_logoff, eapol_eth_header, SIZE_ETHERNET);
    memcpy(eap_response_ident, eapol_eth_header, SIZE_ETHERNET);
    memcpy(eap_response_md5ch, eapol_eth_header, SIZE_ETHERNET);
    memcpy(eap_life_keeping, eapol_eth_header, SIZE_ETHERNET);

    // 完善EAP RESPONSE IDENTITY报文信息
    eap_response_ident[SIZE_ETHERNET + 3] = 5 + (uint8_t)user.length();
    eap_response_ident[SIZE_ETHERNET + 7] = 5 + (uint8_t)user.length();
    memcpy(eap_response_ident + SIZE_ETHERNET + 9, user.c_str(), user.length());

    // 完善EAP RESPONSE MD5 Challenge报文信息
    eap_response_md5ch[SIZE_ETHERNET + 3] = 22 + (uint8_t)user.length();
	eap_response_md5ch[SIZE_ETHERNET + 7] = 22 + (uint8_t)user.length();
    memcpy(eap_response_md5ch + SIZE_ETHERNET + 10 + 16, user.c_str(), user.length());

    // 完善Frame check sequence
    int data_len;
	unsigned char *checksum;

	data_len = sizeof(eapol_start) - 4;
	checksum = Get_CRC32(eapol_start, data_len); // CRC32运算
    memcpy(eapol_start + data_len, checksum, 4); // 写入FCS校验码

    data_len = sizeof(eapol_logoff) - 4;
	checksum = Get_CRC32(eapol_logoff, data_len); // CRC32运算
    memcpy(eapol_logoff + data_len, checksum, 4); // 写入FCS校验码
}

//int EapPacket::getEapType(){}

void EapPacket::eapSuccess()
{
    // TODO(Zeyes) : 之后处理状态的时候用
}

void EapPacket::eapFailure()
{
    // TODO(Zeyes) : 之后处理状态的时候用
}

void EapPacket::responseIdentity()
{
    eap_response_ident[0x13] = eap_head->eap_id;

    int data_len;
    unsigned char *checksum;
    data_len = sizeof(eap_response_ident) - 4;
	checksum = Get_CRC32(eap_response_ident, data_len); // CRC32运算
    memcpy(eap_response_ident + data_len, checksum, 4); // 写入FCS校验码
}

void EapPacket::responseMd5Challenge()
{
    // MD5摘要 = MD5(REQ/MD5包ID + 用户密码 + "zte142052" + EAP-MD5 Value)
    char text[] = "zte142052";
    char* data = (char*)malloc((1 + passwd.length() + 9 + 16) * sizeof(char));
    data[0] = eap_head->eap_id;
    memcpy(data + 1, passwd.c_str(), passwd.length());
    memcpy(data + 1 + passwd.length(), text, 9);
    memcpy(data + 1 + passwd.length() + 9, eap_head->eap_md5_challenge, 16);
    unsigned char * digest;
    digest = Get_Md5_Digest((unsigned char *)data, 1 + passwd.length() + 9 + 16);
    memcpy(eap_response_md5ch + 14 + 10, digest, 16);
    eap_response_md5ch[0x13] = eap_head->eap_id;
    free(data);

    int data_len = sizeof(eap_response_md5ch) - 4;
	unsigned char *checksum;
	checksum = Get_CRC32(eap_response_md5ch, data_len); // CRC32运算
    memcpy(eap_response_md5ch + data_len, checksum, 4); // 写入FCS校验码
}

void EapPacket::keepAlive()
{
    memcpy(eap_life_keeping + 21, packet + 21, 25); // Replay Counter + Key IV + Key Index
    // 使用rc4算法生成Key，基于（Key IV + Key IV最后四个字节）== 20字节
    uint8_t enckey[] = { 0x02, 0x02, 0x14, 0x00 }; // 要加密的数据，固定
    uint8_t wholekey[20];                          // 加密秘钥
    memcpy(wholekey, packet + 29, 16);
    memcpy(wholekey + 16, packet + 41, 4);
    Get_RC4(enckey, 4, wholekey, 20);               // 进行RC4运算
    memcpy(eap_life_keeping + 62, enckey, 4);

    // 使用hmac_md5算法生成Key Signature，此用于包的校验
	u_char encDat[64];
	unsigned char * deckey;
	memset(eap_life_keeping + 46, 0, 16);       // Key Signature的部分先填充0
	memcpy(encDat, eap_life_keeping + 14, 52);  // 要加密的数据
	enckey[0] = eap_life_keeping[45];           // 加密秘钥
	deckey = Get_Hmac_Md5_Digest(encDat, 52, enckey, 1);  // 进行HmacMD5运算
	memcpy(eap_life_keeping + 46, deckey, 16);

	int data_len = sizeof(eap_life_keeping) - 4;
	unsigned char *checksum;
	checksum = Get_CRC32(eap_life_keeping, data_len); // CRC32运算
    memcpy(eap_life_keeping + data_len, checksum, 4); // 写入FCS校验码
}

void EapPacket::notificationMessage()
{
    char msg_length[2] = { 0 };
	int length;
	memcpy(msg_length, (packet + 20), 2);
	length = (int)(msg_length[0] * 10 + msg_length[1]) - 5;
	if (length > 0) {
		memcpy(msg, (packet + 23), length);
		msg[length] = '\0';
#ifdef DEBUG
        //PrintHex((unsigned char *)msg, strlen(msg));
#endif
        const char * tmp = g2u(msg); // 将GBK编码转成UTF-8
        memcpy((void *)msg, (void *)tmp, strlen(tmp) + 1);
	}
	else
        memset(msg, 0, sizeof(msg));
}

void EapPacket::severInfo()
{
    char msg_length[2] = {0};
    int length;
    memcpy(msg_length, (packet + 20), 2);
    length = (int)(msg_length[0] * 10 + msg_length[1]) - 6;
    if(length > 0)
    {
        memcpy(msg, (packet + 24), length);
        if(msg[9] == '\0' && msg[10] != '\0') msg[9] = ':';
        msg[length] = '\0';
#ifdef DEBUG
        //PrintHex((unsigned char *)msg, strlen(msg));
#endif
        const char * tmp = g2u(msg); // 将GBK编码转成UTF-8
        memcpy((void *)msg, (void *)tmp, strlen(tmp) + 1);
    }
    else
        memset(msg, 0, sizeof(msg));
}
