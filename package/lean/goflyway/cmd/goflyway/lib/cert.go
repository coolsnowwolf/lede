package lib

import (
	"crypto/rand"
	"crypto/rsa"
	"crypto/tls"
	"crypto/x509"
	"crypto/x509/pkix"
	"encoding/pem"
	"fmt"
	"io/ioutil"
	"math/big"
	"time"

	"github.com/coyove/goflyway/pkg/logg"
)

var caCert = []byte(`-----BEGIN CERTIFICATE-----
MIICxzCCAa+gAwIBAgIBATANBgkqhkiG9w0BAQsFADATMREwDwYDVQQDEwhnb2Zs
eXdheTAeFw0xNzEwMTMwMjMzMTZaFw0xODEwMTQwMjMzMTZaMBMxETAPBgNVBAMT
CGdvZmx5d2F5MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAm+bfpxzm
t8yGXeSnCvq0M1yTIN0P8BqUdZ3g4eu3jVGq8A9XJMxnmnzLXe1O1kGZbx13AB4m
ZPGYd+BGByErayZ2E6xqJO5lUOtSrWFAf/ujuyOngcn5pO5xABAoCHXIWRvmz9dC
kVirDJ8V/3Ai1mopuNz8975g0mKrJIFqqCErId0vNgM+gmnHPOPrsQYXkaSVggDI
f0qAEFu5/7vIspHaUPBDPS3TQRI2WowFkFJRDZ2m/K8Tokv/gpwHyEqo0gDNTqsZ
jd6d3kdPshJ6ZZblXkUbzeKZIpi9BYf2m3etvou0NjrzwgrcQLSONDCb4vAp9xzF
GuRAy8Kcy1fURwIDAQABoyYwJDAOBgNVHQ8BAf8EBAMCAf4wEgYDVR0TAQH/BAgw
BgEB/wIBAjANBgkqhkiG9w0BAQsFAAOCAQEAOrfvSD08n9yXBflsDF+15SEKuSgC
Z5iblpmfsvzZ9hLUvg3oRxaG62HviF1+ix4Xj5Pc+5PK0wp5ZnCRJu6UVljRruJP
tG02NjANjWU9seS2Mo7ktusIbaUhyRceXVgtIwOyVTXihYAd62DwaV+Q/TxHwf70
yXSZerJ1PXOMmwsgQy5OOiYoS0J1Mu554bDhMaBf59b9hBAwrIc0RRHuWAueBqkj
jytkfsQ4mchR69JsVgpnQP0bRXEUzUP7onbtiGaXlEb0fw3UgpnVasSlBTzn7OLG
pGsF209+iqSNF7QIGButylIN+0T8Va5iM8tLq4loNtwzrVG8E8IPzVs88Q==
-----END CERTIFICATE-----`)

var caKey = []byte(`-----BEGIN RSA PRIVATE KEY-----
MIIEoQIBAAKCAQEAm+bfpxzmt8yGXeSnCvq0M1yTIN0P8BqUdZ3g4eu3jVGq8A9X
JMxnmnzLXe1O1kGZbx13AB4mZPGYd+BGByErayZ2E6xqJO5lUOtSrWFAf/ujuyOn
gcn5pO5xABAoCHXIWRvmz9dCkVirDJ8V/3Ai1mopuNz8975g0mKrJIFqqCErId0v
NgM+gmnHPOPrsQYXkaSVggDIf0qAEFu5/7vIspHaUPBDPS3TQRI2WowFkFJRDZ2m
/K8Tokv/gpwHyEqo0gDNTqsZjd6d3kdPshJ6ZZblXkUbzeKZIpi9BYf2m3etvou0
NjrzwgrcQLSONDCb4vAp9xzFGuRAy8Kcy1fURwIDAQABAoIBACdWvY60iz9Lu4gL
v17U3g2ryXBcd74VZuq62NkJGrrtJZ4HsOyXK3lfDg34aJFF4kqYe8Oq1/NWxytQ
izsXUpZqERdrxSjmguhFSQp45Mamn60EFSt55KWKiTcD3KYt2k0p7jb50VFNt3Oq
3OkkyZWjGj4MfUSiZFoSDa9bfKdJunCpFefvdfyTxqGDj7DKf0YwLFcvthtRy9hi
f2YyKiU4sVP3mQCRHHTlN6ZBHE6XnTkt8moBkNRgpi5EmkiHbvaZ7+4jhzhIhJu1
YgxE4Rycs1FYQ+GwO6xckJylO1nNQhQ44IF1QNEYjppYDWlB8yAmanIv7GnDo56g
tDjr5eECgYEAwSuk5XyO5yKVP/CnTRGCa8KNb6jRO103ESHgua7TnpF3/D7j4jD8
7o1P988T1ffnHdkv4x8V2vgQtU8otf6tS4UYwEYJMF3MgQtO7j41znCpIq0Ci8sg
0iPCjrxMLAFV47/IIyvz6ZwoIIVEtCLZNABHce9p/oKhR0h1i0Ivh1ECgYEAzpwN
vS1dxaQSVrNBkfhaieF1ckz6hrlGXIXD9sD99Ktb/cuRK7cIBk9jft/LjgvPp8qK
OXSsJBej0hL124PVUZDSF2eJMtTFbJde5Vnu8r2+cBqEmTziW6kOTDzKKkFTp0Pe
iaaaSgfFO3Z6FpNy7JERmubrqDYlUtyiNpYU7BcCgYAa2gxD8jqTGpWQm81Fw0dT
32l1ugvLqrt64DCy3wB/rEz3fW6EBLdEGdSh5+Ej/x/bYFNtyOZq18zKx0QreEly
k//KynGAjaldzsqcB33HQpXgQhh2gjf/vgTPfpFAFLdxvzj82qE/OfvB+YSBXc6/
kcJcriEXxeTRWdstDng44QKBgQCE6xxjojX7mow5vuTmNbJ5ldRrDl72IWfEykHC
E3LN3xg1dKsWRpAhDx3hV7Sq06pqNgEq6YZ9ScAYvB3zdSLdcJ/YcsFvNDc2n0Nr
pan1/F/x2v2HYAX4FhCet1UQTiq1JxKZ19bW3Zzjnlr4/y8mRkXfM8RT2VG1tz+c
gn0/TQJ/CienJPLtmLFlWIaO+XXfJtW45Mz21Gdf+Z7JmPpumT+mhD9Z025JtoZN
PhnS0Vmp6MHpXXDE3jbBprBvSZblhPODgq42OZWA1wfHag1WBvvPrGhhDNVhJcdT
b8QbrT0oXmfbvU4hlFemLVNAmQcfKTZj4ECvEgEcx9h1KydouA==
-----END RSA PRIVATE KEY-----`)

func TryLoadCert() tls.Certificate {

	if cert, err := ioutil.ReadFile("ca.pem"); err == nil {
		caCert = cert
		fmt.Println("* ca.pem loaded, read", len(cert), "bytes")
	} else {
		fmt.Println("* ca.pem not found, using the default one")
	}

	if key, err := ioutil.ReadFile("key.pem"); err == nil {
		caKey = key
		fmt.Println("* key.pem loaded, read", len(key), "bytes")
	} else {
		fmt.Println("* key.pem not found, using the default one")
	}

	ca, err := tls.X509KeyPair(caCert, caKey)
	if err != nil {
		logg.F(err)
	}

	return ca
}

func GenCA(name string) (certPEM, keyPEM []byte, err error) {
	tmpl := &x509.Certificate{
		SerialNumber: big.NewInt(1),
		Subject:      pkix.Name{CommonName: name},
		NotBefore:    time.Now().AddDate(0, 0, -1),
		NotAfter:     time.Now().AddDate(1, 0, 0),
		KeyUsage: x509.KeyUsageDigitalSignature |
			x509.KeyUsageContentCommitment |
			x509.KeyUsageKeyEncipherment |
			x509.KeyUsageDataEncipherment |
			x509.KeyUsageKeyAgreement |
			x509.KeyUsageCertSign |
			x509.KeyUsageCRLSign,
		BasicConstraintsValid: true,
		IsCA:               true,
		MaxPathLen:         2,
		SignatureAlgorithm: x509.SHA256WithRSA,
	}

	key, err := rsa.GenerateKey(rand.Reader, 2048)
	if err != nil {
		return
	}

	certDER, err := x509.CreateCertificate(rand.Reader, tmpl, tmpl, key.Public(), key)
	if err != nil {
		return
	}

	keyDER := x509.MarshalPKCS1PrivateKey(key)

	certPEM = pem.EncodeToMemory(&pem.Block{
		Type:  "CERTIFICATE",
		Bytes: certDER,
	})

	keyPEM = pem.EncodeToMemory(&pem.Block{
		Type:  "RSA PRIVATE KEY",
		Bytes: keyDER,
	})

	return
}
