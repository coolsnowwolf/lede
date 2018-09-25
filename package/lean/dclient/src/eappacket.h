#ifndef DPACKET_H_
#define DPACKET_H_

#include <cstdint>
#include <string>

/* default snap length (maximum bytes per packet to capture) */
#define SNAP_LEN 1518

/* ethernet headers are always exactly 14 bytes */
#define SIZE_ETHERNET 14

#define OFFSET_EAPOL    0X0E
#define OFFSET_EAP      0X12
#define ETHER_ADDR_LEN  6


class EapPacket
{
public:
    EapPacket();
    ~EapPacket();

    void init(const std::string & user, const std::string & passwd, uint8_t* mac);

    //int getEapType();

    void eapSuccess();
    void eapFailure();
    void responseIdentity();
    void responseMd5Challenge();
    void keepAlive();
    void notificationMessage();
    void severInfo();

protected:
    uint8_t eapol_start[64];                // EAPOL Start 报文,18
	uint8_t eapol_logoff[64];               // EAPOL LogOff 报文,18
	uint8_t eap_response_ident[64];         // EAP RESPON/IDENTITY报文,23+userlen
	uint8_t eap_response_md5ch[64];         // EAP RESPON/MD5 报文,40+userlen
	uint8_t eap_life_keeping[70];           // EAP Key 报文,66

    struct ether_header                         // Ethernet II Header
    {
        uint8_t ether_dhost[ETHER_ADDR_LEN];     // Destionation
        uint8_t ether_shost[ETHER_ADDR_LEN];     // Source
        uint16_t ether_type;                     // Type
    };

    struct eap_header
    {
        uint8_t eapol_v;                         // Version
        uint8_t eapol_t;                         // Type
        uint16_t eapol_length;                   // Length
        uint8_t eap_t;
        uint8_t eap_id;
        uint16_t eap_length;
        uint8_t eap_op;
        uint8_t eap_v_length;
        uint8_t eap_md5_challenge[16];
    };
    std::string user;                            // 用户名
    std::string passwd;                          // 密码

    uint8_t muticast_mac[ETHER_ADDR_LEN];   // 认证服务器多播地址
    uint8_t local_mac[ETHER_ADDR_LEN];      // 本机网卡mac地址

    struct eap_header * eap_head;
    uint8_t *           packet;

    char msg[300];                          // 服务器信息
};

#endif // DPACKET_H_
